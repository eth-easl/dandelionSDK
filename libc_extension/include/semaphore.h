/* THIS IS A DUMMY HEADER */

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H	1

#include <time.h>

#if __WORDSIZE == 64
# define __SIZEOF_SEM_T	32
#else
# define __SIZEOF_SEM_T	16
#endif

typedef union {
  char __size[__SIZEOF_SEM_T];
  long int __align;
} sem_t;

#define SEM_FAILED      ((sem_t *) 0)

extern int sem_close (sem_t *__sem);
extern int sem_destroy (sem_t *__sem);
extern int sem_getvalue (sem_t *restrict __sem, int *restrict __sval);
extern int sem_init (sem_t *__sem, int __pshared, unsigned int __value);
extern sem_t *sem_open (const char *__name, int __oflag, ...);
extern int sem_post (sem_t *__sem);
extern int sem_timedwait (sem_t *restrict __sem, const struct timespec *restrict __abstime);
extern int sem_trywait (sem_t *__sem);
extern int sem_unlink (const char *__name);
extern int sem_wait (sem_t *__sem);

#endif	/* semaphore.h */
