#include <errno.h>

long syscall(long number, ...) {
  (void)number;
  errno = ENOSYS;
  return -1;
}
