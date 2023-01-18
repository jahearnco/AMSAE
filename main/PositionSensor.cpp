  /*
  PositionSensor.cpp - Library for PositionSensorManager props and methods
*/
#include <PositionSensor.h>

double PositionSensor::readSensor(unsigned long tick, double zReadingMaybe, double zReading, boolean debug){
	using namespace adcspace;
	double zVal = zReadingMaybe;
	double adc_readingAve = 0;

	if (newSamplingCycle){
		startCycleTick = tick;
		newSamplingCycle = false;
	}

	/**
	 * begin: take one reading sample every time here
	 */
	if (adcSampleCount%2 == 1){
		adc1_readings.push_back(adc1_get_raw((adc1_channel_t)adcChannel1));

	}else{
		int adc2_raw;
		esp_err_t r = adc2_get_raw((adc2_channel_t)adcChannel2, adcWidth, &adc2_raw);
		if ( r == ESP_OK ) {
		adc2_readings.push_back((uint32_t)adc2_raw);

		} else if ( r == ESP_ERR_TIMEOUT ) {
			printf("ADC2 used by Wi-Fi.\n");
		}
	}
	/**
	 * end: take one reading sample every time here
	 */
	unsigned long tSamplingElapsed = tick - startCycleTick;
	boolean readingComplete = Brain::completeSensorReading(tSamplingElapsed, adc1_readings, adc2_readings, adc_readingAve, adcSampleCount);

	/**
	* BEGIN DBG - comment out if slowing things down
	*
	* good place to remind what the goal is and what I'm up to here ...
	*
	* 1. noise is bad enough that I'm swapping out caps to see where i get bang for my buck - currently looks like 0.022 uf - 0.033 uf might be ideal
	* 2. I haven't decided on granularity yet. 1.5 mm or possibly 1.7 mm is max.
	*    i'm considering letting things stay noisy and allow it to make it's way through AMSAE which shouldn't have an issue with it
	*    because confidence will be low and kpp eval will  deal
	* 3. if noisy at zDelAbsMin that's fine as long as much less noisy at zDelAbsMid and rarely noisy at zDelAbsMax.
	*    the noise is predictable and falls off rapidly and reliably with voltage
	* 4. peakDev is a critical value and together with values in #2 need to arrive at a sensible value for this which depends on opt2003Range
	* 5. 2450 above might be too safe.
	*    chose to be in linear range which is good. that's ok for now.
	*    i'd rather all these values are on the extra-safe side for the following checkins
	* 6. attenuation setting is worth changing and testing - not exactly sure why but figure it out. might work better with OPT2003
	* 6a. for instance, ideally would like 150 values between 75 and 925. peakDev = 3
	*
	if (!tPingd && adcSampleCount == 1 && tick%20000 < 10000){
		digitalWrite(adcspace::testInputPin1, HIGH);
		tPing = tick;
		tPingd = true;

	}else if (tPingd && adcSampleCount > 2 && adcSampleCount < 5){

		double tDel2 = tick-tPing;
		if (tDel2 > 1100 || tSamplingElapsed < Brain::TMIN_ZREAD_CHANGE) {
			uint32_t adc_readingsTotDBG = 0;
			for (vector<uint32_t>::iterator adc1It = adc1_readings.begin(); adc1It != adc1_readings.end(); ++adc1It){
				adc_readingsTotDBG += *adc1It;
			}
			for (vector<uint32_t>::iterator adc2It = adc2_readings.begin(); adc2It != adc2_readings.end(); ++adc2It){
				adc_readingsTotDBG += *adc2It;
			}
			double adc_readingAveDBG = (double)adc_readingsTotDBG/adcSampleCount;
			double zValDBG = rangeCoef*(adc_readingAveDBG - 150) + rangeMin;//0.026 = (2450-150)/100 - conversion to mm range is 142 - 242 = 100 mm
			double zDelAbs = abs(zValDBG - zReading);
			double zDelTestMax = 6.00;//zDelAbsMax;//3.0;//zDelAbsMin;//1.15;

			if (zDelAbs >= zDelTestMax){
				Serial.println("(tick-tPing) = "+String(tDel2)+" & sampleCount = "+String(adcSampleCount)+" & zDelAbs = "+String(zDelAbs)+" & zValDBG = "+String(zValDBG));
				digitalWrite(adcspace::testInputPin1, LOW);
				tPingd = false;
			}
		}
	}else if (readingComplete){
		digitalWrite(adcspace::testInputPin1, LOW);
	}
	** END DBG */
	if (readingComplete){

		if (adc_readingAve < 5 || adc_readingAve > 3150){
			printf("ADC2 consider this to be the forbidden noisy region");

		}else if (adc_readingAve > lastAdc_readingAve+peakDev || adc_readingAve < lastAdc_readingAve-peakDev){

			/**
			 * consult pics on iPhone for this ...
			 * zEst1 = D - z1 - y
			 * 		= 651.45 (valdosta 12/23/21) - 420.85 - y
			 * 		= 230.6 - y
			 *
			 * 		= 615.89 (valdosta 12/23/21) - 420.85 - y
			 * 		= 195.04 - y
			 */
			zVal = 195.04 - rangeCoef*(adc_readingAve - readMin) + rangeMin;

			/**
			 *
			tPSCalcTotal += tSamplingElapsed;

			if (++psCalcCount == 500) {
				double zOpt2003 = rangeCoef*(adc_readingAve - readMin);
				double tAvePerCalc = tPSCalcTotal / (double)psCalcCount;
				Serial.println("tAvePerCalc = "+String(tAvePerCalc,5));
				Serial.println("zVal = "+String(zVal,5));
				psCalcCount = 0;
				tPSCalcTotal = 0L;
			}
			*/


			/*
			if (adcSampleCount < 5 && tSamplingElapsed < 300L){
				Serial.println("tSamplingElapsed = "+String(tSamplingElapsed)+" & sampleCount = "+String(adcSampleCount)+" & zVal = "+String(zVal)+" & zReadingMaybe = "+String(zReadingMaybe));
			}
			*/
			/*
			if (true){
				double zDelAbs = abs(zVal - zReading);
				if (zDelAbs > 0.7){
					//Serial.println("adc_readingAve = "+String(adc_readingAve));
					Serial.println("tSamplingElapsed = "+String(tSamplingElapsed)+" & zDelAbs = "+String(zDelAbs)+" & sampleCount = "+String(adcSampleCount)+" & zVal = "+String(zVal)+" & zReadingMaybe = "+String(zReadingMaybe));
				}
			}
			*/

		}
		adcSampleCount = 0;
		adc1_readings.clear();
		adc2_readings.clear();
		lastAdc_readingAve = adc_readingAve;
		tPingd = false;
		newSamplingCycle = true;
	}
	return zVal;
}

