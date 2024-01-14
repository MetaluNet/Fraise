/*!
 *  @file Adafruit_APDS9960.cpp
 *
 *  @mainpage Adafruit APDS9960 Proximity, Light, RGB, and Gesture Sensor
 *
 *  @section author Author
 *
 *  Ladyada, Dean Miller (Adafruit Industries)
 *
 *  @section license License
 *
 *  Software License Agreement (BSD License)
 *
 *  Copyright (c) 2017, Adafruit Industries
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holders nor the
 *  names of its contributors may be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fruit.h>
#include <i2c_master.h>
#include "APDS9960.h"

uint8_t APDS9960gestCnt;

uint8_t APDS9960UCount;
uint8_t APDS9960DCount;

uint8_t APDS9960LCount;
uint8_t APDS9960RCount;
/* types: -----------------------------------------------------------------------------------*/

typedef union {
	uint8_t pack;
	struct {
		// power on
		uint8_t PON : 1;

		// ALS enable
		uint8_t AEN : 1;

		// Proximity detect enable
		uint8_t PEN : 1;

		// wait timer enable
		uint8_t WEN : 1;

		// ALS interrupt enable
		uint8_t AIEN : 1;

		// proximity interrupt enable
		uint8_t PIEN : 1;

		// gesture enable
		uint8_t GEN : 1;
	} parts;
} enable_t;
enable_t _enable;

typedef union {
	uint8_t pack;
	struct {
		// ALS Interrupt Persistence. Controls rate of Clear channel interrupt to
		// the host processor
		uint8_t APERS : 4;

		// proximity interrupt persistence, controls rate of prox interrupt to host
		// processor
		uint8_t PPERS : 4;
	} parts;
} pers_t;
pers_t _pers;

typedef union {
	uint8_t pack;
	struct{
		uint8_t _____ : 1;
		uint8_t WLONG : 1;
	} parts;
} config1_t;
config1_t _config1;

typedef union {
	uint8_t pack;
	struct{
		/*Proximity Pulse Count. Specifies the number of proximity pulses to be
		generated on LDR. Number of pulses is set by PPULSE value plus 1.
		*/
		uint8_t PPULSE : 6;

		// Proximity Pulse Length. Sets the LED-ON pulse width during a proximity
		// LDR pulse.
		uint8_t PPLEN : 2;
	} parts;
} ppulse_t;
ppulse_t _ppulse;

typedef union {
	uint8_t pack;
	struct{
		// ALS and Color gain control
		uint8_t AGAIN : 2;

		// proximity gain control
		uint8_t PGAIN : 2;

		// led drive strength
		uint8_t LDRIVE : 2;
	} parts;
} control_t;
control_t _control;

typedef union {
	uint8_t pack;
	struct{
		uint8_t _____ : 4;
		/* Additional LDR current during proximity and gesture LED pulses. Current
		value, set by LDRIVE, is increased by the percentage of LED_BOOST.
		*/

		uint8_t LED_BOOST : 2;

		// clear photodiode saturation int enable
		uint8_t CPSIEN : 1;

		// proximity saturation interrupt enable
		uint8_t PSIEN : 1;

		/*uint8_t get() {
		  return (PSIEN << 7) | (CPSIEN << 6) | (LED_BOOST << 4) | 1;
		}*/
	} parts;
} config2_t;
config2_t _config2;

typedef union {
	uint8_t pack;
	struct{
		/* ALS Valid. Indicates that an ALS cycle has completed since AEN was
		asserted or since a read from any of the ALS/Color data registers.
		*/
		uint8_t AVALID : 1;

		/* Proximity Valid. Indicates that a proximity cycle has completed since PEN
		was asserted or since PDATA was last read. A read of PDATA automatically
		clears PVALID.
		*/
		uint8_t PVALID : 1;

		/* Gesture Interrupt. GINT is asserted when GFVLV becomes greater than
		GFIFOTH or if GVALID has become asserted when GMODE transitioned to zero.
		The bit is reset when FIFO is completely emptied (read).
		*/
		uint8_t GINT : 1;

		// ALS Interrupt. This bit triggers an interrupt if AIEN in ENABLE is set.
		uint8_t AINT : 1;

		// Proximity Interrupt. This bit triggers an interrupt if PIEN in ENABLE is
		// set.
		uint8_t PINT : 1;

		/* Indicates that an analog saturation event occurred during a previous
		proximity or gesture cycle. Once set, this bit remains set until cleared by
		clear proximity interrupt special function command (0xE5 PICLEAR) or by
		disabling Prox (PEN=0). This bit triggers an interrupt if PSIEN is set.
		*/
		uint8_t PGSAT : 1;

		/* Clear Photodiode Saturation. When asserted, the analog sensor was at the
		upper end of its dynamic range. The bit can be de-asserted by sending a
		Clear channel interrupt command (0xE6 CICLEAR) or by disabling the ADC
		(AEN=0). This bit triggers an interrupt if CPSIEN is set.
		*/
		uint8_t CPSAT : 1;

		/*void set(uint8_t data) {
		  AVALID = data & 0x01;
		  PVALID = (data >> 1) & 0x01;
		  GINT = (data >> 2) & 0x01;
		  AINT = (data >> 4) & 0x01;
		  PINT = (data >> 5) & 0x01;
		  PGSAT = (data >> 6) & 0x01;
		  CPSAT = (data >> 7) & 0x01;
		}*/
	} parts;
} status_t;
status_t _status;

