/*
  KineticProfileTickHistoryContainer.cpp - Library for KineticProfileTickHistoryContainer props and methods
*/
#include <KineticProfileTickHistory.h>

void KineticProfileTickHistory::updateProfileData(unsigned long tTick, double mostRecentZ){
  setTProfileTickStamp(tTick);
  setProfileZRead(mostRecentZ);
}

/**
* keeping a rolling history of the rolling history kept prior to now
* 
* long term means first kpth that's at least as old as TDEL_LONGTERM_MIN. the kpth
* that corresponds to this also has info going back TDEL_LONGTERM_MIN. and so on.
* kpths corresponding to tDeltas greater than TDEL_LONGTERM_MIN can be dropped pushed
* off the list since we have all their info now.
* 
* these values allow predictive/adaptive behavior. for instancw ...
* 
* CASE 1: if long term aves are slow then coils can be rested more often by changing CoilContainer::
* tForcedRest and/or dRestPerLifetimeMin and/or setting interruptType to 6 (i.e. idling) more often. 
* also in these cases huge accelerations or loss of confidence is much more significant than if long
* term aves are fast. 
* possible solution: gradually ramp up to quick response while gathering more data about the most recent 
* events. in the meantime keep pulsewidths smaller meaning a more plush ride until it's established that
* riding conditions have changed to faster kinetics
* 
* CASE 2: if aves are fast then it means coils are heating, and therefore pulse width control and resting
* are more critical
* possible solution: if long term aves are fast but excursions are small then it's ok to idle and/or to go with 
* short pulsewidths. if long term aves are fast and excursions are not small then we're at the device's limit
* and need to rest coils more, HOWEVER big excursions means lots of coils are involved and there will 
* automatically be resting. this will be case where it's  critical to monitor Coil::dRestPerLifetime and ensure 
* that pulses are long near extreme excursion and short where velocity is greatest.
*/
void KineticProfileTickHistory::setLongTermTDelta(long tDelta){ longTermTDelta = tDelta;  }
void KineticProfileTickHistory::setLongTermVAve(double longTermVAve){ vAveLongTerm = longTermVAve;  }
void KineticProfileTickHistory::setLongTermAAve(double longTermAAve){ aAveLongTerm = longTermAAve; }

void KineticProfileTickHistory::setProfileZRead(double zReading){ mostRecentZRead = zReading; }
void KineticProfileTickHistory::setTProfileTickStamp(unsigned long tpts){ mostRecentTickStamp = tpts; }
void KineticProfileTickHistory::setVProfile(double vP){ vProfile = vP; }
void KineticProfileTickHistory::setVAve(double vA){ vAve = vA; }
void KineticProfileTickHistory::setAAve(double aA){ aAve = aA; }
void KineticProfileTickHistory::setJAve(double jA){ jAve = jA; }
void KineticProfileTickHistory::setDirectionReversal(boolean dirRev){ directionReversal = dirRev; }
void KineticProfileTickHistory::setMagForceUpReversal(boolean mfuRev){ magForceUpReversal = mfuRev; }
void KineticProfileTickHistory::setMagForceUp(boolean mfu){ magForceUp = mfu; }
void KineticProfileTickHistory::setConfidence(uint8_t co){ confidence = co; }
void KineticProfileTickHistory::setJolt(unsigned long jo){ jolt = jo; }
void KineticProfileTickHistory::setPower(unsigned long po){ power = po; }
void KineticProfileTickHistory::setVProfileAtTransition(double vZ){ vProfileAtTransition = vZ; }
void KineticProfileTickHistory::setShortTermKpthListIndex(uint16_t shortTermListIndex){ shortTermKpthListIndex = shortTermListIndex; }
void KineticProfileTickHistory::setMidTermKpthListIndex(uint16_t midTermListIndex){ midTermKpthListIndex = midTermListIndex; }
void KineticProfileTickHistory::setInterruptType(uint8_t kppt){ interruptType = kppt; }


uint8_t KineticProfileTickHistory::getInterruptType(){ return interruptType; }
double KineticProfileTickHistory::getAAve(){ return aAve; }
uint8_t KineticProfileTickHistory::getConfidence(){ return confidence; }
unsigned long KineticProfileTickHistory::getJolt(){ return jolt; }
unsigned long KineticProfileTickHistory::getPower(){ return power; }
uint16_t KineticProfileTickHistory::getShortTermKpthListIndex(){ return shortTermKpthListIndex; }
uint16_t KineticProfileTickHistory::getMidTermKpthListIndex(){ return midTermKpthListIndex; }
double KineticProfileTickHistory::getVProfileAtTransition(){ return vProfileAtTransition; }
double KineticProfileTickHistory::getVAve(){ return vAve; }
double KineticProfileTickHistory::getJAve(){ return jAve; }
boolean KineticProfileTickHistory::getMagForceUp(){ return magForceUp; }
boolean KineticProfileTickHistory::getMagForceUpReversal(){ return magForceUpReversal; }
boolean KineticProfileTickHistory::getDirectionReversal(){ return directionReversal; }
double KineticProfileTickHistory::getVProfile(){ return vProfile; }
double KineticProfileTickHistory::getVAveLongTerm(){ return vAveLongTerm; }
double KineticProfileTickHistory::getAAveLongTerm(){ return aAveLongTerm; }

boolean KineticProfileTickHistory::isDataUsable(){
  return dataIsUsable;
}

double KineticProfileTickHistory::getProfileZRead(){
  return mostRecentZRead;
}
double KineticProfileTickHistory::getZRead(){
  return zRead;
}
unsigned long KineticProfileTickHistory::getTProfileTickStamp(){
  return mostRecentTickStamp;
}

boolean KineticProfileTickHistory::hasBeenReset(){
  return isReset;
}

void KineticProfileTickHistory::initProps(){
	aAve = 0.0;
	aAveLongTerm = 0.0;
	power = 2L;
	vProfile = 0.0;
	interruptType = 0;
	jAve = 0.0;
	jolt = 2L;
	vAve = 0.0;
	tTickZero = 0L;
	dataIsUsable = false;
	confidence = 0;
	mostRecentTickStamp = 0L;
	mostRecentZRead = 0.0;

	directionReversal = false;
	magForceUp = false;
	magForceUpReversal = false;

	zRead = 0.0;
	previousZRead = 0.0;
	vAveLongTerm = 0.0;
	vProfileAtTransition = 0.0;
	longTermTDelta = 0L;
	midTermKpthListIndex = 0;
	shortTermKpthListIndex = 0;
}

KineticProfileTickHistory::KineticProfileTickHistory(){
	initProps();
	isReset = false;
}

void KineticProfileTickHistory::init(unsigned long tTickInit, double mostRecentZ, boolean usableData){
	initProps();
	isReset = true;
	dataIsUsable = usableData;
	zRead = mostRecentZ;
	tTickZero = tTickInit;
	mostRecentTickStamp = tTickZero;
	mostRecentZRead = zRead;
	vProfileAtTransition = DOUBLE_MAX;
}

