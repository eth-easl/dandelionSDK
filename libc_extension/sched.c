#include <sched.h>
#include <errno.h>

int sched_yield (void) {
  errno = ENOSYS;
  return -1;
}

int sched_getcpu(void) {
  errno = ENOTSUP;
  return -1;
}
