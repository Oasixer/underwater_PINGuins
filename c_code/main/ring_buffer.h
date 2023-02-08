#ifndef FYDP_RING_BUFFER_H
#define FYDP_RING_BUFFER_H

#include <stdint.h>

#define RING_BUFFER_SIZE 1250

typedef struct ring_buffer_t{
    uint16_t i_curr;
    uint16_t data[RING_BUFFER_SIZE];
} ring_buffer_t;

uint16_t ring_buffer_read_and_replace(ring_buffer_t *p_ring_buffer, uint16_t new_data);

#endif