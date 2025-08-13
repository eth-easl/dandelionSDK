#include "file_system.h"
#include "include/fs_interface.h"
#include "paths.h"

#include <dandelion/runtime.h>
#include <dandelion/system/system.h>

#include <stddef.h>

extern D_File *fs_root;
extern OpenFile *open_files;

// Allocate new filesystem chunk, return NULL if ENOMEM;
// round up allocation to next multiple of FS_CHUNK_SIZE
FileChunk *allocate_file_chunk(size_t size) {
  size_t chunk_size =
      ((size + FS_CHUNK_SIZE - 1) / FS_CHUNK_SIZE) * FS_CHUNK_SIZE;
  char *new_buffer = dandelion_alloc(chunk_size, _Alignof(max_align_t));
  if (new_buffer == NULL) {
    return NULL;
  }
  FileChunk *new_chunck =
      dandelion_alloc(sizeof(FileChunk), _Alignof(FileChunk));
  if (new_chunck == NULL) {
    dandelion_free(new_buffer);
    return NULL;
  }
  new_chunck->capacity = chunk_size;
  new_chunck->data = new_buffer;
  new_chunck->used = 0;
  new_chunck->next = NULL;
  return new_chunck;
}

static inline void free_file_chunk(FileChunk *chunk) {
  dandelion_free(chunk->data);
  dandelion_free(chunk);
  return;
}

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

