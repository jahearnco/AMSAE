/*
  LoopManager.cpp - Library for PulseCycleManager props and methods
*/
#include <LoopManager.h>

void LoopManager::doWhileNotIdling(){ 
  /** REMEMBER doWhileNotIdling MAY NOT GET CALLED EVERY TIME*/
  kMan->initKineticsManagerTask();
  /** REMEMBER doWhileNotIdling MAY NOT GET CALLED EVERY TIME*/
  kMan->doLoopTask();

  feedTheWatchDog();
}

void LoopManager::feedTheWatchDog(){
	loopIters++;

	if (loopIters == safeLoopIterMaxForWatchdogFeed){
		//Idler::feedTheDog() - super safe - takes only 12-20 microseconds!!
		/*
		 * gettimeofday(&tv1, NULL);
		 */

		Idler::feedTheDog();

		/*
		gettimeofday(&tv2, NULL);
		unsigned long lDelSec = (unsigned long)((tv2.tv_sec - tv1.tv_sec)*1000000L);
		unsigned long lDelUs = (unsigned long)(tv2.tv_usec - tv1.tv_usec);
		double dDelUs = (double)(lDelSec + lDelUs);
		double usPerIter = dDelUs/(1.0*loopIters);
		printf("us Per Idler::feedTheDog = ");
		printf("%.3f", dDelUs);
		printf("\n");
		*/

		loopIters = 0L;
	}

	//gettimeofday(&tv1, NULL);
}

void LoopManager::testPulsePins(){
	digitalWrite(pulsecyclespace::pulsePin_1, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_1, LOW);

	delayMicroseconds(3000);

	digitalWrite(pulsecyclespace::pulsePin_2, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_2, LOW);
	delay(20);

	digitalWrite(pulsecyclespace::pulsePin_3, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_3, LOW);

	delayMicroseconds(3000);

	digitalWrite(pulsecyclespace::pulsePin_4, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_4, LOW);
	delay(20);

	digitalWrite(pulsecyclespace::pulsePin_5, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_5, LOW);

	delayMicroseconds(3000);

	digitalWrite(pulsecyclespace::pulsePin_6, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_6, LOW);
	delay(20);

	digitalWrite(pulsecyclespace::pulsePin_7, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_7, LOW);

	delayMicroseconds(3000);

	digitalWrite(pulsecyclespace::pulsePin_8, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_8, LOW);
	delay(20);

	digitalWrite(pulsecyclespace::pulsePin_9, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_9, LOW);

	delayMicroseconds(3000);

	digitalWrite(pulsecyclespace::pulsePin_0, HIGH);
	delayMicroseconds(800);
	digitalWrite(pulsecyclespace::pulsePin_0, LOW);
	delay(20);
}

LoopManager::LoopManager(unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo) : TaskManager("lMan", innerLoopIdleIters, outerLoopIdleIters, doPrintIdlingInfo)  {
	using namespace pulsecyclespace;

	pinMode(pulsePin_1, OUTPUT);
	pinMode(pulsePin_2, OUTPUT);
	pinMode(pulsePin_3, OUTPUT);
	pinMode(pulsePin_4, OUTPUT);
	pinMode(pulsePin_5, OUTPUT);     // Initialize the pulsePin_1 pin as an output
	pinMode(pulsePin_6, OUTPUT);
	pinMode(pulsePin_7, OUTPUT);     // Initialize the pulsePin_1 pin as an output
	pinMode(pulsePin_8, OUTPUT);
	pinMode(pulsePin_9, OUTPUT);     // Initialize the pulsePin_1 pin as an output
	pinMode(pulsePin_0, OUTPUT);

	loopIters = 0L;
	safeLoopIterMaxForWatchdogFeed = 1000L;
	gettimeofday(&tv1, NULL);

	CoilContainer* coilCon = coilCon->getInstance();
	delayMicroseconds(10);
	KineticProfileTickHistoryContainer* kpthCon = kpthCon->getInstance();
	delayMicroseconds(10);
	PulseCycleManager* pcMan = pcMan->getInstance();
	delayMicroseconds(10);
	kMan = kMan->getInstance();
	delayMicroseconds(10);//not sure why this improves run time efficiency but it does!

	setInitialized(true);
}

LoopManager* LoopManager::instance = 0;
LoopManager* LoopManager::getInstance(){
	using namespace serialspace;
	if (!instance) instance = new LoopManager(Brain::LMAN_IDLING_ITERS, 1L, NO_PRINT_IDLE_ITERS_AVE);
	return instance;
}
