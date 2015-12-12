/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sun May 31 01:58:07 2009 texane
** Last update Sat Oct 10 22:02:37 2009 texane
*/



/* timer1 provides a way to be interrputed when
   the timer per clock cycle incremented register
   matches the value of a period register.
 */



#include <pic18fregs.h>



static volatile int has_interrupted = 0;


void timer_loop(unsigned int usecs)
{
  /* the timer is set to be incremented at
     each insn cycle. an instruction cycle
     occurs at fosc / 4. at a fosc of 8mhz
     there are 2 insns per micro seconds.
   */

  unsigned int d = 0xffff - usecs * 2;

  T1CONbits.TMR1ON = 0; /* disable timer1 */

  T1CONbits.RD16 = 0; /* read/write in 2 8 bits operations */

  TMR1L = d & 0xff;
  TMR1H = (d >> 8) & 0xff;

  T1CONbits.T1CKPS0 = 0; /* 1:1 prescaler */  
  T1CONbits.T1CKPS1 = 0; /* 1:1 prescaler */  
  T1CONbits.T1OSCEN = 0; /* t1 osc shut off */
  T1CONbits.TMR1CS = 0; /* internal clock */
  T1CONbits.TMR1ON = 1; /* enable timer1 */
  PIE1bits.TMR1IE = 1; /* enable int */

  /* wait for interrupt */

  while (1)
    {
      PIE1bits.TMR1IE = 0; /* acquire */

      if (has_interrupted)
	{
	  has_interrupted = 0;
	  break;
	}

      PIE1bits.TMR1IE = 1; /* release */
    }

  T1CONbits.TMR1ON = 0; /* disable timer1 */
}


unsigned int timer_stop(void)
{
  unsigned int n;

  n = (TMR1H << 8) | TMR1L;

  T1CONbits.TMR1ON = 0;

  return n;
}


void timer_start(void)
{
  T1CONbits.TMR1ON = 0; /* disable timer1 */

  T1CONbits.RD16 = 0; /* read/write in 2 8 bits operations */

  TMR1L = 0;
  TMR1H = 0;

  T1CONbits.T1CKPS0 = 0; /* 1:1 prescaler */  
  T1CONbits.T1CKPS1 = 0; /* 1:1 prescaler */  
  T1CONbits.T1OSCEN = 0; /* t1 osc shut off */
  T1CONbits.TMR1CS = 0; /* internal clock */
  T1CONbits.TMR1ON = 1; /* enable timer1 */
  PIE1bits.TMR1IE = 0; /* enable int */
}


int timer_handle_interrupt(void)
{
  if (!PIR1bits.TMR1IF)
    return -1;

  PIR1bits.TMR1IF = 0;
  has_interrupted = 1;
  return 0;
}
