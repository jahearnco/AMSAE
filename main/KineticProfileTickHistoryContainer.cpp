/*
  KineticProfileTickHistoryContainer.cpp - Library for KineticProfileTickHistoryContainer props and methods
*/
#include <KineticProfileTickHistoryContainer.h>
#include <PositionSensor.h>
 /**
  unsigned long tBeginCalc = micros();
  unsigned long tDeltaCalc = micros() - tBeginCalc;
  if (tDeltaCalc > 40) Serial.println("tDeltaCalc > 10 tDeltaCalc = "+String(tDeltaCalc));

  tKCalcTotal += tDeltaCalc;

  if (++kCalcCount == 5000) {
    double tAvePerCalc = tKCalcTotal / (double)kCalcCount;
    Serial.println("tAvePerCalc = "+String(tAvePerCalc,5));
    kCalcCount = 0;
    tKCalcTotal = 0L;
  }
  */
KineticPulseProfile* KineticProfileTickHistoryContainer::kpp = new KineticPulseProfile();

void KineticProfileTickHistoryContainer::updateKpthList(){
	uint16_t vSize = (uint16_t)kpthVList.size();
	KineticProfileTickHistory* oldestKpth = &kpthVList.at(vSize-1);
	unsigned long oldestProfileTickStamp = oldestKpth->getTProfileTickStamp();
	long largestTDelta = tCurrent > oldestProfileTickStamp ? tCurrent - oldestProfileTickStamp : LONG_MAX;

    if (largestTDelta > TDEL_LONGTERM_MIN*4) {
    	if (largestTDelta == LONG_MAX) {
    		Serial.println("@KineticProfileTickHistoryContainer::updateKpthList largestTDelta == LONG_MAX!!!");
    	}
    	kpthVList.pop_back();
    }else if (vSize > kpthspace::KPTHISTORY_LIST_SIZE_MAX){
    	Serial.println("not sure how but we got here. FIX IT!!");
    }

	KineticProfileTickHistory kpth;
	kpth.init(tCurrent, zReadingMaybe, kpthspace::DATA_IS_USEABLE);

	kpthVList.insert(kpthVList.begin(), kpth);
	currentKpth = &kpthVList.at(0);
	priorKpth = &kpthVList.at(1);
}

uint8_t KineticProfileTickHistoryContainer::finalizeKineticCalcs(){
	return Brain::calculateKinetics(currentKpth, priorKpth, &kpthVList);
}

void KineticProfileTickHistoryContainer::reportKineticResults(uint8_t fKinCode){
  confidence = currentKpth->getConfidence();//confidence of jerk calculation
  power = currentKpth->getPower();//magnitude of jerk
  magForceUp = currentKpth->getMagForceUp();//direction of acceleration
  interruptType = Brain::determineInterruptType(fKinCode, tCurrent, priorKpth);
  currentKpth->setInterruptType(interruptType);
}

/**
  many things can happen here. this is the one interval during normal operation where there's some time.
  that's because it's very unlikely that sensor data will change over the next several loops. so we can play around
  a bit here, clean things up, do complex calculations, look at pulsecycle history???? hey ... that's a good
  one!

   There is good value to recording these profile tickStamps a the end of the ticks' active life, but is there value to calculating
   a profile velocity? so far I don't think so, and am not sure what else should be recorded here. maybe a flag that shows the following
   kpth led to an override? not sure about that either, since that info is available by looking at history

   all i come up with is that the profile tickStamps are significant becasue they give an idea as to how close things occurred and whether the size
   of the time deltas can help to anticipate trends.
*/
void KineticProfileTickHistoryContainer::reset(){

	if (kpthConResetCount++ == 1L){
	  /*** BEGIN:DO TIME CONSUMING STUFF*/
		Brain::checkForSerialUpdates();
		/** this condition doesn't happen frequently because it implies not much movement. for
		 *  that reason this is a good place for doing some cleanup or debugging or extracurricular etc.*/
		if (tCurrent > 3600000000L){
			/** force restart after 60 minutes to avoid unsigned long math issues */
			Brain::handleCatastrophe("NORMAL MAINTENANCE::RESTARTING AFTER ABOUT 60 MINUTES ... 1 ... 2 ... 3 ... GO!");
		}
	  /*** END:DO TIME CONSUMING STUFF*/
	  kpthConResetCount = 0L;
	}

	numCyclesSinceLastReset = 0;
	Brain::MAX_PULSECYCLES_SINCE_LAST_RESET = 1;
	tickerCount = 0;
	zReadingChange = false;
	interruptKpthDefinitionCode = NO_INTERRUPT_KPTH_PROFILING;
	/** don't call micros() here - it's ok for there to be a delay in fact it's more accurate to include it since the idea is to measure tDeltas since last reading */
	tKpthTickZero = tCurrent;
	/** don't call micros() here - it's ok for there to be a delay in fact it's more accurate to include it since the idea is to measure tDeltas since last reading */
	updateKpthList();
}

