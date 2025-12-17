#include <netdb.h>

void endhostent (void) {}

void endnetent (void) {}

void endprotoent (void) {}

void endservent (void) {}

struct hostent *gethostbyaddr (const void *__addr, __socklen_t __len, int __type) {
    return 0;
}

struct hostent *gethostbyname (const char *__name) {
    return 0;
}

struct hostent *gethostent (void) {
    return 0;
}

struct netent *getnetbyaddr (uint32_t __net, int __type) {
    return 0;
}

struct netent *getnetbyname (const char *__name) {
    return 0;
}

struct netent *getnetent (void) {
    return 0;
}

struct protoent *getprotobyname (const char *__name) {
    return 0;
}

struct protoent *getprotobynumber (int __proto) {
    return 0;
}

struct protoent *getprotoent (void) {
    return 0;
}

struct servent *getservbyname (const char *__name, const char *__proto) {
    return 0;
}

struct servent *getservbyport (int __port, const char *__proto) {
    return 0;
}

struct servent *getservent (void) {
    return 0;
}

void sethostent (int __stay_open) {}

void setnetent (int __stay_open) {}

void setprotoent (int __stay_open) {}

void setservent (int __stay_open) {}

