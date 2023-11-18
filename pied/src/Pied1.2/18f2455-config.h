// PIC18LF2455 Configuration Bit Settings          
            
 // CONFIG1L          
 //#pragma config PLLDIV = 4 			// Divide by 4 (16 MHz oscillator input)
 #pragma config PLLDIV = 1 			// Divide by 1 (4 MHz oscillator input)
 #pragma config CPUDIV = OSC1_PLL2	// [Primary Oscillator Src: /1][96 MHz PLL Src: /2].
 #pragma config USBDIV = 2			// USB clock source comes from the 96 MHz PLL divided by 2.
 
 // CONFIG1H          
 #pragma config FOSC = HSPLL_HS		// HS oscillator, PLL enabled (HSPLL).
 #pragma config FCMEN = OFF			// Fail-Safe Clock Monitor disabled.
 #pragma config IESO = OFF			// Oscillator Switchover mode disabled.
 
 // CONFIG2L          
 #pragma config PWRT = ON			// Power-up Timer Enable (Power up timer enabled)
 #pragma config BOR = OFF			// Brown-out Reset Enable (BOR enabled in hardware (SBOREN is ignored))
 #pragma config BORV = 2			// Setting 1 2.79V.
 #pragma config VREGEN = ON			// USB voltage regulator enabled.
 
 // CONFIG2H          
 #pragma config WDT = OFF			// Watchdog Timer Enable bits (WDT disabled in hardware (SWDTEN ignored))
 #pragma config WDTPS = 32768		// Watchdog Timer Postscaler (1:32768)
 
 // CONFIG3H          
 #pragma config CCP2MX = ON			// CCP2 input/output is multiplexed with RC1.
 #pragma config PBADEN = ON			// PORTB A/D Enable bit (PORTB<5:0> pins are configured as analog input channels on Reset)
 #pragma config LPT1OSC = OFF		// Timer1 configured for higher power operation.
 #pragma config MCLRE = ON			// MCLR pin enabled; RE3 input pin disabled.
 
 // CONFIG4L          
 #pragma config STVREN = OFF		// Stack Full/Underflow Reset (Stack full/underflow will not cause Reset)          
 #pragma config LVP = OFF			// Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)          
 #pragma config XINST = OFF			// Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled)          
 #pragma config DEBUG = OFF			// Background debugger disabled, RB6 and RB7 configured as general purpose I/O pins.

 // CONFIG5L          
 #pragma config CP0 = OFF        // Block 0 Code Protect (Block 0 is not code-protected)          
 #pragma config CP1 = OFF        // Block 1 Code Protect (Block 1 is not code-protected)          
 #pragma config CP2 = OFF        // Block 2 Code Protect (Block 2 is not code-protected)          
 
 // CONFIG5H          
 #pragma config CPB = OFF        // Boot Block Code Protect (Boot block is not code-protected)          
 #pragma config CPD = OFF        // Data EEPROM Code Protect (Data EEPROM is not code-protected)          
 
 // CONFIG6L          
 #pragma config WRT0 = OFF       // Block 0 Write Protect (Block 0 (0800-1FFFh) is not write-protected)          
 #pragma config WRT1 = OFF       // Block 1 Write Protect (Block 1 (2000-3FFFh) is not write-protected)          
 #pragma config WRT2 = OFF       // Block 2 Write Protect (Block 2 (04000-5FFFh) is not write-protected)          
 
 // CONFIG6H          
 #pragma config WRTC = OFF       // Configuration Registers Write Protect (Configuration registers (300000-3000FFh) are not write-protected)          
 #pragma config WRTB = OFF       // Boot Block Write Protect (Boot block (0000-7FFh) is not write-protected)          
 #pragma config WRTD = OFF       // Data EEPROM Write Protect (Data EEPROM is not write-protected)          
 
 // CONFIG7L          
 #pragma config EBTR0 = OFF      // Block 0 Table Read Protect (Block 0 is not protected from table reads executed in other blocks)          
 #pragma config EBTR1 = OFF      // Block 1 Table Read Protect (Block 1 is not protected from table reads executed in other blocks)          
 #pragma config EBTR2 = OFF      // Block 2 Table Read Protect (Block 2 is not protected from table reads executed in other blocks)          
 
 // CONFIG7H          
 #pragma config EBTRB = OFF      // Boot Block Table Read Protect (Boot block is not protected from table reads executed in other blocks)
