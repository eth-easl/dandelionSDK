#ifndef _DANDELION_SYSTEM_H
#define _DANDELION_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "dandelion/io_buffer.h"

/*
    This file contains the interface between the platform and the runtime.
    It is not necessarily intented to be consumed directly by the user. Instead,
    the user should use the runtime API, which is defined in runtime.h.
    However, this interface is what the platform must implement.
*/

// Information about an input/output set
struct io_set_info {
  // Name of the set (not necessarily null-terminated)
  const char *ident;
  // Length of the name of the set
  size_t ident_len;

  // Offset of the first buffer in the set in the list of all buffers
  // NOTE: effectively, the number of buffers in set i is set[i + 1].offset -
  // set[i].offset This is also the reason why the last set is a sentinel set;
  // because it has an offset one past the end of the last input buffer,
  // allowing us to find the number of buffers in the last set.
  size_t offset;
};

void __dandelion_system_init(void);
void __dandelion_system_exit(void);
void __dandelion_system_set_thread_pointer(void *ptr);

// Normal exit codes are reserved up to 255 so we can use the space above
#define DANDELION_EXIT_UNINITIALIZED                                           \
  256 // the exit code when the struct was never touched
#define DANDELION_ALLOC_OVERLAP                                                \
  257 // there was an error in dandelion alloc where allocations overlap
#define DANDELION_ALLOC_FREE_UNOCCUPIED                                        \
  258                     // free was called but the index showed no occupation
#define DANDELION_OOM 259 // Ran out of memory for critical operation

struct dandelion_system_data {
  // Exit code of the process, set by the runtime at exit
  int exit_code;

  // Heap bounds, initialized by platform before entry
  size_t heap_begin;
  size_t heap_end;

  // Number of input sets (excluding the sentinel set).
  // Initialized by the platform before entry.
  size_t input_sets_len;
  // Input sets, initialized by the platform before entry.
  // The last set is a sentinel set, which has an `offset`
  // one past the end of the last input buffer.
  struct io_set_info *input_sets;

  // Number of output sets (excluding the sentinel set).
  // Initialized by the platform before entry
  size_t output_sets_len;
  // Same as input sets, but for output sets
  struct io_set_info *output_sets;

  // Input buffers, initialized by the platform before entry
  IoBuffer *input_bufs;
  // Output buffers, set by the runtime at exit
  IoBuffer *output_bufs;
};

// Global symbol available to the platform
extern struct dandelion_system_data __dandelion_system_data;

#ifdef FREESTANDING
// definitions of freestanding functions
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *src1, const void *src2, size_t n);
#endif // FREESTANDING

#ifdef __cplusplus
}
#endif

#endif // DANDELION_SYSTEM_H