#ifndef FYDP_RING_BUFFER_H
#define FYDP_RING_BUFFER_H

#include "constants.h"

#include <stdint.h>

typedef struct ring_buffer_t{
    uint16_t i_curr;
    uint16_t data[SZ_WINDOW];
} ring_buffer_t;

uint16_t ring_buffer_read_and_replace(ring_buffer_t *p_ring_buffer, uint16_t new_data);

#endif