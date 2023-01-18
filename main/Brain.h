/*
  Brain.h - Library for BrainManager props and methods
*/
#ifndef Brain_h
#define Brain_h

#include <PulseCycle.h>
#include <KineticProfileTickHistory.h>
#include <vector>
#include <iostream>
#include <vector>
#include <numeric>
#include <stdexcept>
#include <string>
#include <iterator>

using std::iterator;
using std::vector;
using std::string;
using std::cout;
using std::cin;

//Optional interrupt and shutdown pins.
//#define SHUTDOWN_PIN 2
//#define INTERRUPT_PIN 3
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

const uint8_t OSC_SIZE = 6;

/**
 * safe meaning device is designed to handle velocities at half V_MAX
 * idea is that if this factor is exceeded then return VCALC_WARNING_EXCESSIVE_SPEED
 */
const double SAFE_AVEVELO_OVER_FACTOR = 0.50;
/**
 * VAVE_DELTA_SMOOTHING_FACTOR smooths velocity differences that are out of bounds due to inaccurate calcs during very fast motion.
 * the idea is to ..
 * 1. ensure that A_MAX*SAFE_AVEACCEL_OVER_FACTOR is rarely (not never!) exceeded when max velocity is 1/3 that of V_MAX
 * 2. approach this target predictively and accurately: A_MAX*SAFE_AVEACCEL_OVER_FACTOR is never exceeded when max velocity is 1/4 that of V_MAX
 *
 * TRICKY! - if you truncate too much then vAve becomes the next vAvePrior making the difference even greater!
 *  ALSO! - keep in mind that for fast motion both vAve & vAvePrior may have been truncated by vSafeOverMax condition above
 *  AND possibly in opposite directions, so their difference can still be 2*vSafeOverMax and if so that's a problem
 *
 * so try a few things. this should be app config'd
 */
const double VAVE_DELTA_SMOOTHING_FACTOR = 2.0*SAFE_AVEVELO_OVER_FACTOR;
const long TDEL_PROFILE_MIN = 890L;
const long TDEL_LONGTERM_MIN = 10000L;

struct KPPHistoryEvaluation
{
  boolean initAndUpdate;
  boolean disableCurrentPulseCycle;
  uint8_t pulseType;
  uint8_t interruptType;
  uint8_t incomingKPPQIndex;
  uint8_t evaluatedKPPQIndexOld;
  uint8_t evaluatedKPPQIndexNew;

  KPPHistoryEvaluation();
};

struct OscillatorProfile
{
  double ZMID;
  double ZAMPMIN;

  double currentZMid;
  double currentZAmp;
  unsigned long currentTPeriod;
  boolean doSetVProfileAtTransition;
  boolean firstOscRead;

  /**
  * OFFSET IS OF TYPE LONG - IT CAN BE NEGATIVE
  * offset is what allows a matching of curves at an inflection point
  */
  long tPeriodOffset;
  /**
  * OFFSET IS OF TYPE LONG - IT CAN BE NEGATIVE
  */
  uint8_t profileIndex;
  boolean doTransition;

  double fcfFloor;
  double scf;//singleCycleFraction

  double transitionScf;

  double zVal;
  uint16_t numCycles;
  uint16_t numCyclesBeforeTransition;

  uint16_t numCyclesBeforeTransitionList[OSC_SIZE];
  double zMidList[OSC_SIZE];
  double zAmpList[OSC_SIZE];

  double tPeriodList1[OSC_SIZE];
  double tPeriodList2[OSC_SIZE];
  double tPeriodList3[OSC_SIZE];
  double tPeriodList4[OSC_SIZE];
  double tPeriodList5[OSC_SIZE];
  double tPeriodList6[OSC_SIZE];

  vector<double*> tPeriodLists;
  vector<double*>::iterator tPeriodListsIter;

  double* tPeriodList;

  OscillatorProfile();
  void init();
};

class Brain
{
  private:

  public:
    static OscillatorProfile oscPro;
    static KPPHistoryEvaluation kpphe;

    static unsigned long LMAN_IDLING_ITERS;

    /**
     T_READ_MIN the min amount of time to elapse before trying to read or calc kinetics
      NOTE: want this to be greater than the max amount of time ON AVE per loop through
      kMan. something between 90 us and 150 us should be fine, but the larger the number
      the fewer samples read by ADC
    */
    static long T_READ_MIN;

    /** adc sampling */
    static uint8_t MIN_SAMPLE_SIZE;
    static uint8_t MAX_SAMPLE_SIZE;
    static double MAX_VPEAK_DEV;

