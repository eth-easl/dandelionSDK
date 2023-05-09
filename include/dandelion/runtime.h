#ifndef DANDELION_RUNTIME_H
#define DANDELION_RUNTIME_H

#include "stddef.h"

void dandelion_runtime_init(void);
_Noreturn void dandelion_runtime_exit(void);

void* dandelion_runtime_alloc(size_t size, size_t alignment);

#endif
