#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* https://git.musl-libc.org/cgit/musl/tree/include/sys/mman.h */
#define PROT_NONE 0x0
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4

/* https://git.musl-libc.org/cgit/musl/tree/include/sys/mman.h */
#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_ANON 0x20
#define MAP_ANONYMOUS MAP_ANON

/* https://git.musl-libc.org/cgit/musl/tree/include/sys/mman.h */
#define MAP_FAILED ((void *)-1)

/* https://git.musl-libc.org/cgit/musl/tree/include/sys/mman.h */
#define MS_ASYNC 1
#define MS_INVALIDATE 2
#define MS_SYNC 4

/* https://git.musl-libc.org/cgit/musl/tree/include/sys/mman.h */
#define MCL_CURRENT 1
#define MCL_FUTURE 2

/* https://git.musl-libc.org/cgit/musl/tree/include/sys/mman.h */
#define POSIX_MADV_NORMAL 0
#define POSIX_MADV_RANDOM 1
#define POSIX_MADV_SEQUENTIAL 2
#define POSIX_MADV_WILLNEED 3
#define POSIX_MADV_DONTNEED 4

void *mmap(void *, size_t, int, int, int, off_t);
int mprotect(void *, size_t, int);
int msync(void *, size_t, int);
int munmap(void *, size_t);
int mlock(const void *, size_t);
int mlockall(int);
int munlock(const void *, size_t);
int munlockall(void);
int posix_madvise(void *, size_t, int);
int shm_open(const char *, int, mode_t);
int shm_unlink(const char *);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_MMAN_H */
