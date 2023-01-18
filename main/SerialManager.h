/*
  SerialManager.h - Library for SerialManagerManager props and methods
*/
#ifndef SerialManager_h
#define SerialManager_h
#include <TaskManager.h>

class SerialManager : public TaskManager
{
  private:
    static SerialManager* instance;

    unsigned long sManIters;
    boolean debugCC;
    boolean debugPCM;
    boolean debugTMAN;
    boolean debugBRAIN;
    boolean debugSMAN;
    double debugOSC;
    long tReadMin;
    boolean tReadMinChanged;
    boolean hardcodeOSC;
    boolean debugFlagChanged;
    boolean debugANY;

  public:
    SerialManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo);
    static SerialManager* getInstance();

    void processSerial();
    uint8_t getSerial();
    void setDebugFlagChanged(boolean dfChanged);

    void doWhileNotIdling();
    boolean debugCoilCon();
    boolean doPrintAfterIdle();
    boolean debugPcMan();
    boolean debugtMan();
    boolean debugBrain();
    boolean debugSMan();
    double debugOscillator();
    boolean hardcodeOsc();
    boolean debugAny();
    boolean debugChange();
    long getTReadMin();
    boolean newTReadMin();
};

#endif
