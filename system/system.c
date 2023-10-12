#include "dandelion/system/system.h"
#include "system.h"

struct dandelion_system_data __dandelion_system_data;

void __dandelion_system_init(void) {
    __dandelion_platform_init();

}

void __dandelion_system_exit(void) {

    __dandelion_platform_exit();
}