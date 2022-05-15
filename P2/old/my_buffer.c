#include "my_buffer.h"

int add_to_buffer(struct my_buffer *buffer, double new_value){
    if(buffer->ptr == NULL){ // Inicialização do buffer
        buffer->size = 1;
        buffer->len = 0;
        buffer->ptr = malloc(buffer->size*sizeof(double));
    }
    else{
        if(buffer->len >= buffer->size){ // Aloca mais memoria ao buffer caso o limite seja atingido
            buffer->size *= 2;
            buffer->ptr = realloc(buffer->ptr,buffer->size*sizeof(double));
            // printf("\nNew size: %i\n",buffer->size);
        }
    }
    buffer->ptr[buffer->len] = new_value;

    buffer->len++;
    return buffer->len;
}

void print_buffer(struct my_buffer* buffer){
    if(buffer->ptr == NULL) return;
    int i;
    for(i = 0; i < buffer->len; i++) printf("%i | %.2f\n",i,buffer->ptr[i]);
}

void free_buffer(struct my_buffer *buffer){
    free(buffer->ptr);
}