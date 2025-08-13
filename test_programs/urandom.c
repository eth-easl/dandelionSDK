#include <stdio.h>
#include <stdlib.h>
#include "dandelion/runtime.h"

void _start()
{
    dandelion_init();
    FILE *file = fopen("/dev/urandom", "rb");
    if (file == NULL) {
        perror("Error opening /dev/urandom");
        dandelion_exit(1);
    }

    unsigned char c;
    if (fread(&c, 1, 1, file) == 1) {
        printf("First random byte: %u\n", c);  // as number
        // printf("First random char: %c\n", c); // as ASCII char (may be non-printable)
    } else {
        perror("Error reading from /dev/urandom");
    }

    fclose(file);
    dandelion_exit(c);
}
