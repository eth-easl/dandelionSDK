#ifndef	_DLFCN_H
#define	_DLFCN_H 1

#define RTLD_LAZY	0x00001
#define RTLD_NOW	0x00002
#define RTLD_GLOBAL	0x00100
#define RTLD_LOCAL	0

extern int dlclose (void *__handle);
extern char *dlerror (void);
extern void *dlopen (const char *__file, int __mode);
extern void *dlsym (void *__restrict __handle, const char *__restrict __name);

#endif	/* dlfcn.h */
