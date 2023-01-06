#ifndef MULTILATERATION_H
#define MULTILATERATION_H

#include <math.h>
#include <stdbool.h>
#include <float.h>

#include "positions.h"
#include "utils.h"

coord_3d_t multilaterate(coord_3d_t node_coords_3d[N_ALL_NODES], float dists_3d[N_ALL_NODES], float curr_depth, float og_depth);

#endif //MULTILATERATION_H
