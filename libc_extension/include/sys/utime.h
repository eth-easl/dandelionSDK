#ifndef _SYS_UTIME_H
#define _SYS_UTIME_H

#ifdef __cplusplus
extern "C" {
#endif

struct utimbuf 
{
  time_t actime;
  time_t modtime; 
};

// TODO: These are stub implementations, implement properly:
int utime(const char *path, const struct utimbuf *_Nullable times) {return 0;}

#ifdef __cplusplus
};
#endif

#endif /* _SYS_UTIME_H */