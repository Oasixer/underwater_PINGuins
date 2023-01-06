#ifndef FREQUENCIES_H
#define FREQUENCIES_H

#include <stdint.h>

/* Define constants for DFT*/
#define F_SAMPLE 500000  // Hz

#define N_STATIONARY 3  // number of stationary nodes
#define N_ALL_NODES 4  // number of all nodes (N_STATIONARY + 1)
#define I_ROV 0  // Index of the ROV in all arrays

#define N_FREQUENCIES 6  // number of frequencies

/* Define different frequencies to encode messages*/
const uint16_t F_LOW = 10000;  // Hz
const uint16_t F_HIGH =  18000;  // Hz
const uint16_t F_STATIONARY[N_STATIONARY] = {26000, 34000, 42000}; // Hz
const uint16_t F_ALL = 48000;  // Hz


#endif