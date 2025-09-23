#ifndef __DANDELION_PATHS__
#define __DANDELION_PATHS__

#ifndef FS_PATH_LENGTH
#define FS_PATH_LENGTH 4096
#endif

#include <stddef.h>

typedef struct Path {
  const char *path;
  size_t length;
} Path;

// implement these to make sure this can be build independenlty of any string.h

static inline size_t namelen(const char *const name, size_t max_len) {
  int length = 0;
  while (length < max_len && name[length] != '\0') {
    length++;
  }
  // need to count the last char
  return length;
}

static inline int namecmp(const char *const name1, size_t name1_length, const char *const name2,
                          size_t name2_length) {
  size_t max_length = name1_length < name2_length ? name1_length : name2_length;
  for (size_t index = 0; index < max_length; index++) {
    // this also automatically returns -1 if one of them is null terminated earlier than their length
    if (name1[index] != name2[index])
      return name1[index] < name2[index] ? -1 : 1;
    // are the same if we got here
    if (name1[index] == '\0')
      return 0;
  }
  // they are the same until the end of the shorter one or there has not been null termination
  if(name1_length < name2_length)
    return -1;
  else if(name2_length < name1_length)
    return 1;
  else 
    return 0;
}

Path path_from_string(const char *const str);

Path get_next_component(const Path path);

Path get_component_advance(Path *path);

Path get_directories(Path path);

Path get_file(Path path);

#endif // __DANDELION_PATHS__