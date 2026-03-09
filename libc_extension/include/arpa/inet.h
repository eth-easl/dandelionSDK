#ifndef _ARPA_INET_H
#define _ARPA_INET_H

#include <stdint.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
  in_addr_t s_addr;
};

/* Source for constant values:
 * POSIX netinet/in.h: https://man7.org/linux/man-pages/man0/netinet_in.h.0p.html
 */
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

uint32_t htonl(uint32_t);
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);
uint16_t ntohs(uint16_t);

in_addr_t inet_addr(const char *);
char *inet_ntoa(struct in_addr);
const char *inet_ntop(int, const void *restrict, char *restrict, socklen_t);
int inet_pton(int, const char *restrict, void *restrict);

#ifdef __cplusplus
}
#endif

#endif
