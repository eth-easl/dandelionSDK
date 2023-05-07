#include <stdlib.h>
#include <string.h>

#include "../include/dandelion.h"

#define dandelion __dandelion_global_data

// does some computation and returns an owning pointer to the result
int compute(void* data, size_t size, void** out_data, size_t* out_size) {
    *out_data = malloc(size);
    memcpy(*out_data, data, size);
    *out_size = size;
    return 0;
}

int main() {
    // check that sets are long enough
    if (dandelion.input_sets_len < 1 || dandelion.input_sets[0].buffers_len < 1) {
        dandelion.exit_code = 1;
        return 1;
    }
    if (dandelion.output_sets_len < 1) {
        dandelion.exit_code = 1;
        return 1;
    }

    struct io_buffer* input_buf = &dandelion.input_sets[0].buffers[0];

    // create an output buffer in set 0
    dandelion.output_sets[0].buffers = malloc(sizeof(struct io_buffer));
    dandelion.output_sets[0].buffers_len = 1;
    struct io_buffer* output_buf = &dandelion.output_sets[0].buffers[0];
    memset(output_buf, 0, sizeof(struct io_buffer));

    compute(input_buf->data, input_buf->data_len, &output_buf->data, &output_buf->data_len);

    dandelion.exit_code = 0;
    return 0;
}