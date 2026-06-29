#include <errno.h>
#include <netdb.h>

int h_errno;

void endhostent(void) {}

void endnetent(void) {}

void endprotoent(void) {}

void endservent(void) {}

struct hostent *gethostbyaddr(const void *__addr, __socklen_t __len,
                              int __type) {
  (void)__addr;
  (void)__len;
  (void)__type;
  errno = ENOSYS;
  return 0;
}

struct hostent *gethostbyname(const char *__name) {
  (void)__name;
  errno = ENOSYS;
  return 0;
}

struct hostent *gethostent(void) {
  errno = ENOSYS;
  return 0;
}

struct netent *getnetbyaddr(uint32_t __net, int __type) {
  (void)__net;
  (void)__type;
  errno = ENOSYS;
  return 0;
}

struct netent *getnetbyname(const char *__name) {
  (void)__name;
  errno = ENOSYS;
  return 0;
}

struct netent *getnetent(void) {
  errno = ENOSYS;
  return 0;
}

struct protoent *getprotobyname(const char *__name) {
  (void)__name;
  errno = ENOSYS;
  return 0;
}

struct protoent *getprotobynumber(int __proto) {
  (void)__proto;
  errno = ENOSYS;
  return 0;
}

struct protoent *getprotoent(void) {
  errno = ENOSYS;
  return 0;
}

struct servent *getservbyname(const char *__name, const char *__proto) {
  (void)__name;
  (void)__proto;
  errno = ENOSYS;
  return 0;
}

struct servent *getservbyport(int __port, const char *__proto) {
  (void)__port;
  (void)__proto;
  errno = ENOSYS;
  return 0;
}

struct servent *getservent(void) {
  errno = ENOSYS;
  return 0;
}

void sethostent(int __stay_open) {}

void setnetent(int __stay_open) {}

void setprotoent(int __stay_open) {}

void setservent(int __stay_open) {}

void freeaddrinfo(struct addrinfo *__ai) { (void)__ai; }

int getaddrinfo(const char *__restrict __name, const char *__restrict __service,
                const struct addrinfo *__restrict __req,
                struct addrinfo **__restrict __pai) {
  (void)__name;
  (void)__service;
  (void)__req;
  (void)__pai;
  errno = ENOSYS;
  return EAI_SYSTEM;
}

int getnameinfo(const struct sockaddr *__restrict __sa, socklen_t __salen,
                char *__restrict __host, socklen_t __hostlen,
                char *__restrict __serv, socklen_t __servlen, int __flags) {
  (void)__sa;
  (void)__salen;
  (void)__host;
  (void)__hostlen;
  (void)__serv;
  (void)__servlen;
  (void)__flags;
  errno = ENOSYS;
  return EAI_SYSTEM;
}

const char *gai_strerror(int __ecode) {
  (void)__ecode;
  return "address resolution is not supported";
}
