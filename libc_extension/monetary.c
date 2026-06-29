#include <errno.h>
#include <monetary.h>
// https://man7.org/linux/man-pages/man0/monetary.h.0p.html
ssize_t strfmon(char *restrict s, size_t max, const char *restrict format,
                ...) {
  (void)s;
  (void)max;
  (void)format;
  errno = ENOSYS;
  return -1;
}

ssize_t strfmon_l(char *restrict s, size_t max, locale_t loc,
                  const char *restrict format, ...) {
  (void)s;
  (void)max;
  (void)loc;
  (void)format;
  errno = ENOSYS;
  return -1;
}
