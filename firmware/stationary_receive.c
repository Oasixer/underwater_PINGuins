#include "stationary_receive.h"

bool is_currently_receiving = true;  // if true receiving data. else sending
bool is_peak_finding = false;  // to indicate peak finding state
bool is_message_ready = false;

uint64_t ts_last_heard_other_stationary_node = 0;
uint64_t ts_active_start = 0;
uint64_t ts_peak = 0;
uint64_t ts_peak_finding_timeout = 0;

uint8_t idx_freq_detected = 0;
uint8_t idx_last_freq_detected = 0;

uint8_t idx_beep_to_send = 0;

float prev_max_amplitude = 0;

#define MESSAGE_END_INDICATOR 0
uint8_t i_curr_bit = 0;

#define DEFAULT_ANALOG_VALUE 0 // TODO: check that this is true

bool is_freq_id_for_stationary_node(uint8_t id){
    return id == IDX_F_STATIONARY_1 || id == IDX_F_STATIONARY_2 || id == IDX_F_STATIONARY_3;
}

void detect_frequencies() {
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i) {
        if (freq_amplitudes[i] > DFT_THRESHOLD) {
            prev_max_amplitude = freq_amplitudes[i];
            idx_freq_detected = i;
            ts_peak_finding_timeout = micros() + MICROS_TO_FIND_PEAK;
            is_peak_finding = true;
            return;  // this should only be true for one frequency. there's no point in checking the rest
        }
    }
}


void act_on_beep(){
    if (idx_freq_detected == IDX_F_CURR_STATIONARY || idx_freq_detected == IDX_F_ALL){ // if need to respond
        // TODO: trigger relay to switch to sending
        is_currently_receiving = false; // switch to sending state

    } else if (is_freq_id_for_stationary_node(idx_freq_detected)){ // if other stationary node, start timer
        ts_last_heard_other_stationary_node = ts_peak;

    } else if (idx_freq_detected == IDX_F_HIGH) { // if message start indicator
        if (is_freq_id_for_stationary_node(idx_last_freq_detected)) { // if prev freq detected was stationary node
            // stop timer
            // subtract two because of the difference between freq id and stationary id
            delays_between_call_and_response[idx_last_freq_detected-2] = ts_peak - ts_last_heard_other_stationary_node;
        }  // else do nothing
    } // else do nothing
}

void peak_finding(){
    if (micros() >= ts_peak_finding_timeout){  // finished peak finding
        act_on_beep();
        idx_last_freq_detected = idx_freq_detected;  // store for later
        prev_max_amplitude = 0;  // reset to zero for next time
        is_peak_finding = false;  // start next time not in peak finding state
        ts_active_start = ts_peak + INACTIVE_DURATION_AFTER_PEAK;  // be inactive for a while
    } else {
        if (freq_amplitudes[idx_freq_detected] > prev_max_amplitude){
            prev_max_amplitude = freq_amplitudes[idx_freq_detected];
            ts_peak = micros();
        }
    }
}

void receive_mode_hb(){
    if (micros() >= ts_active_start && is_fourier_updated){  // if not in inactive period and fourier updated
        if (is_peak_finding) {
            peak_finding();
        } else {
            detect_frequencies();
        }
        is_fourier_updated = false;  // indicate that already processed the latest sample
    }
}

void add_dist_to_message(uint16_t *message, uint64_t dist){  // TODO: check this is right
    uint64_t data = dist / 10;
    for (uint8_t i = 0; i < 14; ++i){
        if (data >> i % 2){ // if high
            message[i] = F_HIGH;
        } else { // if low
            message[i] = F_LOW;
        }
    }
}

void add_pressure_to_message(uint16_t *message){
    // TODO: add pressure to message
}

void add_checksum_to_message(uint16_t *message){
    // TODO: add checksum to message
}

void create_message(){
    if (idx_freq_detected == IDX_F_ALL){  // message is the ID of curr
        msg_to_send[0] = FREQUENCIES[IDX_F_CURR_STATIONARY];  // send the ID of curr stationary
        msg_to_send[1] = MESSAGE_END_INDICATOR;
    } else {  // message is 50 bits long
        // start indicator
        msg_to_send[0] = F_HIGH;
        msg_to_send[1] = F_HIGH;
        msg_to_send[2] = F_LOW;

        // determine which indices of delays_between_call_and_response to send
        uint8_t indices_to_send[N_STATIONARY-1];
        switch (IDX_CURR_STATIONARY){
            case 1:
                indices_to_send[0] = 1;
                indices_to_send[1] = 2;
                break;
            case 2:
                indices_to_send[0] = 0;
                indices_to_send[1] = 2;
                break;
            case 3:
                indices_to_send[0] = 0;
                indices_to_send[1] = 1;
                break;
        }

        // the first distance
        add_dist_to_message(msg_to_send[3], delays_between_call_and_response[indices_to_send[0]]);

        // the second distance
        add_dist_to_message(msg_to_send[17], delays_between_call_and_response[indices_to_send[1]]);

        // the pressure
        add_pressure_to_message(msg_to_send[31]);

        // the checksum
        add_checksum_to_message(msg_to_send[42]);

        // the end indicator
        msg_to_send[47] = F_LOW;
        msg_to_send[48] = F_LOW;
        msg_to_send[49] = F_HIGH;
    }
}

void send_data(){
    unsigned long micros_since_bit_start = micros() - ts_active_start;
    if (micros_since_bit_start < MICRO_SECONDS_SEND_DURATION){ // keep sending
        set_analog_float(sinf(2*M_PI*msg_to_send[i_curr_bit]*micros_since_bit_start/1000000 - M_PI_2));
    } else { // finished beep
        set_analog(DEFAULT_ANALOG_VALUE);
        ts_active_start += INACTIVE_DURATION_AFTER_BEEP;
        i_curr_bit++;

        if (i_curr_bit == MSG_LENGTH || msg_to_send[i_curr_bit] == MESSAGE_END_INDICATOR){ // if done sending
            is_currently_receiving = true; // switch to receiving
            is_message_ready = false;  // reset for next time
            i_curr_bit = 0;  // reset for next time
            // TODO: switch relay to receiving mode
        }
    }
}

void send_mode_hb(){
    if (!is_message_ready){
        create_message();
    }
    if (micros() >= ts_active_start){
        send_data();
    }
}

void stationary_main_loop(){
    // TODO: check health

    if (is_currently_receiving){
        receive_mode_hb();
    } else {  // sending
        send_mode_hb();
    }

    // TODO: log if needed
}
