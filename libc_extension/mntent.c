#include <errno.h>
#include <mntent.h>

FILE *setmntent(const char *file, const char *mode) {
  (void)file;
  (void)mode;
  errno = ENOSYS;
  return 0;
}

struct mntent *getmntent(FILE *stream) {
  (void)stream;
  errno = ENOSYS;
  return 0;
}

struct mntent *getmntent_r(FILE *stream, struct mntent *result, char *buffer,
                           int bufsize) {
  (void)stream;
  (void)result;
  (void)buffer;
  (void)bufsize;
  errno = ENOSYS;
  return 0;
}

int addmntent(FILE *stream, const struct mntent *mnt) {
  (void)stream;
  (void)mnt;
  errno = ENOSYS;
  return 1;
}

int endmntent(FILE *stream) {
  (void)stream;
  errno = ENOSYS;
  return 0;
}

char *hasmntopt(const struct mntent *mnt, const char *opt) {
  (void)mnt;
  (void)opt;
  errno = ENOSYS;
  return 0;
}
