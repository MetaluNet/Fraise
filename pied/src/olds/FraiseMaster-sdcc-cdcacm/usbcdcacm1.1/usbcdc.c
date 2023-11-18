/*
 File: usbcdc.h
 
 Copyright (c) 2010,2013 Kustaa Nyholm / SpareTimeLabs
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
 
 Version 1.1     Compatible with SDCC 3.x
 Respond to GET_LINE_CODING to work with overzealous
 Windows software (like Hyperterminal)
 */


#include "pic18f2550.h"
#include "usbcdc.h"
#include "usbpic_defs.h"
#include "usbcdc_defs.h"
#include "usb_defs.h"

typedef __code unsigned char* codePtr;
typedef __data unsigned char* dataPtr;

// Device and configuration descriptors

typedef struct {
	USB_CFG_DSC cd01;
	USB_INTF_DSC i01a00;
	USB_CDC_HEADER_FN_DSC cdc_header_fn_i01a00;
	USB_CDC_CALL_MGT_FN_DSC cdc_call_mgt_fn_i01a00;
	USB_CDC_ACM_FN_DSC cdc_acm_fn_i01a00;
	USB_CDC_UNION_FN_DSC cdc_union_fn_i01a00;
	USB_EP_DSC ep02i_i01a00;
	USB_INTF_DSC i02a00;
	USB_EP_DSC ep03o_i02a00;
	USB_EP_DSC ep03i_i02a00;
} config_struct;

// Global variables
unsigned char usbcdc_device_state;

static unsigned char device_address;
static unsigned char current_configuration; // 0 or 1
static unsigned char idx; // loop counter for data transfers loops
static unsigned char control_stage; // Holds the current stage in a control transfer
static unsigned char request_handled; // Set to 1 if request was understood and processed.
static dataPtr data_ptr; // Data to host from RAM
static codePtr code_ptr; // Data to host from FLASH
static dataPtr in_ptr; // Data from the host
static unsigned char dlen; // Number of unsigned chars of data

// See USB spec chapter 5
#define SETUP_STAGE    0
#define DATA_OUT_STAGE 1
#define DATA_IN_STAGE  2
#define STATUS_STAGE   3

__code unsigned char device_descriptor[] = { //
    0x12, 0x01, // bLength, bDescriptorType
    0x00, 0x02, // bcdUSB lsb, bcdUSB msb
    0x02, 0x00, // bDeviceClass, bDeviceSubClass
    0x00, E0SZ, // bDeviceProtocl, bMaxPacketSize
    0x08, 0x04, // idVendor lsb, idVendor msb
    0x0A, 0x00, // idProduct lsb, idProduct msb
    0x00, 0x01, // bcdDevice lsb, bcdDevice msb
    0x01, 0x00, // iManufacturer, iProduct
    0x00, 0x01 // iSerialNumber (none), bNumConfigurations*/
    
};
__code unsigned char device_qualifier_descriptor[] = { //
    0x0A, 0x06, // bLength, bDescriptorType
    0x00, 0x02, // bcdUSB lsb, bcdUSB msb
    0x02, 0x00, // bDeviceClass, bDeviceSubClass
    0x00, E0SZ, // bDeviceProtocl, bMaxPacketSize
    0x01, 0x00 // iSerialNumber, bNumConfigurations*/
};

static __code char const_values_0x00_0x00[] = { 0, 0 };
static __code char const_values_0x00_0x01[] = { 0, 1 };
static __code char const_values_0x01_0x00[] = { 1, 0 };
static __code char const_values_status[] = { (USBCDC_SELF_POWERED<<0), 0 }; // first byte tells self powered and remote wakeup status

