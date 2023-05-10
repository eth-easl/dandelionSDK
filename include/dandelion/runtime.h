#ifndef _DANDELION_RUNTIME_H
#define _DANDELION_RUNTIME_H

#include <stddef.h>

#include "dandelion/io_buffer.h"

void dandelion_init(void);
_Noreturn void dandelion_exit(int exit_code);

void dandelion_set_thread_pointer(void* ptr);
void* dandelion_alloc(size_t size, size_t alignment);

size_t dandelion_get_input_set_count(void);
size_t dandelion_get_output_set_count(void);
size_t dandelion_input_get_buffer_count(size_t set_idx);
struct io_buffer* dandelion_get_input(size_t set_idx, size_t buf_idx);
void dandelion_add_output(size_t set_idx, struct io_buffer* buf);

#endif
