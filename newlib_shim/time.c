/*
    This file implements the necessary functions from newlib time.h for _POSIX_TIMERS
    We need this compatibitiy as it is required by libcxx in llvmproject
*/
#define _POSIX_MONOTONIC_CLOCK
#include <time.h>
#include <errno.h>
#undef errno
extern int errno;

int clock_settime (clockid_t clock_id, const struct timespec *tp){
    errno = EINVAL;
    return -1;
}
int clock_gettime (clockid_t clock_id, struct timespec *tp){
    if(clock_id == CLOCK_MONOTONIC){
        static struct timespec monotonic_time = {.tv_nsec = 0, .tv_sec = 0};
        monotonic_time.tv_nsec += 1;
        *tp = monotonic_time;
        return 0;
    }
    errno = EINVAL;
    return -1;
}
int clock_getres (clockid_t clock_id, struct timespec *res){
    errno = EINVAL;
    return -1;
}

/* Create a Per-Process Timer, P1003.1b-1993, p. 264 */

int timer_create (clockid_t clock_id,
 	struct sigevent *__restrict evp,
	timer_t *__restrict timerid){
        errno = ENOTSUP;
        return -1;
    }

/* Delete a Per_process Timer, P1003.1b-1993, p. 266 */

int timer_delete (timer_t timerid){
    errno = EINVAL;
    return -1;
}

/* Per-Process Timers, P1003.1b-1993, p. 267 */

int timer_settime (timer_t timerid, int flags,
	const struct itimerspec *__restrict value,
	struct itimerspec *__restrict ovalue){
        errno = EINVAL;
        return -1;
    }

int timer_gettime (timer_t timerid, struct itimerspec *value){
    errno = EINVAL;
    return -1;
}

int timer_getoverrun (timer_t timerid) {
    errno = EINVAL;
    return -1;
}

/* High Resolution Sleep, P1003.1b-1993, p. 269 */

int nanosleep (const struct timespec  *rqtp, struct timespec *rmtp){
    // always pretend to sleep for that amount
    return 0;
}

