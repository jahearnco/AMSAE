  /*
  Brain.cpp - Library for BrainManager props and methods
*/
#include <Brain.h>
#include <PulseCycleManager.h>
#include <KineticProfileTickHistoryContainer.h>

using namespace kpthspace;

KPPHistoryEvaluation Brain::kpphe;
OscillatorProfile Brain::oscPro;

boolean Brain::kpthConInitialized = false;

long Brain::caughtAFish = 0;
long Brain::letOneGo = 0;

unsigned long Brain::primingDelayMs = 10;//0.5 - secs allows time to view init status prior to loop
unsigned long Brain::numNewCycles = 0L;

boolean Brain::debugBrain = false;
double Brain::minPeriodMult = 10.0;// default
boolean Brain::hardcodeOsc = true;
boolean Brain::printOscMssg = false;
boolean Brain::REALIZED_MIN_EXTREMA_INTERRUPT = false;
boolean Brain::REALIZED_SURGE_INTERRUPT = false;

double Brain::MAX_VPEAK_DEV = 1.5*adcspace::peakDev;//~ peakDev*1.5 - if trailing edge of reading group is far from leading edge then continue sampling
uint8_t Brain::MIN_SAMPLE_SIZE = 4;//Brain::MIN_SAMPLE_SIZE x Brain::T_READ_MIN should be just less than Brain::TMIN_ZREAD_CHANGE
uint8_t Brain::MAX_SAMPLE_SIZE = MIN_SAMPLE_SIZE+4;////Brain::MAX_SAMPLE_SIZE x Brain::T_READ_MIN should be just less than 1000 us

unsigned long Brain::LMAN_IDLING_ITERS = 8L;
long Brain::T_READ_MIN = 110L;
uint8_t Brain::CONFIDENCE_THRESHOLD_MID = 57;

/**
 *  ~= T_READ_MIN * min sample count
 *  this value is more critical than I knew before RTFM ...
 *  OPT2003 samples every 750 us HOWEVER it's response time is 660 us
 *  that means if the device response time is at best 660 us + Brain::TMIN_ZREAD_CHANGE
 *  this is fine for light to moderate use, but for taxing cycles where loop takes long
 *  MIN_SAMPLE_SIZE may be too high?
 */
long Brain::TMIN_ZREAD_CHANGE = 250L;
double Brain::ZREAD_DIFF_MIN = 1.5;

unsigned long Brain::POWER_MAX_HIGH = 3000L;
unsigned long Brain::POWER_MAX_MID = 250L;
unsigned long Brain::POWER_RELATIVE_MID = 20L;
unsigned long Brain::POWER_THRESHOLD_MAGREVERSAL = 3200L;

/*b
unsigned long Brain::POWER_RELATIVE_MID = 20L;
unsigned long Brain::POWER_MAX_HIGH = 2000L;
unsigned long Brain::POWER_MAX_MID = 250L;
unsigned long Brain::POWER_THRESHOLD_MAGREVERSAL = 70L;
*/

uint8_t Brain::MAX_PULSECYCLES_SINCE_LAST_RESET = 1;
/**
 * END DEBUG BUT MAYBE PREDICTIVE/ADAPTIVE TOOL AS WELL?
 */

double Brain::KPRIOR_WEIGHT = DOUBLE_MAX;
double Brain::KVERYSHORTTERM_WEIGHT = DOUBLE_MAX;
double Brain::KSHORTTERM_WEIGHT = DOUBLE_MAX;//keep in mind this may not be short at all!
double Brain::KMIDTERM_WEIGHT = DOUBLE_MAX;
double Brain::CONFIDENCE_WEIGHTING_SUM = DOUBLE_MAX;

KPPHistoryEvaluation::KPPHistoryEvaluation(){
	initAndUpdate = false;
	disableCurrentPulseCycle = false;
	pulseType = pulsecyclespace::LOW_POWER_PULSING;
	interruptType = pulsecyclespace::NO_INTERRUPT_CURRENT_PULSE_CYCLE;
	incomingKPPQIndex = 0;
	evaluatedKPPQIndexOld = 0;
	evaluatedKPPQIndexNew = 1;
}

/* interruptTypes ...
 * 
 * NO_INTERRUPT_CURRENT_PULSE_CYCLE :: standard meaning no interruption of existing cycle.
 * SURGE_INTERRUPT :: large change in power either way (power surge or power cutoff)
 * MAX_EXTREMA_INTERRUPT :: extreme kinetic event - start cycle with dummy coils and wait for next interruptType
 * MIN_EXTREMA_INTERRUPT :: idling short pulsewidths long resting for one of many reasons 
 * RESUME_INTERRUPT :: response after MAX_EXTREMA_INTERRUPT when extreme event has passed
 */
KPPHistoryEvaluation Brain::evaluateKPPHistory(boolean currentCycleHasExpired, unsigned long tCurrent, uint8_t confidence, unsigned long power, uint8_t interruptType, boolean magForceUp, PulseCycle* pCycleList, boolean debug){
  using namespace pulsecyclespace;

  uint8_t incomingKPPQIndex = coilspace::BAD_INDEX;
  uint8_t evaluatedKPPQIndexOld = coilspace::BAD_INDEX;
  uint8_t evaluatedKPPQIndexNew = coilspace::BAD_INDEX;
  uint8_t pulseType = LOW_POWER_PULSING;
  boolean initAndUpdate = false;
  boolean disableCurrentPulseCycle = false;

  /**
   * DEBUG OVERRIDE!!!
   */
  //interruptType = NO_INTERRUPT_CURRENT_PULSE_CYCLE;
  /**
   * DEBUG OVERRIDE!!!
   */

  if (interruptType != NO_INTERRUPT_CURRENT_PULSE_CYCLE){

    switch(interruptType){
      case SURGE_INTERRUPT:
        pulseType = (confidence > CONFIDENCE_THRESHOLD_MID && power > Brain::POWER_MAX_HIGH) ? HIGH_POWER_PULSING : MEDIUM_POWER_PULSING;
        /** don't interrupt if current cycle is already a SURGE_INTERRUPT */
        if (pCycleList[0].getInterruptType() != SURGE_INTERRUPT){

          REALIZED_SURGE_INTERRUPT = true;
          
          incomingKPPQIndex = 0;
          initAndUpdate = true;
          disableCurrentPulseCycle = true;
        }else{
          incomingKPPQIndex = 1;
          evaluatedKPPQIndexOld = 1;
          evaluatedKPPQIndexNew = 2;
          initAndUpdate = true;
          disableCurrentPulseCycle = false;
        }
        break;

      case RESUME_INTERRUPT:
        pulseType = (confidence > CONFIDENCE_THRESHOLD_MID && power > Brain::POWER_MAX_HIGH) ? HIGH_POWER_PULSING : MEDIUM_POWER_PULSING;
        incomingKPPQIndex = 0;
        initAndUpdate = true;
        disableCurrentPulseCycle = true;
        break;

      case MAX_EXTREMA_INTERRUPT:
        pulseType = EMPTY_PULSING;
        incomingKPPQIndex = 0;
        initAndUpdate = true;
        disableCurrentPulseCycle = true;
        break;

      case MIN_EXTREMA_INTERRUPT:
        pulseType = STACCATO_PULSING;
        /** don't interrupt if current cycle is already a MIN_EXTREMA_INTERRUPT */
        if (pCycleList[0].getInterruptType() != MIN_EXTREMA_INTERRUPT){
          
          REALIZED_MIN_EXTREMA_INTERRUPT = true;
          
          incomingKPPQIndex = 0;
          initAndUpdate = true;
          disableCurrentPulseCycle = true;
        }else{
          incomingKPPQIndex = 1;
          evaluatedKPPQIndexOld = 1;
          evaluatedKPPQIndexNew = 2;
          initAndUpdate = true;
          disableCurrentPulseCycle = false;
        }
        break;

      default:
        pulseType = (confidence > CONFIDENCE_THRESHOLD_MID && power > Brain::POWER_MAX_HIGH) ? HIGH_POWER_PULSING : MEDIUM_POWER_PULSING;
        incomingKPPQIndex = 0;
        initAndUpdate = true;
        disableCurrentPulseCycle = true;
        break;
    }
    
  }else if (currentCycleHasExpired){

    unsigned long power0 = pCycleList[0].getPower();
    unsigned long power1 = pCycleList[1].getPower();
  
    unsigned long confidence0 = pCycleList[0].getConfidence();
    unsigned long confidence1 = pCycleList[1].getConfidence();
  
    uint8_t cycleStatus0 = pCycleList[0].getCycleStatus(tCurrent, debug);  
    uint8_t cycleStatus1 = pCycleList[1].getCycleStatus(tCurrent, debug);
  
    if (confidence > CONFIDENCE_THRESHOLD_MID && confidence > confidence0){
      pulseType = power > Brain::POWER_MAX_HIGH ? HIGH_POWER_PULSING : power > Brain::POWER_MAX_MID ? MEDIUM_POWER_PULSING : LOW_POWER_PULSING;
      
      incomingKPPQIndex = 0;

    }else if (power0 > power && power0 > power1 && confidence0 > CONFIDENCE_THRESHOLD_MID && cycleStatus0 == PCM_CURRENT_CYCLE_POTENT ){
      pulseType = power0 > Brain::POWER_MAX_HIGH ? HIGH_POWER_PULSING : power0 > Brain::POWER_MAX_MID ? MEDIUM_POWER_PULSING : LOW_POWER_PULSING;
      
      incomingKPPQIndex = power1 > power && confidence1 > confidence && cycleStatus1 == PCM_CURRENT_CYCLE_POTENT ? 2 : 1;
      evaluatedKPPQIndexOld = 0;
      evaluatedKPPQIndexNew = 0;
      
    }else if (power1 > power && confidence1 > CONFIDENCE_THRESHOLD_MID && cycleStatus1 == PCM_CURRENT_CYCLE_POTENT ){
      pulseType = power1 > Brain::POWER_MAX_HIGH ? HIGH_POWER_PULSING : power1 > Brain::POWER_MAX_MID ? MEDIUM_POWER_PULSING : LOW_POWER_PULSING;
      /** this case may never occur but keep it here to allow for further options */
      evaluatedKPPQIndexOld = 1;
      evaluatedKPPQIndexNew = 0;
      incomingKPPQIndex =  1;

    }else{
      pulseType = power > Brain::POWER_MAX_HIGH ? HIGH_POWER_PULSING : power > Brain::POWER_MAX_MID ? MEDIUM_POWER_PULSING : LOW_POWER_PULSING;
      
      incomingKPPQIndex = 0;
      
    }    
    initAndUpdate = true;
    disableCurrentPulseCycle = false;
    
  }else{
    /** pulsing so leave it alone */
    initAndUpdate = false;
    disableCurrentPulseCycle = false;
  }
  
  kpphe.initAndUpdate = initAndUpdate;
  kpphe.pulseType = pulseType;
  kpphe.interruptType = interruptType;
  kpphe.disableCurrentPulseCycle = disableCurrentPulseCycle;
  kpphe.incomingKPPQIndex = incomingKPPQIndex;
  kpphe.evaluatedKPPQIndexOld = evaluatedKPPQIndexOld;
  kpphe.evaluatedKPPQIndexNew = evaluatedKPPQIndexNew;
  return kpphe;
}

