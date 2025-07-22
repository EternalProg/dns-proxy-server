#include <arpa/inet.h>
#include <dns_proxy_config.h>
#include <dns_proxy_error.h>
#include <dns_proxy_request.h>
#include <dns_proxy_server.h>
#include <dns_proxy_utils.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thpool.h>
#include <unistd.h>

typedef struct {
  dns_proxy_server_t *server;
  struct sockaddr_in client_addr;
  socklen_t addr_len;
  dns_packet_t packet[BUFFER_SIZE];
  ssize_t packet_size;
} dns_proxy_task_t;

static int dns_encode_qname(uint8_t *buffer, const char *domain) {
  int offset = 0;
  const char *label = domain;

  while (*label) {
    const char *dot = strchr(label, '.');
    size_t len = dot ? (size_t)(dot - label) : strlen(label);
    buffer[offset++] = (uint8_t)len;
    memcpy(buffer + offset, label, len);
    offset += len;
    if (!dot)
      break;
    label = dot + 1;
  }

  buffer[offset++] = 0x00; // end of QNAME
  return offset;
}

static dns_proxy_error_t
dns_proxy_create_listen_socket(int port, dns_proxy_server_t *server) {
  file_descriptor_t listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (listen_fd < 0) {
    return DNS_PROXY_ERR_SOCKET;
  }

  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_port = htons(port),
                             .sin_addr.s_addr = INADDR_ANY};

  if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(listen_fd);
    return DNS_PROXY_ERR_BIND;
  }

  dns_proxy_set_non_blocking(listen_fd);
  server->listen_fd = listen_fd;
  return DNS_PROXY_OK;
}

static dns_proxy_error_t
dns_proxy_configure_upstream(dns_proxy_server_t *server) {
  file_descriptor_t upstream_dns_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (upstream_dns_fd < 0) {
    return DNS_PROXY_ERR_SOCKET;
  }

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(DNS_UPSTREAM_PORT),
  };

  if (inet_pton(AF_INET, server->config->upstream_dns, &addr.sin_addr) <= 0) {
    close(upstream_dns_fd);
    return DNS_PROXY_ERR_INVALID_IP;
  }

  return DNS_PROXY_OK;
}

static dns_proxy_error_t
dns_proxy_server_configure_epoll(dns_proxy_server_t *server) {
  server->epoll_fd = epoll_create1(0);
  if (server->epoll_fd < 0) {
    LOG_ERROR("Failed to create epoll instance");
    return DNS_PROXY_ERR_EPOLL;
  }
  server->ev.events = EPOLLIN;
  server->ev.data.fd = server->listen_fd;
  if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->listen_fd,
                &server->ev) < 0) {
    LOG_ERROR("Failed to add listen socket to epoll");
    return DNS_PROXY_ERR_EPOLL;
  }
  return DNS_PROXY_OK;
}

static bool dns_proxy_check_blacklist(dns_proxy_config_t *config,
                                      dns_proxy_request_t *request) {
  blacklist_entry_t *entry = NULL;
  HASH_FIND_STR(config->blacklist, request->question.qname, entry);
  if (entry) {
    return true; // Domain is in blacklist
  }
  return false; // Domain is not in blacklist
}

