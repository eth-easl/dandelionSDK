#define __rtems__
#include <pthread.h>

#include <errno.h>
#undef errno
extern int errno;

int pthread_mutexattr_init(pthread_mutexattr_t *attr){ return EINVAL; }
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr){ return EINVAL; }
int pthread_mutexattr_gettype(
        const pthread_mutexattr_t *restrict attr,
        int *restrict type){
    return EINVAL;
}
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type){ return EINVAL;}

int pthread_mutex_init(
        pthread_mutex_t *restrict mutex,
        const pthread_mutexattr_t *restrict attr){
    return EPERM;
}
int pthread_mutex_destroy(pthread_mutex_t *mutex){ return EPERM; }
int pthread_mutex_trylock(pthread_mutex_t *mutex){ return EINVAL; }
int pthread_mutex_lock(pthread_mutex_t *mutex){ return EINVAL; }
int pthread_mutex_unlock(pthread_mutex_t *mutex){ return EPERM; }

int pthread_rwlock_init(
        pthread_rwlock_t *restrict rwlock,
        const pthread_rwlockattr_t *restrict attr){
    return EPERM;
}
int pthread_rwlock_destry(pthread_rwlock_t *rwlock){ return EINVAL; }
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock){ return EINVAL; }
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock){ return EINVAL; }
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock){ return EINVAL; }
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock){ return EINVAL; }
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock){ return EINVAL; }

int pthread_cond_init(
        pthread_cond_t *restrict cond,
        const pthread_condattr_t *restrict attr) {
    return EAGAIN;
}
int pthread_cond_destroy(pthread_cond_t *cond) { return EINVAL;}
int pthread_cond_wait(
        pthread_cond_t *restrict cond,
        pthread_mutex_t *restrict mutex) {
    return ENOTRECOVERABLE;
}
int pthread_cond_timedwait(
        pthread_cond_t *restrict cond,
        pthread_mutex_t *restrict mutex, 
        const struct timespec *restrict abstim) {
    return ENOTRECOVERABLE;
}
int pthread_cond_signal(pthread_cond_t *cond) { return EINVAL; }
int pthread_cond_broadcast(pthread_cond_t *cond) { return EINVAL; }

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)){
    return EPERM;
}

pthread_t pthread_self(void) { return 0;}
int pthread_join(pthread_t thread, void **retval){ return EINVAL; }
int pthread_detach(pthread_t thread) { return EINVAL; }

int pthread_key_create(pthread_key_t *key, void (*destructor)(void*)) { return EAGAIN; }
void* pthread_getspecific(pthread_key_t key) { return NULL; }
int pthread_setspecific(pthread_key_t key, const void* value) { return EINVAL; }