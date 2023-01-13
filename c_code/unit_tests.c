#include "unit_tests.h"

bool test_ring_buffer_read_and_replace(){
    ring_buffer_t ring_buffer = {0};

    bool is_initialization_good = true;
    for (uint16_t i = 0; i < RING_BUFFER_SIZE; ++i){
        is_initialization_good &= ring_buffer.data[i] == 0;
    }
    is_initialization_good &= ring_buffer.i_curr == 0;
    if (is_initialization_good){
        printf("Ring buffer Initialization Test Passed\n");
    } else {
        printf("Ring buffer Initialization Test Failed\n");
    }

    bool is_read_and_replace_good = true;
    for (uint16_t i = 0; i < RING_BUFFER_SIZE; ++i){
        uint16_t curr = ring_buffer_read_and_replace(&ring_buffer, i % 256);
        is_read_and_replace_good &= curr == 0;
        is_read_and_replace_good &= ring_buffer.i_curr == (i+1) % RING_BUFFER_SIZE;
    }
    for (uint16_t i = 0; i < RING_BUFFER_SIZE; ++i){
        uint16_t curr = ring_buffer.data[i];
        is_read_and_replace_good &= curr == i % 256;
    }
    if (is_read_and_replace_good){
        printf("Ring buffer Read and Replace Test Passed\n");
    } else {
        printf("Ring buffer Read and Replace Test Failed\n");
    }

    bool all_tests_pass = is_read_and_replace_good && is_initialization_good;
    if (all_tests_pass){
        printf("Ring buffer all tests Passed\n\n");
    } else {
        printf("Ring buffer tests Failed\n\n");
    }

    return all_tests_pass;
}

bool test_complex_type(){
    complex_t c1 = {3, 4};
    bool is_magnitude_working = complex_magnitude(c1) == 5.0;
    if (is_magnitude_working){
        printf("Complex Magnitude Test Passed\n");
    } else {
        printf("Complex Magnitude Test Failed\n");
    }

    complex_t sum = complex_add_real(c1, 5);
    bool is_addition_working = sum.real == 8 && sum.imaginary == 4;
    if (is_addition_working){
        printf("Complex Addition Test Passed\n");
    } else {
        printf("Complex Addition Test Failed\n");
    }

    complex_t c2 = {6, -9};
    complex_t product = complex_multiply_complex(c1, c2);
    bool is_multiplication_woring = (fabsf(product.real - 54) < 1e-5) && (fabsf(product.imaginary - -3) < 1e-5);
    if (is_multiplication_woring){
        printf("Complex Multiplication Test Passed\n");
    } else {
        printf("Complex Multiplication Test Failed\n");
    }

    bool all_tests_pass = is_magnitude_working && is_addition_working && is_multiplication_woring;
    if (all_tests_pass){
        printf("Complex all tests Passed\n\n");
    } else {
        printf("Complex tests Failed\n\n");
    }

    return all_tests_pass;
}

