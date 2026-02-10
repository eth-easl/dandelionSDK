#include <sched.h>
#include <errno.h>

int sched_yield (void) { return -1; }
int sched_getcpu(void) { return ENOTSUP; }
