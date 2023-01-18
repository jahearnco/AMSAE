/*
  Idler.cpp - Library for PulseCycleManager props and methods
*/
#include <Idler.h>

void Idler::initIdler(unsigned long iter1, unsigned long iter2, boolean returnTime){ tGradation = 8.0; idlerIterCountMAX = iter1; outerLoopIdlerIterCountMax = iter2; returnTimeElapsed = returnTime; neverIdle = false; }

double Idler::doWhileIdling() {  return idlingInterval(); }

void Idler::doLoopTask() {

  if (neverIdle) { doWhileNotIdling(); return; }

  double idlingItersAverage = doWhileIdling();

  if (idlingItersAverage != 0.0) {

    if (returnTimeElapsed) {
      /**
       * print helps to optimize code since consistency is desired
       * HOWEVER when called too often slows down the very loop it's trying to measure
       */
      if (printIters++ == 1L) {
		double idlingMicrosAverage = 64.0*tGradation/idlingItersAverage;
		Serial.println("idlingMicros per 64 iters = "+String(idlingMicrosAverage,3));
      }
      tLoopStart = micros();
    }
    /**
     * NEEDS to be called after all tLoopStart resets!!
     */
    doWhileNotIdling();
    //printf("WE NOT IDLING 2\n");
  }else{
	  //yield();
  }
}

/**
 * Idler::idlingInterval
 * - enforces efficiency so looping can tend to Kinetic data updates from sensors rather than managing coils
 * - granularity required for Serial: max t ~ 500ms. min t ~ 5ms
 * - granularity required for coils: max t ~ 50us. min t ~ 5us
 * - granularity required for kinetics: max t ~ ???. min t ~ 5us
 * - returns double iterations per 30us. target is between 30 and 70. this should be monitored
 */
double Idler::idlingInterval(){
  if (idlerIterCountMAX == 0L) return 0.0;//debug

  idlerIterCount++;
  if (idlerIterCount < idlerIterCountMAX) {
    return 0.0;

  }else if (returnTimeElapsed){
    unsigned long currentTime = micros();
    double idlingItersAverage = ((double)idlerIterCountMAX * (double)outerLoopIdlerIterCountMax * tGradation)/(double)(currentTime-tLoopStart);
    idlerIterCount = 0L;
    return idlingItersAverage;

  }else{
    idlerIterCount = 0L;
    printIters = 0L;
    return DOUBLE_MAX;
  }
}

void Idler::setReturnTimeElapsed(boolean rte) { returnTimeElapsed = rte; }
boolean Idler::getReturnTimeElapsed() { return returnTimeElapsed; }

/**
 * Idler::idleAlways
 * - prevent doWhileNotIdling() from ever being performed
 * - quick way to stop coils from firing
 * - usage should include debug dump or Serial.print instead of killing program
 */
void Idler::idleAlways(boolean returnTimeElapsed){ }

/**
 * due to porting to ESP-IDF and RTOS need to deal with
 * different & more 'touchy' timer watchdog
 */
void Idler::feedTheDog(){
  //esp_task_wdt_reset();

  // feed dog 0
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable // @suppress("Field cannot be resolved")
  TIMERG0.wdt_feed=1;                       // feed dog // @suppress("Field cannot be resolved")
  TIMERG0.wdt_wprotect=0;                   // write protect // @suppress("Field cannot be resolved")
  // feed dog 1
  TIMERG1.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable // @suppress("Field cannot be resolved")
  TIMERG1.wdt_feed=1;                       // feed dog // @suppress("Field cannot be resolved")
  TIMERG1.wdt_wprotect=0;                   // write protect // @suppress("Field cannot be resolved")
}

Idler::Idler(const char* subName) : AMSAESingleton(subName){
	idlerIterCountMAX = 1L;
	outerLoopIdlerIterCountMax = 1L;
	idlerIterCount = 0L;
	printIters = 0L;
	returnTimeElapsed = false;
	neverIdle = true;
	tGradation = 0L;
	tLoopStart = 0L;

}

Idler::Idler() : Idler("@Idler()::WRONG CONSTRUCTOR"){ }

Idler::~Idler(){ }




