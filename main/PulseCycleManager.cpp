/*
  PulseCycleManager.cpp - Library for PulseCycleManager props and methods
*/
#include <PulseCycleManager.h>
#include <KineticProfileTickHistoryContainer.h>

KPPHistoryEvaluation PulseCycleManager::kpphe;

void PulseCycleManager::initPulseCycleTask(){
    zEst = KineticProfileTickHistoryContainer::kpp->zEst;
    magForceUp = KineticProfileTickHistoryContainer::kpp->magForceUp;
    power = KineticProfileTickHistoryContainer::kpp->power;
    confidence = KineticProfileTickHistoryContainer::kpp->confidence;
    interruptType = KineticProfileTickHistoryContainer::kpp->interruptType;
    tCurrent = micros();
}

/**
 * the queue has one active PC only and it's always @ index 0
 * indices 1 & 2 keep history needed for incoming eval
 * index 3 is a dummy slot that never changes. it's used to
 * fill other slots with disabled PCs when incoming is sparse
 */
void PulseCycleManager::processCycleQueue(){
  /**
   *
   */
  if (++pcmIterCount == 100000L){
	unsigned long tElapsed = micros() - tStartPCM;
	double aveTPerIter = (double)tElapsed/(double)pcmIterCount;

	if (sMan->debugPcMan()) { Serial.println("@ pcMan::processCycleQueue mins = "+String(tCurrent/60000000.0, 4)+" & aveTPerIter = "+String(aveTPerIter)); }
	//if (true) { Serial.println("@ pcMan::processCycleQueue mins = "+String(tCurrent/60000000.0, 4)+" & aveTPerIter = "+String(aveTPerIter)); }

	pcmIterCount = 0L;
	tStartPCM = micros();
  }

  boolean currentCycleHasExpired = false;

  if (currentPC->isEnabled() && currentPC->getCycleStatus(tCurrent, debug) == pulsecyclespace::PCM_CURRENT_CYCLE_ENDED){
    currentCycleHasExpired = true;
    currentPC->setDisabled();
    coilCon->allPinsLow(tCurrent);
    pCycleList[0] = pCycleList[1];
    pCycleList[1] = pCycleList[2];
    pCycleList[2] = pCycleList[3];
  }

  kpphe = Brain::evaluateKPPHistory(currentCycleHasExpired, tCurrent, confidence, power, interruptType, magForceUp, pCycleList, debug);

  if (kpphe.initAndUpdate) initPulseCycleAndUpdateQueue();

  enableAndRunCurrentCycle();
}

void PulseCycleManager::initPulseCycleAndUpdateQueue(){
  Coil** prioritizedCoils = CoilContainer::getPrioritizedCoils(zEst, magForceUp, tCurrent);
  PulseCycle pc;
  unsigned long tFullCycle = pc.init(power, confidence, magForceUp, tCurrent, prioritizedCoils, kpphe.pulseType, kpphe.interruptType);

  uint8_t qIndexRetainedOld = kpphe.evaluatedKPPQIndexOld;
  uint8_t qIndexRetainedNew = kpphe.evaluatedKPPQIndexNew;
  uint8_t qIndexIncoming = kpphe.incomingKPPQIndex;

  /** maybe overkill but being super careful*/
  if (kpphe.disableCurrentPulseCycle){
    coilCon->allPinsLow(tCurrent);
    pCycleList[0].setDisabled();
  }

  /** trying to be efficient here. no need to do anything if retained new and old are the same*/
  if (qIndexRetainedNew != qIndexRetainedOld){
    pCycleList[qIndexRetainedNew] = pCycleList[qIndexRetainedOld];
  }

  if (tFullCycle <= coilspace::TON_MAX){
    pCycleList[qIndexIncoming] = pc;
  }else{
    killCycles();
    Serial.println("PCM_CYCLE_ERROR @ PulseCycleManager::initPulseCycleAndUpdateQueue");
  }
}

