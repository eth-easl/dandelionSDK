#pragma once

#define sysdata __dandelion_system_data

void __dandelion_platform_init(void);
_Noreturn void __dandelion_platform_exit(void);
void __dandelion_platform_set_thread_pointer(void* ptr);
