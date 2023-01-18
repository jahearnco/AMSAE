    /*
  PulseCycle.h - Library for PulseCycleManager props and methods
*/
#ifndef PulseCycle_h
#define PulseCycle_h
#include <Pulse.h>

class PulseCycle
{
  private:
    uint8_t pulseCodeDecoder(unsigned long tc, uint8_t pC1, uint8_t pC2, uint8_t pC3);
    Coil* prioritizedCoils;
    Coil* c0;
    Coil* c1;
    Coil* c2;
    
    Pulse firstPulse;
    Pulse secondPulse;
    Pulse thirdPulse;

    uint8_t pulseType;
    uint8_t interruptType;
    unsigned long power;
    uint8_t confidence;
    boolean magForceUp;
    unsigned long tFullCycle;

    uint8_t pcPulsingCode;
    unsigned long tStartPulseCycle;

    boolean enabled;
    uint8_t myOrder;

    boolean dummy;

    void initPulses();
    
  public:
    PulseCycle();

    unsigned long init(unsigned long po, uint8_t co, boolean mfu, unsigned long ts, Coil** prioritizedCoils, uint8_t pType, uint8_t iType);
    uint8_t runPulseSchedule(unsigned long tc, unsigned long ts);   
    uint8_t getIndex();

    unsigned long getTStartPulseCycle();
    uint8_t getCycleStatus(unsigned long tc, boolean dbg);

    uint8_t getPulseType();
    uint8_t getInterruptType();
    unsigned long getPower();
    uint8_t getConfidence();
    boolean getMagForceUp();

    void setInterruptType(uint8_t type);

    void setDisabled();
    void setEnabled();
    boolean isEnabled();

    uint8_t runCycle(unsigned long tc);

    void setOrder(uint8_t o);
    uint8_t getOrder();
};

#endif
