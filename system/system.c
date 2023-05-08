#include <dandelion/system/system.h>

void __dandelion_system_init(void) {
    __dandelion_system_platform_init();

}

_Noreturn void __dandelion_system_exit(void) {

    __dandelion_system_platform_exit();
}