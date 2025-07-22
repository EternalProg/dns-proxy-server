#include <dns_proxy_error.h>
#include <dns_proxy_server.h>
#include <dns_proxy_utils.h>

#define THREAD_COUNT 16
#define PORT 1053

int main(int argc, char *argv[]) {
  dns_proxy_config_t config = {0};
  const char *config_path = get_config_path(argv[0]);

  LOG_DEBUG("PATH:%s\n", config_path);

  if (dns_proxy_load_config(config_path, &config) != DNS_PROXY_OK) {
    LOG_ERROR("Failed to load config from %s", config_path);
    return 1;
  }

  dns_proxy_server_t server;
  dns_proxy_error_t status =
      dns_proxy_server_init(PORT, THREAD_COUNT, &config, &server);
  if (status != DNS_PROXY_OK) {
    LOG_ERROR("Failed to initialize DNS proxy server: %s",
              dns_proxy_strerror(status));
    dns_proxy_config_delete(&config);
    return 1;
  }

  LOG_DEBUG("Upstream dns: %s", config.upstream_dns);
  if (config.blacklist_response == DNS_RESPONSE_RESOLVE) {
    LOG_DEBUG("Resolve IP: %s", config.resolve_ip);
  }
  LOG_DEBUG("Blacklist response: %s",
            dns_proxy_blacklist_response_to_string(config.blacklist_response));
  LOG_DEBUG("Blacklist entries:");
  blacklist_entry_t *entry, *temp;
  HASH_ITER(hh, config.blacklist, entry, temp) {
    LOG_DEBUG("\tBlacklist entry: %s", entry->dns_address);
  }

  dns_proxy_server_run(&server);

  // dns_proxy_server_delete(&server);

  return 0;
}