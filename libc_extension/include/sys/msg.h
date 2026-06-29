#ifndef _SYS_MSG_H
#define _SYS_MSG_H

#include <sys/ipc.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* https://github.com/torvalds/linux/blob/master/include/uapi/linux/msg.h */
#define MSG_NOERROR 010000

typedef unsigned long msgqnum_t;
typedef unsigned long msglen_t;

struct msqid_ds {
  struct ipc_perm msg_perm;
  msgqnum_t msg_qnum;
  msglen_t msg_qbytes;
  pid_t msg_lspid;
  pid_t msg_lrpid;
  time_t msg_stime;
  time_t msg_rtime;
  time_t msg_ctime;
};

int msgctl(int, int, struct msqid_ds *);
int msgget(key_t, int);
ssize_t msgrcv(int, void *, size_t, long, int);
int msgsnd(int, const void *, size_t, int);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_MSG_H */