uint32_t Brain::kkCalcCount = 0;
unsigned long Brain::tKKCalcTotal = 0L;


/**
 * 
     smoothing required for cases where period is twice that of the minimum for which the device is designed.
     the idea is that although the extreme case needs to be handled, much better to handle the more realistic cases
     of slightly slower motion while simultaneously handling the extreme case. "SMOOTHING" meaning not abrupt fix
     of discontinuities at the extrema but rather a prevention of those fixes by smoothing out the data that would
     otherwise result in those discontinuities.

     factor values chosen to be equally balanced between the two conditions which follow

     EXEC DECISION: won't shut down upon extreme motion but rather will just flag this event and pause. this will
     prevent device from accelerating piston in wrong direction and will allow a few more iterations before taking
     action. no this isn't ideal but again, we're defining the limits of the device here.
 */
  /**
   take two readings in order to establish vProfile. logic is something like ...
   1. for tProfileDelta > TDEL_PROFILE_MIN look for first change in zReading
   2. for tProfileDelta > TDEL_PROFILE_MID look for first change in zReading
   3. for tProfileDelta > TDEL_LONGTERM_MIN && tProfileDelta < TDEL_LONGTERM_MIN*2 look for first change in zReading
   see KineticProfileTickHistoryContainer::setInterruptKpthDefinitionCode() for conditions met which create the kpths
  */
