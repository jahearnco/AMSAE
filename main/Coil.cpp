  /*
  Coil.cpp - Library for PulseCycleManager props and methods
*/
#include <Coil.h>
#include <SerialManager.h>

using namespace coilspace;

void Coil::handleBeginPulse(unsigned long tc, long tDelayOffset){
      fieldOn();
      pulsing = true;
      tBeginPulse = tc;
      //if (tBeginFirstPulse == 0L) tBeginFirstPulse = tc;
}

/**
 * Coil::handleEndPulse - this is called for two purposes. 1. normal pulse completion
 * & 2. as a failsafe just before current PC is to be replaced
 * question of the day: if pulsing is false and this method is called, is there anything
 * i need to worry about beyond call to fieldsOff()?
 */
void Coil::handleEndPulse(unsigned long tc, long tDelayOffset){
      fieldsOff();
      if (pulsing){
        pulsing = false;
        tOn = tc - tBeginPulse;

        pulseWidth = 0L;
        tOnTotal = tOnTotal + tOn;//good up to 71 minutes. nice!
        tLifetime = tc - tBeginFirstPulse;  
        dRestPerLifetime = (tOnTotal > 1000000L) ? AMSAEUtils::div((double)(tLifetime-tOnTotal), (double)tLifetime) : 1.0;//3400L-2400L/3400L
        tEndPulse = tc;
      }
}

void Coil::finalizeDummyPulsing(unsigned long tc, long tDelayOffset){
  handleEndPulse(tc, tDelayOffset);
}

void Coil::handleNoChangeOn(long tdo){ }
void Coil::handleNoChangeOff(){ }

uint8_t Coil::priorityTrio[3] = {};

void Coil::fieldOn(){ digitalWrite(highPinNum, HIGH); }
void Coil::fieldsOff(){ digitalWrite(pinNumUp, LOW); digitalWrite(pinNumDown, LOW); }
    
void Coil::allPinsLow(unsigned long tc){
	handleEndPulse(tc, 0L);
	//Serial.print("Coil::ALL PINS LOW & cOrder = ");
	//Serial.println(cOrder);
}

uint8_t Coil::healthCheck(unsigned long tc){
  uint8_t healthCode = C_GOOD_HEALTH;
  if (dRestPerLifetimeMin > 0.001 && tOnTotal > 1000000L){
    boolean tMaxExceeded = (tOn > tOnMax);
    boolean usageExceeded = (dRestPerLifetime < dRestPerLifetimeMin); 
    healthCode = !usageExceeded ? ( !tMaxExceeded ? C_GOOD_HEALTH : C_ERROR_MAX_PULSEWIDTH_EXCEEDED ) : ( !tMaxExceeded ? C_ERROR_USAGE_EXCEEDED : C_ERROR_COIL_ABUSE );
  }
  return healthCode;
}

boolean Coil::isReady(unsigned long tc){ 
  boolean isReady = healthCheck(tc) == C_GOOD_HEALTH;
  return isReady;
}

void Coil::setPulseWidth(unsigned long pw){ pulseWidth = pw; }

/**
 * INDETERMINATE_PULSE_STATUS case is normal and expected. at least two good reasons for getting here ...
 * 1. PC was idling in queue and time ran out before it was enabled and run (maybe should do a check before running????
 * 2. a print statement delayed and time ran out - again, a check would be nice!
 * 3. BIGGER CONCERN: kMan takes too much time gathering and calculating. hope not!
 * 
 * Update on #3 after a lot of time since mentioning the concern: the success of this project directly relates to the success of
 * kMan and the associated classes and routines. acceleration and jerk calculations have proven to be super-efficient and therefore
 * the ONLY obstacle that can prevent this project from succeeding are the sensors. 
 * 
 * case in point, most loops through kMan take approx 1-4 us. finalizeKineticsTHIS IS BEST CASE AND I ACHIEVED IT. AWESOME!!
  */
