#include "utils.h"
#include "constants.h"

float convert_to_2d(float dist_3d, float depth1, float depth2){
    if (dist_3d > fabsf(depth1 - depth2)){  // if input valid
        return sqrtf(powf(dist_3d, 2) - powf(depth1 - depth2, 2));
    }
    else { // input invalid
        return 0.0;
    }
}

float dist_between_points_2d(coord_2d_t p1, coord_2d_t p2){
    return sqrtf(powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2));
}

float dist_between_points_3d(coord_3d_t p1, coord_3d_t p2){
    return sqrtf(powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2)+ powf(p1.z - p2.z, 2));
}


uint16_t get_freq(uint8_t idx_freq){
    return FREQUENCIES[idx_freq];
}