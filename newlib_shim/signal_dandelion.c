#include <errno.h>
#include <signal.h>
#include <string.h>

#undef sigaddset
#undef sigemptyset
#undef sigismember

void psiginfo(const siginfo_t *info, const char *message) {
  (void)info;
  (void)message;
}

int sighold(int sig) {
  (void)sig;
  *__errno() = ENOSYS;
  return -1;
}

int sigignore(int sig) {
  (void)sig;
  *__errno() = ENOSYS;
  return -1;
}

int siginterrupt(int sig, int flag) {
  (void)sig;
  (void)flag;
  *__errno() = ENOSYS;
  return -1;
}

int sigrelse(int sig) {
  (void)sig;
  *__errno() = ENOSYS;
  return -1;
}

_sig_func_ptr sigset(int sig, _sig_func_ptr disp) {
  (void)sig;
  (void)disp;
  *__errno() = ENOSYS;
  return SIG_ERR;
}

int pthread_sigmask(int how, const sigset_t *restrict set,
                    sigset_t *restrict oldset) {
  (void)how;
  (void)set;
  *__errno() = ENOSYS;
  return -1;
}

int sigaddset(sigset_t *set, int signo) {
  (void)set;
  (void)signo;
  *__errno() = ENOSYS;
  return -1;
}

int sigemptyset(sigset_t *set) {
  (void)set;
  *__errno() = ENOSYS;
  return -1;
}

int sigismember(const sigset_t *set, int signo) {
  (void)set;
  (void)signo;
  *__errno() = ENOSYS;
  return -1;
}

int sigprocmask(int how, const sigset_t *restrict set,
                sigset_t *restrict oldset) {
  (void)how;
  (void)set;
  *__errno() = ENOSYS;
  return -1;
}

int sigaltstack(const stack_t *restrict ss, stack_t *restrict old_ss) {
  (void)ss;
  (void)old_ss;
  *__errno() = ENOSYS;
  return -1;
}