void KineticProfileTickHistoryContainer::setInterruptKpthDefinitionCode(){
  /** 
   *  numCyclesSinceLastReset usually can't be larger than 2, but if idling (i.e. EMPTY_PULSING) 
   *  new cycles can come more quickly. this is why MAX_PULSECYCLES_SINCE_LAST_RESET  
   *  is a good candidate for app config or for allowing more adaptive functionality
   */
  if (numCyclesSinceLastReset > Brain::MAX_PULSECYCLES_SINCE_LAST_RESET){
    interruptKpthDefinitionCode = UNINTERRUPTED_PULSECYCLING;

  } else if (zReadingChange){
    interruptKpthDefinitionCode = ZREADING_CHANGED;

  } else if (Brain::oscPro.doSetVProfileAtTransition){
    interruptKpthDefinitionCode = SIMULATION_FORCED_INTERRUPT;

  } else {
    interruptKpthDefinitionCode = NO_INTERRUPT_KPTH_PROFILING;
  }
}

void KineticProfileTickHistoryContainer::updateZReading(){
  /**
    interesting point: the least important time to call readSensor is right after reset(); it's likely that an important
    sensor read just occurred on the previous call - it may have changed from the previous 50 - 100 readings for instance. that makes it
    very unlikely that it changes within 4 readings after that.

    try bitwise math to get faster here ...
    n % m == n & (m - 1)

    tried it. NOT faster!
  */
  double sensorRead = readPositionSensor(tSensorTick, zReadingMaybe, zReading, false);//sMan->debugPcMan());
  double zmagDelta = abs(zReading - sensorRead);

  if (zReadingChange){
	  Serial.println("not sure why we're here when zReadingChange is set to false in reset and reset is ALWAYS called after zReadingChange is true?!");
  }

  if (zmagDelta >= Brain::ZREAD_DIFF_MIN){
	  zReadingMaybe = sensorRead;
	  zReading = zReadingMaybe;
	  zReadingChange = true;

  }else if (zmagDelta > Brain::ZREAD_DIFF_MIN*0.5){
	  zReadingMaybe = sensorRead;
  }
  /**
     DECISION TO MAKE: if going with tSensorTick then tCurrent will be off by amount of time needed to gather data
     what is the danger of using namespace this value for pulse start time? the time will be EARLIER so .. who cares, right?
     keep in mind that new PC was already ... created, put in queue, enabled & likely run, so this is out of date
     or rather this happens one full loop AFTER the new PC was processed
  */
}

/** NOTE: PRIMARY REASON FOR MAINTAINING A FAST LOOP (ex here every 20us or so) VERSUS A SLOW LOOP (ex here every 50us):
   take a micros() reading here. it must be here since this is as close to the actual measurement time as possible, but
   if the calc takes some time then this value will be off a bit IF running a slow loop. one way to deal: don't call this
   tCurrent but rather tSensorTick or something. Then tCurrent is set to micros() just before call to tMan->initXXX
  */
