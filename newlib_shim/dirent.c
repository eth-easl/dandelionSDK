#include <dirent.h>

#include <stdlib.h>
#include <errno.h>

extern int dandelion_opendir(const char *name, DIR *dir);
DIR *opendir(const char *name) {
  DIR *new_dir = (DIR *)malloc(sizeof(DIR));
  int error = dandelion_opendir(name, new_dir);
  if (error != 0) {
    *__errno() = -error;
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
  if(local_dir == NULL){
    *__errno() = ENOMEM;
    return NULL;
  }
  int error = dandelion_readdir(dir, local_dir);
  if (error == 0) {
    return local_dir;
  } else {
    free(local_dir);
    if(error != -1)
      *__errno()  = -error;
    return NULL;
  }
}

// not implementing readdir_r bevause it was depricated in glibc 2.24

extern long int dandelion_telldir(DIR *dir);
long int telldir(DIR *dir) {
  int error = dandelion_telldir(dir);
  if(error < 0){
    *__errno()  = -error;
    return -1; 
  } else {
    return error;
  }
}

extern void dandelion_seekdir(DIR *dir, long int index);
void seekdir(DIR *dir, long int index) { return dandelion_seekdir(dir, index); }

