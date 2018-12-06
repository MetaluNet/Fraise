/*********************************************************************
 *
 *                RFM69 module for Fraise
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  nov 2018     Original.
 ********************************************************************/
// adapted from LowPowerLab RFM69 library for Arduino
/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/
#ifndef RFM69_h
#define RFM69_h

//#define RF69_SPI_CS K1
//#define RF69_IRQ_PIN K9
//#define RF69_SPI 1/2

// available frequency bands
#define RF69_315MHZ            31 // non trivial values to avoid misconfiguration
#define RF69_433MHZ            43
#define RF69_868MHZ            86
#define RF69_915MHZ            91
#define RF69_BROADCAST_ADDR    255

#define RF69_MAX_DATA_LEN       61 // to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)


char RFM69init(char isRFM69HW, uint8_t freqBand, uint8_t nodeID, uint8_t networkID); // return 1 on success, 0 on error.

void RFM69setAddress(uint8_t addr);
void RFM69setNetwork(uint8_t networkID);
char RFM69canSend();
void RFM69send(uint8_t toAddress, const void* buffer, uint8_t bufferSize, char requestACK/*=false*/);
char RFM69sendWithRetry(uint8_t toAddress, const void* buffer, uint8_t bufferSize, 
	uint8_t retries/*=2*/, uint8_t retryWaitTime/*=40*/); // 40ms roundtrip req for 61byte packets
char RFM69receiveDone();
char RFM69ACKReceived(uint8_t fromNodeID);
char RFM69ACKRequested();
void RFM69sendACK(const void* buffer /*= ""*/, uint8_t bufferSize/*=0*/);
void RFM69encrypt(const char* key);
void RFM69promiscuous(char onOff/*=true*/);
void RFM69sleep();
uint8_t RFM69readTemperature(uint8_t calFactor/*=0*/); // get CMOS temperature (8bit)
void RFM69rcCalibration(); // calibrate the internal RC oscillator for use in wide temperature variations - see datasheet section [4.3.5. RC 

void RFM69lowInterrupt();

#ifndef RFMEXTERN
#define RFMEXTERN extern
#endif

RFMEXTERN volatile uint8_t RFM69DATA[RF69_MAX_DATA_LEN]; // recvd buf, including header & crc bytes
RFMEXTERN volatile uint8_t RFM69DATALEN;
RFMEXTERN volatile uint8_t RFM69SENDERID;
RFMEXTERN volatile uint8_t RFM69TARGETID; // should match _address
RFMEXTERN volatile int16_t RFM69RSSI; // most accurate RSSI during reception (closest to the reception). RSSI of last packet.


#endif

