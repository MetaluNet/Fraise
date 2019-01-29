/*********************************************************************
 *
 *                APDS9960 library for Fraise pic18f device
 *				
 *				
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  jan 2019     Original.
 ********************************************************************/
/*  from Adafruit Arduino library:
 *
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
#ifndef _APDS9960_H_
#define _APDS9960_H_

#define APDS9960_ADDRESS (0x39) /**< I2C Address */

/** I2C Registers */
enum {
  APDS9960_RAM = 0x00,
  APDS9960_ENABLE = 0x80,
  APDS9960_ATIME = 0x81,
  APDS9960_WTIME = 0x83,
  APDS9960_AILTIL = 0x84,
  APDS9960_AILTH = 0x85,
  APDS9960_AIHTL = 0x86,
  APDS9960_AIHTH = 0x87,
  APDS9960_PILT = 0x89,
  APDS9960_PIHT = 0x8B,
  APDS9960_PERS = 0x8C,
  APDS9960_CONFIG1 = 0x8D,
  APDS9960_PPULSE = 0x8E,
  APDS9960_CONTROL = 0x8F,
  APDS9960_CONFIG2 = 0x90,
  APDS9960_ID = 0x92,
  APDS9960_STATUS = 0x93,
  APDS9960_CDATAL = 0x94,
  APDS9960_CDATAH = 0x95,
  APDS9960_RDATAL = 0x96,
  APDS9960_RDATAH = 0x97,
  APDS9960_GDATAL = 0x98,
  APDS9960_GDATAH = 0x99,
  APDS9960_BDATAL = 0x9A,
  APDS9960_BDATAH = 0x9B,
  APDS9960_PDATA = 0x9C,
  APDS9960_POFFSET_UR = 0x9D,
  APDS9960_POFFSET_DL = 0x9E,
  APDS9960_CONFIG3 = 0x9F,
  APDS9960_GPENTH = 0xA0,
  APDS9960_GEXTH = 0xA1,
  APDS9960_GCONF1 = 0xA2,
  APDS9960_GCONF2 = 0xA3,
  APDS9960_GOFFSET_U = 0xA4,
  APDS9960_GOFFSET_D = 0xA5,
  APDS9960_GOFFSET_L = 0xA7,
  APDS9960_GOFFSET_R = 0xA9,
  APDS9960_GPULSE = 0xA6,
  APDS9960_GCONF3 = 0xAA,
  APDS9960_GCONF4 = 0xAB,
  APDS9960_GFLVL = 0xAE,
  APDS9960_GSTATUS = 0xAF,
  APDS9960_IFORCE = 0xE4,
  APDS9960_PICLEAR = 0xE5,
  APDS9960_CICLEAR = 0xE6,
  APDS9960_AICLEAR = 0xE7,
  APDS9960_GFIFO_U = 0xFC,
  APDS9960_GFIFO_D = 0xFD,
  APDS9960_GFIFO_L = 0xFE,
  APDS9960_GFIFO_R = 0xFF,
};

/** ADC gain settings */
typedef enum {
  APDS9960_AGAIN_1X = 0x00,  /**< No gain */
  APDS9960_AGAIN_4X = 0x01,  /**< 2x gain */
  APDS9960_AGAIN_16X = 0x02, /**< 16x gain */
  APDS9960_AGAIN_64X = 0x03  /**< 64x gain */
} apds9960AGain_t;

/** Proxmity gain settings */
typedef enum {
  APDS9960_PGAIN_1X = 0x00, /**< 1x gain */
  APDS9960_PGAIN_2X = 0x04, /**< 2x gain */
  APDS9960_PGAIN_4X = 0x08, /**< 4x gain */
  APDS9960_PGAIN_8X = 0x0C  /**< 8x gain */
} apds9960PGain_t;

/** Pulse length settings */
typedef enum {
  APDS9960_PPULSELEN_4US = 0x00,  /**< 4uS */
  APDS9960_PPULSELEN_8US = 0x40,  /**< 8uS */
  APDS9960_PPULSELEN_16US = 0x80, /**< 16uS */
  APDS9960_PPULSELEN_32US = 0xC0  /**< 32uS */
} apds9960PPulseLen_t;

