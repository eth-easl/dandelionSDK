#include <net/if.h>

unsigned int if_nametoindex (const char *__ifname) { return 0; }
char *if_indextoname (unsigned int __ifindex, char __ifname[IF_NAMESIZE]) { return 0; }
struct if_nameindex *if_nameindex (void) { return 0; }
void if_freenameindex (struct if_nameindex *__ptr) { return; }
