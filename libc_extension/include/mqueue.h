#ifndef _MQUEUE_H
#define _MQUEUE_H

#include <signal.h>
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* https://man7.org/linux/man-pages/man0/mqueue.h.0p.html */

typedef int mqd_t;

struct mq_attr {
  long mq_flags;
  long mq_maxmsg;
  long mq_msgsize;
  long mq_curmsgs;
};

int mq_close(mqd_t);
int mq_getattr(mqd_t, struct mq_attr *);
int mq_notify(mqd_t, const struct sigevent *);
mqd_t mq_open(const char *, int, ...);
ssize_t mq_receive(mqd_t, char *, size_t, unsigned *);
int mq_send(mqd_t, const char *, size_t, unsigned);
int mq_setattr(mqd_t, const struct mq_attr *__restrict,
               struct mq_attr *__restrict);
ssize_t mq_timedreceive(mqd_t, char *__restrict, size_t, unsigned *__restrict,
                        const struct timespec *__restrict);
int mq_timedsend(mqd_t, const char *, size_t, unsigned,
                 const struct timespec *);
int mq_unlink(const char *);

#ifdef __cplusplus
}
#endif

#endif
