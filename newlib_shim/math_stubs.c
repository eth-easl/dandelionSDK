#include <errno.h>
#include <math.h>

// defined by newlib, but not always avaliable in headers
#undef errno
extern int errno;

double nexttoward(double x, long double y) {
  (void)x;
  (void)y;
  errno = ENOSYS;
  return NAN;
}

float nexttowardf(float x, long double y) {
  (void)x;
  (void)y;
  errno = ENOSYS;
  return NAN;
}

long double nexttowardl(long double x, long double y) {
  (void)x;
  (void)y;
  errno = ENOSYS;
  return (long double)NAN;
}

long double exp10l(long double x) {
  (void)x;
  errno = ENOSYS;
  return NAN;
}

long double pow10l(long double x) {
  (void)x;
  errno = ENOSYS;
  return NAN;
}

void sincosl(long double x, long double *sinx, long double *cosx) {
  (void)x;
  if (sinx != NULL) {
    *sinx = NAN;
  }
  if (cosx != NULL) {
    *cosx = NAN;
  }
  errno = ENOSYS;
}

long double __kernel_tanl(long double x, long double y, int iy) {
  (void)x;
  (void)y;
  (void)iy;
  errno = ENOSYS;
  return NAN;
}

unsigned long __pleval(const char *expr, unsigned long n) {
  (void)expr;
  (void)n;
  errno = ENOSYS;
  return (unsigned long)-1;
}
