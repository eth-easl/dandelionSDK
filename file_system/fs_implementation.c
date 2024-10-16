#include "file_system.h"
#include "include/fs_interface.h"
#include "paths.h"

#include <dandelion/system/system.h>
#include <dandelion/runtime.h>

#include <stddef.h>

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
    return -ENOTDIR;
  }
  // check that the old file is not a directory
  if (file->type == DIRECTORY) {
    return -EPERM;
  }
  // check we could add a hard link
  if (file->hard_links == (unsigned short)-1) {
    return -EMLINK;
  }
  D_File *new_file = find_file(new_name);
  if (new_file != NULL) {
    return -EEXIST;
  }
  // create necessary folders on the way
  Path new_path = path_from_string(new_name);
  Path new_file_name = get_file(new_path);
  Path new_file_dir = get_directories(new_path);
  D_File *new_dir = create_directories(fs_root, new_file_dir, 0);
  if (new_dir == NULL) {
    return -ENOTDIR;
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
    return -ENOTDIR;
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
int dandelion_open(const char *name, int flags, uint32_t mode) {
  // check if file is already open, if so fail O_TRUNC with EACCESS
  // get path from name
  D_File *current = find_file(name);
  // report error if O_EXCL is set and file exists
  if (flags & O_EXCL && flags & O_CREAT && current != NULL) {
    return -EEXIST;
  }
  // handle if file was not found
  if (current == NULL) {
    // if flag to create was not set return error
    if (!(flags & O_CREAT)) {
      return -ENOENT;
    }
    // should create a file
    // need to create another copy as the original was modified in the loop
    Path total_path = path_from_string(name);
    Path dir_path = get_directories(total_path);
    Path file_name = get_file(total_path);
    if (file_name.length >= FS_NAME_LENGHT) {
      return -EINVAL;
    }
    D_File *parent = create_directories(fs_root, dir_path, 0);
    if (parent == NULL) {
      return -ENOTDIR;
    }
    current = create_file(&file_name, NULL, 0, 0);
    if (current == NULL) {
      return -ENOMEM;
    }
    current->mode = mode;
    if (link_file_to_folder(parent, current)) {
      return -ENOTDIR;
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
    return -EMFILE;
  }

  int open_error = open_existing_file(file_descriptor, current, flags, mode, 1);
  if (open_error < 0) return open_error;

  current->open_descripotors += 1;
  return file_descriptor;
}

int dandelion_mkdir(const char *name, uint32_t mode) {
  // find parent folder
  Path dir_path = path_from_string(name);
  Path parent_dir = get_directories(dir_path);
  Path dir_name = get_file(dir_path);
  D_File *parent_file = find_file_path(parent_dir);
  if (parent_file == NULL) {
    return -ENOENT;
  }
  if (parent_file->type != DIRECTORY) {
    return -ENOTDIR;
  }
  D_File *existing = find_file_in_dir(parent_file, dir_name);
  if (existing != NULL) {
    return -EEXIST;
  }
  D_File *new_dir = create_directories(parent_file, dir_name, 0);
  if (new_dir == NULL) {
    return -ENOMEM;
  }
  new_dir->mode = mode;
  return 0;
}

int dandelion_close(int file) {
  // check the file is open
  D_File *to_close = open_files[file].file;
  if (to_close == NULL) {
    return -EBADF;
  }
  to_close->open_descripotors -= 1;
  free_data(to_close);
  open_files[file].file = NULL;
  return 0;
}

int dandelion_lseek(int file, int offset, int whence) {
  int return_val = 0;
  int total_offset = 0;
  OpenFile *open_file = &open_files[file];
  D_File *backing_file = open_file->file;
  if (backing_file == NULL || backing_file->type != FILE) {
    return -EBADF;
  }
  // perform seek in phases
  // advance current reader to where the seek is supposed to start from
  // advancing in the existing file
  // appending 0ed gap at the end

  // advance current reader to where seek wants to start
  switch (whence) {
  // set file offset to offset, if offset goes beyond fill gap with 0s
  // could optimize by having flag in chunk that marks as gap chunk
  case SEEK_SET:
    open_file->offset = 0;
    open_file->current_chunk = backing_file->content;
    total_offset = 0;
    break;
  case SEEK_CUR:
    // find current offset
    if (open_file->current_chunk == NULL) {
      open_file->current_chunk = backing_file->content;
      open_file->offset = 0;
    }
    for (FileChunk *chunk = backing_file->content; chunk != NULL;
         chunk = chunk->next) {
      if (chunk != open_file->current_chunk) {
        total_offset += chunk->used;
      } else {
        total_offset += open_file->offset;
        break;
      }
    }
    break;
  case SEEK_END:; // need a statement after label
    FileChunk *chunk = backing_file->content;
    while (chunk != NULL) {
      total_offset += chunk->used;
      if (chunk->next == NULL) {
        break;
      } else {
        chunk = chunk->next;
      }
    }
    open_file->current_chunk = chunk;
    open_file->offset = chunk->used;
    break;
  default:
    return -EINVAL;
  }
  // current chunk is now set to the chunk we are supposed to append to
  // if we can advance inside file do that
  if (open_file->current_chunk != NULL) {
    // check if we can advance in the current chunk, otherwise go to next
    size_t to_advance = open_file->current_chunk->used - open_file->offset;
    while (to_advance < offset && open_file->current_chunk->next != NULL) {
      offset -= to_advance;
      total_offset += to_advance;
      open_file->current_chunk = open_file->current_chunk->next;
      open_file->offset = 0;
      to_advance = open_file->current_chunk->used;
    }
    // either advancing inside chunk is enough or there is no next chunk to
    // advance to. That means we can take the smaller and advance by that much
    // in any case.
    to_advance = to_advance >= offset ? offset : to_advance;
    open_file->offset += to_advance;
    total_offset += to_advance;
    offset -= to_advance;
  }

  // if offset is zero we can return otherwise need to append that much to
  // current chunk
  if (offset == 0) {
    return total_offset;
  }
  FileChunk *new_chunk =
      dandelion_alloc(sizeof(FileChunk), _Alignof(FileChunk));
  if (new_chunk == NULL) {
    return -ENOMEM;
  }
  // round allocation size to next multiple of usual block size
  size_t allocation_size =
      ((offset + FS_CHUNCK_SIZE - 1) / FS_CHUNCK_SIZE) * FS_CHUNCK_SIZE;
  new_chunk->data = dandelion_alloc(allocation_size, _Alignof(max_align_t));
  if (new_chunk->data == NULL) {
    return -ENOMEM;
  }
  new_chunk->capacity = allocation_size;
  new_chunk->used = offset;
  new_chunk->next = NULL;
  // have set current chunk in first phase to be up to date with backing file
  // if still NULL, that means backing file also was NULL
  if (open_file->current_chunk == NULL) {
    backing_file->content = new_chunk;
  } else {
    open_file->current_chunk->next = new_chunk;
  }
  open_file->current_chunk = new_chunk;
  open_file->offset = offset;

  return total_offset + offset;
}

int dandelion_read(int file, char *ptr, int len) {
  // get the file descriptor
  OpenFile *open_file = &open_files[file];
  // check there is a valid file descriptor there and that it is writable
  if (open_file->file == NULL || open_file->open_flags & O_WRONLY) {
    return -EBADF;
  }
  if (open_file->file->type != FILE) {
    return -EINVAL;
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
    return -EBADF;
  }
  if (open_file->file->type != FILE) {
    return -EINVAL;
  }

  D_File *d_file = open_file->file;
  // if have no chunck already check if file has a chunck and write to that
  // otherwise allocate one
  if (open_file->current_chunk == NULL) {
    if (d_file->content == NULL) {
      char *new_buffer = dandelion_alloc(FS_CHUNCK_SIZE, _Alignof(max_align_t));
      if (new_buffer == NULL) {
        return -ENOMEM;
      }
      FileChunk *new_chunck =
          dandelion_alloc(sizeof(FileChunk), _Alignof(FileChunk));
      if (new_chunck == NULL) {
        return -ENOMEM;
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
        return -ENOMEM;
      }
      char *allocation = dandelion_alloc(FS_CHUNCK_SIZE, _Alignof(max_align_t));
      if (allocation == NULL) {
        return -ENOMEM;
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
    memcpy(open_file->current_chunk->data + open_file->current_chunk->used, ptr, to_write);
    len -= to_write;
    ptr += to_write;
    open_file->current_chunk->used += to_write;
    writen_bytes += to_write;
  }

  return writen_bytes;
}

static inline int __dandelion_stat(D_File *file, DandelionStat *st) {
  // assume file is non null, caller is supposed to check that
  st->st_mode = file->mode;
  st->hard_links = file->hard_links;
  size_t total_size = 0;
  if (file->type == FILE) {
    for (FileChunk *current = file->content; current != 0;
         current = current->next) {
      total_size += current->used;
    }
  }
  st->file_size = total_size;
  st->blk_size = FS_CHUNCK_SIZE;
  return 0;
}

int dandelion_fstat(int file, DandelionStat *st) {
  D_File *current_file = open_files[file].file;
  if (current_file == NULL) {
    return -EBADF;
  }
  return __dandelion_stat(current_file, st);
}

int dandelion_stat(char *file, DandelionStat *st) {
  D_File *current_file = find_file(file);
  if (current_file == NULL) {
    return -ENOTDIR;
  }
  return __dandelion_stat(current_file, st);
}
// ========================================================
// dirent functions
// ========================================================

int dandelion_opendir(const char *name, DIR *dir) {
  D_File *current_dir = find_file(name);
  if (current_dir == NULL) {
    return -ENOENT;
  }
  if (current_dir->type != DIRECTORY) {
    return -ENOTDIR;
  }
  // allocate and populate open dir struct
  dir->dir = current_dir;
  dir->child = 0;

  // increase to make sure it does not get dealloced
  current_dir->open_descripotors += 1;
  return 0;
}

int dandelion_closedir(DIR *dir) {
  dir->dir -= 1;
  int err = free_data(dir->dir);
  return err;
}

int dandelion_readdir(DIR *directory, struct dirent *dirent) {
  D_File *current_child = directory->dir->child;
  size_t index = 0;
  while (current_child != NULL && index < directory->child) {
    current_child = current_child->next;
    index++;
  }
  // either have reached index == direcotry->child or current_child is NULL
  if (current_child == NULL) {
    if (index != directory->child) {
      return -ENOENT;
    }
    return -1;
  }
  directory->child++;
  size_t max_name_length = MIN(256, FS_NAME_LENGHT);
  memcpy(dirent->d_name, current_child->name, max_name_length-1);
  dirent->d_name[max_name_length] = 0;
  dirent->d_ino = 0;
  dirent->d_off = index;
  dirent->d_type = current_child->type == FILE ? DT_REG : DT_DIR;

  return 0;
}

long int dandelion_telldir(DIR *dir) { return dir->child; }
void dandelion_seekdir(DIR *dir, long int index) { dir->child = index; }