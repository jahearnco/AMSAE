/*
  CoilContainer.cpp - Library for CoilContainer props and methods
*/
#include <CoilContainer.h>

using namespace coilspace;

SerialManager* CoilContainer::sMan = nullptr;

Coil* CoilContainer::coil1 = nullptr;
Coil* CoilContainer::coil2 = nullptr;
Coil* CoilContainer::coil3 = nullptr;
Coil* CoilContainer::coil4 = nullptr;
Coil* CoilContainer::coil5 = nullptr;
Coil* CoilContainer::coilW = nullptr;
Coil* CoilContainer::coilX = nullptr;
Coil* CoilContainer::coilY = nullptr;
Coil* CoilContainer::coilZ = nullptr;

Coil* CoilContainer::prioritizedCoils[3] = {}; 

Coil* CoilContainer::coilList[5] = {};

Coil** CoilContainer::getDummyCoils(){
  prioritizedCoils[0] = coilX;
  prioritizedCoils[1] = coilY;
  prioritizedCoils[2] = coilZ;
  return prioritizedCoils;
}

Coil** CoilContainer::getPrioritizedCoils(double zEst, boolean magForceUp, unsigned long tc){
  uint8_t magIndexCoilA = BAD_INDEX;
  uint8_t magIndexCoilB = BAD_INDEX;
  uint8_t magIndexCoilC = BAD_INDEX;
  uint8_t magIndexCoilD = BAD_INDEX;
  uint8_t magIndexCoilE = BAD_INDEX;

  uint8_t magIndex = BAD_INDEX;
  uint8_t* priorityTrio;
  uint8_t pt1 = ZERO_PRIORITY, pt2 = ZERO_PRIORITY;
  uint8_t p00 = ZERO_PRIORITY, p01 = ZERO_PRIORITY, p10 = ZERO_PRIORITY, p11 = ZERO_PRIORITY;
  uint8_t i00 = BAD_INDEX, i01 = BAD_INDEX, i02 = BAD_INDEX, i10 = BAD_INDEX, i0OOB = BAD_INDEX;
  uint8_t i = 0;
  Coil* c;
  
  for (i = 0; i < 5; i++){ 
    c = coilList[i];
    
    if (!c->isReady(tc)){
    	if (!sMan->debugAny()){
    		Serial.print("\ncontinuing since Coil"+String(c->getOrder())+ "is not ready & dRestPerLifetime = "+String(c->getDRestPerLifetime())+" & tOn = "+String(c->getTOn()));
    	}
    	continue;
    }
    
    priorityTrio = c->getPriorityTrio(zEst, magForceUp);
    magIndex = priorityTrio[0];//2 to MAG_NUM possible. 1 is not: 2 magnets in field range needed for device to work
    
    if (magIndex == BAD_INDEX){ 
      //Serial.print("\ncontinuing sice coil @ "+String(i)+ "had bad magIndex"); 
      continue;
    }else if (i == 0){
    	magIndexCoilA = magIndex;
    }else if (i == 1){
    	magIndexCoilB = magIndex;
    }else if (i == 2){
    	magIndexCoilC = magIndex;
    }else if (i == 3){
    	magIndexCoilD = magIndex;
    }else if (i == 4){
    	magIndexCoilE = magIndex;
    }

    pt1 = priorityTrio[1];
    pt2 = priorityTrio[2];

    //Serial.print("\ncfor coil @ "+String(i)+ "pt1  = "+String(pt1)+" & magIndex = "+String(magIndex));
    
    //never need to compare with p02  since only 3 coils. note i1 and i2 may equal BAD_INDEX
    if (pt1 > MAYBE_LIVE){
      if (p00 > pt1){ 
        if (p01 > pt1){ i02 = i; }else{ i02 = i01; p01 = pt1; i01 = i; }
      }else{ 
        i02 = i01; p01 = p00; i01 = i00; p00 = pt1; i00 = i;
      }
    }else if (pt1 == MAYBE_LIVE){
      /**
       * coil is not in a live zone but is *likely* moving in that direction from within the coil. this is useful
       * when a coil needs rest or when to add a third coil. assign it to i02 here if there is no i02. how bout that?
       * 
       * I think maybe there will be more than one coil like this. possible you could have all 5 coils going?
       */
      i0OOB = i;    
      //Serial.print("\ncfor coil @ "+String(i)+ "pt1  = MAYBE_LIVE & magIndex = "+String(magIndex)+" & zEst = "+String(zEst));
    }
    
    if (pt2 > MAYBE_LIVE){
      if (p10 > pt2){ 
        if (p11 <= pt2){ p11 = pt2; }
      }else{ 
        p11 = p10; p10 = pt2; i10 = i;
      }
    }
    //if (zEst == 113.0) Serial.println("i = "+String(i)+" for Coil#"+String(c->getOrder()));
  }
  /**
   * idea is to swap if lower priority coils have higher next priority. b/c accel opposes mag force. accel is moving
   * mags into a region of greater priority in this case
   * - one benefit here is that first pulses can be short relative to others assuming there is more than one coil involved
   * - a heuristic of a sort : if one coil only then long pulsewidth, if more the first pulse is short while the second is 
   * - longer, if three the same but third needs to be short to keep delay short and so 3 not firing at once
    */  
  if (i10 == i01 && p10 > ZERO_PRIORITY){
    uint8_t coil2Index = i01; //Serial.print(i01);
    i01 = i00;//Serial.print(i01);
    i00 = coil2Index;//Serial.print(i00);Serial.println(i01);
  }

  if (i02 == BAD_INDEX && i0OOB != BAD_INDEX){
    i02 = i0OOB;
  }

  //if (sMan->debugCoilCon() && (i00 == BAD_INDEX || i01 == BAD_INDEX)) Serial.println("\nCoil* CoilContainer::getPrioritizedCoils ONLY ONE COIL FIRING for zEst = "+String(zEst));
  boolean overlapping = false;
  if ((i00 == 0 || i01 == 0 || i02 == 0) && (i00 == 3 || i01 == 3 || i02 == 3)){
  	  overlapping = true;
  }else if ((i00 == 1 || i01 == 1 || i02 == 1) && (i00 == 3 || i01 == 3 || i02 == 3)){
	  overlapping = true;
  }else if ((i00 == 2 || i01 == 2 || i02 == 2) && (i00 == 4 || i01 == 4 || i02 == 4)){
	  overlapping = true;
  }

  boolean idealRange = false;
  if (!overlapping && (i00 == 4 || i01 == 4 || i02 == 4) ){
	  idealRange = true;
  }

  prioritizedCoils[0] = (i00 != BAD_INDEX) ? coilList[i00] : coilW;
  prioritizedCoils[1] = (i01 != BAD_INDEX) ? coilList[i01] : coilX;
  prioritizedCoils[2] = (i02 != BAD_INDEX) ? coilList[i02] : coilY;

  if (sMan->debugAny()){
	ccDebug = true;

	double adjustedDRestPerLifetimeMin = 0.0;
    coil1->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coil2->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coil3->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coil4->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coil5->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coilW->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coilX->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coilY->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coilZ->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);

    int crumbs = tc%1500;
    if (sMan->debugCoilCon() && crumbs < 60) {
    	if (overlapping) Serial.println("Coils overlapping!!");
    	if (idealRange) Serial.println("Coils ideal range!!");
    	Serial.println("magForceUp = "+String(magForceUp)+" & zEst = "+String(zEst)+" & Coil #"+String(prioritizedCoils[0]->getOrder())+" & Coil #"+String(prioritizedCoils[1]->getOrder())+" & Coil #"+String(prioritizedCoils[2]->getOrder())+" & magIndex = #"+String(magIndex)+" & pt1 = "+String(pt1)+" & pt2 = "+String(pt2));
    	/*
    	 *
    	if (i00 == 0) Serial.println("magForceUp = "+String(magForceUp)+" & magIndexCoilA = "+String(magIndexCoilA)+" & zEst = "+String(zEst)+" & Coil #"+String(prioritizedCoils[0]->getOrder())+" & Coil #"+String(prioritizedCoils[1]->getOrder())+" & Coil #"+String(prioritizedCoils[2]->getOrder())+" & magIndex = #"+String(magIndex)+" & pt1 = "+String(pt1)+" & pt2 = "+String(pt2));
    	if (i00 == 1) Serial.println("magForceUp = "+String(magForceUp)+" & magIndexCoilB = "+String(magIndexCoilB)+" & zEst = "+String(zEst)+" & Coil #"+String(prioritizedCoils[0]->getOrder())+" & Coil #"+String(prioritizedCoils[1]->getOrder())+" & Coil #"+String(prioritizedCoils[2]->getOrder())+" & magIndex = #"+String(magIndex)+" & pt1 = "+String(pt1)+" & pt2 = "+String(pt2));
    	if (i00 == 2) Serial.println("magForceUp = "+String(magForceUp)+" & magIndexCoilC = "+String(magIndexCoilC)+" & zEst = "+String(zEst)+" & Coil #"+String(prioritizedCoils[0]->getOrder())+" & Coil #"+String(prioritizedCoils[1]->getOrder())+" & Coil #"+String(prioritizedCoils[2]->getOrder())+" & magIndex = #"+String(magIndex)+" & pt1 = "+String(pt1)+" & pt2 = "+String(pt2));
    	if (i00 == 3) Serial.println("magForceUp = "+String(magForceUp)+" & magIndexCoilD = "+String(magIndexCoilD)+" & zEst = "+String(zEst)+" & Coil #"+String(prioritizedCoils[0]->getOrder())+" & Coil #"+String(prioritizedCoils[1]->getOrder())+" & Coil #"+String(prioritizedCoils[2]->getOrder())+" & magIndex = #"+String(magIndex)+" & pt1 = "+String(pt1)+" & pt2 = "+String(pt2));
    	if (i00 == 4) Serial.println("magForceUp = "+String(magForceUp)+" & magIndexCoilE = "+String(magIndexCoilE)+" & zEst = "+String(zEst)+" & Coil #"+String(prioritizedCoils[0]->getOrder())+" & Coil #"+String(prioritizedCoils[1]->getOrder())+" & Coil #"+String(prioritizedCoils[2]->getOrder())+" & magIndex = #"+String(magIndex)+" & pt1 = "+String(pt1)+" & pt2 = "+String(pt2));
		*/
    }
    
  }else if (ccDebug){
	ccDebug = false;

	double adjustedDRestPerLifetimeMin = dRestPerLifetimeMin;// - 0.1 > 0 ? dRestPerLifetimeMin - 0.1 : 0.00;
    coil1->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coil2->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coil3->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coil4->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coil5->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coilW->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coilX->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coilY->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);
    coilZ->setDRestPerLifetimeMin(adjustedDRestPerLifetimeMin);

  }
  return prioritizedCoils;
}

