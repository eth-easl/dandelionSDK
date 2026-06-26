#include <errno.h>
#include <search.h>

#undef errno
extern int errno;

void insque(void *elem, void *prev) {
  (void)elem;
  (void)prev;
  errno = ENOSYS;
}

void *lfind(const void *key, const void *base, size_t *nelp, size_t width,
            int (*compar)(const void *, const void *)) {
  (void)key;
  (void)base;
  (void)nelp;
  (void)width;
  (void)compar;
  errno = ENOSYS;
  return NULL;
}

void *lsearch(const void *key, void *base, size_t *nelp, size_t width,
              int (*compar)(const void *, const void *)) {
  (void)key;
  (void)base;
  (void)nelp;
  (void)width;
  (void)compar;
  errno = ENOSYS;
  return NULL;
}

void remque(void *elem) {
  (void)elem;
  errno = ENOSYS;
}
