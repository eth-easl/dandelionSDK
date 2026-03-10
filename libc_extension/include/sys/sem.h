#ifndef _SYS_SEM_H
#define _SYS_SEM_H

#include <sys/ipc.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Values match Linux UAPI sem.h:
 * https://github.com/torvalds/linux/blob/master/include/uapi/linux/sem.h
 */
#define SEM_UNDO  0x1000
#define GETNCNT   14
#define GETPID    11
#define GETVAL    12
#define GETALL    13
#define GETZCNT   15
#define SETVAL    16
#define SETALL    17

struct semid_ds {
  struct ipc_perm sem_perm;
  time_t sem_otime;
  time_t sem_ctime;
  unsigned short sem_nsems;
};

struct sembuf {
  unsigned short sem_num;
  short sem_op;
  short sem_flg;
};

int semctl(int, int, int, ...);
int semget(key_t, int, int);
int semop(int, struct sembuf *, size_t);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SEM_H */
