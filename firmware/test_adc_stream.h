#ifndef FYDP_TEST_ADC_STREAM_H
#define FYDP_TEST_ADC_STREAM_H

#include "src/NativeEthernet/src/NativeEthernet.h"

void test_adc_stream_setup();
bool test_adc_stream_loop(bool);
void timer_callback(void);
void adc_isr();
bool check_bytes(uint16_t n, EthernetClient& client);

#endif