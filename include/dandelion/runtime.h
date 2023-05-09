#ifndef _DANDELION_RUNTIME_H
#define _DANDELION_RUNTIME_H

#include <stddef.h>

#include "dandelion/io_buffer.h"

void dandelion_runtime_init(void);
_Noreturn void dandelion_runtime_exit(void);

void* dandelion_runtime_alloc(size_t size, size_t alignment);

struct io_buffer* dandelion_runtime_get_input(size_t set_idx, size_t buf_idx);
void dandelion_runtime_add_output(size_t set_idx, struct io_buffer* buf);

#endif
