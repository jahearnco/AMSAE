/
  LoopManager.h - Library for LoopManagerManager props and methods
/
#ifndef LoopManager_h
#define LoopManager_h
#include <KineticsManager.h>

class LoopManager: public TaskManager
{
  private:
    static LoopManager* instance;

    KineticsManager* kMan;
    struct timeval  tv1, tv2;

    unsigned long loopIters;
    unsigned long safeLoopIterMaxForWatchdogFeed;

    void feedTheWatchDog();

  public:
    LoopManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo);
    static LoopManager* getInstance();

    void doWhileNotIdling();
    void testPulsePins();
};

#endif
