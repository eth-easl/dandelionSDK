#include "../../system.h"
#include <stddef.h>

const void * __capability __dandelion_return_address;

void __dandelion_platform_init(void) {}

void __dandelion_platform_exit(void) {
	__asm__ volatile(
		"ldr c0, [%0] \n"
		"ldpbr c29, [c0] \n"
		: : "r" (&__dandelion_return_address) : "c0"
	);
    __builtin_unreachable();
}

void __dandelion_platform_set_thread_pointer(void* ptr) {
	size_t thread_data = (size_t)ptr;
	asm volatile ("msr tpidr_el0, %0" :: "r"(thread_data));
}