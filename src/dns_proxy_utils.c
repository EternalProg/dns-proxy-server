#include <fcntl.h>
#include <dns_proxy_utils.h>

void dns_proxy_set_non_blocking(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}