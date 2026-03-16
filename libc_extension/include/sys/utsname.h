#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Linux utsname fields are 65 bytes:
 * https://man7.org/linux/man-pages/man2/uname.2.html
 * https://sources.debian.org/src/glibc/2.41-12/sysdeps/unix/sysv/linux/bits/utsname.h/
 * Dandelion SDK sets UTSNAME_FIELD_SIZE in the top-level CMakeLists.txt defaults.
 */
struct utsname {
  char sysname[@UTSNAME_FIELD_SIZE@];
  char nodename[@UTSNAME_FIELD_SIZE@];
  char release[@UTSNAME_FIELD_SIZE@];
  char version[@UTSNAME_FIELD_SIZE@];
  char machine[@UTSNAME_FIELD_SIZE@];
};

int uname(struct utsname *);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_UTSNAME_H */
