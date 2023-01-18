/*
  KineticsManager.h - Library for KineticsManagerManager props and methods
*/
#ifndef KineticsManager_h
#define KineticsManager_h
#include <TimingManager.h>
#include <KineticProfileTickHistoryContainer.h>

/**
 * 
 *gather position info, calc rates of change, calc likely position of magnets which is same as assigning coil hierarchy
  BIG question: how do you set hierarchy while the previous hierarchy is busy firing?
  possible answer: you only do this one step ahead and never 2 or more steps? meaning, every time a pulse sequence is done there's only ONE choice for the new hierarchy.
  one way of implementing that plan is to be very careful when choosing the hierarchy. maybe it's getPrima-Secon-Terti is ONLY called within PulseCycleManager::pulseCoils() and it's never called until the sequence is done???

  ex. is rate of change of a so large that its value is uncertain? s? z? all of them? also, is z an estimate based on??
  although confidence should be mostly related to predicted POSITION of magnets, if rate of change of a is large enough even the direction can be uncertain
  */
/**
 * logic problem: Kinetic granularity is being determined by call to tMan->initTimingTask
 * this is wrong as it is determining how often position sensors can be monitored. it's wrong, even though 30 us granularity should be fine.
 * makes more sense to do an update only when needed??
 */
class KineticsManager : public TaskManager
{
  private:
    static KineticsManager* instance;   
        
    boolean updatePulseTiming;

    PulseCycleManager* pcMan;
    TimingManager* tMan;
    KineticProfileTickHistoryContainer* kpthCon;

    unsigned long kManIterCount;
    uint8_t updateTmanCount;

  public:
    KineticsManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo);
    static KineticsManager* getInstance();

    void doWhileNotIdling();
    void initKineticsManagerTask();
    
};

#endif