int dandelion_link(const char *old, const char *new_name) {
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

int dandelion_unlink(const char *name) {
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
    if (file_name.length >= FS_NAME_LENGTH) {
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
  if (open_error < 0)
    return open_error;

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

int dandelion_rmdir(const char *name) {
  // find parent folder
  D_File *current_file = find_file(name);
  if (current_file == NULL)
    return -ENOENT;
  if (current_file->type != DIRECTORY)
    return -ENOTDIR;
  if (current_file->child != NULL)
    return -ENOTEMPTY;
  return dandelion_unlink(name);
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

int64_t dandelion_lseek(int file, int64_t offset, int whence) {
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
  FileChunk *current;
  size_t chunk_offset = 0;
  // advance current reader to where seek wants to start
  switch (whence) {
  // set file offset to offset, if offset goes beyond fill gap with 0s
  // could optimize by having flag in chunk that marks as gap chunk
  case SEEK_SET:
    chunk_offset = 0;
    current = backing_file->content;
    total_offset = 0;
    break;
  case SEEK_CUR:
    // find current offset
    if (open_file->current_chunk == NULL) {
      current = backing_file->content;
    } else {
      current = open_file->current_chunk;
    }
    for (FileChunk *chunk = backing_file->content; chunk != NULL;
         chunk = chunk->next) {
      if (chunk != current) {
        total_offset += chunk->used;
      } else {
        chunk_offset = open_file->offset;
        total_offset += chunk_offset;
        break;
      }
    }
    break;
  case SEEK_END:
    current = backing_file->content;
    while (current != NULL) {
      total_offset += current->used;
      if (current->next == NULL) {
        break;
      } else {
        current = current->next;
      }
    }
    chunk_offset = current->used;
    break;
  default:
    return -EINVAL;
  }
  // current chunk is now set to the chunk we are supposed to offset from
  // if we can advance inside file do that
  // first handle negative offsets, as we never need to allocate for them
  if (offset < 0) {
    if (current == NULL || total_offset + offset < 0)
      return -EINVAL;
    // new offset that we need to go to
    total_offset += offset;
    // start at beginning and fint the total offset
    int needed_bytes = total_offset;
    current = backing_file->content;
    chunk_offset = 0;
    // know that all chunck need to exist to find the one we are looking for
    while (1) {
      if (current->used < needed_bytes) {
        needed_bytes -= current->used;
        current = current->next;
      } else {
        chunk_offset = needed_bytes;
        break;
      }
    }
    open_file->current_chunk = current;
    open_file->offset = chunk_offset;
    return total_offset;
  }

  if (current != NULL) {
    // check if we can advance in the current chunk, otherwise go to next
    size_t to_advance = current->used - chunk_offset;
    while (to_advance < offset && current->next != NULL) {
      offset -= to_advance;
      total_offset += to_advance;
      current = current->next;
      chunk_offset = 0;
      to_advance = current->used;
    }
    // either advancing inside chunk is enough or there is no next chunk to
    // advance to. That means we can take the smaller and advance by that much
    // in any case.
    to_advance = to_advance >= offset ? offset : to_advance;
    chunk_offset += to_advance;
    total_offset += to_advance;
    offset -= to_advance;
  }

  // if offset is zero we can return otherwise need to append that much to
  // current chunk
  if (offset == 0) {
    open_file->current_chunk = current;
    open_file->offset = chunk_offset;
    return total_offset;
  }
  FileChunk *new_chunk = allocate_file_chunk(offset);
  if (new_chunk == NULL) {
    return -ENOMEM;
  }
  new_chunk->used = offset;
  // have set current chunk in first phase to be up to date with backing file
  // if still NULL, that means backing file also was NULL
  if (current == NULL) {
    backing_file->content = new_chunk;
  } else {
    current->next = new_chunk;
  }
  open_file->current_chunk = new_chunk;
  open_file->offset = offset;

  return total_offset + offset;
}

size_t dandelion_read(int file, char *ptr, size_t len, int64_t offset,
                      char options) {

  // handle special case /dev/urandom URANDOM_FILENO use fake random data
  if (file == URANDOM_FILENO) {
    // generate fake random data
    for (size_t i = 0; i < len; i++) {
      ptr[i] = (offset + i) % 256;
    }
    return len;
  }

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
  FileChunk *current;
  size_t chunk_offset = 0;
  if (options & USE_OFFSET) {
    // find the chunk at the offset
    if (offset < 0)
      return -EINVAL;
    current = open_file->file->content;
    while (offset > 0) {
      if (current == NULL)
        return 0;
      if (current->used < offset) {
        offset -= current->used;
        current = current->next;
      } else {
        chunk_offset = offset;
        break;
      }
    }
  } else {
    // check if current chunk is set, and if not if the file has data to be read
    if (open_file->current_chunk == NULL) {
      // set to file chunk if there is one
      open_file->offset = 0;
      if (d_file->content != NULL)
        open_file->current_chunk = d_file->content;
      else
        return 0;
    }
    current = open_file->current_chunk;
    chunk_offset = open_file->offset;
  }

  int read_bytes = 0;
  int need_to_read = len;
  while (1) {
    size_t readable = current->used - chunk_offset;
    if (readable > need_to_read) {
      memcpy(ptr + read_bytes, current->data + chunk_offset, need_to_read);
      read_bytes += need_to_read;
      chunk_offset += need_to_read;
      break;
    } else {
      // read everythin in chunk and go to next
      memcpy(ptr + read_bytes, current->data + chunk_offset, readable);
      read_bytes += readable;
      // advance to next chunk if there is one, otherwise stay at this, so we
      // can see new appended chunks in the future
      if (current->next != NULL) {
        current = current->next;
        chunk_offset = 0;
      } else {
        chunk_offset += readable;
        break;
      }
    }
  }
  if (options & MOVE_OFFSET) {
    open_file->current_chunk = current;
    open_file->offset = chunk_offset;
  }
  return read_bytes;
}

size_t dandelion_write(int file, char *ptr, size_t len, int64_t offset,
                       char options) {
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
  FileChunk *current;
  size_t chunk_offset = 0;
  // check if file has content, if not allocate and skip to writing
  if (d_file->content == NULL) {
    size_t allocation_size = options & USE_OFFSET ? offset + len : len;
    FileChunk *new_chunk = allocate_file_chunk(allocation_size);
    d_file->content = new_chunk;
    current = new_chunk;
  } else {
    if (options & USE_OFFSET) {
      current = d_file->content;
      // know that the file content is not NULL
      while (offset > 0) {
        // if offset is within the current capacity make sure it is marked used
        // to at least the point in offset and set the writer there
        if (offset < current->capacity) {
          chunk_offset = offset;
          current->used = offset > current->used ? offset : current->used;
          break;
        }
        offset -= current->capacity;
        if (current->next == NULL) {
          FileChunk *new_chunk = allocate_file_chunk(offset + len);
          if (new_chunk == NULL)
            return -ENOMEM;
          current->next = new_chunk;
          current = new_chunk;
          new_chunk->used = offset;
          chunk_offset = offset;
          break;
        }
        current = current->next;
      }
    } else {
      // if have no chunck already check if file has a chunck and write to that
      // otherwise allocate one
      current = open_file->current_chunk;
      if (current == NULL) {
        if (d_file->content == NULL) {
          FileChunk *new_chunk = allocate_file_chunk(len);
          if (new_chunk == NULL)
            return -ENOMEM;
          d_file->content = new_chunk;
          current = new_chunk;
          chunk_offset = 0;
        } else {
          current = d_file->content;
          chunk_offset = 0;
        }
      }
    }
  }

  // check file mode, if writing a O_APPEND file, need to jump to end of file
  // before writing
  if (open_file->open_flags & O_APPEND && !(options & USE_OFFSET)) {
    // check that the current is at the end of what could be
    while (open_file->current_chunk->next != NULL) {
      open_file->current_chunk = open_file->current_chunk->next;
    }
    open_file->offset = open_file->current_chunk->used;
  }
  // know that we can start writing to end of current chunck

  size_t writen_bytes = 0;
  while (len > 0) {
    int writeable = current->capacity - current->used;
    if (writeable == 0) {
      FileChunk *new_chunk = allocate_file_chunk(len);
      current->next = new_chunk;
      current = new_chunk;
      writeable = new_chunk->capacity;
    }
    size_t to_write = MIN(len, writeable);
    memcpy(current->data + current->used, ptr, to_write);
    len -= to_write;
    ptr += to_write;
    current->used += to_write;
    writen_bytes += to_write;
  }

  if (options & MOVE_OFFSET) {
    open_file->current_chunk = current;
    open_file->offset = chunk_offset;
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
  st->blk_size = FS_CHUNK_SIZE;
  return 0;
}

int dandelion_fstat(int file, DandelionStat *st) {
  D_File *current_file = open_files[file].file;
  if (current_file == NULL) {
    return -EBADF;
  }
  return __dandelion_stat(current_file, st);
}

int dandelion_stat(const char *file, DandelionStat *st) {
  D_File *current_file = find_file(file);
  if (current_file == NULL) {
    return -ENOTDIR;
  }
  return __dandelion_stat(current_file, st);
}

static inline int __dandelion_truncate(D_File *file, int64_t length) {
  if (length < 0)
    return -EINVAL;
  if (file->type != FILE)
    return -EISDIR;
  int64_t remaining_size = length;
  FileChunk *current = file->content;
  // go to length, and cut off left over after that if there is any
  while (1) {
    // check if we reached the point to start cutting
    if (current->used >= remaining_size) {
      current->used = remaining_size;
      remaining_size = 0;
      // free any possible remaining chunks
      FileChunk *to_free = current->next;
      while (to_free != NULL) {
        FileChunk *free_chunk = to_free;
        to_free = to_free->next;
        free_file_chunk(free_chunk);
      }
      return 0;
    }
    remaining_size -= current->used;
    if (current->next == NULL)
      break;
    current = current->next;
  }
  // arriving here means that there is no next buffer
  // and need to append a 0 buffer
  FileChunk *new_chunk = allocate_file_chunk(remaining_size);
  current->next = new_chunk;
  new_chunk->used = remaining_size;
  return 0;
}

int dandelion_ftruncate(int fd, int64_t length) {
  D_File *current_file = open_files[fd].file;
  if (current_file == NULL)
    return -EBADF;
  if (!(open_files[fd].open_flags & O_WRONLY))
    return -EBADF;
  return __dandelion_truncate(current_file, length);
}

int dandelion_truncate(const char *path, int64_t length) {
  D_File *current_file = find_file(path);
  if (current_file == NULL)
    return -ENOENT;
  return __dandelion_truncate(current_file, length);
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
  size_t max_name_length = MIN(256, FS_NAME_LENGTH);
  memcpy(dirent->d_name, current_child->name, max_name_length - 1);
  dirent->d_name[max_name_length] = 0;
  dirent->d_ino = 0;
  dirent->d_off = index;
  dirent->d_type = current_child->type == FILE ? DT_REG : DT_DIR;

  return 0;
}

long int dandelion_telldir(DIR *dir) { return dir->child; }
void dandelion_seekdir(DIR *dir, long int index) { dir->child = index; }