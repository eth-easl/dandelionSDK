#ifndef _NETINET_IN_H_
#define _NETINET_IN_H_

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/* https://git.musl-libc.org/cgit/musl/tree/include/netinet/in.h */
#define IPPROTO_IP 0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define IPPROTO_IPV6 41
#define IPPROTO_RAW 255

/* https://git.musl-libc.org/cgit/musl/tree/include/netinet/in.h */
#define INADDR_ANY ((in_addr_t)0)
#define INADDR_BROADCAST ((in_addr_t)0xffffffff)
#define INADDR_NONE ((in_addr_t)0xffffffff)
#define INADDR_LOOPBACK ((in_addr_t)0x7f000001)

struct sockaddr_in {
  sa_family_t sin_family;
  in_port_t sin_port;
  struct in_addr sin_addr;
  uint8_t sin_zero[8];
};

struct in6_addr {
  union {
    uint8_t __s6_addr[16];
    uint16_t __s6_addr16[8];
    uint32_t __s6_addr32[4];
  } __in6_union;
};
#define s6_addr __in6_union.__s6_addr
#define s6_addr16 __in6_union.__s6_addr16
#define s6_addr32 __in6_union.__s6_addr32

struct sockaddr_in6 {
  sa_family_t sin6_family;
  in_port_t sin6_port;
  uint32_t sin6_flowinfo;
  struct in6_addr sin6_addr;
  uint32_t sin6_scope_id;
};

struct ipv6_mreq {
  struct in6_addr ipv6mr_multiaddr;
  unsigned ipv6mr_interface;
};

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

/* https://sources.debian.org/src/glibc/2.41-7/inet/netinet/in.h/ */
#define IN6ADDR_ANY_INIT { { { 0 } } }
#define IN6ADDR_LOOPBACK_INIT { { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } } }

/* https://git.musl-libc.org/cgit/musl/tree/include/netinet/in.h */
#define IPV6_UNICAST_HOPS 16
#define IPV6_MULTICAST_IF 17
#define IPV6_MULTICAST_HOPS 18
#define IPV6_MULTICAST_LOOP 19
#define IPV6_JOIN_GROUP 20
#define IPV6_LEAVE_GROUP 21
#define IPV6_V6ONLY 26

#define IN6_IS_ADDR_UNSPECIFIED(a) \
  ((a)->s6_addr32[0] == 0 && (a)->s6_addr32[1] == 0 && \
   (a)->s6_addr32[2] == 0 && (a)->s6_addr32[3] == 0)
#define IN6_IS_ADDR_LOOPBACK(a) \
  ((a)->s6_addr32[0] == 0 && (a)->s6_addr32[1] == 0 && \
   (a)->s6_addr32[2] == 0 && (a)->s6_addr32[3] == htonl(1))
#define IN6_IS_ADDR_MULTICAST(a) ((a)->s6_addr[0] == 0xff)
#define IN6_IS_ADDR_LINKLOCAL(a) \
  ((a)->s6_addr[0] == 0xfe && ((a)->s6_addr[1] & 0xc0) == 0x80)
#define IN6_IS_ADDR_SITELOCAL(a) \
  ((a)->s6_addr[0] == 0xfe && ((a)->s6_addr[1] & 0xc0) == 0xc0)
#define IN6_IS_ADDR_V4MAPPED(a) \
  ((a)->s6_addr32[0] == 0 && (a)->s6_addr32[1] == 0 && \
   (a)->s6_addr32[2] == htonl(0xffff))
#define IN6_IS_ADDR_V4COMPAT(a) \
  ((a)->s6_addr32[0] == 0 && (a)->s6_addr32[1] == 0 && \
   (a)->s6_addr32[2] == 0 && ntohl((a)->s6_addr32[3]) > 1)
#define IN6_IS_ADDR_MC_NODELOCAL(a) \
  (IN6_IS_ADDR_MULTICAST(a) && ((a)->s6_addr[1] & 0xf) == 0x1)
#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
  (IN6_IS_ADDR_MULTICAST(a) && ((a)->s6_addr[1] & 0xf) == 0x2)
#define IN6_IS_ADDR_MC_SITELOCAL(a) \
  (IN6_IS_ADDR_MULTICAST(a) && ((a)->s6_addr[1] & 0xf) == 0x5)
#define IN6_IS_ADDR_MC_ORGLOCAL(a) \
  (IN6_IS_ADDR_MULTICAST(a) && ((a)->s6_addr[1] & 0xf) == 0x8)
#define IN6_IS_ADDR_MC_GLOBAL(a) \
  (IN6_IS_ADDR_MULTICAST(a) && ((a)->s6_addr[1] & 0xf) == 0xe)

#endif /* _NETINET_IN_H_ */