/** LED drive settings */
typedef enum {
  APDS9960_LEDDRIVE_100MA = 0x00, /**< 100mA */
  APDS9960_LEDDRIVE_50MA = 0x40,  /**< 50mA */
  APDS9960_LEDDRIVE_25MA = 0x80,  /**< 25mA */
  APDS9960_LEDDRIVE_12MA = 0xC0   /**< 12.5mA */
} apds9960LedDrive_t;

/** LED boost settings */
typedef enum {
  APDS9960_LEDBOOST_100PCNT = 0x00, /**< 100% */
  APDS9960_LEDBOOST_150PCNT = 0x10, /**< 150% */
  APDS9960_LEDBOOST_200PCNT = 0x20, /**< 200% */
  APDS9960_LEDBOOST_300PCNT = 0x30  /**< 300% */
} apds9960LedBoost_t;

/** Dimensions */
enum {
  APDS9960_DIMENSIONS_ALL = 0x00,        // All dimensions
  APDS9960_DIMENSIONS_UP_DOWN = 0x01,    // Up/Down dimensions
  APGS9960_DIMENSIONS_LEFT_RIGHT = 0x02, // Left/Right dimensions
};

/** FIFO Interrupts */
enum {
  APDS9960_GFIFO_1 = 0x00,  // Generate interrupt after 1 dataset in FIFO
  APDS9960_GFIFO_4 = 0x01,  // Generate interrupt after 2 datasets in FIFO
  APDS9960_GFIFO_8 = 0x02,  // Generate interrupt after 3 datasets in FIFO
  APDS9960_GFIFO_16 = 0x03, // Generate interrupt after 4 datasets in FIFO
};

/** Gesture Gain */
enum {
  APDS9960_GGAIN_1 = 0x00, // Gain 1x
  APDS9960_GGAIN_2 = 0x01, // Gain 2x
  APDS9960_GGAIN_4 = 0x02, // Gain 4x
  APDS9960_GGAIN_8 = 0x03, // Gain 8x
};

/** Pulse Lenghts */
enum {
  APDS9960_GPULSE_4US = 0x00,  // Pulse 4us
  APDS9960_GPULSE_8US = 0x01,  // Pulse 8us
  APDS9960_GPULSE_16US = 0x02, // Pulse 16us
  APDS9960_GPULSE_32US = 0x03, // Pulse 32us
};

#define APDS9960_UP 0x01    /**< Gesture Up */
#define APDS9960_DOWN 0x02  /**< Gesture Down */
#define APDS9960_LEFT 0x03  /**< Gesture Left */
#define APDS9960_RIGHT 0x04 /**< Gesture Right */

char APDS9960init();
void APDS9960setADCIntegrationTime(uint16_t iTimeMS);
void APDS9960setADCGain(apds9960AGain_t gain);
void APDS9960setLED(apds9960LedDrive_t drive, apds9960LedBoost_t boost);
void APDS9960enableProximity(char en);
void APDS9960setProxGain(apds9960PGain_t gain);
void APDS9960setProxPulse(apds9960PPulseLen_t pLen, uint8_t pulses);
uint8_t APDS9960readProximity();
void APDS9960enableGesture(char en);
char APDS9960gestureValid();
void APDS9960setGestureDimensions(uint8_t dims);
void APDS9960setGestureFIFOThreshold(uint8_t thresh);
void APDS9960setGestureGain(uint8_t gain);
void APDS9960setGestureProximityThreshold(uint8_t thresh);
void APDS9960setGestureOffset(uint8_t offset_up, uint8_t offset_down,
                    uint8_t offset_left, uint8_t offset_right);
uint8_t APDS9960readGesture();
void APDS9960resetCounts();

// light & color
void APDS9960enableColor(char en);
char APDS9960colorDataReady();
void APDS9960getColorData(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c);

// turn on/off elements
void APDS9960enable(char en);

// Gesture
extern uint8_t APDS9960gestCnt;

extern uint8_t APDS9960UCount;
extern uint8_t APDS9960DCount;

extern uint8_t APDS9960LCount;
extern uint8_t APDS9960RCount;

#endif
