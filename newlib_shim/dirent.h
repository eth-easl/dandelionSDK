#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

typedef struct DIR {
  void *dir_ptr;
  long int child;
} DIR;

#define _DIRENT_HAVE_D_TYPE
#define _DIRENT_HAVE_D_OFF
#define DT_UNKOWN 0
#define DT_REG 1
#define DT_DIR 2

struct dirent {
  ino_t d_ino;
  off_t d_off;
  unsigned char d_type;
  char d_name[64];
};

DIR *opendir(const char *name);

int closedir(DIR *dir);

struct dirent *readdir(DIR *dir);
// not implementing readdir_r bevause it was depricated in glibc 2.24

long int telldir(DIR *dir);

void seekdir(DIR *dir, long int index);

#ifdef __cplusplus
}
#endif