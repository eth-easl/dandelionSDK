#pragma once

#include <stddef.h>

#include "dandelion/system/system.h"

typedef struct IoSet {
  const char *ident;
  size_t ident_len;

  struct IoBuffer *buffers;
  size_t buffers_len;
  size_t buffers_cap;
} IoSet;

typedef struct RuntimeData {
  IoSet *input_sets;
  IoSet *output_sets;
} RuntimeData;

extern RuntimeData __runtime_global_data;
#define rtdata __runtime_global_data
