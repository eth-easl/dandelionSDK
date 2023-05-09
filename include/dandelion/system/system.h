#ifndef DANDELION_SYSTEM_H
#define DANDELION_SYSTEM_H

#include <stddef.h>
#include <stdint.h>

struct io_buffer {
	const char* ident;
	size_t ident_len;

	void* data;
	size_t data_len;
};

struct io_set_info {
    const char* ident;
    size_t ident_len;

    size_t offset;
};

void __dandelion_system_init(void);
_Noreturn void __dandelion_system_exit(void);

struct dandelion_system_data {
	int exit_code;

	uintptr_t heap_begin;
	uintptr_t heap_end;

    size_t input_sets_len;
    struct io_set_info* input_sets;

    size_t output_sets_len;
    struct io_set_info* output_sets;

    struct io_buffer* input_bufs;
    struct io_buffer* output_bufs;
};

extern struct dandelion_system_data __dandelion_system_data;

#endif // DANDELION_SYSTEM_H