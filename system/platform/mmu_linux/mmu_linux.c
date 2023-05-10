#include "../../system.h"
#include "syscall.h"

void __dandelion_platform_init(void) {}

_Noreturn void __dandelion_platform_exit(void) {
    __syscall(SYS_exit_group, 0);
    __builtin_unreachable();
}

void __dandelion_platform_set_thread_pointer(void* ptr) {
    __syscall(SYS_arch_prctl, ARCH_SET_FS, ptr);
}