uint8_t Brain::calculateKinetics(KineticProfileTickHistory* currentKpth, KineticProfileTickHistory* priorKpth, vector<KineticProfileTickHistory>* kpthVListPtr){
  using namespace pulsecyclespace;

  uint8_t retCode = VCALC_SUCCESS;
  uint8_t numMagForceUpReversals = 0;
  uint8_t numDirectionReversals = 0;

  if (priorKpth->getInterruptType() != SURGE_INTERRUPT){
    if (priorKpth->getMagForceUpReversal()){ numMagForceUpReversals++; }//previousZRead = priorKpth->getProfileZRead(); mfuRevIndex = 1; }
    if (priorKpth->getDirectionReversal()){ numDirectionReversals++; }//previousZRead = priorKpth->getProfileZRead(); dirRevIndex = 1; }
  }

  double mostRecentZRead = currentKpth->getProfileZRead();
  double zDelta = mostRecentZRead - priorKpth->getProfileZRead();
  
  unsigned long mostRecentTickStamp = currentKpth->getTProfileTickStamp();
  unsigned long priorTickStamp = priorKpth->getTProfileTickStamp();

  long tProfileDelta = (long)(mostRecentTickStamp - priorTickStamp); if (tProfileDelta < 2L) Serial.println("priorKpth div by zero");
  
  KineticProfileTickHistory* testKpth = nullptr;
  uint16_t midTermKpthListIndex = 0;
  uint16_t shortTermKpthListIndex = 0;
  uint16_t longTermKpthListIndex = 0;

  uint8_t shortTermConfidence = 0;
  uint8_t midTermConfidence = 0;
  
  double vAvePrior = priorKpth->getVAve();
  double vAveShortTerm = 0.0;
  double vAveMidTerm = 0.0;
  double vAveLongTerm = 0.0;
  double aAveShortTerm = 0.0;
  double aAveMidTerm = 0.0;
  double aAveLongTerm = 0.0;
  double vProfile = 0.0;

  double vProfileShortTerm = 0.0;
  double vProfileMidTerm = 0.0;
  double vProfileVeryShortTerm = (AMSAEUtils::div(zDelta, (double)tProfileDelta));

  long minProfileTDelta = TDEL_PROFILE_MIN;
  boolean setReversals = true;

  /**
   * priorKpth is hListIndex = 1, currentKpth is hListIndex = 0
   * priorKpth cannot be same as any kpthVListPtr->at(hListIndex) below. confidence calc depends on this
   */
  uint16_t hListIndex = 2;
  uint16_t kpthVListSize = kpthVListPtr->size();

  while (hListIndex < kpthVListSize){
    testKpth = &(kpthVListPtr->at(hListIndex));

    if(setReversals && testKpth->getInterruptType() != SURGE_INTERRUPT){
      if (testKpth->getMagForceUpReversal()){ numMagForceUpReversals++; }//previousZRead = priorKpth->getProfileZRead(); mfuRevIndex = hListIndex; }
      if (testKpth->getDirectionReversal()){ numDirectionReversals++; }//previousZRead = priorKpth->getProfileZRead(); dirRevIndex = hListIndex; }
    }
    
    unsigned long testProfileTickStamp = testKpth->getTProfileTickStamp();
    tProfileDelta = (long)(mostRecentTickStamp - testProfileTickStamp);
    
    if (tProfileDelta < minProfileTDelta){
      //do nothing
      
    } else if (tProfileDelta < TDEL_LONGTERM_MIN*2){
      
      if (shortTermKpthListIndex == 0){
        shortTermConfidence = testKpth->getConfidence();
        zDelta = mostRecentZRead - testKpth->getProfileZRead();
        aAveShortTerm = testKpth->getAAve();
        vAveShortTerm = testKpth->getVAve();
        vProfileShortTerm = (AMSAEUtils::div(zDelta, (double)tProfileDelta));

        setReversals = false;
        
        shortTermKpthListIndex = hListIndex;
        minProfileTDelta = TDEL_PROFILE_MIN * 2;

      } else if (midTermKpthListIndex == 0){
        midTermConfidence = testKpth->getConfidence();
        
        zDelta = mostRecentZRead - testKpth->getProfileZRead();
        aAveMidTerm = testKpth->getAAve();
        vAveMidTerm = testKpth->getVAve();
        vProfileMidTerm = (AMSAEUtils::div(zDelta, (double)tProfileDelta));
        
        midTermKpthListIndex = hListIndex; 
        minProfileTDelta = TDEL_LONGTERM_MIN;
        
      } else if (longTermKpthListIndex == 0){
        vAveLongTerm = testKpth->getVAve();
        aAveLongTerm = testKpth->getAAve();
        longTermKpthListIndex = hListIndex;
        break;
      }
    } else {
      if (shortTermKpthListIndex == 0){
        //Serial.println("at least one kpthroup is very long lived. maybe adjust MAX_PULSECYCLES_SINCE_LAST_RESET? largest value expected less than 20000L tProfileDelta = "+String(tProfileDelta));
        if (tProfileDelta < 2L) Serial.println("short term");
        shortTermConfidence = testKpth->getConfidence();
        zDelta = mostRecentZRead - testKpth->getProfileZRead();
        aAveShortTerm = testKpth->getAAve();
        vAveShortTerm = testKpth->getVAve();
        vProfileShortTerm = (AMSAEUtils::div(zDelta, (double)tProfileDelta));

        shortTermKpthListIndex = hListIndex;
      }
      if (midTermKpthListIndex == 0){
        //Serial.println("at least one kpthroup is very long lived. maybe adjust MAX_PULSECYCLES_SINCE_LAST_RESET? largest value expected less than 20000L tProfileDelta = "+String(tProfileDelta));
        if (tProfileDelta < 2L) Serial.println("profile term");
        midTermConfidence = testKpth->getConfidence();
        zDelta = mostRecentZRead - testKpth->getProfileZRead();
        aAveMidTerm = testKpth->getAAve();
        vAveMidTerm = testKpth->getVAve();
        vProfileMidTerm = (AMSAEUtils::div(zDelta, (double)tProfileDelta));
        
        midTermKpthListIndex = hListIndex;
      }
      if (longTermKpthListIndex != 0 && longTermKpthListIndex == midTermKpthListIndex){
        Serial.println("longTermKpthListIndex == midTermKpthListIndex hListIndex = "+String(hListIndex)+" & longTermKpthListIndex ="+String(longTermKpthListIndex)+" & midTermKpthListIndex = "+String(midTermKpthListIndex)+" & tProfileDelta = "+String(tProfileDelta));
        
      }else if (longTermKpthListIndex != 0){
        Serial.println("WTF??? hListIndex = "+String(midTermKpthListIndex)+" &  longTermKpthListIndex ="+String(longTermKpthListIndex)+" & midTermKpthListIndex = "+String(midTermKpthListIndex)+" & tProfileDelta = "+String(tProfileDelta));
      }
      vAveLongTerm = testKpth->getVAve();
      aAveLongTerm = testKpth->getAAve();
      longTermKpthListIndex = hListIndex;
      break;
    }
    hListIndex++;
  }

  if (testKpth == nullptr){ //(V_MAX-0.0039)){// (V_MAX-0.00215)){
    /** rounding errors are causing namespace some issues hence the V_MAX offset below*/
    Serial.println("testKpth == nullptr::VCALC_ERROR_UNDETERMINED tProfileDelta = " + String(tProfileDelta));
    return VCALC_ERROR_UNDETERMINED;
  }

  if (tProfileDelta <= TDEL_PROFILE_MIN && Brain::kpthConInitialized){ //(V_MAX-0.0039)){// (V_MAX-0.00215)){
	    /** rounding errors are causing namespace some issues hence the V_MAX offset below*/
	    Serial.println("tProfileDelta <= TDEL_PROFILE_MIN::VCALC_ERROR_UNDETERMINED tProfileDelta = " + String(tProfileDelta));
	    return VCALC_ERROR_UNDETERMINED;
  }
  
  if (midTermKpthListIndex < 3){
    testKpth = &(kpthVListPtr->at(hListIndex));
    
    shortTermConfidence = testKpth->getConfidence();
    zDelta = mostRecentZRead - testKpth->getProfileZRead();
    aAveShortTerm = testKpth->getAAve();
    vAveShortTerm = testKpth->getVAve();
    vProfileShortTerm = (AMSAEUtils::div(zDelta, (double)tProfileDelta));

    shortTermKpthListIndex = hListIndex;

    midTermConfidence = testKpth->getConfidence();
    
    aAveMidTerm = testKpth->getAAve();
    vAveMidTerm = testKpth->getVAve();
    vProfileMidTerm = (AMSAEUtils::div(zDelta, (double)tProfileDelta));
    
    midTermKpthListIndex = hListIndex; 

    vAveLongTerm = testKpth->getVAve();
    aAveLongTerm = testKpth->getAAve();
    longTermKpthListIndex = hListIndex;
    
    if (midTermKpthListIndex == 0) {
      Serial.println("midTermKpthListIndex OUT OF BOUNDS!!! tProfileDelta = " + String(tProfileDelta) + " & midTermKpthListIndex = " + String(midTermKpthListIndex) + " & hListIndex = " + String(hListIndex));
    }
    if (Brain::kpthConInitialized) return VCALC_ERROR_UNDETERMINED;
  }
  /**
   * hListIndex starts at 2 so midTermKpthListIndex is 3 or more
   * 
   * KPRIOR_WEIGHT is a measure of how confident one should be in prior confidence vs. short term and mid term confidence
   */
  switch (midTermKpthListIndex){
    case 3:
      /* case of above average and below-average speeds - prior is likely much more representative of current confidence than short term on average  */
      KPRIOR_WEIGHT = 6.0;
      KVERYSHORTTERM_WEIGHT = 1.0;
      KSHORTTERM_WEIGHT = 2.0;
      KMIDTERM_WEIGHT = 2.0;     
      break;

    case 4:
      /* case of above average speeds - prior is likely more representative of current confidence than short term on average  */
      KPRIOR_WEIGHT = 6.0;
      KVERYSHORTTERM_WEIGHT = 1.0;
      KSHORTTERM_WEIGHT = 3.0;
      KMIDTERM_WEIGHT = 2.0;     
      break;

    case 5:
      /* case of fast speeds - prior is likely slightly more representative of current confidence than short term on average */
      KPRIOR_WEIGHT = 5.0;
      KVERYSHORTTERM_WEIGHT = 1.0;
      KSHORTTERM_WEIGHT = 4.0;
      KMIDTERM_WEIGHT = 2.0;     
      break;

    default:
      /* cases of very fast speeds - prior is likely just as representative of current confidence as short term on average */
      KPRIOR_WEIGHT = 6.0;
      KVERYSHORTTERM_WEIGHT = 1.0;
      KSHORTTERM_WEIGHT = 6.0;
      KMIDTERM_WEIGHT = 2.0;  
  }

  double vProfileWeightingSum = KVERYSHORTTERM_WEIGHT + KSHORTTERM_WEIGHT + KMIDTERM_WEIGHT;
  CONFIDENCE_WEIGHTING_SUM = KPRIOR_WEIGHT + KSHORTTERM_WEIGHT + KMIDTERM_WEIGHT;
  
  vProfile = (KVERYSHORTTERM_WEIGHT*vProfileVeryShortTerm + KSHORTTERM_WEIGHT*vProfileShortTerm + KMIDTERM_WEIGHT*vProfileMidTerm) / vProfileWeightingSum; //play with weightings. maybe app config'd?

  if (abs(vProfile) > V_MAX * MAXVELO_OVER_FACTOR){
    vProfile = vProfile > 0 ? V_MAX * MAXVELO_OVER_FACTOR : -1.0 * V_MAX * MAXVELO_OVER_FACTOR;
    //Serial.println("abs(vProfile) > V_MAX * MAXVELO_OVER_FACTOR vProfile/V_MAX = "+String(vProfile/V_MAX));
    retCode = VCALC_WARNING_EXCESSIVE_SPEED;
  }

  /**
   * midTermKpthListIndex is at LEAST 3
   * 
   * the two weighted values here are different animals...
   * 
   * - vProfile under good circumstances is meant to be a good estimate for vAve
   * - vAvePrior under any circumstances isn't meant to be a good estimate for vAve BUT it's meant to smooth out discontinuities in case vProfile is abmormal
   * 
   * all to say that the intent isn't for vAve to be as accurate as possible, but rather for it to be realistic with realistic relative values in most cases, and for it to 
   * be monitored, smoothed and reported when vProfile is extreme.
   * 
   * NOTE: extreme vProfile defines the limits of the device. dealing with those extremes is tantamount to defining the device behavior at its limits
   * those limits are expected to be reached and are meant to result in MAX_EXTREMA_INTERRUPT in cases of very fast/quick/indeterminate motion or MIN_EXTREMA_INTERRUPT
   * in cases of very slow or indeterminate motion when longer term (> 20ms) kinetics hint at idling which can lead to overheating of coils
   */
  double cvProfile = 3.0*(midTermKpthListIndex - 1);
  double cPriorVAve = 1.0*midTermKpthListIndex;
  double numer = 1.0/(cPriorVAve + cvProfile);
  double vAve = (cPriorVAve*vAvePrior + cvProfile*vProfile) * numer;
  double absVAve = abs(vAve);

  /** BEGIN::HANDLING OF EXTREME CASES */
  double vSafeOverMax = V_MAX * SAFE_AVEVELO_OVER_FACTOR;
  if (absVAve > vSafeOverMax){
    vAve = vAve > 0 ? vSafeOverMax : -1.0 * vSafeOverMax;
    //Serial.println("VELOCITY ERROR 1::vAve has exceeded max value!! V_MAX = " + String(V_MAX, 5) + " & vAve = " + String(vAve, 5) + " & vAvePrior = " + String(vAvePrior, 5) + " & vProfile = " + String(vProfile, 11) + " & midTermKpthListIndex = " + String(midTermKpthListIndex, 5));
    retCode = VCALC_WARNING_EXCESSIVE_SPEED;
  }

  double vSmoothingMax = V_MAX * VAVE_DELTA_SMOOTHING_FACTOR;
  if (abs(vAve - vAvePrior) > vSmoothingMax){
    vAve = vAve > vAvePrior ? vAvePrior + vSmoothingMax : vAvePrior - vSmoothingMax;
    //Serial.println("abs(vAve - vAvePrior) > V_MAX * VAVE_DELTA_SMOOTHING_FACTOR vAve - vAvePrior/V_MAX = "+String((vAve - vAvePrior)/V_MAX, 8));
  }
  /** END::HANDLING OF EXTREME CASES */
  currentKpth->setVAve(vAve);
  currentKpth->setVProfile(vProfile);
  currentKpth->setDirectionReversal(vAve * vAvePrior < 0);
  currentKpth->setShortTermKpthListIndex(shortTermKpthListIndex);
  currentKpth->setMidTermKpthListIndex(midTermKpthListIndex);
  currentKpth->setLongTermVAve(vAveLongTerm);
  currentKpth->setLongTermAAve(aAveLongTerm);

  double tenV_MIN = 10*V_MIN;
  double hundredA_MIN = 10*A_MIN;

  if (longTermKpthListIndex < 6 && absVAve < tenV_MIN && abs(vAvePrior) < tenV_MIN && abs(vAveLongTerm) < tenV_MIN && abs(priorKpth->getAAve()) < hundredA_MIN && abs(aAveLongTerm) < hundredA_MIN) {
    retCode = VCALC_MIN_EXTREMA;
  }

  KineticProfileTickHistory* midTermKpth = &(kpthVListPtr->at(midTermKpthListIndex));
  KineticProfileTickHistory* shortTermKpth = &(kpthVListPtr->at(shortTermKpthListIndex));

  uint8_t aSetStatus = setSecondOrderData(currentKpth, priorKpth, midTermKpth, shortTermKpth);
  uint8_t jSetStatus = setThirdOrderData(currentKpth, priorKpth, midTermKpth, shortTermKpth);
  double confidenceHistorySum = KPRIOR_WEIGHT*priorKpth->getConfidence() + KSHORTTERM_WEIGHT*shortTermConfidence + KMIDTERM_WEIGHT*midTermConfidence;
  calcConfidence(confidenceHistorySum, numMagForceUpReversals, numDirectionReversals, vAveShortTerm, vAveMidTerm, aAveShortTerm, aAveMidTerm, currentKpth, priorKpth);

  if (retCode == VCALC_SUCCESS || retCode == VCALC_MIN_EXTREMA){
    
    if (aSetStatus != ACALC_SUCCESS){
      retCode = aSetStatus;
      
    }else if (jSetStatus != JCALC_SUCCESS){
      retCode = jSetStatus;
    }
    
  }
  return retCode;
}

