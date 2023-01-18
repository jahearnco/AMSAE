10/30/21
v1.046

Granada Inn test complete - results are fine with these takeaways ...
1. the spring is too strong for this test. if I increase voltage I'll rip the nylon up so until i fit with Ti rod we need to go weak
2. overall there were no surprises, although I expected to feel more resistance when moving the coil, HOWEVER!!

the shock isn't intended to do much UNLESS there is fast oscillation. need to figure out how to reproduce that.

ALSO for next version .. can hook up to the oscPro simulator - will the result be that the coil moves OPPOSITE
to the virtual motion?? hmmmm do it!

added normalizingFactor per coil to adjust for weaker curvature of larger D&E fields at magnet

unsigned long Pulse::init(uint8_t priority, unsigned long ts, Coil* c, uint8_t type){}
normalizingFactor = coil->getNormalizingFactor();




10/29/21
v1.045

Added debugging in cCon.cpp to ensure pulses are oriented correctly

Next up: Coils D & E are surprisingly weak, likely because field curvature at magnet is much smaller relative to A,B,C

solution for now is to shorten pulse widths of A,B,C



09/??/21
v1.044

NEW LAPTOP YAY!!

07/16/21
v1.043

**IMPORTANT**
**IMPORTANT**
also commented out sensor reader so this is oscillator only

boolean Brain::containersAndManagersAreInitialized(){
...
//kpthCon->postInitializer();
...
}
**IMPORTANT**
**IMPORTANT**

maybe last checkin prior to testing

change to make pulse width customizeable

added pulsecyclespace::minPW = 200L;

see unsigned long Pulse::init(uint8_t priority, unsigned long ts, Coil* c, uint8_t type){}

07/08/21
v1.042

still no idea why things slowed down before. good now, but something is slowing down at some
point. look at these values maybe?? if you see it again maybe print out how many elements are
kpthList??

  const uint16_t KPTHISTORY_LIST_SIZE_MAX = 128;//16 is lowest value within reason but it's not safe. multiply by something large so that the value ALMOST pushes the limits of memory. it's a problem even when optimized. need 10x memory to be worry free.
  const uint16_t KPTHISTORY_LIST_SIZE_MIN = 32;

unsigned long Brain::POWER_MAX_HIGH = 3000L;
unsigned long Brain::POWER_MAX_MID = 250L;
unsigned long Brain::POWER_RELATIVE_MID = 20L;
unsigned long Brain::POWER_THRESHOLD_MAGREVERSAL = 3200L;

/*
unsigned long Brain::POWER_RELATIVE_MID = 20L;
unsigned long Brain::POWER_MAX_HIGH = 2000L;
unsigned long Brain::POWER_MAX_MID = 250L;
unsigned long Brain::POWER_THRESHOLD_MAGREVERSAL = 70L;
*/

07/06/21
v1.041X

hmmm ... all was going VERY well when all of a sudden average loop times went from sub 40us to above 56us. 

must be forgetting a change I made, so time to revert after saving this.

07/06/21
v1.041

more optimization ...

unsigned long Brain::LMAN_IDLING_ITERS = 8L;
long Brain::T_READ_MIN = 110L;
uint8_t Brain::CONFIDENCE_THRESHOLD_MID = 57;


for maybeReadingComplete now looking at only first and last samples not first two and last two

boolean Brain::completeSensorReading(unsigned long tSamplingElapsed, vector<uint32_t> &adc1_readings, vector<uint32_t> &adc2_readings, double &adc_readingAve, uint8_t &adcSampleCount){

      double firstReading = adc2_readings.front();
      double lastReading = adc2_readings.size() > adc1_readings.size() ? adc2_readings.back() : adc1_readings.back();
      double devFirstReading = abs(adc_readingAve - firstReading);
      double devLastReading = abs(adc_readingAve - lastReading);

      if (abs(devFirstReading - devLastReading) > adcspace::peakDev){//weak condition
        /**
         * analyze
         */
        if (devLastReading > MAX_VPEAK_DEV){//strong condition
          /** keep the clock running and take more readings */
          readingComplete = false;
          caughtAFish++;
        }
      }
    }
  }
  if (readingComplete) {
    letOneGo++;
  }

  if (letOneGo > 10000){
    //Serial.println("letOneGo = "+String(letOneGo)+" & caughtAFish = "+String(caughtAFish));
    caughtAFish = 0;
    letOneGo = 0;
  }
  return readingComplete;
}

boolean Brain::containersAndManagersAreInitialized(){
  if (amsaeInitialized){
    Serial.println("@ Brain checking for serial manager updates ...");
    SerialManager* sMan = sMan->getInstance();
    Brain::T_READ_MIN = sMan->getTReadMin();
    Serial.println("@ Brain::LMAN_IDLING_ITERS = "+String(Brain::LMAN_IDLING_ITERS));
    Serial.println("@ Brain::T_READ_MIN = "+String(Brain::T_READ_MIN));
    Serial.println("@ Brain::CONFIDENCE_THRESHOLD_MID = "+String(Brain::CONFIDENCE_THRESHOLD_MID));

    Serial.println("@ Brain::app_main AMSAE initialization complete");
      Serial.println("");
  }
  return amsaeInitialized;


07/05/21
v1.040

interesting bit of optimization!!

by altering Brain::T_READ_MIN loop is changed as expected, however average time per reading cycle has a sweet spot
again, remember there is new logic for when to take samples. since T_MIN_READ is intentionally set to a higher value now
there will be skipped readings to give the device a break and speed up loop. somewhere in there is an average that 
makes everyone happy:-)

for now settling on long Brain::T_READ_MIN = 80L;

