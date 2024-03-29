#include "dac_driver.h"
#include "relay.h"
#include "talker.h"
#include "utils.h"

// #define ASSUME_NODES_AT_ROV_DEPTH 1
Talker::Talker(config_t* _config){
    config = _config;
}

uint64_t Talker::begin_yell_5ms(uint16_t frequency_id){
    freq = get_freq(frequency_id);
    switch_relay_to_send_5ms();

    ts_start = micros();
    yell(); // begin immediately for precision
    return ts_start;
}

bool Talker::yell(){
    if (micros() - ts_start < config->micros_send_duration){
        dac_set_analog_float(config->max_amplitude_factor * sinf(2 * M_PI * freq / 1000000 * (float)(micros() % (1000000 / freq))));
        return false;
    }
    return true;
}