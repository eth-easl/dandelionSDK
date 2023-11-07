#include "../../system.h"
#include <stddef.h>

// TODO probably want to define SDK heap size in user code eventually
const size_t  SDK_HEAP_SIZE = 1; // in wasm pages (one page is 64 KiB)

size_t __wasm_sdk_heap_size(void) {
    return SDK_HEAP_SIZE;
}

void __dandelion_platform_init(void) {}

void __dandelion_platform_exit(void) {}

void __dandelion_platform_set_thread_pointer(void* ptr) {}