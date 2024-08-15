#include "file_system.h"
#include "include/fs_interface.h"
#include "paths.h"

#include <dandelion/system/system.h>

#include <sys/fcntl.h>

extern D_File *fs_root;
extern OpenFile *open_files;

// Fake that stdin, stdout and stderr are TTY
int dandelion_isatty(int file) {
  switch (file) {
  case STDIN_FILENO:
  case STDOUT_FILENO:
  case STDERR_FILENO:
    return 1;
  default:
    return 0;
  }
}

int dandelion_link(char *old, char *new_name) {
  // find the old file
  D_File *file = find_file(old);
  if (file == NULL) {
    errno = ENOTDIR;
    return -1;
  }
  // check that the old file is not a directory
  if (file->type == DIRECTORY) {
    errno = EPERM;
    return -1;
  }
  // check we could add a hard link
  if (file->hard_links == -1) {
    errno = EMLINK;
    return -1;
  }
  D_File *new_file = find_file(new_name);
  if (new_file != NULL) {
    errno = EEXIST;
    return -1;
  }
  // create necessary folders on the way
  Path new_path = path_from_string(new_name);
  Path new_file_name = get_file(new_path);
  Path new_file_dir = get_directories(new_path);
  D_File *new_dir = create_directories(fs_root, new_file_dir, 0);
  if (new_dir == NULL) {
    errno = ENOTDIR;
    return -1;
  }
  // know is directory
  link_file_to_folder(new_dir, file);
  file->hard_links += 1;
  return 0;
}

int dandelion_unlink(char *name) {
  // find the file
  D_File *file = find_file(name);
  if (file == NULL) {
    errno = ENOTDIR;
    return -1;
  }
  D_File *parent = file->parent;
  // remove file from parent
  if (parent->child == file) {
    parent->child = file->next;
  } else {
    for (D_File *child = parent->child; child != NULL; child = child->next) {
      if (child->next == file) {
        child->next = file->next;
        break;
      }
    }
  }
  file->hard_links -= 1;
  free_data(file);
  return 0;
}

// currently do not handle O_TMPFILE
int dandelion_open(const char *name, int flags, mode_t mode) {
  // check if file is already open, if so fail O_TRUNC with EACCESS
  // get path from name
  D_File *current = find_file(name);
  // report error if O_EXCL is set and file exists
  if (flags & O_EXCL && flags & O_CREAT && current != NULL) {
    errno = EEXIST;
    return -1;
  }
  // handle if file was not found
  if (current == NULL) {
    // if flag to create was not set return error
    if (!(flags & O_CREAT)) {
      errno = ENOENT;
      return -1;
    }
    // should create a file
    // need to create another copy as the original was modified in the loop
    Path total_path = path_from_string(name);
    Path dir_path = get_directories(total_path);
    Path file_name = get_file(total_path);
    if (file_name.length >= FS_NAME_LENGHT) {
      errno = EINVAL;
      return -1;
    }
    D_File *parent = create_directories(fs_root, dir_path, 0);
    if (parent == NULL) {
      errno = ENOTDIR;
      return -1;
    }
    current = create_file(&file_name, NULL, 0, 0);
    if (current == NULL) {
      errno = ENOMEM;
      return -1;
    }
    current->mode = mode;
    if (link_file_to_folder(parent, current)) {
      errno = ENOTDIR;
      return -1;
    }
  }
  // at this point know that current is pointing to a valid file
  // find lowerst non taken file descriptor
  unsigned int file_descriptor = 0;
  for (; file_descriptor < FS_MAX_FILES; file_descriptor++) {
    if (open_files[file_descriptor].file == NULL) {
      break;
    }
  }
  if (file_descriptor == FS_MAX_FILES) {
    errno = EMFILE;
    return -1;
  }
  if (open_existing_file(file_descriptor, current, flags, mode, 1) != 0) {
    return -1;
  }
  current->open_descripotors += 1;
  return file_descriptor;
}

int dandelion_close(int file) {
  // check the file is open
  D_File *to_close = open_files[file].file;
  if (to_close == NULL) {
    errno = EBADF;
    return -1;
  }
  to_close->open_descripotors -= 1;
  free_data(to_close);
  open_files[file].file = NULL;
  return 0;
}

int dandelion_lseek(int file, int ptr, int dir) { return 0; }

