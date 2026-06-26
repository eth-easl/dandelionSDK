#include <net/if.h>
#include <errno.h>

void if_freenameindex(struct if_nameindex *ptr) {
  (void)ptr;
  errno = ENOSYS;
}

char *if_indextoname(unsigned ifindex, char *ifname) {
  (void)ifindex;
  (void)ifname;
  errno = ENOSYS;
  return 0;
}

struct if_nameindex *if_nameindex(void) {
  errno = ENOSYS;
  return 0;
}

unsigned if_nametoindex(const char *ifname) {
  (void)ifname;
  errno = ENOSYS;
  return 0;
}
