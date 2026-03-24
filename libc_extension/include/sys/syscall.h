#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Linux x86_64 SYS_read value matches musl:
 * https://git.musl-libc.org/cgit/musl/tree/arch/x86_64/bits/syscall.h.in
 */
#if defined(__x86_64__)
#define SYS_read 0
#endif

/* Linux aarch64 SYS_read value matches musl:
 * https://git.musl-libc.org/cgit/musl/tree/arch/aarch64/bits/syscall.h.in
 */
#if defined(__aarch64__)
#define SYS_read 63
#endif

long syscall(long, ...);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SYSCALL_H */
