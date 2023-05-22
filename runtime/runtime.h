#pragma once

#include <stddef.h>

#include "dandelion/system/system.h"

struct io_set {
	const char* ident;
	size_t ident_len;

	struct io_buffer* buffers;
	size_t buffers_len;
    size_t buffers_cap;
};

struct runtime_data {
	struct io_set* input_sets;
	struct io_set* output_sets;
};

extern struct runtime_data __runtime_global_data;
#define rtdata __runtime_global_data
