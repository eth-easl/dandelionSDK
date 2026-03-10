#ifndef _NL_TYPES_H_
#define _NL_TYPES_H_

#include <sys/_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Values match glibc nl_types.h:
 * https://sources.debian.org/src/glibc/2.41-7/catgets/nl_types.h/
 */
#define NL_SETD 0
#define NL_CAT_LOCALE 1

typedef struct __nl_cat_d {
  void *__data;
  int __size;
} *nl_catd;

#ifndef _NL_ITEM_DECLARED
typedef __nl_item nl_item;
#define _NL_ITEM_DECLARED
#endif

int catclose(nl_catd);
char *catgets(nl_catd, int, int, const char *);
nl_catd catopen(const char *, int);

#ifdef __cplusplus
}
#endif

#endif /* _NL_TYPES_H_ */
