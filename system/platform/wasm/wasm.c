#include "../../system.h"
#include <stddef.h>

// TODO: currently a hard-coded number of bytes for the interface memory
const size_t  WASM_SYSDATA_REGION_SIZE = 12345;  // 16 KiB
const char    WASM_SYSDATA_REGION[WASM_SYSDATA_REGION_SIZE] = {0};
const size_t  SDK_HEAP_SIZE = 1; // in wasm pages (one page is 64 KiB)

size_t __wasm_sysdata_region_size(void) {
    return WASM_SYSDATA_REGION_SIZE;
}

const char* __wasm_sysdata_region_base(void) {
    return WASM_SYSDATA_REGION;
}

size_t __wasm_sdk_heap_size(void) {
    return SDK_HEAP_SIZE;
}

void __dandelion_platform_init(void) {}

void __dandelion_platform_exit(void) {}

void __dandelion_platform_set_thread_pointer(void* ptr) {}