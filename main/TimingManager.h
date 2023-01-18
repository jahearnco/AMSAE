/*
  TimingManager.h - Library for TimingManagerManager props and methods
*/
#ifndef TimingManager_h
#define TimingManager_h
#include <PulseCycleManager.h>

class TimingManager : public TaskManager
{
  private:
    static TimingManager* instance;

    PulseCycleManager* pcMan;

    unsigned long tmIterCount;

    boolean loopInterruptFlag;
  
  public:
    TimingManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo);
    static TimingManager* getInstance();
        
    boolean loopInterrupt();
    void processTiming();
    
    boolean healthCheck();

    void doWhileNotIdling();
    void updatePulseCycleManager();
};

#endif
