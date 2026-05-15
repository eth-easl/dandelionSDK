#ifndef _SYS_UN_H
#define _SYS_UN_H

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr_un {
  sa_family_t sun_family;
  /* https://man7.org/linux/man-pages/man7/unix.7.html */
  char sun_path[108];
};

#ifdef __cplusplus
}
#endif

#endif /* _SYS_UN_H */
