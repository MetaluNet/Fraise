// PIC18LF25K50 Configuration Bit Settings          
            
 // CONFIG1L          
 #pragma config PLLSEL = PLL3X   // PLL Selection (3x clock multiplier)          
 #pragma config CFGPLLEN = OFF   // PLL Enable Configuration bit (PLL Disabled (firmware controlled))          
 #pragma config CPUDIV = NOCLKDIV// CPU System Clock Postscaler (CPU uses system clock (no divide))          
 #pragma config LS48MHZ = SYS48X8// Low Speed USB mode with 48 MHz system clock (System clock at 48 MHz, USB clock divider is set to 8)          
 
 // CONFIG1H          
 #pragma config FOSC = INTOSCIO  // Oscillator Selection (Internal oscillator)          
 #pragma config PCLKEN = ON      // Primary Oscillator Shutdown (Primary oscillator enabled)          
 #pragma config FCMEN = ON       // Fail-Safe Clock Monitor (Fail-Safe Clock Monitor enabled)          
 #pragma config IESO = ON        // Internal/External Oscillator Switchover (Oscillator Switchover mode enabled)          
 
 // CONFIG2L          
 #pragma config nPWRTEN = ON     // Power-up Timer Enable (Power up timer enabled)          
 #pragma config BOREN = SBORDIS  // Brown-out Reset Enable (BOR enabled in hardware (SBOREN is ignored))          
 #pragma config BORV = 190       // Brown-out Reset Voltage (BOR set to 1.9V nominal)          
 #pragma config nLPBOR = ON      // Low-Power Brown-out Reset (Low-Power Brown-out Reset enabled)          
 
 // CONFIG2H          
 #pragma config WDTEN = OFF      // Watchdog Timer Enable bits (WDT disabled in hardware (SWDTEN ignored))          
 #pragma config WDTPS = 32768    // Watchdog Timer Postscaler (1:32768)          
 
 // CONFIG3H          
 #pragma config CCP2MX = RC1     // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)          
 #pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<5:0> pins are configured as analog input channels on Reset)          
 #pragma config T3CMX = RC0      // Timer3 Clock Input MUX bit (T3CKI function is on RC0)          
 #pragma config SDOMX = RB3      // SDO Output MUX bit (SDO function is on RB3)          
 #pragma config MCLRE = OFF      // Master Clear Reset Pin Enable (RE3 input pin enabled; external MCLR disabled)          
 
 // CONFIG4L          
 #pragma config STVREN = OFF     // Stack Full/Underflow Reset (Stack full/underflow will not cause Reset)          
 #pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)          
 #pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled)          
 
 // CONFIG5L          
 #pragma config CP0 = OFF        // Block 0 Code Protect (Block 0 is not code-protected)          
 #pragma config CP1 = OFF        // Block 1 Code Protect (Block 1 is not code-protected)          
 #pragma config CP2 = OFF        // Block 2 Code Protect (Block 2 is not code-protected)          
 #pragma config CP3 = OFF        // Block 3 Code Protect (Block 3 is not code-protected)          
 
 // CONFIG5H          
 #pragma config CPB = OFF        // Boot Block Code Protect (Boot block is not code-protected)          
 #pragma config CPD = OFF        // Data EEPROM Code Protect (Data EEPROM is not code-protected)          
 
 // CONFIG6L          
 #pragma config WRT0 = OFF       // Block 0 Write Protect (Block 0 (0800-1FFFh) is not write-protected)          
 #pragma config WRT1 = OFF       // Block 1 Write Protect (Block 1 (2000-3FFFh) is not write-protected)          
 #pragma config WRT2 = OFF       // Block 2 Write Protect (Block 2 (04000-5FFFh) is not write-protected)          
 #pragma config WRT3 = OFF       // Block 3 Write Protect (Block 3 (06000-7FFFh) is not write-protected)          
 
 // CONFIG6H          
 #pragma config WRTC = OFF       // Configuration Registers Write Protect (Configuration registers (300000-3000FFh) are not write-protected)          
 #pragma config WRTB = OFF       // Boot Block Write Protect (Boot block (0000-7FFh) is not write-protected)          
 #pragma config WRTD = OFF       // Data EEPROM Write Protect (Data EEPROM is not write-protected)          
 
 // CONFIG7L          
 #pragma config EBTR0 = OFF      // Block 0 Table Read Protect (Block 0 is not protected from table reads executed in other blocks)          
 #pragma config EBTR1 = OFF      // Block 1 Table Read Protect (Block 1 is not protected from table reads executed in other blocks)          
 #pragma config EBTR2 = OFF      // Block 2 Table Read Protect (Block 2 is not protected from table reads executed in other blocks)          
 #pragma config EBTR3 = OFF      // Block 3 Table Read Protect (Block 3 is not protected from table reads executed in other blocks)          
 
 // CONFIG7H          
 #pragma config EBTRB = OFF      // Boot Block Table Read Protect (Boot block is not protected from table reads executed in other blocks)