PositionSensor::readSensor() ...

    if (adc_readingAve < 5 || adc_readingAve > 3150){
      printf("ADC2 consider this to be the forbidden noisy region");

    }else if (adc_readingAve > lastAdc_readingAve+peakDev || adc_readingAve < lastAdc_readingAve-peakDev){
      zVal = rangeCoef*(adc_readingAve - readMin) + rangeMin;

      /*
        tPSCalcTotal += tSamplingElapsed;

        if (++psCalcCount == 2000) {
          double tAvePerCalc = tPSCalcTotal / (double)psCalcCount;
          Serial.println("tAvePerCalc = "+String(tAvePerCalc,5));
          psCalcCount = 0;
          tPSCalcTotal = 0L;
        }
      */
      
@ pcMan::processCycleQueue mins = 0.0420 & aveTPerIter = 49.32
@ pcMan::processCycleQueue mins = 0.0822 & aveTPerIter = 48.14
@ pcMan::processCycleQueue mins = 0.1227 & aveTPerIter = 48.63
@ pcMan::processCycleQueue mins = 0.1623 & aveTPerIter = 47.56
@ pcMan::processCycleQueue mins = 0.2037 & aveTPerIter = 49.67
@ pcMan::processCycleQueue mins = 0.2478 & aveTPerIter = 52.97
@ pcMan::processCycleQueue mins = 0.2881 & aveTPerIter = 48.30
tAvePerCalc = 285.24650
@ pcMan::processCycleQueue mins = 0.3283 & aveTPerIter = 48.19
@ pcMan::processCycleQueue mins = 0.3686 & aveTPerIter = 48.44
@ pcMan::processCycleQueue mins = 0.4089 & aveTPerIter = 48.35
@ pcMan::processCycleQueue mins = 0.4485 & aveTPerIter = 47.52
@ pcMan::processCycleQueue mins = 0.4891 & aveTPerIter = 48.65
@ pcMan::processCycleQueue mins = 0.5334 & aveTPerIter = 53.23
@ pcMan::processCycleQueue mins = 0.5735 & aveTPerIter = 48.09
tAvePerCalc = 286.27350
@ pcMan::processCycleQueue mins = 0.6139 & aveTPerIter = 48.52
  
      
07/05/21
v1.039

new sensor reading logic. idea is to never tax device by taking readings one after the other by default
instead ...

rule 1: read if time elapsed since last reading > Brain::T_READ_MIN then take reading
rule 2: read if the prior reading was due to time elapsed since the previous reading > Brain::T_READ_MIN - DO NOT CALC TIME ELAPSED!!
rule 3: if no read last time the calc time elapsed - it's almost certain to be large enough

overall the idea is that instead of readings one after the other slowing down the device like this : 70us ... 70us ... 70us ... 70us ...

we skip a reading or two to speed up loop so readings look more like this : 45us ... 85us ... 45us ... skip ... 85us where the skip is 45us or less



boolean KineticProfileTickHistoryContainer::processSensorData(boolean isPulseCycleNew){
  using namespace pulsecyclespace;
  
  if (isPulseCycleNew) {
    pcMan->setIsPulseCycleNew(false);
    numCyclesSinceLastReset++;
  }

  boolean thisReadIsATimeRead = false;
  tSensorTick = micros();

  if (lastReadWasATimeRead){
    thisReadIsATimeRead = false;
    doRead = true;

  }else{
    thisReadIsATimeRead = tSensorTick - lastTRead > Brain::T_READ_MIN;
    doRead = thisReadIsATimeRead;
  }

  if (doRead){
  lastReadWasATimeRead = thisReadIsATimeRead;
  lastTRead = tSensorTick;
  updateZReading();
    /**
     * ARE YOU SURE ABOUT THIS??
     * MAKE SURE! what is the tDiff between tSensorTick and tCurrent here? FIND OUT!!
     * The reason it's a question: what is the correct time of the sensor reading? that's the question. is it tSensorTick or tCurrent or maybe their average?
     */
    tCurrent = tSensorTick + 35L;//35 is the approx time it takes per updateZReading(); - better than calling micros() again
    //tCurrent = micros();
    /**
     * ARE YOU SURE ABOUT THIS??
     * MAKE SURE! what is the tDiff between tSensorTick and tCurrent here? FIND OUT!!
     */
  }else{
    tCurrent = tSensorTick;
  }


07/05/21
v1.038X - trying out kpp as static vs non-static for performance optimization

left it static

07/05/21
v1.037 - opt2003 optimized

quite a bit went into this one. after this really need to consider ways to speed up loops

for this change ...

1. since response time is longer than i realized, i'm cutting down in min samples and min read time (4 and 250 us respectively)
2. because of #1, max sampling time is 1000us with cutoff coming at 650 us realistically. 650 ~ 5 x 130
3. readSensor logic was wrong part #1: added newSamplingCycle set to true ONLY after readingComplete true. also, startCycleTick
is always = tick upong new sampling cycle (i.e. no carry over from last cycle trying to guess at loop time elapsed since that is
NOT happening during new cycle!)

PositionSensor::readSensor()..

  double adc_readingAve = 0;

  if (newSamplingCycle){
    startCycleTick = tick;
    newSamplingCycle = false;
  }

4. tempting to try and set readingComplete OUTSIDE of Brain::completeSensorReading, but disaster results. there's a reason that 
adc_readingAve is calculated only once when readingComplete is true. 

  unsigned long tSamplingElapsed = tick - startCycleTick;
  boolean readingComplete = Brain::completeSensorReading(tSamplingElapsed, adc1_readings, adc2_readings, adc_readingAve, adcSampleCount);
  
5. latest reading complete logic. memorize this!!

  if (adcSampleCount < MIN_SAMPLE_SIZE){
    //most likely case - do nothing - comment out print
    //if (tSamplingElapsed > 1000) Serial.println("BAD NEWS 1: IF THIS HAPPENS MUCH AFTER INIT - UNLESS IT'S DUE TO PRINTING DELAYS");
    readingComplete = tSamplingElapsed > 1000L;

  }else if (adcSampleCount < MAX_SAMPLE_SIZE){
    //next most likely cases
    if (tSamplingElapsed > TMIN_ZREAD_CHANGE){
      if (tSamplingElapsed < 650){
        maybeReadingComplete = true;
      }else{
        readingComplete = true;
      }
    }else{
      //keep sampling even if adcSampleCount == MIN_SAMPLE_SIZE is true
    }
  }else if (adcSampleCount >= MAX_SAMPLE_SIZE){
    readingComplete = true;
  }

6. new voltage divider and linear region here ...
AMSAEUtils adcspace::
  /**
   * taking cue that adc linear range is between 150 and 2450 for ADC_ATTEN_DB_11...
   * 1. voltage divider values chosen for ~ 10V max output from sensor and highest usable value ~ 2.7V
   * 2. distance measurements were taken within stated linear range: @53mm ~ 200 & @134mm ~ 2443 out of 4095
   * 3. this does not represent usable range but rather most linear range. usable range is ~ 150 - 250
   * 4. meanwhile there's a LOT of room for adjustments of ... Brain::MIN_SAMPLE_SIZE,T_READ_MIN,TMIN_ZREAD_CHANGE
   * 5. note: Brain::MAX_VPEAK_DEV is adjustable and set to 1.5*peakDev ~ 15 for now
   *
   * voltage divider splitting sensor output which maxes ~ 10V ... 5.1k, 0.99k, 0.47k, 0.47k
   * read at 5.1k drop
   */
   

07/03/21
v1.036 - 660us response time OPT2003 

OK. This one's a big deal since it's the first time I've understood the significance of this number 660us

it means i need to lower my sampling times!!  this is experimental but results are great. circuit is more
noisy but also more reponsive as I've swapped out the two 22 nf caps for two 6 nf caps. this means there will
be more fluctuation making its way to pcMan. that's fine. better to test it with. noise happens - don't just 
ignore it -figure out how to deal with it! see app modifiable changes below ...

these are the present values and all should be modifiable on app.

uint8_t Brain::MIN_SAMPLE_SIZE = 4;//Brain::MIN_SAMPLE_SIZE x Brain::T_READ_MIN should be just less than Brain::TMIN_ZREAD_CHANGE
uint8_t Brain::MAX_SAMPLE_SIZE = 12;////Brain::MAX_SAMPLE_SIZE x Brain::T_READ_MIN should be just less than 1000 us

long Brain::T_READ_MIN = 35L;//approx 1/2 average loop time

/**
 *  ~= T_READ_MIN * min sample count
 *  this value is more critical than I knew before RTFM ...
 *  OPT2003 samples every 750 us HOWEVER it's response time is 660 us
 *  that means if the device response time is at best 660 us + Brain::TMIN_ZREAD_CHANGE
 *  this is fine for light to moderate use, but for taxing cycles where loop takes long
 *  MIN_SAMPLE_SIZE may be too high?
 */
long Brain::TMIN_ZREAD_CHANGE = 250L;
double Brain::ZREAD_DIFF_MIN = 1.5;



07/03/21
v1.035


stupid prior code was setting tick an then completely bypassing readings for an entire loop!

FIX: 

  double zVal = zReadingMaybe;
  unsigned long tDel = tick - lastTick;

  if (tDel > 1200L){
    lastTick = tick;
    tDel = tick - lastTick;
    tCycle += tDel;
  }else{
    tCycle = tDel;
  }



07/03/21
v1.034-NOT-A-RELASE-DEBUG-CHANGE-ONLY

test to prove that sensor response it as advertized ~ 700 us

weird that the high-rez mode is just as fast. not sure why!


07/03/21
v1.033-NOT-A-RELASE-DEBUG-CHANGE-ONLY

this isn't really a release because it has too many debug statements added

there was a concern related to coil order debugging. simple fix which isn't a fix but a debug change. 
change from this ...
  int crumbs = 1;//tc%50000;
  if (sMan->debugCoilCon() && crumbs < 1000) {
    Serial.println("Coil #"+String(prioritizedCoils[0]->getOrder())+" & Coil #"+String(prioritizedCoils[1]->getOrder())+" & Coil #"+String(prioritizedCoils[2]->getOrder()));
  }
    
to this

  int crumbs = tc%50000;
  if (sMan->debugCoilCon() && crumbs < 1000) {
    Serial.println("Coil #"+String(prioritizedCoils[0]->getOrder())+" & Coil #"+String(prioritizedCoils[1]->getOrder())+" & Coil #"+String(prioritizedCoils[2]->getOrder()));
  }

07/02/21
v1.033

very happy with this release!!

I moved some sensor logic to Brian and tested thoroughly for noise and boundary conditions.
things got a little tricky trying to sample more after maybeReadingComplete. samples were 
growing unbounded and sample times got way over 1200L.

boolean Brain::completeSensorReading(unsigned long tDel, vector<uint32_t> &adc1_readings, vector<uint32_t> &adc2_readings, double &adc_readingAve, uint8_t &adcSampleCount)


the biggest change is here ...

  if (adcSampleCount < MIN_SAMPLE_SIZE){
    //most likely case - do nothing - comment out print
    //if (tDel > 1000) Serial.println("BAD NEWS 1: IF THIS HAPPENS MUCH AFTER INIT - UNLESS IT'S DUE TO PRINTING DELAYS");

  }else if (adcSampleCount < MAX_SAMPLE_SIZE){
    //next most likely cases
    if (tDel > TMIN_ZREAD_CHANGE){
      if (tDel < 1000){
        maybeReadingComplete = true;
      }else{
        readingComplete = true;
      }
    }else{
      //keep sampling even if adcSampleCount == MIN_SAMPLE_SIZE is true
    }
  }else if (adcSampleCount == MAX_SAMPLE_SIZE){
    readingComplete = true;
  }
  
07/01/21
v1.032

got rid of the appendix clause ...

i like where this is at now, tho maybe a bit more testing ...

double PositionSensor::readSensor(unsigned long tick, double zReadingMaybe, double zReading){
  using namespace adcspace;
  /**
  * active range for OPT2003 is 142 mm to 242 mm = 100 mm
  * largest granularity is 100 mm /0.75 mm = 133 = no. significant data points
  * assume linear range between 150 and 2450 or a range of 2300
  * 2300/133 ~= 17 so +/- 8.5 on either side so ... Brain::peakDev = 0.5*adcVInputRange/(opt2003Range/zDelAbsMin)
  * this is probably too small but we try it
  *
  * sampling over a 1200 ms range. 1200 is a max
  * that is rarely approached but allowed. it's
  * approx twice the average light-duty
  * range.
  */
  double zVal = zReadingMaybe;
  double adc_readingAve = 0;
  boolean readingComplete = false;
  boolean maybeReadingComplete = false;

  unsigned long tDel = tick - lastTick;
  if (tDel > 1200L){
    lastTick = tick;

  }else{

    if (adcSampleCount%2 == 1){
      adc1_readings.push_back(adc1_get_raw((adc1_channel_t)adcChannel1));

    }else{
      int adc2_raw;
      esp_err_t r = adc2_get_raw((adc2_channel_t)adcChannel2, adcWidth, &adc2_raw);
      if ( r == ESP_OK ) {
      adc2_readings.push_back((uint32_t)adc2_raw);

      } else if ( r == ESP_ERR_TIMEOUT ) {
        printf("ADC2 used by Wi-Fi.\n");
      }
    }

    adcSampleCount  = adc1_readings.size() + adc2_readings.size();

    /** DBG */
    if (!tPingd && adc1_readings.size() == 0 && tick%20000 < 10000){
      digitalWrite(adcspace::testInputPin1, HIGH);
      tPing = tick;
      tPingd = true;
    }
    /** DBG */

    if (adcSampleCount > Brain::MIN_SAMPLE_SIZE){

      if (tDel > Brain::TMIN_ZREAD_CHANGE){
        maybeReadingComplete = true;

      }else if (adcSampleCount == Brain::MAX_SAMPLE_SIZE){
        //shouldn't happen often
        readingComplete = true;
      }

    }else if (adcSampleCount == Brain::MIN_SAMPLE_SIZE && tDel > 1000L ){
      //shouldn't happen often
      readingComplete = true;

    }

    if (readingComplete || maybeReadingComplete){
      /** since readingComplete we can be certain condition Brain::MIN_SAMPLE_SIZE is met. */
      double adc_readingsTot = 0;
      for (vector<uint32_t>::iterator adc1It = adc1_readings.begin(); adc1It != adc1_readings.end(); ++adc1It){
        adc_readingsTot += *adc1It;
      }
      for (vector<uint32_t>::iterator adc2It = adc2_readings.begin(); adc2It != adc2_readings.end(); ++adc2It){
        adc_readingsTot += *adc2It;
      }
      adc_readingAve = (double)adc_readingsTot/adcSampleCount;

      if (maybeReadingComplete){
        readingComplete = true;//the most likely case

        /**
         * before assigning zVal first inspect both initial and trailing edge of readings for consistency
         * if deviating significantly from adc_readingAve then later readings are likely to be more accurate
         * do we keep on taking readings? maybe!
         *
         * look at first 2 and last 2 and go from there ...
         */
        double devFirstTwoReadings = abs(adc_readingAve - 0.5*(adc1_readings.at(0)+adc2_readings.at(0)));
        double devLastTwoReadings = abs(adc_readingAve - 0.5*(adc1_readings.at(adc1_readings.size()-1)+adc2_readings.at(adc2_readings.size()-1)));

        if (abs(devFirstTwoReadings - devLastTwoReadings) > peakDev){//weak condition
          /**
           * analyze
           */
          if (devLastTwoReadings > Brain::MAX_VPEAK_DEV){//strong condition
            /** keep the clock running and take more readings */
            readingComplete = false;
          }
        }
      }

      //if (Brain::isReadingComplete(adc1_readings, adc2_readings)){
      if (readingComplete){

        if (adc_readingAve < readMin || adc_readingAve > readMax){
          printf("ADC2 consider this to be the forbidden noisy region");

        }else if (adc_readingAve > lastAdc_readingAve+peakDev || adc_readingAve < lastAdc_readingAve-peakDev){
          zVal = rangeCoef*(adc_readingAve - 150) + rangeMin;//0.026 = (2450-150)/100 - conversion to mm range is 142 - 242 = 100 mm
          /**
          * BEGIN DEBUGGING - comment out if slowing things down
          *
          * good place to remind what the goal is and what I'm up to here ...
          *
          * 1. noise is bad enough that I'm swapping out caps to see where i get bang for my buck - currently looks like 0.022 uf - 0.033 uf might be ideal
          * 2. I haven't decided on granularity yet. 1.5 mm or possibly 1.7 mm is max.
          *    i'm considering letting things stay noisy and allow it to make it's way through AMSAE which shouldn't have an issue with it
          *    because confidence will be low and kpp eval will  deal
          * 3. if noisy at zDelAbsMin that's fine as long as much less noisy at zDelAbsMid and rarely noisy at zDelAbsMax.
          *    the noise is predictable and falls off rapidly and reliably with voltage
          * 4. peakDev is a critical value and together with values in #2 need to arrive at a sensible value for this which depends on opt2003Range
          * 5. 2450 above might be too safe.
          *    chose to be in linear range which is good. that's ok for now.
          *    i'd rather all these values are on the extra-safe side for the following checkins
          * 6. attenuation setting is worth changing and testing - not exactly sure why but figure it out. might work better with OPT2003
          * 6a. for instance, ideally would like 150 values between 75 and 925. peakDev = 3
          */

          /** DBG */
          double zDelAbs = abs(zVal - zReading);
          double zDelTest = 6.00;//zDelAbsMax;//3.0;//zDelAbsMin;//1.15;
          double tDel2 = tick-tPing;

          if (tPingd && zDelAbs >= zDelTest){
          Serial.println("(tick-tPing) = "+String(tDel2)+" & sampleCount = "+String(adcSampleCount)+" & zDelAbs = "+String(zDelAbs)+" & zVal = "+String(zVal));
          }
          /** DBG */
        }
        adcSampleCount = 0;

        adc1_readings.clear();
        adc2_readings.clear();

        lastAdc_readingAve = adc_readingAve;
        lastTick = tick;
        tPingd = false;

        digitalWrite(adcspace::testInputPin1, LOW);
      }
    }
  }

  return zVal;
}



07/01/21
v1.031

 a very small bit of optimization. should probably be in Brain
 
      if (maybeNotReadingComplete){
        readingComplete = true;//the most likely case

        /**
         * before assigning zVal first inspect both initial and trailing edge of readings for consistency
         * if deviating significantly from adc_readingAve then later readings are likely to be more accurate
         * do we keep on taking readings? maybe!
         *
         * look at first 2 and last 2 and go from there ...
         */
        double devFirstTwoReadings = abs(adc_readingAve - 0.5*(adc1_readings.at(0)+adc2_readings.at(0)));
        double devLastTwoReadings = abs(adc_readingAve - 0.5*(adc1_readings.at(adc1_readings.size()-1)+adc2_readings.at(adc2_readings.size()-1)));

        if (abs(devFirstTwoReadings - devLastTwoReadings) > peakDev){//weak condition
          /**
           * analyze
           */
          if (devLastTwoReadings > Brain::MAX_VPEAK_DEV){//strong condition
            /** keep the clock running and take more readings */
            readingComplete = false;
          }
        }
      }


07/01/21
v1.030

readings for each channel are stored in vectors

      adc1_readings.clear();
      adc2_readings.clear();
      
07/01/21
v1.029

function pointer. created a postInitializer() for sensor reading

best would be to create AMSAESingleton::postInitializer virtual function

void KineticProfileTickHistoryContainer::postInitializer() {
  readPositionSensor = PositionSensor::readSensor;//PositionSensor::readSensor(tSensorTick, zReadingMaybe, zReading, initialized);
}


07/01/21
v1.028

excellent performance with testing - GPIO14 pulsing under 1khz achieves expected results with two 0.047 uf caps to ground from adc channels 1 & 2

there are NO issues with these caps slowing things down. the values appear to be exactly what's needed to quash noise and maintain accuracy. huge success!!

note: I need to accompany these latest checkins with schematics or at least pics

one thing I haven't tried it removing the external power supply. doing that now before checkin. hold on ...

it appears to work fine!! i'm attaching pics and checking in to this version directory


06/29/21
v1.027

  if (zValAbs >= zValAbsMax){
        Serial.println("saving before i mess this up. look for digital write above rangeMin = "+String(rangeMin)+" & tDel1 = "+String(tDel1)+" & zValAbs = "+String(zValAbs)+" & zVal = "+String(zVal));
      }
      
      
06/29/21
v1.026 

a rough day but a good checkin. found an issue with the code in previous checkin which made it seem like noise was "fixed" by the cap.

instead the coef 0.0125 was just wrong and hid issues.

this checkin is good stuff which results from all that

more testing to do. seems pretty consistent.

i'd like to make sure reponse is what it appears to be (great!) rather than discharging caps that I'm noticing. i suppose you could test with digital
pins which allow you to see reponse times?

      /**
       * BEGIN DEBUGGING - comment out if slowing things down
       *
       * good place to remind what the goal is and what I'm up to here ...
       *
       * 1. noise is bad enough that I'm swapping out caps to see where i get bang for my buck - currently looks like 0.022 uf - 0.033 uf might be ideal
       * 2. I haven't decided on granularity yet. 1.5 mm or possibly 1.7 mm is max. i'm considering letting things stay noisy and allow it to make it's way through AMSAE which shouldn't have an issue with it because confidence will be low.
       * 3. if noisy at zValAbsMin that's fine at long as much less noisy at zValAbsMid and barely noisy at zValAbsMax. the noise is predictable and falls off rapidly with voltage
       * 4. rangeWidth is a critical value and together with values in #2 need to arrive at a sensible value for this which depends on readingRange
       * 5. 2450 above might be too safe. that's ok for now. i'd rather all these values are on the extra-safe side for the following checkins
       * 6. attenuation setting is worth changing and testing - not exactly sure why but figure it out. might work better with OPT2003
       * 6a. for instance, ideally would like 150 values between 75 and 925. rangeWidth = 3
       */
      double zValAbs = abs(zVal - zReading);
      double zValAbsMin = 0.75;
      double zValAbsMid = 1.0;
      double zValAbsMax = 1.5;
      if (zValAbs >= zValAbsMid){
        Serial.println("rangeMin = "+String(rangeMin)+" & tDel1 = "+String(tDel1)+" & zValAbs = "+String(zValAbs)+" & zVal = "+String(zVal));
      }
      /**
       * END DEBUGGING
       */
       
       
06/29/21
v1.025 - sensorRelease - PositionSensor class

checkin with new PositionSensor class created - still some rewriting to do.

in particular, want to create and then inspect array of measurements to detect
consistent variations. for instance, maybe last two readings are very different 
from first 4. in that case the last two are likely to be representative of current
position. decide to average differently or take more readings.

06/28/21
v1.024 - sensorRelease slightly experimental - consider revert


rolling over previous cycle that didn't have enough samples to 
be valid. so retain valuable info. good news.

so far tests have been perfect. looks good. this is the change ...

      }else if (adcSampleCount == maxSampleCount){
        //interesting case of rollover from previous cycle which
        //wasn't completed - took too long for enough samples -
        //so following through here ...
        readingComplete = true;
      }
    }

    if (readingComplete){

      //double adc_readingAve1 = (double)adc_reading1/(double)adcSampleCount1;
      //double adc_readingAve2 = (double)adc_reading2/(double)adcSampleCount2;

      double adc_readingAve = (double)(adc_reading1 + adc_reading2)/adcSampleCount;

      if (adc_readingAve < 150 || adc_readingAve > 2450){
      printf("ADC2 consider this to be the forbidden noisy region");
      }else if (adc_readingAve > lastAdc_readingAve+15 || adc_readingAve < lastAdc_readingAve-15){
      zVal = 0.0125*(adc_readingAve - 150) + 162.0;
      double zValAbs = abs(zVal - zReadingMaybe);
      if (tDel1 < Brain::TMIN_ZREAD_CHANGE+220 && zValAbs > 0.7){
        Serial.println("tDel1 < 1000 tDel1 = "+String(tDel1)+" & zValAbs = "+String(zValAbs)+" & adc_readingAve = "+String(adc_readingAve));
      }
      }

      adcSampleCount1 = 0;
      adc_reading1 = 0;
      adcSampleCount2 = 0;
      adc_reading2 = 0;
      lastAdc_readingAve = adc_readingAve;
      lastTick1 = tick;
    }

    }else{
      /**
       * wait a few seconds before announcing problems*/
      if(!sMan->debugAny() && tick > 2000000L) {
        //Serial.println("tDel1 >= 1000L not sampling fast enough? tDel1 = "+String(tDel1)+" & adcSampleCount1 = "+String(adcSampleCount1)+" &  adcSampleCount2 = "+String(adcSampleCount2));
      }
      /**
       * this clause is NOT inefficient - we should almost never be here
       * getting here means device is working so hard that each loop takes a
       * lot of time on average. this would be the case if numerous surge interrupts
       * occurred overriding one after the other. this shouldn't happen but it's not certain
       *
       * that's the nice way of stating that something else is probably going on if we got
       * here. if we didn't hit minSampleCount over a 1200L period that means on average
       * each loop was 1200/minSampleCount which is larger than 170 us per loop if minSampleCount
       * is 7. so far that something else is 99.9% related to print statements as expected.
       * I'm not sure the other 0.1% case still occurs though - needs to be tested
       *
       *
       * since we're rarely here this 'skip' is an insignificant imperfection
       * (I won't call it a limitation) of the device at present.
       * easy to fix later so there are never skips which result in 1.2 ms of lost information
       * maybe it shouldn't be lost and lastTick1 should simply be reset?
       *
       * maybe!!
       *
       * really not sure what to do here actually. a decision needs to be made!
       */
      /** comment out for now
       * the values roll over to next cycle where tDel1 starts at 0
       * however, look above for this clause
       * }else if (adcSampleCount == maxSampleCount){
       * not time dependent!
       * this is good device for optimization - the need both demands and defines maxSampleCount and
       * maxSampleCount defines the behavior and its precision
       *
      adcSampleCount1 = 0;
      adc_reading1 = 0;
      adcSampleCount2 = 0;
      adc_reading2 = 0;
      */

      lastTick1 = tick;
    }