int dandelion_read(int file, char *ptr, int len) {
  // get the file descriptor
  OpenFile *open_file = &open_files[file];
  // check there is a valid file descriptor there and that it is writable
  if (open_file->file == NULL || open_file->open_flags & O_WRONLY) {
    errno = EBADF;
    return -1;
  }
  if (open_file->file->type != FILE) {
    errno = EINVAL;
    return -1;
  }
  // if len is 0, it is supposed to only check for these errors and return
  if (len == 0) {
    return 0;
  }

  D_File *d_file = open_file->file;
  // check if current chunk is set, and if not if the file has data to be read
  if (open_file->current_chunk == NULL) {
    // set to file chunk if there is one
    open_file->offset = 0;
    if (d_file->content != NULL)
      open_file->current_chunk = d_file->content;
    else
      return 0;
  }

  int read_bytes = 0;
  int need_to_read = len;
  while (1) {
    FileChunk *current = open_file->current_chunk;
    size_t readable = current->used - open_file->offset;
    if (readable > need_to_read) {
      memcpy(ptr + read_bytes, current->data + open_file->offset, need_to_read);
      open_file->offset += need_to_read;
      read_bytes += need_to_read;
      return read_bytes;
    } else {
      // read everythin in file and go to next
      memcpy(ptr + read_bytes, current->data + open_file->offset, readable);
      read_bytes += readable;
      // advance to next chunk if there is one, otherwise stay at this, so we
      // can see new appended chunks in the future
      if (current->next != NULL) {
        open_file->current_chunk = current->next;
        open_file->offset = 0;
      } else {
        open_file->offset += readable;
        return read_bytes;
      }
    }
  }
  return read_bytes;
}

int dandelion_write(int file, char *ptr, int len) {
  // get the file descriptor
  OpenFile *open_file = &open_files[file];
  // check there is a valid file descriptor there and that it is writable
  if (open_file->file == NULL || open_file->open_flags & O_RDONLY) {
    errno = EBADF;
    return -1;
  }
  if (open_file->file->type != FILE) {
    errno = EINVAL;
    return -1;
  }

  D_File *d_file = open_file->file;
  // if have no chunck already check if file has a chunck and write to that
  // otherwise allocate one
  if (open_file->current_chunk == NULL) {
    if (d_file->content == NULL) {
      char *new_buffer = dandelion_alloc(FS_CHUNCK_SIZE, _Alignof(max_align_t));
      if (new_buffer == NULL) {
        errno = ENOMEM;
        return -1;
      }
      FileChunk *new_chunck =
          dandelion_alloc(sizeof(FileChunk), _Alignof(FileChunk));
      if (new_chunck == NULL) {
        errno = ENOMEM;
        return -1;
      }
      new_chunck->capacity = FS_CHUNCK_SIZE;
      new_chunck->data = new_buffer;
      new_chunck->used = 0;
      new_chunck->next = NULL;
      d_file->content = new_chunck;
      open_file->current_chunk = new_chunck;
      open_file->offset = 0;
    } else {
      open_file->current_chunk = d_file->content;
      open_file->offset = 0;
    }
  }
  // check file mode, if writing a O_APPEND file, need to jump to end of file
  // before writing
  if (open_file->open_flags & O_APPEND) {
    // check that the current is at the end of what could be
    while (open_file->current_chunk->next != NULL) {
      open_file->current_chunk = open_file->current_chunk->next;
    }
    open_file->offset = open_file->current_chunk->used;
  }
  // know that we can start writing to end of current chunck

  size_t writen_bytes = 0;
  while (len > 0) {
    int writeable =
        open_file->current_chunk->capacity - open_file->current_chunk->used;
    if (writeable == 0) {
      FileChunk *new_chunck =
          dandelion_alloc(sizeof(FileChunk), _Alignof(FileChunk));
      if (new_chunck == NULL) {
        errno = ENOMEM;
        return -1;
      }
      char *allocation = dandelion_alloc(FS_CHUNCK_SIZE, _Alignof(max_align_t));
      if (allocation == NULL) {
        errno = ENOMEM;
        return -1;
      }
      new_chunck->data = allocation;
      new_chunck->capacity = FS_CHUNCK_SIZE;
      new_chunck->used = 0;
      new_chunck->next = NULL;
      open_file->current_chunk->next = new_chunck;
      open_file->current_chunk = new_chunck;
      writeable = FS_CHUNCK_SIZE;
    }
    size_t to_write = MIN(len, writeable);
    memcpy(open_file->current_chunk->data, ptr, to_write);
    len -= to_write;
    ptr += to_write;
    open_file->current_chunk->used += to_write;
    writen_bytes += to_write;
  }

  return writen_bytes;
}

int dandelion_fstat(int file, struct stat *st) {
  // st->st_mode = S_IFCHR;
  return 0;
}

int dandelion_stat(char *file, struct stat *st) {
  // st->st_mode = S_IFCHR;
  return 0;
}