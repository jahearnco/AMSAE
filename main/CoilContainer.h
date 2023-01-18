/*
  CoilContainer.h - Library for CoilContainer props and methods
*/
#ifndef CoilContainer_h
#define CoilContainer_h
#include <AMSAESingleton.h>
#include <Coil.h>
#include <SerialManager.h>

class CoilContainer : public AMSAESingleton
{
  private:
	static SerialManager* sMan;
	static CoilContainer* instance;
    static Coil* coil1;
    static Coil* coil2;
    static Coil* coil3;
    static Coil* coil4;
    static Coil* coil5;

    static Coil* coilW;
    static Coil* coilX;
    static Coil* coilY;
    static Coil* coilZ;
    
    static Coil* coilList[5];//array of objects to keep memory addresses local for faster lookup
    static Coil* prioritizedCoils[3];//array of pointers. what would be advantage to array of objects here when the objects are constantly changing and you're accessing this array only once before it changes. right?

    static double dRestPerLifetimeMin;
    static boolean ccDebug;

    unsigned long tForcedRest;
  
  public:
    CoilContainer();
    void init();

    static CoilContainer* getInstance();
    static Coil** getPrioritizedCoils(double zEst, boolean magForceUp, unsigned long tc);
    static Coil** getDummyCoils();

    void allPinsLow(unsigned long tc);
    boolean healthCheck(unsigned long tc);
};

#endif
