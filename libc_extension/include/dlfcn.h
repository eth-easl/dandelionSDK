#ifndef _DLFCN_H
#define _DLFCN_H

#ifdef __cplusplus
extern "C" {
#endif

/* https://sources.debian.org/src/musl/1.1.5-2%2Bdeb8u1/include/dlfcn.h */
#define RTLD_LAZY 1
#define RTLD_NOW 2
#define RTLD_GLOBAL 256
#define RTLD_LOCAL 0

int dlclose(void *);
char *dlerror(void);
void *dlopen(const char *, int);
void *dlsym(void *__restrict, const char *__restrict);

#ifdef __cplusplus
}
#endif

#endif
