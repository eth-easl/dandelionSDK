#include "../../system.h"
#include "syscall.h"

void __dandelion_platform_init(void) {}

void __dandelion_platform_exit(void) {
    __syscall(FREEBSD_SYS_exit, 0);
    __builtin_unreachable();
}

void __dandelion_platform_set_thread_pointer(void* ptr) {
#if defined(__x86_64__)
    __syscall(FREEBSD_SYS_sysarch, FREEBSD_AMD64_GET_FSBASE, ptr);
#elif defined(__aarch64__)
    size_t thread_data = (size_t)ptr;
    asm volatile("msr tpidr_el0, %0" :: "r"(thread_data));
#else
#error "Missing architecture specific code."
#endif
}