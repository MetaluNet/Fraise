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


#include "memimg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



/* mi_make_patch: Allocate and initialize a mi_patch
 */
mi_patch *mi_make_patch(unsigned long base, unsigned long top)
{
  mi_patch *pat;



  pat=malloc(sizeof(mi_patch));
  if(pat == NULL) {
    return NULL;
  }


  pat->base=base;
  pat->top=top;



  pat->contents=malloc(sizeof(mi_byte_t)*(top-base));
  if(pat->contents == NULL) {
    free(pat);
    return NULL;
  }

  memset(pat->contents, 0xff, sizeof(mi_byte_t)*(top-base));



  pat->mask=malloc(sizeof(char)*(top-base));
  if(pat->mask == NULL) {
    free(pat->contents);
    free(pat);
    return NULL;
  }

  memset(pat->mask, 0, sizeof(char)*(top-base));



  return pat;
}



/* mi_free_patch: Free a mi_patch and its buffers
 */
void mi_free_patch(mi_patch *p)
{
  if(p == NULL) {
    return;
  }

  free(p->contents);
  free(p->mask);
  free(p);
}



/* mi_free_image: Free a mi_patch and its contents
 */
void mi_free_image(mi_image *i)
{
  if(i == NULL) {
    return;
  }

  mi_free_patch(i->program);
  mi_free_patch(i->id);
  mi_free_patch(i->config);
  mi_free_patch(i->devid);
  mi_free_patch(i->eeprom);

  free(i);
}



/* mi_modify_patch: Modify patch contents, tagging it as changed
 */
void mi_modify_patch(mi_patch *p, int base, int len, mi_byte_t *data)
{
  int i;

  if(p == NULL) {
    return;
  }



  if(base<p->base || base+len-1 > p->top) {
    printf("*** mi_modify_patch(): patch out of range\n");
    return;
  }



  for(i=0;i<len;i++) {
    p->contents[base - p->base + i]=data[i];
    p->mask[base - p->base + i]=0xff;
  }
}



/* mi_image: Create a mi_image from the contents of filename
 */
mi_image *mi_load_hexfile(char *filename)
{
  mi_image *img;
  hex_record *r;
  FILE *f;
  hex_file *hf;



  if(filename == NULL) {
    return NULL;
  }



  f=fopen(filename, "r");
  if(f == NULL) {
    return NULL;
  }



  hf=hex_open(f);
  if(hf == NULL) {
    fclose(f);
    return NULL;
  }



  img=malloc(sizeof(mi_image));
  if(img == NULL) {
    fclose(f);
    free(hf);
    return NULL;
  }



  /* These nulls may not be required, but make me feel safer when
   *  using free_image() on an error
   */
  img->program = NULL;
  img->id = NULL;
  img->config = NULL;
  img->devid = NULL;
  img->eeprom = NULL;



  img->program=mi_make_patch(MI_PROGRAM_BASE, MI_PROGRAM_TOP);
  img->id=mi_make_patch(MI_ID_BASE, MI_ID_TOP);
  img->config=mi_make_patch(MI_CONFIG_BASE, MI_CONFIG_TOP);
  img->devid=mi_make_patch(MI_DEVID_BASE, MI_DEVID_TOP);
  img->eeprom=mi_make_patch(MI_EEPROM_BASE, MI_EEPROM_TOP);

  if(img->program == NULL || img->id == NULL || img->config == NULL
     || img->devid == NULL || img->eeprom == NULL) {
    fclose(f);
    free(hf);
    mi_free_image(img);
    return NULL;
  }



  while((r=hex_read(hf))) {
    if(r->type == 0) {
      /*
      printf("file: %.2i@0x%.8X:\t", r->datlen, r->addr);
      for(i=0;i<r->datlen;i++) {
        printf("%.2x", r->data[i]);
      }
      printf("\n");
      */

      if(r->addr >= MI_PROGRAM_BASE && r->addr <= MI_PROGRAM_TOP) {
        //        printf("Program memory\n");
        mi_modify_patch(img->program, r->addr, r->datlen, r->data);
      }

      if(r->addr >= MI_ID_BASE && r->addr <= MI_ID_TOP) {
        //        printf("ID memory\n");
        mi_modify_patch(img->id, r->addr, r->datlen, r->data);
      }

      if(r->addr >= MI_CONFIG_BASE && r->addr <= MI_CONFIG_TOP) {
        //        printf("Config memory\n");
        mi_modify_patch(img->config, r->addr, r->datlen, r->data);
      }

      if(r->addr >= MI_DEVID_BASE && r->addr <= MI_DEVID_TOP) {
        //        printf("Devid memory\n");
        mi_modify_patch(img->devid, r->addr, r->datlen, r->data);
      }


    }
    free(r);
    //    printf("\n");
  }



  free(hf);
  fclose(f);
  return img;
}

