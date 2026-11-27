#ifndef	_SYS_IOCTL_H
#define	_SYS_IOCTL_H
#ifdef __cplusplus
extern "C" {
#endif

// TODO: properly implement in a C file with only definition here
// Also check which devices we can actually do useful action for and which should probably return an error.
static int ioctl (int a, int b, ...) {
    return 0;
}

#ifdef __cplusplus
}
#endif
#endif