06/28/21
v1.023 - sensorRelease

huge release I'll call the readSensor release

too many changes to mention. lots of trial and error and lotsa of success.

biggest change occurred when attempting to silence noise with a 0.047 uf cap to ground from adc inputs after using a 0.009 uf cap which appeared to do nothing
the 0.047 uf cap killed anything high frequency however. i tried halving it to 0.0235 and SUCCESS! probably works 10 times better than i was hoping for

this is a real success moment for this device!!

the new code is included here. look at bottom for comment "this code is NOT inefficent ... "

there's a good idea there. do it.

/*
  KineticProfileTickHistoryContainer.cpp - Library for KineticProfileTickHistoryContainer props and methods
*/
#include <KineticProfileTickHistoryContainer.h>

 /**
  unsigned long tBeginCalc = micros();
  unsigned long tDeltaCalc = micros() - tBeginCalc;
  if (tDeltaCalc > 40) Serial.println("tDeltaCalc > 10 tDeltaCalc = "+String(tDeltaCalc));

  tKCalcTotal += tDeltaCalc;

  if (++kCalcCount == 5000) {
    double tAvePerCalc = tKCalcTotal / (double)kCalcCount;
    Serial.println("tAvePerCalc = "+String(tAvePerCalc,5));
    kCalcCount = 0;
    tKCalcTotal = 0L;
  }
  */