boolean KineticProfileTickHistoryContainer::processSensorData(boolean isPulseCycleNew){
  using namespace pulsecyclespace;
  
  if (isPulseCycleNew) {
	  pcMan->setIsPulseCycleNew(false);
	  numCyclesSinceLastReset++;
  }

  boolean thisReadIsATimeRead = false;
  tSensorTick = micros();

  if (lastReadWasATimeRead){
	  thisReadIsATimeRead = false;
	  doRead = true;

  }else{
	  thisReadIsATimeRead = tSensorTick - lastTRead > Brain::T_READ_MIN;
	  doRead = thisReadIsATimeRead;
  }

  if (doRead){
	lastReadWasATimeRead = thisReadIsATimeRead;
	lastTRead = tSensorTick;
	updateZReading();
    /**
     * ARE YOU SURE ABOUT THIS??
     * MAKE SURE! what is the tDiff between tSensorTick and tCurrent here? FIND OUT!!
     * The reason it's a question: what is the correct time of the sensor reading? that's the question. is it tSensorTick or tCurrent or maybe their average?
     */
    tCurrent = tSensorTick + 35L;//35 is the approx time it takes per updateZReading(); - better than calling micros() again
    //tCurrent = micros();
    /**
     * ARE YOU SURE ABOUT THIS??
     * MAKE SURE! what is the tDiff between tSensorTick and tCurrent here? FIND OUT!!
     */
  }else{
    tCurrent = tSensorTick;
  }

  if (isPulseCycleNew) kpthConIterCount++;

  if (Brain::REALIZED_MIN_EXTREMA_INTERRUPT) { numStaccatoInterrupts++; Brain::REALIZED_MIN_EXTREMA_INTERRUPT = false; }
  if (Brain::REALIZED_SURGE_INTERRUPT) { numSurgeInterrupts++; Brain::REALIZED_SURGE_INTERRUPT = false; }
  if (interruptType == MAX_EXTREMA_INTERRUPT) { numExtremaInterrupts++; }
  if (interruptType == RESUME_INTERRUPT) { numResumeInterrupts++; }

  if (kpthConIterCount == 2000L){
	long tDelIters = tCurrent > tStartTimingCycle ? tCurrent - tStartTimingCycle : 0L;
	double tDelAve = (1.0 * tDelIters + 0.5) / (double)kpthConIterCount;

	if (Brain::debugBrain){
		Serial.println("ave cycling = " + String(tDelAve) + " & numStaccatoInterrupts = " + String(numStaccatoInterrupts) + " & numSurgeInterrupts = " + String(numSurgeInterrupts) + " & numExtremaInterrupts = " + String(numExtremaInterrupts) + " & numResumeInterrupts = " + String(numResumeInterrupts));
	}

	kpthConIterCount = 0L;

	numStaccatoInterrupts = 0;
	numSurgeInterrupts = 0;
	numExtremaInterrupts = 0;
	numResumeInterrupts = 0;

	tCurrent = micros();
	tStartTimingCycle = tCurrent;
  }

  //if (updateTicker() && zReading < kpthspace::Z_MAX && zReading > kpthspace::Z_MIN){ //meant to be true 100% of time except for catastrophe
  if (zReading < kpthspace::Z_MAX && zReading > kpthspace::Z_MIN){ //meant to be true 100% of time except for catastrophe
    /** needs to be called before any reset calls - this is needed when finalizing currentKpth and for doing calcs */
    
    setInterruptKpthDefinitionCode();
    updateCurrentKineticProfile();
    
    if (interruptKpthDefinitionCode != NO_INTERRUPT_KPTH_PROFILING){
      /** this is most time consuming aspect of device. it takes approx 67 us per call. not terrible.
       * not a problem as long as sensor reading isn't too time consuming*/
      uint8_t fKinCode = finalizeKineticCalcs();
      reportKineticResults(fKinCode);
      generatePulseProfileData();
      /**
         WHEN TO RESET?? reset means that all kpth move to next higher slot in queue and a new currentKpth is created and put into zeroth slot
         also means that profiles are calculated. What else. is that it? Since there's no reason to create a new kpth every time isPulseCycleNew is true,
         then don't do it! fine, but you will now have issues where tickerCount gets too large. that's fine as well, but handle it keeping in mind
         that tickerCount will be especially large for short loop idles
  
         so question is how large a tickerCount is too large? that's related to the max amount of time before a zReading changes. assume that's 20 ms or 20000 us
         A typical situation is approx 1 tickerCount per 50 us so 20000 us ~ 400
         OR if moving fast can have 1 ticker count per 8 us for ideal situations so ~ 2500
         OR if calcs and readings take long then 1 tickerCount per 200us so ~ 100
         so ... what is maxTickerCount allowed before creating new currentKpth?
      */
      reset();
    }
    return true;

  } else {
    //shut everything down
    if (zReading >= kpthspace::Z_MAX || zReading <= kpthspace::Z_MIN) Serial.println("WHOOPS! zReading = " + String(zReading) + " Shouldn't be here!! piston is out of bounds!! LEAVE THIS IN PLACE!!");
    coilCon->allPinsLow(tCurrent);
    return false;
  }
}

