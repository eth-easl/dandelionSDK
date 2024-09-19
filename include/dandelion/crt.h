#ifndef _DANDELION_CRT_H
#define _DANDELION_CRT_H

#define DANDELION_ENTRY(main)                                                  \
  void __dandelion_entry(void) {                                               \
    dandelion_init();                                                          \
    dandelion_exit((main)());                                                  \
  }

void _start(void) __attribute__((naked));
void _start(void) {
#if defined(__x86_64__)
  __asm__("call __dandelion_entry\n");
#elif defined(__wasm__)
  __asm__("call __dandelion_entry\n");
  __asm__("return\n");
#elif defined(__aarch64__)
  __asm__("bl __dandelion_entry");
#else
#error "Missing architecture specific code."
#endif
}

#endif
