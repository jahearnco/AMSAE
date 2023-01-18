/*
  Idler.h - Library for Idler props and methods
*/
#ifndef Idler_h
#define Idler_h
#include <AMSAESingleton.h>

const boolean NO_PRINT_IDLE_ITERS_AVE = false;
const boolean PRINT_IDLE_ITERS_AVE = true;
const boolean RETURN_TIME_ELAPSED = true;
const boolean NO_RETURN_TIME_ELAPSED = false;

//no idea why but float.h reference isn't resolved correctly so moving on and olling my own
const double DOUBLE_MAX = std::numeric_limits<double>::max();

class Idler : public AMSAESingleton
{
  private:
    unsigned long idlerIterCount;
    unsigned long printIters;
    unsigned long tLoopStart;

    boolean neverIdle;
    unsigned long idlerIterCountMAX;
    unsigned long outerLoopIdlerIterCountMax;
    boolean returnTimeElapsed;
    double tGradation;

  public:
    Idler();
    Idler(const char* subName);
    static void feedTheDog();

    void initIdler(unsigned long iter1, unsigned long iter2, boolean returnTime);
    void doLoopTask();
    double idlingInterval();

    void setReturnTimeElapsed(boolean setRTE);
    boolean getReturnTimeElapsed();

    void idleAlways(boolean returnTimeElapsed);
    double doWhileIdling();

    virtual void doWhileNotIdling() = 0;
    virtual ~Idler();
};

#endif
