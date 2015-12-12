/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sat Oct  3 07:04:33 2009 texane
** Last update Mon Oct 12 10:51:26 2009 texane
*/



#include <pic18fregs.h>
#include "osc.h"



void osc_setup(void)
{
  /* 8Mhz */

  OSCCONbits.IRCF = 7;

  /* internal osc used */

  OSCCONbits.SCS = 2;

  /* idle mode enable so that peripherals are
     clocked with SCS when cpu is sleeping. */

  OSCCONbits.IDLEN = 1;

  /* wait for stable freq */

  while (!OSCCONbits.IOFS)
    ;
}


void osc_set_power(enum osc_pmode pmode)
{
  switch (pmode)
    {
    case OSC_PMODE_SLEEP:
      {
	OSCCONbits.IDLEN = 0;
	__asm sleep __endasm;
	break;
      }

    case OSC_PMODE_PRI_IDLE:
    case OSC_PMODE_SEC_IDLE:
    case OSC_PMODE_RC_IDLE:
      {
	/* assume the current clock */

	OSCCONbits.IDLEN = 1;
	__asm sleep __endasm;
	break;
      }

    case OSC_PMODE_PRI_RUN:
    case OSC_PMODE_SEC_RUN:
    case OSC_PMODE_RC_RUN:
    default:
      {
	break;
      }
    }
}
