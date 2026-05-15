#ifndef _SYS_POLL_H
#define _SYS_POLL_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int nfds_t;

struct pollfd {
  int fd;
  short events;
  short revents;
};

/* https://sources.debian.org/src/glibc/2.43-1/sysdeps/unix/sysv/linux/bits/poll.h */
#define POLLIN 0x001
#define POLLPRI 0x002
#define POLLOUT 0x004
#define POLLERR 0x008
#define POLLHUP 0x010
#define POLLNVAL 0x020
#define POLLRDNORM 0x040
#define POLLRDBAND 0x080
#define POLLWRNORM 0x100
#define POLLWRBAND 0x200

int poll(struct pollfd[], nfds_t, int);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_POLL_H */
