//file: main.cpp
//#include <LoopManager.h>

#include <Arduino.h>
#include <servoControl.h>


//LoopManager* lMan;
servoControl myServo;

int degMin = 16;
int degMax = 86;
int deg = degMin;

bool cw = true;

void setup(void){
  //using namespace adcspace;
  Serial.begin(115200);

 // pinMode(adcspace::testInputPin1, OUTPUT);
  //delay(Brain::primingDelayMs);
  //Serial.println("");
  //Serial.println("@ main::setup config ADC ... ");
  //adc1_config_width(adcWidth);
  //adc1_config_channel_atten((adc1_channel_t)adcChannel1, adcAtten);
 // adc2_config_channel_atten((adc2_channel_t)adcChannel2, adcAtten);
  //Serial.println("");
  //Serial.println("@ main::setup creating and initializing singletonsX ... ");
 // lMan = lMan->getInstance();
  //double dbus = sc.getDutyByuS(77.0);
  //Serial.println("dbus = ");
  //Serial.printf("%lf", dbus);
  //Serial.println(" us");

	// Allow allocation of all timers
	//ESP32PWM::allocateTimer(0);
	//ESP32PWM::allocateTimer(1);
	//ESP32PWM::allocateTimer(2);
	//ESP32PWM::allocateTimer(3);

	myServo.attach(GPIO_NUM_23);
	Serial.println(" setup complete");
}

//boolean initializationLoop(void){
	//lMan->doLoopTask();
//	return Brain::containersAndManagersAreInitialized();
//}

void loop(void){
	//lMan->doLoopTask();
	//lMan->testPulsePins();

	if (deg == degMax){
		cw = false;
		myServo.detach();
		vTaskDelay(2000 / portTICK_RATE_MS);
		myServo.attach(GPIO_NUM_23);
	}else if (deg == degMin){
		cw = true;
		myServo.detach();
		vTaskDelay(2000 / portTICK_RATE_MS);
		myServo.attach(GPIO_NUM_23);
	}

	if (cw){
		deg += 2;;

	}else{
		deg -= 2;;

	}


	myServo.write(deg);
	//delayMicroseconds(200);
	vTaskDelay(10 / portTICK_RATE_MS);
	//delayMicroseconds(400);


}

extern "C" void app_main(void){
	initArduino();
    setup();
    while (1) { loop(); }
}
