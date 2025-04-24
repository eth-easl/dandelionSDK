#include "dandelion/runtime.h"

static void my_memcpy(void *dst, const void *src, size_t size) {
  char *dst_c = dst;
  const char *src_c = src;
  for (size_t i = 0; i < size; i++) {
    dst_c[i] = src_c[i];
  }
}

// does some computation and returns an owning pointer to the result
int compute(void *data, size_t size, void **out_data, size_t *out_size) {
  *out_data = dandelion_alloc(size, 1);
  my_memcpy(*out_data, data, size);
  *out_size = size;
  return 0;
}

void _start() {
  dandelion_init();

  IoBuffer *input_buf = dandelion_get_input(0, 0);
  IoBuffer output_buf = {"output_file", sizeof("output_file") - 1, NULL, 0};
  compute(input_buf->data, input_buf->data_len, &output_buf.data,
          &output_buf.data_len);
  dandelion_add_output(0, output_buf);

  dandelion_exit(0);
}
