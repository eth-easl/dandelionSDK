#include "../../system.h"
#include "dandelion/system/system.h"

#include "syscall.h"
#include <stddef.h>
#include <stdint.h>

#define sysdata __dandelion_system_data

#define DIRENT_BUF_SIZE 4096
#define DT_DIR 4
#define DT_REG 8

static size_t my_strlen(const char *string) {
  size_t len = 0;
  while (string[len]) {
    ++len;
  }
  return len;
}

static void my_memcpy(void *dest, const void *src, size_t size) {
  char *d = (char *)dest;
  const char *s = (const char *)src;
  for (size_t i = 0; i < size; ++i) {
    d[i] = s[i];
  }
}

static void print_and_exit(char *message, int exit_code) {
  size_t string_length = my_strlen(message);
  __syscall(SYS_write, 2, message, string_length);
  __syscall(SYS_exit_group, exit_code);
  __builtin_unreachable();
}

static void write_all(int fd, const void *buffer, size_t size) {
  size_t written = 0;
  while (written < size) {
    // is supposed to be ssize_t, but that is not always available
    // ptrdiff_t should be able to hold the value
    ptrdiff_t e = __syscall(SYS_write, fd, (const char *)buffer + written,
                            size - written);
    if (e < 0) {
      __syscall(SYS_exit_group);
      __builtin_unreachable();
    }
    written += e;
  }
  return;
}

static void dump_io_buf(const char *setid, size_t setidlen, IoBuffer *buf) {
  char tmp[256] = "output_sets/";
  size_t start_len = my_strlen(tmp);
  if (setid == NULL || buf == NULL || buf->ident == NULL) {
    char *message = "Output set id, buffer pointer or buffer id are NULL";
    size_t message_length = my_strlen(message);
    write_all(2, message, message_length);
    return;
  }

  my_memcpy(tmp + start_len, setid, setidlen);
  size_t identlen = buf->ident_len;
  tmp[start_len + setidlen] = '/';
  my_memcpy(tmp + start_len + setidlen + 1, buf->ident, identlen);
  tmp[start_len + setidlen + 1 + identlen] = '\0';
  // print the set and buffer identifiers to console
  write_all(1, tmp, start_len + setidlen + 1 + identlen);
  write_all(1, "\n", 1);
  // open file file for the data
  int out_fd =
      __syscall(SYS_openat, AT_FDCWD, tmp, O_WRONLY | O_CREAT | O_TRUNC, 00666);
  if (out_fd < 0)
    print_and_exit("Failed to open output file\n", -out_fd);
  write_all(out_fd, buf->data, buf->data_len);
}

static void dump_global_data() {
  for (size_t i = 0; i < sysdata.output_sets_len; ++i) {
    struct io_set_info *set = &sysdata.output_sets[i];
    size_t num_elems = sysdata.output_sets[i + 1].offset - set->offset;
    for (size_t j = 0; j < num_elems; ++j) {
      dump_io_buf(set->ident, set->ident_len,
                  &sysdata.output_bufs[set->offset + j]);
    }
  }
}

static void *vm_alloc(size_t size) {
  long ret = __syscall(SYS_mmap, NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret < 0 && ret > -4096) {
    return NULL;
  }
  return (void *)ret;
}

static inline char *round_up_to(char *original, size_t alignment) {
  size_t mod = ((uintptr_t)original) % alignment;
  size_t additional = alignment - (mod == 0 ? alignment : mod);
  return original + additional;
}

// allocation without freeing, only for smaller structures
static void *debug_alloc(size_t size) {
  static size_t *buffer_space = NULL;
  static size_t used = 0;
  static const size_t page_size = 4096;
  size_t new_allocation = ((size + sizeof(size_t) - 1) / sizeof(size_t));
  if (buffer_space == NULL ||
      page_size < (used + new_allocation) * sizeof(size_t)) {
    buffer_space = vm_alloc(page_size);
    used = 0;
  }
  void *ret_ptr = buffer_space + used;
  used += new_allocation;
  return ret_ptr;
}