bool test_fourier(){
    fourier_initialize();

    float amplitudes[N_FREQUENCIES] = {0};

    // Test a low input
    bool is_no_signal_working = true;
    for (uint16_t i = 0; i < SZ_WINDOW; ++i){
        fourier_update(amplitudes, 2048.0);
    }
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        is_no_signal_working &= amplitudes[i] < 100; // should be low
    }
    if (is_no_signal_working){
        printf("Fourier No Signal Test Passed\n");
    } else {
        printf("Fourier No Signal Test Failed\n");
    }

    // Test frequencies one by one as high
    bool is_individual_working = true;
    for (uint8_t i_freq = 0; i_freq < N_FREQUENCIES; ++ i_freq){
        for (uint16_t i = 0; i < SZ_WINDOW; ++i){
            float t = (float)i / F_SAMPLE;
            uint16_t x = (uint16_t)roundf(2048.0 * sinf(2 * M_PI * FREQUENCIES[i_freq] * t) + 2048.0);
            fourier_update(amplitudes, x);
        }
        for (int j = 0; j < 6; ++j){
            if (i_freq != j){  // should be low
                is_individual_working &= amplitudes[j] < 100;
            } else {  // should be high
                is_individual_working &= (amplitudes[j] - 1280000) < 100;
            }
        }
    }
    if (is_individual_working){
        printf("Fourier Individual Test Passed\n");
    } else {
        printf("Fourier Individual Test Failed\n");
    }

    // test all frequencies high
    bool is_multiple_frequencies_high_working = true;
    for (uint16_t i = 0; i < SZ_WINDOW; ++i){
        float t = (float)i / F_SAMPLE;
        float input = 0;
        for (uint8_t i_freq = 0; i_freq < N_FREQUENCIES; ++i_freq){
            input += (2048.0 * sinf(2 * M_PI * FREQUENCIES[i_freq] * t) + 2048.0) / N_FREQUENCIES;
        }
        uint16_t x = (uint16_t)(roundf(input));
        fourier_update(amplitudes, x);
    }
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        is_multiple_frequencies_high_working &= fabs(amplitudes[i] - 213333.333333) < 100; // should be high
    }
    if (is_multiple_frequencies_high_working){
        printf("Fourier Multiple High Test Passed\n");
    } else {
        printf("Fourier Multiple High Test Failed\n");
    }

    bool all_tests_pass = is_no_signal_working && is_individual_working && is_multiple_frequencies_high_working;
    if (all_tests_pass){
        printf("Fourier all tests Passed\n\n");
    } else {
        printf("Fourier tests Failed\n\n");
    }

    return all_tests_pass;
}