double PositionSensor::readSensorInitializer(unsigned long tick, double zReadingMaybe, double zReading, boolean debug){
  using namespace adcspace;

  double zVal = zReadingMaybe;
  unsigned long tSamplingElapsed = tick - startCycleTick;

  startCycleTick = tick;
  /**
   * simulation only but also initializes device if desired
   */
  if (tSamplingElapsed > Brain::TMIN_ZREAD_CHANGE){
    zVal = Brain::readOscillatorProfile(tick, zReading);
    /*
    or is it zReading??
    zVal = Brain::readOscillatorProfile(tick, zReading);
    */
    startCycleTick = tick;
  }
  return zVal;
}

unsigned long PositionSensor::psCalcCount = 0L;
unsigned long PositionSensor::tPSCalcTotal = 0L;

boolean PositionSensor::newSamplingCycle = true;
uint8_t PositionSensor::adcSampleCount = 0;

vector<uint32_t> PositionSensor::adc1_readings = {};
vector<uint32_t> PositionSensor::adc2_readings = {};

double PositionSensor::lastAdc_readingAve = 2000.0;

unsigned long PositionSensor::startCycleTick = micros();

unsigned long PositionSensor::tPing = micros();
boolean PositionSensor::tPingd = false;

PositionSensor::PositionSensor(){}

