#ifndef _SYS_FCNTL_H_
#define _SYS_FCNTL_H_

#include <sys/_default_fcntl.h>

/* https://git.musl-libc.org/cgit/musl/tree/include/fcntl.h */
#ifndef O_TTY_INIT
#define O_TTY_INIT 0
#endif
/* https://www.gnu.org/s/gnulib/manual/html_node/fcntl_002eh.html */
#ifndef O_DSYNC
#define O_DSYNC 0
#endif
#ifndef O_RSYNC
#define O_RSYNC 0
#endif
/* https://github.com/bminor/glibc/blob/master/sysdeps/unix/sysv/linux/bits/fcntl-linux.h */
#ifndef POSIX_FADV_NORMAL
#define POSIX_FADV_NORMAL 0
#define POSIX_FADV_RANDOM 1
#define POSIX_FADV_SEQUENTIAL 2
#define POSIX_FADV_WILLNEED 3
#define POSIX_FADV_DONTNEED 4
#define POSIX_FADV_NOREUSE 5
#endif
/* https://github.com/bminor/glibc/blob/master/io/fcntl.h */
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

__BEGIN_DECLS
int posix_fadvise (int, off_t, off_t, int);
int posix_fallocate (int, off_t, off_t);
__END_DECLS

#endif /* _SYS_FCNTL_H_ */
