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