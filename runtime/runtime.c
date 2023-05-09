#include <dandelion/runtime.h>
#include <dandelion/system/system.h>

#include "runtime.h"

#define sysdata __dandelion_system_data

struct runtime_data __runtime_global_data;

void dandelion_runtime_init(void) {
    __dandelion_system_init();

    // parse raw input data into tree structure
    rtdata.input_sets = dandelion_runtime_alloc(sysdata.input_sets_len * sizeof(struct io_set), _Alignof(struct io_set));
    for (size_t i = 0; i < sysdata.input_sets_len; ++i) {
        struct io_set* set = &rtdata.input_sets[i];
        size_t num_bufs = sysdata.input_sets[i + 1].offset - sysdata.input_sets[i].offset;
        set->ident = sysdata.input_sets[i].ident;
        set->ident_len = sysdata.input_sets[i].ident_len;
        set->buffers = dandelion_runtime_alloc(num_bufs * sizeof(struct io_buffer), _Alignof(struct io_buffer));
        set->buffers_len = num_bufs;
        set->buffers_cap = num_bufs;
        for (size_t j = 0; j < num_bufs; ++j) {
            size_t buf_offset = sysdata.input_sets[i].offset + j;
            set->buffers[j].ident = sysdata.input_bufs[buf_offset].ident;
            set->buffers[j].ident_len = sysdata.input_bufs[buf_offset].ident_len;
            set->buffers[j].data = sysdata.input_bufs[buf_offset].data;
            set->buffers[j].data_len = sysdata.input_bufs[buf_offset].data_len;
        }
    }

    rtdata.output_sets = dandelion_runtime_alloc(sysdata.output_sets_len * sizeof(struct io_set), _Alignof(struct io_set));
    for (size_t i = 0; i < sysdata.output_sets_len; ++i) {
        rtdata.output_sets[i].ident = sysdata.output_sets[i].ident;
        rtdata.output_sets[i].ident_len = sysdata.output_sets[i].ident_len;
        rtdata.output_sets[i].buffers = NULL;
        rtdata.output_sets[i].buffers_len = 0;
        rtdata.output_sets[i].buffers_cap = 0;
    }
}

_Noreturn void dandelion_runtime_exit(void) {
    // convert tree structure into raw output data
    size_t num_output_bufs = 0;
    for (size_t i = 0; i < sysdata.output_sets_len; ++i) {
        num_output_bufs += rtdata.output_sets[i].buffers_len;
    }

    sysdata.output_bufs = dandelion_runtime_alloc(num_output_bufs * sizeof(struct io_buffer), _Alignof(struct io_buffer));

    size_t current_offset = 0;
    for (size_t i = 0; i < sysdata.output_sets_len; ++i) {
        sysdata.output_sets[i].offset = current_offset;

        struct io_set* tree_set = &rtdata.output_sets[i];
        current_offset += tree_set->buffers_len;
        for (size_t j = 0; j < tree_set->buffers_len; ++j) {
            struct io_buffer* tree_buf = &tree_set->buffers[j];
            struct io_buffer* raw_buf = &sysdata.output_bufs[sysdata.output_sets[i].offset + j];
            raw_buf->ident = tree_buf->ident;
            raw_buf->ident_len = tree_buf->ident_len;
            raw_buf->data = tree_buf->data;
            raw_buf->data_len = tree_buf->data_len;
        }
    }

    __dandelion_system_exit();
}

void* dandelion_runtime_alloc(size_t size, size_t alignment) {
	static uintptr_t alloc_base = 0;
    if (alloc_base == 0) {
        alloc_base = sysdata.heap_begin;
    }
    alloc_base = ((alloc_base - 1) | (alignment - 1)) + 1;
	if (alloc_base + size > sysdata.heap_end) {
		return NULL;
	}
	alloc_base += size;
	return (void*)alloc_base;
}
