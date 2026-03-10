#include <errno.h>
#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int shmflg) {
  (void)shmid;
  (void)shmaddr;
  (void)shmflg;
  errno = ENOSYS;
  return (void *)-1;
}

int shmctl(int shmid, int cmd, struct shmid_ds *buf) {
  (void)shmid;
  (void)cmd;
  (void)buf;
  errno = ENOSYS;
  return -1;
}

int shmdt(const void *shmaddr) {
  (void)shmaddr;
  errno = ENOSYS;
  return -1;
}

int shmget(key_t key, size_t size, int shmflg) {
  (void)key;
  (void)size;
  (void)shmflg;
  errno = ENOSYS;
  return -1;
}