typedef union {
	uint8_t pack;
	struct{
		// proximity mask
		uint8_t PMASK_R : 1;
		uint8_t PMASK_L : 1;
		uint8_t PMASK_D : 1;
		uint8_t PMASK_U : 1;

		/* Sleep After Interrupt. When enabled, the device will automatically enter
		low power mode when the INT pin is asserted and the state machine has
		progressed to the SAI decision block. Normal operation is resumed when INT
		pin is cleared over I2C.
		*/
		uint8_t SAI : 1;

		/* Proximity Gain Compensation Enable. This bit provides gain compensation
		when proximity photodiode signal is reduced as a result of sensor masking.
		If only one diode of the diode pair is contributing, then only half of the
		signal is available at the ADC; this results in a maximum ADC value of 127.
		Enabling PCMP enables an additional gain of 2X, resulting in a maximum ADC
		value of 255.
		*/
		uint8_t PCMP : 1;

		/*uint8_t get() {
		  return (PCMP << 5) | (SAI << 4) | (PMASK_U << 3) | (PMASK_D << 2) |
				 (PMASK_L << 1) | PMASK_R;
		}*/
	} parts;
} config3_t;
config3_t _config3;

typedef union {
	uint8_t pack;
	struct{
		/* Gesture Exit Persistence. When a number of consecutive "gesture end"
		occurrences become equal or greater to the GEPERS value, the Gesture state
		machine is exited.
		*/
		uint8_t GEXPERS : 2;

		/* Gesture Exit Mask. Controls which of the gesture detector photodiodes
		(UDLR) will be included to determine a "gesture end" and subsequent exit
		of the gesture state machine. Unmasked UDLR data will be compared with the
		value in GTHR_OUT. Field value bits correspond to UDLR detectors.
		*/
		uint8_t GEXMSK : 4;

		/* Gesture FIFO Threshold. This value is compared with the FIFO Level (i.e.
		the number of UDLR datasets) to generate an interrupt (if enabled).
		*/
		uint8_t GFIFOTH : 2;

		//uint8_t get() { return (GFIFOTH << 6) | (GEXMSK << 2) | GEXPERS; }
	} parts;
} gconf1_t;
gconf1_t _gconf1;

typedef union {
	uint8_t pack;
	struct{
		/* Gesture Wait Time. The GWTIME controls the amount of time in a low power
		mode between gesture detection cycles.
		*/
		uint8_t GWTIME : 3;

		// Gesture LED Drive Strength. Sets LED Drive Strength in gesture mode.
		uint8_t GLDRIVE : 2;

		// Gesture Gain Control. Sets the gain of the proximity receiver in gesture
		// mode.
		uint8_t GGAIN : 2;

		//uint8_t get() { return (GGAIN << 5) | (GLDRIVE << 3) | GWTIME; }
	} parts;
} gconf2_t;
gconf2_t _gconf2;

typedef union {
	uint8_t pack;
	struct{
		/* Number of Gesture Pulses. Specifies the number of pulses to be generated
		on LDR. Number of pulses is set by GPULSE value plus 1.
		*/
		uint8_t GPULSE : 6;

		// Gesture Pulse Length. Sets the LED_ON pulse width during a Gesture LDR
		// Pulse.
		uint8_t GPLEN : 2;

		//uint8_t get() { return (GPLEN << 6) | GPULSE; }
	} parts;
} gpulse_t;
gpulse_t _gpulse;

