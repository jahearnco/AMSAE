/*
  AMSAEUtils.h - Library for AMSAEUtils props and methods
*/
#ifndef AMSAEUtils_h
#define AMSAEUtils_h
#include <Arduino.h>
#include <float.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <sys/time.h>
#include <soc/timer_group_struct.h>
#include <soc/timer_group_reg.h>

namespace AMSAEUtilsspace{
	const double e = 2.7183;
	const double pi = 3.141593;
	const double FOUR_DIV_PI2 = 4.0/(pi*pi);

	const double V_MIN_POS = 0.0000001;// units are mm/us - this corresponds to 0.1mm/s -
	const double V_MIN_NEG = -0.0000001;// units are mm/us - this corresponds to 0.1mm/s
}

namespace serialspace {
	const unsigned long KMAN_IDLING_ITERS = 1L;
	const unsigned long TMAN_IDLING_ITERS = 1L;
	const unsigned long PCMAN_IDLING_ITERS = 1L;
	const unsigned long SMAN_IDLING_ITERS = 1000L;
	const unsigned long SMAN_NO_PRINT_ITERS = 70L;
}

namespace coilspace {
	//NORMAL OPERATIONAL CODES
	const uint8_t END_PULSE = 51;
	const uint8_t BEGIN_PULSE = 52;
	const uint8_t NO_CHANGE_ON = 53;
	const uint8_t NO_CHANGE_OFF = 54;
	const uint8_t DO_PULSE = 55;
	const uint8_t PULSE_WIDTH_EXCEEDED = 56;
	const uint8_t IN_QUEUE = 57;
	const uint8_t INDETERMINATE_PULSE_STATUS = 250;//DEBUG

	const unsigned long TON_MAX = 5000L;

	const uint8_t BYTE_MAX = 255;
	const uint8_t BAD_INDEX = BYTE_MAX;
	const uint8_t BAD_STATUS = BYTE_MAX;
	const uint8_t BAD_ORDER = BYTE_MAX;

	const uint8_t MAG_NUM = 6;
	const uint8_t MAYBE_LIVE = 1;
	const uint8_t ZERO_PRIORITY = 0;

	const uint8_t PRIORITY_TRIO_INIT_ARRAY[3] = {BAD_INDEX, ZERO_PRIORITY, ZERO_PRIORITY};
	const uint8_t PRIORITY_SCALE[12] = {MAYBE_LIVE, 10 + 12, 12 + 14, 14 + 16, 17 + 18, 18 + 20, 18 + 20, 20 + 20, 18 + 20, 17 + 16, ZERO_PRIORITY + 15, ZERO_PRIORITY};

	//STATUS CODES
	//GOOD
	const uint8_t C_PULSE_ENDED = 60;
	const uint8_t C_IN_QUEUE_NOT_PULSING = 71;
	const uint8_t C_PULSING = 72;
	const uint8_t C_DEFAULT_NOT_PULSING_CODE = 73;
	const uint8_t C_FORCED_REST = 74;

	//WARNING
	const uint8_t C_ERROR_UNKNOWN = 75;

	//ERROR
	const uint8_t C_ERROR_NOT_PULSING = 76;
	const uint8_t C_ERROR_PULSING_IN_QUEUE = 77;
	const uint8_t C_ERROR_MAX_PULSEWIDTH_EXCEEDED = 78;
	const uint8_t C_ERROR_PULSEWIDTH_EXCEEDED = 79;

	//HEALTH CODES
	const uint8_t C_ERROR_COIL_ABUSE = 203;
	const uint8_t C_ERROR_USAGE_EXCEEDED = 201;
	const uint8_t C_GOOD_HEALTH = 200;

	const double COIL_MIDPOINT_DELTA = 16.11;//15.9;//13.25 + 13.25/5;
	const double MAG_FULL_SEP = 62.05;//43mm + 19.05mm (this is 3/4 inch)
	const double REST_PER_LIFETIME_MIN = 0.142;
}