static dns_proxy_error_t
dns_proxy_send_response(dns_proxy_server_t *server,
                        dns_proxy_request_t *request,
                        struct sockaddr_in *client_addr) {
  dns_proxy_blacklist_response_t response_type =
      server->config->blacklist_response;
  LOG_DEBUG("Blocked request for domain: %s\n", request->question.qname);

  dns_packet_t response_packet[BUFFER_SIZE];
  int offset = 0;

  dns_proxy_header_t response_header = {
      .id = htons(request->header.id),
      .flags = htons(0x8000), // Standard response
      .qdcount = htons(1),
      .ancount = 0,
      .nscount = 0,
      .arcount = 0,
  };

  switch (response_type) {
  case DNS_RESPONSE_NOT_FOUND:
    // NXDOMAIN (RCODE = 3)
    response_header.flags |= htons(0x0003);
    break;

  case DNS_RESPONSE_REFUSED:
    // REFUSED (RCODE = 5)
    response_header.flags |= htons(0x0005);
    break;

  case DNS_RESPONSE_RESOLVE:
    // NOERROR with answer
    response_header.flags |= htons(0x0000);
    response_header.ancount = htons(1);
    break;

  default:
    LOG_ERROR("Unknown blacklist response type: %d\n", response_type);
    return DNS_PROXY_ERR_UNKNOWN;
  }

  // Copy header
  memcpy(response_packet + offset, &response_header,
         sizeof(dns_proxy_header_t));
  offset += sizeof(dns_proxy_header_t);

  // Encode QNAME
  offset += dns_encode_qname(response_packet + offset, request->question.qname);

  // Add QTYPE and QCLASS
  uint16_t qtype = htons(request->question.qtype);
  uint16_t qclass = htons(request->question.qclass);
  memcpy(response_packet + offset, &qtype, sizeof(uint16_t));
  offset += sizeof(uint16_t);
  memcpy(response_packet + offset, &qclass, sizeof(uint16_t));
  offset += sizeof(uint16_t);

  if (response_type == DNS_RESPONSE_RESOLVE) {
    // Name (pointer to original name: 0xC00C)
    uint16_t name_ptr = htons(0xC00C);
    memcpy(response_packet + offset, &name_ptr, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // TYPE A (1)
    uint16_t type_a = htons(1);
    memcpy(response_packet + offset, &type_a, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // CLASS IN (1)
    uint16_t class_in = htons(1);
    memcpy(response_packet + offset, &class_in, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // TTL (4 bytes)
    uint32_t ttl = htonl(60); // 60 seconds
    memcpy(response_packet + offset, &ttl, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // RDLENGTH (4 bytes for IPv4)
    uint16_t rdlength = htons(4);
    memcpy(response_packet + offset, &rdlength, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // RDATA: IP address
    struct in_addr addr;
    if (inet_pton(AF_INET, server->config->resolve_ip, &addr) != 1) {
      LOG_ERROR("Invalid resolve IP\n");
      return DNS_PROXY_ERR_INVALID_IP;
    }

    memcpy(response_packet + offset, &addr.s_addr, 4);
    offset += 4;
  }

  // Send to client
  ssize_t sent = sendto(server->listen_fd, response_packet, offset, 0,
                        (struct sockaddr *)client_addr, sizeof(*client_addr));
  if (sent < 0) {
    LOG_ERROR("Failed to send blacklist response");
    return DNS_PROXY_ERR_CONNECTION;
  }

  return DNS_PROXY_OK;
}
static int
dns_proxy_create_upstream_socket_with_timeout(const char *ip,
                                              struct sockaddr_in *addr) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    return -1;

  struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  addr->sin_family = AF_INET;
  addr->sin_port = htons(53);

  if (inet_pton(AF_INET, ip, &addr->sin_addr) != 1) {
    close(fd);
    return -1;
  }

  return fd;
}
static void dns_proxy_worker_thread(void *arg) {
  dns_proxy_task_t *task = (dns_proxy_task_t *)arg;

  dns_proxy_request_t request;
  dns_proxy_parse_packet(task->packet, &request);
  LOG_DEBUG("Received request for domain: %s", request.question.qname);

  if (dns_proxy_check_blacklist(task->server->config, &request)) {
    dns_proxy_send_response(task->server, &request, &task->client_addr);
    free(task);
    return;
  }

  struct sockaddr_in upstream_addr;
  int upstream_fd = dns_proxy_create_upstream_socket_with_timeout(
      task->server->config->upstream_dns, &upstream_addr);

  if (upstream_fd < 0) {
    LOG_ERROR("Failed to create upstream socket");
    free(task);
    return;
  }

  ssize_t sent =
      sendto(upstream_fd, task->packet, task->packet_size, 0,
             (struct sockaddr *)&upstream_addr, sizeof(upstream_addr));
  if (sent < 0) {
    LOG_ERROR("sendto upstream failed: %s", strerror(errno));
    close(upstream_fd);
    free(task);
    return;
  }

  dns_packet_t response[BUFFER_SIZE];
  ssize_t recvd = recvfrom(upstream_fd, response, BUFFER_SIZE, 0, NULL, NULL);
  if (recvd < 0) {
    LOG_ERROR("recvfrom upstream failed: %s", strerror(errno));
    close(upstream_fd);
    free(task);
    return;
  }

  close(upstream_fd);

  ssize_t client_sent =
      sendto(task->server->listen_fd, response, recvd, 0,
             (struct sockaddr *)&task->client_addr, task->addr_len);
  if (client_sent < 0) {
    LOG_ERROR("sendto client failed: %s", strerror(errno));
  }

  free(task);
}

dns_proxy_error_t dns_proxy_server_init(int port, int thread_count,
                                        dns_proxy_config_t *config,
                                        dns_proxy_server_t *server) {
  if (!config || !server) {
    return DNS_PROXY_ERR_MEMORY;
  }

  dns_proxy_error_t status = dns_proxy_create_listen_socket(port, server);
  if (status != DNS_PROXY_OK) {
    return status;
  }

  server->config = config;
  server->thpool = thpool_init(thread_count);
  status = dns_proxy_configure_upstream(server);
  if (status != DNS_PROXY_OK) {
    LOG_ERROR("Failed to configure upstream DNS server: %s",
              dns_proxy_strerror(status));
    goto cleanup;
  }

  status = dns_proxy_server_configure_epoll(server);
  if (status != DNS_PROXY_OK) {
    LOG_ERROR("Failed to configure epoll: %s", dns_proxy_strerror(status));
    goto cleanup;
  }

  return status;

cleanup:
  thpool_destroy(server->thpool);
  return status;
}

void dns_proxy_server_delete(dns_proxy_server_t *server) {
  close(server->listen_fd);
  close(server->epoll_fd);
  dns_proxy_config_delete(server->config);
  free(server->config);
  thpool_destroy(server->thpool);
}

void dns_proxy_server_run(dns_proxy_server_t *server) {
  for (;;) {
    size_t event_count =
        epoll_wait(server->epoll_fd, server->events, MAX_EVENTS, -1);

    for (size_t i = 0; i < event_count; ++i) {
      int fd = server->events[i].data.fd;
      if (fd == server->listen_fd) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        dns_packet_t packet[BUFFER_SIZE];
        ssize_t packet_size =
            recvfrom(server->listen_fd, packet, BUFFER_SIZE, 0,
                     (struct sockaddr *)&client_addr, &addr_len);

        if (packet_size < 0) {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            continue;
          LOG_ERROR("recvfrom failed: %s", strerror(errno));
          continue;
        }

        dns_proxy_task_t *task = malloc(sizeof(dns_proxy_task_t));
        memcpy(&task->client_addr, &client_addr, sizeof(client_addr));
        task->addr_len = addr_len;
        task->packet_size = packet_size;
        memcpy(task->packet, packet, packet_size);
        task->server = server;

        thpool_add_work(server->thpool, (void *)dns_proxy_worker_thread,
                        (void *)task);
      }
    }
  }
}
