/*
  PulseCycleManager.cpp - Library for PulseCycleManager props and methods
*/
#include <AMSAEUtils.h>

using namespace AMSAEUtilsspace;

double AMSAEUtils::div(double numer, double denom){
  /*if (abs(denom) < 0.1){
    if (denom == 0){ 
      Serial.println("DEBUG @ AMSAEUtils::div() denom is ZERO!!"); 
      return 0.0;
    }else{ 
      Serial.println("DEBUG @ AMSAEUtils::div() denom is very small!!"); 
    }
  }*/
  return numer/denom;
}

unsigned long AMSAEUtils::bwDiv(unsigned long ul1, unsigned long ul2){
	unsigned long temp = 1;
	unsigned long result = 0;
	while(ul2 <= ul1){
		ul2 <<= 1;
		temp <<= 1;
	}
	while(temp > 1){
		ul2 >>= 1;
		temp >>= 1;

		if(ul1 >= ul2){
			ul1 -= ul2;
			result += temp;
		}
	}
	return result;
}

int32_t AMSAEUtils::convertToBaseQ(double v){
  int32_t qVal = 0;
  if (v > V_MIN_POS){
    qVal = (int32_t)(e*log(v/V_MIN_POS) + 0.5);
    
  }else if (v < V_MIN_NEG){
    qVal = -1*(int32_t)(e*log(v/V_MIN_NEG) + 0.5);
    
  }
  return qVal;
}

AMSAEUtils::AMSAEUtils()  { }
