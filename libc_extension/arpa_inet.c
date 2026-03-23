#include <arpa/inet.h>
#include <errno.h>

uint32_t htonl(uint32_t hostlong) {
  (void)hostlong;
  errno = ENOSYS;
  return 0;
}

in_addr_t inet_addr(const char *cp) {
  (void)cp;
  errno = ENOSYS;
  return (in_addr_t)-1;
}

char *inet_ntoa(struct in_addr in) {
  (void)in;
  errno = ENOSYS;
  return 0;
}

const char *inet_ntop(int af, const void *restrict src, char *restrict dst,
                      socklen_t size) {
  (void)af;
  (void)src;
  (void)dst;
  (void)size;
  errno = ENOSYS;
  return 0;
}

int inet_pton(int af, const char *restrict src, void *restrict dst) {
  (void)af;
  (void)src;
  (void)dst;
  errno = ENOSYS;
  return -1;
}