__code config_struct
config_descriptor = {
    {
        // Configuration descriptor
        sizeof(USB_CFG_DSC), DSC_CFG, // bLength, bDescriptorType (Configuration)
        sizeof(config_struct), // wTotalLength
        0x02, 0x01, // bNumInterfaces, bConfigurationValue
        0x00, 0x80 | (USBCDC_SELF_POWERED<<6),//_DEFAULT, // iConfiguration, bmAttributes ()
        USBCDC_MAXPOWER / 2, // bMaxPower
    },
    /* Interface Descriptor */
    { sizeof(USB_INTF_DSC), // Size of this descriptor in unsigned chars
        DSC_INTF, // INTERFACE descriptor type
        0, // Interface Number
        0, // Alternate Setting Number
        1, // Number of endpoints in this intf
        COMM_INTF, // Class __code
        ABSTRACT_CONTROL_MODEL, // Subclass __code
        V25TER, // Protocol __code
        0 // Interface string index
    },
    /* CDC Class-Specific Descriptors */
    { sizeof(USB_CDC_HEADER_FN_DSC), CS_INTERFACE,DSC_FN_HEADER,
        0x0110 },
    { sizeof(USB_CDC_CALL_MGT_FN_DSC), CS_INTERFACE,
        DSC_FN_CALL_MGT, 0x01, CDC_DATA_INTF_ID},
    { sizeof(USB_CDC_ACM_FN_DSC), CS_INTERFACE,
        DSC_FN_ACM, 0x02 },
    { sizeof(USB_CDC_UNION_FN_DSC), CS_INTERFACE,
        DSC_FN_UNION, CDC_COMM_INTF_ID, CDC_DATA_INTF_ID },
    /* Endpoint Descriptor */
    {//notification endpoint
        sizeof(USB_EP_DSC), DSC_EP,_EP01_IN, _INT,CDC_INT_EP_SIZE, 0x0A, },
    /* Interface Descriptor */
    { sizeof(USB_INTF_DSC), // Size of this descriptor in unsigned chars
        DSC_INTF, // INTERFACE descriptor type
        1, // Interface Number
        0, // Alternate Setting Number
        2, // Number of endpoints in this intf
        DATA_INTF, // Class __code
        0, // Subclass __code
        NO_PROTOCOL, // Protocol __code
        2, // Interface string index
    },
    /* Endpoint Descriptors */
    { sizeof(USB_EP_DSC), DSC_EP,_EP02_OUT,
        _BULK,USBCDC_BUFFER_LEN, 0x00, }, //
    { sizeof(USB_EP_DSC), DSC_EP,_EP02_IN, _BULK,USBCDC_BUFFER_LEN,
        0x00 }, //
};

__code unsigned char string_descriptor0[] = { // available languages  descriptor
    0x04, STRING_DESCRIPTOR, //
    0x09, 0x04, //
};

__code unsigned char string_descriptor1[] = { //
    0x0E, STRING_DESCRIPTOR, // bLength, bDscType
    'T', 0x00, //
    'e', 0x00, //
    's', 0x00, //
    't', 0x00, //
    'i', 0x00, //
    '!', 0x00, //
};
__code unsigned char string_descriptor2[] = { //
    0x20, STRING_DESCRIPTOR, //
    'U', 0x00, //
    'S', 0x00, //
    'B', 0x00, //
    ' ', 0x00, //
    'G', 0x00, //
    'e', 0x00, //
    'n', 0x00, //
    'e', 0x00, //
    'r', 0x00, //
    'i', 0x00, //
    'c', 0x00, //
    ' ', 0x00, //
    'C', 0x00, //
    'D', 0x00, //
    'C', 0x00, //
};

// Put endpoint 0 buffers into dual port RAM
// Put USB I/O buffers into dual port RAM.
//#pragma udata usbram5 setup_packet control_transfer_buffer cdc_rx_buffer cdc_tx_buffer cdcint_buffer
//#pragma udata usb_data cdc_tx_buffer cdcint_buffer
//#pragma udata usb_data2 setup_packet control_transfer_buffer cdc_rx_buffer

static volatile setup_packet_struct setup_packet;
static volatile unsigned char control_transfer_buffer[E0SZ];

// CDC buffers
static unsigned char tx_len = 0;
static unsigned char rx_idx = 0;


//static volatile unsigned char cdcint_buffer[USBCDC_BUFFER_LEN];
volatile unsigned char cdc_rx_buffer[USBCDC_BUFFER_LEN];
volatile unsigned char cdc_tx_buffer[USBCDC_BUFFER_LEN];
static volatile unsigned char cdcint_buffer[USBCDC_BUFFER_LEN];
static __code char cdc_line_coding[7] = {9600&0xFF,9600>>8,0,0,0,0,8};
//static __code char cdc_line_coding[7] = {9600&0xFF,9600>>8,0,0,1,2,8};

void usbcdc_putchar(char c)__wparam
{
	while (usbcdc_wr_busy());
	cdc_tx_buffer[tx_len++]=c;
	if (tx_len>=sizeof(cdc_tx_buffer)) {
		usbcdc_flush();
	}
}

char usbcdc_wr_busy() {
	return (ep2_i.STAT & UOWN)!=0;
}

