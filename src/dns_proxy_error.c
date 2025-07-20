#include <dns_proxy_error.h>

const char *dns_proxy_strerror(dns_proxy_error_t err) {
  switch (err) {
    case DNS_PROXY_OK:
      return "No error";
    case DNS_PROXY_ERR_CONFIG_FILE:
      return "Failed to open config file";
    case DNS_PROXY_ERR_CONFIG_PARSE:
      return "YAML parse error";
    case DNS_PROXY_ERR_MISSING_FIELD:
      return "Missing required config field";
    case DNS_PROXY_ERR_INVALID_IP:
      return "Invalid IP address";
    case DNS_PROXY_ERR_SOCKET:
      return "Socket creation failed";
    case DNS_PROXY_ERR_BIND:
      return "Socket bind failed";
    case DNS_PROXY_ERR_MEMORY:
      return "Memory allocation failed";
    default:
      return "Unknown error";
  }
}
