#ifndef _DANDELION_IO_BUFFER_H
#define _DANDELION_IO_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct IoBuffer {
  const char *ident;
  size_t ident_len;

  void *data;
  size_t data_len;

  size_t key;
} IoBuffer;

#ifdef __cplusplus
}
#endif

#endif // _DANDELION_IO_H