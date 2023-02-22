#include <ADC.h>
// and IntervalTimer
#include <IntervalTimer.h>

const int readPin = A17;
const int period = 2; // us

ADC *adc = new ADC(); // adc object

IntervalTimer timer; // timer

int startTimerValue = 0;

volatile uint64_t num_times_isr_called = 0;
uint64_t micros_since_start = 0;

#define N_SAMPLES_TO_BUFFER 200000
uint16_t adc_buffer[N_SAMPLES_TO_BUFFER] = {0};

void setup() {
  pinMode(readPin, INPUT_DISABLE);

  Serial.begin(9600);

  delay(1000);

  ///// ADC0 ////
  adc->adc0->setAveraging(0);  // set number of averages
  adc->adc0->setResolution(12); // set bits of resolution
  adc->adc0->setConversionSpeed(
      ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed
  adc->adc0->setSamplingSpeed(
      ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed

  adc->adc0->enableInterrupts(adc_isr);
}

char command = 0;

void loop() {
  if (Serial.available()){
    command = Serial.read();
    if (command == 'g') { // start reading and storing
      Serial.println("Starting data collection");
      delay(100); // start after 100 ms
      startTimerValue = timer.begin(timer_callback, period);
      if (startTimerValue == false) {
        Serial.println("Timer setup failed");
      }
      else {
        micros_since_start = micros();
      }
    }
  }

  if (num_times_isr_called >= N_SAMPLES_TO_BUFFER){
    uint32_t micros_recorded = (uint32_t)(micros() - micros_since_start);
    timer.end();
    Serial.printf("It took %i micros\n", micros_recorded);
    num_times_isr_called = 0;
    for (uint32_t i = 0; i < N_SAMPLES_TO_BUFFER; ++i){
      Serial.printf("%i,", adc_buffer[i]);
    }
    Serial.println();
  }
}

// This function will be called with the desired frequency
// start the measurement
void timer_callback(void) {
  if (num_times_isr_called < N_SAMPLES_TO_BUFFER){
    adc->adc0->startSingleRead(readPin);
  }
}

// when the measurement finishes, this will be called
void adc_isr() {
  adc_buffer[num_times_isr_called++] = adc->adc0->readSingle();

  // restore ADC config if it was in use before being interrupted by the analog
  // timer
  if (adc->adc0->adcWasInUse) {
    // restore ADC config, and restart conversion
    adc->adc0->loadConfig(&adc->adc0->adc_config);
    // avoid a conversion started by this isr to repeat itself
    adc->adc0->adcWasInUse = false;
  }

  // digitalWriteFast(ledPin+2, !digitalReadFast(ledPin+2));

#if defined(__IMXRT1062__) // Teensy 4.0
  asm("DSB");
#endif
}