uint8_t Coil::runPulseSchedule(unsigned long tCurrent, long tDelayOffset){
  uint8_t runStatus = BAD_STATUS;
  uint8_t prePulseEvalCode = tDelayOffset < 0L ? IN_QUEUE : tDelayOffset >= pulseWidth ? PULSE_WIDTH_EXCEEDED : DO_PULSE;
  
  switch(prePulseEvalCode){

    case DO_PULSE:
      if (pulsing){
        handleNoChangeOn(tDelayOffset);
        runStatus = NO_CHANGE_ON;
      }else{
        handleBeginPulse(tCurrent, tDelayOffset);
        runStatus = BEGIN_PULSE;
      }
      break;

    case IN_QUEUE:
      if (!pulsing){
        handleNoChangeOff();
        runStatus = NO_CHANGE_OFF;
      }else{
        handleEndPulse(tCurrent, tDelayOffset);
        //if (cOrder == 104 && zMidLiveRangeBottom != 101.22){
        //if (cOrder == 104){
          //mystery - why doesn't 104 eventually stick with C_PULSE_ENDED?? the idea of getting to this clause is that a coil can get here ONLY ONCE!!
          //THIS IS A STUPID QUeSTION!!
          //that code doesn't stick with a coil. it sticks with a PULSE. DUH!
          //Serial.println("ERROR @ Coil"+String(cOrder)+"::runPulseSchedule() IN_QUEUE and pulsing??? & tDelayOffset ="+String(tDelayOffset)+" & zMidLiveRangeBottom="+String(zMidLiveRangeBottom));
        //}
        runStatus = END_PULSE;
      }
      break;

    case PULSE_WIDTH_EXCEEDED:
      if (pulsing){
        handleEndPulse(tCurrent, tDelayOffset);
        runStatus = END_PULSE;
      }else{
        handleEndPulse(tCurrent, tDelayOffset);
        runStatus = INDETERMINATE_PULSE_STATUS;
      }
      break;

    default:
      Serial.println("ERROR @ Coil"+String(cOrder)+"::runPulseSchedule() CATCH ALL CODING ERROR");
      break;
  }
  /**
   * IMPORTANT! pulsing boolean is updated outside this method and 
   * it MUST stay that way since handleEndPulse is called elsewhere
   */
  return evaluateRunStatus(runStatus, tDelayOffset, pulsing);
}

uint8_t Coil::evaluateRunStatus(uint8_t runStatus, long tdo, boolean coilIsPulsing){
  uint8_t evaluatedStatus = C_ERROR_COIL_ABUSE;

  if (runStatus == END_PULSE || runStatus == INDETERMINATE_PULSE_STATUS){
    evaluatedStatus = C_PULSE_ENDED;
    
  }else if (coilIsPulsing){//most likely case but need to catch the above first
    
    if (tdo < 0){ evaluatedStatus = C_ERROR_PULSING_IN_QUEUE; }
    else if (tdo > pulseWidth){ evaluatedStatus = C_ERROR_PULSEWIDTH_EXCEEDED; }
    else if (tdo >= 0 && tdo <=  pulseWidth){ evaluatedStatus = C_PULSING; }
    else { evaluatedStatus = C_ERROR_UNKNOWN; }  
        
  }else{
    if (tdo < 0){ evaluatedStatus = C_IN_QUEUE_NOT_PULSING; }
    else if (tdo >= 0L && tdo <  pulseWidth){ evaluatedStatus = C_ERROR_NOT_PULSING; }
    else { evaluatedStatus = C_DEFAULT_NOT_PULSING_CODE; }     
  } 
  return evaluatedStatus;
}

