/** ESP32 ShockAbsorber by John Ahearn 2021 */
    /**
     * SIMULATION NOTES:
     *
    speed at center is amplitude (A) times angular velocity
    so ... pick A first. something reasonable like 10mm-30mm. let's start with 30mm.
    anything faster than 15mm/1ms is probably unreasonable, and this is pretty fast as it is.
    but we can use this as a max and go down from here. OK??
    for fastest (15mm/ms) ... 30mm * 2 * pi / P = 15mm / 1000us
    for normal fast (5mm/ms) ... 30mm * 2 * pi / P = 5mm / 1000us
    for normal  (2mm/ms) ... 30mm * 2 * pi / P = 2mm / 1000us
    for slow  (0.2mm/ms) ... 30mm * 2 * pi / P = 0.2mm / 1000us
    Pmin = 2.0 * 2 * pi * 1000 us ~ 12600 us
    Pfast = 6.0 * 2 * pi * 1000 us ~ 37800 us
    Pnorm = 15.0 * 2 * pi * 1000 us ~ 94500 us
    Pslow = 150.0 * 2 * pi * 1000 us ~ 945000 us
  */
    /**
       TEST SANDBOX
      Pmin @ 15mm = 1.0 * 2 * pi * 1000 us ~ 6300 us
      Pfast @ 45mm = 9.0 * 2 * pi * 1000 us ~ 56700 us
      Pnorm @ 30mm = 15.0 * 2 * pi * 1000 us ~ 94500 us
      Pslow @ 60mm = 300.0 * 2 * pi * 1000 us ~ 1900000 us
    */
    /*
    double zMid1 = Z_MID;
    double zMidDel1 = 15.0;
    unsigned long tPeriod1 = 6300L;//this is fast, but estimating not too fast? i'm more concerned about being too slow at the amplitudes

    double zMid2 = Z_MID + 10.0;
    double zMidDel2 = 30.0;
    unsigned long tPeriod2 = 94500L;//this is fast, but estimating not too fast? i'm more concerned about being too slow at the amplitudes

    double zMid3 = Z_MID - 25.0;
    double zMidDel3 = 45.0;
    unsigned long tPeriod3 = 56700L;//this is fast, but estimating not too fast? i'm more concerned about being too slow at the amplitudes
    */
#include "LoopManager.h"
LoopManager* lMan;
unsigned long tSetupDelay = 2000L;//long because testing OscillatorProfile has a period that's long and if everything starts too soon time deltas go negative
void setup(){ Serial.begin(115200); delay(tSetupDelay); Serial.println(String(tSetupDelay)+"ms Hello! Here we go!"); lMan = lMan->getInstance("lMan"); }
void loop(){ lMan->doLoopTask(); }
