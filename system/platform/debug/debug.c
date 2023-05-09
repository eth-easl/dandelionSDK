#include <dandelion/system/system.h>

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <syscall.h>

static int write(int fd, const void *buffer, size_t size, ssize_t *bytes_written) {
	auto ret = do_cp_syscall(SYS_write, fd, buffer, size);
	if(int e = sc_error(ret); e)
		return e;
	*bytes_written = sc_int_result<ssize_t>(ret);
	return 0;
}

static int write_all(int fd, const void *buffer, size_t size) {
	size_t written = 0;
	while (written < size) {
		ssize_t bytes_written;
		int e = write(fd, (const char*)buffer + written, size - written, &bytes_written);
		if (e < 0) {
			return e;
		}
		written += bytes_written;
	}
	return 0;
}

static void dump_io_buf(const char* setid, io_buffer* buf) {
	char tmp[256];
	size_t setidlen = 0;
	if (setid) {
		setidlen = strlen(setid);
		memcpy(tmp, setid, setidlen);
	}
	size_t identlen = buf->ident_len;
	if (buf->ident) {
		tmp[setidlen] = ' ';
		memcpy(tmp + setidlen + 1, buf->ident, identlen);
		++identlen;
	}
	tmp[setidlen + identlen] = ':';
	tmp[setidlen + identlen + 1] = '\n';
	write_all(1, tmp, setidlen + identlen + 2);
	write_all(1, buf->data, buf->data_len);
	write_all(1, "\n", 1);
}

static void dump_io_set(io_set* set) {
	for (io_buffer* buf = set->buffers; buf != set->buffers + set->buffers_len; ++buf) {
		dump_io_buf(set->ident, buf);
	}
}

static void dump_global_data() {

	dump_io_buf("stdout", &dandelion.stdout);
	dump_io_buf("stderr", &dandelion.stderr);

	for (auto* set = dandelion.output_sets; set != dandelion.output_sets + dandelion.output_sets_len; ++set) {
		debug::dump_io_set(set);
	}
}

static int vm_map(void *hint, size_t size, int prot, int flags,
		int fd, off_t offset, void **window) {
	auto ret = do_syscall(SYS_mmap, hint, size, prot, flags, fd, offset);
	// TODO: musl fixes up EPERM errors from the kernel.
	if(int e = sc_error(ret); e)
		return e;
	*window = sc_ptr_result<void>(ret);
	return 0;
}

static int vm_unmap(void *pointer, size_t size) {
	auto ret = do_syscall(SYS_munmap, pointer, size);
	if(int e = sc_error(ret); e)
		return e;
	return 0;
}

void __dandelion_system_platform_init(void) {
	static const char input_file_content[] = "This is an example input file";
	static const char input_file_name[] = "input.txt";
	static const char output_file_name[] = "root_output.txt";
	static io_buffer example_input_file{input_file_name, sizeof(input_file_name) - 1, (void*)input_file_content, sizeof(input_file_content)};
	static io_buffer example_output_file{output_file_name, sizeof(output_file_name) - 1, nullptr, 0};

	static io_set input_sets[] = {
		{nullptr, 0, &example_input_file, 1},
	};
	static io_set output_sets[] = {
		{nullptr, 0, &example_output_file, 1},
		{"output", 6, nullptr, 0},
	};

	dandelion.stdin = {nullptr, 0, nullptr, 0};

	dandelion.input_sets = input_sets;
	dandelion.input_sets_len = sizeof(input_sets) / sizeof(input_sets[0]);

	dandelion.output_sets = output_sets;
	dandelion.output_sets_len = sizeof(output_sets) / sizeof(output_sets[0]);

	size_t alloc_size = 1ull << 32;
	void* heap_ptr = nullptr;

	vm_map(nullptr, alloc_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0, &heap_ptr);

	dandelion.heap_begin = (uintptr_t)heap_ptr;
	dandelion.heap_end = dandelion.heap_begin + alloc_size;
}

_Noreturn void __dandelion_system_platform_exit(void) {
    dump_global_data();
	__syscall(SYS_exit_group, 0);
    __builtin_unreachable();
}