KineticPulseProfile* KineticProfileTickHistoryContainer::kpp = new KineticPulseProfile();

double KineticProfileTickHistoryContainer::readSensor(unsigned long tick){
  using namespace adcspace;

  /**
   * largest granularity is 40 values 1.5 mm apart
   * assume linear range between 100 and 3700 or a range of 3600
   * 3600/40 = 90 so +/- 45 on either side
   *
   * In any case since the circuit seems noisier for lower voltage
   * readings I'm restricting the scale to a range from 1800 - 3600
   *
   * without the noise i can dial in the values of adcSampleCount and
   * region around the mean where uncertainty is acceptable
   */
  unsigned long tDel1 = tick - lastTick1;
  unsigned long tDel2 = tick - lastTick2;
  double zVal = zReadingMaybe;
  boolean readingComplete = false;
  uint8_t minSampleCount = 7;
  uint8_t maxSampleCount = 10;

  /**
   * sampling over a 1200 ms range. 1200 is a max
   * that is rarely approached but allowed. it's
   * approx twice the average light-duty
   */
  if (initialized){

    if (tDel1 < 1200L){
      uint8_t adcSampleCount = adcSampleCount1 + adcSampleCount2;

    if (adcSampleCount%2 == 1){
      adc_reading1 += adc1_get_raw((adc1_channel_t)adcChannel1);
      adcSampleCount1++;

    }else{
      int adc2_raw;
      esp_err_t r = adc2_get_raw((adc2_channel_t)adcChannel2, adcWidth, &adc2_raw);
      //adc2_raw = 2000;
      if ( r == ESP_OK ) {
        adc_reading2 += (uint32_t)adc2_raw;
        adcSampleCount2++;
      } else if ( r == ESP_ERR_TIMEOUT ) {
        printf("ADC2 used by Wi-Fi.\n");
      }
    }

    adcSampleCount = adcSampleCount1 + adcSampleCount2;

    if (adcSampleCount >= minSampleCount){
      //800 ~ 5 x 166us - 10 readings : device is operating at faster speeds and slower loop times
      //400 ~ 5 x 80us - 10 readings : device is operating at slower speeds and faster loop times
      if (adcSampleCount == minSampleCount && tDel1 > 1000L ){
        readingComplete = true;
      }else if ( tDel1 > Brain::TMIN_ZREAD_CHANGE ){
        readingComplete = true;
      }
    }

    if (readingComplete){

      //double adc_readingAve1 = (double)adc_reading1/(double)adcSampleCount1;
      //double adc_readingAve2 = (double)adc_reading2/(double)adcSampleCount2;

      double adc_readingAve = (double)(adc_reading1 + adc_reading2)/adcSampleCount;

      if (adc_readingAve < 150 || adc_readingAve > 2450){
      printf("ADC2 consider this to be the forbidden noisy region");
      }else if (adc_readingAve > lastAdc_readingAve+15 || adc_readingAve < lastAdc_readingAve-15){
      zVal = 0.0125*(adc_readingAve - 150) + 162.0;
      double zValAbs = abs(zVal - zReadingMaybe);
      if (tDel1 < Brain::TMIN_ZREAD_CHANGE+220 && zValAbs > 0.7){
        Serial.println("tDel1 < 1000 tDel1 = "+String(tDel1)+" & zValAbs = "+String(zValAbs)+" & adc_readingAve = "+String(adc_readingAve));
      }
      }

      adcSampleCount1 = 0;
      adc_reading1 = 0;
      adcSampleCount2 = 0;
      adc_reading2 = 0;
      lastAdc_readingAve = adc_readingAve;
      lastTick1 = tick;
    }

    }else{
      /**
       * wait a few seconds before announcing problems*/
      if(!sMan->debugAny() && tick > 2000000L) {
        Serial.println("tDel1 >= 1000L not sampling fast enough? tDel1 = "+String(tDel1)+" & adcSampleCount1 = "+String(adcSampleCount1)+" &  adcSampleCount2 = "+String(adcSampleCount2));
      }
      /**
       * this clause is NOT inefficient - we should almost never be here
       * getting here means device is working so hard that each loop takes a
       * lot of time on average. this would be the case if numerous surge interrupts
       * occurred overriding one after the other. this shouldn't happen but it's not certain
       * since we're rarely here this 'skip' is an insignificant imperfection
       * (I won't call it a limitation) of the device at present.
       * easy to fix later so there are never skips which result in 1.2 ms of lost information
       * maybe it shouldn't be lost and lastTick1 should simply be reset?
       *
       * maybe!!
       */
      adcSampleCount1 = 0;
      adc_reading1 = 0;
      adcSampleCount2 = 0;
      adc_reading2 = 0;
      lastTick1 = tick;
    }

  }else{
    lastTick1 = tick;
  }

  if (!initialized && tDel2 > Brain::TMIN_ZREAD_CHANGE){
    zVal = Brain::readOscillatorProfile(tick, zReading, currentKpth);
    lastTick2 = tick;
  }

  return zVal;
}

