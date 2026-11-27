#include <arpa/inet.h>
#include <errno.h>

#if defined(__x86_64__) || defined(__aarch64__)
// reverse 4 byte number
uint32_t htonl(uint32_t host_num) {
  return (host_num << 24) | (host_num >> 24) |
    ((host_num & 0x00FF0000) >> 8) | ((host_num & 0x0000FF00) << 8);
}

uint16_t htons(uint16_t host_num){
  return (host_num << 8) | (host_num >> 8);
}
uint32_t ntohl(uint32_t host_num){
  return htonl(host_num);
}
uint16_t ntohs(uint16_t host_num){
  return htons(host_num);
}
#else
  #error "Compiling for machine with unknown endianness"
#endif

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