typedef union {
	uint8_t pack;
	struct{
		/* Gesture Dimension Select. Selects which gesture photodiode pairs are
		enabled to gather results during gesture.
		*/
		uint8_t GDIMS : 2;

		//uint8_t get() { return GDIMS; }
	} parts;
} gconf3_t;
gconf3_t _gconf3;

typedef union {
	uint8_t pack;
	struct{
		/* Gesture Mode. Reading this bit reports if the gesture state machine is
		actively running, 1 = Gesture, 0= ALS, Proximity, Color. Writing a 1 to this
		bit causes immediate entry in to the gesture state machine (as if GPENTH had
		been exceeded). Writing a 0 to this bit causes exit of gesture when current
		analog conversion has finished (as if GEXTH had been exceeded).
		*/
		uint8_t GMODE : 1;

		/* Gesture interrupt enable. Gesture Interrupt Enable. When asserted, all
		gesture related interrupts are unmasked.
		*/
		uint8_t GIEN : 2;

		/*uint8_t get() { return (GIEN << 1) | GMODE; }
		void set(uint8_t data) {
		  GIEN = (data >> 1) & 0x01;
		  GMODE = data & 0x01;
		}*/
	} parts;
} gconf4_t;
gconf4_t _gconf4;

typedef union {
	uint8_t pack;
	struct{
		/* Gesture FIFO Data. GVALID bit is sent when GFLVL becomes greater than
		GFIFOTH (i.e. FIFO has enough data to set GINT). GFIFOD is reset when GMODE
		= 0 and the GFLVL=0 (i.e. All FIFO data has been read).
		*/
		uint8_t GVALID : 1;

		/* Gesture FIFO Overflow. A setting of 1 indicates that the FIFO has filled
		to capacity and that new gesture detector data has been lost.
		*/
		uint8_t GFOV : 1;

		/*void set(uint8_t data) {
		  GFOV = (data >> 1) & 0x01;
		  GVALID = data & 0x01;
		}*/
	} parts;
} gstatus_t;
gstatus_t _gstatus;

#define true      TRUE
#define false    FALSE

/*----------------------------------------------------------------------------------------*/
static void write8(byte reg, byte value);
static uint8_t read8(byte reg);
static uint16_t read16(uint8_t reg);
static uint16_t read16R(uint8_t reg);
static uint8_t readbuf(uint8_t reg, uint8_t *buf, uint8_t num);
static void writebuf(uint8_t reg, uint8_t *buf, uint8_t num);

static void delay(word millisecs)
{
	t_delay del;
	delayStart(del, millisecs * 1000);
	while(!delayFinished(del)){}
}

static unsigned long int millis()
{
	return time()/1000;
}

/*----------------------------------------------------------------------------------------*/

/*!
 *  @brief  Enables the device
 *          Disables the device (putting it in lower power sleep mode)
 *  @param  en
 *          Enable (True/False)
 */
void APDS9960enable(char en) {
	_enable.parts.PON = (en != 0);
	write8(APDS9960_ENABLE, _enable.pack);
}

/*!
 *  @brief  Initializes I2C and configures the sensor
 *  @param  iTimeMS
 *          Integration time
 *  @param  aGain
 *          Gain
 *  @param  addr
 *          I2C address
 *  @return True if initialization was successful, otherwise false.
 */
char APDS9960init() {
	/* Make sure we're actually connected */
	uint8_t x = read8(APDS9960_ID);
	if (x != 0xAB) {
		return false;
	}

	/* Set default integration time and gain */
	APDS9960setADCIntegrationTime(10);
	APDS9960setADCGain(APDS9960_AGAIN_4X);

	// disable everything to start
	APDS9960enableGesture(false);
	APDS9960enableProximity(false);
	APDS9960enableColor(false);

	_enable.parts.AIEN = 0; //disableColorInterrupt();
	_enable.parts.PIEN = 0; //disableProximityInterrupt();
	write8(APDS9960_ENABLE, _enable.pack);

	writebuf(APDS9960_AICLEAR, NULL, 0); // clear interrupts

	/* Note: by default, the device is in power down mode on bootup */
	APDS9960enable(false);
	delay(10);
	APDS9960enable(true);
	delay(10);

	// default to all gesture dimensions
	APDS9960setGestureDimensions(APDS9960_DIMENSIONS_ALL);
	APDS9960setGestureFIFOThreshold(APDS9960_GFIFO_4);
	APDS9960setGestureGain(APDS9960_GGAIN_4);
	APDS9960setGestureProximityThreshold(50);
	APDS9960resetCounts();

	_gpulse.parts.GPLEN = APDS9960_GPULSE_32US;
	_gpulse.parts.GPULSE = 9; // 10 pulses
	write8(APDS9960_GPULSE, _gpulse.pack);

	return true;
}

