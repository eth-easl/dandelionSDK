#ifndef _MONETARY_H_
#define _MONETARY_H_

#include <sys/_locale.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

ssize_t strfmon(char *restrict, size_t, const char *restrict, ...);
ssize_t strfmon_l(char *restrict, size_t, locale_t, const char *restrict, ...);

__END_DECLS

#endif /* _MONETARY_H_ */
