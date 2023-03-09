#ifndef FYDP_CONSTANTS_H
#define FYDP_CONSTANTS_H

#include <stdint.h>

/* Define constants for DFT*/
#define F_SAMPLE 500000  // Hz
#define ADC_PERIOD 2
#define MAX_SZ_WINDOW 1250  // Samples

// #define MSGS_IN_BUF 100

#define N_STATIONARY 3  // number of stationary nodes
#define N_ALL_NODES 4  // number of all nodes (N_STATIONARY + 1)
#define I_ROV 0  // Index of the ROV in all arrays

#define N_FREQUENCIES 6  // number of frequencies

static uint16_t FREQUENCIES[N_FREQUENCIES] = {
    11000,  // Hz
    18000,  // Hz
    25000,  // Hz
    35000,  // Hz
    41000,  // Hz
    49000,  // Hz
};

#define MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD 5000

/* constants for DAC */
#define DAC_AMPLITUDE 3600  // this results in full 60V range
#define DAC_CENTER 11715

/* constants on the board*/
#define DAC_PIN 40
#define DAC_CLR_PIN 39
#define HV_ENABLE_PIN 18
#define PIEZO_IN_PIN 41
#define NO_LEAK_PIN 14
#define RELAY_PIN 25

/* constants for leak detection*/
#define THERE_IS_A_LEAK false

/* constants for relay*/
#define RELAY_SEND_MODE HIGH
#define RELAY_RECEIVE_MODE LOW

/* constants for serial message indicators*/
static const char message_delimiter = ' ';
static const char message_terminator = '\n';

/* constants for saving trip duration*/
#define MAX_N_TRIPS 50

/* constants for calibration*/
#define MAX_CALIBRATION_CYCLES 50

/* constants for float value comparison*/
#define EPSILON 0.000001

#endif