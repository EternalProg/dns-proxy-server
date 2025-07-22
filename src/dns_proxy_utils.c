#include <dns_proxy_utils.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void dns_proxy_set_non_blocking(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

char *get_config_path(char *program_name) {
  const char *configFile = "config.yaml";
  char *absolutePath = malloc(PATH_MAX);

  char *resolvedPath = realpath(configFile, absolutePath);
  if (resolvedPath == NULL) {
    perror("realpath");
    return NULL;
  }
  if (resolvedPath[0] == '/') {
    return resolvedPath;
  } else {
    char *lastSlash = strrchr(program_name, '/');
    if (lastSlash != NULL) {
      *lastSlash = '\0';
      snprintf(absolutePath, PATH_MAX, "%s/%s", program_name, configFile);
      return absolutePath;
    } else {
      snprintf(absolutePath, PATH_MAX, "./%s", configFile);
      return absolutePath;
    }
  }
}