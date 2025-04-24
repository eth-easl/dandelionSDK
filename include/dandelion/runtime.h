#ifndef _DANDELION_RUNTIME_H
#define _DANDELION_RUNTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "io_buffer.h"

void dandelion_init(void);
void dandelion_exit(int exit_code);

void dandelion_set_thread_pointer(void *ptr);
void *dandelion_sbrk(size_t size);
void *dandelion_alloc(size_t size, size_t alignment);
void dandelion_free(void *free_ptr);

size_t dandelion_input_set_count(void);
size_t dandelion_output_set_count(void);

size_t dandelion_input_buffer_count(size_t set_idx);
size_t dandelion_output_buffer_count(size_t set_idx);

const char *dandelion_input_set_ident(size_t set_idx);
size_t dandelion_input_set_ident_len(size_t set_idx);
const char *dandelion_output_set_ident(size_t set_idx);
size_t dandelion_output_set_ident_len(size_t set_idx);

IoBuffer *dandelion_get_input(size_t set_idx, size_t buf_idx);
void dandelion_add_output(size_t set_idx, IoBuffer buf);

#ifdef __cplusplus
}
#endif

#endif
