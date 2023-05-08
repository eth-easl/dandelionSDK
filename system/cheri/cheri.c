#include <dandelion/system/system.h>


struct dandelion_system_data __dandelion_system_data;
const void * __capability __dandelion_return_address;


void __dandelion_system_platform_init(void) {}

_Noreturn void __dandelion_system_platform_exit(void) {
	__asm__ volatile(
		"ldr c0, [%0] \n"
		"ldpbr c29, [c0] \n"
		: : "r" (&__dandelion_return_address) : "c0"
	);
    __builtin_unreachable();
}
