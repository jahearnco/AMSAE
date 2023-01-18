//file: main.cpp
#include <LoopManager.h>

LoopManager* lMan;
void setup(void){
  using namespace adcspace;

  Serial.begin(115200);
  pinMode(adcspace::testInputPin1, OUTPUT);
  delay(Brain::primingDelayMs);
  Serial.println("");
  Serial.println("@ main::setup config ADC ... ");
  adc1_config_width(adcWidth);
  adc1_config_channel_atten((adc1_channel_t)adcChannel1, adcAtten);
  adc2_config_channel_atten((adc2_channel_t)adcChannel2, adcAtten);
  Serial.println("");
  Serial.println("@ main::setup creating and initializing singletons ... ");
  lMan = lMan->getInstance();
}

boolean initializationLoop(void){
	lMan->doLoopTask();
	return Brain::containersAndManagersAreInitialized();
}

void loop(void){
	lMan->doLoopTask();
	//lMan->testPulsePins();
}

extern "C" void app_main(void){
	initArduino();
    setup();
    while (1) { boolean initialized = initializationLoop(); if (initialized) break; }
    while (1) { loop(); }
}
