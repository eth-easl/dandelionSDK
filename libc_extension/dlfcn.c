#include <dlfcn.h>
#include <errno.h>

int dlclose(void *handle) {
  (void)handle;
  errno = ENOSYS;
  return -1;
}

char *dlerror(void) {
  errno = ENOSYS;
  return "dynamic loading is not supported";
}

void *dlopen(const char *file, int mode) {
  (void)file;
  (void)mode;
  errno = ENOSYS;
  return NULL;
}

void *dlsym(void *restrict handle, const char *restrict name) {
  (void)handle;
  (void)name;
  errno = ENOSYS;
  return NULL;
}
