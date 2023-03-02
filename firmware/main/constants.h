#ifndef FYDP_CONSTANTS_H
#define FYDP_CONSTANTS_H

#include <stdint.h>

/* Define constants for DFT*/
#define F_SAMPLE 500000  // Hz
#define ADC_PERIOD 2
#define MAX_SZ_WINDOW 1250  // Samples

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


// #define INACTIVE_DURATION_AFTER_BEEP 195000
// #define INACTIVE_DURATION_BEFORE_TALKING 200000
#define MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD 5000

/* constants for message */
#define MSG_LENGTH 50  // bits

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
#define MAX_N_TRIPS 500

/* constants for calculating distance*/
#define SPEED_SOUND 1351.89889440064462178270
#define MARCO_POLO_TIME_DELAY_US 200056.07745910956873558462

/* constants for float value comparison*/
#define EPSILON 0.000001

#endif