/* THIS IS A DUMMY HEADER */

#ifndef	_SYS_MMAN_H
#define	_SYS_MMAN_H	1

#include <sys/types.h>

#define PROT_READ	0x1
#define PROT_WRITE	0x2
#define PROT_EXEC	0x4
#define PROT_NONE	0x0

#define MAP_SHARED	0x01
#define MAP_PRIVATE	0x02
#define MAP_FIXED	0x10

#define MS_ASYNC	1
#define MS_SYNC		4
#define MS_INVALIDATE	2

#define MCL_CURRENT	1
#define MCL_FUTURE	2

#define MAP_FAILED	((void *) -1)

extern int mlock (const void *__addr, size_t __len);
extern int mlockall (int __flags);
extern void *mmap (void *__addr, size_t __len, int __prot, int __flags, int __fd, __off_t __offset);
extern int mprotect (void *__addr, size_t __len, int __prot);
extern int msync (void *__addr, size_t __len, int __flags);
extern int munlock (const void *__addr, size_t __len);
extern int munlockall (void);
extern int munmap (void *__addr, size_t __len);
extern int shm_open (const char *__name, int __oflag, mode_t __mode);
extern int shm_unlink (const char *__name);

#endif	/* sys/mman.h */
