#include <errno.h>
#include <utmpx.h>

void endutxent(void) { errno = ENOSYS; }

struct utmpx *getutxent(void) {
  errno = ENOSYS;
  return 0;
}

struct utmpx *getutxid(const struct utmpx *id) {
  (void)id;
  errno = ENOSYS;
  return 0;
}

struct utmpx *getutxline(const struct utmpx *line) {
  (void)line;
  errno = ENOSYS;
  return 0;
}

struct utmpx *pututxline(const struct utmpx *utmpx) {
  (void)utmpx;
  errno = ENOSYS;
  return 0;
}

void setutxent(void) { errno = ENOSYS; }
