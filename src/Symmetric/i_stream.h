#include <stdio.h>
struct i_stream
{
	int (*read_func)(struct i_stream *, unsigned int size, char** out);
	void * ptr;
	char * buf;
	int size;
	int bytes_read;
}

int read_FILE(struct i_stream * i_s,unsigned int size, char** out)
{
	fread((FILE*)i_s->ptr
}
void init_i_stream_FILE(struct i_stream * i_s, FILE* f)
{
	i_s->read_func = &read_FILE;
	i_s->ptr = (void*)f;
	i_s->size = -1;
}

