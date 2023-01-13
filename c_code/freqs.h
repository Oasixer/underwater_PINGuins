#ifndef FYDP_FREQUENCIES_H
#define FYDP_FREQUENCIES_H

#include <stdint.h>

/* Define constants for DFT*/
#define F_SAMPLE 500000  // Hz

#define N_STATIONARY 3  // number of stationary nodes
#define N_ALL_NODES 4  // number of all nodes (N_STATIONARY + 1)
#define I_ROV 0  // Index of the ROV in all arrays

#define N_FREQUENCIES 6  // number of frequencies

/* Define different frequencies to encode messages*/
#define F_LOW 10000  // Hz
#define F_HIGH 18000  // Hz
#define F_STATIONARY_1 26000  // Hz
#define F_STATIONARY_2 34000  // Hz
#define F_STATIONARY_3 42000  // Hz
#define F_ALL 48000  // Hz

enum FREQ_INDICES{
    IDX_F_LOW,
    IDX_F_HIGH,
    IDX_F_STATIONARY_1,
    IDX_F_STATIONARY_2,
    IDX_F_STATIONARY_3,
    IDX_F_ALL,
};

static uint16_t FREQUENCIES[N_FREQUENCIES] = {
        F_LOW,
        F_HIGH,
        F_STATIONARY_1,
        F_STATIONARY_2,
        F_STATIONARY_3,
        F_ALL,
};

#endif