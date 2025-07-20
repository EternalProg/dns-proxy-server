#include "dns_proxy_error.h"
#include <dns_proxy_server.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_COUNT 8
#define PORT 1053

char *get_config_path(char *program_name) {
  const char *configFile = "config.yaml";
  char *absolutePath = malloc(PATH_MAX);

  char *resolvedPath = realpath(configFile, absolutePath);
  if (resolvedPath == NULL) {
    perror("realpath");
    return NULL;
  }
  // Check if the resolved path is absolute
  if (resolvedPath[0] == '/') {
    return resolvedPath; // Return absolute path
  } else {
    // If not absolute, prepend the program's directory
    char *lastSlash = strrchr(program_name, '/');
    if (lastSlash != NULL) {
      *lastSlash = '\0'; // Remove the program name
      snprintf(absolutePath, PATH_MAX, "%s/%s", program_name, configFile);
      return absolutePath;
    } else {
      snprintf(absolutePath, PATH_MAX, "./%s", configFile);
      return absolutePath; // Fallback to current directory
    }
  }
}

int main(int argc, char *argv[]) {
  dns_proxy_config_t config = {0};
  const char *config_path = get_config_path(argv[0]);

  printf("PATH:%s\n", config_path);

  if (dns_proxy_load_config(config_path, &config) != DNS_PROXY_OK) {
    fprintf(stderr, "Failed to i  nitialize DNS proxy configuration from %s\n",
            config_path);
    return 1;
  }

  dns_proxy_server_t server;
  dns_proxy_error_t status =
      dns_proxy_server_init(PORT, THREAD_COUNT, &config, &server);
  if (status != DNS_PROXY_OK) {
    fprintf(stderr, "Failed to initialize DNS proxy server: %s\n",
            dns_proxy_strerror(status));
    dns_proxy_config_delete(&config);
    return 1;
  }

  printf("Upstream dns: %s\n", config.upstream_dns);
  if (config.blacklist_response == DNS_RESPONSE_RESOLVE) {
    printf("Resolve IP: %s\n", config.resolve_ip);
  }
  printf("Blacklist response: %s\n",
         dns_proxy_blacklist_response_to_string(config.blacklist_response));
  printf("Blacklist entries:\n");
  blacklist_entry_t *entry, *temp;
  HASH_ITER(hh, config.blacklist, entry, temp) {
    printf("\tBlacklist entry: %s\n", entry->dns_address);
  }
  printf("\n");

  if (dns_proxy_server_run(&server) != DNS_PROXY_OK) {
    fprintf(stderr, "Failed to run DNS proxy server\n");
    dns_proxy_server_delete(&server);
    return 1;
  }

  dns_proxy_server_delete(&server);

  return 0;
}