void KineticProfileTickHistoryContainer::updateKpthList(){
  uint16_t vSize = (uint16_t)kpthVList.size();
  KineticProfileTickHistory* oldestKpth = &kpthVList.at(vSize-1);
  unsigned long oldestProfileTickStamp = oldestKpth->getTProfileTickStamp();
  long largestTDelta = tCurrent > oldestProfileTickStamp ? tCurrent - oldestProfileTickStamp : LONG_MAX;

    if (largestTDelta > TDEL_LONGTERM_MIN*4) {
      if (largestTDelta == LONG_MAX) {
        Serial.println("@KineticProfileTickHistoryContainer::updateKpthList largestTDelta == LONG_MAX!!!");
      }
      kpthVList.pop_back();
    }else if (vSize > kpthspace::KPTHISTORY_LIST_SIZE_MAX){
      Serial.println("not sure how but we got here. FIX IT!!");
    }

  KineticProfileTickHistory kpth;
  kpth.init(tCurrent, zReadingMaybe, kpthspace::DATA_IS_USEABLE);

  kpthVList.insert(kpthVList.begin(), kpth);
  currentKpth = &kpthVList.at(0);
  priorKpth = &kpthVList.at(1);
}

uint8_t KineticProfileTickHistoryContainer::finalizeKineticCalcs(){
  return Brain::calculateKinetics(currentKpth, priorKpth, &kpthVList);
}

void KineticProfileTickHistoryContainer::reportKineticResults(uint8_t fKinCode){
  confidence = currentKpth->getConfidence();//confidence of jerk calculation
  power = currentKpth->getPower();//magnitude of jerk
  magForceUp = currentKpth->getMagForceUp();//direction of acceleration
  interruptType = Brain::determineInterruptType(fKinCode, tCurrent, priorKpth);
  currentKpth->setInterruptType(interruptType);
}

void KineticProfileTickHistoryContainer::checkForSerialUpdates(){
  sMan->setDebugFlagChanged(false);

  while (Serial.available()){
    sMan->getSerial();
    double lastMult = Brain::minPeriodMult;
    Brain::minPeriodMult = sMan->debugOscillator()*2.5;
    Brain::hardcodeOsc = sMan->hardcodeOsc();
    Brain::printOscMssg = Brain::minPeriodMult != lastMult;
  }
}

/**
  many things can happen here. this is the one interval during normal operation where there's some time.
  that's because it's very unlikely that sensor data will change over the next several loops. so we can play around
  a bit here, clean things up, do complex calculations, look at pulsecycle history???? hey ... that's a good
  one!

   There is good value to recording these profile tickStamps a the end of the ticks' active life, but is there value to calculating
   a profile velocity? so far I don't think so, and am not sure what else should be recorded here. maybe a flag that shows the following
   kpth led to an override? not sure about that either, since that info is available by looking at history

   all i come up with is that the profile tickStamps are significant becasue they give an idea as to how close things occurred and whether the size
   of the time deltas can help to anticipate trends.
*/
void KineticProfileTickHistoryContainer::reset(){

  if (kpthConResetCount++ == 1L){
    /*** BEGIN:DO TIME CONSUMING STUFF*/
    checkForSerialUpdates();
    /** this condition doesn't happen frequently because it implies not much movement. for
     *  that reason this is a good place for doing some cleanup or debugging or extracurricular etc.*/
    if (tCurrent > 3600000000L){
      /** force restart after 60 minutes to avoid unsigned long math issues */
      Brain::handleCatastrophe("NORMAL MAINTENANCE::RESTARTING AFTER ABOUT 60 MINUTES ... 1 ... 2 ... 3 ... GO!");
    }
    /*** END:DO TIME CONSUMING STUFF*/
    kpthConResetCount = 0L;
  }

  numCyclesSinceLastReset = 0;
  Brain::MAX_PULSECYCLES_SINCE_LAST_RESET = 1;
  tickerCount = 0;
  zReadingChange = false;
  interruptKpthDefinitionCode = NO_INTERRUPT_KPTH_PROFILING;
  /** don't call micros() here - it's ok for there to be a delay in fact it's more accurate to include it since the idea is to measure tDeltas since last reading */
  tKpthTickZero = tCurrent;
  /** don't call micros() here - it's ok for there to be a delay in fact it's more accurate to include it since the idea is to measure tDeltas since last reading */
  updateKpthList();
}

void KineticProfileTickHistoryContainer::setInterruptKpthDefinitionCode(){
  /** 
   *  numCyclesSinceLastReset usually can't be larger than 2, but if idling (i.e. EMPTY_PULSING) 
   *  new cycles can come more quickly. this is why MAX_PULSECYCLES_SINCE_LAST_RESET  
   *  is a good candidate for app config or for allowing more adaptive functionality
   */
  if (numCyclesSinceLastReset > Brain::MAX_PULSECYCLES_SINCE_LAST_RESET){
    interruptKpthDefinitionCode = UNINTERRUPTED_PULSECYCLING;

  } else if (zReadingChange && tCurrent - tKpthTickZero > Brain::TMIN_ZREAD_CHANGE){
    interruptKpthDefinitionCode = ZREADING_CHANGED;

  } else if (Brain::oscPro.doSetVProfileAtTransition){
    interruptKpthDefinitionCode = SIMULATION_FORCED_INTERRUPT;

  } else {
    interruptKpthDefinitionCode = NO_INTERRUPT_KPTH_PROFILING;
  }
}

void KineticProfileTickHistoryContainer::updateZReading(){
  /**
    interesting point: the least important time to call readSensor is right after reset(); it's likely that an important
    sensor read just occurred on the previous call - it may have changed from the previous 50 - 100 readings for instance. that makes it
    very unlikely that it changes within 4 readings after that.

    try bitwise math to get faster here ...
    n % m == n & (m - 1)

    tried it. NOT faster!
  */
  lastTRead = tSensorTick;
  double sensorRead = readSensor(tSensorTick);
  double zmagDelta = 0;

  if (!zReadingChange){
    zmagDelta = abs(zReading - sensorRead);
    zReadingChange = zmagDelta > Brain::ZREAD_DIFF_MIN;
  }

  if (zReadingChange){
    zReading = sensorRead;
    zReadingMaybe = sensorRead;
  } else if (zmagDelta > Brain::ZREAD_DIFF_MIN*0.25){
    zReadingMaybe = sensorRead;
  }

  /**
     DECISION TO MAKE: if going with tSensorTick then tCurrent will be off by amount of time needed to gather data
     what is the danger of using namespace this value for pulse start time? the time will be EARLIER so .. who cares, right?

     keep in mind that new PC was already ... created, put in queue, enabled & likely run, so this is out of date
     or rather this happens one full loop AFTER the new PC was processed
  */
}

/** NOTE: PRIMARY REASON FOR MAINTAINING A FAST LOOP (ex here every 20us or so) VERSUS A SLOW LOOP (ex here every 50us):
   take a micros() reading here. it must be here since this is as close to the actual measurement time as possible, but
   if the calc takes some time then this value will be off a bit IF running a slow loop. one way to deal: don't call this
   tCurrent but rather tSensorTick or something. Then tCurrent is set to micros() just before call to tMan->initXXX
  */