boolean CoilContainer::healthCheck(unsigned long tc){ 
  boolean goodHealth = coil1->healthCheck(tc) == C_GOOD_HEALTH && coil2->healthCheck(tc) == C_GOOD_HEALTH && coil3->healthCheck(tc) == C_GOOD_HEALTH && coil4->healthCheck(tc) == C_GOOD_HEALTH && coil5->healthCheck(tc) == C_GOOD_HEALTH; 
  return goodHealth;
}

void CoilContainer::allPinsLow(unsigned long tc){  
  coil1->allPinsLow(tc);
  coil2->allPinsLow(tc);
  coil3->allPinsLow(tc);
  coil4->allPinsLow(tc);
  coil5->allPinsLow(tc);
  coilW->allPinsLow(tc);
  coilX->allPinsLow(tc);
  coilY->allPinsLow(tc);
  coilZ->allPinsLow(tc);
}

  /**
   * LOGIC SAY: if coil is being forced to rest 230L us per pulse, and the max pw is TON_MAX,
   * then even if there is no time between pulses and no delays, the min fraction of time resting
   * is 230L/5000L - for real world that denom is much larger, so this is our MAX = 0.046. More
   * likely there will be this for 10 pulses: 1000L x 10 off. 2400 x 10 on. no resting otherwise
   * so for 10 pulses rest fraction will be 1000L/3400L = 3400L-2400L/3400L. we want MORE rest. 
   * this is MIN
   */
