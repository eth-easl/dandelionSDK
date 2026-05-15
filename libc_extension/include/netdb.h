#ifndef	_NETDB_H
#define	_NETDB_H 1

#include <sys/types.h>
#include <inttypes.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Description of data base entry for a single host.  */
struct hostent
{
  char *h_name;			/* Official name of host.  */
  char **h_aliases;		/* Alias list.  */
  int h_addrtype;		/* Host address type.  */
  int h_length;			/* Length of address.  */
  char **h_addr_list;		/* List of addresses from name server.  */
};

/* Description of data base entry for a single network.  NOTE: here a
   poor assumption is made.  The network number is expected to fit
   into an unsigned long int variable.  */
struct netent
{
  char *n_name;			/* Official name of network.  */
  char **n_aliases;		/* Alias list.  */
  int n_addrtype;		/* Net address type.  */
  uint32_t n_net;		/* Network number.  */
};

/* Description of data base entry for a single service.  */
struct protoent
{
  char *p_name;			/* Official protocol name.  */
  char **p_aliases;		/* Alias list.  */
  int p_proto;			/* Protocol number.  */
};

/* Description of data base entry for a single service.  */
struct servent
{
  char *s_name;			/* Official service name.  */
  char **s_aliases;		/* Alias list.  */
  int s_port;			/* Port number.  */
  char *s_proto;		/* Protocol to use.  */
};

# define IPPORT_RESERVED 1024

extern int h_errno;

# define HOST_NOT_FOUND	1	/* Authoritative Answer Host not found.  */
# define TRY_AGAIN	    2	/* Non-Authoritative Host not found, or SERVERFAIL.  */
# define NO_RECOVERY	3	/* Non recoverable errors, FORMERR, REFUSED, NOTIMP.  */
# define NO_DATA	    4	/* Valid name, no data record of requested type.  */

struct addrinfo
{
  int ai_flags;
  int ai_family;
  int ai_socktype;
  int ai_protocol;
  socklen_t ai_addrlen;
  struct sockaddr *ai_addr;
  char *ai_canonname;
  struct addrinfo *ai_next;
};

/* https://sources.debian.org/src/glibc/2.43-1/resolv/netdb.h */
# define AI_PASSIVE	    0x0001
# define AI_CANONNAME	    0x0002
# define AI_NUMERICHOST	    0x0004
# define AI_V4MAPPED	    0x0008
# define AI_ALL		    0x0010
# define AI_ADDRCONFIG	    0x0020
# define AI_NUMERICSERV	    0x0400

/* https://git.musl-libc.org/cgit/musl/tree/include/netdb.h */
 # define NI_NUMERICHOST	    0x0001
 # define NI_NUMERICSERV	    0x0002
# define NI_NOFQDN	    0x0004
# define NI_NAMEREQD	    0x0008
# define NI_DGRAM	    0x0010
# define NI_NUMERICSCOPE    0x000000100

/* https://sources.debian.org/src/glibc/2.43-1/resolv/netdb.h */
# define EAI_BADFLAGS	    -1
# define EAI_NONAME	    -2
# define EAI_AGAIN	    -3
# define EAI_FAIL	    -4
# define EAI_FAMILY	    -6
# define EAI_SOCKTYPE	    -7
# define EAI_SERVICE	    -8
# define EAI_MEMORY	    -10
# define EAI_SYSTEM	    -11
# define EAI_OVERFLOW	    -12

extern void endhostent (void);
extern void endnetent (void);
extern void endprotoent (void);
extern void endservent (void);
extern struct hostent *gethostbyaddr (const void *__addr, __socklen_t __len, int __type);
extern struct hostent *gethostbyname (const char *__name);
extern struct hostent *gethostent (void);
extern struct netent *getnetbyaddr (uint32_t __net, int __type);
extern struct netent *getnetbyname (const char *__name);
extern struct netent *getnetent (void);
extern struct protoent *getprotobyname (const char *__name);
extern struct protoent *getprotobynumber (int __proto);
extern struct protoent *getprotoent (void);
extern struct servent *getservbyname (const char *__name, const char *__proto);
extern struct servent *getservbyport (int __port, const char *__proto);
extern struct servent *getservent (void);
extern void sethostent (int __stay_open);
extern void setnetent (int __stay_open);
extern void setprotoent (int __stay_open);
extern void setservent (int __stay_open);
extern void freeaddrinfo (struct addrinfo *__ai);
extern int getaddrinfo (const char *__restrict __name,
                        const char *__restrict __service,
                        const struct addrinfo *__restrict __req,
                        struct addrinfo **__restrict __pai);
extern int getnameinfo (const struct sockaddr *__restrict __sa,
                        socklen_t __salen,
                        char *__restrict __host, socklen_t __hostlen,
                        char *__restrict __serv, socklen_t __servlen,
                        int __flags);
extern const char *gai_strerror (int __ecode);

#ifdef __cplusplus
}
#endif

#endif // _NETDB_H