boolean KineticProfileTickHistoryContainer::processSensorData(boolean isPulseCycleNew){
  using namespace pulsecyclespace;
  
  if (isPulseCycleNew) {
    generatePulseProfileData();
    pcMan->setIsPulseCycleNew(false);
    numCyclesSinceLastReset++;
  }

  tSensorTick = micros();
  
  if (tSensorTick - lastTRead > Brain::T_READ_MIN){
    updateZReading();
    /**
     * ARE YOU SURE ABOUT THIS??
     * MAKE SURE! what is the tDiff between tSensorTick and tCurrent here? FIND OUT!!
     * The reason it's a question: what is the correct time of the sensor reading? that's the question. is it tSensorTick or tCurrent or maybe their average?
     */
    //tCurrent = tSensorTick;
    tCurrent = micros();
    /**
     * ARE YOU SURE ABOUT THIS??
     * MAKE SURE! what is the tDiff between tSensorTick and tCurrent here? FIND OUT!!
     */
  }else{
    tCurrent = tSensorTick;
  }

  if (sMan->debugBrain()){

    if (isPulseCycleNew) kpthConIterCount++;

    if (Brain::REALIZED_MIN_EXTREMA_INTERRUPT) { numStaccatoInterrupts++; Brain::REALIZED_MIN_EXTREMA_INTERRUPT = false; }
    if (Brain::REALIZED_SURGE_INTERRUPT) { numSurgeInterrupts++; Brain::REALIZED_SURGE_INTERRUPT = false; }
    if (interruptType == MAX_EXTREMA_INTERRUPT) { numExtremaInterrupts++; }
    if (interruptType == RESUME_INTERRUPT) { numResumeInterrupts++; }

    if (kpthConIterCount == 2000L){
    long tDelIters = tCurrent > tStartTimingCycle ? tCurrent - tStartTimingCycle : 0L;
    double tDelAve = (1.0 * tDelIters + 0.5) / (double)kpthConIterCount;

    Serial.println("ave cycling = " + String(tDelAve) + " & numStaccatoInterrupts = " + String(numStaccatoInterrupts) + " & numSurgeInterrupts = " + String(numSurgeInterrupts) + " & numExtremaInterrupts = " + String(numExtremaInterrupts) + " & numResumeInterrupts = " + String(numResumeInterrupts));

    kpthConIterCount = 0L;

    numStaccatoInterrupts = 0;
    numSurgeInterrupts = 0;
    numExtremaInterrupts = 0;
    numResumeInterrupts = 0;

    tCurrent = micros();
    tStartTimingCycle = tCurrent;
    }
  }
  
  //if (updateTicker() && zReading < kpthspace::Z_MAX && zReading > kpthspace::Z_MIN){ //meant to be true 100% of time except for catastrophe
  if (zReading < kpthspace::Z_MAX && zReading > kpthspace::Z_MIN){ //meant to be true 100% of time except for catastrophe
    /** needs to be called before any reset calls - this is needed when finalizing currentKpth and for doing calcs */
    
    setInterruptKpthDefinitionCode();
    updateCurrentKineticProfile();
    
    if (interruptKpthDefinitionCode != NO_INTERRUPT_KPTH_PROFILING){
      /** this is most time consuming aspect of device. it takes approx 67 us per call. not terrible. not a problem as long as sensor reading isn't too time consuming*/
      uint8_t fKinCode = finalizeKineticCalcs();
      reportKineticResults(fKinCode);
      /**
         WHEN TO RESET?? reset means that all kpth move to next higher slot in queue and a new currentKpth is created and put into zeroth slot
         also means that profiles are calculated. What else. is that it? Since there's no reason to create a new kpth every time isPulseCycleNew is true,
         then don't do it! fine, but you will now have issues where tickerCount gets too large. that's fine as well, but handle it keeping in mind
         that tickerCount will be especially large for short loop idles
  
         so question is how large a tickerCount is too large? that's related to the max amount of time before a zReading changes. assume that's 20 ms or 20000 us
         A typical situation is approx 1 tickerCount per 50 us so 20000 us ~ 400
         OR if moving fast can have 1 ticker count per 8 us for ideal situations so ~ 2500
         OR if calcs and readings take long then 1 tickerCount per 200us so ~ 100
         so ... what is maxTickerCount allowed before creating new currentKpth?
      */
      reset();
    }
    return true;

  } else {
    //shut everything down
    if (zReading >= kpthspace::Z_MAX || zReading <= kpthspace::Z_MIN) Serial.println("WHOOPS! zReading = " + String(zReading) + " Shouldn't be here!! piston is out of bounds!! LEAVE THIS IN PLACE!!");
    coilCon->allPinsLow(tCurrent);
    return false;
  }
}

boolean KineticProfileTickHistoryContainer::updateTicker(){
  boolean success = true;//most likely as there are over 1000 run thrus
  tickerCount++;

  if (tickerCount == TICKER_COUNT_MAX){
    coilCon->allPinsLow(micros());
    Serial.println("@ zReading = " + String(zReading) + " NEW PCs AREN'T BEING RUN OR COMPLETED. IS LOOP TOO FAST? TICKER_COUNT_MAX TOO LOW? PULSE WIDTHS, DELAYS, TON_MAX TOO HIGH? note: this print statement could create some bad delays and even loops");
    tickerCount = 0;
    success = false;
  }
  return success;
}

KineticProfileTickHistoryContainer::KineticProfileTickHistoryContainer() : AMSAESingleton("kpthCon"){
  coilCon = coilCon->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!
  sMan = sMan->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!
  pcMan = pcMan->getInstance();
  delayMicroseconds(10);//not sure why this improves run time efficiency but it does!

  tickerCount = 0;
  kCalcCount = 0;
  tKCalcTotal = 0L;
  sensorReadIters = 0L;
  kpthConIterCount = 0L;
  kpthConResetCount = 0L;
  numStaccatoInterrupts = 0;
  numSurgeInterrupts = 0;
  numExtremaInterrupts = 0;
  numResumeInterrupts = 0;
  tStartTimingCycle = micros();
  tSensorTick = tStartTimingCycle;
  tCurrent = tStartTimingCycle;
  lastTRead = tStartTimingCycle;
  tKpthTickZero = tStartTimingCycle;
  tDebugTick = 0L;
  numCyclesSinceLastReset = 0;

  adc_reading1 = 0;
  adc_reading2 = 0;
  lastAdc_readingAve1 = 2000.0;
  lastAdc_readingAve2 = 2000.0;
  lastAdc_readingAve = 2000.0;
  adcSampleCount1 = 0;
  adcSampleCount2 = 0;

  lastTick1 = tStartTimingCycle;
  lastTick2 = tStartTimingCycle;

  /**
     need intelligent init val for zReading.
     const double Z_MAX = 260.0;//214.0;//verysafe
     const double Z_MIN = 127.0;//162.0;//verysafe
  */
  zReadingChange = false;
  interruptKpthDefinitionCode = NO_INTERRUPT_KPTH_PROFILING;
  zReading = kpthspace::Z_MID;
  zReadingMaybe = kpthspace::Z_MID;
  magForceUp = false;
  interruptType = 0;
  confidence = 0;
  power = 0L;

  /**
   * only two history objects needed @ init. more will be created
   * as needed to keep long term history up to
   */
  for (uint16_t i = 0; i < kpthspace::KPTHISTORY_LIST_SIZE_MIN; i++){
    KineticProfileTickHistory kpth;

    kpth.init(tCurrent + i, zReading, !kpthspace::DATA_IS_USEABLE);
    kpthVList.push_back(kpth);
  }

  currentKpth = &kpthVList.at(0);
  priorKpth = &kpthVList.at(1);

  //init here because Brain::oscPro would have been defined long before now
  Brain::oscPro.init();
}

void KineticProfileTickHistoryContainer::updateCurrentKineticProfile(){
  currentKpth->updateProfileData(tCurrent, zReadingMaybe);//zReadingMaybe not zReading. could be more accurate when comparing profiles
}

void KineticProfileTickHistoryContainer::generatePulseProfileData(){
  kpp->zEst = zReading;
  kpp->magForceUp = magForceUp;
  kpp->power = power;
  kpp->confidence = confidence;
  kpp->interruptType = interruptType;
}

KineticProfileTickHistory* KineticProfileTickHistoryContainer::getlongTermKpth(){
  return &(kpthVList.at(kpthspace::KPTHISTORY_LIST_SIZE_MIN-1));
}

KineticProfileTickHistory* KineticProfileTickHistoryContainer::getCurrentKpth(){
  return currentKpth;
}

KineticProfileTickHistoryContainer* KineticProfileTickHistoryContainer::instance = 0;
KineticProfileTickHistoryContainer* KineticProfileTickHistoryContainer::getInstance(){
  if (!instance) instance = new KineticProfileTickHistoryContainer();
  return instance;
}






06/27/21
v1.022

the code below is what i was working on before interrupted and had to save a lot of work fast

suffice to say adc read has been successful today! more on that later ....


double KineticProfileTickHistoryContainer::readSensor(unsigned long tick){
  /*
  potValue = analogRead(pulsecyclespace::potPin);//analogRead(pulsecyclespace::potPin);

  if (potValue > lastPotValue+100 || potValue < lastPotValue-100){
    Serial.println(" potValue = "+String(potValue));
    lastPotValue = potValue;
  }
  */

  /*
  #define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
  #define NO_OF_SAMPLES   64          //Multisampling
  */

  /**
   * largest granularity is 40 values 1.5 mm apart
   * assume linear range between 100 and 3700 or a range of 3600
   * 3600/40 = 90 so +/- 45 on either side
   *
   * In any case since the circuit seems noisier for lower voltage
   * readings I'm restricting the scale to a range from 1800 - 3600
   *
   * without the noise i can dial in the values of adcSampleCount and
   * region around the mean where uncertainty is acceptable
   */
  unsigned long tDel1 = tick - lastTick1;
  unsigned long tDel2 = tick - lastTick2;
  double zVal = zReadingMaybe;
  boolean readingComplete = false;
  boolean wasRead = false;
  uint8_t maxSampleCount = 8;

  if (initialized){

    if (tDel1 < 1000L && adcSampleCount < maxSampleCount){
      adcSampleCount++;
      adc_reading += adc1_get_raw((adc1_channel_t)adcChannel);

      /**
       * 1. fast read times may not give internal caps enough time to discharge
       * 2. longer read times might indicate fast motion since looping is slower
       * 
       * let's say aver loop time is 70 us - what is it for fast motion? 120 us?
       * we want accurate readings within 1000 us but hope for accurate readings within 600 us
       * 1000/120 =~ 8 & 600/70 ~= 8 & 600/120 ~= 5
       * 8 should be the max and 5 the min with 5 corresponding to the lowest value for big activity
       * so if we hit 5 within 600 we're good otherwise keep counting ... 6 within 720?
       * 
       */
      
      if (adcSampleCount == maxSampleCount) {
        readingComplete = true;
      }else if (adcSampleCount == maxSampleCount-1 && tDel1 < 700L){//630 = 9x70
        readingComplete = true;
      }else if (adcSampleCount == maxSampleCount-2 && tDel1 > 860L){
        readingComplete = true;
      }
      wasRead = true;
    }



    if (wasRead){
      if (adcSampleCount > 2){
        readingComplete = true;
        if (tDel1 < 400L) Serial.println("tDel1 = "+String(tDel1)+" & adcSampleCount = "+String(adcSampleCount));

      }else{
        if (tDel1 < 600L) Serial.println(" didn't make the cut adcSampleCount = "+String(adcSampleCount));
        adcSampleCount = 0;
        adc_reading = 0;
        lastTick1 = tick;
      }
    }

      if (readingComplete){
        double adc_readingAve = (double)adc_reading/(double)adcSampleCount;

        if (adc_readingAve < 1755){
          printf("consider this to be the forbidden noisy region");
        }else if (adc_readingAve > lastAdc_readingAve+45|| adc_readingAve < lastAdc_readingAve-45){
          printf("Raw: %f\tVoltage: %dmV\n", adc_readingAve, 1100);
        }

        adcSampleCount = 0;
        adc_reading = 0;
        lastAdc_readingAve = adc_readingAve;
        lastTick1 = tick;

        //zVal = adc_readingAve / 10.0;
      }


  }else{
    adcSampleCount = 0;
    adc_reading = 0;
    lastTick1 = tick;
  }

  if (tDel2 > Brain::TMIN_ZREAD_CHANGE){
    zVal = Brain::readOscillatorProfile(tick, zReading, currentKpth);
    lastTick2 = tick;
  }
  return zVal;
}
06/26/21
v1.021X

