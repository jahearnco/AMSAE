/*
  Idler.cpp - Library for AMSAESingleton props and methods
*/
#include <AMSAESingleton.h>

boolean AMSAESingleton::isInitialized(){
	return initialized;
}

void AMSAESingleton::setInitialized(boolean isInited){
	initialized = isInited;
	//if (initialized){
	//	Serial.print("@ ");
	//	Serial.print(subClassName);
	//	Serial.println(" initialized.");
	//}
}


void AMSAESingleton::printTCreated() {
	Serial.print("@ ");
	Serial.print(subClassName);
	Serial.print(" tCreated = ");
	Serial.println(tCreated);
}

AMSAESingleton::AMSAESingleton(const char* subName){
	initialized = false;
	subClassName = subName;
	tCreated = micros();
	printTCreated();
}

AMSAESingleton::AMSAESingleton() : AMSAESingleton("@AMSAESingleton()::WRONG CONSTRUCTOR"){}
