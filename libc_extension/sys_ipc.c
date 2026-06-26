#include <errno.h>
#include <sys/ipc.h>

key_t ftok(const char *path, int id) {
  (void)path;
  (void)id;
  errno = ENOSYS;
  return (key_t)-1;
}
