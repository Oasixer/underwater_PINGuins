#include "adc_isr.h"
#include "fourier.h"
#include <IntervalTimer.h>
#include <Arduino.h>

#include <ADC.h>
#include <stdint.h>

// variables used for checking health
uint64_t fourier_counter = 0;
uint16_t last_reading = 0;

ADC *adc = new ADC();
float frequency_magnitudes[N_FREQUENCIES] = {0};

// when the measurement finishes, this will be called
void adc_isr() {
    const uint16_t reading = adc->adc0->readSingle();
    // uint16_t val = (uint16_t)roundf(2048.0 * sinf(2 * M_PI * (float)MY_FREQUENCY / 1000000.0 * (float)(micros() % (1000000 / MY_FREQUENCY))) + 2048.0);
    fourier_update(frequency_magnitudes, reading);
    last_reading = reading;

    if (adc->adc0->adcWasInUse) {
        // restore ADC config, and restart conversion
        adc->adc0->loadConfig(&adc->adc0->adc_config);
        // avoid a conversion started by this isr to repeat itself
        adc->adc0->adcWasInUse = false;
    }

    #if defined(__IMXRT1062__) // i.MX RT1062 => Teensy 4.0/4.1
    asm("DSB"); // ensure memory access inside the ISR is complete before returning by inserting DSB (Data Sync Barrier) in ASM.
    #endif
}

float* get_frequency_magnitudes(){
    return frequency_magnitudes;
}
uint16_t* get_last_reading(){
    return &last_reading;
}
uint64_t* get_fourier_counter(){
    return &fourier_counter;
}

void adc_setup(){
    ///// ADC0 ////
    adc->adc0->setAveraging(0);    // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
    adc->adc0->setConversionSpeed(
        ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed
    adc->adc0->setSamplingSpeed(
        ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed

    adc->adc0->enableInterrupts(adc_isr);
}

void adc_timer_callback(void) {
    adc->adc0->startSingleRead(PIEZO_IN_PIN);
    fourier_counter++;
}
