#include <dns_proxy_error.h>

static const char *DNS_PROXY_ERROR_MESSAGES[] = {
    "No error",           "Failed to open config file",
    "YAML parse error",   "Missing required config field",
    "Invalid IP address", "Socket creation failed",
    "Socket bind failed", "Socket listen failed",
    "Connection failed",  "Memory allocation failed",
    "Epoll error",        "Unknown error"};

const char *dns_proxy_strerror(dns_proxy_error_t err) {
  if (err < 0 || err >= sizeof(DNS_PROXY_ERROR_MESSAGES) /
                            sizeof(*DNS_PROXY_ERROR_MESSAGES)) {
    return "Invalid error code";
  }
  return DNS_PROXY_ERROR_MESSAGES[err];
}