uint8_t Brain::setSecondOrderData(KineticProfileTickHistory* currentKpth, KineticProfileTickHistory* priorKpth, KineticProfileTickHistory* midTermKpth, KineticProfileTickHistory* shortTermKpth){
  uint8_t retCode = ACALC_SUCCESS;
  
  long deltaShortTermTick = (long)(currentKpth->getTProfileTickStamp() - shortTermKpth->getTProfileTickStamp());
  long deltaMidTermTick = (long)(currentKpth->getTProfileTickStamp()- midTermKpth->getTProfileTickStamp());
  
  double deltaShortTermVAve = currentKpth->getVAve() - shortTermKpth->getVAve();
  double shortTermAAve = AMSAEUtils::div(deltaShortTermVAve, (double)deltaShortTermTick);
  
  double deltaProfileVAve = currentKpth->getVAve() - midTermKpth->getVAve();
  double midTermAAve  = AMSAEUtils::div(deltaProfileVAve, (double)deltaMidTermTick);
  
  if (deltaMidTermTick < 1L) Serial.println("adeltaMidTermTick");
  if (deltaShortTermTick < 1L) Serial.println("adeltaShortTermTick");

  double aAve = (KSHORTTERM_WEIGHT * shortTermAAve + KMIDTERM_WEIGHT * midTermAAve) / (KSHORTTERM_WEIGHT+KMIDTERM_WEIGHT);
  boolean magForceUp;
  double absAAve;

  if (aAve < 0){
	  magForceUp = true;//convention: if aAve is positive that means in direction of increasing z. mag force needs to oppose that
	  absAAve = -1.0*aAve;
  }else{
	  magForceUp = false;
	  absAAve = aAve;
  }

  boolean magForceUpReversal = absAAve > A_MIN && magForceUp != priorKpth->getMagForceUp();
  unsigned long priorPower = priorKpth->getPower();
  /**
   * attempting to speed up bitwise division by ensuring power is always even
   */
  unsigned long power = 2*((unsigned long)(absAAve*A_MIN_HALFNUM + 1.0));

  unsigned long relativePower1 = AMSAEUtils::bwDiv(power, priorPower);//(double)power/priorPower;
  unsigned long relativePower2 = AMSAEUtils::bwDiv(priorPower, power);//(double)priorPower/power;

  currentKpth->setAAve(aAve);
  currentKpth->setMagForceUp(magForceUp);
  currentKpth->setMagForceUpReversal(magForceUpReversal);
  currentKpth->setPower(power);
  
  if (absAAve > A_MAX * SAFE_AVEACCEL_OVER_FACTOR){
    //Serial.println("ERROR::aAve HAS EXCEEDED MAXIMUM VALUE!!");//MUST KEEP IN MIND THIS IS THE RESULT OF POOR GRANULARITY NOT POOR CALC. POOR GRAN IS WHAT WE GOT SO DEAL WITH IT.
    retCode = ACALC_WARNING_EXCESSIVE_ACCEL;
    
  }else if (relativePower1 > POWER_RELATIVE_MID || relativePower2 > POWER_RELATIVE_MID){
    //if (T_CURRENT > 5000000L) { Serial.println("relative power is BIG relativePower1 = "+String(relativePower1)+" &  relativePower2 = "+String(relativePower2)); }
    retCode = ACALC_SIGNIFICANT_EVENT;
    
  }else if (magForceUpReversal && power > POWER_THRESHOLD_MAGREVERSAL){
    //if (T_CURRENT > 5000000L) { Serial.println("magForceUpReversal && power big enough = "+String(power)); }
    retCode = ACALC_MAGFORCEUP_REVERSAL;
    
  }
  return retCode;
}

