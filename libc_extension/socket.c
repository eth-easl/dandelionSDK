#include <sys/socket.h>
#include <errno.h>

int accept (int __fd, struct sockaddr *__addr, socklen_t *__addr_len) {
    (void)__fd; (void)__addr; (void)__addr_len;
    errno = ENOSYS;
    return -1;
}

int bind (int __fd, const struct sockaddr *__addr, socklen_t __len) {
    (void)__fd; (void)__addr; (void)__len;
    errno = ENOSYS;
    return -1;
}

int connect (int __fd, const struct sockaddr *__addr, socklen_t __len) {
    (void)__fd; (void)__addr; (void)__len;
    errno = ENOSYS;
    return -1;
}

int getpeername (int __fd, struct sockaddr *__addr, socklen_t *__len) {
    (void)__fd; (void)__addr; (void)__len;
    errno = ENOSYS;
    return -1;
}

int getsockname (int __fd, struct sockaddr *__addr, socklen_t *__len) {
    (void)__fd; (void)__addr; (void)__len;
    errno = ENOSYS;
    return -1;
}

int getsockopt (int __fd, int __level, int __optname, void *__optval, socklen_t *__optlen) {
    (void)__fd; (void)__level; (void)__optname; (void)__optval; (void)__optlen;
    errno = ENOSYS;
    return -1;
}

int listen (int __fd, int __n) {
    (void)__fd; (void)__n;
    errno = ENOSYS;
    return -1;
}

ssize_t recv (int __fd, void *__buf, size_t __n, int __flags) {
    (void)__fd; (void)__buf; (void)__n; (void)__flags;
    errno = ENOSYS;
    return -1;
}

ssize_t recvfrom (int __fd, void *__buf, size_t __n, int __flags, struct sockaddr *__addr, socklen_t *__addr_len) {
    (void)__fd; (void)__buf; (void)__n; (void)__flags; (void)__addr; (void)__addr_len;
    errno = ENOSYS;
    return -1;
}

ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags) {
    (void)__fd; (void)__message; (void)__flags;
    errno = ENOSYS;
    return -1;
}

ssize_t send (int __fd, const void *__buf, size_t __n, int __flags) {
    (void)__fd; (void)__buf; (void)__n; (void)__flags;
    errno = ENOSYS;
    return -1;
}

ssize_t sendmsg (int __fd, const struct msghdr *__message, int __flags) {
    (void)__fd; (void)__message; (void)__flags;
    errno = ENOSYS;
    return -1;
}

ssize_t sendto (int __fd, const void *__buf, size_t __n, int __flags, const struct sockaddr *__addr, socklen_t __addr_len) {
    (void)__fd; (void)__buf; (void)__n; (void)__flags; (void)__addr; (void)__addr_len;
    errno = ENOSYS;
    return -1;
}

int setsockopt (int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) {
    (void)__fd; (void)__level; (void)__optname; (void)__optval; (void)__optlen;
    errno = ENOSYS;
    return -1;
}

int shutdown (int __fd, int __how) {
    (void)__fd; (void)__how;
    errno = ENOSYS;
    return -1;
}

int socket (int __domain, int __type, int __protocol) {
    (void)__domain; (void)__type; (void)__protocol;
    errno = ENOSYS;
    return -1;
}

int sockatmark (int __fd) {
    (void)__fd;
    errno = ENOSYS;
    return -1;
}

int socketpair (int __domain, int __type, int __protocol, int __fds[2]) {
    (void)__domain; (void)__type; (void)__protocol; (void)__fds;
    errno = ENOSYS;
    return -1;
}
