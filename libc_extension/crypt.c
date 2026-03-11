#include <errno.h>
#include <crypt.h>

char *crypt(const char *phrase, const char *setting) {
  (void)phrase;
  (void)setting;
  errno = ENOSYS;
  return 0;
}
