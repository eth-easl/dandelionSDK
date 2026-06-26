#include <errno.h>
#include <sys/msg.h>

int msgctl(int msqid, int cmd, struct msqid_ds *buf) {
  (void)msqid;
  (void)cmd;
  (void)buf;
  errno = ENOSYS;
  return -1;
}

int msgget(key_t key, int msgflg) {
  (void)key;
  (void)msgflg;
  errno = ENOSYS;
  return -1;
}

ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg) {
  (void)msqid;
  (void)msgp;
  (void)msgsz;
  (void)msgtyp;
  (void)msgflg;
  errno = ENOSYS;
  return -1;
}

int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
  (void)msqid;
  (void)msgp;
  (void)msgsz;
  (void)msgflg;
  errno = ENOSYS;
  return -1;
}
