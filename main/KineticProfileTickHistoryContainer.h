/*
  KineticProfileTickHistoryContainer.h - Library for KineticProfileTickHistoryContainer props and methods
*/
#ifndef KineticProfileTickHistoryContainer_h
#define KineticProfileTickHistoryContainer_h
#include <KineticPulseProfile.h>
#include <KineticProfileTickHistory.h>
#include <CoilContainer.h>
#include <Brain.h>
#include <PulseCycleManager.h>

/**
 * BEGIN::EXTREME VALUES FOR EXTREME LIMITS - NAMESPACE PLEASE!!!
 */
const uint32_t TICKER_COUNT_MAX = 500000;//uint32_t max is 4294967296;//need to play around with this number - it will likely be changed via app
/**
 * END::EXTREME VALUES FOR EXTREME LIMITS - NAMESPACE PLEASE!!!
 */
const uint8_t NO_INTERRUPT_KPTH_PROFILING = 221;
const uint8_t ZREADING_CHANGED = 222;
//const uint8_t TICKER_COUNT_MAX_EXCEEDED = 223;
const uint8_t SIMULATION_FORCED_INTERRUPT = 224;
const uint8_t UNINTERRUPTED_PULSECYCLING = 225;

class KineticProfileTickHistoryContainer : public AMSAESingleton
{ 
  private: 
    static KineticProfileTickHistoryContainer* instance;

    PulseCycleManager* pcMan;

    vector<KineticProfileTickHistory> kpthVList;
    KineticProfileTickHistory* currentKpth;
    KineticProfileTickHistory* priorKpth;

    CoilContainer* coilCon;
    SerialManager* sMan;

    uint8_t interruptKpthDefinitionCode;

    boolean zReadingChange;
    double zReading;
    double zReadingMaybe;
    
    uint32_t tickerCount;
    uint32_t kCalcCount;
    unsigned long tKCalcTotal;
    unsigned long sensorReadIters;
    uint8_t numCyclesSinceLastReset;
    uint32_t numStaccatoInterrupts;
    uint32_t numSurgeInterrupts;
    uint32_t numExtremaInterrupts;
    uint32_t numResumeInterrupts;
  
    unsigned long tStartTimingCycle;
    unsigned long lastTRead;
    unsigned long tCurrent;
    unsigned long tSensorTick;
    unsigned long kpthConIterCount;
    unsigned long kpthConResetCount;
    unsigned long tKpthTickZero;
    unsigned long tReadMin;
    unsigned long tDebugTick;

    boolean doRead;
    boolean lastReadWasATimeRead;

    boolean magForceUp;
    uint8_t interruptType;
    uint8_t confidence;
    unsigned long power;
    
    void updateKpthList();
    boolean updateTicker();
    void setInterruptKpthDefinitionCode();
    
    void generateKinetics();
    uint8_t finalizeKineticCalcs();
    double sinePosOnly(double intervalRadians);
    double sinePosOnlyObsolete(double intervalRadians);

    void reset();
    void updateZReading();
    void updateCurrentKineticProfile();
    void reportKineticResults(uint8_t fKinCode);
    void checkForSerialUpdates();
    double (*readPositionSensor)(unsigned long, double, double, boolean);

  public:
    KineticProfileTickHistoryContainer();

    static KineticProfileTickHistoryContainer* getInstance();
    static KineticPulseProfile* kpp;

    KineticProfileTickHistory* getlongTermKpth();
    KineticProfileTickHistory* getCurrentKpth();

    boolean processSensorData(boolean isPulseCycleNew);
    void generatePulseProfileData();
    void preInitializer();
    void postInitializer();
};

#endif
