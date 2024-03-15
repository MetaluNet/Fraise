/*
** This file is part of fsusb_picdem
**
** fsusb_picdem is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version.
**
** fsusb_picdem is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with fsusb_picdem; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
** 02110-1301, USA
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rjlhex.h"



/*
 * hex_open: Create a hex_file from a FILE *
 *
 * f is assumed to already be open for reading, with the
 *  pointer at the start of the file.
 */
hex_file *hex_open(FILE *f)
{
  hex_file *r;

  if(f==NULL) {
    return NULL;
  }

  r=malloc(sizeof(hex_file));
  if(r == NULL) {
    return NULL;
  }

  r->f=f;
  r->addr=0;
  return r;
}


/*
 * hex_raw_read: Create a hex_record from the next line of f
 *
 * f is assumed to already be open for reading, with the
 *  pointer at the start of the line to parse.
 */
hex_record *hex_raw_read(FILE *f)
{
  hex_record *r;
  hex_record *tempr;
  char *s=NULL;
  size_t ssize=0;
  char temps[10];
  int i;
  unsigned char check=0;



  if(f == NULL) {
    return NULL;
  }

  r=malloc(sizeof(hex_record));
  if(r == NULL) {
    return NULL;
  }



  getline(&s, &ssize, f);

  // :llaaaatt[dd...]cc

  // :
  if(strlen(s)<1 || s[0] != ':') {
    free(r);
    free(s);
    return NULL;
  }



  // ll
  if(strlen(s)<3 || !isxdigit(s[1]) || !isxdigit(s[2])) {
    free(r);
    free(s);
    return NULL;
  }
  sprintf(temps, "0x%c%c", s[1], s[2]);
  r->datlen=strtol(temps, NULL, 16);
  check += r->datlen;



  if(strlen(s) < r->datlen*2 + 11) {
    free(r);
    free(s);
    return NULL;
  }

  for(i=3;i<r->datlen*2+11;i++) {
    if(!isxdigit(s[i])) {
      free(r);
      free(s);
      return NULL;
    }
  }
  tempr=realloc(r, sizeof(hex_record) + r->datlen*2);
  if(tempr == NULL) {
    free(r);
    free(s);
    return NULL;
  }
  r=tempr;



  // aaaa

  sprintf(temps, "0x%c%c%c%c", s[3], s[4], s[5], s[6]);
  r->addr=strtol(temps, NULL, 16);

  sprintf(temps, "0x%c%c", s[3], s[4]);
  check+=strtol(temps, NULL, 16);
  sprintf(temps, "0x%c%c", s[5], s[6]);
  check+=strtol(temps, NULL, 16);



  // tt

  sprintf(temps, "0x%c%c", s[7], s[8]);
  r->type=strtol(temps, NULL, 16);
  check += r->type;



  // [dd...]

  for(i=0;i<r->datlen;i++) {
    sprintf(temps, "0x%c%c", s[9+2*i], s[10+2*i]);
    r->data[i]=strtol(temps, NULL, 16);
    check+=r->data[i];
  }



  // cc
  sprintf(temps, "0x%c%c", s[r->datlen*2+9], s[r->datlen*2+10]);
  r->checksum=strtol(temps, NULL, 16);

  //  printf("check is %x, 2c of check is %x\n", check, (unsigned char)(-((int)check)));
  //  printf("checksum wanted is %x\n", r->checksum);

  free(s);

  if((unsigned char)(-((int)check)) != r->checksum) {
    printf("hex_raw_read(): BAD CHECKSUM: got %x, wanted %x\n",
           (unsigned char)(-((int)check)), r->checksum);
    free(r);
    return NULL;
  }

  return r;

}


/*
 * hex_read: Return the next hex_record from f
 */
hex_record *hex_read(hex_file *f)
{
  hex_record *r;



  if(f == NULL) {
    return NULL;
  }



  r=hex_raw_read(f->f);
  if(r == NULL) {
    return NULL;
  }



  switch(r->type) {
  case 0: // data
    r->addr += f->addr;
    break;


  case 1: // EOF
    /*
     * Do nothing, although something could be done on these
     *
     * It'll only get more data past this on a funny file,
     *  and the assumption is that user-supplied files are usually ok
     *  (which may not be a good assumption)
     */
    break;


  case 2: // hex86 address
    f->addr = (r->data[0] << 12) + (r->data[1] << 4); // endianness?
    break;


  case 4: // hex386 address
    f->addr = (r->data[0] << 24) + (r->data[1] << 16); // endianness?
    break;
  }



  return r;
}