namespace kpthspace{
	/*
	 * why is index uint32_t and not int?? because there's no need to even approach the uint16_t max (~65000).
	 * each tick is at least 20us and even 60000 ticks corresponds to 12 seconds of product use. is there a reason to store
	 * 200 seconds worth of kinetic data within kptList? that's at least 1000 pulse cycles worth of kinetic data. if historical
	 * info is needed (e.g. to profile the terrain or riding style) then it should be stored elsewhere.
	 */
	const double Z_MAX = 260.0;//214.0;//verysafe//242.0;//safe
	const double Z_MIN = 127.0;//162.0;//verysafe//142.0;//safe
	const double Z_MID = 0.5*(Z_MAX + Z_MIN);//193.5 mm
	const double ZAMP_PMIN = 30.0;
	const unsigned long PERIOD_MIN = 12600L;//6300L;
	const unsigned long PERIOD_MAX = 12600000L;//63000000L;// = 1000 x PERIOD_MIN

	const uint16_t KPTHISTORY_LIST_SIZE_MAX = 128;//16 is lowest value within reason but it's not safe. multiply by something large so that the value ALMOST pushes the limits of memory. it's a problem even when optimized. need 10x memory to be worry free.
	const uint16_t KPTHISTORY_LIST_SIZE_MIN = 32;

	/**
	 * the idea is that since each kpt has 6 velo props, the 6th one is set *only* after 1024 sensor reads. that will be very rare but could be useful
	 * */

	const uint8_t JCALC_SIGNIFICANT_EVENT = 187;
	const uint8_t VCALC_SUCCESS = 188;
	const uint8_t VCALC_WARNING_EXCESSIVE_SPEED = 189;
	const uint8_t ACALC_SUCCESS = 190;
	const uint8_t ACALC_WARNING_EXCESSIVE_ACCEL= 191;
	const uint8_t ACALC_SIGNIFICANT_EVENT =192;
	const uint8_t ACALC_MAGFORCEUP_REVERSAL = 193;
	const uint8_t JCALC_SUCCESS = 194;
	const uint8_t JCALC_WARNING_EXCESSIVE_JERK = 195;
	const uint8_t VCALC_ERROR_UNDETERMINED = 196;
	const uint8_t KPTHISTORY_DATA_NOT_SET = 197;
	const uint8_t VCALC_MIN_EXTREMA = 198;
	const uint8_t KPT_LIST_SIZE = 0;//deprecated since esp32 don't have enough memory to keep such a large data set. but leave for now

	const double V_MAX = ZAMP_PMIN*2.0*AMSAEUtilsspace::pi/PERIOD_MIN;// ~ 15 mm/ms
	const double V_MIN = ZAMP_PMIN*2.0*AMSAEUtilsspace::pi/PERIOD_MAX;// ~ 1.5 mm/sec

	const double A_MAX = V_MAX * 2.0 * AMSAEUtilsspace::pi / PERIOD_MIN;
	const double A_MIN = V_MIN * 2.0 * AMSAEUtilsspace::pi / PERIOD_MAX;
	const double A_MIN_HALFNUM = 0.5/A_MIN;

	const double J_MAX = A_MAX * 2.0 * AMSAEUtilsspace::pi / PERIOD_MIN;
	const double J_MIN = A_MIN * 2.0 * AMSAEUtilsspace::pi / PERIOD_MAX;
	const double J_MIN_HALFNUM = 0.5/J_MIN;

	const boolean DATA_IS_USEABLE = true;
}

namespace adcspace{
	const adc_channel_t adcChannel1 = ADC_CHANNEL_0;     //GPIO36 ADC1
	const adc_channel_t adcChannel2 = ADC_CHANNEL_0;     //GPIO4 ADC2
	const adc_bits_width_t adcWidth = ADC_WIDTH_BIT_12;
	const adc_atten_t adcAtten = ADC_ATTEN_DB_11;
	const uint8_t testInputPin1 = 14;

	const double zDelAbsMin = 0.75;
	const double zDelAbsMid = 1.0;
	const double zDelAbsMax = 1.5;

