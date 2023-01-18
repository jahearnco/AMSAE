/*
  PulseCycleManager.cpp - Library for PulseCycleManager props and methods
*/
#include <SerialManager.h>
#include <Brain.h>

void SerialManager::doWhileNotIdling() {
  /**
   * inefficient to use this time for such fluff.
   * moved to void KineticProfileTickHistoryContainer::checkForSerialUpdates()
   *
  while (blockForSerialInput && Serial.available()){ getSerial(); }
  if (debugSMAN && sManIters++ > serialspace::SMAN_NO_PRINT_ITERS) {
    setReturnTimeElapsed(!getReturnTimeElapsed());
    sManIters = 0L;
  }
  */
}

//precursor to iOS input method
//for now this is where prima,secon,terti coils and their start & delay props
//pcMan is needed here
uint8_t SerialManager::getSerial(){
  uint8_t retDefault =  10;
  uint8_t retCode = retDefault;

  String sRead = Serial.readString();
  sRead.trim();

  if (sRead == "r") { ESP.restart(); return 1; }

  if (sRead == "c") { debugCC = !debugCC;  retCode = 2; } else
  if (sRead == "p") { debugPCM = !debugPCM; retCode = 3; } else
  if (sRead == "t") { debugTMAN = !debugTMAN; retCode = 4; } else
  if (sRead == "b") { debugBRAIN = !debugBRAIN; retCode = 5; } else
  if (sRead == "s") { debugSMAN = !debugSMAN; retCode = 6; } else
  if (sRead == "+") { tReadMin += 5L; tReadMinChanged = true; retCode = 7; } else
  if (sRead == "-") { tReadMin -= 5L; tReadMinChanged = true; retCode = 8; } else

  if (sRead == "0" || sRead == "1" || sRead == "2" || sRead == "3" || sRead == "4" ||
	  sRead == "5" || sRead == "6" || sRead == "7" || sRead == "8" || sRead == "9"){

	  debugOSC = sRead.toDouble();
	  hardcodeOSC = debugOSC > 0;
	  retCode = 9;
  }
  
  debugFlagChanged = retCode != retDefault;
  debugANY = debugCC || debugPCM || debugTMAN || debugBRAIN || debugSMAN || debugFlagChanged;

  return retCode;

}

boolean SerialManager::debugCoilCon(){ return debugCC; }
boolean SerialManager::debugPcMan(){ return debugPCM; }
boolean SerialManager::debugtMan(){ return debugTMAN; }
boolean SerialManager::debugBrain(){ return debugBRAIN; }
boolean SerialManager::debugSMan(){ return debugSMAN; }
boolean SerialManager::debugChange(){ return debugFlagChanged; }
boolean SerialManager::hardcodeOsc(){ return hardcodeOSC; }
double SerialManager::debugOscillator(){ return debugOSC; }
long SerialManager::getTReadMin(){ return tReadMin; }
boolean SerialManager::newTReadMin() { boolean change = tReadMinChanged; tReadMinChanged = false; return change; }
boolean SerialManager::debugAny(){ return debugANY; }

void SerialManager::setDebugFlagChanged(boolean dfChanged){ debugFlagChanged = dfChanged; }


SerialManager::SerialManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo) : TaskManager("sMan", innerLoopIdleIters, outerLoopIdleIters, doPrintIdlingInfo) {
  sManIters = 0L; 
  debugCC = false;
  debugPCM = false;
  debugTMAN = false;
  debugBRAIN = false;
  debugSMAN = false;
  debugOSC = 4;
  hardcodeOSC = true;
  debugFlagChanged = false;
  debugANY = false;
  tReadMin = 85L;
  tReadMinChanged = true;

  setInitialized(true);
}

SerialManager* SerialManager::instance = 0;
SerialManager* SerialManager::getInstance() {
	using namespace serialspace;
	if (!instance) instance = new SerialManager(SMAN_IDLING_ITERS, PCMAN_IDLING_ITERS*Brain::LMAN_IDLING_ITERS*KMAN_IDLING_ITERS*TMAN_IDLING_ITERS, NO_PRINT_IDLE_ITERS_AVE);
	return instance;
}
