#ifndef _SYS_IPC_H
#define _SYS_IPC_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ipc_perm {
  uid_t uid;
  gid_t gid;
  uid_t cuid;
  gid_t cgid;
  mode_t mode;
};

/* Values match Linux musl sys/ipc.h:
 * https://git.musl-libc.org/cgit/musl/tree/include/sys/ipc.h
 */
#define IPC_CREAT   01000
#define IPC_EXCL    02000
#define IPC_NOWAIT  04000
#define IPC_PRIVATE ((key_t)0)

/* Values match Linux UAPI ipc.h:
 * https://github.com/torvalds/linux/blob/master/include/uapi/linux/ipc.h
 */
#define IPC_RMID    0
#define IPC_SET     1
#define IPC_STAT    2

key_t ftok(const char *, int);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_IPC_H */
