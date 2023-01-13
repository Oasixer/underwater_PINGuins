cmake_minimum_required(VERSION 3.24)
project(c_code C)

set(CMAKE_C_STANDARD 17)

add_executable(c_code
        complex_type.c
        fourier.c
        freqs.c
        main.c
        multilateration.c
        positions.c
        ring_buffer.c
        unit_tests.c
        utils.c
        )
