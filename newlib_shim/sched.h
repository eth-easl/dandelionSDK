#ifndef _SYS_SCHED_H_
#define _SYS_SCHED_H_

#include <sys/_timespec.h>
#include <sys/cpuset.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Scheduling Policies */
/* Open Group Specifications Issue 6 */
#define SCHED_OTHER    0
#define SCHED_FIFO     1
#define SCHED_RR       2
#define SCHED_SPORADIC 4

/* Scheduling Parameters */
/* Open Group Specifications Issue 6 */

struct sched_param {
  int sched_priority;           /* Process execution scheduling priority */

#if defined(_POSIX_SPORADIC_SERVER) || defined(_POSIX_THREAD_SPORADIC_SERVER)
  int sched_ss_low_priority;    /* Low scheduling priority for sporadic */
                                /*   server */
  struct timespec sched_ss_repl_period;
                                /* Replenishment period for sporadic server */
  struct timespec sched_ss_init_budget;
                               /* Initial budget for sporadic server */
  int sched_ss_max_repl;       /* Maximum pending replenishments for */
                               /* sporadic server */
#endif
};

#ifdef __cplusplus
}
#endif

#endif
/* end of include file */