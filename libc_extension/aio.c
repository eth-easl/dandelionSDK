#include <aio.h>
#include <errno.h>

// https://man7.org/linux/man-pages/man7/aio.7.html

int aio_cancel(int fd, struct aiocb *aiocbp) {
  (void)fd;
  (void)aiocbp;
  errno = ENOSYS;
  return -1;
}

int aio_error(const struct aiocb *aiocbp) {
  (void)aiocbp;
  errno = ENOSYS;
  return -1;
}

int aio_fsync(int op, struct aiocb *aiocbp) {
  (void)op;
  (void)aiocbp;
  errno = ENOSYS;
  return -1;
}

int aio_read(struct aiocb *aiocbp) {
  (void)aiocbp;
  errno = ENOSYS;
  return -1;
}

ssize_t aio_return(struct aiocb *aiocbp) {
  (void)aiocbp;
  errno = ENOSYS;
  return -1;
}

int aio_suspend(const struct aiocb *const aiocb_list[], int nitems,
                const struct timespec *timeout) {
  (void)aiocb_list;
  (void)nitems;
  (void)timeout;
  errno = ENOSYS;
  return -1;
}

int aio_write(struct aiocb *aiocbp) {
  (void)aiocbp;
  errno = ENOSYS;
  return -1;
}

int lio_listio(int mode, struct aiocb *restrict const list[restrict], int nent,
               struct sigevent *restrict sig) {
  (void)mode;
  (void)list;
  (void)nent;
  (void)sig;
  errno = ENOSYS;
  return -1;
}
