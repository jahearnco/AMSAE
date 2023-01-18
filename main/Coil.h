/*
  Coil.h - Library for PulseCycleManager props and methods
*/
#ifndef Coil_h
#define Coil_h
#include <AMSAEUtils.h>

class Coil
{
    /**
       keep thinking on this. tOn needs to be updated often and never be out of bounds
       primary concern is esp32 malfunction and pin kept high. after that it's not turning
       off coil in some unforseen circumstances

       tOn is set to 0 in constructor
       tOn is set to non 0 in one place ONLY and that should be enforced somehow. maybe
       it takes a parameter that would be incorrect if called within some other scope? I could
       declare a var that makes it clear to not call it elsewhere? this is a good idea to start
    */

    /**
       need logic that sets polarNorthUp based on incoming zEst. problem is that coil doesn't know zEst.
       begs the question: is polarNorthUp a coil prop or a pulse prop???
       coil has current and direction of current. that means polarNorthUp so it's a coil prop. now where is it set?
       Pulse is what sets Coil props so ... yeah. good answer!

       what is the logic? 1.figure out livezones 2.how to iterate through livezones? if zEst comes in as 87.7,
       need fast way to determine which coils are live and their polarNorthUp, right? yep. this also determines pririty
       which is pulsecycle logic. got it! flesh out these 2 methods! coil.setPolarity from within Pulse and setPrioty
       from within PulseCycle
    */
  private:
    static uint8_t priorityTrio[3];

    boolean pulsing;

    uint8_t cOrder;
    boolean isDummy;
    boolean polarNorthUp;

    uint8_t pinNumUp;
    uint8_t pinNumDown;
    uint8_t highPinNum;

    unsigned long pulseWidth;//micros
    unsigned long tFullCycle;

    unsigned long tEndPulse;
    unsigned long tForcedRest;

    unsigned long tOnPulseAve;//micros

    unsigned long tOnTotal;//micros
    unsigned long tOnMax;//micros
    double dRestPerLifetimeMin;

    unsigned long tLifetime;
    unsigned long tBeginPulse;
    unsigned long tBeginFirstPulse;
    unsigned long tOn;//micros
    double dRestPerLifetime;
    double normalizingFactor;

    double zMidLiveRangeBottom;

    double liveRangeMidpoints[coilspace::MAG_NUM - 1]; //not obvious. requirement is that coil *can* (not must) be live at both ends, so a mag is close to live range at both ends

    void fieldOn();
    void fieldsOff();

    void handleEndPulse(unsigned long tc, long tDelayOffset);
    void handleBeginPulse(unsigned long tc, long tDelayOffset);
    void handleNoChangeOn(long tdo);
    void handleNoChangeOff();

    uint8_t evaluateRunStatus(uint8_t runStatus, long tdo, boolean coilIsPulsing);
    void setHighPinNum();
    void initProps();

  public:
    Coil();

    void init(uint8_t o, uint8_t p1, uint8_t p2, unsigned long tMax, double dRestMinPerSecFrac, unsigned long tRest, double zMidBottom, double nFactor, boolean dummyCoil);

    void allPinsLow(unsigned long tc);

    uint8_t healthCheck(unsigned long tc);
    uint8_t getOrder();

    double getDRestPerLifetime();
    double getNormalizingFactor();

    unsigned long getTEndPulse();
    unsigned long getTForcedRest();
    unsigned long getTOnMax();
    unsigned long getTOn();

    uint8_t* getPriorityTrio(double zEst, boolean magForceUp);
    boolean isReady(unsigned long tc);

    uint8_t runPulseSchedule(unsigned long tc, long tDelayOffset);

    void setPulseWidth(unsigned long pw);
    void setDRestPerLifetimeMin(double dRestMin);

    void finalizeDummyPulsing(unsigned long tc, long tDelayOffset);

    boolean isADummy();
};

#endif
