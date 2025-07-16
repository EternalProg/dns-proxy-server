#include <dns_proxy.h>
#include <stdio.h>

dns_proxy_blacklist_response_t dns_proxy_parse_response_type(const char* str) {
  if (strcmp(str, "not_found") == 0) return NOT_FOUND;
  if (strcmp(str, "refused") == 0) return REFUSED;
  if (strcmp(str, "resolve") == 0) return RESOLVE;
  return UNKNOWN;
}

const char* dns_proxy_blacklist_response_to_string(
    dns_proxy_blacklist_response_t res_type) {
  switch (res_type) {
    case NOT_FOUND:
      return "not_found";
    case REFUSED:
      return "refused";
    case RESOLVE:
      return "resolve";
    default:
      return "unknown";
  }
}

dns_proxy_error_t dns_proxy_initialize(const char* filename,
                                       dns_proxy_config* out_config) {
  dns_proxy_config_yaml* temp = NULL;
  cyaml_err_t err = cyaml_load_file(filename, &cyaml_config, &config_schema,
                                    (void**)&temp, NULL);
  if (err != CYAML_OK) {
    fprintf(stderr, "Error loading config file: %s\n", cyaml_strerror(err));
    cyaml_free(&cyaml_config, &config_schema, temp, 0);
    return DNS_PROXY_ERR_CONFIG_PARSE;
  }

  memset(out_config, 0, sizeof(*out_config));
  strncpy(out_config->upstream_dns, temp->upstream_dns,
          sizeof(out_config->upstream_dns) - 1);
  out_config->blacklist_response =
      dns_proxy_parse_response_type(temp->blacklist_response);
  if (out_config->blacklist_response == RESOLVE) {
    if (!temp->resolve_ip) {
      fprintf(stderr,
              "Error: resolve_ip must be set when blacklist_response is "
              "'resolve'\n");
      cyaml_free(&cyaml_config, &config_schema, temp, 0);
      return DNS_PROXY_ERR_MISSING_FIELD;
    }
    strncpy(out_config->resolve_ip, temp->resolve_ip,
            sizeof(out_config->resolve_ip) - 1);
  }

  for (unsigned i = 0; i < temp->blacklist_count; ++i) {
    blacklist_entry_t* entry = malloc(sizeof(blacklist_entry_t));
    strncpy(entry->dns_address, temp->blacklist[i], MAX_DNS_ADDRESS_LENGTH);
    HASH_ADD_STR(out_config->blacklist, dns_address, entry);
  }

  cyaml_free(&cyaml_config, &config_schema, temp, 0);
  return DNS_PROXY_OK;
}

void dns_proxy_config_free(dns_proxy_config* config) {
  blacklist_entry_t *entry, *temp;
  HASH_ITER(hh, config->blacklist, entry, temp) {
    HASH_DEL(config->blacklist, entry);
    free(entry);
  }
}