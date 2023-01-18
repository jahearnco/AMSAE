/*
  TaskManager.cpp - Library for PulseCycleManager props and methods
*/
#include <TaskManager.h>

TaskManager::TaskManager(const char* subName, unsigned long innerLoopIdleIters, unsigned long outerLoopIdleIters, boolean doPrintIdlingInfo) : Idler(subName) {
  if (innerLoopIdleIters != 0L && outerLoopIdleIters  != 0L){
    initIdler(innerLoopIdleIters, outerLoopIdleIters, doPrintIdlingInfo);
  }else{
    Serial.println("SET Idler iteration values");
  }
}
