/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Sat Jan 30 15:43:29 2010 texane
** Last update Sun Feb  7 18:18:43 2010 texane
*/



#include <pic18fregs.h>
#include "i2c.h"



/* globals */

enum i2c_state
{
  I2C_STATE_IDLE = 0,
  I2C_STATE_READ,
  I2C_STATE_WRITE
};

static volatile enum i2c_state i2c_state = I2C_STATE_IDLE;
static volatile unsigned char i2c_buffer[32];
static volatile unsigned int i2c_pos = 0;
static volatile i2c_addr_t i2c_addr = 0;


/* locking */

static void lock_i2c(void)
{
  PIE1bits.SSPIE = 0;
}


static void unlock_i2c(void)
{
  PIE1bits.SSPIE = 1;
}


/* exported */

int i2c_setup(void)
{
  /* disable interrupts */
  PIE1bits.SSPIE = 0;

  /* tris */
#define I2C_SDA_TRIS TRISBbits.TRISB0
#define I2C_SCL_TRIS TRISBbits.TRISB1
  I2C_SCL_TRIS = 1;
  I2C_SDA_TRIS = 1;

  /* addressing */
#define I2C_SLAVE_ADDR 0x8
  SSPADD = I2C_SLAVE_ADDR;

  /* enable module, slave mode 7 bit addressing, start and stop bits enabled */
/*   SSPCON1 = 0x2e; */
  SSPCON1 = 0x26;

  SSPSTAT = 0x00;

  /* enable general call, clock stretching */
  SSPCON2 = 0x81;
/*   SSPCON2 = 0x01; */

  /* enable interrupts */
  IPR1bits.SSPIP = 0;
  PIR1bits.SSPIF = 0;
  PIE1bits.SSPIE = 1;

  return 0;
}


static void write_byte(unsigned char b)
{
  PIE1bits.SSPIE = 1;
  PIR1bits.SSPIF = 0;

  SSPBUF = b;
  SSPCON1bits.CKP = 1;

  while (PIR1bits.SSPIF == 0)
    ;

  PIE1bits.SSPIE = 1;
}


unsigned int i2c_write(i2c_addr_t a, unsigned char* s, unsigned int n)
{
#if 0

  /* async mode. tx at the next master read request. */

  unsigned int i;

  if ((i2c_size + n) > sizeof(i2c_buffer))
    n = sizeof(i2c_buffer) - i2c_pos;

  if (n == 0)
    return 0;

  lock_i2c();

  for (i = 0; i < n; ++i, ++i2c_pos)
    i2c_buffer[i2c_pos] = s[i];

  unlock_i2c();

  return i;

#else

  unsigned int i;

  write_byte((unsigned char)a);

  for (i = 0; i < n; ++i, ++s)
    write_byte(*s);

  return i;

#endif
}


unsigned int i2c_read(unsigned char* s, unsigned int n)
{
  /* non blocking mode. bugged testing version. */

  unsigned int i;

  /* not safe, but will do for testing */

  if (i2c_pos == 0)
    return 0;

  lock_i2c();

  if (n > i2c_pos)
    n = i2c_pos;

  i2c_pos = 0;

  for (i = 0; i < n; ++i)
    s[i] = i2c_buffer[i];

  unlock_i2c();

  return i;
}


void i2c_handle_interrupt(void)
{
  static unsigned char buf;
  static int is_avail;

  if (!PIR1bits.SSPIF)
    return ;

  PIR1bits.SSPIF = 0;

  switch (i2c_state)
  {
  case I2C_STATE_IDLE:

    i2c_state = I2C_STATE_READ;

    /* fallback */

  case I2C_STATE_READ:

    is_avail = SSPSTATbits.BF | SSPCON1bits.SSPOV;

    buf = SSPBUF;

    if (is_avail)
    {
      if (SSPSTATbits.D_A == 1)
      {
	if (SSPSTATbits.R_W == 1)
	  i2c_state = I2C_STATE_READ;
	else
	  i2c_state = I2C_STATE_WRITE;

	if (i2c_pos < sizeof(i2c_buffer))
	  i2c_buffer[i2c_pos++] = buf;
      }
      else
      {
	i2c_addr = (i2c_addr_t)buf;
      }

      SSPSTATbits.BF = 0;
      SSPCON1bits.SSPOV = 0;
    }

    /* stop bit */
    if (SSPSTATbits.P)
      i2c_state = I2C_STATE_IDLE;

    /* release the line on last bit */
    SSPCON1bits.CKP = 1;

    break;

  case I2C_STATE_WRITE:
    /* stop bit */
    if (SSPSTATbits.P)
      i2c_state = I2C_STATE_IDLE;

    /* release the line once data put in buffer */
    SSPBUF = i2c_buffer[i2c_pos++];

    SSPCON1bits.CKP = 1;
    i2c_state = I2C_STATE_IDLE;

    SSPCON1bits.WCOL = 0;

    break;

  default:
    i2c_state = I2C_STATE_IDLE;
    break;
  }
}
