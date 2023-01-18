/*
  PositionSensor.h - Library for PositionSensorManager props and methods
*/
#ifndef PositionSensor_h
#define PositionSensor_h
#include <Arduino.h>
#include <AMSAEUtils.h>
#include <Brain.h>

class PositionSensor
{
  private:

  public:
	static uint8_t adcSampleCount;

    static vector<uint32_t> adc1_readings;
    static vector<uint32_t> adc2_readings;

    static double lastAdc_readingAve;

    static unsigned long startCycleTick;
    static unsigned long tPing;

    static boolean tPingd;
    static boolean newSamplingCycle;

    static unsigned long psCalcCount;
    static unsigned long tPSCalcTotal;

    static double readSensorInitializer(unsigned long tick, double zReadingMaybe, double zReading, boolean debug);
    static double readSensor(unsigned long tick, double zReadingMaybe, double zReading, boolean debug);
    PositionSensor();
};

#endif
