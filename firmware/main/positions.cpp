#include "positions.h"
#include <wiring.h> // for pi

#define MIN_2D_DISTANCE 0.001  // m

float cosine_rule(float adj1, float adj2, float opp){
    float cos_angle = (powf(adj1, 2) + powf(adj2, 2) - powf(opp, 2)) / (2. * adj1 * adj2);
    // ensure that its between
    if (cos_angle > 1.0){
        cos_angle = 1.0;
    } else if (cos_angle < -1.0){
        cos_angle = -1.0;
    }
    return acosf(cos_angle);
}

bool calculate_coordinates(coord_3d_t *coords, distances_t dists_3d[N_ALL_NODES], float depths[N_ALL_NODES]){
    // project distances to 2D
    distances_t dists_2d[N_ALL_NODES];
    for (uint8_t i = 0; i < N_ALL_NODES; i++){
        for (uint8_t j = 0; j < N_ALL_NODES; j++){
            if (i != j){
                dists_2d[i].dist[j] = convert_to_2d(dists_3d[i].dist[j], depths[i], depths[j]);
            } else {
                dists_2d[i].dist[j] = 0.0;
            }
        }
    }

    // check if possible to calculate positions
    bool possible_to_calculate = true;

    // check that no pair of nodes are directly above each other
    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
        for (uint8_t j = 0; j < N_STATIONARY; ++j){
            if (i != j && dists_2d[i].dist[j] < MIN_2D_DISTANCE){
                possible_to_calculate = false;
            }
        }
    }

    if (possible_to_calculate){
        // create angles for all stationary nodes
        float angles[N_ALL_NODES];

        // the angle to the ROV and node 1 are defined as 0
        angles[I_ROV] = 0.0;
        angles[1] = 0.0;

        // use the cosine rule to get angle to node 2
        // the angle to node 2 is defined as positive
        angles[2] = cosine_rule(dists_2d[I_ROV].dist[1], dists_2d[I_ROV].dist[2], dists_2d[1].dist[2]);

        // calculate all the possible values of angle to node 3
        float guess_using_1[2];
        guess_using_1[0] = cosine_rule(dists_2d[I_ROV].dist[1], dists_2d[I_ROV].dist[3], dists_2d[1].dist[3]);
        guess_using_1[1] = -guess_using_1[0];

        // calculate all the possible values of angle to node 3
        float guess_using_2[2];
        float angle_2_to_3 = cosine_rule(dists_2d[I_ROV].dist[2], dists_2d[I_ROV].dist[3], dists_2d[2].dist[3]);
        guess_using_2[0] =  angle_2_to_3 + angles[2];
        guess_using_2[1] = -angle_2_to_3 + angles[2];
        // ensure that all angles are between -pi and pi
        for (uint8_t i = 0; i < 2; ++i){
            if (guess_using_2[i] > M_PI){
                guess_using_2[i] -= 2*M_PI;
            }
        }

        // get the most consistent angle 3
        float angle3_pair_averages[4] = {
                (guess_using_1[0] + guess_using_2[0]) / 2,
                (guess_using_1[0] + guess_using_2[1]) / 2,
                (guess_using_1[1] + guess_using_2[0]) / 2,
                (guess_using_1[1] + guess_using_2[1]) / 2,
        };
        float angle3_pair_diff[4] = {
                fabsf(guess_using_1[0] - guess_using_2[0]),
                fabsf(guess_using_1[0] - guess_using_2[1]),
                fabsf(guess_using_1[1] - guess_using_2[0]),
                fabsf(guess_using_1[1] - guess_using_2[1]),
        };
        float min_diff = FLT_MAX;
        uint8_t i_min_diff;
        for (uint8_t i = 0; i < 4; ++i){
            if (angle3_pair_diff[i] < min_diff){
                min_diff = angle3_pair_diff[i];
                i_min_diff = i;
            }
        }
        angles[3] = angle3_pair_averages[i_min_diff];  // the actual angle3 is the most consistent angle

        // now that we know the 2d distances, angles, and elevation differences between nodes, we can calculate coords
        for (uint8_t i = 0; i < N_ALL_NODES; ++i){
            coords[i].x = dists_2d[I_ROV].dist[i] * cosf(angles[i]); // x = d*cos(angle)
            coords[i].y = dists_2d[I_ROV].dist[i] * sinf(angles[i]); // y = d*cos(angle)
            coords[i].z = depths[i] - depths[I_ROV]; // z-coordinate is the difference in depth between node and ROV
        }
    }
    return possible_to_calculate;
}