#include <errno.h>
#include <syslog.h>

void closelog(void) {
  errno = ENOSYS;
}

void openlog(const char *ident, int option, int facility) {
  (void)ident;
  (void)option;
  (void)facility;
  errno = ENOSYS;
}

int setlogmask(int maskpri) {
  (void)maskpri;
  errno = ENOSYS;
  return 0;
}

void syslog(int priority, const char *format, ...) {
  (void)priority;
  (void)format;
  errno = ENOSYS;
}
