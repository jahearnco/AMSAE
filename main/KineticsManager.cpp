/*
  KineticsManager.cpp - Library for KineticsManager props and methods
*/
#include <KineticsManager.h>

void KineticsManager::initKineticsManagerTask(){
  updatePulseTiming = kpthCon->processSensorData(pcMan->getIsPulseCycleNew());
}

void KineticsManager::doWhileNotIdling(){ 
  if (updatePulseTiming){
    /** REMEMBER doWhileNotIdling MAY NOT GET CALLED EVERY TIME*/
    tMan->doLoopTask();
  }
}

KineticsManager::KineticsManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo) : TaskManager("kMan", innerLoopIdleIters, outerLoopIdleIters, doPrintIdlingInfo)  {

  updatePulseTiming = false;
  
  updateTmanCount = 0;
  kManIterCount = 0L;
  tMan = tMan->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!
  pcMan = pcMan->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!
  kpthCon = kpthCon->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!

  setInitialized(true);
}

KineticsManager* KineticsManager::instance = 0;
KineticsManager* KineticsManager::getInstance(){
	using namespace serialspace;
	if (!instance) instance = new KineticsManager(KMAN_IDLING_ITERS, Brain::LMAN_IDLING_ITERS, NO_PRINT_IDLE_ITERS_AVE);
	return instance;
}
