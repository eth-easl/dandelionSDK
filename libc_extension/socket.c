#include <sys/socket.h>

int accept (int __fd, struct sockaddr *__addr, socklen_t *__addr_len) {
    return -1;
}

int bind (int __fd, const struct sockaddr *__addr, socklen_t __len) {
    return -1;
}

int connect (int __fd, const struct sockaddr *__addr, socklen_t __len) {
    return -1;
}

int getpeername (int __fd, struct sockaddr __addr, socklen_t *__len) {
    return -1;
}

int getsockname (int __fd, struct sockaddr __addr, socklen_t *__len) {
    return -1;
}

int getsockopt (int __fd, int __level, int __optname, void *__optval, socklen_t *__optlen) {
    return -1;
}

int listen (int __fd, int __n) {
    return -1;
}

ssize_t recv (int __fd, void *__buf, size_t __n, int __flags) {
    return -1;
}

ssize_t recvfrom (int __fd, void *__buf, size_t __n, int __flags, struct sockaddr __addr, socklen_t *__addr_len) {
    return -1;
}

ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags) {
    return -1;
}

ssize_t send (int __fd, const void *__buf, size_t __n, int __flags) {
    return -1;
}

ssize_t sendmsg (int __fd, const struct msghdr *__message, int __flags) {
    return -1;
}

ssize_t sendto (int __fd, const void *__buf, size_t __n, int __flags, const struct sockaddr *__addr, socklen_t __addr_len) {
    return -1;
}

int setsockopt (int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) {
    return -1;
}

int shutdown (int __fd, int __how) {
    return -1;
}

int socket (int __domain, int __type, int __protocol) {
    return -1;
}

int sockatmark (int __fd) {
    return -1;
}

int socketpair (int __domain, int __type, int __protocol, int __fds[2]) {
    return -1;
}
