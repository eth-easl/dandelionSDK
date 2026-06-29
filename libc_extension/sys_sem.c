#include <errno.h>
#include <stdarg.h>
#include <sys/sem.h>

int semctl(int semid, int semnum, int cmd, ...) {
  va_list ap;

  (void)semid;
  (void)semnum;
  (void)cmd;
  va_start(ap, cmd);
  va_end(ap);
  errno = ENOSYS;
  return -1;
}

int semget(key_t key, int nsems, int semflg) {
  (void)key;
  (void)nsems;
  (void)semflg;
  errno = ENOSYS;
  return -1;
}

int semop(int semid, struct sembuf *sops, size_t nsops) {
  (void)semid;
  (void)sops;
  (void)nsops;
  errno = ENOSYS;
  return -1;
}
