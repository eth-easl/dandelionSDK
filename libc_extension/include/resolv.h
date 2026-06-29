#ifndef _RESOLV_H
#define _RESOLV_H

#ifdef __cplusplus
extern "C" {
#endif

int dn_expand(const unsigned char *, const unsigned char *,
              const unsigned char *, char *, int);

#ifdef __cplusplus
}
#endif

#endif /* _RESOLV_H */
