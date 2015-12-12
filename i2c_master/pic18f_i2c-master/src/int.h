/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sat Oct  3 07:08:02 2009 texane
** Last update Sun Oct 11 06:36:11 2009 texane
*/



#ifndef INT_H_INCNLUDED
# define INT_H_INCNLUDED



extern unsigned int int_counter;


void int_setup(void);
void int_disable(unsigned char*);
void int_restore(unsigned char);



#endif /* ! INT_H_INCNLUDED */
