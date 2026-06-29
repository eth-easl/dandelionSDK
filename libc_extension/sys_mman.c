#include <errno.h>
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
  (void)addr;
  (void)length;
  (void)prot;
  (void)flags;
  (void)fd;
  (void)offset;
  errno = ENOSYS;
  return MAP_FAILED;
}

int mprotect(void *addr, size_t len, int prot) {
  (void)addr;
  (void)len;
  (void)prot;
  errno = ENOSYS;
  return -1;
}

int msync(void *addr, size_t len, int flags) {
  (void)addr;
  (void)len;
  (void)flags;
  errno = ENOSYS;
  return -1;
}

int munmap(void *addr, size_t len) {
  (void)addr;
  (void)len;
  errno = ENOSYS;
  return -1;
}

int mlock(const void *addr, size_t len) {
  (void)addr;
  (void)len;
  errno = ENOSYS;
  return -1;
}

int mlockall(int flags) {
  (void)flags;
  errno = ENOSYS;
  return -1;
}

int munlock(const void *addr, size_t len) {
  (void)addr;
  (void)len;
  errno = ENOSYS;
  return -1;
}

int munlockall(void) {
  errno = ENOSYS;
  return -1;
}

int posix_madvise(void *addr, size_t len, int advice) {
  (void)addr;
  (void)len;
  (void)advice;
  errno = ENOSYS;
  return -1;
}

int shm_open(const char *name, int oflag, mode_t mode) {
  (void)name;
  (void)oflag;
  (void)mode;
  errno = ENOSYS;
  return -1;
}

int shm_unlink(const char *name) {
  (void)name;
  errno = ENOSYS;
  return -1;
}
