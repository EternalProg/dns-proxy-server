#ifndef DNS_PROXY_H
#define DNS_PROXY_H

#include <dns_proxy_config.h>
#include <dns_proxy_error.h>
#include <sys/epoll.h>
#include <thpool.h>

typedef int file_descriptor_t;

#define MAX_EVENTS 256

typedef struct {
  threadpool thpool;
  dns_proxy_config_t *config;
  file_descriptor_t listen_fd;
  file_descriptor_t epoll_fd;
  struct epoll_event ev;
  struct epoll_event events[MAX_EVENTS];

  pthread_mutex_t upstream_lock;
} dns_proxy_server_t;

dns_proxy_error_t dns_proxy_server_init(int port, int thread_count,
                                        dns_proxy_config_t *config,
                                        dns_proxy_server_t *server);
void dns_proxy_server_delete(dns_proxy_server_t *server);
void dns_proxy_server_run(dns_proxy_server_t *server);

#endif