#include <dlfcn.h>

int dlclose (void *__handle) { return -1; }
char *dlerror (void) { return 0; }
void *dlopen (const char *__file, int __mode) { return 0; }
void *dlsym (void *__restrict __handle, const char *__restrict __name) { return 0; }
