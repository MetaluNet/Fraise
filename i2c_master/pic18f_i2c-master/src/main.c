/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sun Sep 20 14:08:30 2009 texane
** Last update Sun Feb  7 18:22:17 2010 texane
*/



#include <pic18fregs.h>
#include "config.h"
#include "int.h"
#include "osc.h"
#include "i2c.h"
#include "serial.h"



void toggle_led(unsigned int i)
{
  static unsigned char led = 0;

  led ^= (1 << i);

  TRISA = 0;
  LATA = led;
}


int main(void)
{
  volatile int is_done = 0;
  unsigned int size;
  unsigned char buffer[32];
  unsigned int i = 0;
  unsigned int j;
  unsigned int k;
  unsigned char cmp[] = { 'f', 'o', 'o' };

  osc_setup();
  int_setup();
  serial_setup();
  i2c_setup();

  toggle_led(0);
  toggle_led(1);
  toggle_led(2);

  while (!is_done)
  {
    for (k = 0; k < 0xf000; ++k)
      ;

    size = i2c_read(buffer + i, sizeof(buffer) - i);
    if (!size)
      continue ;

    i += (unsigned int)size;

    if (i >= sizeof(cmp))
      toggle_led(0);

    while (i >= sizeof(cmp))
    {
      for (j = 0; (j < sizeof(cmp)) && (buffer[j] == cmp[j]); ++j)
	;

      if (j >= sizeof(cmp))
	toggle_led(1);

      j += 1;

      for (k = 0; j < i; ++k, ++j)
	buffer[k] = buffer[j];
      i = k;
    }
  }

  return 0;
}
