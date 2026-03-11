#ifndef _UTMPX_H
#define _UTMPX_H

#include <sys/time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Linux glibc bits/utmpx.h sizes:
 * https://sources.debian.org/src/glibc/2.41-12/sysdeps/gnu/bits/utmpx.h/
 */
#define __UT_LINESIZE 32
#define __UT_NAMESIZE 32

struct utmpx {
  short ut_type;
  pid_t ut_pid;
  char ut_line[__UT_LINESIZE];
  char ut_id[4];
  char ut_user[__UT_NAMESIZE];
  struct timeval ut_tv;
};

/* Linux glibc ut_type values:
 * https://sources.debian.org/src/glibc/2.41-12/sysdeps/gnu/bits/utmpx.h/
 */
#define EMPTY         0
#define BOOT_TIME     2
#define NEW_TIME      3
#define OLD_TIME      4
#define INIT_PROCESS  5
#define LOGIN_PROCESS 6
#define USER_PROCESS  7
#define DEAD_PROCESS  8

void endutxent(void);
struct utmpx *getutxent(void);
struct utmpx *getutxid(const struct utmpx *);
struct utmpx *getutxline(const struct utmpx *);
struct utmpx *pututxline(const struct utmpx *);
void setutxent(void);

#ifdef __cplusplus
}
#endif

#endif /* _UTMPX_H */