void PulseCycleManager::enableAndRunCurrentCycle(){
  /** the only place this assignment occurs following init*/
  currentPC = &pCycleList[0];

  if (!currentPC->isEnabled()){
    coilCon->allPinsLow(tCurrent);
    currentPC->setEnabled();
    setIsPulseCycleNew(true);
  }

  pcPulsingCode = currentPC->runCycle(tCurrent);

  switch(pcPulsingCode){
    case coilspace::C_ERROR_COIL_ABUSE:
      killCycles();
      Serial.print("C_ERROR_COIL_ABUSE @ PulseCycleManager::enableAndRunCurrentCycle");
      idleAlways(NO_RETURN_TIME_ELAPSED);
      //create loop of print statements - how to get specific coil info?
      break;

    case PCM_CYCLE_WARNING:
      killCycles();
      Serial.print("PCM_CYCLE_WARNING @ PulseCycleManager::enableAndRunCurrentCycle");
      idleAlways(NO_RETURN_TIME_ELAPSED);
      //create loop of print statements - how to get specific cycle info?
      break;

    case PCM_PULSE_ERROR:
      killCycles();
      Serial.print("PULSE_ERROR @ PulseCycleManager::enableAndRunCurrentCycle");
      idleAlways(NO_RETURN_TIME_ELAPSED);
      //create loop of print statements
      break;

    case PCM_CYCLE_ERROR:
      killCycles();
      Serial.print("PCM_CYCLE_ERROR @ PulseCycleManager::enableAndRunCurrentCycle");
      idleAlways(NO_RETURN_TIME_ELAPSED);
      //create loop of print statements
      break;

    default:
      break;
  }
}

/**
 * called approx once every 2000 - 2700 us on average.
 * ok to spend an extra few us extracurricular here
 */
void PulseCycleManager::setIsPulseCycleNew(boolean isNew){
	pulseCycleIsNew = isNew;
	if (pulseCycleIsNew) Brain::numNewCycles++;
}

boolean PulseCycleManager::getIsPulseCycleNew(){
	return pulseCycleIsNew;
}

uint8_t PulseCycleManager::getCurrentInterruptType(){
	return currentPC->getInterruptType();
}

PulseCycle PulseCycleManager::pCycleList[4] = {};

void PulseCycleManager::doWhileNotIdling(){
	processCycleQueue();
}

boolean PulseCycleManager::healthCheck(){
  boolean healthCode = coilCon->healthCheck(tCurrent);
  return healthCode;
}

void PulseCycleManager::killCycles(){
  coilCon->allPinsLow(tCurrent);//seems redundant but here in case of override
}

PulseCycleManager::PulseCycleManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo) : TaskManager("pcMan", innerLoopIdleIters, outerLoopIdleIters, doPrintIdlingInfo){
	using namespace pulsecyclespace;

	coilCon = coilCon->getInstance();
	delayMicroseconds(10);//not sure why this improves run time efficiency but it does!

	sMan = sMan->getInstance();
	delayMicroseconds(10);//not sure why this improves run time efficiency but it does!

	unsigned long pw = 2400L;
	uint8_t co = 100;
	boolean mfu = false;

	pulseCycleIsNew = false;

	interruptType = NO_INTERRUPT_CURRENT_PULSE_CYCLE;
	debug = false;
	pcPulsingCode = PC_PRE_INIT;

	tCurrent = micros();
	tStartPCM = tCurrent;
	pcmIterCount = 0L;

	Coil** prioritizedCoils = CoilContainer::getDummyCoils();
	for (int i=0;i<sizeof(pCycleList)/sizeof(pCycleList[0]);i++){
		PulseCycle pc;
		unsigned long tFullCycle = pc.init(pw, co, mfu, tCurrent, prioritizedCoils, EMPTY_PULSING, INIT_INTERRUPT_TYPE);
		if(tFullCycle > coilspace::TON_MAX){
		  killCycles();
		  Serial.println("PCM_CYCLE_ERROR @ PulseCycleManager::PulseCycleManager(String subName)");
		}
		pCycleList[i] = pc;
	}
	currentPC = &pCycleList[0];
	currentPC->setEnabled();

	coilCon->setInitialized(true);
}

PulseCycleManager* PulseCycleManager::instance = 0;
PulseCycleManager* PulseCycleManager::getInstance(){
	using namespace serialspace;
	if (!instance) instance = new PulseCycleManager(PCMAN_IDLING_ITERS, PCMAN_IDLING_ITERS, NO_PRINT_IDLE_ITERS_AVE);
	return instance;
}