/*!
 *  @brief  Sets the integration time for the ADC of the APDS9960, in millis
 *  @param  iTimeMS
 *          Integration time
 */
void APDS9960setADCIntegrationTime(uint16_t iTimeMS) {
	int temp;

	// convert ms into 2.78ms increments
	//  temp = 256 - (iTimeMS / 2.78);
	temp = 256 - ((iTimeMS * 32) / 89);
	if (temp > 255)
	temp = 255;
	if (temp < 0)
	temp = 0;

	/* Update the timing register */
	write8(APDS9960_ATIME, (uint8_t)temp);
}


/*!
 *  @brief  Adjusts the color/ALS gain on the APDS9960 (adjusts the sensitivity
 *          to light)
 *  @param  aGain
 *          Gain
 */
void APDS9960setADCGain(apds9960AGain_t aGain) {
	_control.parts.AGAIN = aGain;

	/* Update the timing register */
	write8(APDS9960_CONTROL, _control.pack);
}

/*!
 *  @brief  Adjusts the Proximity gain on the APDS9960
 *  @param  pGain
 *          Gain
 */
void APDS9960setProxGain(apds9960PGain_t pGain) {
	_control.parts.PGAIN = pGain;

	/* Update the timing register */
	write8(APDS9960_CONTROL, _control.pack);
}

/*!
 *  @brief  Sets number of proxmity pulses
 *  @param  pLen
 *          Pulse Length
 *  @param  pulses
 *          Number of pulses
 */
void APDS9960setProxPulse(apds9960PPulseLen_t pLen, uint8_t pulses) {
	if (pulses < 1)
	pulses = 1;
	if (pulses > 64)
	pulses = 64;
	pulses--;

	_ppulse.parts.PPLEN = pLen;
	_ppulse.parts.PPULSE = pulses;

	write8(APDS9960_PPULSE, _ppulse.pack);
}

/*!
 *  @brief  Enable proximity readings on APDS9960
 *  @param  en
 *          Enable (True/False)
 */
void APDS9960enableProximity(char en) {
	_enable.parts.PEN = (en != 0);

	write8(APDS9960_ENABLE, _enable.pack);
}



/*!
 *  @brief  Read proximity data
 *  @return Proximity
 */
uint8_t APDS9960readProximity() { return read8(APDS9960_PDATA); }

/*!
 *  @brief  Returns validity status of a gesture
 *  @return Status (True/False)
 */
char APDS9960gestureValid() {
	_gstatus.pack = read8(APDS9960_GSTATUS);
	return _gstatus.parts.GVALID;
}

/*!
 *  @brief  Sets gesture dimensions
 *  @param  dims
 *          Dimensions (APDS9960_DIMENSIONS_ALL, APDS9960_DIMENSIONS_UP_DOWM,
 *          APDS9960_DIMENSIONS_UP_DOWN, APGS9960_DIMENSIONS_LEFT_RIGHT)
 */
void APDS9960setGestureDimensions(uint8_t dims) {
	_gconf3.parts.GDIMS = dims;
	write8(APDS9960_GCONF3, _gconf3.pack);
}

/*!
 *  @brief  Sets gesture FIFO Threshold
 *  @param  thresh
 *          Threshold (APDS9960_GFIFO_1, APDS9960_GFIFO_4, APDS9960_GFIFO_8,
 *          APDS9960_GFIFO_16)
 */
void APDS9960setGestureFIFOThreshold(uint8_t thresh) {
	_gconf1.parts.GFIFOTH = thresh;
	write8(APDS9960_GCONF1, _gconf1.pack);
}

/*!
 *  @brief  Sets gesture sensor gain
 *  @param  gain
 *          Gain (APDS9960_GAIN_1, APDS9960_GAIN_2, APDS9960_GAIN_4,
 *          APDS9960_GAIN_8)
 */
void APDS9960setGestureGain(uint8_t gain) {
	_gconf2.parts.GGAIN = gain;
	write8(APDS9960_GCONF2, _gconf2.pack);
}

