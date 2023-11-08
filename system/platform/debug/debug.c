#include "dandelion/system/system.h"
#include "../../system.h"

#include <stddef.h>
#include <stdint.h>
#include "syscall.h"

#define sysdata __dandelion_system_data

static size_t my_strlen(const char* string) {
	size_t len = 0;
	while (string[len]) {
		++len;
	}
	return len;
}

static void my_memcpy(void* dest, const void* src, size_t size) {
	char* d = (char*)dest;
	const char* s = (const char*)src;
	for (size_t i = 0; i < size; ++i) {
		d[i] = s[i];
	}
}

static int my_write(int fd, const void *buffer, size_t size, long *bytes_written) {
	long ret = __syscall(SYS_write, fd, buffer, size);
	if (ret < 0) {
		return -ret;
	}
	*bytes_written = ret;
	return 0;
}

static int write_all(int fd, const void *buffer, size_t size) {
	size_t written = 0;
	while (written < size) {
		long bytes_written;
		int e = my_write(fd, (const char*)buffer + written, size - written, &bytes_written);
		if (e < 0) {
			return e;
		}
		written += bytes_written;
	}
	return 0;
}

static void dump_io_buf(const char* setid, struct io_buffer* buf) {
	char tmp[256];
	size_t setidlen = 0;
	if (setid) {
		setidlen = my_strlen(setid);
		my_memcpy(tmp, setid, setidlen);
	}
	size_t identlen = buf->ident_len;
	if (buf->ident) {
		tmp[setidlen] = ' ';
		my_memcpy(tmp + setidlen + 1, buf->ident, identlen);
		++identlen;
	}
	tmp[setidlen + identlen] = ':';
	tmp[setidlen + identlen + 1] = '\n';
	write_all(1, tmp, setidlen + identlen + 2);
	write_all(1, buf->data, buf->data_len);
	write_all(1, "\n", 1);
}

static void dump_global_data() {
	for (size_t i = 0; i < sysdata.output_sets_len; ++i) {
		struct io_set_info* set = &sysdata.output_sets[i];
		size_t num_elems = sysdata.output_sets[i + 1].offset - set->offset;
		for (size_t j = 0; j < num_elems; ++j) {
			dump_io_buf(set->ident, &sysdata.output_bufs[set->offset + j]);
		}
	}
}

static void* vm_alloc(size_t size) {
	long ret = __syscall(SYS_mmap, NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ret < 0 && ret > -4096) {
		return NULL;
	}
	return (void*)ret;
}

void __dandelion_platform_init(void) {
	static const char input_file_content[] = "This is an example input file";
	static const char input_file_name[] = "input.txt";

	static struct io_set_info input_sets[] = {
		{NULL, 0, 0},
		{NULL, 0, 1},
	};
	static struct io_set_info output_sets[] = {
		{NULL, 0, 0},
		{"output", sizeof("output") - 1, 0},
		{NULL, 0, 0},
	};

	static struct io_buffer input_bufs[] = {
		{input_file_name, sizeof(input_file_name) - 1, (void*)input_file_content, sizeof(input_file_content)},
	};

	sysdata.input_bufs = input_bufs;
	sysdata.input_sets = input_sets;
	sysdata.input_sets_len = sizeof(input_sets) / sizeof(input_sets[0]) - 1;

	sysdata.output_bufs = NULL;
	sysdata.output_sets = output_sets;
	sysdata.output_sets_len = sizeof(output_sets) / sizeof(output_sets[0]) - 1;


	size_t alloc_size = 1ull << 32;
	void* heap_ptr = vm_alloc(alloc_size);
	if (heap_ptr == NULL) {
		__syscall(SYS_exit_group, 1);
	}

	sysdata.heap_begin = (uintptr_t)heap_ptr;
	sysdata.heap_end = sysdata.heap_begin + alloc_size;
}

void __dandelion_platform_exit(void) {
	dump_global_data();
	__syscall(SYS_exit_group, 0);
	__builtin_unreachable();
}

void __dandelion_platform_set_thread_pointer(void *ptr) {
#if defined(__x86_64__)
	__syscall(SYS_arch_prctl, ARCH_SET_FS, ptr);
#elif defined(__aarch64__)
	size_t thread_data = (size_t)ptr;
	asm volatile("msr tpidr_el0, %0" :: "r"(thread_data));
#else
#error "Missing architecture specific code."
#endif
}