unsigned char usbcdc_rd_ready() {
	if (ep2_o.STAT & UOWN)
		return 0;
	if (rx_idx >= ep2_o.CNT) {
		usbcdc_read();
		return 0;
	}
	return 1;
}

void usbcdc_write(unsigned char len)__wparam
{
	if (len> 0) {
		ep2_i.CNT = len;
		if (ep2_i.STAT & DTS)
            ep2_i.STAT = UOWN | DTSEN;
		else
            ep2_i.STAT = UOWN | DTS | DTSEN;
	}
}

void usbcdc_flush() {
	usbcdc_write(tx_len);
	tx_len = 0;
    
}

void usbcdc_read() {
	rx_idx=0;
	ep2_o.CNT = sizeof(cdc_rx_buffer);
	if (ep2_o.STAT & DTS)
		ep2_o.STAT = UOWN | DTSEN;
	else
		ep2_o.STAT = UOWN | DTS | DTSEN;
}


char usbcdc_getchar() {
	char c;
	while (!usbcdc_rd_ready());
    
	c = cdc_rx_buffer[rx_idx++];
	if (rx_idx>=ep2_o.CNT) {
		usbcdc_read();
    }
	return c;
}

static void get_descriptor(void) {
	if (setup_packet.bmrequesttype == 0x80) {
		unsigned char descriptorType = setup_packet.wvalue1;
		unsigned char descriptorIndex = setup_packet.wvalue0;
        
		if (descriptorType == DEVICE_DESCRIPTOR) {
			request_handled = 1;
			code_ptr = (codePtr) &device_descriptor;
			dlen = *code_ptr;//DEVICE_DESCRIPTOR_SIZE;
		} else if (descriptorType == QUALIFIER_DESCRIPTOR) {
			request_handled = 1;
			code_ptr = (codePtr) &device_qualifier_descriptor;
			dlen = sizeof(device_qualifier_descriptor);
		} else if (descriptorType == CONFIGURATION_DESCRIPTOR) {
			request_handled = 1;
            
			code_ptr = (codePtr) &config_descriptor;
			dlen = *(code_ptr + 2);
            
		} else if (descriptorType == STRING_DESCRIPTOR) {
			request_handled = 1;
			if (descriptorIndex == 0) {
				code_ptr = (codePtr) &string_descriptor0;
			} else if (descriptorIndex == 1) {
				code_ptr = (codePtr) &string_descriptor1;
			} else {
				code_ptr = (codePtr) &string_descriptor2;
			}
			dlen = *code_ptr;
		}
	}
}

// Process GET_STATUS
static void get_status(void) {
	// Mask off the Recipient bits
	unsigned char recipient = setup_packet.bmrequesttype & 0x1F;
    
	// See where the request goes
	if (recipient == 0x00) {
		// Device
		request_handled = 1;
		code_ptr = (codePtr) &const_values_status; // hard __code device status
        
	} else if (recipient == 0x01) {
		// Interface
		code_ptr = (codePtr) &const_values_0x00_0x00;
		request_handled = 1;
	} else if (recipient == 0x02) {
		// Endpoint
		unsigned char endpointNum = setup_packet.windex0 & 0x0F;
		unsigned char endpointDir = setup_packet.windex0 & 0x80;
		request_handled = 1;
		// Endpoint descriptors are 8 unsigned chars long, with each in and out taking 4 unsigned chars
		// within the endpoint. (See PIC datasheet.)
		in_ptr = (dataPtr) &ep0_o + (endpointNum * 8);
		if (endpointDir)
			in_ptr += 4;
		if (*in_ptr & BSTALL)
			code_ptr = (codePtr) &const_values_0x01_0x00;
	}
	if (request_handled) {
		dlen = 2;
	}
}

// Process SET_FEATURE and CLEAR_FEATURE
static void set_feature(void) {
	unsigned char recipient = setup_packet.bmrequesttype & 0x1F;
	unsigned char feature = setup_packet.wvalue0;
    
	if (recipient == 0x02) {
		// Endpoint
		unsigned char endpointNum = setup_packet.windex0 & 0x0F;
		unsigned char endpointDir = setup_packet.windex0 & 0x80;
		if ((feature == ENDPOINT_HALT) && (endpointNum != 0)) {
			char temp; // FIXME can't we find a global variable
            
			// Halt endpoint (as long as it isn't endpoint 0)
			request_handled = 1;
			// Endpoint descriptors are 8 unsigned chars long, with each in and out taking 4 unsigned chars
			// within the endpoint. (See PIC datasheet.)
			in_ptr = (dataPtr) &ep0_o + (endpointNum * 8);
			if (endpointDir)
				in_ptr += 4;
			// FIXME figure out what this is
			if (setup_packet.brequest == SET_FEATURE)
				temp = 0x84;
			else {
				if (endpointDir == 1)
					temp = 0x00;
				else
					temp = 0x88;
			}
			*in_ptr = temp;
            
		}
	}
}

