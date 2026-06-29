#include <errno.h>
#include <sys/uio.h>

ssize_t readv(int __fd, const struct iovec *__iovec, int __count) {
  (void)__fd;
  (void)__iovec;
  (void)__count;
  errno = ENOSYS;
  return -1;
}

ssize_t writev(int __fd, const struct iovec *__iovec, int __count) {
  (void)__fd;
  (void)__iovec;
  (void)__count;
  errno = ENOSYS;
  return -1;
}
