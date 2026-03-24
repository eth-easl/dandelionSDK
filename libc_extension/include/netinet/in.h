/* THIS IS A DUMMY HEADER */

#ifndef	_NETINET_IN_H
#define	_NETINET_IN_H	1

#include <sys/socket.h>
#include <inttypes.h>

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
    in_addr_t s_addr;
};

struct sockaddr_in {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
};

struct in6_addr {
	uint8_t	s6_addr[16];
};

struct sockaddr_in6 {
    sa_family_t sin6_family;
    in_port_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
};

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

struct ipv6_mreq {
    struct in6_addr ipv6mr_multiaddr;
    unsigned int ipv6mr_interface;
};

enum {
    IPPROTO_IP = 0,
    IPPROTO_ICMP = 1,
    IPPROTO_TCP = 6,
    IPPROTO_UDP = 17,
    IPPROTO_IPV6 = 41,
    IPPROTO_RAW = 255,
};
#define IPPROTO_IP		IPPROTO_IP
#define IPPROTO_ICMP	IPPROTO_ICMP
#define IPPROTO_TCP		IPPROTO_TCP
#define IPPROTO_UDP		IPPROTO_UDP
#define IPPROTO_IPV6	IPPROTO_IPV6
#define IPPROTO_RAW		IPPROTO_RAW

#define	INADDR_ANY		((in_addr_t) 0x00000000)
#define	INADDR_BROADCAST	((in_addr_t) 0xffffffff)

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

# define IN6_IS_ADDR_UNSPECIFIED(a) \
  (__extension__							      \
   ({ const struct in6_addr *__a = (const struct in6_addr *) (a);	      \
      __a->__in6_u.__u6_addr32[0] == 0					      \
      && __a->__in6_u.__u6_addr32[1] == 0				      \
      && __a->__in6_u.__u6_addr32[2] == 0				      \
      && __a->__in6_u.__u6_addr32[3] == 0; }))

# define IN6_IS_ADDR_LOOPBACK(a) \
  (__extension__							      \
   ({ const struct in6_addr *__a = (const struct in6_addr *) (a);	      \
      __a->__in6_u.__u6_addr32[0] == 0					      \
      && __a->__in6_u.__u6_addr32[1] == 0				      \
      && __a->__in6_u.__u6_addr32[2] == 0				      \
      && __a->__in6_u.__u6_addr32[3] == htonl (1); }))

# define IN6_IS_ADDR_LINKLOCAL(a) \
  (__extension__							      \
   ({ const struct in6_addr *__a = (const struct in6_addr *) (a);	      \
      (__a->__in6_u.__u6_addr32[0] & htonl (0xffc00000)) == htonl (0xfe800000); }))

# define IN6_IS_ADDR_SITELOCAL(a) \
  (__extension__							      \
   ({ const struct in6_addr *__a = (const struct in6_addr *) (a);	      \
      (__a->__in6_u.__u6_addr32[0] & htonl (0xffc00000)) == htonl (0xfec00000); }))

# define IN6_IS_ADDR_V4MAPPED(a) \
  (__extension__							      \
   ({ const struct in6_addr *__a = (const struct in6_addr *) (a);	      \
      __a->__in6_u.__u6_addr32[0] == 0					      \
      && __a->__in6_u.__u6_addr32[1] == 0				      \
      && __a->__in6_u.__u6_addr32[2] == htonl (0xffff); }))

# define IN6_IS_ADDR_V4COMPAT(a) \
  (__extension__							      \
   ({ const struct in6_addr *__a = (const struct in6_addr *) (a);	      \
      __a->__in6_u.__u6_addr32[0] == 0					      \
      && __a->__in6_u.__u6_addr32[1] == 0				      \
      && __a->__in6_u.__u6_addr32[2] == 0				      \
      && ntohl (__a->__in6_u.__u6_addr32[3]) > 1; }))

#define IN6_IS_ADDR_MC_NODELOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((const uint8_t *) (a))[1] & 0xf) == 0x1))

#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((const uint8_t *) (a))[1] & 0xf) == 0x2))

#define IN6_IS_ADDR_MC_SITELOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((const uint8_t *) (a))[1] & 0xf) == 0x5))

#define IN6_IS_ADDR_MC_ORGLOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((const uint8_t *) (a))[1] & 0xf) == 0x8))

#define IN6_IS_ADDR_MC_GLOBAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((const uint8_t *) (a))[1] & 0xf) == 0xe))
    
#endif // _NETINET_IN_H