#ifndef _LINUX_SYSCALL_H
#define _LINUX_SYSCALL_H

#include "syscall_arch.h"

#define FREEBSD_SYS_exit 1
#define FREEBSD_SYS_sysarch 165

// from freebsd sources
#define FREEBSD_AMD64_GET_FSBASE 128
#define FREEBSD_AMD64_SET_FSBASE 129
#define FREEBSD_AMD64_GET_GSBASE 130
#define FREEBSD_AMD64_SET_GSBASE 131
#define FREEBSD_AMD64_GET_XFPUSTATE 132
#define FREEBSD_AMD64_SET_PKRU 133
#define FREEBSD_AMD64_CLEAR_PKRU 134

#ifndef __scc
#define __scc(X) ((long)(X))
typedef long syscall_arg_t;
#endif

#define __syscall1(n, a) __syscall1(n, __scc(a))
#define __syscall2(n, a, b) __syscall2(n, __scc(a), __scc(b))
#define __syscall3(n, a, b, c) __syscall3(n, __scc(a), __scc(b), __scc(c))
#define __syscall4(n, a, b, c, d)                                              \
  __syscall4(n, __scc(a), __scc(b), __scc(c), __scc(d))
#define __syscall5(n, a, b, c, d, e)                                           \
  __syscall5(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e))
#define __syscall6(n, a, b, c, d, e, f)                                        \
  __syscall6(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e), __scc(f))
#define __syscall7(n, a, b, c, d, e, f, g)                                     \
  __syscall7(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e), __scc(f),    \
             __scc(g))

#define __SYSCALL_NARGS_X(a, b, c, d, e, f, g, h, n, ...) n
#define __SYSCALL_NARGS(...)                                                   \
  __SYSCALL_NARGS_X(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0, )
#define __SYSCALL_CONCAT_X(a, b) a##b
#define __SYSCALL_CONCAT(a, b) __SYSCALL_CONCAT_X(a, b)
#define __SYSCALL_DISP(b, ...)                                                 \
  __SYSCALL_CONCAT(b, __SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define __syscall(...) __SYSCALL_DISP(__syscall, __VA_ARGS__)

#endif
