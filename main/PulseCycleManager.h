/*
  PulseCycleManager.h - Library for PulseCycleManager props and methods
*/
#ifndef PulseCycleManager_h
#define PulseCycleManager_h
#include <SerialManager.h>
#include <Brain.h>

  /**
   * maintaining a list of pulses in PulseCycle objects.
   * when is interruptType checked?
   * how to decide which position in queue? and is queue 2 or 3 elements?
   * first order of business: check health of current PulseCycle for two reasons ...
   * 1) if bad then put next cycle in queue
   * 2) if good then run one more iteration to get result
   * 
   * really don't know how to stop one and start another do ya? NOPE. I DON'T. 
   * maybe first question is about the number 2400micros and how often that width needs to be interrupted.
   * that;s not the right question. the first question is do I kick this off by creating a pulsecycle or doing a healthcheck?
   * 
   */

   /**
    * Queue maintains three PulseCycle objects at all time. necessary to give some history regarding priority. since TimingManager nor PulseCycleManager
    * maintains state beyong pulsing/not pulsing (and maybe a WARNING, but haven't considered all possible warnings yet, not whether they should simply be
    * ERRORs that kill the program?)
    * 
    * History helps to answer 3 or more questions: 1) why does current cycle have priority? 2) should current cycle be killed? 3) what to do with incoming data?
    * Answers depend on status of each cycle in queue. Why 3 and not just 2? Because otherwise there's no answer to the question "did the cycle on deck get there
    * because of a timing priority or a power priority or a confidence priority?
    */
   /**
    * so you chose your pulsecycle, or rather you updated the queue. Now ... how do you process the PulseCycle in [0]?
    * 
    * I guess something like pCycle[0].runPulseSchedule()?
    * 
    * OH! Shoot. For this program there's a 1-1 relationship Pulse to Coil. Forgot! So, priority must be set by PulseCycle,
    * reset is function of Coil and Pulse and called within PulseCycle and Pulse
    * 
    * so ... setPolarity is Pulse, setPriority sets Pulse priority not Coil priority (as there is none). 
    * 
    * OK! So we have queueStatus which includes health status. If all is well, queue is updated, we check queueStatus and processCurrentPulseCycle
    * or rather, so we have reference to PulseCycle[0] and call ... is it 'doCycle()' or ... most of the time the action is continuing the cycle. what's 
    * another word. 'do' is good, but is there better? process? run? runPulseCycle() and that calls runPulseSchedule for each Pulse? runPulse? no! monitorPulse?
    * maybe, but is there a startPulse and stopPulse also? updatePulse()? makes sense, because things like timeOn and pulsing and pulseStatus are updated
    * 
    * OK! updatePulse makes sense since Pulse has already been created. no need to 'do' it because Coil is active. updatePulse it is!
    * 
    * so important Pulse methods are initPulse and updatePulse. Coil is only method that needs to be reset. MAYBE PulseCycleManager but seems superfluous
    */
    
  /*
  indexHighestPower
  indexHighestConfidence
  magForceUp?? - IF INCOMING zUP is DIFFERENT THAN RUNNING CYCLE ... 
    a. check on deck and if not in different zUP then replace it with incoming
    b. check on deck and if it IS different then stop cycle and start incoming and flag the other two as "dead" or something
  */


  /**
   * which conditions are most critical for possible stoppage of current cycle? 
   * 1 zEst - together with confidence how far off is it?
   * 2 power - together with confidence how large is it? and what if power is high but confidence low??? STILL NEEDS BIG TIME ATTENTION
   * 3 mfup - if true but power is low then not such a big deal
   * 4 time left for cycle - if cycle has less than 0.8 ms to go then let it finish. otherwise don't
   * 
   * so algorithm can look like this assuming these ranges: power 1-100, confidence 1-100, and ...
   * need to come of with zEst range for each coil. ex. coil A zBottom ranges from 50 +/- 1 15mm & zTop from 100 +/- 15mm, so if
   * incoming zEst is 36mm with confidence of 20 then put in queue but if 80 then ... well, still no good reason to stop current 
   * cycle. DO THIS: REPLACE AND/OR STOP CURRENT CYCLE ONLY IN EXTREME CASES!! at least for now, so zEst would need to be outside range
   * for both coils, power would need to be high with high confidence, magForceUp needs to be different for two cycles. OK! This is something
   * I can work with! 
   * 
   * CRITICAL POINT!! every coil has four boundaries, two for each pole. TWO ARE "LOOSE" AND TWO ARE "TIGHT", meaning if a magnet is right at the tight
   * boundary (where magnet is all or partially inside the coil) THERE IS ONLY ONE DIRECTION for which it's ok to fire the coil. that direction is the one moving the magnet AWAY from the coil
   * 
   * 
   * 
    */

const uint8_t PCM_PULSE_WARNING = 24;
const uint8_t PCM_CYCLE_WARNING = 23;
const uint8_t PCM_PULSE_ERROR = 22;
const uint8_t PCM_CYCLE_ERROR = 21;

const uint8_t END_CYCLE_STATUS_CHECK = 20;

const uint8_t USABLE_QUEUE_SIZE = 3;

class PulseCycleManager: public TaskManager
{
  private:
    static PulseCycleManager* instance;
    
    SerialManager* sMan;
    PulseCycle* currentPC;
    /**
     * SIMPLE IDEA BEHIND THESE:
     * - they use Coil objects with pins not connected to actual coils
     * - we run these dummies just like any others
     * - after first few milliseconds PulseCycle::getCycleStatus() will ALWAYS return PCM_CURRENT_CYCLE_ENDED for these 
     * so there will always be something to run or enable or update each time through processCycleQueue. That's the hope
     * anyhow. a bit concerned that these are being overwritten somehow??
     */
    static PulseCycle pCycleList[4];
    PulseCycle pc00;
    PulseCycle pc01;
    PulseCycle pc02;
    PulseCycle pc03;
    
    CoilContainer* coilCon;
    
    double zEst;
    boolean magForceUp;
    unsigned long power;
    uint8_t confidence;
    unsigned long tCurrent;
    unsigned long tStartPCM;
    unsigned long pcmIterCount;

    uint8_t interruptType;
    uint8_t pcPulsingCode;

    boolean debug;
    boolean pulseCycleIsNew;

    void killCycles();
    void initPulseCycleAndUpdateQueue();
    void enableAndRunCurrentCycle();

    void processCycleQueue();
    
  public:
    PulseCycleManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo);

    static PulseCycleManager* getInstance();
    static KPPHistoryEvaluation kpphe;

    void doWhileNotIdling();
    boolean healthCheck();
    void initPulseCycleTask();
    void setIsPulseCycleNew(boolean isNew);
    boolean getIsPulseCycleNew();
    uint8_t getCurrentInterruptType();
};

#endif