// Data stage for a Control Transfer that sends data to the host
void in_data_stage(void) {
    
	unsigned char bufferSize;
	// Determine how many unsigned chars are going to the host
	if (dlen < E0SZ)
		bufferSize = dlen;
	else
		bufferSize = E0SZ;
    
	// Load the high two bits of the unsigned char dlen into BC8:BC9
	ep0_i.STAT &= ~(BC8| BC9); // Clear BC8 and BC9
	//ep0_i.STAT |= (unsigned char) ((bufferSize & 0x0300) >> 8);
	//ep0_i.CNT = (unsigned char) (bufferSize & 0xFF);
	ep0_i.CNT = bufferSize;
	ep0_i.ADDR = (int) &control_transfer_buffer;
	// Update the number of unsigned chars that still need to be sent.  Getting
	// all the data back to the host can take multiple transactions, so
	// we need to track how far along we are.
	dlen = dlen - bufferSize;
	// Move data to the USB output buffer from wherever it sits now.
	in_ptr = (dataPtr) &control_transfer_buffer;
    
	//	for (idx = 0; idx < bufferSize; idx++)
	for (idx = bufferSize; idx--;)
		*in_ptr++ = *code_ptr++;
    
}

void prepare_for_setup_stage(void) {
	control_stage = SETUP_STAGE;
	ep0_o.CNT = E0SZ;
	ep0_o.ADDR = (int) &setup_packet;
	ep0_o.STAT = UOWN | DTSEN;
	ep0_i.STAT = 0x00;
	UCONbits.PKTDIS = 0;
}

char debug=0;

