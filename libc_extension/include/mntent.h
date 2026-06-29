#ifndef _MNTENT_H
#define _MNTENT_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* https://codebrowser.dev/glibc/glibc/misc/mntent.h.html */
#define MNTTYPE_IGNORE "ignore"
#define MNTTYPE_NFS "nfs"
#define MNTTYPE_SWAP "swap"
#define MNTOPT_DEFAULTS "defaults"
#define MNTOPT_RO "ro"
#define MNTOPT_RW "rw"
#define MNTOPT_SUID "suid"
#define MNTOPT_NOSUID "nosuid"
#define MNTOPT_NOAUTO "noauto"

struct mntent {
  char *mnt_fsname;
  char *mnt_dir;
  char *mnt_type;
  char *mnt_opts;
  int mnt_freq;
  int mnt_passno;
};

FILE *setmntent(const char *, const char *);
struct mntent *getmntent(FILE *);
struct mntent *getmntent_r(FILE *, struct mntent *, char *, int);
int addmntent(FILE *, const struct mntent *);
int endmntent(FILE *);
char *hasmntopt(const struct mntent *, const char *);

#ifdef __cplusplus
}
#endif

#endif /* _MNTENT_H */
