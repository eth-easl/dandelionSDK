#include <errno.h>
#include <unistd.h>

// Implement functions from unistd that are not already defined in other parts
// of newlibs libc Or in something we supply

// unsigned  alarm (unsigned __secs);
int chdir(const char *__path) {
  *__errno() = EACCES;
  return -1;
}
int fchdir(int __fildes) {
  *__errno() = EACCES;
  return -1;
}

int chroot(const char *__path) {
  *__errno() = EACCES;
  return -1;
}

// int     chmod (const char *__path, mode_t __mode);
// int     chown (const char *__path, uid_t __owner, gid_t __group);
// int     close_range (unsigned int __firstfd, unsigned int __lastfd, int
// __flags); size_t	confstr (int __name, char *__buf, size_t __len); char *
// crypt (const char *__key, const char *__salt); char *  ctermid (char *__s);
// char *  cuserid (char *__s);
// int	daemon (int nochdir, int noclose);

// function to duplicate file descriptor to second file number
// the filedescriptors should act as one, meaning if seek is called on one of
// them, the effects should also be visible accessing the file through the new
// one. if the second parameter is -1, the lowest free file descriptor is
// chosen, otherwise the file descriptor in filedes2 is used to duplicate the
// filedescriptor. If filedes2 was open before, it will be closed.
int dup3(int __fildes, int __fildes2, int flags) {
  *__errno() = EMFILE;
  return -1;
}
int dup2(int __fildes, int __fildes2) { return dup3(__fildes, __fildes2, 0); }
int dup(int __fildes) { return dup3(__fildes, -1, 0); }

int eaccess(const char *__path, int __mode) { return access(__path, __mode); }
int euidaccess(const char *__path, int __mode) {
  return eaccess(__path, __mode);
}

// void	encrypt (char *__block, int __edflag);
// void	endusershell (void);

// execve is defined in newlib
// int     execl (const char *__path, const char *, ...);
// int     execle (const char *__path, const char *, ...);
// int     execlp (const char *__file, const char *, ...);
// #if __MISC_VISIBLE
// int     execlpe (const char *__file, const char *, ...);
// #endif
int execv(const char *__path, char *const __argv[]) {
  return execve(__path, __argv, environ);
}
// already given in newlib
// int     execve (const char *__path, char * const __argv[], char * const
// __envp[]); int     execvp (const char *__file, char * const __argv[]); int
// execvpe (const char *__file, char * const __argv[], char * const __envp[]);
// #if __ATFILE_VISIBLE
// int	faccessat (int __dirfd, const char *__path, int __mode, int __flags);
// #endif
// int     fchmod (int __fildes, mode_t __mode);
// int     fchown (int __fildes, uid_t __owner, gid_t __group);
// #if __ATFILE_VISIBLE
// int	fchownat (int __dirfd, const char *__path, uid_t __owner, gid_t __group,
// int __flags); #endif int	fexecve (int __fd, char * const __argv[], char *
// const __envp[]); long    fpathconf (int __fd, int __name);
int fsync(int __fd) { return 0; }
int fdatasync(int __fd) { return 0; }
// char *  get_current_dir_name (void);
char *getcwd(char *__buf, size_t __size) { return "/"; }
// int	getdomainname  (char *__name, size_t __len);
// gid_t   getegid (void);
// uid_t   geteuid (void);
// gid_t   getgid (void);
// int     getgroups (int __gidsetsize, gid_t __grouplist[]);
// long    gethostid (void);
// char *  getlogin (void);
// #if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
// int getlogin_r (char *name, size_t namesize) ;
// #endif
// char *  getpass (const char *__prompt);
// int	getpagesize (void);
// pid_t   getpgid (pid_t);
// pid_t   getpgrp (void);
// pid_t   getpid (void); // already implementedc
// pid_t   getppid (void);
// pid_t   getsid (pid_t);
// uid_t   getuid (void);
// char *	getusershell (void);
// char *  getwd (char *__buf);
// int     lchown (const char *__path, uid_t __owner, gid_t __group);
// #if __ATFILE_VISIBLE
// int     linkat (int __dirfd1, const char *__path1, int __dirfd2, const char
// *__path2, int __flags); #endif #if __MISC_VISIBLE || __XSI_VISIBLE int
// nice (int __nice_value); #endif #if __MISC_VISIBLE || __XSI_VISIBLE >= 4 int
// lockf (int __fd, int __cmd, off_t __len); #endif long    pathconf (const char
// *__path, int __name); int     pause (void); #if __POSIX_VISIBLE >= 199506 int
// pthread_atfork (void (*)(void), void (*)(void), void (*)(void)); #endif int
// pipe (int __fildes[2]) {
//     pipe2(__fileds[2], 0);
// }
// int     pipe2 (int __fildes[2], int flags);
// #if __POSIX_VISIBLE >= 200809 || __XSI_VISIBLE >= 500
// ssize_t pread (int __fd, void *__buf, size_t __nbytes, off_t __offset);
// ssize_t pwrite (int __fd, const void *__buf, size_t __nbytes, off_t
// __offset); #endif _READ_WRITE_RETURN_TYPE read (int __fd, void *__buf, size_t
// __nbyte); #if __BSD_VISIBLE int	rresvport (int *__alport); int	revoke
// (char *__path); #endif int     rmdir (const char *__path); #if __BSD_VISIBLE
// int	ruserok (const char *rhost, int superuser, const char *ruser, const char
// *luser); #endif #if __BSD_VISIBLE || (__XSI_VISIBLE >= 4 && __POSIX_VISIBLE <
// 200112) void *  sbrk (ptrdiff_t __incr); #endif #if __BSD_VISIBLE ||
// __POSIX_VISIBLE >= 200112 int     setegid (gid_t __gid); int     seteuid
// (uid_t __uid); #endif int     setgid (gid_t __gid);
int setgroups(int ngroups, const gid_t *grouplist) {
  *__errno() = EPERM;
  return -1;
}
// #if __BSD_VISIBLE || (__XSI_VISIBLE && __XSI_VISIBLE < 500)
// int	sethostname (const char *, size_t);
// #endif
// int     setpgid (pid_t __pid, pid_t __pgid);
// #if __SVID_VISIBLE || __XSI_VISIBLE >= 500
// int     setpgrp (void);
// #endif
// #if __BSD_VISIBLE || __XSI_VISIBLE >= 4
// int	setregid (gid_t __rgid, gid_t __egid);
// int	setreuid (uid_t __ruid, uid_t __euid);
// #endif
// pid_t   setsid (void);
// int     setuid (uid_t __uid);
// #if __BSD_VISIBLE || (__XSI_VISIBLE && __XSI_VISIBLE < 500)
// void	setusershell (void);
// #endif
// unsigned sleep (unsigned int __seconds);
// #if __XSI_VISIBLE
// void    swab (const void *__restrict, void *__restrict, ssize_t);
// #endif
// long    sysconf (int __name);
// pid_t   tcgetpgrp (int __fildes);
// int     tcsetpgrp (int __fildes, pid_t __pgrp_id);
// char *  ttyname (int __fildes);
// int     ttyname_r (int, char *, size_t);
// int     unlink (const char *__path);
// #if __XSI_VISIBLE >= 500 && __POSIX_VISIBLE < 200809 || __BSD_VISIBLE
int usleep(useconds_t __useconds) { return 0; }
// #endif
// #if __BSD_VISIBLE
// int     vhangup (void);
// #endif
// _READ_WRITE_RETURN_TYPE write (int __fd, const void *__buf, size_t __nbyte);