void process_control_transfer(void) {
	if (USTAT == USTAT_OUT) {
		unsigned char PID = (ep0_o.STAT & 0x3C) >> 2; // Pull PID from middle of BD0STAT
		if (PID == 0x0D) {
			// Setup stage
			// Note: Microchip says to turn off the UOWN bit on the IN direction as
			// soon as possible after detecting that a SETUP has been received.
			ep0_i.STAT &= ~UOWN;
			ep0_o.STAT &= ~UOWN;
            
			// Initialize the transfer process
			control_stage = SETUP_STAGE;
			request_handled = 0; // Default is that request hasn't been handled
            
			dlen = 0; // No unsigned chars transferred
			// See if this is a standard (as definded in USB chapter 9) request
            
			debug=setup_packet.bmrequesttype;
			if (1 /* (setup_packet.bmrequesttype & 0x60) == 0x00*/) {// ----------
				unsigned char request = setup_packet.brequest;
				debug = request;
                
                
				if (request == SET_ADDRESS) {
					// Set the address of the device.  All future requests
					// will come to that address.  Can't actually set UADDR
					// to the new address yet because the rest of the SET_ADDRESS
					// transaction uses address 0.
                    
					request_handled = 1;
					usbcdc_device_state = ADDRESS;
					device_address = setup_packet.wvalue0;
				} else if (request == GET_DESCRIPTOR) {
					get_descriptor();
				} else if (request == SET_CONFIGURATION) {
                    
					request_handled = 1;
					current_configuration = setup_packet.wvalue0;
					// TBD: ensure the new configuration value is one that
					// exists in the descriptor.
					if (current_configuration == 0) {
						// If configuration value is zero, device is put in
						// address state (USB 2.0 - 9.4.7)
						usbcdc_device_state = ADDRESS;
					} else {
						// Set the configuration.
						usbcdc_device_state = CONFIGURED;
                        
						// Initialize the endpoints for all interfaces
						{ // Turn on both in and out for this endpoint
							UEP1 = 0x1E;
                            
							ep1_i.ADDR = (int) &cdcint_buffer;
							ep1_i.STAT = DTS;
                            
							UEP2 = 0x1E;
                            
							ep2_o.CNT = sizeof(cdc_rx_buffer);
							ep2_o.ADDR = (int) &cdc_rx_buffer;
							ep2_o.STAT = UOWN | DTSEN; //set up to receive stuff as soon as we get something
                            
							ep2_i.ADDR = (int) &cdc_tx_buffer;
							ep2_i.STAT = DTS;
						}
					}
				} else if (request == GET_CONFIGURATION) { // Never seen in Windows
					request_handled = 1;
					code_ptr = (codePtr) &const_values_0x00_0x01[current_configuration];
					dlen = 1;
				} else if (request == GET_STATUS) {  // Never seen in Windows
					get_status();
				} else if ((request == CLEAR_FEATURE) || (request
                                                          == SET_FEATURE)) {  // Never seen in Windows
					set_feature();
				} else if (request == GET_INTERFACE) { // Never seen in Windows
					// No support for alternate interfaces.  Send
					// zero back to the host.
                    
					request_handled = 1;
					code_ptr = (codePtr) (&const_values_0x00_0x00);
					dlen = 1;
                } else if ((request == SET_INTERFACE) || (request == SET_LINE_CODING) || (request == SET_CONTROL_LINE_STATE)) {
                    
					// No support for alternate interfaces - just ignore.
                    
					request_handled = 1;
				} else if (request == GET_LINE_CODING) {
					code_ptr = (codePtr) (&cdc_line_coding);
					dlen = sizeof(cdc_line_coding);
					request_handled = 1;
				}
                
                
                ////////////////////
                /*
                 // If request recipient is not an interface then return
                 if((SetupPacket.bmRequestType & 0x1f)!= 0x01) return;
                 
                 // If request type is not class-specific then return
                 if((SetupPacket.bmRequestType & 0x60)!= 0x20) return;
                 
                 
                 // Interface ID must match interface numbers associated with
                 // CDC class, else return
                 
                 if((SetupPacket.wIndex0 != 0x00)&&
                 (SetupPacket.wIndex0 != 0x01)) return;
                 
                 switch(SetupPacket.bRequest)
                 {
                 case SEND_ENCAPSULATED_COMMAND:
                 requestHandled = 1;
                 outPtr = dummy_encapsulated_cmd_response;
                 wCount = DUMMY_LENGTH;
                 break;
                 case GET_ENCAPSULATED_RESPONSE:
                 requestHandled = 1;
                 // Populate dummy_encapsulated_cmd_response first.
                 inPtr = dummy_encapsulated_cmd_response;
                 break;
                 case SET_LINE_CODING:
                 requestHandled = 1;
                 inPtr = (byte*)&line_coding;    // Set destination
                 break;
                 case GET_LINE_CODING:
                 requestHandled = 1;
                 outPtr = (byte*) &line_coding;    // Set source
                 wCount = sizeof(LINE_CODING);   // Set data count
                 break;
                 */
                ////////////////////
                
                
                
                
                
			} //----------
            
            
			if (!request_handled) {
				// If this service wasn't handled then stall endpoint 0
				ep0_o.CNT = E0SZ;
				ep0_o.ADDR = (int) &setup_packet;
				ep0_o.STAT = UOWN | BSTALL;
				ep0_i.STAT = UOWN | BSTALL;
			} else if (setup_packet.bmrequesttype & 0x80) {
				// Device-to-host
				if (setup_packet.wlength < dlen)//9.4.3, p.253
					dlen = setup_packet.wlength;
                
				in_data_stage();
				control_stage = DATA_IN_STAGE;
				// Reset the out buffer descriptor for endpoint 0
				ep0_o.CNT = E0SZ;
				ep0_o.ADDR = (int) &setup_packet;
				ep0_o.STAT = UOWN;
				// Set the in buffer descriptor on endpoint 0 to send data
				// NOT NEEDED ep0_i.ADDR = (int) &control_transfer_buffer;
				// Give to SIE, DATA1 packet, enable data toggle checks
				ep0_i.STAT = UOWN | DTS | DTSEN;
			} else {
                
				// Host-to-device
				control_stage = DATA_OUT_STAGE;
				// Clear the input buffer descriptor
				ep0_i.CNT = 0;
				ep0_i.STAT = UOWN | DTS | DTSEN;
				// Set the out buffer descriptor on endpoint 0 to receive data
				ep0_o.CNT = E0SZ;
				ep0_o.ADDR = (int) &control_transfer_buffer;
				// Give to SIE, DATA1 packet, enable data toggle checks
				ep0_o.STAT = UOWN | DTS | DTSEN;
			}
			// Enable SIE token and packet processing
			UCONbits.PKTDIS = 0;
            
		} else if (control_stage == DATA_OUT_STAGE) {
			// Complete the data stage so that all information has
			// passed from host to device before servicing it.
            
			{
                
				unsigned char bufferSize;
				//bufferSize = ((0x03 & ep0_o.STAT) << 8) | ep0_o.CNT;
				bufferSize = ep0_o.CNT;
                
				// Accumulate total number of unsigned chars read
				dlen = dlen + bufferSize;
				data_ptr = (dataPtr) &control_transfer_buffer;
				for (idx = bufferSize; idx--;)
					*in_ptr++ = *data_ptr++;
                
			}
            
			// Turn control over to the SIE and toggle the data bit
			if (ep0_o.STAT & DTS)
				ep0_o.STAT = UOWN | DTSEN;
			else
				ep0_o.STAT = UOWN | DTS | DTSEN;
		} else {
			// Prepare for the Setup stage of a control transfer
			prepare_for_setup_stage();
		}
	} else if (USTAT == USTAT_IN) {
		// Endpoint 0:in
        
		//set address
		if ((UADDR == 0) && (usbcdc_device_state == ADDRESS)) {
			// TBD: ensure that the new address matches the value of
			// "device_address" (which came in through a SET_ADDRESS).
			UADDR = setup_packet.wvalue0;
			if (UADDR == 0) {
				// If we get a reset after a SET_ADDRESS, then we need
				// to drop back to the Default state.
				usbcdc_device_state = DEFAULT;
			}
		}
        
		if (control_stage == DATA_IN_STAGE) {
			// Start (or continue) transmitting data
			in_data_stage();
			// Turn control over to the SIE and toggle the data bit
			if (ep0_i.STAT & DTS)
				ep0_i.STAT = UOWN | DTSEN;
			else
				ep0_i.STAT = UOWN | DTS | DTSEN;
		} else {
			// Prepare for the Setup stage of a control transfer
			prepare_for_setup_stage();
		}
	}
}

