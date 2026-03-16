#include <errno.h>
#include <signal.h>

#undef errno
extern int errno;

void psiginfo(const siginfo_t *info, const char *message) {
  (void)info;
  (void)message;
}

int sighold(int sig) {
  (void)sig;
  errno = ENOSYS;
  return -1;
}

int sigignore(int sig) {
  (void)sig;
  errno = ENOSYS;
  return -1;
}

int siginterrupt(int sig, int flag) {
  (void)sig;
  (void)flag;
  errno = ENOSYS;
  return -1;
}

int sigrelse(int sig) {
  (void)sig;
  errno = ENOSYS;
  return -1;
}

int raise(int sig) {
  (void)sig;
  errno = ENOSYS;
  return -1;
}

_sig_func_ptr sigset(int sig, _sig_func_ptr disp) {
  (void)sig;
  (void)disp;
  errno = ENOSYS;
  return SIG_ERR;
}
