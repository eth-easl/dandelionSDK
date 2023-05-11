#include "../../system.h"
#include "syscall.h"

void __dandelion_platform_init(void) {}

void __dandelion_platform_exit(void) {
    __syscall(FREEBSD_SYS_exit, 0);
    __builtin_unreachable();
}

void __dandelion_platform_set_thread_pointer(void* ptr) {
    __syscall(FREEBSD_SYS_sysarch, FREEBSD_AMD64_GET_FSBASE, ptr);
}