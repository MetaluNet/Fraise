/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sun Jan 31 09:03:43 2010 texane
** Last update Sun Jan 31 09:04:52 2010 texane
*/



#ifndef I2C_H_INCLUDED
# define I2C_H_INCLUDED



typedef unsigned int i2c_addr_t;


int i2c_setup(void);
unsigned int i2c_write(i2c_addr_t, unsigned char*, unsigned int);
unsigned int i2c_read(unsigned char*, unsigned int);
void i2c_handle_interrupt(void);



#endif /* ! I2C_H_INCLUDED */
