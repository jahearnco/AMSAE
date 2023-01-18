/*
  PulseCycle.cpp - Library for PulseCycleManager props and methods
*/
#include <PulseCycle.h>

using namespace pulsecyclespace;

unsigned long PulseCycle::init(unsigned long po, uint8_t co, boolean mfu, unsigned long ts, Coil** prioritizedCoils, uint8_t pType, uint8_t iType){
  power = po;
  confidence = co;
  pcPulsingCode = PC_INIT;
  magForceUp = mfu;
  tStartPulseCycle = ts; 
  pulseType = pType;
  interruptType = iType;
  enabled = false;
  dummy = false;

  tFullCycle = 0L;  

  c0 = prioritizedCoils[0];
  c1 = prioritizedCoils[1];
  c2 = prioritizedCoils[2];

  initPulses();
  return tFullCycle;
}
    
void PulseCycle::initPulses(){
  unsigned long tFullCycle1 = firstPulse.init(1, tStartPulseCycle, c0, pulseType);
  unsigned long tFullCycle2 = secondPulse.init(2, tStartPulseCycle, c1, pulseType);
  unsigned long tFullCycle3 = thirdPulse.init(3, tStartPulseCycle, c2, pulseType);
  
  tFullCycle = tFullCycle1 >= tFullCycle2 ? (tFullCycle1 >= tFullCycle3 ? tFullCycle1 : tFullCycle3) : (tFullCycle2 >= tFullCycle3 ? tFullCycle2 : tFullCycle3);
}

uint8_t PulseCycle::runCycle(unsigned long tc){
  return pulseCodeDecoder(tc, firstPulse.getStatus(tc, c0), secondPulse.getStatus(tc, c1), thirdPulse.getStatus(tc, c2));
}

uint8_t PulseCycle::pulseCodeDecoder(unsigned long tc, uint8_t p1, uint8_t p2, uint8_t p3){
  using namespace coilspace;

  uint8_t maxHealthyCode = C_FORCED_REST;
  long tDel = tc > tStartPulseCycle ? tc - tStartPulseCycle : 0L;
  
 if (p1 == C_PULSE_ENDED && p2 == C_PULSE_ENDED && p3 == C_PULSE_ENDED){//bypassing check for number of pulses active in cycle
    pcPulsingCode = PC_CYCLE_ENDED;
    //Serial.println("PulseCycle::pulseCodeDecoder PC_CYCLE_ENDED p1,p2 = "+String(p1)+","+String(p2));
    //if (tDel > (long)(tFullCycle+55L)) Serial.println("PulseCycle::debugPulsingCode tDel = " + String(tDel) + " & pcPulsingCode = PC_CYCLE_ENDED");
    
  }else if (p1 == C_DEFAULT_NOT_PULSING_CODE && p2 == C_DEFAULT_NOT_PULSING_CODE && p3 == C_DEFAULT_NOT_PULSING_CODE){//bypassing check for number of pulses active in cycle
    pcPulsingCode = PC_CYCLE_WARNING;
    //Serial.println("PulseCycle::pulseCodeDecoder PC_CYCLE_WARNING p1,p2 = "+String(p1)+","+String(p2));
    Serial.println("PulseCycle::debugPulsingCode tDel = " + String(tDel) + " & pcPulsingCode = PC_CYCLE_WARNING");
    
  }else if (p1 > maxHealthyCode || p2 > maxHealthyCode || p3 > maxHealthyCode){ 
    pcPulsingCode = PC_CYCLE_ERROR;
    //Serial.println("PulseCycle::pulseCodeDecoder PC_CYCLE_ERROR p1,p2,p3 = "+String(p1)+","+String(p2)+","+String(p3));
    Serial.println("PulseCycle::debugPulsingCode tDel = " + String(tDel) + " & pcPulsingCode = PC_CYCLE_ERROR");

  } else if (p1 == C_PULSING || p2 == C_PULSING || p3 == C_PULSING){
    pcPulsingCode = PC_CYCLE_PULSING;
    //Serial.println("PulseCycle::debugPulsingCode tDel = " + String(tDel) + " & pcPulsingCode = PC_CYCLE_PULSING");

  } else if (p1 == maxHealthyCode || p2 == maxHealthyCode || p3 == maxHealthyCode){ 
    pcPulsingCode = PC_CYCLE_RESTING;
    //Serial.println("PulseCycle::debugPulsingCode tDel = " + String(tDel) + " & pcPulsingCode = PC_CYCLE_RESTING");

  } else if (p1 == C_IN_QUEUE_NOT_PULSING || p2 == C_IN_QUEUE_NOT_PULSING || p3 == C_IN_QUEUE_NOT_PULSING){
    pcPulsingCode = PC_NOT_PULSING_IN_QUEUE;
    //Serial.println("PulseCycle::debugPulsingCode tDel = " + String(tDel) + " & pcPulsingCode = PC_NOT_PULSING_IN_QUEUE");

  } else if (p1 == C_ERROR_COIL_ABUSE || p2 == C_ERROR_COIL_ABUSE || p3 == C_ERROR_COIL_ABUSE){
    pcPulsingCode = C_ERROR_COIL_ABUSE;
    Serial.println("PulseCycle::pulseCodeDecoder C_ERROR_COIL_ABUSE p1,p2,p3 = "+String(p1)+","+String(p2)+","+String(p3));
    
  } else {
    pcPulsingCode = PC_CYCLE_WARNING;
    Serial.println("PulseCycle::pulseCodeDecoder PC_CYCLE_WARNING CAUSE UNKNOWN p1,p2,p3 = "+String(p1)+","+String(p2)+","+String(p3));
    //Serial.println("PulseCycle::debugPulsingCode tDel = " + String(tDel) + " & pcPulsingCode = PC_CYCLE_WARNING");
  }
  return pcPulsingCode;
}

