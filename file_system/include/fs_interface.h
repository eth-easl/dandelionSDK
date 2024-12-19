#ifndef _DANDELION_FS_INTERFACE_H
#define _DANDELION_FS_INTERFACE_H

#include "../file_system.h"

#include <stddef.h>
#include <stdint.h>

#define EPERM          1
#define ENOENT         2
#define EBADF          9
#define ENOMEM        12
#define EACCES        13
#define EEXIST        17
#define ENOTDIR       20
#define EINVAL        22
#define EMFILE        24
#define EMLINK        31
#define ENAMETOOLONG  36
#define ENOTEMPTY     39 

typedef struct DIR {
  D_File *dir;
  long int child;
} DIR;

#define DT_UNKOWN 0
#define DT_REG 1
#define DT_DIR 2

#define	SEEK_SET	0	/* set file offset to offset */
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#define	SEEK_END	2	/* set file offset to EOF plus offset */

struct dirent {
  size_t d_off;
  uint16_t d_ino;
  unsigned char d_type;
  char d_name[64];
};

#define S_IXUSR 00100
#define S_IWUSR 00200
#define S_IRUSR 00400
#define S_IRWXU (S_IXUSR|S_IWUSR|S_IRUSR)

#define S_IFDIR 0040000
#define S_IFREG 0100000

// Are matched to newlib, need to ensure that if they change them we keep it up to date
// if this changes more often might want to define our own sys/fctl.h
#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_APPEND  0x008
#define O_CREAT   0x200
#define O_TRUNC   0x400
#define O_EXCL    0x800
#define O_ACCMODE (O_RDONLY|O_WRONLY|O_RDWR) 

// Fake that stdin, stdout and stderr are TTY
int dandelion_isatty(int file);

int dandelion_link(char *old, char *new_name);

int dandelion_unlink(const char *name);

int dandelion_open(const char *name, int flags, uint32_t mode);

int dandelion_close(int file);

int dandelion_lseek(int file, int ptr, int dir);

int dandelion_read(int file, char *ptr, int len, int offset, char options);

int dandelion_write(int file, char *ptr, int len, int offset, char options);

typedef struct DandelionStat {
  size_t st_mode;
  size_t hard_links;
  size_t file_size;
  size_t blk_size;
} DandelionStat;

// use dandelion stat and convert externally so the conversion happens with th
// expected structs of the caller
int dandelion_stat(char *file, DandelionStat *st);
int dandelion_fstat(int file, DandelionStat *st);

/// @brief initializes the filesystem from the existing sets and item buffers
/// @return an erorr code or 0 if there were no error
int fs_initialize(int *argc, char ***argv, char ***environ);

/// @brief close all files, write them into contiguous buffers where necessary
/// and set up output buffer descriptors where necessary
/// @return an error code or 0 if there was no error
int fs_terminate();

#endif // __DANDELION_FS_INTERFACE_H__