void usbcdc_init() {
	UCFG = 0x14; // Enable pullup resistors; full speed mode
    
	usbcdc_device_state = DETACHED;
	//	remote_wakeup = 0x00;
	current_configuration = 0x00;
    
	// attach
	if (UCONbits.USBEN == 0) {//enable usb controller
		UCON = 0;
		UIE = 0;
		UCONbits.USBEN = 1;
		usbcdc_device_state = ATTACHED;
        
	}
    
	{//Wait for bus reset
		UIR = 0;
		UIE = 0;
		UIEbits.URSTIE = 1;
		usbcdc_device_state = POWERED;
	}
    
	PIE2bits.USBIE = 1;
}
// Main entry point for USB tasks.  Checks interrupts, then checks for transactions.
void usbcdc_handler(void) {
	if ((UCFGbits.UTEYE == 1) || //eye test
        (usbcdc_device_state == DETACHED) || //not connected
        (UCONbits.SUSPND == 1))//suspended
		return;
    
	// Process a bus reset
	if (UIRbits.URSTIF && UIEbits.URSTIE) {
		{ // bus_reset
            
			UEIR = 0x00;
			UIR = 0x00;
			UEIE = 0x9f;
			UIE = 0x2b;
			UADDR = 0x00;
            
			// Set endpoint 0 as a control pipe
			UEP0 = 0x16;
            
			// Flush any pending transactions
			while (UIRbits.TRNIF == 1)
				UIRbits.TRNIF = 0;
            
			// Enable packet processing
			UCONbits.PKTDIS = 0;
            
			// Prepare for the Setup stage of a control transfer
			prepare_for_setup_stage();
            
			current_configuration = 0; // Clear active configuration
			usbcdc_device_state = DEFAULT;
            
		}
		UIRbits.URSTIF = 0;
	}
	//nothing is done to start of frame
	if (UIRbits.SOFIF && UIEbits.SOFIE) {
		UIRbits.SOFIF = 0;
	}
    
	// stall processing
	if (UIRbits.STALLIF && UIEbits.STALLIE) {
		if (UEP0bits.EPSTALL == 1) {
			// Prepare for the Setup stage of a control transfer
			prepare_for_setup_stage();
			UEP0bits.EPSTALL = 0;
		}
		UIRbits.STALLIF = 0;
	}
	if (UIRbits.UERRIF && UIEbits.UERRIE) {
		// Clear errors
		UIRbits.UERRIF = 0;
	}
	// A transaction has finished.  Try default processing on endpoint 0.
	if (UIRbits.TRNIF && UIEbits.TRNIE) {
		process_control_transfer();
		// Turn off interrupt
		UIRbits.TRNIF = 0;
	}
}
