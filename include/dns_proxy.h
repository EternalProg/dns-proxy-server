#ifndef DNS_PROXY_H
#define DNS_PROXY_H

#include <cyaml.h>
#include <dns_error.h>
#include <uthash/uthash.h>

#define MAX_DNS_ADDRESS_LENGTH 256

typedef struct {
  char dns_address[MAX_DNS_ADDRESS_LENGTH];
  UT_hash_handle hh;
} blacklist_entry_t;

typedef enum {
  NOT_FOUND,
  REFUSED,
  RESOLVE,
  UNKNOWN
} dns_proxy_blacklist_response_t;

typedef struct {
  char upstream_dns[64];
  char resolve_ip[64];
  dns_proxy_blacklist_response_t blacklist_response;
  blacklist_entry_t* blacklist;
} dns_proxy_config;

typedef struct {
  char* upstream_dns;
  char* resolve_ip;
  char* blacklist_response;
  char** blacklist;
  unsigned blacklist_count;
} dns_proxy_config_yaml;

static const cyaml_strval_t blacklist_response_type[] = {
    {"not_found", NOT_FOUND},
    {"refused", REFUSED},
    {"resolve", RESOLVE},
};

static const cyaml_schema_value_t blacklist_entry_schema = {
    CYAML_VALUE_STRING(CYAML_FLAG_POINTER, char, 0, CYAML_UNLIMITED)};

static const cyaml_schema_field_t config_yaml_fields[] = {
    CYAML_FIELD_STRING_PTR("upstream_dns", CYAML_FLAG_POINTER,
                           dns_proxy_config_yaml, upstream_dns, 0, 64),
    CYAML_FIELD_STRING_PTR("blacklist_response", CYAML_FLAG_POINTER,
                           dns_proxy_config_yaml, blacklist_response, 0, 16),
    CYAML_FIELD_STRING_PTR("resolve_ip", CYAML_FLAG_OPTIONAL,
                           dns_proxy_config_yaml, resolve_ip, 0, 64),
    CYAML_FIELD_SEQUENCE("blacklist", CYAML_FLAG_POINTER, dns_proxy_config_yaml,
                         blacklist, &blacklist_entry_schema, 0,
                         CYAML_UNLIMITED),
    CYAML_FIELD_END};

static const cyaml_schema_value_t config_schema = {CYAML_VALUE_MAPPING(
    CYAML_FLAG_POINTER, dns_proxy_config_yaml, config_yaml_fields)};

static const cyaml_config_t cyaml_config = {
    .log_fn = cyaml_log,            /* Use the default logging function. */
    .mem_fn = cyaml_mem,            /* Use the default memory allocator. */
    .log_level = CYAML_LOG_WARNING, /* Logging errors and warnings only. */
};

dns_proxy_blacklist_response_t dns_proxy_parse_response_type(const char* str);
const char* dns_proxy_blacklist_response_to_string(
    dns_proxy_blacklist_response_t res_type);
dns_proxy_error_t dns_proxy_initialize(const char* filename,
                                       dns_proxy_config* config);
void dns_proxy_config_free(dns_proxy_config* config);

#endif