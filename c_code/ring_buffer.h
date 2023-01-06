#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

#define RING_BUFFER_DIFF_BETWEEN_READ_AND_WRITE 250
#define RING_BUFFER_SIZE 1500

typedef struct ring_buffer_t{
    uint16_t i_read;
    uint16_t i_write;
    uint16_t size;
    uint8_t data[1500];
} ring_buffer_t;

#define RING_BUFFER_DEFAULT {0, RING_BUFFER_DIFF_BETWEEN_READ_AND_WRITE, RING_BUFFER_SIZE, {0},}

uint8_t ring_buffer_read(ring_buffer_t *p_ring_buffer);
void ring_buffer_write(ring_buffer_t *p_ring_buffer, uint8_t info);

#endif