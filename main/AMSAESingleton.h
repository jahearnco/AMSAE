/*
  AMSAESingleton.h - Library for AMSAESingleton props and methods
*/
#ifndef AMSAESingleton_h
#define AMSAESingleton_h
#include <AMSAEUtils.h>

class AMSAESingleton{

  private:
    unsigned long tCreated;
    const char* subClassName;

  public:
    boolean initialized;
    boolean isInitialized();
    void setInitialized(boolean isInited);
    void printTCreated();

    AMSAESingleton(const char* subName);
    AMSAESingleton();
};

#endif
