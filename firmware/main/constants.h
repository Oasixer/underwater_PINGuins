#ifndef FYDP_CONSTANTS_H
#define FYDP_CONSTANTS_H

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

#define DFT_THRESHOLD 0.005

#define INACTIVE_DURATION_AFTER_PEAK 17500
#define MICRO_SECONDS_TO_FIND_PEAK 5000  // TODO: CHECK
#define MICRO_SECONDS_BETWEEN_RECEIVE_AND_PEAK 2500
#define MICRO_SECONDS_SEND_DURATION 2500
#define INACTIVE_DURATION_AFTER_BEEP 17500

/* constants for message */
#define MSG_LENGTH 50  // bits

/* constants for DAC */
#define DAC_AMPLITUDE 500
#define DAC_CENTER 11715

#endif