uint8_t PulseCycle::getCycleStatus(unsigned long tc, boolean dbg){
  uint8_t cpcStatus = 0;
  long tDelta = tc > tStartPulseCycle ? tc - tStartPulseCycle : 0L;
  
  if (pcPulsingCode == PC_CYCLE_ENDED){
    cpcStatus = PCM_CURRENT_CYCLE_ENDED;
    if (dbg) Serial.println("\n@PC::getCycleStatus PC_CYCLE_ENDED tDelta = "+String(tDelta));
    
  }else{
    double tFullCycleFraction = (double)tDelta/(double)tFullCycle;
    if (tFullCycleFraction < 0.3){
      cpcStatus = PCM_CURRENT_CYCLE_POTENT; 
      if (dbg) Serial.println("\n@PC::getCycleStatus PCM_CURRENT_CYCLE_POTENT tDelta = "+String(tDelta));
      
    }else if (tFullCycleFraction > 0.7){
      cpcStatus = PCM_CURRENT_CYCLE_ENDING; 
      //if (dbg) Serial.println("\n@PC::getCycleStatus PCM_CURRENT_CYCLE_ENDING tDelta = "+String(tDelta));
      
    }else {
      cpcStatus = PCM_CURRENT_CYCLE_POTENT;
      if (dbg) Serial.println("\n@PC::getCycleStatus PCM_CURRENT_CYCLE_POTENT tDelta = "+String(tDelta));
    }
  }
  return cpcStatus;
}

unsigned long PulseCycle::getPower(){ return power; }
uint8_t PulseCycle::getConfidence(){ return confidence; }
boolean PulseCycle::getMagForceUp(){ return magForceUp; }
boolean PulseCycle::isEnabled(){ return enabled; }
void PulseCycle::setOrder(uint8_t o){ myOrder = o; }
uint8_t PulseCycle::getOrder(){ return myOrder; }
unsigned long PulseCycle::getTStartPulseCycle(){ return tStartPulseCycle; }
uint8_t PulseCycle::getPulseType(){ return pulseType; }
uint8_t PulseCycle::getInterruptType(){ return interruptType; }

void PulseCycle::setInterruptType(uint8_t type){
	interruptType = type;
}
    
void PulseCycle::setDisabled(){ 
  enabled = false;
  firstPulse.disable();
  secondPulse.disable();
  thirdPulse.disable();
}

void PulseCycle::setEnabled(){
  enabled = true; 
  //Serial.println("\nPC tStartPulseCycle = "+String(tStartPulseCycle));
  firstPulse.enable();
  secondPulse.enable();
  thirdPulse.enable();
}

PulseCycle::PulseCycle(){
  tFullCycle = 1L;  
  tStartPulseCycle = 0L;
  enabled = false;
  dummy = false;
  pcPulsingCode = PC_PRE_INIT;
  interruptType = 0;
  c0 = nullptr;
  c1 = nullptr;
  c2 = nullptr;

  power = 0L;
  confidence = 0;
  pulseType = 0;
  myOrder = 0;
  prioritizedCoils = nullptr;
  magForceUp = false;
}
