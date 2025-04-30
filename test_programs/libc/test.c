#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "unistd.h"

int main(int argc, char const *argv[]) {
  // print to std out and std err with the usual mode
  int err;
  if ((err = printf("Test string to stdout\n")) < 0) {
    return -1;
  }
  if ((err = fputs("Test string to stderr\n", stderr)) < 0) {
    return -2;
  }
  // read stdin
  char in_buffer[128];
  ssize_t read_chars = fread(in_buffer, 1, 128, stdin);
  if (read_chars < 0) {
    perror("Trying to read stdin failed\n");
    return -3;
  }
  printf("read %zi characters from stdin\n", read_chars);
  ssize_t written = fwrite(in_buffer, 1, read_chars, stdout);
  if (written != read_chars)
    return -4;
  // print string arguments
  for (int iter = 0; iter < argc; iter++) {
    printf("argument %d is %s\n", iter, argv[iter]);
  }
  char *env_home = getenv("HOME");
  printf("environmental variable HOME is %s\n", env_home);
  return 0;
}