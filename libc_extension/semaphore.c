#include <errno.h>
#include <semaphore.h>
#include <stdarg.h>

int sem_close(sem_t *sem) {
  (void)sem;
  errno = ENOSYS;
  return -1;
}

int sem_destroy(sem_t *sem) {
  (void)sem;
  errno = ENOSYS;
  return -1;
}

int sem_getvalue(sem_t *restrict sem, int *restrict sval) {
  (void)sem;
  (void)sval;
  errno = ENOSYS;
  return -1;
}

int sem_init(sem_t *sem, int pshared, unsigned value) {
  (void)sem;
  (void)pshared;
  (void)value;
  errno = ENOSYS;
  return -1;
}

sem_t *sem_open(const char *name, int oflag, ...) {
  va_list ap;

  (void)name;
  (void)oflag;
  va_start(ap, oflag);
  va_end(ap);
  errno = ENOSYS;
  return SEM_FAILED;
}

int sem_post(sem_t *sem) {
  (void)sem;
  errno = ENOSYS;
  return -1;
}

int sem_timedwait(sem_t *restrict sem,
                  const struct timespec *restrict abs_timeout) {
  (void)sem;
  (void)abs_timeout;
  errno = ENOSYS;
  return -1;
}

int sem_trywait(sem_t *sem) {
  (void)sem;
  errno = ENOSYS;
  return -1;
}

int sem_unlink(const char *name) {
  (void)name;
  errno = ENOSYS;
  return -1;
}

int sem_wait(sem_t *sem) {
  (void)sem;
  errno = ENOSYS;
  return -1;
}
