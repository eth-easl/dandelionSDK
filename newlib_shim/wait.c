#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#undef errno
extern int errno;

int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
  (void)idtype;
  (void)id;
  (void)infop;
  (void)options;
  errno = ENOSYS;
  return -1;
}
