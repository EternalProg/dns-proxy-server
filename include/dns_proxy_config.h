#ifndef DNS_PROXY_CONFIG_H
#define DNS_PROXY_CONFIG_H

#include <dns_proxy_error.h>
#include <uthash/uthash.h>

#define MAX_DNS_ADDRESS_LENGTH 256
typedef struct {
  char dns_address[MAX_DNS_ADDRESS_LENGTH];
  UT_hash_handle hh;
} blacklist_entry_t;

typedef enum {
  DNS_RESPONSE_RESOLVE = 0,
  DNS_RESPONSE_NOT_FOUND = 3,
  DNS_RESPONSE_REFUSED = 5,
  DNS_RESPONSE_UNKNOWN = -1
} dns_proxy_blacklist_response_t;

typedef struct {
  char upstream_dns[64];
  char resolve_ip[64];
  dns_proxy_blacklist_response_t blacklist_response;
  blacklist_entry_t *blacklist;
} dns_proxy_config_t;

typedef struct {
  char *upstream_dns;
  char *resolve_ip;
  char *blacklist_response;
  char **blacklist;
  unsigned blacklist_count;
} dns_proxy_config_yaml_t;

dns_proxy_blacklist_response_t dns_proxy_parse_response_type(const char *str);
const char *
dns_proxy_blacklist_response_to_string(dns_proxy_blacklist_response_t res_type);
dns_proxy_error_t dns_proxy_load_config(const char *filename,
                                        dns_proxy_config_t *config);
void dns_proxy_config_delete(dns_proxy_config_t *config);

#endif