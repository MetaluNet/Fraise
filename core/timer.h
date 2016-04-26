#ifndef _TIMER_H_
#define _TIMER_H_

#ifndef TIMER
#error you must define TIMER before calling timer.h !
#endif

//determine PIE/PIR/IPR number
#if TIMER <= 2
#define TIMER_PNUM 1
#elif TIMER == 3
#define TIMER_PNUM 2
#else 
#define TIMER_PNUM 5
#endif

#define TIMER_ON_(Timer) T##Timer##CONbits.TMR##Timer##ON
#define TIMER_ON CALL_FUN(TIMER_ON_,TIMER)

#define TIMER_H_(Timer)  TMR##Timer##H
#define TIMER_H CALL_FUN(TIMER_H_,TIMER)

#define TIMER_L_(Timer)  TMR##Timer##L
#define TIMER_L CALL_FUN(TIMER_L_,TIMER)

#define TIMER_CON_(Timer)  T##Timer##CON
#define TIMER_CON CALL_FUN(TIMER_CON_,TIMER)

#define TIMER_CONbits_(Timer)  T##Timer##CONbits
#define TIMER_CONbits CALL_FUN(TIMER_CONbits_,TIMER)

#define TIMER_PS_(Timer, Bit) T##Timer##CONbits.T##Timer##CKPS##Bit
#define TIMER_PS0 CALL_FUN2(TIMER_PS_,TIMER, 0)
#define TIMER_PS1 CALL_FUN2(TIMER_PS_,TIMER, 1)

#define TIMER_SRC_(Timer, Bit) T##Timer##CONbits.TMR##Timer##CS##Bit
#define TIMER_SRC0 CALL_FUN2(TIMER_SRC_,TIMER, 0)
#define TIMER_SRC1 CALL_FUN2(TIMER_SRC_,TIMER, 1)

#define TIMER_16BIT_(Timer) T##Timer##CONbits.T##Timer##RD16
#define TIMER_16BIT CALL_FUN(TIMER_16BIT_,TIMER)

#define TIMER_IF_(Timer,TimerPNum) PIR##TimerPNum##bits.TMR##Timer##IF
#define TIMER_IF CALL_FUN2(TIMER_IF_,TIMER,TIMER_PNUM)

#define TIMER_IE_(Timer,TimerPNum) PIE##TimerPNum##bits.TMR##Timer##IE
#define TIMER_IE CALL_FUN2(TIMER_IE_,TIMER,TIMER_PNUM)

#define TIMER_IP_(Timer,TimerPNum) IPR##TimerPNum##bits.TMR##Timer##IP
#define TIMER_IP CALL_FUN2(TIMER_IP_,TIMER,TIMER_PNUM)


#endif // _TIMER_H

