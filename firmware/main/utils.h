#ifndef FYDP_UTILS_H
#define FYDP_UTILS_H

#include <float.h>
#include <math.h>
#include <stdint.h>


typedef struct coord_2d_t{
    float x;
    float y;
} coord_2d_t;

typedef struct coord_3d_t{
    float x;
    float y;
    float z;
} coord_3d_t;


uint16_t get_freq(uint8_t idx_freq);

float convert_to_2d(float dist_3d, float depth1, float depth2);

float dist_between_points_2d(coord_2d_t p1, coord_2d_t p2);
float dist_between_points_3d(coord_3d_t p1, coord_3d_t p2);

#endif
