#ifndef _INTPIN_H_
#define _INTPIN_H_

#ifndef INTPIN
#error you must define INTPIN (interrupt number such as 0, 1 or 2) before calling intpin.h !
#endif

#if INTPIN==0
    #define INTPIN_IF INTCONbits.INT0IF
    #define INTPIN_IE INTCONbits.INT0IE
    #define INTPIN_IP WREG // INT0 is always high
    #define INTPIN_EDGE INTCON2bits.INTEDG0
#elif INTPIN==1
    #define INTPIN_IF INTCON3bits.INT1IF
    #define INTPIN_IE INTCON3bits.INT1IE
    #define INTPIN_IP INTCON3bits.INT1IP
    #define INTPIN_EDGE INTCON2bits.INTEDG1
#elif INTPIN==2
    #define INTPIN_IF INTCON3bits.INT2IF
    #define INTPIN_IE INTCON3bits.INT2IE
    #define INTPIN_IP INTCON3bits.INT2IP
    #define INTPIN_EDGE INTCON2bits.INTEDG2
#endif

#endif // _INTPIN_H

