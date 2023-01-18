/*
  Pulse.h - Library for PulseManager props and methods
*/
#ifndef Pulse_h
#define Pulse_h
#include <CoilContainer.h>

const uint8_t P_PRE_INIT = 67;
const uint8_t P_INIT = 68;

class Pulse
{
  private:
    Coil* coil;
    
    uint8_t cOrder;
    unsigned long tTimerStart;
    unsigned long pulseWidth;//micros    
    unsigned long tStartDelay;//micros

    unsigned long tEndLastPulse;
    unsigned long tForcedRest;

    uint8_t pStatus;
    boolean enabled;

    double adjustedDRestPerLifetimeMin;
    double normalizingFactor;

  public:
    Pulse();
    
    unsigned long init(uint8_t priority, unsigned long tStart, Coil* coil, uint8_t type);
    void enable();    
    void disable();  

    uint8_t getStatus(unsigned long tCurrent, Coil* coil);
};

#endif
