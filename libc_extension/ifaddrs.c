#include <ifaddrs.h>

int getifaddrs (struct ifaddrs **__ifap) { return -1; }
void freeifaddrs (struct ifaddrs *__ifa) { return; }