// #ifdef __CYGWIN__
// # define __UNISTD_GETOPT__
// # include <getopt.h>
// # undef __UNISTD_GETOPT__
// #else
// extern char *optarg;			/* getopt(3) external variables */
// extern int optind, opterr, optopt;
// int	 getopt(int, char * const [], const char *);
// extern int optreset;			/* getopt(3) external variable */
// #endif

// #if __BSD_VISIBLE || (__XSI_VISIBLE >= 4 && __POSIX_VISIBLE < 200809)
// pid_t   vfork (void);
// #endif

// #if __BSD_VISIBLE || __POSIX_VISIBLE < 200112
// int	getdtablesize (void);
// #endif
// #if __BSD_VISIBLE || __POSIX_VISIBLE >= 200809 || __XSI_VISIBLE >= 500
// useconds_t ualarm (useconds_t __useconds, useconds_t __interval);
// #endif

// #if __BSD_VISIBLE || __POSIX_VISIBLE >= 200112 || __XSI_VISIBLE >= 500
// #if !(defined  (_WINSOCK_H) || defined (_WINSOCKAPI_) || defined
// (__USE_W32_SOCKETS))
// /* winsock[2].h defines as __stdcall, and with int as 2nd arg */
//  int	gethostname (char *__name, size_t __len);
// #endif
// #endif

// #if __MISC_VISIBLE
// int	setdtablesize (int);
// #endif

// #if __BSD_VISIBLE || __XSI_VISIBLE >= 500
// void    sync (void);
// #endif

// #if __BSD_VISIBLE || __POSIX_VISIBLE >= 200112 || __XSI_VISIBLE >= 4
// ssize_t readlink (const char *__restrict __path,
//                           char *__restrict __buf, size_t __buflen);

int symlink(const char *__name1, const char *__name2) {
  *__errno() = EPERM;
  return -1;
}
// #endif
// #if __ATFILE_VISIBLE
// ssize_t        readlinkat (int __dirfd1, const char *__restrict __path,
//                             char *__restrict __buf, size_t __buflen);
// int	symlinkat (const char *, int, const char *);
// int	unlinkat (int, const char *, int);
// #endif