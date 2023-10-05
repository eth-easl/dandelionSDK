#pragma once

#include <stddef.h>

#define sysdata __dandelion_system_data

void __dandelion_platform_init(void);
void __dandelion_platform_exit(void);
void __dandelion_platform_set_thread_pointer(void* ptr);

// definitions of freestanding functions
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int c, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* src1, const void* src2, size_t n);