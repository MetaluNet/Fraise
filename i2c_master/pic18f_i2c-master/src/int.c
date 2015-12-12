/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sat Oct  3 07:05:54 2009 texane
** Last update Sun Jan 31 18:01:55 2010 texane
*/



#include <pic18fregs.h>

#ifdef CONFIG_ENABLE_I2C
# include "i2c.h"
#endif

#ifdef CONFIG_ENABLE_SERIAL
# include "serial.h"
#endif


void on_low_interrupt(void) __interrupt 2;


void on_low_interrupt(void) __interrupt 2
{
#ifdef CONFIG_ENABLE_I2C
  i2c_handle_interrupt();
#endif

#ifdef CONFIG_ENABLE_SERIAL
  serial_handle_interrupt();
#endif
}


void int_setup(void)
{
  /* disable high prio ints */

  RCON = 0;
  INTCON = 0;
  INTCON2 = 0;

  RCONbits.IPEN = 0;

  INTCONbits.PEIE = 1;
  INTCONbits.GIE = 1;

  INTCON2bits.RBIP = 0;

  /* to remove */
  {
    unsigned int i;

    for (i = 0; i < 1000; ++i)
      __asm nop __endasm;
  }
  /* to remove */

}


void int_disable(unsigned char* prev_state)
{
  *prev_state = INTCONbits.GIE;
}


void int_restore(unsigned char prev_state)
{
  INTCONbits.GIE = prev_state;
}
