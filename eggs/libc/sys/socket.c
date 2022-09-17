#include "sys/socket.h"

#include "netinet/in.h"

int socket(int domain, int type, int protocol) {
  printf("unimpl socket\n");
  return 1;
}

uint32_t ntohl(uint32_t n) {
  printf("unimpl ntohl\n");

  return n;
}
uint16_t htons(uint16_t n) {
  printf("unimpl htons\n");

  return n;
}
uint32_t htonl(uint32_t n) {
  printf("unimpl htonl\n");

  return n;
}

int setsockopt(int fd, int level, int optname, const void *optval,
               socklen_t optlen) {
  printf("unimpl setsockopt\n");
  return 1;
}

int connect(int fd, const struct sockaddr *addr, socklen_t len) {
  printf("unimpl connect\n");
  return 1;
}

int accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict len) {
  printf("unimpl connect\n");
  return 1;
}

int bind(int fd, const struct sockaddr *addr, socklen_t len) {
  printf("unimpl connect\n");
  return 1;
}

int listen(int fd, int backlog) {
  printf("unimpl connect\n");
  return 1;
}