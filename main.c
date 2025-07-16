#include <dns_proxy.h>
#include <stdio.h>

int main(void) {
  dns_proxy_config config = {0};
  const char* path =
      "/home/eternal/Eternal/Programming/InangoTasks/DNS_Proxy/config.yaml";
  if (dns_proxy_initialize(path, &config) != DNS_PROXY_OK) {
    fprintf(stderr, "Failed to initialize DNS proxy configuration from %s\n",
            path);
    return 1;
  }

  printf("Upstream dns: %s\n", config.upstream_dns);
  if (config.blacklist_response == RESOLVE) {
    printf("Resolve IP: %s\n", config.resolve_ip);
  }
  printf("Blacklist response: %s\n\n",
         dns_proxy_blacklist_response_to_string(config.blacklist_response));
  blacklist_entry_t *entry, *temp;
  HASH_ITER(hh, config.blacklist, entry, temp) {
    printf("Blacklist entry: %s\n", entry->dns_address);
  }

  return 0;
}