#ifndef FYDP_STATIONARY_RECEIVE_H
#define FYDP_STATIONARY_RECEIVE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "constants.h"

#define IDX_CURR_STATIONARY 1

static const uint8_t IDX_F_CURR_STATIONARY = IDX_CURR_STATIONARY + 1;

static uint16_t msg_to_send[MSG_LENGTH] = {};
static uint64_t delays_between_call_and_response[N_STATIONARY] = {0};  // to time the distance to other nodes

static bool is_fourier_updated = false;
float freq_amplitudes[N_FREQUENCIES] = {0};  // the fourier amplitudes of frequencies
// TODO: setup ADC interrupt with a pointer to ^

#endif
