#include <errno.h>
#include <sys/utsname.h>

int uname(struct utsname *buf) {
  (void)buf;
  errno = ENOSYS;
  return -1;
}
