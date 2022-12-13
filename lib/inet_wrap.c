#include "inet_wrap.h"

int _socket(int domain, int type, int protocol) {
  int socketfd;
  if ((socketfd = socket(domain, type, protocol)) < 0) {
    perror("socket error");
    return EXIT_FAILURE;
  }

  return socketfd;
}

int _inet_pton(int af, const char *__restrict cp, void *__restrict buf) {
  int result;
  if ((result = inet_pton(af, cp, buf)) <= 0) {
    perror("inet_pton error");
    return EXIT_FAILURE;
  }

  return result;
}

int _connect(int fd, const struct sockaddr *addr, socklen_t len) {
  int result;
  if ((result = connect(fd, addr, len)) < 0) {
    perror("connect error");
    return EXIT_FAILURE;
  }

  return result;
}

int _bind(int fd, const struct sockaddr *addr, socklen_t len) {
  int result;
  if ((result = bind(fd, addr, len)) < 0) {
    perror("bind error");
    return EXIT_FAILURE;
  }

  return result;
}

int _listen(int fd, int n) {
  int result;
  if ((result = listen(fd, n)) < 0) {
    perror("listen error");
    return EXIT_FAILURE;
  }

  return result;
}

int _accept(int fd, struct sockaddr *restrict addr,
            socklen_t *restrict addr_len) {
  int connfd;

  if ((connfd = accept(fd, addr, addr_len)) < 0) {
    perror("accept error");
    return EXIT_FAILURE;
  }

  return connfd;
}

int _close(int fd) {
  int result;
  if ((result = close(fd)) < 0) {
    perror("close error");
    return EXIT_FAILURE;
  }

  return result;
}

int _write(int fd, const void *buf, size_t n) {
  int result;
  if ((result = write(fd, buf, n)) < 0) {
    perror("write error");
    return EXIT_FAILURE;
  }

  return result;
}

int _setsockopt(int fd, int level, int optname, const void *optval,
                socklen_t optlen) {
  int result;
  if ((result = setsockopt(fd, level, optname, optval, optlen)) < 0) {
    perror("setsockopt error");
    return EXIT_FAILURE;
  }
  return result;
}

