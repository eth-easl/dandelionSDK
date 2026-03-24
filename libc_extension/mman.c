#include <sys/mman.h>

int mlock (const void *__addr, size_t __len) { return -1; }
int mlockall (int __flags) { return -1; }
void *mmap (void *__addr, size_t __len, int __prot, int __flags, int __fd, __off_t __offset) {
    return 0; 
}
int mprotect (void *__addr, size_t __len, int __prot) { return -1; }
int msync (void *__addr, size_t __len, int __flags) { return -1; }
int munlock (const void *__addr, size_t __len) { return -1; }
int munlockall (void) { return -1; }
int munmap (void *__addr, size_t __len) { return -1; }
int shm_open (const char *__name, int __oflag, mode_t __mode) { return -1; }
int shm_unlink (const char *__name) { return -1; }