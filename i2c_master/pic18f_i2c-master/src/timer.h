/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sun Jun  7 11:21:15 2009 texane
** Last update Sun Sep 27 19:37:31 2009 texane
*/



#ifndef TIMER_H_INCLUDED
# define TIMER_H_INCLUDED



void timer_loop(unsigned int);
void timer_start(void);
unsigned int timer_stop(void);
int timer_handle_interrupt(void);



#endif /* ! TIMER_H_INCLUDED */