uint8_t Brain::setThirdOrderData(KineticProfileTickHistory* currentKpth, KineticProfileTickHistory* priorKpth, KineticProfileTickHistory* midTermKpth, KineticProfileTickHistory* shortTermKpth){
  uint8_t retCode = JCALC_SUCCESS;

  long deltaShortTermTick = (long)(currentKpth->getTProfileTickStamp() - shortTermKpth->getTProfileTickStamp());
  long deltaMidTermTick = (long)(currentKpth->getTProfileTickStamp()- midTermKpth->getTProfileTickStamp());
  
  double deltaMidTermAAve = currentKpth->getAAve() - midTermKpth->getAAve();
  double midTermJAve  = AMSAEUtils::div(deltaMidTermAAve, (double)deltaMidTermTick);

  if (deltaMidTermTick < 1L) Serial.println("jdeltaMidTermTick");

  double deltaShortTermAAve = currentKpth->getAAve() - shortTermKpth->getAAve();
  double shortTermJAve = AMSAEUtils::div(deltaShortTermAAve, (double)deltaShortTermTick);

  if (deltaShortTermTick < 1L) Serial.println("jdeltaShortTermTick");

  double jAve = (KSHORTTERM_WEIGHT * shortTermJAve + KMIDTERM_WEIGHT * midTermJAve) / (KSHORTTERM_WEIGHT+KMIDTERM_WEIGHT);
  double absJAve = jAve < 0 ? -1.0*jAve : jAve;

  unsigned long priorJolt = priorKpth->getJolt();
  /**
   * attempting to speed up bitwise division by ensuring jolt is always even
   */
  unsigned long jolt = 2*((unsigned long)(absJAve*J_MIN_HALFNUM + 1.0));

  unsigned long relativeJolt1 = AMSAEUtils::bwDiv(jolt, priorJolt);//(double)jolt/priorJolt;
  unsigned long relativeJolt2 = AMSAEUtils::bwDiv(priorJolt, jolt);//(double)priorJolt/jolt;

  if (jolt > ULONG_MAX) {
    Serial.println("ULONG_MAX EXCEEDED!!!  jAve = "+String(jAve,26));
    retCode = JCALC_WARNING_EXCESSIVE_JERK;
  }

  currentKpth->setJAve(jAve);
  currentKpth->setJolt(jolt);

  if (absJAve > J_MAX * SAFE_AVEJERK_OVER_FACTOR){
    //Serial.println("ERROR 1::jAve HAS EXCEEDED MAXIMUM VALUE!!");//MUST KEEP IN MIND THIS IS THE RESULT OF POOR GRANULARITY NOT POOR CALC. POOR GRAN IS WHAT WE GOT SO DEAL WITH IT.
    retCode = JCALC_WARNING_EXCESSIVE_JERK;
    
  }else if (priorJolt > 200000L && relativeJolt1 > 200){
    //Serial.println("ERROR 2::jAve CHANGE IS VERY LARGE!");//MUST KEEP IN MIND THIS IS THE RESULT OF POOR GRANULARITY NOT POOR CALC. POOR GRAN IS WHAT WE GOT SO DEAL WITH IT.
    retCode = JCALC_WARNING_EXCESSIVE_JERK;
    
   }else if (jolt > 200000L && relativeJolt2 > 200){
    //Serial.println("ERROR 3::jAve CHANGE IS VERY LARGE!");//MUST KEEP IN MIND THIS IS THE RESULT OF POOR GRANULARITY NOT POOR CALC. POOR GRAN IS WHAT WE GOT SO DEAL WITH IT.
    retCode = JCALC_WARNING_EXCESSIVE_JERK;
       
  }else if (priorJolt > 100000L && relativeJolt1 > 20){
    retCode = JCALC_SIGNIFICANT_EVENT;
    
  }else if (jolt > 100000L && relativeJolt2 > 20){
    retCode = JCALC_SIGNIFICANT_EVENT;
       
  }
  return retCode;
}

/**
* define confidence
* 1. existing confidence implies confidence, so take average of recent confidence - as this is historical there's a lot of knowledge in every kpth up to and including midTermkpth
* 2. consistency of magForceUp - very recent change to this value would require an extra calc before proceeding - look at every kpth up through profile - interruptType = 5 is a solution while waiting
* 3. consistency of directionReversal - very recent change to this value would require an extra calc before proceeding - look at every kpth up through profile - interruptType = 5 is a solution while waiting
* 4. consistency of velocity - in the short term is it all over the place or is there an obvious trend - look at prior, short term and midTermkpth
* 5. consistency of acceleration - in the short term is it all over the place or is there an obvious trend - look at prior, short term and midTermkpth

 * uint8_t confidence values ...
 * 100: highest confidence
 * 60: good confidence that current kpth is ready to be converted to a pulseCycle
 * 50: lowest value of confidence whereby current kpth might be acceptable for conversion to pulseCycle
 * 40: more is needed before conversion to pulsecycle - could be just an extra calc -> interruptType = 5
 * 30: another reading is needed -> interruptType = 5 or interruptType = 6
 * 20: no good -> interruptType = 6
 */
void Brain::calcConfidence(double confidenceHistorySum, uint8_t numMagForceUpReversals, uint8_t numDirectionReversals, double vAveShortTerm, double vAveMidTerm, double aAveShortTerm, double aAveMidTerm, KineticProfileTickHistory* currentKpth, KineticProfileTickHistory* priorKpth){
  double confWeightingSum = CONFIDENCE_WEIGHTING_SUM;

  double confidenceSum = confidenceHistorySum;
  double fullWeightingSum = confWeightingSum;
  double mfurWeighting = confWeightingSum;
  double drWeighting = confWeightingSum;
  double vWeighting = 2.0*confWeightingSum;
  double aWeighting = confWeightingSum;

  //magForceUpReversals
  boolean magForceUpReversal = currentKpth->getMagForceUpReversal();
  if (magForceUpReversal) numMagForceUpReversals++;
  double mfurConfidence = numMagForceUpReversals == 0 ? 94 : numMagForceUpReversals == 1 && !magForceUpReversal ? 60 : numMagForceUpReversals == 1 ? 50 : numMagForceUpReversals == 2 ? 40 : 0;
  confidenceSum += mfurWeighting * mfurConfidence;
  fullWeightingSum += mfurWeighting;

  //directionReversals
  boolean directionReversal = currentKpth->getDirectionReversal();
  if (directionReversal) numDirectionReversals++;
  double drConfidence = numDirectionReversals == 0 ? 94 : numDirectionReversals == 1 && !numDirectionReversals ? 60 : numDirectionReversals == 1 ? 50 : numDirectionReversals == 2 ? 40 : 0;
  confidenceSum += drWeighting * drConfidence;
  fullWeightingSum += drWeighting;

  //ave velocities
  double vConfidence = 0.0;

  double vAve = currentKpth->getVAve();
  double vAvePrior = priorKpth->getVAve();
  
  double meanVAve = (vAve + vAvePrior + vAveShortTerm + vAveMidTerm) * 0.25;

  double vPriorDelta = vAve - vAvePrior;
  double vShortDelta = vAve - vAveShortTerm;
  double vMidDelta = vAve - vAveMidTerm;

  double vmagCurrentDev = abs(vAve - meanVAve);
  double vmagPriorDev = abs(vAvePrior - meanVAve);

  double vmagShortDelta = abs(vShortDelta);
  double vmagMidDelta = abs(vMidDelta);
      
  if (vPriorDelta >= 0){
    /** vPriorDelta is positive or 0*/
    if (vPriorDelta <= vShortDelta && vShortDelta <= vMidDelta){
      vConfidence = vShortDelta == vMidDelta ? 84 : 94;
      
    }else if (vPriorDelta <= vShortDelta || vPriorDelta < 3*vmagShortDelta){
      vConfidence = vmagShortDelta < 1.5*vmagMidDelta ? 75 : vmagShortDelta < 3*vmagMidDelta ? 70 : vmagShortDelta < 6*vmagMidDelta ? 50 : vmagShortDelta < 18*vmagMidDelta ? 45 : 30;
    }
    
  }else{
    /** vPriorDelta is negative*/
    if (vPriorDelta >= vShortDelta && vShortDelta >= vMidDelta){
      vConfidence = vShortDelta == vMidDelta ? 84 : 94;
      
    }else if (vPriorDelta >= vShortDelta || vPriorDelta > -3*vmagShortDelta){
      vConfidence = vmagShortDelta < 1.5*vmagMidDelta ? 75 : vmagShortDelta < 3*vmagMidDelta ? 70 : vmagShortDelta < 6*vmagMidDelta ? 50 : vmagShortDelta < 18*vmagMidDelta ? 45 : 30;
    }
  }

  /** next in case no condition above was met */
  if (vConfidence < 1.0){

    if (vmagCurrentDev <= vmagPriorDev && (vmagShortDelta < vmagMidDelta || vmagShortDelta < 3*vmagMidDelta)){
      /**this is lower confidence case where it's reasonable to assume vAvePrior swung wildly
       *all but vPriorDelta might be negative */
      vConfidence = 1.5*vmagCurrentDev > vmagPriorDev ? 60 : 4.5*vmagCurrentDev > vmagPriorDev ? 50 : 9*vmagCurrentDev > vmagPriorDev ? 45 : 18*vmagCurrentDev > vmagPriorDev ? 40 : 30;
    
    }else if (vmagCurrentDev > vmagPriorDev && (vmagShortDelta < vmagMidDelta || vmagShortDelta < 3.0*vmagMidDelta)){
      /**this is even lower confidence case where it's reasonable to assume vAve swung wildly
       *all but vPriorDelta might be negative */
      vConfidence = vmagCurrentDev < 3*vmagPriorDev ? 50 : vmagCurrentDev < 6*vmagPriorDev ? 45 : vmagCurrentDev < 9*vmagPriorDev ? 40 : 30;
      
    }else if (vmagCurrentDev <= vmagPriorDev){
      /** low confidence */
      vConfidence = 1.5*vmagCurrentDev > vmagPriorDev ? 55 : 4.5*vmagCurrentDev > vmagPriorDev ? 45 : 6*vmagCurrentDev > vmagPriorDev ? 40 : 30;
      
    }else{
      /** lowest confidence */
      vConfidence = vmagCurrentDev < 3*vmagPriorDev ? 40 : 30;
    }
  }
  confidenceSum += vWeighting * vConfidence;
  fullWeightingSum += vWeighting;

  /**
   * KEY POINT: vAvePrior & vAveShortTerm cannot be from the same kpth!! 
   * uint16_t hListIndex = 2; above
   */
  double aConfidence = 0.0;
  double aAve = currentKpth->getAAve();
  double priorAAve = priorKpth->getAAve();
  
  double meanAAve = (aAve + priorAAve + aAveShortTerm) * 0.33;

  double amagCurrentDev = abs(aAve - meanAAve);
  double amagPriorDev = abs(priorAAve - meanAAve);

  double amagPriorDelta = abs(aAve - priorAAve);
  double amagShortDelta = abs(aAve - aAveShortTerm);

  if (amagPriorDelta <= amagShortDelta){
    aConfidence = amagPriorDelta == amagShortDelta ? 84 : 94;
      
  }else if (amagCurrentDev < amagPriorDev){
    /** both vShortDelta and vMidDelta might be negative */
    aConfidence = 1.5*amagCurrentDev > amagPriorDev ? 60 : 4.5*amagCurrentDev > amagPriorDev ? 50 : 9*amagCurrentDev > amagPriorDev ? 45 : 18*amagCurrentDev > amagPriorDev ? 40 : 30;
    
  }else{
    aConfidence = amagCurrentDev < 3*amagPriorDev ? 50 : amagCurrentDev < 6*amagPriorDev ? 45 : amagCurrentDev < 9*amagPriorDev ? 40 : 30;
  }  
  confidenceSum += aWeighting * aConfidence;
  fullWeightingSum += aWeighting;
  
  uint8_t confidence = (uint8_t)(confidenceSum / fullWeightingSum);

  /*
  if (confidence < 38){
	  Serial.println("confidence < 38 confidence = "+String(confidence)+" & confidenceHistory = "+String(confidenceHistorySum/confWeightingSum)+" & numMagForceUpReversals = "+String(numMagForceUpReversals)+" & drConfidence = "+String(drConfidence)+" & vConfidence = "+String(vConfidence)+" & aConfidence = "+String(aConfidence));
  }else if (confidence > 99) {
	  Serial.println("confidence > 99 confidence = "+String(confidence));
  }
  */

  currentKpth->setConfidence(confidence);
  
  if (numMagForceUpReversals > 4) Serial.println("numMagForceUpReversals > 1 numMagForceUpReversals = "+String(numMagForceUpReversals));//+" & mostRecentZRead = "+String(mostRecentZRead)+" & previousZRead = "+String(previousZRead)+" & mfuRevIndex = "+String(mfuRevIndex));
  if (numDirectionReversals > 4) Serial.println("numDirectionReversals > 1 numDirectionReversals = "+String(numDirectionReversals));//+" & mostRecentZRead = "+String(mostRecentZRead)+" & previousZRead = "+String(previousZRead)+" & dirRevIndex = "+String(dirRevIndex));
}

