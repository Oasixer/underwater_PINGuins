#include <stdio.h>

#include "unit_tests.h"

int main() {
    bool ring_buffer_pass = test_ring_buffer_read_and_replace();
    bool complex_pass = test_complex_type();
    bool fourier_pass = test_fourier();
    bool multilateration_pass = test_multilateration();
    bool calculate_coordinates_pass = test_calculate_coordinates();

    bool all_tests_pass = ring_buffer_pass
                          && complex_pass
                          && fourier_pass
                          && multilateration_pass
                          && calculate_coordinates_pass;
    if (all_tests_pass){
        printf("All tests pass!!\n\n");
    } else {
        printf("Tests Failed :(\n\n");
    }
    return all_tests_pass - 1;
}
