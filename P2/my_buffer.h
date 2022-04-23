#include <stdlib.h>

#ifndef __MY_BUFFER_
#define __MY_BUFFER_

struct my_buffer
{
    double* ptr;
    size_t size;
    size_t len;
};

int add_to_buffer(struct my_buffer *buffer, double new_value);

void print_buffer(struct my_buffer* buffer);

void free_buffer(struct my_buffer *buffer);

#endif // __MY_BUFFER_