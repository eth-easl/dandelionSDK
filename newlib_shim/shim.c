#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>

#include <errno.h>
#undef errno
extern int errno;

extern void dandelion_init(void);
extern void dandelion_exit(int errcode);

extern int fs_initialize(int *argc, char ***argv, char ***environ);
extern int fs_terminate();
extern int main(int argc, char const *argv[]);
int __initialization() {
  int errcode = 0;
  int argc;
  char const **argv;
  char const **environ;
  errcode = fs_initialize(&argc, &argv, &environ);
  if (errcode != 0) {
    return errcode;
  }
  errcode = main(argc, argv);
  fs_terminate();
  return errcode;
}

#include "crt.h"
DANDELION_ENTRY(__initialization)

void _exit(void) {
  fs_terminate();
  dandelion_exit(errno);
};

extern int dandelion_isatty(int file);
int isatty(int file) { return dandelion_isatty(file); }

extern void *dandelion_sbrk(int incr);
void *sbrk(int incr) { return dandelion_sbrk(incr); }

extern int dandelion_open(const char *name, int flags, mode_t mode);
int open(const char *name, int flags, mode_t mode) {
  return dandelion_open(name, flags, mode);
}

extern int dandelion_mkdir(const char *name, mode_t mode);
int mkdir(const char *name, mode_t mode) { return dandelion_mkdir(name, mode); }

extern int dandelion_close(int file);
int close(int file) { return dandelion_close(file); }

extern int dandelion_link(char *old, char *new);
int link(char *old, char *new) { return dandelion_link(old, new); }

extern int dandelion_unlink(char *name);
int unlink(char *name) { return dandelion_unlink(name); }

extern int dandelion_lseek(int file, int ptr, int dir);
int lseek(int file, int ptr, int dir) {
  return dandelion_lseek(file, ptr, dir);
}

extern int dandelion_read(int file, char *ptr, int len);
int read(int file, char *ptr, int len) {
  return dandelion_read(file, ptr, len);
}

extern int dandelion_write(int file, char *ptr, int len);
int write(int file, char *ptr, int len) {
  return dandelion_write(file, ptr, len);
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
  if (error != 0)
    return error;
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
  if (error != 0)
    return error;
  st->st_mode = local.st_mode;
  st->st_nlink = local.hard_links;
  st->st_size = local.file_size;
  st->st_blksize = local.blk_size;
  st->st_blocks = (local.file_size + 511) / 512;
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

// not yet available in dandelion or default implementation

/* pointer to array of char * strings that define the current environment
 * variables */
// char **environ = {0};

int execve(char *name, char **argv, char **env) {
  int errno = ENOMEM;
  return -1;
}

int fork() {
  errno = EAGAIN;
  return -1;
}

int getpid() { return 1; }
int kill(int pid, int sig) {
  errno = EINVAL;
  return -1;
}

int wait(int *status) {
  errno = ECHILD;
  return -1;
}

clock_t times(struct tms *buf) { return -1; }
int gettimeofday(struct timeval *p, struct timezone *z) { return -1; }