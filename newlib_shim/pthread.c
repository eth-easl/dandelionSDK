#define __rtems__
#include <pthread.h>
#include <malloc.h>
#include <errno.h>
#undef errno
extern int errno;

// pthread_mutex_t is defined as a uint32, and the default initializer is 0xfff..
// because that is defined elsewhere, we use setting flags to 0 to turn them on
#define LOCKED      0x01
#define RECURSIVE   0x02
#define DESTROYED   0x80

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
    // initialize mutex
    pthread_mutex_t initial = _PTHREAD_MUTEX_INITIALIZER;
    if(attr != NULL && attr->recursive != 0){
        initial &= ~RECURSIVE;
    }
    *mutex = initial;
    return 0;
}
int pthread_mutex_destroy(pthread_mutex_t *mutex){ 
    // check is valid mutex index
    pthread_mutex_t current = *mutex;
    if((current & DESTROYED) == 0)
        return EINVAL;
    // fail destroying locked mutex
    if((current & LOCKED) == 0)
        return EBUSY;
    *mutex &= ~DESTROYED;
    return 0;
}
int pthread_mutex_trylock(pthread_mutex_t *mutex){ 
    pthread_mutex_t current = *mutex;
    // check it is a valid mutex
    if((current & DESTROYED) == 0)
        return EINVAL;
    // check it is locked and not recursive, to give deadlock error, since we only have 1 thread
    if((current & LOCKED) == 0 && (current & RECURSIVE) != 0)
        return EDEADLK;
    // is either not locked, so can just lock it, or is recursive so can relock it
    *mutex &= ~LOCKED;
    return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mutex){
    // since there is only 1 thread, there is no difference
    return pthread_mutex_trylock(mutex);
}
int pthread_mutex_unlock(pthread_mutex_t *mutex){ 
    pthread_mutex_t current = *mutex;
    // check is a valid mutex
    if((current & DESTROYED) == 0)
        return EINVAL;
    // fail if the lock is not locked, meaning the current (and only) thread does not hold the lock
    if((current & LOCKED) != 0)
        return EPERM;
    *mutex |= LOCKED;
    return 0;
}

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
    if(once_control->init_executed == 0){
        init_routine();
        once_control->init_executed = 1;
    }
    return 0;
}

pthread_t pthread_self(void) { return 0;}
int pthread_create(
        pthread_t *restrict thread,
        const pthread_attr_t *restrict attr,
        void *(*start_routine)(void*),
        void *restrict arg){
    return EPERM;
}
int pthread_join(pthread_t thread, void **retval){ return EINVAL; }
int pthread_detach(pthread_t thread) { return EINVAL; }

// TODO replace with heap
typedef struct thread_key_struct {
    pthread_key_t key;
    struct thread_key_struct* next;
    const void* content;
} thread_key_t;

static thread_key_t* key_root = NULL;

// currently ignore destructors, since we don't have a clean up step
// TODO once cleanup step is implemented add proper destructor support
int pthread_key_create(pthread_key_t *key, void (*destructor)(void*)) {
    thread_key_t* new_node = malloc(sizeof(thread_key_t));  
    new_node->next = NULL;
    new_node->content = NULL;

    if(key_root == NULL){
        new_node->key = 0;
        key_root = new_node;
    } else {
        thread_key_t* current = key_root;
        while(current->next != NULL){
            if(current->next->key != current->key + 1)
                break;
            current = current->next;
        }
        new_node->key = current->key + 1;
        new_node->next = current->next;
        current->next = new_node; 
    }

    *key = new_node->key;
    return 0;
}
void* pthread_getspecific(pthread_key_t key) { 
    thread_key_t* current = key_root;
    while(current != NULL){
        if(current->key == key)
            return current->content;
        current = current->next;
    }
    return NULL;
}
int pthread_setspecific(pthread_key_t key, const void* value) { 
    thread_key_t* current = key_root;
    while(current != NULL){
        if(current->key == key){
            current->content = value;
            return 0;
        }   
        current = current->next;
    }
    return EINVAL;
}
int	pthread_key_delete (pthread_key_t key){
    thread_key_t* current = key_root;
    if(current == NULL)
        return EINVAL;
    if(current->key == key){
        key_root = key_root->next;
        return 0;
    }
    while(current->next != NULL && current->next->key != key){
        current = current->next;
    }
    // either are at end of list of at node where current next has the key
    if(current->next == NULL)
        return EINVAL;
    // current next has the key
    thread_key_t* to_destroy = current->next;
    current->next = to_destroy->next;
    free(to_destroy);
    return 0;
}

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)){
    return ENOMEM;
}