#ifndef __DANDELION_FILE_SYSTEM__
#define __DANDELION_FILE_SYSTEM__

#include "paths.h"

#include <stdint.h>

#ifndef FS_NAME_LENGTH
#define FS_NAME_LENGHT 64
#endif

#ifndef FS_CHUNK_SIZE
#define FS_CHUNK_SIZE 4096
#endif

#ifndef FS_MAX_FILES
#define FS_MAX_FILES 1024
#endif

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define USE_OFFSET 1
#define MOVE_OFFSET 2

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef enum FileType {
  FILE,
  DIRECTORY,
} FileType;

typedef struct FileChunk {
  char *data;
  size_t capacity;
  size_t used;
  struct FileChunk *next;
} FileChunk;

// Use D_File instead of File, to avoid potential naming overlap
typedef struct D_File {
  char name[FS_NAME_LENGHT];
  struct D_File *next;
  struct D_File *parent;
  FileType type;
  union {
    struct D_File *child;
    FileChunk *content;
  };
  unsigned short hard_links;
  unsigned short open_descripotors;
  // contains mode as described in stat.h
  // https://pubs.opengroup.org/onlinepubs/007904975/basedefs/sys/stat.h.html
  // the mode contains the file type, access bits and set-id bits
  uint32_t mode;
} D_File;

typedef struct OpenFile {
  D_File *file;
  // only meaningful for files
  FileChunk *current_chunk;
  size_t offset;
  // the flags store the flags from the original open call to the file
  // this means they contain one of O_RDONLY, O_WRONLY or O_RDWR and optionally
  // more
  int open_flags;
} OpenFile;

// find a file in a directory using a path as name
// caller needs to ensure that direcotry is actually a directory
// and that path is short enough to be valid file name
D_File *find_file_in_dir(D_File *directory, Path file);

// find a file using a absolute string path
D_File *find_file(const char *name);
// find a file using a path of the absolute name
D_File *find_file_path(Path file_path);

// follow a path and create all directories on the way that do not already
// exist prevent up prevents moving up in the file tree, to prevent input
// items getting written into different input sets by going up the file tree
D_File *create_directories(D_File *directory, Path path, char prevent_up);

D_File *create_file(Path *name, char *content, size_t length, uint32_t mode);

// deallocate file and all data it holds on to
// for directory also deallocate files in folder
int free_data(D_File *file);

// add file to be pointed to by folder
int link_file_to_folder(D_File *folder, D_File *file);

int open_existing_file(unsigned int index, D_File *file, int flags, uint32_t mode,
                       char skip_checks);

#endif // __DANDELION_FILE_SYSTEM__