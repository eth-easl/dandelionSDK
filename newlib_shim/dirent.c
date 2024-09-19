#include <dirent.h>

#include <stdlib.h>

extern int dandelion_opendir(const char *name, DIR *dir);
DIR *opendir(const char *name) {
  DIR *new_dir = (DIR *)malloc(sizeof(DIR));
  int errno = dandelion_opendir(name, new_dir);
  if (errno != 0) {
    return NULL;
  } else {
    return new_dir;
  }
}

extern int dandelion_closedir(DIR *dir);
int closedir(DIR *dir) { return dandelion_closedir(dir); }

extern int dandelion_readdir(DIR *dir, struct dirent *dirent_ptr);
struct dirent *readdir(DIR *dir) {
  struct dirent *local_dir = (struct dirent *)malloc(sizeof(struct dirent));
  int errno = dandelion_readdir(dir, local_dir);
  if (errno == 0) {
    return local_dir;
  } else {
    free(local_dir);
    return NULL;
  }
}

// not implementing readdir_r bevause it was depricated in glibc 2.24

extern long int dandelion_telldir(DIR *dir);
long int telldir(DIR *dir) { return dandelion_telldir(dir); }

extern void dandelion_seekdir(DIR *dir, long int index);
void seekdir(DIR *dir, long int index) { return dandelion_seekdir(dir, index); }

