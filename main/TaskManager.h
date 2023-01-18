/*
  TaskManager.h - Library for TaskManager props and methods
*/
#ifndef TaskManager_h
#define TaskManager_h
#include <Idler.h>

class TaskManager : public Idler{
  private:

  public:
    TaskManager(const char* subName, unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo);
};

#endif
