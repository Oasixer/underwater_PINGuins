#include "ring_buffer.h"

uint16_t ring_buffer_read_and_replace(ring_buffer_t *p_ring_buffer, uint16_t new_data){
    uint16_t curr = p_ring_buffer->data[p_ring_buffer->i_curr];
    p_ring_buffer->data[p_ring_buffer->i_curr] = new_data;
    p_ring_buffer->i_curr = (p_ring_buffer->i_curr + 1) % SZ_WINDOW;  // TODO: macro equality vs mod
    return curr;
}
