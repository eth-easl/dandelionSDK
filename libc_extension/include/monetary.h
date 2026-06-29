#ifndef _MONETARY_H_
#define _MONETARY_H_

#include <sys/_locale.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

__BEGIN_DECLS

ssize_t strfmon(char *__restrict, size_t, const char *__restrict, ...);
ssize_t strfmon_l(char *__restrict, size_t, locale_t, const char *__restrict,
                  ...);

__END_DECLS

#ifdef __cplusplus
}
#endif

#endif /* _MONETARY_H_ */
