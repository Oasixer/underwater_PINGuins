#include "positions.h"
#include <wiring.h> // for pi
#include <Arduino.h>

#define MIN_2D_DISTANCE 0.001  // m

float valid_sqrt(float val){
    if (val > 0.0){
        return sqrt(val);
    } else {
        return 0.0;
    }
}

float get_x(float right, float towards, float across){
    return (powf(right, 2) + powf(towards, 2) - powf(across, 2)) / (2. * right);
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
        // Place the ROV as the origin
        coords[0].x = 0;
        coords[0].y = 0;

        // Place node 1 to right
        coords[1].x = dists_2d[0].dist[1];
        coords[1].y = 0.0;

        // Place node 2 in the +ve y direction
        coords[2].x = get_x(dists_2d[0].dist[1], dists_2d[0].dist[2], dists_2d[1].dist[2]);
        coords[2].y = valid_sqrt(powf(dists_2d[0].dist[2], 2) - powf(coords[2].x, 2));

        // calculate x3
        coords[3].x = get_x(dists_2d[0].dist[1], dists_2d[0].dist[3], dists_2d[1].dist[3]);

        // calculate all possible values of y3
        float possible_y3s[4] = {
            valid_sqrt(powf(dists_2d[0].dist[3], 2) - powf(coords[3].x, 2)),
            -valid_sqrt(powf(dists_2d[0].dist[3], 2) - powf(coords[3].x, 2)),
            coords[2].y + valid_sqrt(powf(dists_2d[2].dist[3], 2) - powf(coords[2].x - coords[3].x, 2)),
            coords[2].y - valid_sqrt(powf(dists_2d[2].dist[3], 2) - powf(coords[2].x - coords[3].x, 2)),
        };

        // get the most consistent value for y3
        float y3_averages[4] = {
            (possible_y3s[0] + possible_y3s[2]) / (float)2,
            (possible_y3s[0] + possible_y3s[3]) / (float)2,
            (possible_y3s[1] + possible_y3s[2]) / (float)2,
            (possible_y3s[1] + possible_y3s[3]) / (float)2,
        };
        float y3_diffs[4] = {
            fabsf(possible_y3s[0] - possible_y3s[2]),
            fabsf(possible_y3s[0] - possible_y3s[3]),
            fabsf(possible_y3s[1] - possible_y3s[2]),
            fabsf(possible_y3s[1] - possible_y3s[3]),
        };

        float min_diff = FLT_MAX;
        uint8_t i_min_diff = 0;
        for (uint8_t i = 0; i < 4; ++i){
            if (y3_diffs[i] < min_diff){
                min_diff = y3_diffs[i];
                i_min_diff = i;
            }
        }
        coords[3].y = y3_averages[i_min_diff]; // the actual y3 is the most consistent value

        // now that we know the 2d distances, store depths
        for (uint8_t i = 0; i < N_ALL_NODES; ++i){
            coords[i].z = depths[i]; // z-coordinate is the depth
        }
    }
    return possible_to_calculate;
}


