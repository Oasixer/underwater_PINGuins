#include "ring_buffer.h"

uint8_t ring_buffer_read(ring_buffer_t *p_ring_buffer){
    return p_ring_buffer->data[p_ring_buffer->i_read++];
}

void ring_buffer_write(ring_buffer_t *p_ring_buffer, uint8_t info){
    p_ring_buffer->data[p_ring_buffer->i_write] = info;
    p_ring_buffer->i_write = ++p_ring_buffer->i_write % p_ring_buffer->size;
}