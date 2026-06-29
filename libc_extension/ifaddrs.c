#include <errno.h>
#include <ifaddrs.h>

int getifaddrs(struct ifaddrs **__ifap) {
  (void)__ifap;
  errno = ENOSYS;
  return -1;
}
void freeifaddrs(struct ifaddrs *__ifa) { return; }
