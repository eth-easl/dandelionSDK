#ifndef MLIBC_DANDELION_H
#define MLIBC_DANDELION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct io_buffer {
	const char* ident;
	size_t ident_len;

	void* data;
	size_t data_len;
};

struct io_set {
	const char* ident;
	size_t ident_len;

	struct io_buffer* buffers;
	size_t buffers_len;
};

struct dandelion_data {
	int exit_code;

	uintptr_t heap_begin;
	uintptr_t heap_end;

	struct io_buffer stdin;
	struct io_buffer stdout;
	struct io_buffer stderr;

	struct io_set* input_sets;
	size_t input_sets_len;

	struct io_set* output_sets;
	size_t output_sets_len;
};

extern struct dandelion_data __dandelion_global_data;

#ifdef __cplusplus
}; // extern "C"
#endif

#endif
