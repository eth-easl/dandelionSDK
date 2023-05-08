#ifndef DANDELION_SYSTEM_H
#define DANDELION_SYSTEM_H

#include <stddef.h>
#include <stdint.h>

struct dandelion_io_buffer {
	const char* ident;
	size_t ident_len;

	void* data;
	size_t data_len;
};

void __dandelion_system_init(void);
void __dandelion_system_platform_init(void);
_Noreturn void __dandelion_system_exit(void);
_Noreturn void __dandelion_system_platform_exit(void);

struct dandelion_system_data {
	int exit_code;

	uintptr_t heap_begin;
	uintptr_t heap_end;

};

extern struct dandelion_system_data __dandelion_system_data;

#endif // DANDELION_SYSTEM_H