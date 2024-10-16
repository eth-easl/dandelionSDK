#include "paths.h"
#include "include/fs_interface.h"

Path path_from_string(const char *const str) {
  Path path = {};
  size_t index = 0;
  while (str[index] != 0 && index < FS_PATH_LENGTH) {
    index++;
  }
  if (index >= FS_PATH_LENGTH) {
    dandelion_exit(ENAMETOOLONG);
    return path;
  }
  path.path = str;
  path.length = index;
  return path;
}

Path get_next_component(const Path path) {
  size_t index = 0;
  const char *new_path = path.path;
  // skip leading '/'
  while (index < path.length && new_path[index] == '/') {
    index++;
  }
  new_path += index;
  size_t new_length = path.length - index;
  index = 0;

  while (index < new_length && new_path[index] != '/') {
    index++;
  }

  Path return_path = {.path = new_path, .length = index};

  return return_path;
}

Path get_component_advance(Path *path) {
  Path new_component = get_next_component(*path);
  const char *new_end = new_component.path + new_component.length;
  path->length = path->path + path->length - new_end;
  path->path = new_component.path + new_component.length;
  return new_component;
}

Path get_directories(Path path) {
  // search for last component
  int last_slash = -1;
  for (size_t index = 0; index < path.length; index++) {
    if (path.path[index] == '/') {
      last_slash = index;
    }
  }
  last_slash = last_slash > 0 ? last_slash : 0;
  Path dir_path = {.path = path.path, .length = last_slash};
  return dir_path;
}

Path get_file(Path path) {
  size_t file_start = path.length - 1;
  // skip trailing '/'
  while (file_start > 0 && path.path[file_start] == '/') {
    file_start--;
  }
  // want to count backwards until we hit '/'
  while (file_start > 0 && path.path[file_start] != '/') {
    file_start--;
  }
  // if there was no slash we are fine, if there was one, overcounted by one
  file_start = path.path[file_start] == '/' ? file_start + 1 : file_start;
  Path file_path = {.path = path.path + file_start,
                    .length = path.length - file_start};
  return file_path;
}