/**
   interruptType is set to MAX_EXTREMA_INTERRUPT to force idling when kinetics are extreme.
   conveniently, it strongly suggests very fast motion which means another sensor read will
   occur quickly (assume 1-2 ms for realistic case). therefore, there's no need to treat as special
   case when coils are idling. just wait as usual for code  to be run. if interruptType is MAX_EXTREMA_INTERRUPT
   and it shouldn't remain MAX_EXTREMA_INTERRUPT then it will be set to RESUME_INTERRUPT and idling will stop
*/
uint8_t Brain::determineInterruptType(uint8_t fKinCode, unsigned long tCurrent, KineticProfileTickHistory* priorKpth){
  using namespace pulsecyclespace;

  uint8_t interruptType = NO_INTERRUPT_CURRENT_PULSE_CYCLE;

  //return NO_INTERRUPT_CURRENT_PULSE_CYCLE;
  
  if (fKinCode != VCALC_SUCCESS){

    switch (fKinCode){

      case KPTHISTORY_DATA_NOT_SET:
        Serial.println("INITIALIZING::KPTHISTORY_DATA_NOT_SET");
        interruptType = MAX_EXTREMA_INTERRUPT;//idle
        break;

      case ACALC_SIGNIFICANT_EVENT:
        //Serial.println("ACALC_SIGNIFICANT_EVENT!!");
        interruptType = SURGE_INTERRUPT;
        break;
        
      case ACALC_MAGFORCEUP_REVERSAL:
        //Serial.println("ACALC_MAGFORCEUP_REVERSAL!!");
        interruptType = SURGE_INTERRUPT;
        break;
        
      case JCALC_SIGNIFICANT_EVENT:
        //Serial.println("JCALC_SIGNIFICANT_EVENT!!");
        interruptType = SURGE_INTERRUPT;
        break;

      case JCALC_WARNING_EXCESSIVE_JERK:
        //Serial.println("JCALC::EXCESSIVE KINETICS HANDLER SHOULD FLAG KPP TO INDICATE NOTHING SHOULD BE DONE - OVERRIDE CODE OF 6 MAYBE - fKinCode = "+String(fKinCode));
        interruptType = MAX_EXTREMA_INTERRUPT;//extreme kinetics
        break;
        
      case VCALC_WARNING_EXCESSIVE_SPEED:
        //Serial.println("VCALC::EXCESSIVE KINETICS HANDLER SHOULD FLAG KPP TO INDICATE NOTHING SHOULD BE DONE - OVERRIDE CODE OF 6 MAYBE - fKinCode = "+String(fKinCode));
        interruptType = MAX_EXTREMA_INTERRUPT;//extreme kinetics
        break;
        
      case ACALC_WARNING_EXCESSIVE_ACCEL:
        //Serial.println("ACALC::EXCESSIVE KINETICS HANDLER SHOULD FLAG KPP TO INDICATE NOTHING SHOULD BE DONE - OVERRIDE CODE OF 6 MAYBE - fKinCode = "+String(fKinCode));
        interruptType = MAX_EXTREMA_INTERRUPT;//extreme kinetics
        break;

      case VCALC_ERROR_UNDETERMINED:
        interruptType = MAX_EXTREMA_INTERRUPT;//extreme kinetics

        Brain::handleCatastrophe("ERROR:: VCALC_ERROR_UNDETERMINED allPinsLow() fKinCode = " + String(fKinCode));
        break;

      case VCALC_MIN_EXTREMA:
        //Serial.println("VCALC_MIN_EXTREMA proceed with short quick pulse widths and long rests fKinCode = " + String(fKinCode));
        interruptType = MIN_EXTREMA_INTERRUPT;//idle
        break;

      default:
        Serial.println("NOT SURE WTF!! fKinCode = " + String(fKinCode));
        interruptType = MAX_EXTREMA_INTERRUPT;//indeterminate kinetics
        break;
    }

  } else if (priorKpth->getInterruptType() == MAX_EXTREMA_INTERRUPT){
    interruptType = RESUME_INTERRUPT;
  }

  if (interruptType == MAX_EXTREMA_INTERRUPT) { 
    /** tFullCycle of EMPTY_PULSING PulseCycle is 250 us 
       * don't keep creating kpt histories every cycle - wait about
       * 1 ms */
    Brain::MAX_PULSECYCLES_SINCE_LAST_RESET = 4; 
    
  }else if(interruptType == MIN_EXTREMA_INTERRUPT){
    /** tFullCycle of STACCATO_PULSING PulseCycle is 1400 us 
       * don't keep creating kpt histories every cycle - wait about
       * 3 ms */
    Brain::MAX_PULSECYCLES_SINCE_LAST_RESET = 2; 
  }
  
  return interruptType;
}

/**
 * all pins are low now so it's safe to idle and wait for input
 */
void Brain::handleCatastrophe(const String s){

	while (true){
		SerialManager* sMan = sMan->getInstance();
		if(sMan->debugAny()) {
			//Serial.println("Brain::handleCatastrophe()@ sMan->debugAny() true.");
			return;

		}else if (sMan->debugChange()){
			/** back to no debugging but sMan took time to read serial
			 * ignore errors for this case
			 */
			Serial.println("Brain::handleCatastrophe()@ sMan->debugChange() true.");
			return;

		}else{
			unsigned long tCurrent = micros();
			CoilContainer* coilCon = coilCon->getInstance();
			coilCon->allPinsLow(tCurrent);
			double tMin = tCurrent/60000000.0;
			Serial.println("Brain::handleCatastrophe()@ "+String(tMin)+" minutes & "+s+".");


			while (Serial.available()){
				//uint8_t gs = sMan->getSerial();
				sMan->getSerial();
			}
			delay(5000);
		}
	}

}

