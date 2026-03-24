/* THIS IS A DUMMY HEADER */

#ifndef _ARPA_INET_H
#define	_ARPA_INET_H	1

#include <netinet/in.h>		/* To define `struct in_addr'.  */

extern in_addr_t inet_addr (const char *__cp);
extern char *inet_ntoa (struct in_addr __in);
extern const char *inet_ntop (int __af, const void * __cp, char * __buf, socklen_t __len);
extern int inet_pton (int __af, const char * __cp, void * __buf);

#endif // _ARPA_INET_H