void __dandelion_platform_init(void) {

  size_t alloc_size = 1ull << 31;
  void *heap_ptr = vm_alloc(alloc_size);
  void *heap_end = (void *)((char *)heap_ptr + alloc_size);
  if (heap_ptr == NULL) {
    __syscall(SYS_exit_group, 1);
  }

  // attempt to open folder input_sets
  int in_sets_fd = __syscall(SYS_openat, AT_FDCWD, "input_sets", O_RDONLY);
  if (in_sets_fd < 0) {
    print_and_exit("No input_sets directory in current working directory\n",
                   -1);
  }
  // start going through files
  typedef struct linux_dirent {
    uint64_t d_ino;
    uint64_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
  } linux_dirent;
  char dirent_buffer[DIRENT_BUF_SIZE];
  char set_dirent_buffer[DIRENT_BUF_SIZE];
  char item_dirent_buffer[DIRENT_BUF_SIZE];

  long dirent_read = 0;
  size_t total_buffers = 0;
  size_t input_set_index = 0;
  struct io_set_info *input_sets = heap_ptr;

  write_all(1, "input_sets:\n", 13);

  while ((dirent_read = __syscall(SYS_getdents64, in_sets_fd, &dirent_buffer,
                                  DIRENT_BUF_SIZE)) > 0) {
    // have read one or more entries, need to process them before reading again
    for (long dirent_offset = 0; dirent_offset < dirent_read;
         dirent_offset +=
         ((linux_dirent *)&dirent_buffer[dirent_offset])->d_reclen) {
      // get the dirent for the current input set
      linux_dirent *dirent = (linux_dirent *)&dirent_buffer[dirent_offset];
      size_t ident_len = my_strlen(dirent->d_name);
      if ((ident_len == 1 && dirent->d_name[0] == '.') ||
          (ident_len == 2 && dirent->d_name[0] == '.' &&
           dirent->d_name[1] == '.'))
        continue;

      if (dirent->d_type != DT_DIR)
        print_and_exit("file in input_sets not direcotory", -1);

      // setupt the input set descriptor
      char *ident_buffer = (char *)debug_alloc(ident_len);
      my_memcpy(ident_buffer, dirent->d_name, ident_len);
      input_sets[input_set_index].ident = ident_buffer;
      input_sets[input_set_index].ident_len = ident_len;
      input_set_index++;
    }
  }
  if (dirent_read < 0)
    print_and_exit("getdents failed\n", -dirent_read);

  heap_ptr += (input_set_index + 1) * sizeof(struct io_set_info);
  // prepare for IoBuffer array for input sets
  heap_ptr = round_up_to(heap_ptr, _Alignof(IoBuffer));
  sysdata.input_bufs = heap_ptr;

  for (size_t input_set = 0; input_set < input_set_index; input_set++) {
    input_sets[input_set].offset = total_buffers;
    write_all(1, "\t", 1);
    write_all(1, input_sets[input_set].ident, input_sets[input_set].ident_len);
    write_all(1, "\n", 1);
    // open the input set folder
    int set_folder_fd = __syscall(SYS_openat, in_sets_fd,
                                  input_sets[input_set].ident, O_RDONLY);
    if (set_folder_fd < 0)
      print_and_exit("could not open set folder\n", -1);

    // read the directory
    long set_dirent_read = 0;
    while ((set_dirent_read = __syscall(SYS_getdents64, set_folder_fd,
                                        &set_dirent_buffer, DIRENT_BUF_SIZE)) >
           0) {
      // found one or more items
      for (long set_dirent_offset = 0; set_dirent_offset < set_dirent_read;
           set_dirent_offset +=
           ((linux_dirent *)&set_dirent_buffer[set_dirent_offset])->d_reclen) {
        linux_dirent *set_dirent =
            (linux_dirent *)&set_dirent_buffer[set_dirent_offset];
        size_t item_ident_len = my_strlen(set_dirent->d_name);
        if ((item_ident_len == 1 && set_dirent->d_name[0] == '.') ||
            (item_ident_len == 2 && set_dirent->d_name[0] == '.' &&
             set_dirent->d_name[1] == '.'))
          continue;

        write_all(1, "\t\t", 2);
        write_all(1, set_dirent->d_name, item_ident_len);
        write_all(1, "\n", 1);
        // add one buffer to heap
        IoBuffer *current_buffer = heap_ptr;
        heap_ptr += sizeof(IoBuffer);
        char *item_ident_buffer = (char *)debug_alloc(item_ident_len);
        my_memcpy(item_ident_buffer, set_dirent->d_name, item_ident_len);
        // pretend the files were in folders
        for (size_t index = 0; index < item_ident_len; index++) {
          if (item_ident_buffer[index] == '+') {
            item_ident_buffer[index] = '/';
          }
        }
        current_buffer->ident = item_ident_buffer;
        current_buffer->ident_len = item_ident_len;
        current_buffer->key = 0;
        int item_fd =
            __syscall(SYS_openat, set_folder_fd, set_dirent->d_name, O_RDONLY);
        if (item_fd < 0)
          print_and_exit("could not open item file", -1);
        ptrdiff_t file_size = __syscall(SYS_lseek, item_fd, 0, 2);
        if (file_size < 0)
          print_and_exit("Could not get file size\n", -1);
        char *file_addr = NULL;
        if (file_size > 0) {
          file_addr = (char *)__syscall(SYS_mmap, NULL, file_size, PROT_READ,
                                        MAP_PRIVATE, item_fd, 0);
          if ((long)file_addr < 0)
            print_and_exit("Could not map item file\n", -(int)file_addr);
        }
        current_buffer->data = file_addr;
        current_buffer->data_len = file_size;
        total_buffers++;
      }
    }
    if (set_dirent_read < 0)
      print_and_exit("getdents failed\n", -dirent_read);
  }

  // set up sentinel set
  input_sets[input_set_index].ident = NULL;
  input_sets[input_set_index].ident_len = 0;
  input_sets[input_set_index].offset = total_buffers;

  sysdata.input_sets = input_sets;
  sysdata.input_sets_len = input_set_index;

  // start processing output sets
  write_all(1, "output_sets:\n", 14);

  int out_sets_fd = __syscall(SYS_openat, AT_FDCWD, "output_sets", O_RDONLY);
  if (out_sets_fd < 0) {
    print_and_exit("No output set directory in current working directory\n",
                   -1);
  }

  heap_ptr = round_up_to(heap_ptr, _Alignof(struct io_set_info));
  struct io_set_info *output_sets = heap_ptr;
  size_t output_set_index = 0;
  while ((dirent_read = __syscall(SYS_getdents64, out_sets_fd, &dirent_buffer,
                                  DIRENT_BUF_SIZE)) > 0) {
    for (long dirent_offset = 0; dirent_offset < dirent_read;
         dirent_offset +=
         ((linux_dirent *)&dirent_buffer[dirent_offset])->d_reclen) {
      linux_dirent *dirent = (linux_dirent *)&dirent_buffer[dirent_offset];
      size_t ident_len = my_strlen(dirent->d_name);
      if ((ident_len == 1 && dirent->d_name[0] == '.') ||
          (ident_len == 2 && dirent->d_name[0] == '.' &&
           dirent->d_name[1] == '.'))
        continue;

      write_all(1, "\t", 1);
      write_all(1, dirent->d_name, ident_len);
      write_all(1, "\n", 1);
      // check it is a directory
      if ((ident_len == 1 && dirent->d_name[0] == '.') ||
          (ident_len == 2 && dirent->d_name[0] == '.' &&
           dirent->d_name[1] == '.'))
        continue;
      if (dirent->d_type != DT_DIR)
        print_and_exit("file in output_sets not direcotory", -1);
      char *ident_buffer = (char *)debug_alloc(ident_len);
      my_memcpy(ident_buffer, dirent->d_name, ident_len);
      output_sets[output_set_index].ident = ident_buffer;
      output_sets[output_set_index].ident_len = ident_len;
      output_sets[output_set_index].offset = 0;
      output_set_index++;
    }
  }
  if (dirent_read < 0)
    print_and_exit("getdents failed\n", -dirent_read);

  // set up sentinel set
  output_sets[output_set_index].ident = NULL;
  output_sets[output_set_index].ident_len = 0;
  output_sets[output_set_index].offset = 0;

  heap_ptr += (output_set_index + 1) * sizeof(struct io_set_info);

  sysdata.output_bufs = NULL;
  sysdata.output_sets = output_sets;
  sysdata.output_sets_len = output_set_index;

  sysdata.heap_begin = (uintptr_t)heap_ptr;
  sysdata.heap_end = (uintptr_t)heap_end;
}

void __dandelion_platform_exit(void) {
  dump_global_data();
  // print exit code
  char exit_message[] = "Exiting with code ";
  size_t message_len = my_strlen(exit_message);
  write_all(1, exit_message, message_len);
  char exit_code_string[12] = " 0000000000\n";
  // convert int to string
  int exit_code = sysdata.exit_code;
  // remove sign
  if (exit_code < 0) {
    exit_code_string[0] = '-';
    exit_code *= -1;
  }
  for (size_t index = 0; index < 10; index++) {
    exit_code_string[10 - index] = '0' + (exit_code % 10);
    exit_code = exit_code / 10;
    if (exit_code == 0)
      break;
  }
  write_all(1, exit_code_string, 12);
  __syscall(SYS_exit_group, sysdata.exit_code);
  __builtin_unreachable();
}

void __dandelion_platform_set_thread_pointer(void *ptr) {
#if defined(__x86_64__)
  __syscall(SYS_arch_prctl, ARCH_SET_FS, ptr);
#elif defined(__aarch64__)
  size_t thread_data = (size_t)ptr;
  asm volatile("msr tpidr_el0, %0" ::"r"(thread_data));
#else
#error "Missing architecture specific code."
#endif
}
