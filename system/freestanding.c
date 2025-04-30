#include <stdint.h>

#include "system.h"

// need to copy forward, as memmove assumes this to be true
void *memcpy(void *dest, const void *src, size_t n) {
  // copy using size_t steps if available
  size_t pointer_size = sizeof(size_t);
  size_t copied = 0;
  size_t to_copy = pointer_size * (n / pointer_size);
  size_t *to = dest;
  const size_t *from = src;
  for (; copied < to_copy; copied += pointer_size) {
    *to = *from;
    from++;
    to++;
  }
  uint8_t *to8 = (uint8_t *)to;
  const uint8_t *from8 = (uint8_t *)from;
  for (; copied < n; copied++) {
    *to8 = *from8;
    from8++;
    to8++;
  }
  return dest;
}

void *memset(void *dest, int c, size_t n) {
  for (size_t i = 0; i < n; i++) {
    ((uint8_t *)dest)[i] = ((uint8_t)c);
  }
  return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
  // need to make sure we copy non overlapping regions first before overwriting
  uint8_t *from = (uint8_t *)src;
  uint8_t *to = (uint8_t *)dest;

  if (from == to || n == 0)
    return dest;
  if (to > from && to - from < (int)n) {
    /* to overlaps with from */
    /*  <from......>         */
    /*         <to........>  */
    /* copy in reverse, to avoid overwriting from */
    int i;
    for (i = n - 1; i >= 0; i--)
      to[i] = from[i];
    return dest;
  }
  // either overlapps the other way, which is unproblematic,
  // if we copy forward or does not overlap
  memcpy(dest, src, n);
  return dest;
}

int memcmp(const void *str1, const void *str2, size_t n) {
  uint8_t *one = (uint8_t *)str1;
  uint8_t *two = (uint8_t *)str2;
  for (size_t i = 0; i < n; i++) {
    if (one[i] != two[i])
      return one > two ? 1 : -1;
  }
  return 0;
}