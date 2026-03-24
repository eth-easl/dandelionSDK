#include <arpa/inet.h>

in_addr_t inet_addr (const char *__cp) {
    return -1;
}
char *inet_ntoa (struct in_addr __in) {
    return 0;
}
const char *inet_ntop (int __af, const void *restrict __cp, char *restrict __buf, socklen_t __len) {
    return 0;
}
int inet_pton (int __af, const char *restrict __cp, void *restrict __buf) {
    return -1;
}