double Brain::readOscillatorProfile(unsigned long tick, double zReading){
	oscPro.doSetVProfileAtTransition = false;
	/**  to preserve the same scf from one period value to the next, there must be an offset. which matches the curves at an inflection point
	 tDelta is set to absolute value of difference because just after booting micros() is smaller than the largest currentTPeriod */
	long currentPeriodMinusOffset = (long)oscPro.currentTPeriod - oscPro.tPeriodOffset;
	long tDelta = (long)tick - currentPeriodMinusOffset;
	long multi = 1L;
	/**
	 * OK.
	 * 1.) currentPeriodMinusOffset needs to ALWAYS be positive
	 * 2.) therefore instead of oscPro.tPeriodOffset subtract the remainder
	 */
	while (tDelta < 0L){
		currentPeriodMinusOffset = ++multi*(long)oscPro.currentTPeriod - oscPro.tPeriodOffset;
		tDelta = (long)tick - currentPeriodMinusOffset;
	}

	double fullCycleFrac = (double)tDelta / (double)oscPro.currentTPeriod;
	double fcfFloor = floor(fullCycleFrac);
	oscPro.scf = fullCycleFrac - fcfFloor;//single cycle fraction
	if ((long)fcfFloor > (long)oscPro.fcfFloor){
		oscPro.numCycles++;
		//Serial.println("numCycles incremented");
	}
	/** match sine waves of different periods near pi/2 phase. this point makes the most sense since it corresponds to lowest speed
	 choice of lowest speed is critical for two reasons ...
	 1. since piston will spend more time around pi/2 than it will around 0 & pi, it's more likely that below condition is met
	 2. even though magnitude of acceleration is greatest here this is where velocity calc will be most accurate.

	 The following condition if met ensures near zero speed. since scf changes from 0.0 to 1.0 and never in the opposite direction, idea is to catch the motion as it's slowing and just before
	 speed is zero at scf = 0.25. this way the next reading is more likely to occur while motion is still slow - i.e. piston can't move much between readings.
	 */
	if (oscPro.numCycles >= oscPro.numCyclesBeforeTransition && oscPro.scf < 0.249 && oscPro.scf > 0.247){
		oscPro.doSetVProfileAtTransition = true;
		unsigned long priorTPeriod = oscPro.currentTPeriod;
		/**
		   NOTE: DO NOT RESET oscPro.scf here. THIS IS THE POINT! We're changing params right at the same point of each cycle.
		*/
		if (oscPro.profileIndex == OSC_SIZE - 1){
			if (oscPro.tPeriodListsIter < oscPro.tPeriodLists.end()-1){
				oscPro.tPeriodListsIter++;
			}else{
				oscPro.tPeriodListsIter = oscPro.tPeriodLists.begin();
			}
			oscPro.tPeriodList = *oscPro.tPeriodListsIter;
			oscPro.profileIndex = 0;
		}else{
			oscPro.profileIndex = oscPro.profileIndex + 1;
		}

		oscPro.numCyclesBeforeTransition = oscPro.numCyclesBeforeTransitionList[oscPro.profileIndex];
		oscPro.currentZMid = oscPro.zMidList[oscPro.profileIndex];//midpoint of sinewave oscillations
		oscPro.currentZAmp = oscPro.zAmpList[oscPro.profileIndex];//maximum excursion of sinewave oscillations
		oscPro.currentTPeriod = (long)(PERIOD_MIN*oscPro.tPeriodList[oscPro.profileIndex]);//period of sinewave oscillations

		/** DEBUG
		if (oscPro.currentTPeriod == (long)(PERIOD_MIN*22.3) || oscPro.currentTPeriod == (long)(PERIOD_MIN*999.9) || oscPro.currentTPeriod == (long)(PERIOD_MIN*4)){
			Serial.println(String(oscPro.currentTPeriod));
		}
		 DEBUG */

		long newOffsetMaybe = (long)((double)tick * ( (double)oscPro.currentTPeriod / (double)priorTPeriod - 1.0 )  + ((double)oscPro.currentTPeriod / (double)priorTPeriod) * (double)oscPro.tPeriodOffset );
		oscPro.tPeriodOffset = newOffsetMaybe;
		oscPro.numCycles = 0;
		/*
		 * BELOW: A TEST WHICH PROVES CURVES MATCH
		long newPeriodMinusOffsetMaybe = (long)oscPro.currentTPeriod - newOffsetMaybe;
		long newTDeltaMaybe = (long)tick - newPeriodMinusOffsetMaybe;

		long currentPeriodMinusOffset2 = (long)oscPro.currentTPeriod - oscPro.tPeriodOffset;
		long tDelta2 = (long)abs((long)tick - currentPeriodMinusOffset2);

		if (tDelta2 > 0L){
		  double  fullCycleFrac = (double)tDelta2 / (double)oscPro.currentTPeriod;
		  double fcfFloor = floor(fullCycleFrac);
		  oscPro.transitionScf = fullCycleFrac - fcfFloor;//single cycle fraction
		}else{
		  Serial.println("tDelta2 is negative. FIX THIS NOW!!");
		}
		* ABOVE: A TEST WHICH PROVES CURVES MATCH
		*/
  }

  if (oscPro.scf < 0.0 || oscPro.scf > 1.0){
    Serial.println("readSensor scf out of bounds");
    return zReading;//can fix this later if needed
  }

  /* HARDCODE FOR TESTING */
  if (hardcodeOsc){
	  oscPro.currentTPeriod = (unsigned long)(minPeriodMult*PERIOD_MIN);
	  oscPro.currentZMid = Z_MID;
	  oscPro.currentZAmp = 1.0*ZAMP_PMIN;
	  oscPro.tPeriodOffset = 0L;
	  oscPro.doSetVProfileAtTransition = false;
	  if (printOscMssg) {
		  Serial.println("@ Brain::readOscillatorProfile hardcoded minPeriodMult = "+(String(minPeriodMult,4)));
		  printOscMssg = false;
	  }
  }else{
	  if (printOscMssg) {
		  Serial.println("@ Brain::readOscillatorProfile running through list of profiles");
		  printOscMssg = false;
	  }
  }
  /* HARDCODE FOR TESTING */
  double scRad = 2 * AMSAEUtilsspace::pi * oscPro.scf; //single cycle radians
  oscPro.zVal = oscPro.currentZMid + oscPro.currentZAmp * sin(scRad);

  if (oscPro.zVal > Z_MAX) Serial.println("osc pro oscPro.zVal = " + String(oscPro.zVal) + " & oscPro.oscPro.currentZAmp = " + String(oscPro.currentZAmp) + " & sin(scRad) = " + String(sin(scRad), 4));

  /** TESTING ONLY - currentKpth->setVProfileAtTransition() should be deprecated as it does nothing to improve curve fitting at transition
  if (oscPro.doSetVProfileAtTransition){
    double scfDelta = abs(oscPro.scf - oscPro.transitionScf);

    if ( scfDelta  > 0.005 ){
      Serial.println("scfDelta  > 0.05 !! scfDelta = "+String(scfDelta));
    }

    double vAtTransition = oscPro.currentZAmp * cos(scRad) * (2 * pi / (double)oscPro.currentTPeriod);
    currentKpth->setVProfileAtTransition(vAtTransition);
  }
  */
  return oscPro.zVal;
}

OscillatorProfile::OscillatorProfile():
	ZMID{Z_MID},
	ZAMPMIN{ZAMP_PMIN},
	numCyclesBeforeTransitionList{ 2, 2, 2, 2, 2, 2},
	zMidList{ ZMID, ZMID, ZMID, ZMID, ZMID, ZMID },
	zAmpList{ ZAMPMIN*1.0, ZAMPMIN*1.0, ZAMPMIN*1.0, ZAMPMIN*1.0, ZAMPMIN*1.0, ZAMPMIN*1.0 },
	tPeriodList1{ 10.0, 4.5, 20.0, 3.0, 20.0, 13.0 },
	tPeriodList2{ 6.0, 7.5, 40.0, 10.0, 80.0, 3.0 },
	tPeriodList3{ 6.0, 750.0, 40.0, 10.0, 80.0, 3.0 },
	tPeriodList4{ 50.0, 30.0, 600.0, 150.0, 21.0, 999.9 },
	tPeriodList5{ 16.0, 27.5, 40.0, 19.0, 50.0, 22.3 },
	tPeriodList6{ 10.0, 10.0, 10.0, 10.0, 10.0, 10.0 }
	{
		currentZMid = 0.0;
		currentZAmp = 0.0;
		currentTPeriod = PERIOD_MIN;
		doSetVProfileAtTransition = false;
		firstOscRead = true;

		tPeriodOffset = 0L;

		profileIndex = 0;
		doTransition = false;

		fcfFloor = 0.0;
		scf = 0.0;//singleCycleFraction

		transitionScf = 0.0;

		zVal = 0.0;
		numCycles = 0;
		numCyclesBeforeTransition = 0;

		tPeriodLists = { tPeriodList6, tPeriodList6, tPeriodList6, tPeriodList6, tPeriodList6 };
		tPeriodList = tPeriodLists.at(0);
}

