#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Linux x86_64 SYS_read value:
 * https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_64.tbl
 */
#if defined(__x86_64__)
#define SYS_read 0
#endif

/* Linux arm64 SYS_read value:
 * https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/unistd.h
 */
#if defined(__aarch64__)
#define SYS_read 63
#endif

long syscall(long, ...);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SYSCALL_H */
