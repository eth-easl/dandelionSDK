#include <setjmp.h>

/* https://pubs.opengroup.org/onlinepubs/9699919799/functions/siglongjmp.html
 * Signal mask management is not supported; restore the jmp_buf context only. */
void siglongjmp(sigjmp_buf env, int val) {
  longjmp(env, val);
}
