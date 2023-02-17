#include <ADC.h>
// and IntervalTimer
#include <IntervalTimer.h>

const int readPin = A17;
const int period = 2; // us

ADC *adc = new ADC(); // adc object

IntervalTimer timer; // timer

int startTimerValue = 0;

volatile uint64_t num_times_isr_called = 0;
uint64_t last_micros_printed = 0;
volatile uint16_t latest_adc_value = 0;

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

  Serial.println("Starting Timer");

  // start the timers, if it's not possible, startTimerValuex will be false
  startTimerValue = timer.begin(timer_callback, period);

  adc->adc0->enableInterrupts(adc_isr);

  Serial.println("Timers started");

  delay(500);
}

void loop() {

  if (startTimerValue == false) {
    Serial.println("Timer setup failed");
  }

  if (micros() - last_micros_printed >= 1000000){
    Serial.printf("ISR was called %i times, latest value %i\n", (uint32_t)num_times_isr_called, latest_adc_value);
    num_times_isr_called = 0;
    last_micros_printed = micros();
  }
}

// This function will be called with the desired frequency
// start the measurement
void timer_callback(void) {
  adc->adc0->startSingleRead(readPin);
}

// when the measurement finishes, this will be called
void adc_isr() {
  latest_adc_value = adc->adc0->readSingle();

  num_times_isr_called++;

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
