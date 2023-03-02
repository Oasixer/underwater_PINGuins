#include "multilateration.h"

void get_points_of_intersection(coord_2d_t *intersections, float dists[2], coord_2d_t coords[2]){
    float diff = dist_between_points_2d(coords[0], coords[1]);
    float angle;
    if (fabsf(coords[1].x] - coords[0].x) >= EPSILON && fabsf(coords[1].y] - coords[0].y]) >= EPSILON){
        angle = atan2f(coords[1].y - coords[0].y, coords[1].x - coords[0].x);
    } else {
        angle = 0;
    }

    float x_local = (powf(dists[0], 2) - powf(dists[1], 2) + powf(diff, 2)) / (2 * diff);
    float y1_local;
    if (dists[0] > fabsf(x_local)){
        y1_local = sqrtf(powf(dists[0], 2) - powf(x_local, 2));
    } else {
        y1_local = 0.0;
    }
    float y2_local = -y1_local;

    intersections[0].x = x_local*cosf(angle) - y1_local*sinf(angle) + coords[0].x;
    intersections[1].x = x_local*cosf(angle) - y2_local*sinf(angle) + coords[0].x;

    intersections[0].y = x_local*sinf(angle) + y1_local*cosf(angle) + coords[0].y;
    intersections[1].y = x_local*sinf(angle) + y2_local*cosf(angle) + coords[0].y;
}

coord_3d_t multilaterate(coord_3d_t node_coords_3d[N_ALL_NODES], float dists_3d[N_ALL_NODES], float curr_depth, float og_depth){
    // ensure that dists are not negative
    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
        if (dists_3d[i] < 0.0){
            dists_3d[i] = 0.0;
        }
    }

    // get 2d projections
    float dists_2d[N_ALL_NODES];
    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
        dists_2d[i] = convert_to_2d(dists_3d[i], node_coords_3d[i].z, curr_depth - og_depth);
    }

    // get 2d coordinates
    coord_2d_t node_coords_2d[N_ALL_NODES];
    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
        node_coords_2d[i].x = node_coords_3d[i].x;
        node_coords_2d[i].y = node_coords_3d[i].y;
    }

    // get the coordinates of the points of intersection
    coord_2d_t points_of_intersection[6];
    for (uint8_t node_i = 1; node_i < N_ALL_NODES-1; node_i++){
        for (uint8_t node_j = node_i+1; node_j < N_ALL_NODES; node_j++){
            float curr_dists[2] = {dists_2d[node_i], dists_2d[node_j]};
            coord_2d_t curr_node_coords[2] = {node_coords_2d[node_i], node_coords_2d[node_j]};
            get_points_of_intersection(
                    &points_of_intersection[(node_i + node_j - 3) * 2],
                    curr_dists,
                    curr_node_coords);
        }
    }

    // get the pair closest to each other, and get the centroid of the pair
    uint8_t i_closest1, i_closest2;
    float min_dist = FLT_MAX;
    for (uint8_t i = 0; i < 6; ++i){
        for (uint8_t j = i+1; j < 6; ++j){
            float pair_dist = dist_between_points_2d(points_of_intersection[i], points_of_intersection[j]);
            if (pair_dist < min_dist){
                min_dist = pair_dist;
                i_closest1 = i;
                i_closest2 = j;
            }
        }
    }
    coord_2d_t centroid = {
            (points_of_intersection[i_closest1].x + points_of_intersection[i_closest2].x) / 2,
            (points_of_intersection[i_closest1].y + points_of_intersection[i_closest2].y) / 2,
    };

    // get the closest point to the centroid, and calculate the estimate
    uint8_t i_closest_to_centroid;
    float min_dist_to_centroid = FLT_MAX;
    for (uint8_t i = 0; i < 6; ++i){
        if (i != i_closest1 && i != i_closest2){  // if point not already in set
            float dist_to_centroid = dist_between_points_2d(centroid, points_of_intersection[i]);
            if (dist_to_centroid < min_dist_to_centroid){
                min_dist_to_centroid = dist_to_centroid;
                i_closest_to_centroid = i;
            }
        }
    }
    coord_2d_t estimate = {
            (points_of_intersection[i_closest_to_centroid].x + centroid.x) / 2,
            (points_of_intersection[i_closest_to_centroid].y + centroid.y) / 2,
    };

    // finally, construct the 3d estimate of the position and return it
    coord_3d_t rov_position = {
            estimate.x,
            estimate.y,
            curr_depth - og_depth,
    };
    return rov_position;
}