/**
 * many ways to implement. I've settled on the following ...
 * 1. quick loop through the livezones returns either the difference between zEst and the liveRangeMidpoint OR DBL_MIN if out of bounds
 * 2. if DBL_MIN not returned value can be +,0,- 
 * 3. there are two values for a coil, one at each pole.
 * 4. depending on magForceUp value, priority relates differently based on proximity to coil pole. For instance, if accel is to be AWAY from coil then
 * zEst close to the pole is high priority. If accel is TOWARD the coil then zEst close to pole has ZERO priority for two reasons: first, pushing magnet
 * too far into coil introduces an unecessary risk in case zEst is far off. second, there is little benefit to pulsing in this condition since force
 * quickly becomes weak once magnet is fully inside the coil
 * 
 * double zEst:
 *    millimeters above x=0 which is 33.5 mm below coil assembly (i.e. below coil A).
 *    measuring bottom of second magnet from top of magnet assembly. needs to be second mag and not first. 
 *    would be considering scenario of a single magnet moving through assembly otherwise - that would be an impossible config.
 *    so, zEst<0 is out of bounds
 *    
 * uint8_t lastMagIndex:
 *    maps specific configuration of magnets relative to this coil. lastMagIndex%2 and magForceUp determine polarNorthUp. BAD_STATUS renders coil disabled
 *  
 * uint8_t priorityScore:
 *    sum of priority scores from each side of coil. higher number = higher priority
 *  
 * double midPointOffset:
 *    distance between zEst and nearest coil midpoint. DBL_MIN if zEst is out of bounds
 *    
 * BIG BIG pat on the back for this method. It has four distinct merits: 
 * 1. no need for polarNorthUp info to set props although this is where polarNorthUp is set
 * 2. symmetry leading to simple code and logic 
 * 3. easy to determine nextPriorityScore because it was well written. 
 * 4. scores are just relative priorities. there is no relationship to absolute powers or speeds.
 * 
 * there are no kludges here. not easy to do for something like this. woo hoo!
 */
uint8_t* Coil::getPriorityTrio(double zEst, boolean magForceUp){
    double mpd = COIL_MIDPOINT_DELTA;
    uint8_t magIndex = BAD_INDEX;
    uint8_t priorityScore = ZERO_PRIORITY;
    uint8_t nextPriorityScore = ZERO_PRIORITY;
    uint8_t nextPriorityScoreMaybe = ZERO_PRIORITY;
    double zEstLowerBoundary = zEst - mpd;
    double zEstUpperBoundary = zEst + mpd;
    uint8_t i = 0;
    double midPoint;
    double midPointOffset;

    for (i=0; i<MAG_NUM-1; i++){
      midPoint = liveRangeMidpoints[i];
      if (zEstUpperBoundary < midPoint){//faster than checking absolute value of difference
        break;
      }else if (zEstLowerBoundary < midPoint){
        magIndex = i+2;//first possible is second magnet from top, then 3rd, 4th, 5th, 6th
        break;
      }
    }
    //nothing more to do if magIndex unchanged. coil is out of bounds and will not be pulsed
    if (magIndex == BAD_INDEX){
      *priorityTrio = *PRIORITY_TRIO_INIT_ARRAY;
      return priorityTrio;
    }

    /**
     * zEstLowerBoundarys can now be valued given magForceUp - convention is 0th config will have pN = 1.0
     */  
   int8_t pN = magIndex%2 == 0 ? 1.0 : -1.0;//keeping track of magnet pole configs 
   polarNorthUp = (magForceUp && magIndex%2 == 0) || (!magForceUp && magIndex%2 != 0);//implementing the convention

   /** can set high pin now that polarNorthUp is set */
   setHighPinNum();
   
   midPointOffset = pN*(zEst - midPoint);//symmetry on either side when force direction is flipped
  /** 
   *  MUST USE FLOOR AND NOT JUST CAST - THIS CAN BE NEGATIVE!!
   */
   int intervalConversion = (int)floor(6.0*midPointOffset/mpd);

  //Serial.println("priorityScore for Coil#"+String(cOrder)+" = "+String(priorityScore)+" & zEst = "+String(zEst)+" & zd/mpd = "+String(zd/mpd)); 
   
   if (intervalConversion < -6) intervalConversion = -6;//just in case of a comparison bug
   if (intervalConversion > 5) intervalConversion = 5;//just in case of a comparison bug
   
   int scaleIndex = polarNorthUp ? 6+intervalConversion : 5-intervalConversion;
   int nextScaleIndex = polarNorthUp ? scaleIndex-1 : scaleIndex+1;//counterintuitive - we are looking at next field going AWAY from magForce direction! 
   
   priorityScore = PRIORITY_SCALE[scaleIndex];

  //Serial.println("priorityScore for Coil#"+String(cOrder)+" = "+String(priorityScore)+" & zEst = "+String(zEst)+" & scaleIndex = "+String(scaleIndex)); 

   /**
    * to set nextPriorityScore need to know in which direction piston is likely moving and NOT the direction we want it to go
    * if a magnet is accelerating into a region of greater strength and therefore force, this affects priority
    * for a few reasons. for one, another coil can be used to push this coil into that field. so may want to delay
    * even though a coil has more strength.
    * 
    * KEY: magForce will likely be in direction OPPOSITE velocity! must consider inertia that keeps piston moving in opposite 
    * direction from magnetic force. in other words - don't forget that this is the entire point of absorption!
    * 
    * Elements at either end of PRIORITY_SCALE are out of bounds, but MAYBE_LIVE is out of bounds and likely moving piston toward becoming in bounds
    * while it's the opposite with ZERO_PRIORITY which is a region where coil is ALWAYS disabled/out of bounds. note that nextPriorityScoreMaybe will
    * never be ZERO_PRIORITY
    * 
    * Note: we don't care if nextScaleIndex is 10 or 11 since those two indices involve too much risk
    */
  if (nextScaleIndex >=0 && nextScaleIndex<11){
    nextPriorityScoreMaybe = PRIORITY_SCALE[nextScaleIndex];
    if (nextPriorityScoreMaybe > priorityScore){
      nextPriorityScore = nextPriorityScoreMaybe;
    }
  }
  priorityTrio[0] = magIndex;
  priorityTrio[1] = priorityScore; 
  priorityTrio[2] = nextPriorityScore; 
  return priorityTrio;
}

