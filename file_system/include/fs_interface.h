#ifndef _DANDELION_FS_INTERFACE_H
#define _DANDELION_FS_INTERFACE_H

#include <sys/stat.h>

#include "../../include/dandelion/runtime.h"
#include "../file_system.h"

// Fake that stdin, stdout and stderr are TTY
int dandelion_isatty(int file);

int dandelion_link(char *old, char *new_name);

int dandelion_unlink(char *name);

int dandelion_open(const char *name, int flags, mode_t mode);

int dandelion_close(int file);

int dandelion_lseek(int file, int ptr, int dir);

int dandelion_read(int file, char *ptr, int len);

int dandelion_write(int file, char *ptr, int len);

int dandelion_fstat(int file, struct stat *st);

int dandelion_stat(char *file, struct stat *st);

/// @brief initializes the filesystem from the existing sets and item buffers
/// @return an erorr code or 0 if there were no error
int fs_initialize();

/// @brief close all files, write them into contiguous buffers where necessary
/// and set up output buffer descriptors where necessary
/// @return an error code or 0 if there was no error
int fs_terminate();

#endif // __DANDELION_FS_INTERFACE_H__