/*!
 *  @brief  Sets gesture sensor threshold
 *  @param  thresh
 *          Threshold
 */
void APDS9960setGestureProximityThreshold(uint8_t thresh) {
	write8(APDS9960_GPENTH, thresh);
}

/*!
 *  @brief  Sets gesture sensor offset
 *  @param  offset_up
 *          Up offset
 *  @param  offset_down
 *          Down offset
 *  @param  offset_left
 *          Left offset
 *  @param  offset_right
 *          Right offset
 */
void APDS9960setGestureOffset(uint8_t offset_up, uint8_t offset_down,
                                         uint8_t offset_left,
                                         uint8_t offset_right) {
	write8(APDS9960_GOFFSET_U, offset_up);
	write8(APDS9960_GOFFSET_D, offset_down);
	write8(APDS9960_GOFFSET_L, offset_left);
	write8(APDS9960_GOFFSET_R, offset_right);
}

/*!
 *  @brief  Enable gesture readings on APDS9960
 *  @param  en
 *          Enable (True/False)
 */
void APDS9960enableGesture(char en) {
	if (!en) {
		_gconf4.parts.GMODE = 0;
		write8(APDS9960_GCONF4, _gconf4.pack);
	}
	_enable.parts.GEN = (en != 0);
	write8(APDS9960_ENABLE, _enable.pack);
	APDS9960resetCounts();
}

/*!
 *  @brief  Resets gesture counts
 */
void APDS9960resetCounts() {
	APDS9960gestCnt = 0;
	APDS9960UCount = 0;
	APDS9960DCount = 0;
	APDS9960LCount = 0;
	APDS9960RCount = 0;
}

/*!
 *  @brief  Reads gesture
 *  @return Received gesture (APDS9960_DOWN APDS9960_UP, APDS9960_LEFT
 *          APDS9960_RIGHT)
 */
uint8_t APDS9960readGesture() {
	uint8_t toRead;
	uint8_t buf[256];
	//uint8_t fraisebuf[8];
	unsigned long t = 0;
	uint8_t gestureReceived;
	int up_down_diff = 0;
	int left_right_diff = 0;
	
	/*while*/if (1) {
		up_down_diff = 0;
		left_right_diff = 0;
		gestureReceived = 0;
		if (!APDS9960gestureValid()) return 0;

		//delay(30);
		toRead = read8(APDS9960_GFLVL);
		readbuf(APDS9960_GFIFO_U, buf, toRead);

		/*fraisebuf[0] = 'B';
		fraisebuf[1] = 11;
		fraisebuf[2] = buf[0];
		fraisebuf[3] = buf[1];
		fraisebuf[4] = buf[2];
		fraisebuf[5] = buf[3];
		fraisebuf[6] = toRead;
		fraisebuf[7] = '\n';
		fraiseSend(fraisebuf, 8);
		return 0;*/

		if (abs((int)buf[0] - (int)buf[1]) > 13)
			up_down_diff += (int)buf[0] - (int)buf[1];

		if (abs((int)buf[2] - (int)buf[3]) > 13)
			left_right_diff += (int)buf[2] - (int)buf[3];

		if (up_down_diff != 0) {
			if (up_down_diff < 0) {
				if (APDS9960DCount > 0) {
					gestureReceived = APDS9960_UP;
				} else
					APDS9960UCount++;
			} else if (up_down_diff > 0) {
				if (APDS9960UCount > 0) {
					gestureReceived = APDS9960_DOWN;
				} else
					APDS9960DCount++;
			}
		}

		if (left_right_diff != 0) {
			if (left_right_diff < 0) {
				if (APDS9960RCount > 0) {
					gestureReceived = APDS9960_LEFT;
				} else
					APDS9960LCount++;
			} else if (left_right_diff > 0) {
				if (APDS9960LCount > 0) {
					gestureReceived = APDS9960_RIGHT;
				} else
					APDS9960RCount++;
			}
		}

		if (up_down_diff != 0 || left_right_diff != 0)
			t = millis();

		if (gestureReceived /*|| millis() - t > 300*/) {
			APDS9960resetCounts();
			return gestureReceived;
		}
		return 0;
	}
}

/*!
 *  @brief  Set LED brightness for proximity/gesture
 *  @param  drive
 *          LED Drive
 *  @param  boost
 *          LED Boost
 */