bool test_calculate_coordinates(){
    // Test in the following coordinates
    coord_3d_t origin_coord = {0, 0, -1};
    coord_3d_t stationary_1_coords[2] = {{1, 0, -1}, {2.5, 0, -3}};  // both in 1st quadrant
    coord_3d_t stationary_2_coords[3] = {{3, 2, -2}, {-1, 4, -1}, {0, 3, -2}}; // 1st, 2nd quadrant, and y axis
    coord_3d_t stationary_3_coords[4] = {{2, 5, -4}, {-4, 1, -1}, {-7, -2, 2}, {2, 5, -1}}; // each quadrant
    float rov_depths[2] = {-5, -10};  // two different depths of the ROV

    bool quadrant_tests_pass = true;

    for (uint8_t idx_depth = 0; idx_depth < 2; idx_depth++) {
        for (uint8_t idx_stationary_1_coord = 0; idx_stationary_1_coord < 2; idx_stationary_1_coord++) {
            for (uint8_t idx_stationary_2_coord = 0; idx_stationary_2_coord < 3; idx_stationary_2_coord++) {
                for (uint8_t idx_stationary_3_coord = 0; idx_stationary_3_coord < 4; idx_stationary_3_coord++) {
                    // create array of actual coordinates
                    coord_3d_t actual_coords[N_ALL_NODES] = {
                            origin_coord,
                            stationary_1_coords[idx_stationary_1_coord],
                            stationary_2_coords[idx_stationary_2_coord],
                            stationary_3_coords[idx_stationary_3_coord],
                    };

                    // create depths array
                    float depths[N_ALL_NODES] = {0};
                    for (uint8_t i = 0; i < N_ALL_NODES; ++i) {
                        depths[i] = actual_coords[i].z;
                    }

                    // create array of distances between nodes
                    distances_t distances[N_ALL_NODES] = {{0}};
                    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
                        for (uint8_t j = 0; j < N_ALL_NODES; ++j){
                            distances[i].dist[j] = dist_between_points_3d(actual_coords[i], actual_coords[j]);
                        }
                    }

                    // create a calculated array and call the calculate_coordinates function
                    coord_3d_t calculated_coords[N_ALL_NODES] = {0};
                    bool test_passed = calculate_coordinates(calculated_coords, distances, depths);

                    // verify if correct
                    for (int i = 0; i < N_ALL_NODES; ++i){
                        quadrant_tests_pass &= (fabsf(calculated_coords[i].x - actual_coords[i].x) < 1e-5
                                                && fabsf(calculated_coords[i].y - actual_coords[i].y) < 1e-5
                                                && fabsf(calculated_coords[i].z - actual_coords[i].z) < 1e-5);
                    }

                    if(!test_passed){
                        quadrant_tests_pass = false;
//                        Serial.println("actual: (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f)",
//                               actual_coords[0].x, actual_coords[0].y, actual_coords[0].z,
//                               actual_coords[1].x, actual_coords[1].y, actual_coords[1].z,
//                               actual_coords[2].x, actual_coords[2].y, actual_coords[2].z,
//                               actual_coords[3].x, actual_coords[3].y, actual_coords[3].z);
//                        Serial.println("calculated: (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f)",
//                               calculated_coords[0].x, calculated_coords[0].y, calculated_coords[0].z,
//                               calculated_coords[1].x, calculated_coords[1].y, calculated_coords[1].z,
//                               calculated_coords[2].x, calculated_coords[2].y, calculated_coords[2].z,
//                               calculated_coords[3].x, calculated_coords[3].y, calculated_coords[3].z);
                    }
                }
            }
        }
    }
    if (quadrant_tests_pass){
        Serial.println("Calibration quadrant tests Passed");
    } else {
        Serial.println("Calibration quadrant tests Failed");
    }

    /* for completeness, test that edge cases won't break the code */
    // if ROV collinear with nodes, should not cause an issue
    coord_3d_t rov_collinear_actual[N_ALL_NODES] = {
            {0, 0, -1},
            {1, 0, -1},
            {-2, 0, -1},
            {4, -3, -1}
    };
    distances_t distances_collinear[N_ALL_NODES] = {
            {{0, 1, 2, 5}},
            {{1, 0, 3, sqrtf(18)}},
            {{2, 3, 0, sqrtf(45)}},
            {{5, sqrtf(18), sqrtf(45), 0}}
    };
    float collinear_depths[N_ALL_NODES] = {-5, -5, -5, -5};
    coord_3d_t rov_collinear_calculated[N_ALL_NODES] = {{}};
    // verify if doesn't break. Don't check values
    bool rov_collinear_test_passed = calculate_coordinates(rov_collinear_calculated, distances_collinear, collinear_depths);;

    // test if all three nodes are collinear
    coord_3d_t all_three_collinear_coords_actual[N_ALL_NODES] = {
            {0, 0, -1},
            {1, 0, -1},
            {1, 1, -1},
            {1, -1, -1},
    };
    distances_t all_three_collinear_distances[N_ALL_NODES] = {
            {{0, 1, sqrtf(2), sqrtf(2)}},
            {{1, 0, 1, 1}},
            {{sqrtf(2), 1, 0, 2}},
            {{sqrtf(2), 1, 2, 0}},
    };
    coord_3d_t all_three_collinear_calculated[N_ALL_NODES] = {{}};
    // verify if doesn't break. Don't check values
    bool all_three_collinear_test_passed = calculate_coordinates(all_three_collinear_calculated, all_three_collinear_distances, collinear_depths);

    bool all_tests_pass = quadrant_tests_pass && rov_collinear_test_passed && all_three_collinear_test_passed;
    if (all_tests_pass){
        Serial.println("Calibration all tests Passed\n");
    } else {
        Serial.println("Calibration tests Failed\n");
    }
    return all_tests_pass;
}