	/**
	 * taking cue that adc linear range is between 150 and 2450 for ADC_ATTEN_DB_11...
	 * 1. voltage divider values chosen for ~ 10V max output from sensor and highest usable value ~ 2.7V
	 * 2. distance measurements were taken within stated linear range: @53mm ~ 200 & @134mm ~ 2443 out of 4095
	 * 3. this does not represent usable range but rather most linear range. usable range is ~ 150 - 250
	 * 4. meanwhile there's a LOT of room for adjustments of ... Brain::MIN_SAMPLE_SIZE,T_READ_MIN,TMIN_ZREAD_CHANGE
	 * 5. note: Brain::MAX_VPEAK_DEV is adjustable and set to 1.5*peakDev ~ 15 for now
	 *
	 * voltage divider splitting sensor output which maxes ~ 10V ... 5.1k, 0.99k, 0.47k, 0.47k
	 * read at 5.1k drop
	 */
	const uint16_t readMin = 200;
	const uint16_t readMax = 2443;

	const double opt2003Range = 81.0;
	const double adcVInputRange = readMax-readMin;
	const double rangeCoef = opt2003Range/adcVInputRange;

	/** rangeMin may vary with installation for various reasons
	 *
	 TEST FAN SETUP =>> const double rangeMin = kpthspace::Z_MID - opt2003Range/2;
	 *
	 */

	const double rangeMin = 1.0 - opt2003Range/2;

	/** GRANADA INN 102921
	 * opt2003 calls
	 *
	 * const double rangeMin = 226.- opt2003Range/2;
	 *
	 */

	const double peakDev = 0.5*zDelAbsMin*adcVInputRange/opt2003Range;//
}

namespace pulsecyclespace{
	const unsigned long minPW = 200L;

	const uint8_t pulsePin_1 = 13;//A
	const uint8_t pulsePin_2 = 12;//A

	const uint8_t pulsePin_3 = 26;//B
	const uint8_t pulsePin_4 = 25;//B

	const uint8_t pulsePin_5 = 5;//C
	const uint8_t pulsePin_6 = 18;//C

	const uint8_t pulsePin_7 = 14;//D
	const uint8_t pulsePin_8 = 27;//D

	const uint8_t pulsePin_9 = 19;//E
	const uint8_t pulsePin_0 = 21;//E

	const uint8_t pulsePin_Z1 = 99;//4;
	const uint8_t pulsePin_Z2 = 99;//4;

	const uint8_t NO_INTERRUPT_CURRENT_PULSE_CYCLE = 0;
	const uint8_t RESUME_INTERRUPT = 1;
	const uint8_t SURGE_INTERRUPT = 2;
	const uint8_t MIN_EXTREMA_INTERRUPT = 3;
	const uint8_t MAX_EXTREMA_INTERRUPT = 4;
	const uint8_t INIT_INTERRUPT_TYPE = 5;

	const uint8_t HIGH_POWER_PULSING = 6;
	const uint8_t MEDIUM_POWER_PULSING = 7;
	const uint8_t LOW_POWER_PULSING = 8;
	const uint8_t STACCATO_PULSING = 9;
	const uint8_t EMPTY_PULSING = 10;

	const uint8_t PCM_CURRENT_CYCLE_DISABLED = 25;
	const uint8_t PCM_CURRENT_CYCLE_ENDED = 29;
	//const uint8_t PCM_CURRENT_CYCLE_BEGINNING = 30;
	const uint8_t PCM_CURRENT_CYCLE_ENDING = 31;
	//const uint8_t PCM_CURRENT_CYCLE_MIDWAY = 32;
	const uint8_t PCM_CURRENT_CYCLE_POTENT = 33;

	const uint8_t PC_DISABLED = 18;
	const uint8_t PC_CYCLE_ENDED = 19;
	const uint8_t PC_CYCLE_WARNING = 34;
	const uint8_t PC_CYCLE_ERROR = 35;
	const uint8_t PC_CYCLE_ZUP_CHANGE = 36;
	const uint8_t PC_NOT_PULSING_IN_QUEUE = 37;
	const uint8_t PC_CYCLE_PULSING = 38;
	const uint8_t PC_INIT = 39;
	const uint8_t PC_CYCLE_RESTING = 40;
	const uint8_t PC_PRE_INIT = 41;
}

class AMSAEUtils
{
  public:
    static double div(double numer, double denom);
    static unsigned long bwDiv(unsigned long ul1, unsigned long ul2);
    static int32_t convertToBaseQ(double v);
    AMSAEUtils();
};

#endif
