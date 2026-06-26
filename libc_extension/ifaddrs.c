#include <ifaddrs.h>
#include <errno.h>

int getifaddrs (struct ifaddrs **__ifap) {
  (void)__ifap;
  errno = ENOSYS;
  return -1;
}
void freeifaddrs (struct ifaddrs *__ifa) { return; }
