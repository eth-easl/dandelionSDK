#include "dandelion/system/system.h"

#include "system.h"

struct dandelion_system_data __dandelion_system_data = {
    .exit_code = 256,  // one above any expected exit code
    .heap_begin = -1,
    .heap_end = 0,
    .input_sets_len = -1,
    .input_sets = NULL,
    .output_sets_len = -1,
    .output_sets = NULL,
    .input_bufs = NULL,
    .output_bufs = NULL};

void __dandelion_system_init(void) { __dandelion_platform_init(); }

void __dandelion_system_exit(void) { __dandelion_platform_exit(); }

void __dandelion_system_set_thread_pointer(void *ptr) {
  return __dandelion_platform_set_thread_pointer(ptr);
}