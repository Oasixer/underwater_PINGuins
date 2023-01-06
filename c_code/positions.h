#ifndef FYDP_POSITIONS_H
#define FYDP_POSITIONS_H

#include <math.h>
#include <float.h>
#include <stdbool.h>

#include "freqs.h"
#include "utils.h"

typedef struct distances_t{
    float dist[N_ALL_NODES];
} distances_t;

bool calculate_coordinates(coord_3d_t *coords, distances_t dists_3d[N_ALL_NODES], float depths[N_ALL_NODES]);

#endif
