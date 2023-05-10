#ifndef _DANDELION_IO_BUFFER_H
#define _DANDELION_IO_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct io_buffer {
	const char* ident;
	size_t ident_len;

	void* data;
	size_t data_len;
};

#ifdef __cplusplus
}
#endif

#endif // _DANDELION_IO_H