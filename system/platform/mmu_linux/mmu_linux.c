#include "../../system.h"
#include "syscall.h"

void __dandelion_platform_init(void) {}

void __dandelion_platform_exit(void) {
    __syscall(SYS_exit_group, 0);
    __builtin_unreachable();
}