unsigned long Coil::getTEndPulse(){ return tEndPulse; }
unsigned long Coil::getTForcedRest(){ return tForcedRest; }
unsigned long Coil::getTOnMax(){ return tOnMax; }
unsigned long Coil::getTOn(){ return tOn; }
double Coil::getDRestPerLifetime(){ return dRestPerLifetime; }
double Coil::getNormalizingFactor(){ return normalizingFactor; }

void Coil::setDRestPerLifetimeMin(double dRestMin){
  dRestPerLifetimeMin = dRestMin;
  tOnTotal = 0L;
  tOn = 0L;
}

void Coil::setHighPinNum(){
	highPinNum = polarNorthUp ? pinNumUp : pinNumDown;
}

void Coil::init(uint8_t o, uint8_t p1, uint8_t p2, unsigned long tMax, double dRestFracMin, unsigned long tRest, double zMb, double nFactor, boolean dummyCoil){
  initProps();
  cOrder = o;
  pinNumUp = p1;
  pinNumDown = p2;
  tOnMax = tMax;
  dRestPerLifetimeMin = dRestFracMin;
  tForcedRest = tRest;
  normalizingFactor = nFactor;
  isDummy = dummyCoil;


  //Serial.println("Coil"+String(cOrder)+" is initialized");

  /**
   * liverange is a misnomer because there are regions at each end of the range
   * where magnets are out of bounds. but we're storing info here anyhow as explained 
   * in uint8_t* Coil::getPriorityTrio(double zEst, boolean magForceUp) and 
   * Coil** CoilContainer::getPrioritizedCoils(double zEst, boolean magForceUp)
   */

  zMidLiveRangeBottom = zMb;

  //e.g. for 6 magnets 5 enabled configurations possible for each coil
  for (uint8_t i=0; i<MAG_NUM-1; i++){
    liveRangeMidpoints[i] = zMidLiveRangeBottom + MAG_FULL_SEP*i;
  }
}

boolean Coil::isADummy(){ return isDummy; }

void Coil::initProps(){
	cOrder = BAD_INDEX;
	polarNorthUp = false;
	pinNumUp = 0;
	pinNumDown = 0;
	tOn = 0L;
	tOnMax = 0L;
	dRestPerLifetimeMin = 0L;
	tOnTotal = 0L;
	tForcedRest = 0L;
	tEndPulse = 0L;
	dRestPerLifetime = 0.0;
	normalizingFactor = 0.0;
	pulsing = false;
	tBeginPulse = 0L;
	tBeginFirstPulse = micros();
	tLifetime = 0L;
	isDummy = false;

	pulseWidth = 0L;
	highPinNum = 0;
	zMidLiveRangeBottom = 160.0;
}

//instead of checking for disabled or priority
Coil::Coil(){
	initProps();
}

uint8_t Coil::getOrder(){ return cOrder; }
