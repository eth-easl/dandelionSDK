#ifndef _NET_IF_H_
#define _NET_IF_H_

#include <sys/cdefs.h>
/* https://man7.org/linux/man-pages/man0/net_if.h.0p.html */
/* https://git.musl-libc.org/cgit/musl/tree/include/net/if.h */
#define IF_NAMESIZE 16

struct if_nameindex {
  unsigned if_index;
  char *if_name;
};

__BEGIN_DECLS

void if_freenameindex(struct if_nameindex *);
char *if_indextoname(unsigned, char *);
struct if_nameindex *if_nameindex(void);
unsigned if_nametoindex(const char *);

__END_DECLS

#endif /* _NET_IF_H_ */
