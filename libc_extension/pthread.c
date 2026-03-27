#include <pthread.h>
#include <errno.h>

int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock) {
  (void)__rwlock;
  errno = ENOSYS;
  return -1;
}
