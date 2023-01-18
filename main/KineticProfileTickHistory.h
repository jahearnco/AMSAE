/*
  KineticProfileTickHistory.h - Library for KineticProfileTickHistory props and methods
*/
#ifndef KineticProfileTickHistory_h
#define KineticProfileTickHistory_h
#include <SerialManager.h>

/**
 * BEGIN::EXTREME VALUES FOR EXTREME LIMITS - NAMESPACE PLEASE!!!
 */

/**
 * why don't the two values below equal 1.0? because this is an esp32 not a supercomputer. 
 * to be efficient, time intervals between readings and calculations need to be at least 1 us and realistically closer to the range 50 - 120 us
 * 
 * there's also the difference between the ideal sensor readings and realistic ones. ideally this could be set to check for changes in zReadings as low as 0.1 mm. 
 * as that's ridiculous with huge standard deviation, I'm choosing a more realistic lower boundary while keeping an eye on the uncertainty around it. the range of
 * that more realistic lower boundary is 0.9 - 2.0 mm. commonly I'm using namespace a value near 1.5 mm
 * 
 * smoothing factors above are values chosen empirically for the given real-life granularity necessitated by limitations of sensors + esp32 + random noise + extreme
 * and unexpected kinetic discontinuity (i.e. superjerk). these factors prevent the device from responding to excessive speeds and accelerations. they are the primary 
 * limiting factors of the device, since they disallow responses to any extreme yet realistic kinetic behavior.
 * 
 * as the smoothing factors above represent the definition of the device limitations, they also have uncertainty around them. i.e. the device is senstive at the 
 * extrema involved. the "SAFE" factors below prevent that sensitivity from crashing the device giving it some wiggle room in a sense. all to say that even though
 * these are over factors, they do not necessary represent the upper boundaries of extreme kinetics but rather the upper boundaries of the calculation of those kinetics
 * 
 * should there be a calculation outside this upper boundary the device will stop working.
 */

/**
 * DEVICE IS INTENDED TO HANDLE MAX ACCEL / 2.0 without putting effort into curve correction. some small correction is fine, but it should be the exception (i.e. only once per 10 cycles maybe)
 */
/**
 * safe meaning device is designed to handle velocities at half V_MAX which means 1/4 A_MAX
 * idea is that if this factor is exceeded then return ACALC_WARNING_EXCESSIVE_ACCEL
 */
const double SAFE_AVEACCEL_OVER_FACTOR = 0.25;//0.5^2 is the target
/**
* jerk is tough calc to get accurate because of the large granularity of both t and z measurements.
* jerk calc is used for determining power. only for extreme and unexpected values should it interrupt
* normal cycling. out of bounds kinetics are handled via the velocity and acceleration calcs
* 
* setting this to an extremely large value so it can't be exceeded except in very strange circumstances
*/
const double SAFE_AVEJERK_OVER_FACTOR = 0.5;//0.5^3 is the target but 
/**
 * MAXVELO_OVER_FACTOR is tuned so that Brain::SAFE_AVEVELO_OVER_FACTOR is rarely but not always satisfied at half V_MAX 
 * why not always? because of granularity of tickStamp deltas and zReading deltas - NOT very granular so this catches inaccuracies
 * chosen so it never or almost never is exceeded whne velocity is 1/3 V_MAX
 */
const double MAXVELO_OVER_FACTOR = 1.0;
/**
 * END::EXTREME VALUES FOR EXTREME LIMITS - NAMESPACE PLEASE!!!
 */
class KineticProfileTickHistory
{
  /**
   * each instance of this class has a zRead that specifically matches to kptList
   * not like a hash though, since zRead can be the same for another instance of this class
   */
  private:
    boolean isReset;
    boolean dataIsUsable;
    
    unsigned long tTickZero;
    double zRead;

    unsigned long mostRecentTickStamp;
    double mostRecentZRead;
  
    uint8_t confidence;
    unsigned long jolt;
    unsigned long power;
    boolean magForceUp;
    boolean magForceUpReversal;
    boolean directionReversal;

    /** DEBUG */
    double previousZRead; 
    uint16_t shortTermKpthListIndex;
    uint16_t midTermKpthListIndex;

    long longTermTDelta;
    double vAveLongTerm;
    double vProfile;
    double aAveLongTerm;
    double vAve;
    double aAve;
    double jAve;

    uint8_t interruptType;
    /**
     * useful for simulations during transition at inflection point where v chosen to be very small
     */
    double vProfileAtTransition;

    void initProps();

  public:
    void updateProfileData(unsigned long tTick, double mostRecentZ);
    
    boolean hasBeenReset();
    boolean isDataUsable();
    
    void setProfileZRead(double zReading);
    void setTProfileTickStamp(unsigned long tPTS);
    
    void setVAve(double vA);
    void setVProfile(double vP);
    void setAAve(double aA);
    void setJAve(double jA);

    void setDirectionReversal(boolean dirRev);
    void setMagForceUpReversal(boolean mfuRev);
    void setMagForceUp(boolean mfu);
    
    void setConfidence(uint8_t co);
    void setJolt(unsigned long jo);
    void setPower(unsigned long po);
    void setVProfileAtTransition(double vZ);
    void setLongTermVAve(double longTermVAve);
    void setLongTermAAve(double longTermAAve);
    void setLongTermTDelta(long tDelta);

    void setShortTermKpthListIndex(uint16_t shortTermKpthListIndex);
    void setMidTermKpthListIndex(uint16_t midTermKpthListIndex);

    uint16_t getShortTermKpthListIndex();
    uint16_t getMidTermKpthListIndex();

    double getVAveLongTerm();
    double getVProfile();
    double getVAve();
    double getAAveLongTerm();
    double getAAve();
    double getJAve();
    double getZRead();
    double getVProfileAtTransition();

    unsigned long getTProfileTickStamp();
    double getProfileZRead();

    boolean getDirectionReversal();
    boolean getMagForceUpReversal();
    boolean getMagForceUp();
    uint8_t getConfidence();
    unsigned long getJolt();
    unsigned long getPower();

    void setInterruptType(uint8_t interruptType);
    uint8_t getInterruptType();
    
    void init(unsigned long tTickInit, double mostRecentZRead, boolean usableData);
    KineticProfileTickHistory();
};

#endif
