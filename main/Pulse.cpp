  /*
  Pulse.cpp - Library for PulseManager props and methods
*/
#include <Pulse.h>

using namespace coilspace;

uint8_t Pulse::getStatus(unsigned long tCurrent, Coil* coil){ 
  switch(pStatus){
    case C_PULSE_ENDED:
      //SUCCESS do nothing
      break;
      
    case C_ERROR_COIL_ABUSE:
      Serial.println("ERROR C_ERROR_COIL_ABUSE @ Pulse"+String(cOrder)+"::getStatus{}");
      break;

    case C_ERROR_NOT_PULSING:
      Serial.println("ERROR C_ERROR_NOT_PULSING @ Pulse"+String(cOrder)+"::getStatus{}");
      break;

    case C_ERROR_PULSING_IN_QUEUE:
      Serial.println("ERROR C_ERROR_PULSING_IN_QUEUE @ Pulse"+String(cOrder)+"::getStatus{}");
      break;

    case C_ERROR_PULSEWIDTH_EXCEEDED:
      Serial.println("ERROR C_ERROR_PULSEWIDTH_EXCEEDED @ Pulse"+String(cOrder)+"::getStatus{}");
      break;

    case C_ERROR_UNKNOWN:
      Serial.println("ERROR C_ERROR_UNKNOWN @ Pulse"+String(cOrder)+"::getStatus{}");
      break;
      
    default:
      //default is only condition under which pStatus can change
      if (tCurrent - tEndLastPulse < tForcedRest){ 
        pStatus = C_FORCED_REST;
      }else{
        unsigned long tDel = tCurrent > tTimerStart ? tCurrent - tTimerStart : 0L;//PCBAGT: PARANOIA CAN BE A GOOD THING
        long tDelayOffset = (long)tDel - (long)tStartDelay;//REMINDER THAT NOT ALL LONGS ARE ALIKE

        /** 
         *  a bit kludgey simply because I haven't cleaned this up yet and not sure I will since it works perfectly, but dummy coils
         *  really should be treated exactly like real coils, albeit with different pulse widths, delays and RestPerLifetime values. and
         *  specifically to that last point ... is it helpful to code non-zero values for RestPerLifetime values for dummy coils? maybe,
         *  because it may offer a glimpse into how much dummys are being used for PulseCycle::firstPulse & PulseCycle::secondPulse
         *  
         *  regardless, there should be some way of keeping track of how often dummys are used for primary and secondary pulses.
         */
        if (coil->isADummy()){
          coil->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
          if (tDelayOffset > 0L && tDelayOffset >= pulseWidth){
            coil->finalizeDummyPulsing(tCurrent, tDelayOffset);
            pStatus = C_PULSE_ENDED;
          }else{
            pStatus = C_IN_QUEUE_NOT_PULSING;
          }
        }else{
          coil->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
          pStatus = coil->runPulseSchedule(tCurrent, tDelayOffset);
        }
      }
      break;
  }
  return pStatus;
}

void Pulse::enable(){
  if (!enabled){
    coil->setPulseWidth(pulseWidth);
    enabled = true;
  }
}

void Pulse::disable(){ 
  if (enabled){
    enabled = false;
  }
}

unsigned long Pulse::init(uint8_t priority, unsigned long ts, Coil* c, uint8_t type){
  using namespace pulsecyclespace;

  unsigned long tFullCycle = 0L;
  
  if (pStatus != P_PRE_INIT){
    enabled = false;
    Serial.println("ERROR @Pulse"+String(cOrder)+"::init() MULTIPLE INITS. CODING ERROR!!");
    return TON_MAX+1L;
  }
  
  coil = c;
  
  cOrder = coil->getOrder(); 
  tEndLastPulse = coil->getTEndPulse();
  tForcedRest = coil->getTForcedRest();
  normalizingFactor = coil->getNormalizingFactor();
    
  tTimerStart = ts;

  double fullWidth;
  double longestPW;
  unsigned long unNormalizedPulseWidth = 0L;

  switch(type){

    case LOW_POWER_PULSING:
      /** * low power for plush low excursion - dRestPerLifetimeMin = (2400 - 1400)/2400 = 0.417 */
      fullWidth = 4+8;
      longestPW = 7;

      unNormalizedPulseWidth = priority == 1 ? 7*minPW : (priority == 2 ? 6*minPW : 4*minPW);
      tStartDelay = priority == 1 ? 0L : (priority == 2 ? 2*minPW : 8*minPW);
      break;
      
    case MEDIUM_POWER_PULSING:
      /** * medium power for average medium excursion conditions - dRestPerLifetimeMin = (2800 - 1600)/2800 = 0.429 */
      fullWidth = 6+8;
      longestPW = 8;

      unNormalizedPulseWidth = priority == 1 ? 8*minPW : (priority == 2 ? 8*minPW : 6*minPW);
      tStartDelay = priority == 1 ? 0L : (priority == 2 ? 2*minPW : 8*minPW);
      break;

    case HIGH_POWER_PULSING:
      /** high power for large jerk - dRestPerLifetimeMin = (2800 - 2000)/2800 = 0.286 */
      fullWidth = 6+8;
      longestPW = 10;

      unNormalizedPulseWidth = priority == 1 ? 10*minPW : (priority == 2 ? 9*minPW : 6*minPW);
      tStartDelay = priority == 1 ? 0L : (priority == 2 ? 2*minPW : 8*minPW);
      break;
      
    case STACCATO_PULSING:
      /** * minimum power for idling - dRestPerLifetimeMin = (1400 - 800)/1400 = 0.429 */
      fullWidth = 4+3;
      longestPW = 4;

      unNormalizedPulseWidth = priority == 1 ? 4*minPW : (priority == 2 ? 4*minPW : 4*minPW); //
      tStartDelay = priority == 1 ? 0L : (priority == 2 ? 2*minPW : 3*minPW);
      break;

    case EMPTY_PULSING:
      /** * no power for indeterminate cases :: wait for SURGE_INTERRUPT override to choose next option - dRestPerLifetimeMin = (250 - 0)/250 = 1.0 */
	  fullWidth = 0+2;
	  longestPW = 0;

      unNormalizedPulseWidth = priority == 1 ? 0L : (priority == 2 ? 0L : 0L);
      tStartDelay = priority == 1 ? 2*minPW : (priority == 2 ? 2*minPW : 2*minPW);
      break;

    default:
  	  fullWidth = 4+8;
  	  longestPW = 7;

  	  Serial.println("NO GET HERE NO??");
      break;
  }

  pulseWidth = normalizingFactor * unNormalizedPulseWidth;

  adjustedDRestPerLifetimeMin = (fullWidth - longestPW)/fullWidth - 0.02;//0.02 is a safety factor for coils to cool a bit more than the calculated limit
  tFullCycle = unNormalizedPulseWidth + tStartDelay;
  enabled = false;

  pStatus = P_INIT;
  return tFullCycle;
}

Pulse::Pulse(){ 
  cOrder = BAD_INDEX;
  tTimerStart = 0L;
  pulseWidth = 0L;  
  tStartDelay = 0L;
  enabled = false;
  adjustedDRestPerLifetimeMin = 0;
  normalizingFactor = 0;
  pStatus = P_PRE_INIT;
  tEndLastPulse = 0L;
  coil = nullptr;
  tForcedRest = 5*pulsecyclespace::minPW;
}