    /** 
      ZREAD_DIFF_MIN changes in zRead less than this can be ignored or bookmarked
     */
    static double ZREAD_DIFF_MIN;
    /**
     TMIN_ZREAD_CHANGE may need to be variable as there is precision loss for
     fast impulses
    */
    static long TMIN_ZREAD_CHANGE;
    /**
     * value that keeps kpths from hanging around where there are no events to create new ones.
     * there will always be pulseCycles running and the max lifetime of each cycle is 5000us by convention.
     * this includes cycles that are idling (i.e. interruptType = 6). the reason why this number is needed
     * is that for very slow motion a lot of time can go by before sensors read a change, but software has     
     * been developed for continuous cycling. not to mention that it *is* an important event when a cycle
     * has ended! this should have been included long ago!
     * 
     * NOTE: this value isn't a constant for at least two reasons... 
     * 1. when idling we may want to increase it's value. at least it gives that option.
     * 2. predictive: for slow motion we want this small. for fast however, this may help to fine tune accuracy.
     * 
     * QUESTION: real case is very different from simulated cases near speed inflection. in simulated case 
     * piston can spend a LOT of time near inflection while there is no reason to think the same for real cases
     * 
     * It's an important point, becasue the simulation exists primarily to see how device will handle fast motion
     * the simulation isn't meant to be realistic for very slow motion near inflection for long tPeriods
     */
    static uint8_t MAX_PULSECYCLES_SINCE_LAST_RESET;

    /** values above this indicate confidence that kinetic measurement + calc is good enough to proceed */
    static uint8_t CONFIDENCE_THRESHOLD_MID;
    
    static double KPRIOR_WEIGHT;
    static double KVERYSHORTTERM_WEIGHT;
    static double KSHORTTERM_WEIGHT;//keep in mind this may not be short at all!
    static double KMIDTERM_WEIGHT;
    static double CONFIDENCE_WEIGHTING_SUM;

    static unsigned long POWER_RELATIVE_MID;
    static unsigned long POWER_MAX_HIGH;
    static unsigned long POWER_MAX_MID;
    static unsigned long POWER_THRESHOLD_MAGREVERSAL;

     /**
      * changed this for the rewrite starting at v042
      * 
      * 900L is a very small number, but the idea now is that instead of one profile being
      * used for vAve calcs we'll have many. probably 4, 5 or 6 to be assessed for several 
      * reasons. confidence, tunability, accuracy.
      */
    //static long TDEL_PROFILE_MIN;
    //static long TDEL_LONGTERM_MIN;

    /** DEBUG */
    static boolean REALIZED_MIN_EXTREMA_INTERRUPT;
    static boolean REALIZED_SURGE_INTERRUPT;
    /** DEBUG */

    static uint32_t kkCalcCount;
    static unsigned long tKKCalcTotal;

    static boolean kpthConInitialized;
    static unsigned long primingDelayMs;
    static unsigned long numNewCycles;

    static double minPeriodMult;
    static boolean hardcodeOsc;
    static boolean printOscMssg;
    static boolean debugBrain;

    static long caughtAFish;
    static long letOneGo;

    static boolean amsaeInitialized;

    static uint8_t calculateKinetics(KineticProfileTickHistory* currentKpth, KineticProfileTickHistory* priorKpth, vector<KineticProfileTickHistory>* kpthVList);
    static uint8_t setSecondOrderData(KineticProfileTickHistory* currentKpth, KineticProfileTickHistory* priorKpth, KineticProfileTickHistory* midTermKpth, KineticProfileTickHistory* shortTermKpth);
    static uint8_t setThirdOrderData(KineticProfileTickHistory* currentKpth, KineticProfileTickHistory* priorKpth, KineticProfileTickHistory* midTermKpth, KineticProfileTickHistory* shortTermKpth);
    static void calcConfidence(double confidenceHistorySum, uint8_t numMagForceUpReversals, uint8_t numDirectionReversals, double vAveShortTerm, double vAveMidTerm, double aAveShortTerm, double aAveMidTerm, KineticProfileTickHistory* currentKpth, KineticProfileTickHistory* priorKpth);
    static  KPPHistoryEvaluation evaluateKPPHistory(boolean currentCycleHasExpired, unsigned long tCurrent, uint8_t confidence, unsigned long power, uint8_t interruptType, boolean magForceUp, PulseCycle* pCycleList, boolean debug);
    static double readOscillatorProfile(unsigned long tick, double zReading);
    static uint8_t determineInterruptType(uint8_t fKinCode, unsigned long tCurrent, KineticProfileTickHistory* priorKpth);
    static boolean containersAndManagersAreInitialized();
    static void handleCatastrophe(const String s);
    static void checkForSerialUpdates();

    static boolean completeSensorReading(unsigned long tDel, vector<uint32_t> &adc1_readings, vector<uint32_t> &adc2_readings, double &adc_readingAve, uint8_t &adcSampleCount);

    Brain();
};

#endif
