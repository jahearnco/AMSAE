/*
  KineticPulseProfile.h - Library for KineticPulseProfile props and methods
*/
#ifndef KineticPulseProfile_h
#define KineticPulseProfile_h
#include <Arduino.h>

struct KineticPulseProfile
{
  double zEst;
  boolean magForceUp;
  unsigned long power;
  uint8_t confidence;
  uint8_t interruptType;

  KineticPulseProfile(){
	  zEst = 0.0;
	  magForceUp = true;
	  power = 50000L;
	  confidence = 100;
	  interruptType = 0;
  }
};

#endif
