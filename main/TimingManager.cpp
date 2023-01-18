/*
  PulseCycleManager.cpp - Library for PulseCycleManager props and methods
*/
#include <TimingManager.h>
#include <KineticPulseProfile.h>
#include <KineticProfileTickHistoryContainer.h>

void TimingManager::updatePulseCycleManager(){
  pcMan->initPulseCycleTask();
  pcMan->doLoopTask();
}

void TimingManager::doWhileNotIdling(){ 
  /** REMEMBER THIS SHOULD NOT GET CALLED EVERY TIME*/
  updatePulseCycleManager();
}

boolean TimingManager::loopInterrupt(){ return loopInterruptFlag; }

boolean TimingManager::healthCheck(){
  boolean healthCode = pcMan->healthCheck();
  return healthCode;
}

TimingManager::TimingManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo) : TaskManager("tMan", innerLoopIdleIters, outerLoopIdleIters, doPrintIdlingInfo){
  pcMan = pcMan->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!
  tmIterCount = 0L;
  loopInterruptFlag = false;

  setInitialized(true);
}

TimingManager* TimingManager::instance = 0;
TimingManager* TimingManager::getInstance(){
  using namespace serialspace;
  if (!instance) instance = new TimingManager(TMAN_IDLING_ITERS, Brain::LMAN_IDLING_ITERS*KMAN_IDLING_ITERS, NO_PRINT_IDLE_ITERS_AVE);
  return instance;
}