tried to code a feature that accepts serial input to change period multiplier coef. seems like it slowed everything down for no reason

keeping this source code but reverting to v1.021

06/26/21
v1.021

last checkin before hooking up ADC

I gave up on trying to add getInstance() to AMSAESingleton using template

too messy at least for now

06/26/21
v1.020

more delayMicroseconds(10); added to initializations

06/26/21
v1.019

super fast checkin just in case - i made changes to Coil and Pulse and Kpth that may change efficiency

interesting to note that if I comment out kpthCon->updateTicker() efficiency increases by a lot

06/26/21
v1.018

checking in with lots of changes to includes and AMSAESingleton

06/26/21
v1.017

mini update - better way to introduce a tiny delay after instantiating cons and mans 

1. move the delays out of the Brain initializer
2. put much smaller delays here. only 3 us

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

  /**
  * 20000L is max val - 1000L is for extreme case of ...
  * LMAN_IDLING_ITERS = 4L;
  * unsigned long Brain::T_READ_MIN = 150L;
  * unsigned long Brain::TMIN_ZREAD_CHANGE = 250L;// 500L;//500L;//
  * double Brain::ZREAD_DIFF_MIN = 0.33;
  */
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
}



06/25/21
v1.016

note this from KineticProfileTickHistoryContainer::processSensorData()

    /**
     * ARE YOU SURE ABOUT THIS??
     * MAKE SURE! what is the tDiff between tSensorTick and tCurrent here? FIND OUT!!
     */
    tCurrent = micros();
    /**
     * ARE YOU SURE ABOUT THIS??
     * MAKE SURE! what is the tDiff between tSensorTick and tCurrent here? FIND OUT!!
     */
     
     