bool test_multilateration(){
    /* Note that it is expected that multilateration won't work if:
     * - Any 2D distance is 0 (ie ROV directly over a node)
     * - The 3 stationary nodes are collinear (in 2D)
     */
    // Test in the following coordinates
    coord_3d_t origin_coord = {0, 0, 0};
    coord_3d_t stationary_1_coord = {1, 0, 0};
    coord_3d_t stationary_2_coords[3] = {{3, 2, -2}, {-1, 4, 2}, {0, 3, -2}}; // 1st, 2nd quadrant, and y axis
    coord_3d_t stationary_3_coords[4] = {{2, 5, -4}, {-4, 1, 0}, {-7, -2, 2}, {2, 5, -1}}; // each quadrant
    coord_3d_t rov_coords[5] = {{0, 0, 0}, {1, 1, 1}, {-1, 1, 2}, {-1, -1, -1}, {1, -1, -2}}; // center and each quadrant
    float og_depth = -5;

    bool quadrant_tests_pass = true;

    for (uint8_t idx_stationary_2_coord = 0; idx_stationary_2_coord < 3; idx_stationary_2_coord++){
        for (uint8_t idx_stationary_3_coord = 0; idx_stationary_3_coord < 4; idx_stationary_3_coord++){
            for (uint8_t idx_rov_coord = 0; idx_rov_coord < 5; idx_rov_coord++){
                coord_3d_t coords_3d[N_ALL_NODES] = {
                        origin_coord,
                        stationary_1_coord,
                        stationary_2_coords[idx_stationary_2_coord],
                        stationary_3_coords[idx_stationary_3_coord],
                };
                coord_3d_t rov_coord = rov_coords[idx_rov_coord];
                float curr_depth = og_depth + rov_coord.z;

                float dists_3d[N_ALL_NODES] = {0};
                for (uint8_t i = 1; i < N_ALL_NODES; ++i){
                    dists_3d[i] = dist_between_points_3d(rov_coord, coords_3d[i]);
                }

                coord_3d_t position = multilaterate(coords_3d, dists_3d, curr_depth, og_depth);

                bool test_passed = (fabsf(position.x - rov_coord.x) < 1e-5
                                    && fabsf(position.y - rov_coord.y) < 1e-5
                                    && fabsf(position.z - rov_coord.z) < 1e-5);
                if(!test_passed){
                    quadrant_tests_pass = false;
                    printf("coords_3d: (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f)\n",
                           coords_3d[0].x, coords_3d[0].y, coords_3d[0].z,
                           coords_3d[1].x, coords_3d[1].y, coords_3d[1].z,
                           coords_3d[2].x, coords_3d[2].y, coords_3d[2].z,
                           coords_3d[3].x, coords_3d[3].y, coords_3d[3].z);
                    printf("dists3d: %.2f, %.2f, %.2f, estimate: (%.2f, %.2f, %.2f), actual: (%.2f, %.2f, %.2f)\n\n",
                           dists_3d[1], dists_3d[2], dists_3d[3],
                           position.x, position.y, position.z,
                           rov_coord.x, rov_coord.y, rov_coord.z);
                }
            }
        }
    }
    if (quadrant_tests_pass){
        printf("Multilateration quadrant tests Passed\n");
    } else {
        printf("Multilateration quadrant tests Failed\n");
    }

    /* for completeness, test that edge cases won't break the code */
    // if the stationary nodes are collinear, it is assumed that the ROV is in one of the two options (positive)
    coord_3d_t collinear_stationary_coords[N_ALL_NODES] = {
            {0, 0, 0},
            {1, 0, 0},
            {2, 0, 0},
            {3, 0, 0},
    };
    float collinear_stationary_dists_3d[N_ALL_NODES] = {0, 1, sqrtf(2), sqrtf(5)}; // ie ROV at (1, 1, 0) or (1, -1, 0)
    coord_3d_t collinear_stationary_estimated_position = multilaterate(collinear_stationary_coords, collinear_stationary_dists_3d, -5, -5);
    bool stationary_collinear_test_passed = (fabsf(collinear_stationary_estimated_position.x - 1) < 1e-5
                                  && fabsf(collinear_stationary_estimated_position.y - 1) < 1e-5
                                  && fabsf(collinear_stationary_estimated_position.z - 0) < 1e-5);
    if (stationary_collinear_test_passed){
        printf("Multilateration stationary collinear tests Passed\n");
    } else {
        printf("Multilateration stationary collinear tests Failed\n");
    }

    // if ROV collinear with nodes, should not cause an issue
    coord_3d_t collinear_rov_coords[N_ALL_NODES] = {
            {0, 0, 0},
            {1, 0, 0},
            {2, 0, 0},
            {3, 4, 0},
    };
    float rov_collinear_dists_3d[N_ALL_NODES] = {0, 1, 2, 5};  // ie ROV at (0, 0, 0)
    coord_3d_t rov_collinear_estimated_position = multilaterate(collinear_rov_coords, rov_collinear_dists_3d, -5, -5);
    bool rov_collinear_test_passed = (fabsf(rov_collinear_estimated_position.x - 0) < 1e-5
                                      && fabsf(rov_collinear_estimated_position.y - 0) < 1e-5
                                      && fabsf(rov_collinear_estimated_position.z - 0) < 1e-5);
    if (rov_collinear_test_passed){
        printf("Multilateration ROV collinear tests Passed\n");
    } else {
        printf("Multilateration ROV collinear tests Failed\n");
    }

    // if ROV on top of nodes
    coord_3d_t superposition_coords[N_ALL_NODES] = {
            {0, 0, 0},
            {1, 0, 0},
            {2, 0, 0},
            {4, 4, 0},
    };
    float superposition_dists_3d[N_ALL_NODES] = {0, 0, 1, 5}; // ie ROV at (1, 0, 0)
    coord_3d_t superposition_estimate = multilaterate(superposition_coords, superposition_dists_3d, -5, -5);
    bool superposition_test_passed = (fabsf(superposition_estimate.x - 1) < 1e-5
                                      && fabsf(superposition_estimate.y - 0) < 1e-5
                                      && fabsf(superposition_estimate.z - 0) < 1e-5);
    if (superposition_test_passed){
        printf("Multilateration superposition tests Passed\n");
    } else {
        printf("Multilateration superposition tests Failed\n");
    }

    bool all_tests_pass = quadrant_tests_pass && rov_collinear_test_passed &&
                          stationary_collinear_test_passed && superposition_test_passed;
    if (all_tests_pass){
        printf("Multilateration all tests Passed\n\n");
    } else {
        printf("Multilateration tests Failed\n\n");
    }

    return all_tests_pass;
}
