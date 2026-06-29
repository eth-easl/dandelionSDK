/* THIS IS A DUMMY HEADER */

#ifndef _IFADDRS_H
#define _IFADDRS_H 1

#include <sys/socket.h>

struct ifaddrs {
  struct ifaddrs *ifa_next;

  char *ifa_name;
  unsigned int ifa_flags;

  struct sockaddr *ifa_addr;
  struct sockaddr *ifa_netmask;
  union {
    struct sockaddr *ifu_broadaddr;
    struct sockaddr *ifu_dstaddr;
  } ifa_ifu;

  void *ifa_data;
};

extern int getifaddrs(struct ifaddrs **__ifap);
extern void freeifaddrs(struct ifaddrs *__ifa);

#endif /* ifaddrs.h */
