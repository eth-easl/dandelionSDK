#ifndef	_NETDB_H
#define	_NETDB_H 1

#include <sys/types.h>
#include <inttypes.h>

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

#endif // _NETDB_H