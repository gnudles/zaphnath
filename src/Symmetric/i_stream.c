#include <stdio.h>
#include <memory.h>
#include "i_stream.h"
int read_FILE(struct i_stream * i_s, unsigned int size, char* out)
{
	size_t bytes = fread(out, 1, size, (FILE*)i_s->ptr);
	i_s->bytes_read += bytes;
	if (feof((FILE*)i_s->ptr))
		i_s->size = i_s->bytes_read;
	return bytes;
}
void init_i_stream_FILE(struct i_stream * i_s, FILE* f)
{
	i_s->read_func = &read_FILE;
	i_s->ptr = (void*)f;
	i_s->size = -1;
	i_s->bytes_read = 0;
}

#define MIN_FUNC(TYPE,NAME) \
static inline TYPE NAME(TYPE a, TYPE b) \
{ if (a<b) return a; return b; }

MIN_FUNC(int,i_min)

int read_MEMORY(struct i_stream * i_s, unsigned int size, char* out)
{
	size_t bytes = i_min(size, i_s->size - i_s->bytes_read);
	memcpy(out,((char*)i_s->ptr)+i_s->bytes_read, bytes);
	i_s->bytes_read += bytes;
	return bytes;
}

void init_i_stream_MEMORY(struct i_stream * i_s, char* mem, int size)
{
	i_s->read_func = &read_MEMORY;
	i_s->ptr = (void*)mem;
	i_s->size = size;
	i_s->bytes_read = 0;
}