one important thing coming out of this checkin is that the loop iter time inconsistency may have nothing to do with my
code. it seems the compiler gets confused maybe? i just proved that by simply removing an unused member prop (try it out
 - add debugPCM to pcMan and set to false in constructor - this speeds up the things substantially. HOW??? i assume this
 member has been added to the index and when the index doesn't see it things slow down? a debugging thing maybe??
 
 whatever the answer, check out the solution i came up with! I put this statement inside every constructor of cons and mans
 after every getInstance() call and it works to improve efficiency!
 
 delayMicroseconds(10);//not sure why this improves run time efficiency but it does!


to do next: need to make sure you have a firm hold on static vars and methods. i'm now more than a little concerened
that maybe including a header incorrectly can mean a completely different value or method for a static?? it seems 
impossible but check anyhow. need to make sure those includes are correct!!

no more sMan->doLoopTask() since there seems to be no good reason for it other than printing average loop times.
SerialManager::doWhileNotIdling() no longer does anything


this was simply a confusing one. still not sure exaclty what the issue is or was. i've been trying to optimize
time per iteration through PCM, but that value seems to fluctuate in ways I dont understand. still, it looks 
damn efficient which is probably all that matters. just wich i had an idea for what causes the inconsistency.

anyhow, here's the main change ...

boolean initializationLoop(void){
  lMan->doLoopTask();
  return Brain::containersAndManagersAreInitialized();
}

boolean Brain::containersAndManagersAreInitialized(){
  if (!coilConInitialized){
    CoilContainer* coilCon = coilCon->getInstance();
    if (coilCon->isInitialized()){
      Serial.println("@ CoilContainer::initialized.");
      coilConInitialized = true;
    }

  }else if (!pcManInitialized){
    if (numNewCycles > 10){
      KineticProfileTickHistoryContainer* kpthCon = kpthCon->getInstance();
      KineticProfileTickHistory* currentKpth = kpthCon->getCurrentKpth();
      uint8_t kpthInterruptType = currentKpth->getInterruptType();

      PulseCycleManager* pcMan = pcMan->getInstance();
      uint8_t pcInterruptType = pcMan->getCurrentInterruptType();
      if (kpthInterruptType == pulsecyclespace::NO_INTERRUPT_CURRENT_PULSE_CYCLE && pcInterruptType == pulsecyclespace::NO_INTERRUPT_CURRENT_PULSE_CYCLE){
        Serial.println("@ PulseCycleManager::initialized::interruptType is default");
        pcManInitialized = true;
      }else{
        //Serial.println("@ PulseCycleManager NOT INITIALIZED numNewCycles = "+String(numNewCycles)+" && kpthInterruptType = "+String(kpthInterruptType)+" && pcInterruptType = "+String(pcInterruptType));
      }
    }

  }else if (!kpthConInitialized){
    KineticProfileTickHistoryContainer* kpthCon = kpthCon->getInstance();
    KineticProfileTickHistory* longTermKpth = kpthCon->getlongTermKpth();
    uint8_t interruptType = longTermKpth->getInterruptType();
    boolean dataIsUsable = longTermKpth->isDataUsable();
    if (dataIsUsable && interruptType == pulsecyclespace::NO_INTERRUPT_CURRENT_PULSE_CYCLE){
      Serial.println("@ KineticProfileTickHistoryContainer::initialized. numNewCycles = "+String(numNewCycles));
      kpthConInitialized = true;
    }
  }
  return coilConInitialized && pcManInitialized && kpthConInitialized ;
}


06/22/21
v1.016X

REALLY having a hard time with initialations causing a lack of performance

the entire idea behind the code for v1.016 was to speed things up but they slowed down
mysteriously. 

new code surrounds this logic ...

boolean initializationLoop(void){
  lMan->doLoopTask();
  return Brain::modulesAreInitialized();
}

boolean Brain::modulesAreInitialized(){
  if (!Brain::pcManInitialized && Brain::numNewCycles > 3){
    Serial.println("@ PulseCycleManager::initialized.");
    Brain::pcManInitialized = true;
  }
  if (!coilCon->isInitialized()){
    Serial.println("@ CoilContainer::initialized.");
  }
  if (!Brain::kpthConInitialized && Brain::kpthVListPtr != nullptr && Brain::kpthVListPtr->at(kpthspace::KPTHISTORY_LIST_SIZE_MIN-1).isDataUsable()){
    Brain::kpthConInitialized = true;
    Serial.println("@ KineticProfileTickHistoryContainer::initialized.");
  }
  return Brain::pcManInitialized && coilCon->isInitialized() && Brain::kpthConInitialized;
}

06/20/21
v1.015X

calling this an 'X version' while I figure out why it's a bit less efficient. not bad, but
i need to know


06/20/21
v1.015

premature checkin for more initialization settings and announcements

1. not sure when to annouunce coilCon init since it happens prior to Serial conn being established
2. really confused now about statics in namespaces and statics in class defs when those class headers are included in multiple places

06/20/21
v1.014

added output to announce completion of initialization

A good version and one to upload to Siteground

a lot accomplished wrt sMan input read for oscillator and debugging output

more tPeriodLists and potential switching combinations

  /* HARDCODE FOR TESTING */
  if (hardcodeOsc){
    oscPro.currentTPeriod = (unsigned long)(minPeriodMult*PERIOD_MIN);
    oscPro.currentZMid = Z_MID;
    oscPro.currentZAmp = 1.0*ZAMP_PMIN;
    oscPro.tPeriodOffset = 0L;
    oscPro.doSetVProfileAtTransition = false;
    if (printOscMssg) {
      Serial.println("@ Brain::readOscillatorProfile hardcoded minPeriodMult = "+(String(minPeriodMult,4)));
      printOscMssg = false;
    }
  }else{
    if (printOscMssg) {
      Serial.println("@ Brain::readOscillatorProfile running through list of profiles");
      printOscMssg = false;
    }
  }
  /* HARDCODE FOR TESTING */


06/20/21
v1.013

added a new sMan feature when input is numerical

0 indicates user wants OscPro to cycle through lists
1-9 indicate user wants to test a multiple of the min period only

seems to have slowed things down though so am checking in this code
and will test against earlier version v1.011 to see if mistakes exist

  /* HARDCODE FOR TESTING*/
  //double priorHardCodeMinPeriodMult = oscPro.hardCodeMinPeriodMult;

  oscPro.hardCodeMinPeriodMult = 10.0;//sMan->debugOscillator()*2.0;

  oscPro.hardcodeOsc = true;//oscPro.hardCodeMinPeriodMult > 0;

  boolean printOut = oscPro.firstOscRead;// || priorHardCodeMinPeriodMult != oscPro.hardCodeMinPeriodMult;

  String printStr = "@ Brain::readOscillatorProfile running through list of profiles";

  if (oscPro.hardcodeOsc){
    oscPro.currentTPeriod = (unsigned long)(oscPro.hardCodeMinPeriodMult*PERIOD_MIN);
    oscPro.currentZMid = Z_MID;
    oscPro.currentZAmp = 1.0*ZAMP_PMIN;
    oscPro.tPeriodOffset = 0L;
    oscPro.doSetVProfileAtTransition = false;
    printStr = "@ Brain::readOscillatorProfile hardcoded hardCodeMinPeriodMult = "+(String(oscPro.hardCodeMinPeriodMult,4));
  }

  if (printOut) {
    Serial.println(printStr);
    oscPro.firstOscRead = false;
  }




06/19/21
v1.012

cleaned up Brain structs - figured out how to init - look at OscillatorProfile for a good reminder

bitwise math to speed up calulateKinetics 

reviewed interrupts and realized I need to be careful to ensure PCM is called at least once every 120 us

the below is the only way PCM will know about interrupts. don't want them to be stale by the time PCM
gets to them. But 120 us max isn't a concern if it's good for performace. an average of 60 us is a good target

void PulseCycleManager::initPulseCycleTask(){
    zEst = KineticProfileTickHistoryContainer::kpp->zEst;
    magForceUp = KineticProfileTickHistoryContainer::kpp->magForceUp;
    power = KineticProfileTickHistoryContainer::kpp->power;
    confidence = KineticProfileTickHistoryContainer::kpp->confidence;
    tCurrent = KineticProfileTickHistoryContainer::kpp->tCurrent;
    interruptType = KineticProfileTickHistoryContainer::kpp->interruptType;

    /** STOP!! test to see how often delTc is large. as of now you have no clue! */
    //unsigned long oldTC = tCurrent;
    tCurrent = micros();
    //unsigned long delTc = tCurrent - oldTC;
    //if (delTc > 40L) Serial.println("WARNING:initPulseCycleTask tCurrent - oldTC = "+String(delTc));
    if (sMan->debugPcMan()) debug = true;

}



06/18/21
v1.011

big time performance increases! changed some statics to constants and dealt with some
division issues that were sucking up cycles. perf has increased overall by about 30%

renamed SAMath to AMSAEUtils


06/17/21
v1.010

namespaces and some confusion about how to initialize structures

because it IS confusing or rather half baked!! so I initialized right into the declaration. 
because it's ok according to the compiler and because just about anything else is an absurd
use of valuable time.


06/17/21
v1.009

checkpoint after editing kMan dependence upon pcMan and adding iterator to OscPro ...

void KineticsManager::initKineticsManagerTask(){
  updatePulseTiming = kpthCon->processSensorData(pcMan->getIsPulseCycleNew());
}

KineticProfileTickHistoryContainer::processSensorData(boolean isPulseCycleNew){
  
  if (isPulseCycleNew) {
    generatePulseProfileData();
    pcMan->setIsPulseCycleNew(false);
    numCyclesSinceLastReset++;
  }
  

Brain::readOscillatorProfile(
    if (oscPro.profileIndex == OSCILLATOR_LIST_SIZE - 1){
        if (oscPro.tPeriodListsIter < oscPro.tPeriodLists.end()-1){
          oscPro.tPeriodListsIter++;
        }else{
          oscPro.tPeriodListsIter = oscPro.tPeriodLists.begin();
        }
        oscPro.tPeriodList = *oscPro.tPeriodListsIter;
      oscPro.profileIndex = 0;
    }else{
      oscPro.profileIndex = oscPro.profileIndex + 1;
    }

06/16/21
v1.008

vectors added as shown below

void KineticProfileTickHistoryContainer::updateKpthList(){
  uint16_t vectorSize = (uint16_t)kpthVList.size();
  KineticProfileTickHistory* oldestKpth = &kpthVList.at(vectorSize-1);
  unsigned long oldestProfileTickStamp = oldestKpth->getTProfileTickStamp();
  long largestTDelta = tCurrent > oldestProfileTickStamp ? tCurrent - oldestProfileTickStamp : LONG_MAX;

    if (largestTDelta > Brain::TDEL_LONGTERM_MIN*4) {
      if (largestTDelta == LONG_MAX) {
        Serial.println("not sure how but we got here. FIX IT!!");
      }
      if (vectorSize >= KPTHISTORY_LIST_SIZE_MAX) {
        Serial.println("vectorSize growth stopped at "+String(vectorSize));
      }
      kpthVList.pop_back();
    }

  KineticProfileTickHistory kpth;
  boolean dataIsUsable = true;
  kpth.init(tCurrent, zReadingMaybe, dataIsUsable);

  kpthVList.insert(kpthVList.begin(), kpth);
  currentKpth = &kpthVList.at(0);
  priorKpth = &kpthVList.at(1);
}

06/15/21
v1.007

checkpoint after adding better debugging

06/14/21
v1.006

checkpoint before possibly damaging with my stl vector rewrites





06/13/21
v1.005

this is a recovery version

i had tried to stout to a file and stdin from another file via Elcipse settings. all went wrong from there.

this is a reversion to v1.004 plus some changes to lMan and Idler wrt. watchdog efficiency


06/13/21
v1.004

SUPER SUCCESS:: NO AUTO REBOOTING!!!
- changed CPU speed to 240 mhz from 120 mhz
- changed flash speed from 40 mhz to 80 mhz
- unchecked 'arduino autostart with setup and loop' from menuconfig
- main now has this in it extern "C" void app_main(void)
- this appears to be about 25% faster than setup() loop() design for some reason
- added TWD (timer watchdog) reset to Idler.cpp & LoopManager.cpp
- everything works as expected except for time per iter which is large - not sure why
- SerialManager.h is edited accordingly for LMAN_ITERS, KMAN_ITERS, etc
- even though feedDog() only takes 12-13 us, if called often it will suck up time. 
- seems to work perfectly, but keep an eye on LoopManager::safeLoopIterMaxForWatchdogFeed this can be much much smaller if needed!!

/**
 * due to porting to ESP-IDF and RTOS need to deal with
 * different & more 'touchy' timer watchdog
 */
void Idler::feedTheDog(){
  // feed dog 0
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable // @suppress("Field cannot be resolved")
  TIMERG0.wdt_feed=1;                       // feed dog // @suppress("Field cannot be resolved")
  TIMERG0.wdt_wprotect=0;                   // write protect // @suppress("Field cannot be resolved")
  // feed dog 1
  TIMERG1.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable // @suppress("Field cannot be resolved")
  TIMERG1.wdt_feed=1;                       // feed dog // @suppress("Field cannot be resolved")
  TIMERG1.wdt_wprotect=0;                   // write protect // @suppress("Field cannot be resolved")
}


06/12/21
v1.003

oscillator hardcoded here
  oscPro.currentTPeriod = (unsigned long)(3.0*PERIOD_MIN);
  oscPro.currentZMid = Z_MID;
  oscPro.currentZAmp = 1.0*ZAMP_PMIN;
  oscPro.tPeriodOffset = 0L;
  oscPro.doSetVProfileAtTransition = false;
  
first checkin with all components registered and linked up

some cleanup that was worthwhile: 
extra references removed
all member props were initialized in constructor
forced me to go through code almost line by line after 2 weeks away from it
forcing me to see what goes on during the idling loops both with the esp32 and the arduino and rtos code

and now it's forcing me to consider empty loops because ...

this code checked in results in a CPU idling related reboot several times a minute

i've tried placing yield(); in select places to no avail - i'm not convinced it's
actually doing anything.

currently i understand it may be an issue with RTOS. that's fine. time to jump in
and fix it. following this link below for now 

https://esp32.com/viewtopic.php?t=10411








v1.002

finally a clean build!! what a slog!! and lots of lessons learned.

LESSONS:
1. to create a new idf project DO NOT START WITH get-started examples!!
2. New->Espressif IDF Project
3. exit eclipse
4. follow instructions here ... https://github.com/espressif/arduino-esp32/blob/master/docs/esp-idf_component.md
5. BE PATIENT with idf.py menuconfig because IT MAY NOT SHOW ARDUINO CONFIG FIRST TIME THROUGH
6. run idf.py build
7. AGAIN RUN idf.py menuconfig and set the Arduino options and save
8. run idf.py build again
9. open eclipse with ec command
10. project -> properties -> C++ General -> Indexer -> Enable Project Specific Settings
11. main -> main.c change to main.cpp
12. add other files *.cpp and *.h or try to link to a dir
13. main -> CMakeLists.txt: change main.c to main.cpp
14. main -> CMakeLists.txt: add other files. example is below these bullet points
15. build
16. run

CMakeLists.txt ex.
# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
	"main.cpp" 
	"Idler.h"
	"Idler.cpp" 
	"TaskManager.h" 
	"TaskManager.cpp" 
	"LoopManager.h" 
	"LoopManager.cpp" 
	"KineticsManager.h" 
	"KineticsManager.cpp" 
	"SerialManager.h" 
	"SerialManager.cpp" 
	"Brain.cpp"
	"Brain.h"
	"CoilContainer.cpp"
	"CoilContainer.h"
	"Coil.cpp"
	"Coil.h"
	"KineticProfileTickHistoryContainer.cpp"
	"KineticProfileTickHistoryContainer.h"
	"KineticProfileTickHistory.cpp"
	"KineticProfileTickHistory.h"
	"KineticPulseProfile.h"
	"Pulse.cpp"
	"PulseCycle.cpp"
	"PulseCycle.h"
	"PulseCycleManager.cpp"
	"PulseCycleManager.h"
	"Pulse.h"
	"SAMath.cpp"
	"SAMath.h"
	"TimingManager.cpp"
	"TimingManager.h"
	)
set(COMPONENT_ADD_INCLUDEDIRS 
	"."
	)
register_component()
