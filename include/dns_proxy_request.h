#ifndef DNS_PROXY_JOB_QUEUE_H
#define DNS_PROXY_JOB_QUEUE_H

#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>

#define BUFFER_SIZE 512
#define MAX_DOMAIN_NAME_SIZE 64
#define JOB_QUEUE_CAPACITY 256

typedef int file_descriptor_t;
typedef uint8_t dns_packet_t;

typedef struct {
  uint16_t id;

  uint16_t flags;

  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
} dns_proxy_header_t;

typedef struct {
  char qname[MAX_DOMAIN_NAME_SIZE];
  uint16_t qtype;
  uint16_t qclass;
} dns_proxy_question_t;

typedef struct {
  dns_proxy_header_t header;
  dns_proxy_question_t question;
} dns_proxy_request_t;

void dns_proxy_parse_packet(dns_packet_t *packet, dns_proxy_request_t *request);

#endif