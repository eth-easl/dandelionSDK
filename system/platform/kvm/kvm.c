#include "../../system.h"

void __dandelion_platform_init(void) {}

void __dandelion_platform_exit(void) {
#if defined(__x86_64__)
    asm volatile("hlt");
#elif defined(__aarch64__)
    asm volatile("ldr x0, =0x84000008\n\t"
                 "hvc #0");
#else
#error "Missing architecture specific code."
#endif
    __builtin_unreachable();
}

void __dandelion_platform_set_thread_pointer(void *ptr) {
    size_t thread_data = (size_t)ptr;
#if defined(__x86_64__)
    // TODO: use wrmsr for older CPUs that don't support FSGSBASE
    asm volatile("wrfsbase %0" :: "r"(thread_data) : "memory");
#elif defined(__aarch64__)
    asm volatile("msr tpidr_el0, %0" :: "r"(thread_data));
#else
#error "Missing architecture specific code."
#endif
}