void OscillatorProfile::init(){
	    scf = 0.0;
	    numCycles = 0;
	    profileIndex = 0;
	    doTransition = false;
	    numCyclesBeforeTransition = numCyclesBeforeTransitionList[profileIndex];
	    currentZMid = zMidList[profileIndex];//midpoint of sinewave oscillations
	    currentZAmp = zAmpList[profileIndex];//maximum excursion of sinewave oscillations
	    currentTPeriod = tPeriodList[profileIndex];//period of sinewave oscillations
	    fcfFloor = 0.0;
	    /**OFFSET IS OF TYPE LONG - IT CAN BE NEGATIVE */
	    tPeriodOffset = 0L;
	    firstOscRead = true;

	    tPeriodLists = { tPeriodList1, tPeriodList2, tPeriodList3, tPeriodList4, tPeriodList5 };
	    tPeriodListsIter = tPeriodLists.begin();
	    tPeriodList = *tPeriodListsIter;
}

boolean Brain::completeSensorReading(unsigned long tSamplingElapsed, vector<uint32_t> &adc1_readings, vector<uint32_t> &adc2_readings, double &adc_readingAve, uint8_t &adcSampleCount){
	/**
	* active range for OPT2003 is 150 mm to 250 mm = 100 mm
	* largest granularity is 100 mm /0.75 mm = 133 = no. significant data points
	* assume linear range between 150 and 2450 or a range of 2300
	* 2300/133 ~= 17 so +/- 8.5 on either side so ... Brain::peakDev = 0.5*adcVInputRange/(opt2003Range/zDelAbsMin)
	* the above are averages to give an idea of the setup. final config is described in AMSAEUtils adcspace
	*
	* sampling over a 1000 ms range. 1000 is a max that is rarely approached but allowed. it's
	* approx twice the average light-duty range.
	*/
	boolean readingComplete = false;
	boolean maybeReadingComplete = false;

	adcSampleCount  = adc1_readings.size() + adc2_readings.size();

	if (adcSampleCount < MIN_SAMPLE_SIZE){
		//most likely case - do nothing - comment out print
		//if (tSamplingElapsed > 1000) Serial.println("BAD NEWS 1: IF THIS HAPPENS MUCH AFTER INIT - UNLESS IT'S DUE TO PRINTING DELAYS");
		readingComplete = tSamplingElapsed > 1000L;

	}else if (adcSampleCount < MAX_SAMPLE_SIZE){
		//next most likely cases
		if (tSamplingElapsed > TMIN_ZREAD_CHANGE){
			if (tSamplingElapsed < 650){
				maybeReadingComplete = true;
			}else{
				readingComplete = true;
			}
		}else{
			//keep sampling even if adcSampleCount == MIN_SAMPLE_SIZE is true
		}
	}else if (adcSampleCount >= MAX_SAMPLE_SIZE){
		readingComplete = true;
	}

	if (readingComplete || maybeReadingComplete){
		/** since readingComplete we can be certain condition Brain::MIN_SAMPLE_SIZE is met. */
		double adc_readingsTot = 0;
		for (vector<uint32_t>::iterator adc1It = adc1_readings.begin(); adc1It != adc1_readings.end(); ++adc1It){
			adc_readingsTot += *adc1It;
		}
		for (vector<uint32_t>::iterator adc2It = adc2_readings.begin(); adc2It != adc2_readings.end(); ++adc2It){
			adc_readingsTot += *adc2It;
		}
		adc_readingAve = (double)adc_readingsTot/adcSampleCount;

		if (maybeReadingComplete){
			readingComplete = true;//the most likely case
			/**
			 * before assigning zVal first inspect both initial and trailing edge of readings for consistency
			 * if deviating significantly from adc_readingAve then later readings are likely to be more accurate
			 * do we keep on taking readings? maybe!
			 *
			 * look at first 2 and last 2 and go from there ...
			 */
			double firstReading = adc2_readings.front();
			double lastReading = adc2_readings.size() > adc1_readings.size() ? adc2_readings.back() : adc1_readings.back();
			double devFirstReading = abs(adc_readingAve - firstReading);
			double devLastReading = abs(adc_readingAve - lastReading);

			if (abs(devFirstReading - devLastReading) > adcspace::peakDev){//weak condition
				/**
				 * analyze
				 */
				if (devLastReading > MAX_VPEAK_DEV){//strong condition
					/** keep the clock running and take more readings */
					readingComplete = false;
					caughtAFish++;
				}
			}
		}
	}
	if (readingComplete) {
		letOneGo++;
	}

	if (letOneGo > 10000){
		//Serial.println("letOneGo = "+String(letOneGo)+" & caughtAFish = "+String(caughtAFish));
		caughtAFish = 0;
		letOneGo = 0;
	}
	return readingComplete;
}

void Brain::checkForSerialUpdates(){
	SerialManager* sMan = sMan->getInstance();
	sMan->setDebugFlagChanged(false);

	while (Serial.available()){
		sMan->getSerial();
		double lastMult = minPeriodMult;
		minPeriodMult = sMan->debugOscillator()*2.5;
		hardcodeOsc = sMan->hardcodeOsc();
		printOscMssg = minPeriodMult != lastMult;
		debugBrain = sMan->debugBrain();
		if (sMan->newTReadMin() && sMan->getTReadMin() > 0) {
			T_READ_MIN = sMan->getTReadMin();
			Serial.println("Brain::T_READ_MIN  set to "+String(T_READ_MIN ));
		}
	}
}

boolean Brain::containersAndManagersAreInitialized(){
	boolean amsaeInitialized = false;

	CoilContainer* coilCon = coilCon->getInstance();
	delayMicroseconds(10);
	KineticProfileTickHistoryContainer* kpthCon = kpthCon->getInstance();
	delayMicroseconds(10);
	PulseCycleManager* pcMan = pcMan->getInstance();
	delayMicroseconds(10);

	if (!coilCon->isInitialized()){
		//do nothing just pass by until PCM initializes CoilCon

	}else if (!pcMan->isInitialized()){
		if (numNewCycles > 10){
			KineticProfileTickHistory* currentKpth = kpthCon->getCurrentKpth();
			uint8_t kpthInterruptType = currentKpth->getInterruptType();
			uint8_t pcInterruptType = pcMan->getCurrentInterruptType();
			if (kpthInterruptType == pulsecyclespace::NO_INTERRUPT_CURRENT_PULSE_CYCLE && pcInterruptType == pulsecyclespace::NO_INTERRUPT_CURRENT_PULSE_CYCLE){
				pcMan->setInitialized(true);
			}else{
				//Serial.println("@ PulseCycleManager NOT INITIALIZED numNewCycles = "+String(numNewCycles)+" && kpthInterruptType = "+String(kpthInterruptType)+" && pcInterruptType = "+String(pcInterruptType));
			}
		}

	}else if (!kpthCon->isInitialized()){
		KineticProfileTickHistory* longTermKpth = kpthCon->getlongTermKpth();
		uint8_t interruptType = longTermKpth->getInterruptType();
		boolean dataIsUsable = longTermKpth->isDataUsable();
		if (dataIsUsable && interruptType == pulsecyclespace::NO_INTERRUPT_CURRENT_PULSE_CYCLE){
			kpthCon->setInitialized(true);
			/** remind: move to base */
			kpthCon->postInitializer();
			/** remind: move to base */
			kpthConInitialized = true;
		}
	}

	amsaeInitialized = coilCon->isInitialized() && pcMan->isInitialized() && kpthCon->isInitialized();

	if (amsaeInitialized){
		Serial.println("@ Brain checking for serial manager updates ...");
		SerialManager* sMan = sMan->getInstance();
		Brain::T_READ_MIN = sMan->getTReadMin();
		Serial.println("@ Brain::LMAN_IDLING_ITERS = "+String(Brain::LMAN_IDLING_ITERS));
		Serial.println("@ Brain::T_READ_MIN = "+String(Brain::T_READ_MIN));
		Serial.println("@ Brain::CONFIDENCE_THRESHOLD_MID = "+String(Brain::CONFIDENCE_THRESHOLD_MID));

		Serial.println("@ Brain::app_main AMSAE initialization complete");
	    Serial.println("");
	}
	return amsaeInitialized;
}

Brain::Brain(){}
