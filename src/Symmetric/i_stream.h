#include <stdio.h>

struct i_stream
{
	int (*read_func)(struct i_stream *, unsigned int size, char* out);
	void * ptr;
	int size;
	int bytes_read;
};

int read_FILE(struct i_stream * i_s, unsigned int size, char* out);
void init_i_stream_FILE(struct i_stream * i_s, FILE* f);

int read_MEMORY(struct i_stream * i_s, unsigned int size, char* out);
void init_i_stream_MEMORY(struct i_stream * i_s, char* mem, int size);