boolean KineticProfileTickHistoryContainer::updateTicker(){
  boolean success = true;//most likely as there are over 1000 run thrus
  tickerCount++;

  if (tickerCount == 100){
    coilCon->allPinsLow(micros());
    Serial.println("@ zReading = " + String(zReading) + " NEW PCs AREN'T BEING RUN OR COMPLETED. IS LOOP TOO FAST? TICKER_COUNT_MAX TOO LOW? PULSE WIDTHS, DELAYS, TON_MAX TOO HIGH? note: this print statement could create some bad delays and even loops");
    tickerCount = 0;
    success = false;
  }
  return success;
}

void KineticProfileTickHistoryContainer::preInitializer(){
	readPositionSensor = PositionSensor::readSensorInitializer;//PositionSensor::readSensor(tSensorTick, zReadingMaybe, zReading, initialized);
}

void KineticProfileTickHistoryContainer::postInitializer() {
	readPositionSensor = PositionSensor::readSensor;//PositionSensor::readSensor(tSensorTick, zReadingMaybe, zReading, initialized);
}

KineticProfileTickHistoryContainer::KineticProfileTickHistoryContainer() : AMSAESingleton("kpthCon"){
  preInitializer();

  coilCon = coilCon->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!
  sMan = sMan->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!
  pcMan = pcMan->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!

  tickerCount = 0;
  kCalcCount = 0;
  tKCalcTotal = 0L;
  sensorReadIters = 0L;
  kpthConIterCount = 0L;
  kpthConResetCount = 0L;
  numStaccatoInterrupts = 0;
  numSurgeInterrupts = 0;
  numExtremaInterrupts = 0;
  numResumeInterrupts = 0;
  tStartTimingCycle = micros();
  tSensorTick = tStartTimingCycle;
  tCurrent = tStartTimingCycle;
  lastTRead = tStartTimingCycle;
  tKpthTickZero = tStartTimingCycle;
  tDebugTick = 0L;
  numCyclesSinceLastReset = 0;
  tReadMin = Brain::T_READ_MIN;
  doRead = true;
  lastReadWasATimeRead = false;

  /**
     need intelligent init val for zReading.
     const double Z_MAX = 260.0;//214.0;//verysafe
     const double Z_MIN = 127.0;//162.0;//verysafe
  */
  zReadingChange = false;
  interruptKpthDefinitionCode = NO_INTERRUPT_KPTH_PROFILING;
  zReading = kpthspace::Z_MID;
  zReadingMaybe = kpthspace::Z_MID;
  magForceUp = false;
  interruptType = 0;
  confidence = 0;
  power = 0L;

  /**
   * only two history objects needed @ init. more will be created
   * as needed to keep long term history up to
   */
  for (uint16_t i = 0; i < kpthspace::KPTHISTORY_LIST_SIZE_MIN; i++){
    KineticProfileTickHistory kpth;

    kpth.init(tCurrent + i, zReading, !kpthspace::DATA_IS_USEABLE);
    kpthVList.push_back(kpth);
  }

  currentKpth = &kpthVList.at(0);
  priorKpth = &kpthVList.at(1);

  //init here because Brain::oscPro would have been defined long before now
  Brain::oscPro.init();
}

void KineticProfileTickHistoryContainer::updateCurrentKineticProfile(){
  currentKpth->updateProfileData(tCurrent, zReadingMaybe);//zReadingMaybe not zReading. could be more accurate when comparing profiles
}

void KineticProfileTickHistoryContainer::generatePulseProfileData(){
  kpp->zEst = zReading;
  kpp->magForceUp = magForceUp;
  kpp->power = power;
  kpp->confidence = confidence;
  kpp->interruptType = interruptType;
}

KineticProfileTickHistory* KineticProfileTickHistoryContainer::getlongTermKpth(){
	return &(kpthVList.at(kpthspace::KPTHISTORY_LIST_SIZE_MIN-1));
}

KineticProfileTickHistory* KineticProfileTickHistoryContainer::getCurrentKpth(){
	return currentKpth;
}

KineticProfileTickHistoryContainer* KineticProfileTickHistoryContainer::instance = 0;
KineticProfileTickHistoryContainer* KineticProfileTickHistoryContainer::getInstance(){
  if (!instance) instance = new KineticProfileTickHistoryContainer();
  return instance;
}
