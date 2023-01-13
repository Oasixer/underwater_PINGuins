#include <stdio.h>

#include "unit_tests.h"

int main() {
    bool ring_buffer_pass = test_ring_buffer_read_and_replace();
    bool complex_pass = test_complex_type();
    bool fourier_pass = test_fourier();
    return 0;
}
