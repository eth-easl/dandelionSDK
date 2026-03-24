/* THIS IS A DUMMY HEADER */

#ifndef _NET_IF_H
#define _NET_IF_H	1

#define IF_NAMESIZE	16

struct if_nameindex {
    unsigned int if_index;
    char *if_name;
};

extern unsigned int if_nametoindex (const char *__ifname);
extern char *if_indextoname (unsigned int __ifindex, char __ifname[IF_NAMESIZE]);
extern struct if_nameindex *if_nameindex (void);
extern void if_freenameindex (struct if_nameindex *__ptr);

#endif /* net/if.h */
