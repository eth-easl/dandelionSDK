#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/unistd.h>
#include <sys/signal.h>
#include <time.h>
#include <wordexp.h>

#include <errno.h>
#undef errno
extern int errno;

// defined by newlib, but not always avaliable in headers
extern char** environ;
extern int fcloseall(void);

extern void dandelion_init(void);
extern void dandelion_exit(int errcode);

extern int fs_initialize(int *argc, char ***argv, char ***environ);
extern int fs_terminate();
extern int main(int argc, char *argv[]);
extern void __libc_init_array();
extern void __libc_fini_array();

static inline int process_error(int error){
  if(error < 0){
    errno = -error;
    return -1;
  } else {
    return error;
  }
}

int __initialization() {
  int errcode = 0;
  int argc;
  char **argv;
  errcode = fs_initialize(&argc, &argv, &environ);
  if (errcode != 0) {
    return errcode;
  }
  __libc_init_array();
  errcode = main(argc, argv);
  __libc_fini_array();
  fcloseall();
  fs_terminate();
  return errcode;
}

#include "crt.h"
DANDELION_ENTRY(__initialization)

void _exit(int __status) {
  fs_terminate();
  dandelion_exit(errno);
  __builtin_unreachable();
};

extern int dandelion_isatty(int file);
int isatty(int file) { return  dandelion_isatty(file); }

extern void *dandelion_sbrk(size_t incr);
void *sbrk(ptrdiff_t __incr) { return dandelion_sbrk(__incr); }

extern int dandelion_open(const char *name, int flags, uint32_t mode);
int open(const char *name, int flags, mode_t mode) {
  return process_error(dandelion_open(name, flags, mode));
}

extern int dandelion_mkdir(const char *name, uint32_t mode);
int mkdir(const char *name, mode_t mode) { return process_error(dandelion_mkdir(name, mode)); }

extern int dandelion_rmdir(const char* pathname);
int rmdir(const char* pathname) { return process_error(dandelion_rmdir(pathname)); }

extern int dandelion_close(int file);
int close(int file) { return process_error(dandelion_close(file)); }

extern int dandelion_link(const char *old, const char *new);
int link(const char *__path1, const char *__path2) { return process_error(dandelion_link(__path1, __path2)); }

extern int dandelion_unlink(const char *name);
int unlink(const char *__path) { return process_error(dandelion_unlink(__path)); }

extern int64_t dandelion_lseek(int file, int64_t ptr, int whence);
off_t lseek(int __fildes, off_t __offset, int __whence) {
  return process_error(dandelion_lseek(__fildes, __offset, __whence));
}

extern size_t dandelion_read(int file, char *ptr, size_t len, int64_t offset, char options);
#define USE_OFFSET 1
#define MOVE_OFFSET 2
_READ_WRITE_RETURN_TYPE read(int file, void *ptr, size_t len) {
  return process_error(dandelion_read(file, ptr, len, 0, MOVE_OFFSET));
}
ssize_t pread(int file, void *ptr, size_t len, off_t offset){
  return process_error(dandelion_read(file, ptr, len, offset, USE_OFFSET));
}

extern size_t dandelion_write(int file, const char *ptr, size_t len, int64_t offset, char options);
_READ_WRITE_RETURN_TYPE write(int file, const void *ptr, size_t len) {
  return process_error(dandelion_write(file, (const char*) ptr, len, 0, MOVE_OFFSET));
}
ssize_t pwrite(int file, const void *ptr, size_t len, off_t offset){
  return process_error(dandelion_write(file, (const char*) ptr, len, offset, USE_OFFSET));
}

extern int dandelion_truncate(const char* path, int64_t length);
int truncate(const char* path, off_t length){
  return process_error(dandelion_truncate(path, length));
}
extern int dandelion_ftruncate(int fd, int64_t length);
int ftruncate(int fd, off_t length){
  return process_error(dandelion_ftruncate(fd, length));
}

typedef struct DandelionStat {
  size_t st_mode;
  size_t hard_links;
  size_t file_size;
  size_t blk_size;
} DandelionStat;

extern int dandelion_fstat(int file, DandelionStat *dst);
int fstat(int file, struct stat *st) {
  DandelionStat local = {};
  int error = dandelion_fstat(file, &local);
  if (error < 0){
    errno = -error;
    return -1;
  }
  st->st_mode = local.st_mode;
  st->st_nlink = local.hard_links;
  st->st_size = local.file_size;
  st->st_blksize = local.blk_size;
  st->st_blocks = (local.file_size + 511) / 512;
  return 0;
}

extern int dandelion_stat(const char *file, DandelionStat *dst);
int stat(const char *file, struct stat *st) {
  DandelionStat local = {};
  int error = dandelion_stat(file, &local);
  if (error < 0){
    errno = -error;
    return -1;
  }
  st->st_mode = local.st_mode;
  st->st_nlink = local.hard_links;
  st->st_size = local.file_size;
  st->st_blksize = local.blk_size;
  st->st_blocks = (local.file_size + 511) / 512;
  return 0;
}

int access(const char *file, int mode){
  // check if file exists
  DandelionStat local = {};
  int error = dandelion_stat(file, &local);
  if(error < 0){
    errno = -error;
    return -1;
  }
  // check if the correct permissions are present 
  if((mode & R_OK && !(local.st_mode & S_IRUSR)) 
    || (mode & W_OK && !(local.st_mode & S_IWUSR))
    || (mode & X_OK && !(local.st_mode & S_IXUSR)))
    return -1;

  return 0;
}

int lstat(const char *file, struct stat *buf){
  return stat(file, buf);
}

int statvfs(const char* file, struct statvfs *st){
  errno = ENOSYS;
  return -1;
}

int fstatvfs(int fd, struct statvfs *buf){
  errno = ENOSYS;
  return -1;
}

/* ===========================================================================
  Starting section of symbols that have not real implementation
  Some could have implementations, but don't have one for now.
=========================================================================== */ 

int execve(const char *name, char * const argv[], char * const env[]) {
  errno = ENOMEM;
  return -1;
}

int fcntl(int fd, int op, ...){
  return -1;
}

int fork() {
  errno = EAGAIN;
  return -1;
}

int getpid() { return 1; }

int gettimeofday(struct timeval *p, struct timezone *z) { return -1; }

int kill(int pid, int sig) {
  errno = EINVAL;
  return -1;
}

int pipe(int pipefd[2]){
  errno = EFAULT;
  return -1;
}

int posix_memalign(void **memptr, size_t alignment, size_t size){
  errno = ENOMEM;
  return -1;
}

ssize_t readlink(const char *restrict path, char* restrict buf, size_t bufsize){
  errno = EACCES;
  return -1;
}

char* realpath(const char* path, char* resolved_path){
  errno = EACCES;
  return NULL;
}

int sigaction(
    int signum,
    const struct sigaction *_Nullable restrict act,
    struct sigaction *_Nullable restrict oldact){
  errno = EPERM;
  return -1;
}

long sysconf(int name){
  return -1;
}

clock_t times(struct tms *buf) { return -1; }

int wait(int *status) {
  errno = ECHILD;
  return -1;
}

int wordexp(const char* s, wordexp_t *p, int flags){
  return WRDE_NOSPACE;
}

void wordfree(wordexp_t *p){
  return;
}