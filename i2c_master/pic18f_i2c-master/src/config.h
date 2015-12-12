/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sun Nov 30 04:42:01 2008 texane
** Last update Sun Feb  7 15:54:14 2010 texane
*/



#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED



#ifdef SDCC

# define CONFIG(k, n) code static char at __ ## k _ ## k = n

/* bits value meaning
   5    0     clock comes from the primary osc block, no prescale
   4-3  3     system clock postscaler (none)
   2-0  0     prescaler (none)
 */

CONFIG(CONFIG1L, 0x10);

/* bits value meaning
   7    0     osc switchover mode disabled
   6    0     failsafe clock mon disabled
   3-0  a     osc selection: internal osc used, usb use xt
 */

CONFIG(CONFIG1H, 0x0a);

CONFIG(CONFIG2L, 0x00);

/* bits value meaning
   0    0     wdt disabled
 */

CONFIG(CONFIG2H, 0x00);

/* bits value meaning
   1    0     portb<4:0> are digital io
   0    1     ccp2 mx with rc1
 */

CONFIG(CONFIG3H, 0x01);

/* bits value meaning
   7    1     background debugger disabled (rb6,7 are general io pins)
   6    0     extended instruction disabled
 */

CONFIG(CONFIG4L, 0x80);

#endif /* _SDCC */



#endif /* ! CONFIG_H_INCLUDED */