double CoilContainer::dRestPerLifetimeMin = coilspace::REST_PER_LIFETIME_MIN;//0.142;//0.429;//fraction of coil life spent resting - less than this is working too hard
boolean CoilContainer::ccDebug = false;

CoilContainer::CoilContainer() : AMSAESingleton("coilCon"){
  using namespace pulsecyclespace;

  unsigned long tOnMax = TON_MAX;

  /**
   * smart pulse profiles should keep this from being needed, but here just in case
   * key question: should primary pulse ever be delayed? also, are there any cases where
   * a coil can be allowed to go under dRestPerLifetimeMin??
   */
  tForcedRest = 0L;
  boolean dummyCoil = true;
  
  /** all coils have zDelta around midpoint of 13.25 mm which is distance from edge of coil
   *  there is another 2.65 mm which extends INTO the coil that is considered part of the active range
   *  same wrt to the other edge. for instance, coilA had a midpoint 15.9 mm from zEst = 0.
   *  from there the edge of the coil is 13.25 mm and then the active region extends another 2.65 mm in
   *  so ... edge of coil A is 15.9 mm + 13.25 mm = 29.15 = 1.15 inches =~ 1 1/8"
   *  first coil (coil1 which is same as coil A) h
   *
   *  only need lower midpoint. upper is fixed relative to lower */
  coil1 = new Coil(); coil1->init(1, pulsePin_1, pulsePin_2, tOnMax, dRestPerLifetimeMin, tForcedRest, 15.9, 0.5, !dummyCoil);//A::15.9-13.25 = 2.65::
  coil2 = new Coil(); coil2->init(2, pulsePin_3, pulsePin_4, tOnMax, dRestPerLifetimeMin, tForcedRest, 64.65, 0.5, !dummyCoil);//B::64.65-62 = 2.65
  coil3 = new Coil(); coil3->init(3, pulsePin_5, pulsePin_6, tOnMax, dRestPerLifetimeMin, tForcedRest, 104.15, 0.5, !dummyCoil);//C::104.15-101.5 = 2.65
  coil4 = new Coil(); coil4->init(4, pulsePin_7, pulsePin_8, tOnMax, dRestPerLifetimeMin, tForcedRest, 26.15, 1.0, !dummyCoil);//D::26.15-23.5 = 2.65
  coil5 = new Coil(); coil5->init(5, pulsePin_9, pulsePin_0, tOnMax, dRestPerLifetimeMin, tForcedRest, 113.15, 1.0, !dummyCoil);//E::113.15-110.5 = 2.65

  coilW = new Coil(); coilW->init(101, pulsePin_Z1, pulsePin_Z2, 0L, 0.0, 0.0, 113.15, 1.0, dummyCoil);//113.15-110.5 = 2.65
  coilX = new Coil(); coilX->init(102, pulsePin_Z1, pulsePin_Z2, 0L, 0.0, 0.0, 113.15, 1.0, dummyCoil);//113.15-110.5 = 2.65
  coilY = new Coil(); coilY->init(103, pulsePin_Z1, pulsePin_Z2, 0L, 0.0, 0.0, 113.15, 1.0, dummyCoil);//113.15-110.5 = 2.65
  coilZ = new Coil(); coilZ->init(104, pulsePin_Z1, pulsePin_Z2, 0L, 0.0, 0.0, 101.22, 1.0, dummyCoil);//113.15-110.5 = 2.65

  coilList[0] = coil1;
  coilList[1] = coil2;
  coilList[2] = coil3;
  coilList[3] = coil4;
  coilList[4] = coil5;

  sMan = sMan->getInstance();
}

CoilContainer* CoilContainer::instance = 0;
CoilContainer* CoilContainer::getInstance(){
	if (!instance) {
		instance = new CoilContainer();
	}
	return instance;
}




