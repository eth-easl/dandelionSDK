#include <errno.h>
#include <stdio.h>

ssize_t getdelim(char **restrict lineptr, size_t *restrict n, int delimiter,
                 FILE *restrict stream) {
  (void)lineptr;
  (void)n;
  (void)delimiter;
  (void)stream;
  *__errno() = ENOSYS;
  return -1;
}

ssize_t getline(char **restrict lineptr, size_t *restrict n,
                FILE *restrict stream) {
  (void)lineptr;
  (void)n;
  (void)stream;
  *__errno() = ENOSYS;
  return -1;
}
