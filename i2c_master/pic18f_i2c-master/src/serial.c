/*
** Made by fabien le mentec <texane@gmail.com>
** 
** Started on  Mon Sep 21 08:54:27 2009 texane
** Last update Wed Nov 11 18:46:21 2009 texane
*/



#include <pic18fregs.h>



#define nop() __asm NOP __endasm

/* fifo */

struct fifo
{
  unsigned char size : 4;
  unsigned char gie : 1;
  unsigned char peie : 1;
  unsigned char error : 1;
  unsigned char overflow : 1;

  unsigned char buffer[8];
};


#define FIFO_INITIALIZER { 0, }


static void fifo_init(struct fifo* f)
{
  f->size = 0;
  f->error = 0;
  f->overflow = 0;
}


static unsigned char fifo_pop(struct fifo* f)
{
  return f->buffer[--f->size];
}


static void fifo_push(struct fifo* f, unsigned char c)
{
  f->buffer[f->size++] = c;
}


static void fifo_lock(struct fifo* f)
{
  f->gie = INTCONbits.GIE;
  f->peie = INTCONbits.PEIE;

  INTCONbits.GIE = 0;
  INTCONbits.PEIE = 0;
}


static void fifo_unlock(struct fifo* f)
{
  INTCONbits.PEIE = f->peie;
  INTCONbits.GIE = f->gie;
}


static struct fifo gfifo;


#define SERIAL_TX_TRIS TRISCbits.TRISC6
#define SERIAL_TX_PIN LATCbits.LATC6

#define SERIAL_RX_TRIS TRISCbits.TRISC7
#define SERIAL_RX_PIN LATCbits.LATC7


static void write_byte(unsigned char c)
{
  /* load tx register
   */

  TXREG = c;
  
  nop();
  nop();
  nop();

  /* wait to be empty
   */

  while (!PIR1bits.TXIF)
    ;
}


static unsigned char peek_byte(void)
{
  unsigned char c;

  while (!PIR1bits.RCIF)
    ;

  c = RCREG;

  PIR1bits.RCIF = 0;

  return c;
}


static void int_wait(void)
{
  INTCONbits.PEIE = 1;
  INTCONbits.GIE = 1;

  __asm SLEEP __endasm ;
}


static unsigned char read_byte(void)
{
  unsigned char has_read = 0;
  unsigned char c = 0;

  while (!has_read)
    {
      int_wait();

      fifo_lock(&gfifo);

      if (gfifo.size)
	{
	  c = fifo_pop(&gfifo);
	  has_read = 1;
	}
      else if (gfifo.error)
	{
	  has_read = 1;
	}

      fifo_unlock(&gfifo);
    }

  return c;
}


/* exported */

void serial_setup(void)
{
  fifo_init(&gfifo);

  SERIAL_TX_TRIS = 0;
  SERIAL_RX_TRIS = 1;

  /* for PORTC
   */

  TXSTA = 0;
  TXSTAbits.TXEN = 1;

  RCSTA = 0;
  RCSTAbits.SPEN = 1;
  RCSTAbits.CREN = 1;

  /* disable rx/tx ints
   */

  PIR1bits.RCIF = 0;
  PIR1bits.TXIF = 0;
  PIE1bits.RCIE = 1;
  PIE1bits.TXIE = 0;

  /* 9600 bauds, 8n1
   */

  SPBRG = 12;
  TXSTA = 0x20;

  BAUDCON = 0x00;
}


void serial_read(unsigned char* s, unsigned char len)
{
  len = len;

  *s = peek_byte();
}


int serial_pop_fifo(unsigned char* c)
{
  int res = -1;

  fifo_lock(&gfifo);

  if (gfifo.size)
    {
      *c = fifo_pop(&gfifo);
      res = 0;
    }

  fifo_unlock(&gfifo);

  return res;
}


void serial_write(unsigned char* s, unsigned char len)
{
  unsigned char i;

  for (i = 0; i < len; ++i)
    write_byte(s[i]);
}


void serial_writei(unsigned int i)
{
#define MASK_BYTE(VALUE, OFFSET) (((VALUE) >> (OFFSET * 8)) & 0xff)
  write_byte(MASK_BYTE(i, 0));
  write_byte(MASK_BYTE(i, 1));
}


void serial_writeb(unsigned char b)
{
  write_byte(b);
}


void serial_handle_interrupt(void)
{
  if (!PIR1bits.RCIF)
    return ;

  if (RCSTAbits.OERR)
    {
      unsigned char c;

      c = RCREG;

      gfifo.error = 1;
    }
  else if (RCSTAbits.FERR)
    {
      RCSTAbits.CREN = 0;
      RCSTAbits.CREN = 1;

      gfifo.error = 1;
    }
  else
    {
      if (gfifo.size < sizeof(gfifo.buffer))
	fifo_push(&gfifo, RCREG);
      else
	gfifo.overflow = 1;
    }

  PIR1bits.RCIF = 0;
}
