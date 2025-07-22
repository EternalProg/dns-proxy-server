#include <dns_proxy_error.h>
#include <cyaml.h>
#include <dns_proxy_config.h>
#include <stdio.h>

static const cyaml_strval_t blacklist_response_type[] = {
    {"not_found", DNS_RESPONSE_NOT_FOUND},
    {"refused", DNS_RESPONSE_REFUSED},
    {"resolve", DNS_RESPONSE_RESOLVE},
};

static const cyaml_schema_value_t blacklist_entry_schema = {
    CYAML_VALUE_STRING(CYAML_FLAG_POINTER, char, 0, CYAML_UNLIMITED)};

static const cyaml_schema_field_t config_yaml_fields[] = {
    CYAML_FIELD_STRING_PTR("upstream_dns", CYAML_FLAG_POINTER,
                           dns_proxy_config_yaml_t, upstream_dns, 0, 64),
    CYAML_FIELD_STRING_PTR("blacklist_response", CYAML_FLAG_POINTER,
                           dns_proxy_config_yaml_t, blacklist_response, 0, 16),
    CYAML_FIELD_STRING_PTR("resolve_ip", CYAML_FLAG_OPTIONAL,
                           dns_proxy_config_yaml_t, resolve_ip, 0, 64),
    CYAML_FIELD_SEQUENCE("blacklist", CYAML_FLAG_POINTER,
                         dns_proxy_config_yaml_t, blacklist,
                         &blacklist_entry_schema, 0, CYAML_UNLIMITED),
    CYAML_FIELD_END};

static const cyaml_schema_value_t config_schema = {CYAML_VALUE_MAPPING(
    CYAML_FLAG_POINTER, dns_proxy_config_yaml_t, config_yaml_fields)};

static const cyaml_config_t cyaml_config = {
    .log_fn = cyaml_log,            /* Use the default logging function. */
    .mem_fn = cyaml_mem,            /* Use the default memory allocator. */
    .log_level = CYAML_LOG_WARNING, /* Logging errors and warnings only. */
};

dns_proxy_blacklist_response_t dns_proxy_parse_response_type(const char *str) {
  if (strcmp(str, "not_found") == 0)
    return DNS_RESPONSE_NOT_FOUND;
  if (strcmp(str, "refused") == 0)
    return DNS_RESPONSE_REFUSED;
  if (strcmp(str, "resolve") == 0)
    return DNS_RESPONSE_RESOLVE;
  return DNS_RESPONSE_UNKNOWN;
}

const char *dns_proxy_blacklist_response_to_string(
    dns_proxy_blacklist_response_t res_type) {
  switch (res_type) {
  case DNS_RESPONSE_NOT_FOUND:
    return "not_found";
  case DNS_RESPONSE_REFUSED:
    return "refused";
  case DNS_RESPONSE_RESOLVE:
    return "resolve";
  default:
    return "unknown";
  }
}

dns_proxy_error_t dns_proxy_load_config(const char *filename,
                                        dns_proxy_config_t *out_config) {
  dns_proxy_config_yaml_t *temp = NULL;
  dns_proxy_error_t status = DNS_PROXY_OK;
  cyaml_err_t cyaml_status = cyaml_load_file(
      filename, &cyaml_config, &config_schema, (void **)&temp, NULL);
  if (cyaml_status != CYAML_OK) {
    fprintf(stderr, "Error loading config file: %s\n",
            cyaml_strerror(cyaml_status));
    status = DNS_PROXY_ERR_CONFIG_FILE;
    goto cleanup;
  }

  memset(out_config, 0, sizeof(*out_config));
  if (temp->upstream_dns) {
    size_t len = sizeof(out_config->upstream_dns);
    strncpy(out_config->upstream_dns, temp->upstream_dns, len - 1);
    out_config->upstream_dns[len - 1] = '\0';
  } else {
    fprintf(stderr, "Missing 'upstream_dns' in config\n");
    status = DNS_PROXY_ERR_MISSING_FIELD;
    goto cleanup;
  }
  out_config->blacklist_response =
      dns_proxy_parse_response_type(temp->blacklist_response);
  if (out_config->blacklist_response == DNS_RESPONSE_RESOLVE) {
    if (!temp->resolve_ip) {
      fprintf(stderr,
              "Error: resolve_ip must be set when blacklist_response is "
              "'resolve'\n");
      status = DNS_PROXY_ERR_MISSING_FIELD;
      goto cleanup;
    }
    strncpy(out_config->resolve_ip, temp->resolve_ip,
            sizeof(out_config->resolve_ip) - 1);
  }

  for (unsigned i = 0; i < temp->blacklist_count; ++i) {
    blacklist_entry_t *entry = malloc(sizeof(blacklist_entry_t));
    if(!entry) {
      fprintf(stderr, "Memory allocation failed for blacklist entry\n");
      status = DNS_PROXY_ERR_MEMORY;
      goto cleanup;
    }
    strncpy(entry->dns_address, temp->blacklist[i], MAX_DNS_ADDRESS_LENGTH);
    HASH_ADD_STR(out_config->blacklist, dns_address, entry);
  }

  cyaml_free(&cyaml_config, &config_schema, temp, 0);
  return DNS_PROXY_OK;

cleanup:
  cyaml_free(&cyaml_config, &config_schema, temp, 0);
  return status;
}

void dns_proxy_config_delete(dns_proxy_config_t *config) {
  blacklist_entry_t *entry, *temp;
  HASH_ITER(hh, config->blacklist, entry, temp) {
    HASH_DEL(config->blacklist, entry);
    free(entry);
  }
}