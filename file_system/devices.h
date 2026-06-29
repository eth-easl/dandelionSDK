#ifndef __DANDELION_DEVICES__
#define __DANDELION_DEVICES__

#include <stdint.h>

// Define /dev/urandom
// Using the newlib random functions
extern void srand(unsigned int seed);
extern void srandom(unsigned int seed);
extern long random(void);
void urandom_init(int64_t seed) {
  // TODO also seed the other random functions from newlib
  srand(seed);
  srandom(seed);
};
size_t urandom_read(char *ptr, size_t len, int64_t offset, char options) {
  // ignoring offset and flags;
  size_t long_size = sizeof(long);
  size_t long_interations = len / long_size;

  long *long_ptr = (long *)ptr;
  for (size_t long_index = 0; long_index < long_interations; long_index++) {
    long_ptr[long_index] = random();
  }

  size_t long_offset = long_interations * long_size;
  long random_num = random();
  for (size_t char_index = 0; char_index < len - long_offset; char_index++) {
    ptr[long_offset + char_index] = (char)(random_num >> (8 * char_index));
  }
  return len;
};
size_t urandom_write(char *ptr, size_t len, int64_t offset, char options) {
  return 0;
};

// TODO: implement /dev/random with the arc4random, for better but slower
// randomness

#endif //__DANDELION_DEVICES__