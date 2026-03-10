#include <errno.h>
#include <mqueue.h>
#include <stdarg.h>

int mq_close(mqd_t mqdes) {
  (void)mqdes;
  errno = ENOSYS;
  return -1;
}

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat) {
  (void)mqdes;
  (void)mqstat;
  errno = ENOSYS;
  return -1;
}

int mq_notify(mqd_t mqdes, const struct sigevent *notification) {
  (void)mqdes;
  (void)notification;
  errno = ENOSYS;
  return -1;
}

mqd_t mq_open(const char *name, int oflag, ...) {
  va_list ap;

  (void)name;
  (void)oflag;
  va_start(ap, oflag);
  va_end(ap);
  errno = ENOSYS;
  return (mqd_t)-1;
}

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                   unsigned *msg_prio) {
  (void)mqdes;
  (void)msg_ptr;
  (void)msg_len;
  (void)msg_prio;
  errno = ENOSYS;
  return -1;
}

int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
            unsigned msg_prio) {
  (void)mqdes;
  (void)msg_ptr;
  (void)msg_len;
  (void)msg_prio;
  errno = ENOSYS;
  return -1;
}

int mq_setattr(mqd_t mqdes, const struct mq_attr *restrict mqstat,
               struct mq_attr *restrict omqstat) {
  (void)mqdes;
  (void)mqstat;
  (void)omqstat;
  errno = ENOSYS;
  return -1;
}

ssize_t mq_timedreceive(mqd_t mqdes, char *restrict msg_ptr, size_t msg_len,
                        unsigned *restrict msg_prio,
                        const struct timespec *restrict abs_timeout) {
  (void)mqdes;
  (void)msg_ptr;
  (void)msg_len;
  (void)msg_prio;
  (void)abs_timeout;
  errno = ENOSYS;
  return -1;
}

int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                 unsigned msg_prio, const struct timespec *abs_timeout) {
  (void)mqdes;
  (void)msg_ptr;
  (void)msg_len;
  (void)msg_prio;
  (void)abs_timeout;
  errno = ENOSYS;
  return -1;
}

int mq_unlink(const char *name) {
  (void)name;
  errno = ENOSYS;
  return -1;
}
