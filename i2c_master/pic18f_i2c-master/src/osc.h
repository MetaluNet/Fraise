/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sat Oct  3 07:04:58 2009 texane
** Last update Mon Oct 12 07:18:01 2009 texane
*/



#ifndef OSC_H_INCLUDED
# define OSC_H_INCLUDED



#define OSC_FREQ 8000000


enum osc_pmode
  {
    /* sleep mode: both cpu and peripherals not clocked */
    OSC_PMODE_SLEEP = 0,

    /* run mode: both cpu and peripherals clocked */
    OSC_PMODE_PRI_RUN,
    OSC_PMODE_SEC_RUN,
    OSC_PMODE_RC_RUN,
#define OSC_PMODE_RUN OSC_PMODE_PRI_RUN

    /* idle mode: cpu not clocked, peripherals clocked */
    OSC_PMODE_PRI_IDLE,
    OSC_PMODE_SEC_IDLE,
    OSC_PMODE_RC_IDLE,
#define OSC_PMODE_IDLE OSC_PMODE_PRI_IDLE

    OSC_PMODE_MAX
  };


void osc_setup(void);
void osc_set_power(enum osc_pmode);



#endif /* ! OSC_H_INCLUDED */
