#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#include <sys/ipc.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long shmatt_t;

/* Values match Linux UAPI shm.h:
 * https://github.com/torvalds/linux/blob/master/include/uapi/linux/shm.h
 */
#define SHM_RDONLY  010000
#define SHM_RND     020000

/* Linux AMD64 defines SHMLBA as __getpagesize():
 * https://refspecs.linuxfoundation.org/LSB_2.0.1/LSB-Core-AMD64/LSB-Core-AMD64.diff.pdf
 * Dandelion SDK sets PAGE_SIZE to 4096 in the top-level CMake defaults.
 * TODO: __getpagesize() should be used instead.
 */
#define SHMLBA 4096

struct shmid_ds {
  struct ipc_perm shm_perm;
  size_t shm_segsz;
  pid_t shm_lpid;
  pid_t shm_cpid;
  shmatt_t shm_nattch;
  time_t shm_atime;
  time_t shm_dtime;
  time_t shm_ctime;
};

void *shmat(int, const void *, int);
int shmctl(int, int, struct shmid_ds *);
int shmdt(const void *);
int shmget(key_t, size_t, int);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SHM_H */
