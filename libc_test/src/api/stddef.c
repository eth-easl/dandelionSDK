#include <stddef.h>
#define T(t) (t*)0;
#define C(n) switch(n){case n:;}
static void f()
{
{void *p=NULL;}
// fix c99 compatibility issue with offsetof
struct off_s { int i; };
C(offsetof(struct off_s, i))
T(ptrdiff_t)
T(wchar_t)
T(size_t)
}
