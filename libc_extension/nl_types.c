#include <errno.h>
#include <nl_types.h>

int catclose(nl_catd catd) {
  (void)catd;
  errno = ENOSYS;
  return -1;
}

char *catgets(nl_catd catd, int set_id, int msg_id, const char *s) {
  (void)catd;
  (void)set_id;
  (void)msg_id;
  errno = ENOSYS;
  return (char *)s;
}

nl_catd catopen(const char *name, int oflag) {
  (void)name;
  (void)oflag;
  errno = ENOSYS;
  return (nl_catd)-1;
}
