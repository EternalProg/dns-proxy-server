#ifndef DNS_PROXY_UTILS_H
#define DNS_PROXY_UTILS_H

void dns_proxy_set_non_blocking(int fd);

char *get_config_path(char *program_name);

#endif