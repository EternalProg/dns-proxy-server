#ifndef DNS_PROXY_ERROR_H
#define DNS_PROXY_ERROR_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_NONE 4

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

static inline const char *_log_timestamp() {
  static char buffer[20];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(buffer, sizeof(buffer), "%H:%M:%S", t);
  return buffer;
}

#define _LOG(level_str, fmt, ...)                                              \
  do {                                                                         \
    fprintf(stderr, "[%s] [%s] %s:%d: " fmt "\n", _log_timestamp(), level_str, \
            __FILE__, __LINE__, ##__VA_ARGS__);                                \
    fflush(stderr);                                                            \
  } while (0)

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...)                                                    \
  do {                                                                         \
    fprintf(stderr, "[%s] [DEBUG]: " fmt "\n", _log_timestamp(),               \
            ##__VA_ARGS__);                                                    \
    fflush(stderr);                                                            \
  } while (0)
#else
#define LOG_DEBUG(fmt, ...)                                                    \
  do {                                                                         \
  } while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...)                                                     \
  do {                                                                         \
    fprintf(stderr, "[%s] [INFO]: " fmt "\n", _log_timestamp(),                \
            ##__VA_ARGS__);                                                    \
    fflush(stderr);                                                            \
  } while (0)
#else
#define LOG_INFO(fmt, ...)                                                     \
  do {                                                                         \
  } while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(fmt, ...) _LOG("WARN", fmt, ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...)                                                     \
  do {                                                                         \
  } while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(fmt, ...) _LOG("ERROR", fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)                                                    \
  do {                                                                         \
  } while (0)
#endif

typedef enum {
  DNS_PROXY_OK,
  DNS_PROXY_ERR_CONFIG_FILE,
  DNS_PROXY_ERR_CONFIG_PARSE,
  DNS_PROXY_ERR_MISSING_FIELD,
  DNS_PROXY_ERR_INVALID_IP,
  DNS_PROXY_ERR_SOCKET,
  DNS_PROXY_ERR_BIND,
  DNS_PROXY_ERR_LISTEN,
  DNS_PROXY_ERR_CONNECTION,
  DNS_PROXY_ERR_MEMORY,
  DNS_PROXY_ERR_EPOLL,
  DNS_PROXY_ERR_UNKNOWN
} dns_proxy_error_t;

const char *dns_proxy_strerror(dns_proxy_error_t err);

#endif