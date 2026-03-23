/*
    This file implements the necessary functions from newlib time.h for
   _POSIX_TIMERS We need this compatibitiy as it is required by libcxx in
   llvmproject
*/
#define _POSIX_MONOTONIC_CLOCK
#define __GNU_VISIBLE 0
#include <errno.h>
#include <sys/reent.h>
#include <time.h>
#undef errno
extern int errno;

#if defined(__aarch64__)
  static uint64_t __rdtsc() {
    uint64_t t;
    __asm volatile ("mrs %0, cntvct_el0" : "=r"(t));
    return t;
  }
#elif defined(__x86_64__)
  static uint64_t __rdtsc() {
    uint64_t t;
    __asm volatile ("rdtsc": "=A"(t));
    return t;
  }
#else
#error "Unsupported architecture"
#endif

static int dandelion_getdate_err;

int *__getdate_err(void) { return &dandelion_getdate_err; }

struct tm *getdate(const char *string) {
  (void)string;
  dandelion_getdate_err = 0;
  errno = ENOSYS;
  return NULL;
}

int getdate_r(const char *string, struct tm *result) {
  (void)string;
  (void)result;
  dandelion_getdate_err = 0;
  errno = ENOSYS;
  return -1;
}

int clock_settime(clockid_t clock_id, const struct timespec *tp) {
  errno = EINVAL;
  return -1;
}
int clock_gettime(clockid_t clock_id, struct timespec *tp) {
  // TODO add static timestamp at the start to offset here and possibly fix to get actual realtime instead of offset one
  // TODO check if we need to differentiate any of these and if they need setup from the dandelion side
  static char is_initialized = 0;
  static uint64_t base_time;
  if (is_initialized == 0) {
    base_time = __rdtsc();
    is_initialized = 1;
  }
  uint64_t time_diff = __rdtsc() - base_time;
  uint64_t divisor = 1000000000;
  tp->tv_nsec = time_diff % divisor;
  tp->tv_sec = time_diff / divisor;
  return 0;
}
int clock_getres(clockid_t clock_id, struct timespec *res) {
  errno = EINVAL;
  return -1;
}

/* Create a Per-Process Timer, P1003.1b-1993, p. 264 */

int timer_create(clockid_t clock_id, struct sigevent *__restrict evp,
                 timer_t *__restrict timerid) {
  errno = ENOTSUP;
  return -1;
}

/* Delete a Per_process Timer, P1003.1b-1993, p. 266 */

int timer_delete(timer_t timerid) {
  errno = EINVAL;
  return -1;
}

/* Per-Process Timers, P1003.1b-1993, p. 267 */

int timer_settime(timer_t timerid, int flags,
                  const struct itimerspec *__restrict value,
                  struct itimerspec *__restrict ovalue) {
  errno = EINVAL;
  return -1;
}

int timer_gettime(timer_t timerid, struct itimerspec *value) {
  errno = EINVAL;
  return -1;
}

int timer_getoverrun(timer_t timerid) {
  errno = EINVAL;
  return -1;
}

/* High Resolution Sleep, P1003.1b-1993, p. 269 */

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp) {
  // always pretend to sleep for that amount
  return 0;
}
