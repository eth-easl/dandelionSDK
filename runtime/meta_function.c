#include "../include/dandelion/meta_function.h"

#include "../include/dandelion/runtime.h"

// size of an input_metadata item: number of items and total size, both as
// little endian u64
#define INPUT_METADATA_ITEM_SIZE 16

static int ident_equals(const char *ident, size_t ident_len, const char *name,
                        size_t name_len) {
  if (ident_len != name_len) {
    return 0;
  }
  for (size_t i = 0; i < name_len; i++) {
    if (ident[i] != name[i]) {
      return 0;
    }
  }
  return 1;
}

// finds the input set with the given name, returns -1 if there is none
static long find_input_set(const char *name, size_t name_len) {
  for (size_t i = 0; i < dandelion_input_set_count(); i++) {
    if (ident_equals(dandelion_input_set_ident(i),
                     dandelion_input_set_ident_len(i), name, name_len)) {
      return (long)i;
    }
  }
  return -1;
}

// finds the output set with the given name, returns -1 if there is none
static long find_output_set(const char *name, size_t name_len) {
  for (size_t i = 0; i < dandelion_output_set_count(); i++) {
    if (ident_equals(dandelion_output_set_ident(i),
                     dandelion_output_set_ident_len(i), name, name_len)) {
      return (long)i;
    }
  }
  return -1;
}

static long input_metadata_set(void) {
  return find_input_set(DANDELION_META_INPUT_METADATA_SET,
                        sizeof(DANDELION_META_INPUT_METADATA_SET) - 1);
}

static long output_metadata_set(void) {
  return find_input_set(DANDELION_META_OUTPUT_METADATA_SET,
                        sizeof(DANDELION_META_OUTPUT_METADATA_SET) - 1);
}

// reads a little endian u64, independent of alignment and host byte order
static size_t read_le_u64(const unsigned char *bytes) {
  size_t value = 0;
  for (size_t i = 0; i < 8; i++) {
    value |= (size_t)bytes[i] << (8 * i);
  }
  return value;
}

size_t dandelion_meta_input_count(void) {
  long set_idx = input_metadata_set();
  if (set_idx < 0) {
    return 0;
  }
  return dandelion_input_buffer_count((size_t)set_idx);
}

// get input sets metadata
int dandelion_meta_get_input(size_t idx, DandelionMetaInput *input) {
  long set_idx = input_metadata_set();
  if (set_idx < 0 || input == NULL) {
    return -1;
  }
  // items are in application order
  IoBuffer *item = dandelion_get_input((size_t)set_idx, idx);
  if (item == NULL || item->data_len != INPUT_METADATA_ITEM_SIZE) {
    return -1;
  }
  const unsigned char *data = item->data;
  input->ident = item->ident;
  input->ident_len = item->ident_len;
  input->num_items = read_le_u64(data);
  input->total_size = read_le_u64(data + 8);
  return 0;
}

size_t dandelion_meta_output_count(void) {
  long set_idx = output_metadata_set();
  if (set_idx < 0) {
    return 0;
  }
  return dandelion_input_buffer_count((size_t)set_idx);
}

// get output set metadata
int dandelion_meta_get_output(size_t idx, DandelionMetaOutput *output) {
  long set_idx = output_metadata_set();
  if (set_idx < 0 || output == NULL) {
    return -1;
  }
  // items are in application order
  IoBuffer *item = dandelion_get_input((size_t)set_idx, idx);
  if (item == NULL) {
    return -1;
  }
  output->ident = item->ident;
  output->ident_len = item->ident_len;
  return 0;
}

// set the planned composition (as string) by adding the composition string to the output set
int dandelion_meta_set_plan(const char *composition, size_t composition_len) {
  long set_idx = find_output_set(DANDELION_META_COMPOSITION_SET,
                                 sizeof(DANDELION_META_COMPOSITION_SET) - 1);
  // the platform takes exactly one item from the composition set
  if (set_idx < 0 || dandelion_output_buffer_count((size_t)set_idx) != 0) {
    return -1;
  }
  // outputs are read after the function exits, so the source needs to be in
  // memory that outlives the caller
  char *copy = NULL;
  if (composition_len > 0) {
    copy = dandelion_alloc(composition_len, 1);
    if (copy == NULL) {
      return -1;
    }
    for (size_t i = 0; i < composition_len; i++) {
      copy[i] = composition[i];
    }
  }
  IoBuffer plan = {
      .ident = DANDELION_META_COMPOSITION_SET,
      .ident_len = sizeof(DANDELION_META_COMPOSITION_SET) - 1,
      .data = copy,
      .data_len = composition_len,
      .key = 0,
  };
  dandelion_add_output((size_t)set_idx, plan);
  return 0;
}
