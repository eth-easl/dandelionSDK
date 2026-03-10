#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Linux utsname fields are 65 bytes:
 * https://man7.org/linux/man-pages/man2/uname.2.html
 * https://codebrowser.dev/glibc/glibc/sysdeps/unix/sysv/linux/bits/utsname.h.html
 */
struct utsname {
  char sysname[65];
  char nodename[65];
  char release[65];
  char version[65];
  char machine[65];
};

int uname(struct utsname *);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_UTSNAME_H */
