#ifndef _SYS_UIO_H
#define _SYS_UIO_H 1

#include <sys/types.h>

/* Structure for scatter/gather I/O.  */
struct iovec {
  void *iov_base; /* Pointer to data.  */
  size_t iov_len; /* Length of data.  */
};

/* Read data from file descriptor FD, and put the result in the
   buffers described by IOVEC, which is a vector of COUNT 'struct iovec's.
   The buffers are filled in the order specified.
   Operates just like 'read' (see <unistd.h>) except that data are
   put in IOVEC instead of a contiguous buffer.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t readv(int __fd, const struct iovec *__iovec, int __count);

/* Write data pointed by the buffers described by IOVEC, which
   is a vector of COUNT 'struct iovec's, to file descriptor FD.
   The data is written in the order specified.
   Operates just like 'write' (see <unistd.h>) except that the data
   are taken from IOVEC instead of a contiguous buffer.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t writev(int __fd, const struct iovec *__iovec, int __count);

#endif // _SYS_UIO_H