void APDS9960setLED(apds9960LedDrive_t drive,
                               apds9960LedBoost_t boost) {
  // set BOOST
	_config2.parts.LED_BOOST = boost;
	write8(APDS9960_CONFIG2, _config2.pack);

	_control.parts.LDRIVE = drive;
	write8(APDS9960_CONTROL, _control.pack);
}

/*!
 *  @brief  Enable proximity readings on APDS9960
 *  @param  en
 *          Enable (True/False)
 */
void APDS9960enableColor(char en) {
	_enable.parts.AEN = (en != 0);
	write8(APDS9960_ENABLE, _enable.pack);
}

/*!
 *  @brief  Returns status of color data
 *  @return True if color data ready, False otherwise
 */
char APDS9960colorDataReady() {
	_status.pack = read8(APDS9960_STATUS);
	return _status.parts.AVALID;
}

/*!
 *  @brief  Reads the raw red, green, blue and clear channel values
 *  @param  *r
 *          Red value
 *  @param  *g
 *          Green value
 *  @param  *b
 *          Blue value
 *  @param  *c
 *          Clear channel value
 */
void APDS9960getColorData(uint16_t *r, uint16_t *g, uint16_t *b,
                                     uint16_t *c) {
	*c = read16R(APDS9960_CDATAL);
	*r = read16R(APDS9960_RDATAL);
	*g = read16R(APDS9960_GDATAL);
	*b = read16R(APDS9960_BDATAL);
}


/*!
 *  @brief  Writes specified value to given register
 *  @param  reg
 *          Register to write to
 *  @param  value
 *          Value to write
 */
static void write8(byte reg, byte value) {
	writebuf(reg, &value, 1);
}

/*!
 *  @brief  Reads 8 bits from specified register
 *  @param  reg
 *          Register to write to
 *  @return Value in register
 */
static uint8_t read8(byte reg) {
	uint8_t ret;
	//readbuf(reg, &ret, 1);
	i2cm_begin(APDS9960_ADDRESS, 0);
	i2cm_writechar(reg);
	i2cm_stop();

	i2cm_begin(APDS9960_ADDRESS, 1);
	ret = i2cm_readchar();
	i2cm_stop();

	return ret;
}

/*!
 *  @brief  Reads 16 bytes from specified register
 *  @param  reg
 *          Register to write to
 *  @return Value in register
 */
static uint16_t read16(uint8_t reg) {
	uint8_t ret[2];
	readbuf(reg, ret, 2);

	return (ret[0] << 8) | ret[1];
}

/*!
 *  @brief  Reads 16 bytes from specified register
 *  @param  reg
 *          Register to write to
 *  @return Value in register
 */
static uint16_t read16R(uint8_t reg) {
	uint8_t ret[2];
	//readbuf(reg, ret, 2);
	i2cm_begin(APDS9960_ADDRESS, 0);
	i2cm_writechar(reg);
	i2cm_stop();

	i2cm_begin(APDS9960_ADDRESS, 1);
	ret[0] = i2cm_readchar();
	i2cm_ack();
	ret[1] = i2cm_readchar();
	i2cm_stop();

	return (ret[1] << 8) | ret[0];
}


/*!
 *  @brief  Reads num bytes from specified register into a given buffer
 *  @param  reg
 *          Register
 *  @param  *buf
 *          Buffer
 *  @param  num
 *          Number of bytes
 *  @return Position after reading
 */
static uint8_t readbuf(uint8_t reg, uint8_t *buf, uint8_t num) {
	uint8_t rtn = 0;
	i2cm_begin(APDS9960_ADDRESS, 0);
	i2cm_writechar(reg);
	i2cm_stop();

	i2cm_begin(APDS9960_ADDRESS, 1);
	while(num--) {
		*buf++ = i2cm_readchar();
		if(num) i2cm_ack();
		rtn++;
	}
	//rtn = i2cm_readstr(buf, (unsigned char)num);
	i2cm_stop();
	return rtn;
}

/*!
 *  @brief  Writes num bytes from specified buffer into a given register
 *  @param  reg
 *          Register
 *  @param  *buf
 *          Buffer
 *  @param  num
 *          Number of bytes
 */
static void writebuf(uint8_t reg, uint8_t *buf, uint8_t num) {
	i2cm_begin((uint8_t)APDS9960_ADDRESS, 0);
	i2cm_writechar(reg);
	while( num-- ) {
		i2cm_writechar(*buf++);
	}
	i2cm_stop();
}

