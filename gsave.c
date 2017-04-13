#ifndef LIBRARY
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gload.h"
#define STATIC
#endif

#define NUMFORMATS 10

STATIC uchar winpal []={0,0,0, 128,0,0, 0,128,0, 128,128,0, 0,0,128, 128,0,128, 0,128,128, 151,151,151, 128,128,128, 255,0,0, 192,192,192, 255,255,0, 0,0,255, 0,255,0,     0,255,255, 255,255,255};
STATIC float Gamma=1;
STATIC long Dither=1, Interpolate=0, pfree, UniformPal=0;
STATIC long degrade=0;
STATIC uchar *ColorTable=0, MasterPal[768], *ppic;
STATIC uchar jpghead[]={  /* 0x27B = 635 bytes long */
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43,
    0x00, 0x10, 0x0B, 0x0C, 0x0E, 0x0C, 0x0A, 0x10, 0x0E, 0x0D, 0x0E, 0x12,
    0x11, 0x10, 0x13, 0x18, 0x28, 0x1A, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23,
    0x25, 0x1D, 0x28, 0x3A, 0x33, 0x3D, 0x3C, 0x39, 0x33, 0x38, 0x37, 0x40,
    0x48, 0x5C, 0x4E, 0x40, 0x44, 0x57, 0x45, 0x37, 0x38, 0x50, 0x6D, 0x51,
    0x57, 0x5F, 0x62, 0x67, 0x68, 0x67, 0x3E, 0x4D, 0x71, 0x79, 0x70, 0x64,
    0x78, 0x5C, 0x65, 0x67, 0x63, 0xFF, 0xDB, 0x00, 0x43, 0x01, 0x11, 0x12,
    0x12, 0x18, 0x15, 0x18, 0x2F, 0x1A, 0x1A, 0x2F, 0x63, 0x42, 0x38, 0x42,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x00, 0x00, 0x00, 0x03,
    0x00, 0x22, 0x00, 0x01, 0x11, 0x01, 0x02, 0x11, 0x01, 0xFF, 0xC4, 0x00,
    0x1F, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xFF, 0xC4, 0x00, 0xB5, 0x10, 0x00,
    0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00,
    0x00, 0x01, 0x7D, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21,
    0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81,
    0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24,
    0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56,
    0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86,
    0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
    0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3,
    0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6,
    0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9,
    0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1,
    0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF, 0xC4, 0x00,
    0x1F, 0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xFF, 0xC4, 0x00, 0xB5, 0x11, 0x00,
    0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00,
    0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31,
    0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08,
    0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15,
    0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18,
    0x19, 0x1A, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55,
    0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84,
    0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA,
    0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4,
    0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
    0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF, 0xDA, 0x00,
    0x0C, 0x03, 0x00, 0x00, 0x01, 0x11, 0x02, 0x11, 0x00, 0x3F, 0x00, 0xFF,
    0xDA, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x3F, 0x00, 0xFF, 0xD9};

#ifndef DOSLIB
STATIC long append_curico(FILE *fptr, char *spec)
/* Append a picture to a CUR or ICO file.  Returns with file written and
 *  closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: See save_curico for spec details.
 * Exit:  long error: 0 for good, otherwise error number.      10/9/96-DWM */
{
  char total[4], okay=0;
  uchar *buf;
  long size, num, i;

  (*fread2)(total, 4, 1, fptr);
  if (spec[0]==4 && !memcmp(total,"\0\0\1\0",4))  okay = 1;
  if (spec[0]==1 && !memcmp(total,"\0\0\2\0",4))  okay = 1;
  if (!okay) { (*fclose2)(fptr);  return(perr=5); }
  (*fseek2)(fptr, 0, SEEK_END);
  size = (*ftell2)(fptr);
  if (!(buf=malloc2(size+16))) { perr = 3; (*fclose2)(fptr); return(3); }
  (*fseek2)(fptr, 0, SEEK_SET);
  (*fread2)(buf, 1, size, fptr);
  num = ((short *)(buf+4))[0];
  memmove(buf+6+16*num+16, buf+6+16*num, size-6-16*num);
  for (i=0; i<num; i++)
    ((long *)(buf+6+i*16+12))[0] += 16;
  ((short *)(buf+4))[0]++;
  ((long *)(buf+6+num*16+12))[0] = size+16;
  (*fseek2)(fptr, 0, SEEK_SET);
  (*fwrite2)(buf, 1, size+16, fptr);
  free2(buf);
  (*fseek2)(fptr, 6+num*16, SEEK_SET);
  return(save_curico(fptr, spec));
}
#endif

STATIC long append_gif(FILE *fptr, char *spec)
/* Append a picture to a GIF file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: See save_gif for spec details.
 * Exit:  long error: 0 for good, otherwise error number.      10/9/96-DWM */
{
  char total[10];
  long x, y, mod=0;

  (*fread2)(total, 10, 1, fptr);
  if (memcmp(total, "GIF8", 4)) { (*fclose2)(fptr);  return(perr=5); }
  x = ((short *)(total+6))[0];  y = ((short *)(total+6))[1];
  if (pwidth>x)  { ((short *)(total+6))[0] = x;  mod = 1; }
  if (pheight>y) { ((short *)(total+6))[1] = y;  mod = 1; }
  if (mod) {
    (*fseek2)(fptr, 0, SEEK_SET);
    (*fwrite2)(total, 10, 1, fptr); }
  (*fseek2)(fptr, -1, SEEK_END);
  (*fread2)(total, 1, 1, fptr);
  if (total[0]!=';') { (*fclose2)(fptr);  return(perr=5); }
  return(save_gif(fptr, spec));
}

STATIC long append_jpg(FILE *fptr, char *spec)
/* Append a picture to a JPG file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: See save_jpg for spec details.
 * Exit:  long error: 0 for good, otherwise error number.      10/9/96-DWM */
{
  char total[4];

  (*fread2)(total, 4, 1, fptr);
  if (memcmp(total, "\xFF\xD8\xFF\xE0", 4)) { (*fclose2)(fptr); return(perr=5); }
  (*fseek2)(fptr, 0, SEEK_END);
  return(save_jpg(fptr, spec));
}

STATIC long append_tif(FILE *fptr, char *spec)
/* Append a picture to a TIF file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: See save_tif for spec details.
 * Exit:  long error: 0 for good, otherwise error number.      10/9/96-DWM */
{
  char total[8];
  long end, size;

  (*fread2)(total, 8, 1, fptr);
  if (memcmp(total, "MM\0\x2A", 4) && (end=memcmp(total, "II\x2A", 4))) {
    (*fclose2)(fptr);  return(perr=5); }
  end = (end!=0);
  if (end!=1-spec[6]) { (*fclose2)(fptr);  return(perr=7); }
  (*fseek2)(fptr, 0, SEEK_END);
  size = (*ftell2)(fptr);
  do {
    if (end)  endianl((long *)(total+4));
    (*fseek2)(fptr, ((long *)total)[1], SEEK_SET);
    if ((*fread2)(total, 2, 1, fptr)!=1) { (*fclose2)(fptr);  return(perr=5); }
      if (end)  endian((short *)(total));
    (*fseek2)(fptr, 12*((short *)total)[0], SEEK_CUR);
    if ((*fread2)(total+4, 4, 1, fptr)!=1) { (*fclose2)(fptr);  return(perr=5); } }
  while (((long *)total)[1]);
  (*fseek2)(fptr, -4, SEEK_CUR);
  if (end) endianl(&size);
  (*fwrite2)(&size, 4, 1, fptr);
  (*fseek2)(fptr, 0, SEEK_END);
  return(save_tif(fptr, spec));
}

STATIC long best_color(long allowed, long trans, long inver)
/* Convert the image in ppic to the lowest color mode permitted.  ppic is a
 *  pointer to the converted image.  If a transparent color is used, it is
 *  mapped to color number 255 for palettized images.  If an inverse color is
 *  used, it is mapped to color 254 for palettized images.  For RGB images,
 *  the trans and inv options are ignored.  The resulting image is placed in
 *  ppic, which only contains a palette if the result is in 8-bit palette
 *  mode.  Regardless of mode, each color channel has one byte per pixel (of
 *  which 1, 4, or 8 bits are actually used).
 * Note that if YCbCr color or Padded Grey is used, the image is
 *  automatically inflated so that it's size is a multiple of 32 wide and a
 *  multiple of 16 high.  A calling program should check pwidth and pheight
 *  prior to calling this routine to determine the real size of the image.
 *  The Y component is shifted to the range -128 to 127.  Also, YCbCr data is
 *  stored in a PLANAR configuration, not the normal PACKED configuration.
 *  The Cb and Cr components are downsampled based on the degrade variable,
 *  0-full res., 1-half horizontal, 2-half hor. and vertical, 3-quarter hor,
 *  half vert.  Full space is allocated for the Cb and Cr components, but the
 *  actual stored values are stored as densely as possible.  This means that
 *  there is usused space between Cb and Cr and after Cr if degrade is not
 *  zero.
 * Enter: long allowed: bit array specifying allowed modes.  Bit 0:24-bit
 *                      RGB, 1:8-bit palette, 2:1-bit, 3:4-bit Windows, 4:
 *                      8-bit grey, 5:32-bit CMYK, 6:4-bit Windows or grey,
 *                      7:YCbCr, 8:padded grey.
 *        long trans: color to use for transparancy.  -1 for none.  Only
 *                    modes 1, 2, 3, and 6 use this value.
 *        long inver: color to use for inverse.  -1 for none.
 * Exit:  long mode: Number of mode converted to.  This matches the bit
 *                   number and is also stored in pinter.  Note that modes 6
 *                   and 8 are never explicitly returned.  -1 for failed.
 *                                                             6/21/96-DWM */
{
  long i, j, k, bw=1, grey=1, four=1, s, t, w, h, d, d2, padded=0;
  long w2, best=pinter;
  uchar *new=0, pal[1792], val[4]={0,0x55,0xAA,0xFF}, *new2, *new3;
  char pref[]={0,1,5,7,4,3,2,-1};

  if (allowed&0x100) {
    padded = 1;  allowed = (allowed^0x100)|0x10; }
  if (pinter) {
    for (i=0; i<256; i++) {
      if (ppic[i*3]!=ppic[i*3+1] || ppic[i*3]!=ppic[i*3+2])  grey = 0;
      if (ppic[i*3]!=0 && ppic[i*3]!=255)  bw = 0;
      for (j=0; j<16; j++)
        if (abs(ppic[i*3]-winpal[j*3])<3 && abs(ppic[i*3+1]-winpal[j*3+1])
            <3 && abs(ppic[i*3+2]-winpal[j*3+2])<3) break;
      if (j==16)  four = 0; }
    if (!grey)  bw = 0;
    if (bw)                 best = 2;
    else if (four && grey)  best = 6;
    else if (four)          best = 3;
    else if (grey)          best = 4; }
  if (!(allowed&(1<<best)) && best==5)   best = 0;
  if (!(allowed&(1<<best)) && best==2)   best = 4;
  if (!(allowed&(1<<best)) && best==6)   best = 4;
  if (!(allowed&(1<<best)) && best==4)   best = 1;
  if (!(allowed&(1<<best)) && best==3)   best = 1;
  if (!(allowed&(1<<best)) && best==1)   best = 0;
  if (!(allowed&(1<<best)) && best==0)   best = 7;
  if (!(allowed&(1<<best)))
    for (i=0; pref[i]>-1; i++)
      if (allowed&(1<<pref[i])) { best = pref[i];  break; }
  if (!(allowed&(1<<best)))  return(-1);
  for (i=0; i<256; i++) {
    pal[i]=i*0.299+0.5;  pal[i+256]=i*0.587+0.5;  pal[i+512]=i*0.114+0.5;
    while (pal[i]+pal[i+256]+pal[i+512]<i)  pal[i+256]++; }
  switch (pinter*100+best) {
    case 0: break;
    case 1: if (!(new=malloc2((long)pwidth*pheight+768))) {
        perr = 3; return(-1); }
      if (!palettize(ppic, pwidth, pheight,
          1+2*(trans!=-1 && inver==-1)+4*(inver!=-1))) {
        perr = 3;  free2(new);  return(-1); }
      memcpy(new, MasterPal, 768);
      if (trans!=-1) {
        new[765] = (trans>>16)&0xFF;  new[766] = (trans>>8)&0xFF;
        new[767] = trans&0xFF; }
      if (inver!=-1) {
        new[762] = (trans>>16)&0xFF;  new[763] = (trans>>8)&0xFF;
        new[764] = trans&0xFF; }
      for (i=j=0; i<(long)pwidth*pheight; i++, j+=3)
        if ((ppic[j]<<16)+(ppic[j+1]<<8)+ppic[j+2]==trans)
          new[i+768] = 255;
        else if ((ppic[j]<<16)+(ppic[j+1]<<8)+ppic[j+2]==inver)
          new[i+768] = 254;
        else
          new[i+768] = ColorTable[((ppic[j]&0xF8)<<8)+((ppic[j+1]&0xFC)<<3)+
                                  (ppic[j+2]>>3)];  break;
    case 2: if (!(new=malloc2((long)pwidth*pheight))) {
        perr = 3;  return(-1); }
      for (i=j=0; i<(long)pwidth*pheight; i++, j+=3)
        if ((s=(ppic[j]<<16)+(ppic[j+1]<<8)+ppic[j+2])==trans)
          new[i] = 255;
        else if (s==inver)
          new[i] = 254;
        else
          new[i] = (pal[ppic[j]]+pal[ppic[j+1]+256]+pal[ppic[j+2]+512]>127);
      break;
    case 3: case 6: for (i=0; i<64; i++)
        for (j=0, s=1024; j<16; j++) {
          t = abs(val[i>>4]-winpal[j*3])+abs(val[(i&0xC)>>2]-winpal[j*3+1])+
              abs(val[i&3]-winpal[j*3+2]);
          if (t<s) {  s = t;  pal[i] = j; } }
      if (!(new=malloc2((long)pwidth*pheight))) { perr = 3; return(-1); }
      for (i=j=0; i<(long)pwidth*pheight; i++, j+=3)
        if ((s=(ppic[j]<<16)+(ppic[j+1]<<8)+ppic[j+2])==trans)
          new[i] = 255;
        else if (s==inver)
          new[i] = 254;
        else
          for (k=0, s=1024; k<16; k++) {
            t = abs(ppic[j]-winpal[k*3])+abs(ppic[j+1]-winpal[k*3+1])+
                abs(ppic[j+2]-winpal[k*3+2]);
            if (t<s) { s = t;  new[i] = k; } } break;
    case 4: w = pwidth;  h = pheight;
      if (padded) { w = ((pwidth+31)/32)*32;  h = ((pheight+15)/16)*16; }
      if (!(new=malloc2(w*h))) {
        perr = 3;  return(-1); }
      for (j=d=s=0; j<pheight; j++) {
        for (i=0; i<pwidth; i++,d++,s+=3)
          new[d] = pal[ppic[s]]+pal[ppic[s+1]+256]+pal[ppic[s+2]+512];
        for (; i<w; i++,d++)
          new[d] = new[d-1]; }
      for (; j<h; j++, d+=w)
        memcpy(new+d, new+d-w, w);
      if (padded)
        for (i=0; i<w*h; i++)
          new[i] ^= 0x80;
      pwidth = w;  pheight = h; break;
    case 5: if (!(new=malloc2((long)pwidth*pheight*4))) {
        perr = 3;  return(-1); }
      for (i=s=0; i<(long)pwidth*pheight*4; i+=4, s+=3) {
        new[i+3] = min(min(255-ppic[s], 255-ppic[s+1]), 255-ppic[s+2]);
        new[i]   = 255-ppic[s]  -new[i+3];
        new[i+1] = 255-ppic[s+1]-new[i+3];
        new[i+2] = 255-ppic[s+2]-new[i+3]; } break;
    case 7: w = ((pwidth+31)/32)*32;  h = ((pheight+15)/16)*16;
      if (!(new=malloc2(w*h*3))) { perr = 3; return(-1); }
      new2 = new+w*h;  new3 = new2+w*h;
      for (i=-255; i<=255; i++) {
        ((signed char *)pal)[i+1024] = max(min((float)i/1.772, 127), -127);
        ((signed char *)pal)[i+1536] = max(min((float)i/1.402, 127), -127); }
      w2 = w;  if (degrade>=1)  w2 /= 2;  if (degrade==3)  w2 /= 2;
      for (j=s=d=d2=0; j<pheight; j++) {
        for (i=0; i<pwidth; i++,s+=3,d++) {
          ((char *)new)[d] = pal[ppic[s]]+pal[ppic[s+1]+256]+
                             pal[ppic[s+2]+512]-128;
          if (degrade<2 || !(j&1))
            if (!degrade || (degrade<3 && !(i&1)) || !(i&3)) {
              new2[d2] = pal[ppic[s+2]-(new[d]^0x80)+1024];
              new3[d2] = pal[ppic[s]-(new[d]^0x80)+1536];
              d2++; } }
        for (; i<w; i++,d++) {
          new[d] = new[d-1];
          if (degrade<2 || !(j&1))
            if (!degrade || (degrade<3 && !(i&1)) || !(i&3)) {
              new2[d2] = new2[d2-1];  new3[d2] = new3[d2-1];  d2++; } } }
      for (; j<h; j++, d+=w) {
        memcpy(new+d,  new+d-w,  w);
        if (degrade<2 || !(j&1)) {
          memcpy(new2+d2, new2+d2-w2, w2);
          memcpy(new3+d2, new3+d2-w2, w2);  d2+=w2; } }
      pwidth = w;  pheight = h; break;
    case 100: if (!(new=malloc2((long)pwidth*pheight*3))) {
        perr = 3; return(-1); }
      for (i=s=0; i<(long)pwidth*pheight; i++, s+=3)
        memcpy(new+s, ppic+ppic[i+768]*3, 3); break;
    case 101: if (trans<0 || trans>254) break;
      if (!(new=malloc2((long)pwidth*pheight+768))) break;
      memmove(new, ppic, (long)pwidth*pheight+768);
      for (i=0; i<(long)pwidth*pheight; i++) {
        if (new[i+768]==trans)    new[i+768] = 255;
        else if (new[i+768]==255) new[i+768] = trans; }
      memcpy(pal, new+trans*3, 3);
      memcpy(new+trans*3, new+255*3, 3);
      memcpy(new+255*3, pal, 3); break;
    case 102: if (!(new=malloc2((long)pwidth*pheight))) {
        perr = 3;  return(-1); }
      for (i=s=0; i<256; i++, s+=3)
        if (i==trans)
          pal[i+768] = 255;
        else if (i==inver)
          pal[i+768] = 254;
        else
          pal[i+768] = (pal[ppic[s]]+pal[ppic[s+1]+256]+pal[ppic[s+2]+512]>
                        127);
      for (i=0; i<(long)pwidth*pheight; i++)
        new[i] = pal[ppic[i+768]+768]; break;
    case 103: case 106: for (i=0; i<64; i++)
        for (j=0, s=1024; j<16; j++) {
          t = abs(val[i>>4]-winpal[j*3])+abs(val[(i&0xC)>>2]-winpal[j*3+1])+
              abs(val[i&3]-winpal[j*3+2]);
          if (t<=s) {  s = t;  pal[i] = j; } }
      if (!(new=malloc2((long)pwidth*pheight))) { perr = 3; return(-1); }
      for (i=j=0; i<256; i++, j+=3)
        if (i==trans)
          pal[i+768] = 255;
        else if (i==inver)
          pal[i+768] = 254;
        else
          for (k=0, s=1024; k<16; k++) {
            t = abs(ppic[j]-winpal[k*3])+abs(ppic[j+1]-winpal[k*3+1])+
                abs(ppic[j+2]-winpal[k*3+2]);
            if (t<s) {  s = t;  pal[i+768] = k; } }
      for (i=0; i<(long)pwidth*pheight; i++)
        new[i] = pal[ppic[i+768]+768]; break;
    case 104: w = pwidth;  h = pheight;
      if (padded) { w = ((pwidth+31)/32)*32;  h = ((pheight+15)/16)*16; }
      if (!(new=malloc2(w*h))) {
        perr = 3;  return(-1); }
      for (i=s=0; i<256; i++, s+=3) {
        pal[i+768] = pal[ppic[s]]+pal[ppic[s+1]+256]+pal[ppic[s+2]+512];
        if (padded)
          pal[i+768] ^= 0x80; }
      for (j=d=s=0; j<pheight; j++) {
        for (i=0; i<pwidth; i++,d++,s++)
          new[d] = pal[ppic[s+768]+768];
        for (; i<w; i++,d++)
          new[d] = new[d-1]; }
      for (; j<h; j++, d+=w)
        memmove(new+d, new+d-w, w);
      pwidth = w;  pheight = h; break;
    case 105: if (!(new=malloc2((long)pwidth*pheight*4))) {
        perr = 3; return(-1); }
      for (i=s=0; i<256*4; i+=4, s+=3) {
        pal[i+3] = min(min(255-ppic[s], 255-ppic[s+1]), 255-ppic[s+2]);
        pal[i]   = 255-ppic[s]  -pal[i+3];
        pal[i+1] = 255-ppic[s+1]-pal[i+3];
        pal[i+2] = 255-ppic[s+2]-pal[i+3]; }
      for (i=s=0; i<(long)pwidth*pheight; i++, s+=4)
        memcpy(new+s, pal+ppic[i+768]*4, 4); break;
    case 107: w = ((pwidth+31)/32)*32;  h = ((pheight+15)/16)*16;
      if (!(new=malloc2(w*h*3))) { perr = 3; return(-1); }
      new2 = new+w*h;  new3 = new2+w*h;
      for (i=-255; i<=255; i++) {
        ((signed char *)pal)[i+1024] = max(min((float)i/1.772, 127), -127);
        ((signed char *)pal)[i+1536] = max(min((float)i/1.402, 127), -127); }
      for (i=s=0; i<256; i++,s+=3) {
        new[i] = (pal[ppic[s]]+pal[ppic[s+1]+256]+pal[ppic[s+2]+512])^0x80;
        new[i+256] = pal[ppic[s+2]-(new[i]^0x80)+1024];
        new[i+512] = pal[ppic[s]-(new[i]^0x80)+1536]; }
      memcpy(pal, new, 768);
      w2 = w;  if (degrade>=1)  w2 /= 2;  if (degrade==3)  w2 /= 2;
      for (j=d=d2=0, s=768; j<pheight; j++) {
        for (i=0; i<pwidth; i++,s++,d++) {
          new[d]  = pal[ppic[s]];
          if (degrade<2 || !(j&1))
            if (!degrade || (degrade<3 && !(i&1)) || !(i&3)) {
              new2[d2] = pal[ppic[s]+256];
              new3[d2] = pal[ppic[s]+512];
              d2++; } }
        for (; i<w; i++,d++) {
          new[d] = new[d-1];
          if (degrade<2 || !(j&1))
            if (!degrade || (degrade<3 && !(i&1)) || !(i&3)) {
              new2[d2] = new2[d2-1];  new3[d2] = new3[d2-1];  d2++; } } }
      for (; j<h; j++, d+=w) {
        memcpy(new+d,  new+d-w,  w);
        if (degrade<2 || !(j&1)) {
          memcpy(new2+d2, new2+d2-w2, w2);
          memcpy(new3+d2, new3+d2-w2, w2);  d2+=w2; } }
      pwidth = w;  pheight = h; break;
    default: perr = 1;  return(-1); }
  if (new) {
    if (pfree)  free2(ppic);
    ppic = new;  pfree = 1; }
  pinter = best;
  return(best);
}

STATIC void bmp4rle(uchar *src, long w, FILE *fptr, uchar *buf)
/* Compress a BMP 4-bit/pixel image.
 * Enter: char *src: pointer to original data.
 *        long w: length of data to compress.
 *        FILE *fptr: file to store compress data.
 *        char *buf: buffer large enough to hold compressed data.
 *                                                             8/24/96-DWM */
{
  long run=0, d=0, s=0, cur, cur2, i, count=0;

  while (s<w) {
    cur = src[s];  if (s+1<w)  cur2 = src[s+1];  run = 2-(s+1==w);
    while (1) {
      if (src[s+run]!=cur || s+run>=w || run==255) break;  run++;
      if (src[s+run]!=cur2 || s+run>=w || run==255) break;  run++; }
    if (run>=3 || count || s+run>=w) {
      buf[d] = run;  buf[d+1] = (cur<<4)+cur2;  d+=2;  s+=run; }
    run = min(4, w-s);
    count++;
    while (1) {
      if ((src[s+run]==src[s+run-4] && src[s+run]==src[s+run-2] &&
          src[s+run-1]==src[s+run-3] && src[s+run-1]==src[s+run-5]) ||
          s+run>=w || run==255) break;
      run++; }
    if (src[s+run]==src[s+run-4] && src[s+run]==src[s+run-2] &&
        src[s+run-1]==src[s+run-3] && src[s+run-1]==src[s+run-5])
      run -= 5;
    if (run>=3) {
      buf[d] = 0;  buf[d+1] = run;
      for (i=0; i<run; i++)
        if (i&1) buf[d+2+i/2] |= src[s+i];
        else     buf[d+2+i/2] = src[s+i]<<4;
      s += run;  d += 2+((run+3)/4)*2; } }
  buf[d] = buf[d+1] = 0;  d+=2;
  (*fwrite2)(buf, 1, d, fptr);
}

STATIC void bmp8rle(uchar *src, long w, FILE *fptr, uchar *buf)
/* Compress a BMP 8-bit/pixel image.
 * Enter: char *src: pointer to original data.
 *        long w: length of data to compress.
 *        FILE *fptr: file to store compress data.
 *        char *buf: buffer large enough to hold compressed data.
 *                                                             8/24/96-DWM */
{
  long run=0, d=0, s=0, cur, count=0;

  while (s<w) {
    cur = src[s];  run = 1;
    while (1) {
      if (src[s+run]!=cur || s+run>=w || run==255) break;  run++; }
    if (run>=3 || count || s+run>=w) {
      buf[d] = run;  buf[d+1] = cur;  d+=2;  s+=run; }
    run = 2-(s+1==w);
    count++;
    while (1) {
      if ((src[s+run]==src[s+run-1] && src[s+run]==src[s+run]-2) || s+run>=w
          || run==255) break;
      run++; }
    if (run>=3) {
      buf[d] = 0;  buf[d+1] = run;
      memcpy(buf+d+2, src+s, run);
      s += run;  d += 2+((run+1)/2)*2; } }
  buf[d] = buf[d+1] = 0;  d+=2;
  (*fwrite2)(buf, 1, d, fptr);
}

STATIC void dct_1d(char *src, long *dest)
/* Perform a one dimensional forward discrete-cosine transformation on a set
 *  of 8 values.  The values returned from this routine have been scaled up
 *  by 512*sqrt(2) (i.e., they must be divided by 512*sqrt(2) to get the
 *  correct values).
 * Enter: char *src: source array of 8 signed values.
 *        long *dest: destination array for 8 values.          9/20/96-DWM */
{
  long t0, t1, t2, t3, t4, t5, t6, t7, t10, t11, t12, t13, z1, z2, z3, z4;
  long z5;

  t0=src[0]+src[7];  t1=src[1]+src[6];  t2=src[2]+src[5];  t3=src[3]+src[4];
  t7=src[0]-src[7];  t6=src[1]-src[6];  t5=src[2]-src[5];  t4=src[3]-src[4];
  t10 = t0+t3;  t13 = t0-t3;  t11 = t1+t2;  t12 = t1-t2;
  dest[0] = (t10+t11)<<8;  dest[4] = (t10-t11)<<8;
  z1 = (t12+t13)*UNDCT1;
  dest[6] = z1-t12*UNDCT2;  dest[2] = z1+t13*UNDCT3;
  z1 = t4+t7;  z2 = t5+t6;  z3 = t4+t6;  z4 = t5+t7;  z5 = (z3+z4)*UNDCT4;
  t4 *= UNDCT5;  t5 *= UNDCT6;  t6 *= UNDCT7;  t7 *= UNDCT8;
  z1 *= UNDCT9;  z2 *= UNDCT10;  z3 = z3*UNDCT11+z5;  z4 = z4*UNDCT12+z5;
  dest[7]=t4+z1+z3;  dest[5]=t5+z2+z4;  dest[3]=t6+z2+z3;  dest[1]=t7+z1+z4;
}

STATIC void dct_1d_alt(long *src, long *dest)
/* Perform a one dimensional forward discrete-cosine transformation on a set
 *  of 8 spaced out values.  The values returned from this routine have been
 *  scaled up by 512*sqrt(2) (i.e., they must be divided by 512*sqrt(2) to
 *  get the correct values).
 * Enter: long *src: source array of 57 fixed values.  Only every 8th value
 *                   is used.
 *        long *dest: destination array for 57 fixed values.  This may be the
 *                    same as the source array.                9/21/96-DWM */
{
  long t0, t1, t2, t3, t4, t5, t6, t7, t10, t11, t12, t13, z1, z2, z3, z4;
  long z5;

  t0=src[0]+src[56];   t1=src[8]+src[48];  t2=src[16]+src[40];
  t3=src[24]+src[32];  t7=src[0]-src[56];  t6=src[8]-src[48];
  t5=src[16]-src[40];  t4=src[24]-src[32];
  t10 = t0+t3;  t13 = t0-t3;  t11 = t1+t2;  t12 = t1-t2;
  dest[0] = (t10+t11)<<8;  dest[32] = (t10-t11)<<8;
  z1 = (t12+t13)*UNDCT1;
  dest[48] = z1-t12*UNDCT2;  dest[16] = z1+t13*UNDCT3;
  z1 = t4+t7;  z2 = t5+t6;  z3 = t4+t6;  z4 = t5+t7;  z5 = (z3+z4)*UNDCT4;
  t4 *= UNDCT5;  t5 *= UNDCT6;  t6 *= UNDCT7;  t7 *= UNDCT8;
  z1 *= UNDCT9;  z2 *= UNDCT10;  z3 = z3*UNDCT11+z5;  z4 = z4*UNDCT12+z5;
  dest[56]=t4+z1+z3;  dest[40]=t5+z2+z4;  dest[24]=t6+z2+z3;
  dest[8]=t7+z1+z4;
}

STATIC void fill_zone24(uchar *dest, long bclr, long count)
/* Fill a memory area with a 24-bit value.  This is useful for "erasing" a
 *  memory array to a non-grey color.
 * Enter: char *dest: location to store results.
 *        long bclr: 24-bit value to fill memory with.
 *        long count: number of 24-bit values to store.  Note that three
 *                    times this many bytes are written.        6/9/96-DWM */
{
  if (bclr==0xFFFFFF || !bclr) {
    memset(dest, bclr, count*3);  return; }
  _asm {
          mov edi, dest
          mov eax, bclr
          mov bx, ax
          shr eax, 0x10
          mov ecx, count
fill1:    mov es:[edi], bx
          mov es:[edi+2], al
          add edi, 0x03
          loop    fill1
    }
}

STATIC void jpeg_dct(char *src, long w, long *div, long channel, long *huff)
/* Encode an 8x8 block of an image.  This process first applies a discrete
 *  cosine transform, quantizes the results, differences the dc value,
 *  entropy encodes the ac values, huffman encodes that, stuffs the 0xFF
 *  bytes, then writes it out to a file.
 * Enter: char *src: array of signed values to encode.  Zero for special
 *                   function.  See alternative inputs below.
 *        long w: width of source array.
 *        long *div: table of divisors.  This is a table of 128 values, the
 *                   first 64 are the JPEG quantization table left shifted
 *                   19 bits.  The second 64 are the same table shifted 18
 *                   bits.
 *        long channel: color channel 0-Y, 1-Cb, 2-Cr.  Used for DC
 *                      differencing.
 *        long *huff: huffman compression table.  This is a set of 272
 *                    values, a gap of 272 values, and then another 272
 *                    values.  The first set are the bits for the 16 DC and
 *                    the 256 AC components, and the second set are the
 *                    lengths of these codes.
 * Alt:   char *zero: 0 to indicate the alternate entry.
 *        FILE *fptr: file to write.  If non-zero, initializes routine.  If
 *                    zero, finishes writing file, and all other inputs can
 *                    also be zero.
 *        char *scrap: pointer to a 6 kb buffer.  This buffer must remain
 *                     allocated until the finished command is sent to this
 *                     routine.
 *        long zero2: null value, not used.
 *        long *zero3: null value, not used.                   9/22/96-DWM */
{
  long i, j, k, l, v[1], dct[64], *dest, en=0, *div2=div+64, dc, dcv;
  long ent[64], ents[64], entv[64];
  static long lastdc[3], fp, fb;
  static FILE *fptr;
  static uchar *buf, *fbuf;
  static short nvaltbl[]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,
                          16383,32767};
  uchar *vc=(uchar *)v, *temp;

  if (!src) {
    if (w) {                                               /* Setup tables */
      lastdc[0] = lastdc[1] = lastdc[2] = 0;
      fptr = ((FILE *)w);
      buf = (char *)div;
      for (i=0, j=1, k=0; i<2048; i++) {
        buf[i] = k;
        if (i+1==j) {  j *= 2;  k++; } }
      fbuf = buf+2048;  fp = fb = 0;
      memset(fbuf, 0, 2048); }
    else {
      if (fb) {                                      /* Write last of data */
        v[0] = 0xFF;  v[0] <<= (24-fb-8);
        fbuf[fp] |= vc[2];  fp++;  fb = 0; }
      i = k = 0;  j = fp;                                   /* Stuff bytes */
      if (j) temp = memchr(fbuf+i, 0xFF, j);  else temp = 0;
      while (temp) {
        l = temp-fbuf+1-i;
        memcpy(fbuf+2048+k, fbuf+i, l);
        fbuf[2048+l+k] = 0;
        k += l+1;  i += l;  j -= l;
        if (j)  temp = memchr(fbuf+i, 0xFF, j);  else  temp = 0; }
      if (j)  memcpy(fbuf+2048+k, fbuf+i, j);
      k += j;
      if (k)  (*fwrite2)(fbuf+2048, 1, k, fptr); }
    return; }
  for (i=0, dest=dct; i<8; i++, src+=w, dest+=8)                    /* DCT */
    dct_1d(src, dest);
  for (i=0; i<8; i++)
    dct_1d_alt(dct+i, dct+i);
  if (dct[0]<0) { dcv = 1;  dct[0] = - dct[0]; }  else  dcv = 0;
  if (dct[0]<div2[0])  dc = 0;  else dc = (dct[0]+div2[0])/div[0];
  if (dcv)  dc = -dc;  j = dc;
  dc -= lastdc[channel];  lastdc[channel] = j;
  if (dc<0) { dcv = 1;  dc = -dc; } else  dcv = 0;
  ents[0] = l = k = 0;
  for (i=1; i<64; i++) {
    j = dct[zagtbl[i]];
    if (j<0) { entv[en] = 1;  j = -j; }  else entv[en] = 0;
    if (j<div2[i]) {
      if (ents[en]<240)  ents[en]+=16;
      else { ent[en] = 0;  en++;  ents[en] = 0; } }
    else {
      ent[en] = (j+div2[i])/div[i];  en++;  ents[en] = 0;
      l = en;  k = i; } }
  if (k!=63) {
    ent[l] = ents[l] = 0;                                /* No more values */
    en = l+1; }
  i = buf[dc];  v[0] = huff[i];  l = huff[i+544];        /* Huffman encode */
  v[0] <<= (24-l-fb);
  fbuf[fp] |= vc[2];  fbuf[fp+1] |= vc[1];  fbuf[fp+2] |= vc[0];
  fb += l;  while (fb>=8) { fp++;  fb -= 8; }
  if (i) {
    if (dcv)  v[0] = nvaltbl[i]-dc;  else v[0] = dc;
    v[0] <<= (24-i-fb);
    fbuf[fp] |= vc[2];  fbuf[fp+1] |= vc[1];  fbuf[fp+2] |= vc[0];
    fb += i;  while (fb>=8) { fp++;  fb -= 8; } }
  for (j=0; j<en; j++) {
    i = buf[ent[j]];  k = i+ents[j];
    v[0] = huff[k+16];  l = huff[k+560];
    v[0] <<= (24-l-fb);
    fbuf[fp] |= vc[2];  fbuf[fp+1] |= vc[1];  fbuf[fp+2] |= vc[0];
    fb += l;  while (fb>=8) { fp++;  fb -= 8; }
    if (i) {
      if (entv[j])  v[0] = nvaltbl[i]-ent[j];  else v[0] = ent[j];
      v[0] <<= (24-i-fb);
      fbuf[fp] |= vc[2];  fbuf[fp+1] |= vc[1];  fbuf[fp+2] |= vc[0];
      fb += i;  while (fb>=8) { fp++;  fb -= 8; } } }
  if (fp>=1024) {
    i = k = 0;  j = 1024;                                   /* Stuff bytes */
    temp = memchr(fbuf+i, 0xFF, j);
    while (temp) {
      l = temp-fbuf+1-i;
      memcpy(fbuf+2048+k, fbuf+i, l);
      fbuf[2048+k+l] = 0;
      k += l+1;  i += l;  j -= l;
      if (j)  temp = memchr(fbuf+i, 0xFF, j);  else  temp = 0; }
    if (j)  memcpy(fbuf+2048+k, fbuf+i, j);
    k += j;
    (*fwrite2)(fbuf+2048, 1, k, fptr);
    fp -= 1024;
    memmove(fbuf, fbuf+1024, 1024);
    memset(fbuf+1024, 0, 1024); }
}

#ifndef DOSLIB
STATIC long lzwtif(uchar *dest, uchar *source, uchar *buf, long len,
                   long maxlen)
/* Compress a set of data using the LZW TIF compression scheme.  Note that if
 *  the destination buffer is filled, there will not be an eof code written.
 *  In the worst case, the LZW routine will produce output which is 1.391
 *  times the size of the original input.
 * Enter: long dest: absolute memory location for compressed data.
 *        long source: memory location for data to compress.
 *        char *buf: 28k of work space.
 *        long len: length of data to compress.  Must be at least 2 bytes.
 *        long maxlen: length of output buffer.  Must be at least 8 bytes.
 * Exit:  long len: length of compressed data, including eof.   2/2/96|DWM */
{
  uchar codesize, initcodesize, pos;
  ushort clearcode, eof, firstfree, freecode, maxcode, initmaxcode, incode,
         nextlink;
  long finlen;

  _asm {
          xor ebx, ebx
          mov ecx, maxlen                    ;zero out the destination buffer
          shr ecx, 0x02
          dec ecx
          mov edx, dest
          xor eax, eax
lzwt1:    mov [edx], eax
          add edx, 0x04
          loop    lzwt1
          mov ecx, maxlen
          and ecx, 0x03
          add ecx, 0x04
lzwt2:    mov [edx], al
          inc edx
          loop    lzwt2
          mov eax, dest                ;almost the same beginning as unlzwtif
          sub eax, 0x04
          add maxlen, eax
          mov al, 0x8
          mov dx, 0x0100
          mov clearcode, dx
          mov cx, dx
          inc dx
          mov eof, dx
          inc dx
          mov firstfree, dx
          inc al
          mov codesize, al
          mov initcodesize, al
          shl cx, 0x01
          dec cx
          mov maxcode, cx
          mov initmaxcode, cx
          mov edx, dest
          xor cx, cx
          xor eax, eax
          mov esi, source
          mov al, [esi]
          inc source
          mov esi, buf
          dec len
          mov incode, ax
          mov ax, clearcode
lzwt3:    mov edi, eax                                    ;add code to output
          mov pos, cl
          mov cl, 0x20
          sub cl, pos
          sub cl, codesize
          shl eax, cl
          mov ch, al                 ;These 7 lines are the same as bswap eax
          mov cl, ah
          shl ecx, 0x10
          shr eax, 0x10
          mov ch, al
          mov cl, ah
          mov eax, ecx
          xor ecx, ecx
          mov cl, pos
          or  [edx], eax
          add cl, codesize
          xor eax, eax
          mov al, cl
          shr ax, 0x03
          add edx, eax
          and cl, 0x07
          cmp edx, maxlen
          jge     lzwt14
          cmp di, eof
          je      lzwt14
          cmp di, clearcode
          jne     lzwt7
          mov al, initcodesize                                   ;clear codes
          mov codesize, al
          mov ax, initmaxcode
          mov maxcode, ax
          mov ax, firstfree
          mov freecode, ax
          xor eax, eax
          xor ebx, ebx
lzwt4:    mov [esi+ebx], eax
          add bx, 0x04
          cmp bx, 0x2000
          jne     lzwt4
          mov nextlink, bx
          jmp     lzwt7
lzwt5:    mov ax, [esi+edi+1]                         ;continue building code
          mov incode, ax
          jmp     lzwt7
lzwt6:    mov ax, [esi+ebx+1]
          mov incode, ax
lzwt7:    xor eax, eax                                       ;build next code
          cmp len, 0x00
          je      lzwt12
          mov esi, source
          mov al, [esi]
          inc source
          mov esi, buf
          dec len
          mov bx, incode
          shl bx, 0x01
          mov edi, ebx
          sub edi, 0x03
          mov bx, [esi+ebx]
lzwt8:    test bx, bx
          je      lzwt10
          cmp [esi+ebx], al
          je      lzwt6
          xor edi, edi
          mov di, [esi+ebx+3]
          test edi, edi
          je      lzwt9
          cmp [esi+edi], al
          je      lzwt5
          mov bx, [esi+edi+3]
          jmp     lzwt8
lzwt9:    mov edi, ebx
lzwt10:   mov bx, nextlink
          mov [esi+edi+3], bx
          add nextlink, 0x05
          mov [esi+ebx], al
          xor edi, edi
          mov di, freecode
          mov [esi+ebx+1], edi
          mov di, incode
          mov incode, ax
          push eax
          mov eax, edi
          mov pos, cl
          mov cl, 0x20
          sub cl, pos
          sub cl, codesize
          shl eax, cl
          mov ch, al                 ;These 7 lines are the same as bswap eax
          mov cl, ah
          shl ecx, 0x10
          shr eax, 0x10
          mov ch, al
          mov cl, ah
          mov eax, ecx
          xor ecx, ecx
          mov cl, pos
          or  [edx], eax
          pop eax
          add cl, codesize
          mov ax, cx
          shr ax, 0x03
          add edx, eax
          and cl, 0x07
          cmp edx, maxlen
          jge     lzwt14
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jle     lzwt7
          inc codesize
          shl maxcode, 0x01
          inc maxcode
          cmp codesize, 0x0C
          jl      lzwt7
          jg      lzwt11
          sub maxcode, 0x01
          jmp     lzwt7
lzwt11:   dec codesize
          mov ax, clearcode
          jmp     lzwt3
lzwt12:   mov ax, incode                                     ;write last code
          mov pos, cl
          mov cl, 0x20
          sub cl, pos
          sub cl, codesize
          shl eax, cl
          mov ch, al                 ;These 7 lines are the same as bswap eax
          mov cl, ah
          shl ecx, 0x10
          shr eax, 0x10
          mov ch, al
          mov cl, ah
          mov eax, ecx
          xor ecx, ecx
          mov cl, pos
          or  [edx], eax
          add cl, codesize
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jle     lzwt13
          cmp codesize, 0x0C
          jne     lzwt13
          inc codesize
lzwt13:   xor eax, eax
          mov ax, eof
          jmp     lzwt3
lzwt14:   xor eax, eax                               ;return total lzw length
          mov al, cl
          add al, 0x07
          shr al, 0x03
          add eax, edx
          sub eax, dest
          mov finlen, eax
    }
  return(finlen);
}
#endif

#ifndef DOSLIB
STATIC long lzwtiflen(uchar *source, uchar *buf, long len)
/* Compute the length that data compressed with the LZW TIF compression
 *  scheme will end up being.  No actual data is written.
 * Enter: long source: memory location for data to compress.
 *        char *buf: 28k of work space.
 *        long len: length of data to compress.  Must be at least 2 bytes.
 * Exit:  long len: length of compressed data, including eof.   2/2/96|DWM */
{
  uchar codesize, initcodesize, pos;
  ushort clearcode, eof, firstfree, freecode, maxcode, initmaxcode, incode,
         nextlink;
  long finlen;

  _asm {
          xor ebx, ebx
          mov al, 0x8
          mov dx, 0x0100
          mov clearcode, dx
          mov cx, dx
          inc dx
          mov eof, dx
          inc dx
          mov firstfree, dx
          inc al
          mov codesize, al
          mov initcodesize, al
          shl cx, 0x01
          dec cx
          mov maxcode, cx
          mov initmaxcode, cx
          xor edx, edx
          xor cx, cx
          xor eax, eax
          mov esi, source
          mov al, [esi]
          inc source
          mov esi, buf
          dec len
          mov incode, ax
          mov ax, clearcode
lzwt3:    mov edi, eax                                    ;add code to output
          mov pos, cl
          mov cl, 0x20
          sub cl, pos
          sub cl, codesize
          shl eax, cl
          mov ch, al                 ;These 7 lines are the same as bswap eax
          mov cl, ah
          shl ecx, 0x10
          shr eax, 0x10
          mov ch, al
          mov cl, ah
          mov eax, ecx
          xor ecx, ecx
          mov cl, pos
          add cl, codesize
          xor eax, eax
          mov al, cl
          shr ax, 0x03
          add edx, eax
          and cl, 0x07
          cmp di, eof
          je      lzwt14
          cmp di, clearcode
          jne     lzwt7
          mov al, initcodesize                                   ;clear codes
          mov codesize, al
          mov ax, initmaxcode
          mov maxcode, ax
          mov ax, firstfree
          mov freecode, ax
          xor eax, eax
          xor ebx, ebx
lzwt4:    mov [esi+ebx], eax
          add bx, 0x04
          cmp bx, 0x2000
          jne     lzwt4
          mov nextlink, bx
          jmp     lzwt7
lzwt5:    mov ax, [esi+edi+1]                         ;continue building code
          mov incode, ax
          jmp     lzwt7
lzwt6:    mov ax, [esi+ebx+1]
          mov incode, ax
lzwt7:    xor eax, eax                                       ;build next code
          cmp len, 0x00
          je      lzwt12
          mov esi, source
          mov al, [esi]
          inc source
          mov esi, buf
          dec len
          mov bx, incode
          shl bx, 0x01
          mov edi, ebx
          sub edi, 0x03
          mov bx, [esi+ebx]
lzwt8:    test bx, bx
          je      lzwt10
          cmp [esi+ebx], al
          je      lzwt6
          xor edi, edi
          mov di, [esi+ebx+3]
          test edi, edi
          je      lzwt9
          cmp [esi+edi], al
          je      lzwt5
          mov bx, [esi+edi+3]
          jmp     lzwt8
lzwt9:    mov edi, ebx
lzwt10:   mov bx, nextlink
          mov [esi+edi+3], bx
          add nextlink, 0x05
          mov [esi+ebx], al
          xor edi, edi
          mov di, freecode
          mov [esi+ebx+1], edi
          mov di, incode
          mov incode, ax
          add cl, codesize
          mov ax, cx
          shr ax, 0x03
          add edx, eax
          and cl, 0x07
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jle     lzwt7
          inc codesize
          shl maxcode, 0x01
          inc maxcode
          cmp codesize, 0x0C
          jl      lzwt7
          jg      lzwt11
          sub maxcode, 0x01
          jmp     lzwt7
lzwt11:   dec codesize
          mov ax, clearcode
          jmp     lzwt3
lzwt12:   mov ax, incode                                     ;write last code
          add cl, codesize
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jle     lzwt13
          cmp codesize, 0x0C
          jne     lzwt13
          inc codesize
lzwt13:   mov ax, eof
          jmp     lzwt3
lzwt14:   xor eax, eax                               ;return total lzw length
          mov al, cl
          add al, 0x07
          shr al, 0x03
          add eax, edx
          mov finlen, eax
    }
  return(finlen);
}
#endif

STATIC long palettize(uchar *image, long w, long h, long scale)
/* Compute the appropriate 8-bit palette for a 24-bit image.  The computed
 *  palette is stored in MasterPal.
 * Enter: uchar *image: pointer to 24-bt image.
 *        long w, h: size of 24-bit image.
 *        long scale: 0 for colors in terms of 0-63, 1 for 0-255.  +2 to only
 *                    use 255 color palette, +4 to use 254 color palette.
 * Exit:  long exact: 0 for insufficient memory, 1 for exact palette, 2 for
 *                    best fit palette.                        6/28/96-DWM */
{
  long i, j;

  if (!ColorTable)
    if (!(ColorTable=malloc2(65536))) return(0);
  memset(ColorTable, UniformPal, 65536);
  if (!UniformPal) for (i=j=0; i<w*h; i++, j+=3)
    ColorTable[((image[j]&0xF8)<<8)+((image[j+1]&0xFC)<<3)+(image[j+2]>>3)]
               = 1;
  j = set_color_table(254-((scale&2)!=0)-2*((scale&4)!=0), -1);
  if (scale&1)
    for (i=0; i<768; i++)
      MasterPal[i] *= 4.06;
  return(2-j);
}

STATIC long rle(uchar *output, uchar *input, ushort ln)
/* Convert a string into a compacted Run Length Encoded format.  The maximum
 *  value for len is 32511 bytes.  The output string should be at least 1
 *  byte longer than the input string per 128 characters.  The end of RLE
 *  code is not included.  If this is the last of the rle code, a value of
 *  0x80 must be added at the end of the string.
 * Enter: char *output: pointer to output string.
 *        char *input: pointer to string to convert.
 *        int len: length of input string (both strings may contain nulls).
 * Exit:  long len: length of output string.                  11/30/92|DWM */
{
  ushort i, t;
  long finlen;

  _asm {
          mov edi, output
          mov esi, input
          xor ebx, ebx
          mov i,  0x0000
rle1:     mov bx, i
          cmp bx, ln
          jge     rle13
          xor dx, dx
          mov t,  dx
          mov cx, 0x8000
          xor ax, ax
rle2:     cmp t,  0x03
          jge     rle5
          cmp bx, ln
          jge     rle6
          mov al, [esi+ebx]
          cmp ax, cx
          jne     rle3
          inc t
          jmp     rle4
rle3:     mov t,  0x1
          mov cx, ax
rle4:     inc dx
          inc bx
          jmp     rle2
rle5:     sub dx, 0x03
rle6:     cmp dx, 0x0000
          je      rle10
          cmp dx, 0x0080
          jle     rle8
          mov BYTE PTR [edi], 0x7F
          inc edi
          mov bx, i
          mov ah, 0x80
rle7:     mov al, [esi+ebx]
          mov [edi], al
          inc bx
          inc edi
          dec ah
          jne     rle7
          add i,  0x80
          sub dx, 0x80
          jmp     rle6
rle8:     mov [edi], dl
          sub BYTE PTR [edi], 0x01
          inc edi
          mov ah, dl
          mov bx, i
rle9:     mov al, [esi+ebx]
          mov [edi], al
          inc bx
          inc edi
          dec ah
          jne     rle9
          add i,  dx
          xor dx, dx
          jmp     rle6
rle10:    mov ax, i
          cmp ax, ln
          je      rle13
          xor dx, dx
          mov bx, i
rle11:    cmp bx, ln
          jge     rle12
          cmp dx, 0x80
          jge     rle12
          cmp [esi+ebx], cl
          jne     rle12
          inc bx
          inc dx
          jmp     rle11
rle12:    mov ax, 0x0101
          sub ax, dx
          mov [edi], al
          mov [edi+1], cl
          add edi, 0x02
          add i,  dx
          jmp     rle1
rle13:    mov eax, edi
          sub eax, output
          mov finlen, eax
    }
  return(finlen);
}

STATIC long save_bmp(FILE *fptr, char *spec)
/* Save a picture to a BMP file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: byte 2-color: 0-best, 1-1 bit, 2-4 bit, 3-8 bit, 4-rgb.
 *                    byte 3-compression: 0-none, 1-rle.
 *                    byte 4-(short)dpi.
 *                    byte 6-version: 0-40, 1-12.
 * Exit:  long error: 0 for good, otherwise error number.      8/24/96-DWM */
{
  long clrtbl[]={0xF, 4, 8, 2, 1}, rw, i, j;
  uchar header[54], bits[]={24,8,1,4,0,0,0}, pal[1024];
  uchar *buf, *src, zero[4]={0,0,0,0};

  if (best_color(clrtbl[spec[2]], -1, -1)<0) {
    (*fclose2)(fptr);  return(perr); }
  memcpy(header, "BM", 2);
  ((long *)(header+2))[1] = 0;
  if ((pinter!=1 && pinter!=3) || spec[6])  spec[3] = 0;
  switch (bits[pinter]) {
    case 1: rw = ((pwidth+31)/32)*4; break;
    case 4: rw = ((pwidth+7)/8)*4; break;
    case 8: rw = ((pwidth+3)/4)*4; break;
    case 24: rw = ((pwidth*3+3)/4)*4; }
  if (!(buf=malloc2(rw*2))) { perr = 3; (*fclose2)(fptr); return(3); }
  if (!spec[6]) {                                               /* Type 40 */
    ((long *)(header+14))[0] = 40;
    ((long *)(header+14))[1] = pwidth;
    ((long *)(header+14))[2] = pheight;
    ((short *)(header+26))[0] = 1;
    ((short *)(header+26))[1] = bits[pinter];
    ((long *)(header+14))[4] = spec[3]*(1+(bits[pinter]==4));
    ((long *)(header+14))[5] = rw*pheight;
    ((long *)(header+14))[6] = ((long *)(header+14))[7] =
                                                ((short *)(spec+4))[0]*39.37;
    ((long *)(header+14))[8] = ((long *)(header+14))[9] = 0;
    (*fwrite2)(header, 54, 1, fptr); }
  else {
    ((long *)(header+14))[0] = 12;                              /* Type 12 */
    ((short *)(header+14))[2] = pwidth;
    ((short *)(header+14))[3] = pheight;
    ((short *)(header+14))[4] = 1;
    ((short *)(header+14))[5] = bits[pinter];
    (*fwrite2)(header, 26, 1, fptr); }
  switch (bits[pinter]+100*spec[6]) {                     /* Write palette */
    case 1: pal[0] = pal[1] = pal[2] = pal[3] = pal[7] = 0;
      pal[4] = pal[5] = pal[6] = 255;  (*fwrite2)(pal, 4, 2, fptr); break;
    case 4: for (i=0; i<16; i++) { pal[i*4] = winpal[i*3+2];
        pal[i*4+1]=winpal[i*3+1];  pal[i*4+2]=winpal[i*3];  pal[i*4+3]=0; }
      (*fwrite2)(pal, 4, 16, fptr); break;
    case 8: for (i=0; i<256; i++) { pal[i*4] = ppic[i*3+2];
        pal[i*4+1] = ppic[i*3+1];  pal[i*4+2] = ppic[i*3];  pal[i*4+3] = 0; }
      (*fwrite2)(pal, 4, 256, fptr); break;
    case 101: pal[0] = pal[1] = pal[2] = 0;  pal[3] = pal[4] = pal[5] = 255;
      (*fwrite2)(pal, 3, 2, fptr); break;
    case 104: for (i=0; i<16; i++) { pal[i*3] = winpal[i*3+2];
        pal[i*3+1] = winpal[i*3+1];  pal[i*3+2] = winpal[i*3];  }
      (*fwrite2)(pal, 3, 16, fptr); break;
    case 108: for (i=0; i<256; i++) { pal[i*3] = ppic[i*3+2];
        pal[i*3+1] = ppic[i*3+1];     pal[i*3+2] = ppic[i*3];  }
      (*fwrite2)(pal, 3, 256, fptr); }
  ((long *)(header+2))[2] = (*ftell2)(fptr);
  switch (bits[pinter]+spec[3]*100) {
    case 1: for (j=pheight-1; j>=0; j--) {
      memset(buf, 0, rw);  src = ppic+j*pwidth;
      for (i=0; i<pwidth; i++)
        if (src[i])  buf[i>>3] |= imask[i&7];
      (*fwrite2)(buf, 1, rw, fptr); } break;
    case 4: for (j=pheight-1; j>=0; j--) {
      memset(buf, 0, rw);  src = ppic+j*pwidth;
      for (i=0; i<pwidth; i++)
        if (i&1)  buf[i>>1] |= src[i];
        else      buf[i>>1] = (src[i]<<4);
      (*fwrite2)(buf, 1, rw, fptr); } break;
    case 8: for (j=pheight-1; j>=0; j--) {
      src = ppic+j*pwidth+768;
      (*fwrite2)(src, 1, pwidth, fptr);
      if (pwidth!=rw)
        (*fwrite2)(zero, 1, rw-pwidth, fptr); } break;
    case 24: for (j=pheight-1; j>=0; j--) {
      memset(buf, 0, rw);  src = ppic+j*pwidth*3;
      memcpy(buf, src, pwidth*3);  bgr_to_rgb(buf, pwidth);
      (*fwrite2)(buf, 1, rw, fptr); } break;
    case 104: for (j=pheight-1; j>=0; j--) {
      src = ppic+j*pwidth;
      bmp4rle(src, pwidth, fptr, buf); } break;
    case 108: for (j=pheight-1; j>=0; j--) {
      src = ppic+j*pwidth+768;
      bmp8rle(src, pwidth, fptr, buf); } }
  ((long *)(header+2))[0] = (*ftell2)(fptr);
  if (!spec[6])
    ((long *)(header+14))[5] = (*ftell2)(fptr)-((long *)(header+2))[2];
  (*fseek2)(fptr, 0, SEEK_SET);
  (*fwrite2)(header, 1, 14+40*(!spec[6]), fptr);
  (*fclose2)(fptr);
  free2(buf);
  return(0);
}

#ifndef DOSLIB
STATIC long save_curico(FILE *fptr, char *spec)
/* Save a picture to a CUR or ICO file.  Returns with file written and
 *  closed.  The hot spot is in the original image coordinates.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: byte 0-type: 1-cur, 4-ico.
 *                    byte 2-color: 0-best, 1-1 bit, 2-4 bit, 3-8 bit, 4-rgb.
 *                    byte 4-(short)dpi.
 *                    byte 6-size: 0-current or crop to 255x255, 1-16x16,
 *                                 2-32x32.
 *                    byte 8-(long)transparent color.
 *                    byte C-(long)inverse color.
 *                    byte 10-(short)x hot spot (cursor files only)
 *                    byte 12-(short)y hot spot (cursor files only)
 * Exit:  long error: 0 for good, otherwise error number.      8/24/96-DWM */
{
  long mx, my, trans=((long *)(spec+8))[0], inv=((long *)(spec+12))[0];
  long clrtbl[]={0xF, 4, 8, 2, 1}, rw, rw1, i, j, k, loc, hx, hy, append=0;
  long aphead, aptail, trans2, inv2;
  uchar *new, header[62], pal[1024], *buf, *src, bits[]={24,8,1,4,0,0,0};
  uchar zero[4]={0,0,0,0};
  ushort nclrs[]={0xFFFF, 256, 2, 16};
  float scale;

  if (spec[1]) {
    append = 1;
    aphead = (*ftell2)(fptr);
    (*fseek2)(fptr, 0, SEEK_END);
    aptail = (*ftell2)(fptr); }
  switch (spec[6]) {
    case 0: mx = pwidth;  my = pheight;
      if (mx>255) { mx = 255;  my = pheight*255/pwidth; }
      if (my>255) { my = 255;  mx = pwidth*255/pheight; } break;
    case 1: mx = my = 16; break;
    case 2: mx = my = 32; }
  scale = pwidth/mx;  if (pheight/my>scale)  scale = pheight/my;
  hx = ((short *)(spec+16))[0]/scale+(mx-pwidth/scale)/2;
  hy = ((short *)(spec+16))[1]/scale+(my-pheight/scale)/2;
  if (mx!=pwidth || my!=pheight) {                        /* Scale picture */
    if (pinter) {
      if (!(new=malloc2(mx*my+768))) {
        perr = 3; (*fclose2)(fptr); return(3); }
      memcpy(new, ppic, 768);
      scale_pic(new+768, ppic+768, pwidth, mx, pwidth, mx, pheight, my,
                trans, -1); }
    else {
      if (!(new=malloc2(mx*my*3))) { perr = 3; (*fclose2)(fptr); return(3); }
      scale_pic24(new, ppic, pwidth, mx, pwidth, mx, pheight, my,trans,-1); }
    if (pfree)  free2(ppic);
    ppic = new;  pfree = 1;
    pwidth = mx;  pheight = my; }
  if (pinter) {
    trans2 = (ppic[trans*3]<<16)+(ppic[trans*3+1]<<8)+ppic[trans*3+2];
    inv2   = (ppic[inv  *3]<<16)+(ppic[inv  *3+1]<<8)+ppic[inv  *3+2]; }
  else {
    trans2 = trans;  inv2 = inv; }
  if (best_color(clrtbl[spec[2]], trans, inv)<0) {
    (*fclose2)(fptr);  return(perr); }
  if (!pfree) {
    i = pwidth*pheight*(3-2*pinter)+768*pinter;
    if (!(new=malloc2(i))) {
      perr = 3; (*fclose2)(fptr); return(3); }
    memcpy(new, ppic, i);
    ppic = new;  pfree = 1; }
  switch (bits[pinter]) {                                 /* Create header */
    case 1: rw = ((pwidth+31)/32)*4; break;
    case 4: rw = ((pwidth+7)/8)*4; break;
    case 8: rw = ((pwidth+3)/4)*4; break;
    case 24: rw = ((pwidth*3+3)/4)*4; }
  if (!(buf=malloc2(rw*2))) { perr = 3; (*fclose2)(fptr); return(3); }
  rw1 = ((pwidth+31)/32)*4;
  ((short *)header)[0] = 0;  ((short *)header)[2] = 1;
  ((short *)header)[1] = 1+(spec[0]==1);
  header[6] = mx;  header[7] = my;
  ((short *)header)[4] = nclrs[pinter];
  ((short *)header)[5] = ((short *)(spec+16))[0];
  ((short *)header)[6] = ((short *)(spec+16))[1];
  ((long *)(header+14))[1] = 22;
  ((long *)(header+22))[0] = 40;
  ((long *)(header+22))[1] = mx;
  ((long *)(header+22))[2] = my*2;
  ((short *)(header+34))[0] = 1;
  ((short *)(header+34))[1] = bits[pinter];
  ((long *)(header+22))[4] = 0;
  ((long *)(header+22))[5] = rw*pheight;
  ((long *)(header+22))[6] = ((long *)(header+22))[7] =
                                                ((short *)(spec+4))[0]*39.37;
  ((long *)(header+22))[8] = ((long *)(header+22))[9] = 0;
  if (!append)
    (*fwrite2)(header, 62, 1, fptr);
  else {
    (*fseek2)(fptr, aptail, SEEK_SET);
    (*fwrite2)(header+22, 1, 40, fptr); }
  switch (bits[pinter]) {                                 /* Write palette */
    case 1: pal[0] = pal[1] = pal[2] = pal[3] = pal[7] = 0;
      pal[4] = pal[5] = pal[6] = 255;  (*fwrite2)(pal, 4, 2, fptr); break;
    case 4: for (i=0; i<16; i++) { pal[i*4] = winpal[i*3+2];
        pal[i*4+1]=winpal[i*3+1];  pal[i*4+2]=winpal[i*3];  pal[i*4+3]=0; }
      (*fwrite2)(pal, 4, 16, fptr); break;
    case 8: for (i=0; i<256; i++) { pal[i*4] = ppic[i*3+2];
        pal[i*4+1] = ppic[i*3+1];  pal[i*4+2] = ppic[i*3];  pal[i*4+3] = 0; }
      (*fwrite2)(pal, 4, 256, fptr); }
  loc = (*ftell2)(fptr);                              /* Write basic image */
  switch (bits[pinter]) {
    case 1: for (j=pheight-1; j>=0; j--) {
      memset(buf, 0, rw);  src = ppic+j*pwidth;
      for (i=0; i<pwidth; i++)
        if (src[i] && src[i]!=255)  buf[i>>3] |= imask[i&7];
      (*fwrite2)(buf, 1, rw, fptr); } break;
    case 4: for (j=pheight-1; j>=0; j--) {
      memset(buf, 0, rw);  src = ppic+j*pwidth;
      for (i=0; i<pwidth; i++) {
        k = src[i];
        if (k==254 && inv!=-1)    k = 0xF;
        if (k==255 && trans!=-1)  k = 0;
        if (i&1)  buf[i>>1] |= k;
        else      buf[i>>1] = (k<<4); }
      (*fwrite2)(buf, 1, rw, fptr); } break;
    case 8: for (j=pheight-1; j>=0; j--) {
      src = ppic+j*pwidth+768;
      for (i=0; i<pwidth; i++) {
        k = src[i];
        if (k==254 && inv!=-1)    k = 0xFF;
        if (k==255 && trans!=-1)  k = 0;
        (*fwrite2)(&k, 1, 1, fptr); }
      if (pwidth!=rw)
        (*fwrite2)(zero, 1, rw-pwidth, fptr); } break;
    case 24: for (j=pheight-1; j>=0; j--) {
      memset(buf, 0, rw);  src = ppic+j*pwidth*3;
      memcpy(buf, src, pwidth*3);
      for (i=0; i<pwidth; i++) {
        k = (buf[i*3]<<16)+(buf[i*3+1]<<8)+buf[i*3+2];
        if (k==trans2)  buf[i*3] = buf[i*3+1] = buf[i*3+2] = 0;
        if (k==inv2)    buf[i*3] = buf[i*3+1] = buf[i*3+2] = 0xFF; }
      bgr_to_rgb(buf, pwidth);
      (*fwrite2)(buf, 1, rw, fptr); } }
  if (bits[pinter]!=24) {
    k = 768*(bits[pinter]==8);
    for (i=0; i<mx*my; i++)
      if (ppic[i+k]>=254+(inv!=-1))  ppic[i] = 0x80;
      else                           ppic[i] = 0x00; }
  else
    for (i=0; i<mx*my; i++) {
      k = (ppic[i*3]<<16)+(ppic[i*3+1]<<8)+ppic[i*3+2];
      if (k==trans2 || k==inv2)  ppic[i] = 0x80;
      else                       ppic[i] = 0x00; }
  for (j=pheight-1; j>=0; j--) {
    memset(buf, 0, rw1);  src = ppic+j*pwidth;
    for (i=0; i<pwidth; i++)
      if (src[i]&0x80)  buf[i>>3] |= imask[i&7];
    (*fwrite2)(buf, 1, rw1, fptr); }
  ((long *)(header+22))[5] = (*ftell2)(fptr) - loc;
  ((long *)(header+14))[0] = (*ftell2)(fptr) - 22;
  if (!append) {
    (*fseek2)(fptr, 0, SEEK_SET);
    (*fwrite2)(header, 1, 62, fptr); }
  else {
    (*fseek2)(fptr, aphead, SEEK_SET);
    (*fwrite2)(header+6, 1, 12, fptr);
    (*fseek2)(fptr, aptail, SEEK_SET);
    (*fwrite2)(header+22, 1, 40, fptr); }
  (*fclose2)(fptr);
  free2(buf);
  return(0);
}
#endif

STATIC long save_gif(FILE *fptr, char *spec)
/* Save a picture to a GIF file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: byte 2-use transparent: 0-no, 1-yes.
 *                    byte 6-version: 0-87a, 1-89a.
 *                    byte 7-interlacing: 0-no, 1-yes.
 *                    byte 8-(long)transparent color
 *                    byte C-(short)delay time in 1/100ths of a second.
 *                    byte E-(short)persist image: 0-no, 1-yes
 *                    byte 10-(short)number of times to playback
 * Exit:  long error: 0 for good, otherwise error number.      8/21/96-DWM */
{
  long trans=((long *)(spec+8))[0], len, size, i, rem, gtm=0;
  uchar buf[13]="GIF87a", tran[]={0x21,0xF9,4,9,0,0,0xFF,0}, *new, *temp;
  uchar loop[]="\x21\xFF\xBNETSCAPE2.0\3\1\x10\x27\0", oldpal[768];
  long clrs=1, bits, oldclrs;

  if (!spec[1])
    gtm = 1;
  if (!(temp=malloc2(28*1024))) { perr = 3; (*fclose2)(fptr); return(3); }
  if (!spec[2])  trans = -1;
  if (best_color(2, trans, -1)!=1) {
    (*fclose2)(fptr);  free2(temp);  return(perr); }
  for (i=768; i<pwidth*pheight+768 && clrs<128; i++) {
    if (ppic[i]>clrs && (ppic[i]!=255 || !spec[2]))
      clrs = ppic[i]; }
  if (spec[2] && clrs<127) {
    clrs++;
    for (i=768; i<pwidth*pheight+768; i++)
      if (ppic[i]==255)  ppic[i] = clrs;
    ppic[clrs*3] = ppic[255*3];
    ppic[clrs*3+1] = ppic[255*3+1];
    ppic[clrs*3+2] = ppic[255*3+2];
    tran[6] = clrs; }
  bits = 8;  while (!(clrs&0x80)) { clrs <<= 1;  bits--; }
  if (spec[7]) {
    if (!pfree) {
      if (!(new=malloc2((long)pwidth*pheight+768))) {
        perr = 3;  free2(temp);  (*fclose2)(fptr); return(3); }
      memcpy(new, ppic, (long)pwidth*pheight+768);
      ppic = new;  pfree = 1; }
    uninterlace(ppic+768, 1); }
  if (spec[6])  buf[4] = '9';
  ((short *)(buf+6))[0] = pwidth;  ((short *)(buf+6))[1] = pheight;
  buf[10] = 0x70+bits-1+0x80*gtm;  buf[11] = 255;  buf[12] = 0;
  if (!spec[1]) {
    (*fwrite2)(buf, 13, 1, fptr);
    if (gtm)
      (*fwrite2)(ppic, 3*(1<<bits), 1, fptr); }
  else {
    (*fseek2)(fptr, 10, SEEK_SET);
    oldpal[0] = 0;
    (*fread2)(oldpal, 1, 1, fptr);
    if (oldpal[0]&0x80) {
      oldclrs = (oldpal[0]&7)+1;
      if (oldclrs==bits) {
        (*fseek2)(fptr, 13, SEEK_SET);
        (*fread2)(oldpal, 3*(1<<oldclrs), 1, fptr);
        if (!memcmp(oldpal, ppic, 3*(1<<oldclrs)))
          gtm = 1; } }
    (*fseek2)(fptr, -1, SEEK_END); }
  if (spec[2] || ((short *)(spec+12))[0]) {
    if ((((short *)(spec+12))[0] || ((short *)(spec+16))[0]) && !spec[1]) {
      ((short *)(loop+16))[0] = ((short *)(spec+16))[0];
      (*fwrite2)(loop, 1, 19, fptr); }
    if (!(spec[2]))  tran[3] = 8;
    ((short *)(tran+4))[0] = ((short *)(spec+12))[0];
    if (spec[14]&1)  tran[3] ^= 0xC;
    (*fwrite2)(tran, 1, 8, fptr); }
  buf[0] = ',';
  ((short *)(buf+1))[0] = 0;       ((short *)(buf+1))[1] = 0;
  ((short *)(buf+1))[2] = pwidth;  ((short *)(buf+1))[3] = pheight;
  buf[9] = 0x80+bits-1+0x40*(spec[7]!=0)-0x80*gtm;  buf[10] = 8;
  (*fwrite2)(buf, 10, 1, fptr);
  if (!gtm)
    (*fwrite2)(ppic, 3*(1<<bits), 1, fptr);
  (*fwrite2)(buf+10, 1, 1, fptr);
  if (!(new=malloc2(size=(long)(pwidth*pheight*1.397+10)))) {
    len = lzwlen(ppic+768, temp, (long)pwidth*pheight, 8);
    if (!(new=malloc2(size=(len*256/255+10)))) {
      perr = 3;  free2(temp);  (*fclose2)(fptr); return(3); } }
  len = lzw(new, ppic+768, temp, (long)pwidth*pheight, size, 8);
  size = (len+254)/255;
  rem = len-(size-1)*255;
  memmove(new+(size-1)*256+1, new+(size-1)*255, rem);
  new[(size-1)*256] = rem;
  len = (size-1)*256+rem+1;
  for (i=size-2; i>=0; i--) {
    memmove(new+i*256+1, new+i*255, 255);
    new[i*256] = 255; }
  (*fwrite2)(new, len, 1, fptr);
  free2(temp);
  buf[0] = 0;  buf[1] = ';';
  (*fwrite2)(buf, 2, 1, fptr);
  (*fclose2)(fptr);
  return(0);
}

STATIC long save_graphic(char *name, uchar *image, char *spec, long free)
/* Save a picture to a file.  See switch statement to determine file format
 *  numbers.  The image size and type are stored in pwidth, pheight, and
 *  pinter.  Errors numbers are:
 *   0-none
 *   1-unsupported format
 *   2-couldn't open write file
 *   3-insufficient memory.
 *   4-couldn't open append file
 *   5-file format doesn't match append format
 *   6-first image must be at least as large as appended images
 *   7-different endians
 * Enter: char *name: name, including path, of file to save.
 *        uchar *image: pointer to image data.
 *        char *spec: file format specification.  spec[0] is the actual file
 *                    format.  spec[1] is 0 for a new file or 1 for append to
 *                    an existing file.  The remaining bytes are format
 *                    specific (see GLOAD.H).
 *        long free: If this is non-zero, the image array is free2'ed before
 *                   the call returns.
 * Exit:  long error: 0 for good, otherwise error number.      6/21/96-DWM */
{
  long form, err;
  char *style[]={"wb","r+b"};
  FILE *fptr;

  ppic = image;  pfree = free;
  form = spec[0];
  if (form<0 || form>=NUMFORMATS) {
    if (free && ppic)  free2(ppic);  return(1); }
  if (!(fptr=(*fopen2)(name, style[spec[1]!=0])))
    if (spec[1]!=0) {
      fptr = (*fopen2)(name, style[0]);  spec[1] = 0; }
  if (!fptr) {
    if (free && ppic)  free2(ppic);  return(2+2*(spec[1]!=0)); }
  switch (form+100*(spec[1]!=0)) {
    case 0: err = save_bmp(fptr, spec); break;
#ifndef DOSLIB
    case 1: err = save_curico(fptr, spec); break;
#endif
    case 2: err = save_gif(fptr, spec); break;
#ifndef DOSLIB
    case 3: err = save_grob(fptr, spec); break;
    case 4: err = save_curico(fptr, spec); break;
#endif
    case 5: err = save_jpg(fptr, spec); break;
    case 6: err = save_ppm(fptr, spec); break;
#ifndef DOSLIB
    case 7: err = save_pcx(fptr, spec); break;
    case 8: err = save_tga(fptr, spec); break;
#endif
    case 9: err = save_tif(fptr, spec); break;
#ifndef DOSLIB
    case 101: err = append_curico(fptr, spec); break;
#endif
    case 102: err = append_gif(fptr, spec); break;
#ifndef DOSLIB
    case 104: err = append_curico(fptr, spec); break;
#endif
    case 105: err = append_jpg(fptr, spec); break;
    case 109: err = append_tif(fptr, spec); break;
/** Additional formats go here.  Also change NUMFORMATS. **/
    default: (*fclose2)(fptr);  if (free && ppic)  free2(ppic);  return(1); }
  if (pfree && ppic)  free2(ppic);
  return(perr=err);
}

#ifndef DOSLIB
STATIC long save_grob(FILE *fptr, char *spec)
/* Save a picture to a GROB file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: byte 6-binary: 0-ascii, 1-binary.
 *                    byte 7-size: 0-current, 1-131x64.
 *                    byte 8-(float)gamma value.
 *                    byte C-(long)number of shades.
 * Exit:  long error: 0 for good, otherwise error number.      8/24/96-DWM */
{
  uchar *new, pal[256], *pic;
  long colors=((long *)(spec+12))[0], w, h, i, j, x, size;
  float gamma=((float *)(spec+8))[0];
  uchar tbl[]={16,32,64,128,1,2,4,8};
  uchar mask[]={1,2,4,8,16,32,64,128};
  uchar header[18]={0x48,0x50,0x48,0x50,0x34,0x38,0x2D,0x45,0x1E,
                    0x2B,0x30,0xCA,0x01,0x00,0x00,0x00,0x00,0x00};

  if (spec[7]) {
    if (pinter) {
      if (!(new=malloc2(131*64+768))) { perr = 3; (*fclose2)(fptr); return(3); }
      memcpy(new, ppic, 768);
      for (i=0; i<255; i++)
        if (ppic[i*3]==255 && ppic[i*3+1]==255 && ppic[i*3+2]==255) break;
      scale_pic(new+768, ppic+768, pwidth, 131,pwidth,131,pheight,64,i,-1); }
    else {
      if (!(new=malloc2(131*64*3))) { perr = 3; (*fclose2)(fptr); return(3); }
      scale_pic24(new,ppic,pwidth,131, pwidth, 131,pheight,64,0xFFFFFF,-1); }
    if (pfree)  free2(ppic);
    ppic = new;  pfree = 1;
    pwidth = 131;  pheight = 64; }
  if (best_color(0x10, -1, -1)<0) {
    (*fclose2)(fptr);  return(perr); }
  w = pwidth;  h = pheight;  x = (w+7)/8;
  if (gamma!=1 && gamma>0 && gamma<1e8) {
    for (i=0; i<256; i++)
      pal[i] = pow((float)i/255, gamma)*255;
    for (i=0; i<w*h; i++)
      ppic[i] = pal[ppic[i]]; }
  if (colors<2 || colors>256)  colors = 10;
  for (i=0; i<256; i++)
    pal[i] = floor(i*colors/256);
  for (i=0; i<w*h; i++)
    ppic[i] = pal[ppic[i]];
  if (!(new=malloc2((w+x)*h*(colors-1)))) {
    perr = 3; (*fclose2)(fptr); return(3); }
  for (i=0; i<colors-1; i++)
    for (j=0; j<w*h; j++)
      new[j+i*w*h] = (ppic[j]>i);
  free2(ppic);  ppic = new;
  pic = new+w*h*(colors-1);
  memset(pic, 0, x*h*(colors-1));
  if (!spec[6]) {
    for (j=0; j<h*(colors-1); j++)
      for (i=0; i<w; i++)
        if (!new[i+j*w])
          pic[j*x+i/8] |= tbl[i&7];
    fprintf(fptr, "%%%%HP:T(1);\r\nGROB %d %d ", w, h*(colors-1));
    for (i=0; i<x*h*(colors-1); i++)
      fprintf(fptr, "%02X", pic[i]); }
  else {
    for (j=0; j<h*(colors-1); j++)
      for (i=0; i<w; i++)
        if (!new[i+j*w])
          pic[j*x+i/8] |= mask[i&7];
    header[13] = (h*(colors-1))&0xFF;  header[14] = (h*(colors-1))>>8;
    header[15] = (w&0xF)<<4;  header[16] = (w&0xFF0)>>4;
    size = x*h*(colors-1)*2+0xF;
    header[10] = (size&0xF)<<4;  header[11] = (size>>4)&0xFF;
    header[12] = (size>>12)&0xFF;
    (*fwrite2)(header, 1, 18, fptr);
    (*fwrite2)(pic, 1, x*h*(colors-1), fptr); }
  (*fclose2)(fptr);
  return(0);
}
#endif

STATIC long save_jpg(FILE *fptr, char *spec)
/* Save a picture to a JPEG file.  Returns with file written and closed.  The
 *   custom quantization table is 128 uchar values, the first 64 being the
 *   luminance table and the second 64 being the chrominance table.  Each 64
 *   is in the order of the DC followed by the AC components.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: byte 2-color mode: 0-best, 1-greyscale, 2-YCbCr.
 *                    byte 3-compression: 0-baseline, 1-extended, 2-
 *                           progressive.  bits 6-7: 0-all, 0x40-only data,
 *                           0x80-only header.
 *                    byte 4-(short)dpi.
 *                    byte 6-decimation: 0-1:1, 1-2:1, 2-2x2:1x1, 3-4x2:1x1.
 *                    byte 7-custom: 0-use quality, 1-custom quantization.
 *                    byte 8-(long)quality (0-25500).  100 for no change.
 *                    byte C-(uchar *)pointer to custom quantization table.
 * Exit:  long error: 0 for good, otherwise error number.      8/24/96-DWM */
{
  long clrtbl[]={0x180, 0x100, 0x80}, pw=pwidth, ph=pheight, pw2, pw4, jp;
  long mcu[10], nmcu, mw=1, mh=1, nw, nh, i, j, k, l, m, s, size, size2;
  long *divtbl, *divtbl2, *huff, *huff2, only=((uchar *)spec)[3]&0xC0;
  uchar *head, *buf;
  long hstart[]={0xB6,0xD7,0x18E,0x1AF,0,16,272,288};
  long ln[16]={32768,16384,8192,4096,2048,1024,512,256,128,64,32,16,8,4,2,1};

  if (!(buf=malloc2(6144+0x27B+256*4+(256+16)*16))) {
    perr = 3; (*fclose2)(fptr); return(3); }
  head = buf+6144;  divtbl = (long *)(head+0x27B);  divtbl2 = divtbl+128;
  huff = divtbl+256;
  degrade = spec[6];
  if (best_color(clrtbl[spec[2]], -1, -1)<0) {
    free2(buf);  (*fclose2)(fptr);  return(perr); }
  degrade = 0;
  memset(mcu, 0, 10*sizeof(long));
  switch ((spec[6]+1)*(pinter==7)) {
    case 0: nmcu = 1;  break;
    case 1: nmcu = 3;  mcu[1] = 1;  mcu[2] = 2; break;
    case 2: nmcu = 4;  mcu[2] = 1;  mcu[3] = 2; mw = 2; break;
    case 3: nmcu = 6;  mcu[4] = 1;  mcu[5] = 2; mw = 2; mh = 2; break;
    case 4: nmcu = 10; mcu[8] = 1;  mcu[9] = 2; mw = 4; mh = 2; }
  nw = ((pw+8*mw-1)/(8*mw));
  nh = ((ph+8*mh-1)/(8*mh));
  memcpy(head, jpghead, 0x27B);
  ((short *)(head+0xE))[0]  = ((short *)(spec+4))[0];  endian((short *)(head+0xE));
  ((short *)(head+0x10))[0] = ((short *)(spec+4))[0];  endian((short *)(head+0x10));
  i = spec[3]&0x3F;
  head[0x9F] = 0xC0+i*(i<2)+2*(i>=2);
  for (i=0; i<64; i++) {
    if (spec[7])  j = ((uchar *)(((long *)(spec+0xC))[0]))[i];
    else          j = (long)(head[0x19+i]);
    j *= 0.01*((long *)(spec+8))[0];
    if (j<=0) j = 1;  if (j>255) j = 255;
    head[0x19+i] = j;  divtbl[i] = j<<19;  divtbl[i+64] = j<<18;
    if (j==255)  divtbl[i] = divtbl[i+64] = 0x7FFFFFFF;
    if (spec[7])  j = ((uchar *)(((long *)(spec+0xC))[0]))[i+64];
    else          j = (long)head[0x5E+i];
    j *= 0.01*((long *)(spec+8))[0];
    if (j<=0) j = 1;  if (j>255) j = 255;
    head[0x5E+i] = j;  divtbl[i+128] = j<<19;  divtbl[i+192] = j<<18;
    if (j==255)  divtbl[i+128] = divtbl[i+192] = 0x7FFFFFFF; }
  ((short *)(head+0xA3))[0] = ph;  endian((short *)(head+0xA3));
  ((short *)(head+0xA5))[0] = pw;  endian((short *)(head+0xA5));
  head[0xA9] = mw*16+mh;
  if (!spec[1])
    (*fwrite2)(head, 1, 0x14-0x12*(only==0x80), fptr);
  if (pinter!=7) {
    head[0xA1] = 0xB;  head[0xA7] = 1;
    if (only!=0x40)
      (*fwrite2)(head+0x14,  1, 0x45, fptr);
    if (only!=0x80)
      (*fwrite2)(head+0x9E,  1, 0x0D, fptr);
    if (only!=0x40)
      (*fwrite2)(head+0xB1,  1, 0xD8, fptr);
    if (only!=0x80)
      (*fwrite2)(head+0x26F, 1, 0x0A, fptr); }
  else {
    if (only!=0x40)
      (*fwrite2)(head+0x14,  1, 0x8A, fptr);
    if (only!=0x80)
      (*fwrite2)(head+0x9E,  1, 0x13, fptr);
    if (only!=0x40)
      (*fwrite2)(head+0xB1,  1,0x1B0, fptr);
    if (only!=0x80)
      (*fwrite2)(head+0x261, 1, 0x0E, fptr); }
  if (only!=0x80) {
    size = pwidth*pheight;  size2 = size<<1;
    pw2 = pwidth>>1;  pw4 = pwidth>>2;
    for (i=0; i<4; i++) {                                /* Huffman tables */
      huff2 = huff+hstart[4+i];
      k = hstart[i];
      j = k+16;
      for (l=1, s=0; l<=16; l++)
        for (m=0; m<head[k+l-1]; m++, j++) {
          huff2[head[j]+544] = l;
          huff2[head[j]] = s>>(16-l);
          s += ln[l-1]; } }
    jpeg_dct(0, (long)fptr, (long *)buf, 0, 0);                     /* DCT */
    for (j=0; j<nh*mh*8; j+=mh*8) {
      jp = j*pwidth;
      for (i=0; i<nw*mw*8; i+=mw*8) {
        switch ((spec[6]+1)*(pinter==7)) {
          case 0: jpeg_dct(ppic+jp+i,     pwidth, divtbl, 0, huff); break;
          case 1: jpeg_dct(ppic+jp+i,     pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+size,      pwidth, divtbl2,1, huff+272);
            jpeg_dct(ppic+jp+i+size2,     pwidth, divtbl2,2,huff+272); break;
          case 2: jpeg_dct(ppic+jp+i,     pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+8,         pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+j*pw2+(i>>1)+size, pw2, divtbl2,1, huff+272);
            jpeg_dct(ppic+j*pw2+(i>>1)+size2,pw2, divtbl2,2,huff+272); break;
          case 3: jpeg_dct(ppic+jp+i,     pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+8,         pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+pwidth*8,  pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+8+pwidth*8,pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+(j>>1)*pw2+(i>>1)+size, pw2,divtbl2,1,huff+272);
            jpeg_dct(ppic+(j>>1)*pw2+(i>>1)+size2,pw2,divtbl2,2,huff+272);break;
          case 4: jpeg_dct(ppic+jp+i,     pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+8,         pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+16,        pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+24,        pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+pwidth*8,  pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+8+pwidth*8,pwidth, divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+16+pwidth*8,pwidth,divtbl, 0, huff);
            jpeg_dct(ppic+jp+i+24+pwidth*8,pwidth,divtbl, 0, huff);
            jpeg_dct(ppic+(j>>1)*pw4+(i>>2)+size, pw4,divtbl2,1,huff+272);
            jpeg_dct(ppic+(j>>1)*pw4+(i>>2)+size2,pw4,divtbl2,2,huff+272); } } }
    jpeg_dct(0, 0, 0, 0, 0); }
  (*fwrite2)(head+0x279, 1, 2, fptr);
  (*fclose2)(fptr);
  free2(buf);
  return(0);
}

#ifndef DOSLIB
STATIC long save_pcx(FILE *fptr, char *spec)
/* Save a picture to a PCX file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: byte 2-color mode: 0-best, 1-1-bit, 2-8-bit palette.
 * Exit:  long error: 0 for good, otherwise error number.      8/24/96-DWM */
{
  long clrtbl[]={6,4,2}, x, i, j, cur, d, s, run;
  uchar header[128], *src, *dest, *new;

  if (best_color(clrtbl[spec[2]], -1, -1)<0) {
    (*fclose2)(fptr);  return(perr); }
  x = pwidth;  if (pinter==2) x = (pwidth+7)/8;
  if (!(new=malloc2(x*pheight*2))) { perr = 3; (*fclose2)(fptr); return(0); }
  memset(header, 0, 128);
  header[0] = 0xA;  header[1] = 5;  header[2] = 1;
  header[3] = 1+7*(pinter==1);
  ((short *)(header+8))[0] = pwidth-1;  ((short *)(header+8))[1] = pheight-1;
  ((short *)(header+8))[2] = ((short *)(header+8))[3] = 160;
  header[0x41] = 1;  ((short *)(header+0x42))[0] = x;
  ((short *)(header+0x44))[0] = pinter;
  header[0x13] = header[0x14] = header[0x15] = 255;
  (*fwrite2)(header, 1, 128, fptr);
  if (pinter==2) {
    for (j=0; j<pheight; j++) {
      src = ppic+j*pwidth;  dest = ppic+j*x;
      for (i=0; i<pwidth; i++)
        if (!(i&7))       dest[i>>3] = 0x80*(src[i]!=0);
        else if (src[i])  dest[i>>3] |= imask[i&7]; } }
  d = 0;
  for (j=0; j<pheight; j++) {
    src = ppic+j*x;  s = 0;  if (pinter==1)  src += 768;
    while (s<x) {
      cur = src[s];  run = 1;
      while (s+run<x && run<63)
        if (src[s+run+1]==src[s]) run++;
        else break;
      if (run>1 || src[s]>63) {
        new[d] = run | 0xC0;  new[d+1] = src[s];  d+=2; }
      else {
        new[d] = src[s];  d++; }
      s += run; } }
  (*fwrite2)(new, 1, d, fptr);
  free2(new);
  if (pinter==1) {
    header[0] = 0x0C;
    (*fwrite2)(header, 1, 1, fptr);
    (*fwrite2)(ppic, 1, 768, fptr); }
  (*fclose2)(fptr);
  return(0);
}
#endif

STATIC long save_ppm(FILE *fptr, char *spec)
/* Save a picture to a PPM file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: byte 2-color mode: 0-best, 1-1-bit, 2-8-bit, 3-24-bit.
 *                    byte 6-binary: 0-ascii, 1-binary.
 * Exit:  long error: 0 for good, otherwise error number.      6/21/96-DWM */
{
  long clrtbl[]={0x15,4,0x10,1}, l=0, i, b, d;
  char buf[80];
  uchar mask[]={0x80,0x40,0x20,0x10,8,4,2,1};

  if (best_color(clrtbl[spec[2]], -1, -1)<0) {
    (*fclose2)(fptr);  return(perr); }
  switch (spec[6]*10+pinter) {
    case 0: fprintf(fptr, "P3 %d %d 255\r\n", pwidth, pheight);
      l = (long)pwidth*pheight*3;
    case 2: if (!l) {
        fprintf(fptr, "P1 %d %d\r\n", pwidth, pheight);
        l = (long)pwidth*pheight; }
    case 4: if (!l) {
        fprintf(fptr, "P2 %d %d 255\r\n", pwidth, pheight);
        l = (long)pwidth*pheight; }
      buf[0] = 0;
      for (i=0; i<l; i++) {
        if (strlen(buf)>75) {
          fprintf(fptr, "%s\r\n", buf+1);
          buf[0] = 0; }
        sprintf(buf+strlen(buf), " %d", ppic[i]); }
      fprintf(fptr, "%s\r\n", buf+1); break;
    case 10: fprintf(fptr, "P6 %d %d 255 ", pwidth, pheight);
      (*fwrite2)(ppic, 3, (long)pwidth*pheight, fptr); break;
    case 12: for (i=b=d=0; i<(long)pwidth*pheight; i++, b=(b+1)%8) {
        if (!b)           ppic[d] = mask[0]*ppic[i];
        else if (ppic[i]) ppic[d] |= mask[b];
        if (b==7) d++; }
      if (b)  d++;
      fprintf(fptr, "P4 %d %d ", pwidth, pheight);
      (*fwrite2)(ppic, 1, d, fptr); break;
    case 14: fprintf(fptr, "P5 %d %d 255 ", pwidth, pheight);
      (*fwrite2)(ppic, 1, (long)pwidth*pheight, fptr); }
  (*fclose2)(fptr);
  return(0);
}

#ifndef DOSLIB
STATIC long save_tga(FILE *fptr, char *spec)
/* Save a picture to a TARGA file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: ignored.  This can be any value.
 * Exit:  long error: 0 for good, otherwise error number.      8/24/96-DWM */
{
  uchar header[18];

  if (best_color(1, -1, -1)<0) {
    (*fclose2)(fptr);  return(perr); }
  memset(header, 0, 18);
  header[2] = 2;  header[3] = 1;
  ((short *)(header+12))[0] = pwidth;  ((short *)(header+12))[1] = pheight;
  header[16] = 24;
  (*fwrite2)(header, 1, 18, fptr);
  vertical_flip(ppic, pwidth*3, pheight);
  bgr_to_rgb(ppic, (long)pwidth*pheight);
  (*fwrite2)(ppic, 1, (long)pwidth*pheight*3, fptr);
  bgr_to_rgb(ppic, (long)pwidth*pheight);
  vertical_flip(ppic, pwidth*3, pheight);
  (*fclose2)(fptr);
  return(0);
}
#endif

STATIC long save_tif(FILE *fptr, char *spec)
/* Save a picture to a TIF file.  Returns with file written and closed.
 * Enter: FILE *fptr: pointer to file to write to.
 *        char *spec: byte 2-color mode: 0-best, 1-1-bit, 2-greyscale,
 *                           3-palette, 4-rgb, 5-cmyk.
 *                    byte 3-compression: 0-none, 1-rle, 2-lzw, 3-lzw with
 *                           prediction, 4-lzh, 5-lzh with prediction.
 *                    byte 4-(short)dpi.
 *                    byte 6-endian: 0-big (motorola), 1-little (intel).
 *                    byte 8-(long)rows per strip.
 * Exit:  long error: 0 for good, otherwise error number.      8/24/96-DWM */
{
  long clrtbl[]={0x37, 4, 0x10, 2, 1, 0x20}, hsize, tagsize, numtag;
  long d1, d2, i, j, k, l, val, so, sc, h, y, w, sifd=0;
  long rps=((long *)(spec+8))[0], ns, end=1-spec[6], bpp=1;
  short tag[]={0x100,0x101,0x102,0x103,0x106,0x111,0x115,0x116,0x117,0x11A,
               0x11B,0x128,0x131,0x13D,0x140};
  ushort comp[]={1, 32773, 5, 5, 0xA5AE, 0xA5AE};
  ushort photoint[]={2, 3, 1, 0, 1, 5};
  uchar *head, *dest, *buf;
  short *shead;
  long *lhead, zero=0;

  if (spec[1])  zero = (*ftell2)(fptr)-8;
  if (best_color(clrtbl[spec[2]], -1, -1)<0) {
    (*fclose2)(fptr);  return(perr); }
  if (rps<1 || rps>pheight)  rps = pheight;
  ns = (pheight+rps-1)/rps;                      /* Maximum of 3000 strips */
  if (ns>3000) {
    rps = (pheight+2999)/3000;
    ns = (pheight+rps-1)/rps; }
  software[TIF_MAXSOFTWARELEN-1] = 0;
  hsize = 198+8*ns*(ns>1);
  if (pinter==1)  hsize += 1548;
  if (pinter==0)  hsize += 6;
  if (pinter==5)  hsize += 5;
  hsize += max(5, strlen(software)+9);
  if (!(head=malloc2(hsize+28*1024*(spec[3]>=2)))) {
    perr = 3;  (*fclose2)(fptr);  return(3); }
  shead = (short *)head;  lhead = (long *)head;  buf = head+hsize;
  shead[0] = 0x4949;  if (end) shead[0] = 0x4D4D;
  shead[1] = 42;              if (end)  endian(shead+1);
  lhead[1] = 8;               if (end)  endianl(lhead+1);
  shead[4] = 14+(pinter==1);  if (end)  endian(shead+4);
  d1 = 10;  d2 = 14+(14+(pinter==1))*12;
  for (i=0; i<14+(pinter==1); i++, d1+=12) {
    ((short *)(head+d1))[0] = tag[i];  if (end)  endian((short *)(head+d1));
    tagsize = 3;  numtag = 1;
    switch (tag[i]) {
      case 0x100: val = pwidth; break;
      case 0x101: val = pheight; break;
      case 0x102: bpp = numtag = 1+2*(pinter==0)+3*(pinter==5);  val = 8;
        if (pinter==2)  val = 1;
        if (numtag!=1) {
          val = d2+zero;
          for (j=0; j<numtag; j++,d2+=2) {
            ((short *)(head+d2))[0] = 8;  if (end)  endian((short *)(head+d2)); } }
        break;
      case 0x103: val = comp[spec[3]]; break;
      case 0x106: val = photoint[pinter]; break;
      case 0x111: tagsize = 4;  numtag = ns;  so = d1+8;
        if (ns>1) { val = d2+zero;  so = d2;  d2 += ns*4; } break;
      case 0x115: val = bpp; break;
      case 0x116: val = rps; break;
      case 0x117: tagsize = 4;  numtag = ns;  sc = d1+8;
        if (ns>1) { val = d2+zero;  sc = d2;  d2 += ns*4; } break;
      case 0x11A: case 0x11B: tagsize = 5;  val = d2+zero;
        ((long *)(head+d2))[0] = ((short *)(spec+4))[0];
        ((long *)(head+d2))[1] = 1;
        d2 += 8; break;
      case 0x128: val = 2; break;
      case 0x131: tagsize = 2;  numtag = max(5, strlen(software)+1);
        val = d2+zero;  strcpy(head+d2, software);
        if (strlen(software)>=10)
          if (strstr(software, "IFD 0x")) {
            sifd = d2+(strstr(software, "IFD 0x")-software)+6;
            numtag = sifd-d2+9; }
        d2 += numtag; break;
      case 0x13D: val = 1+(spec[3]==3 || spec[3]==5); break;
      case 0x140: numtag = 768;  val = d2+zero;
        for (j=0; j<256; j++) {
          head[d2+j*2]      = head[d2+j*2+1]    = ppic[j*3];
          head[d2+j*2+512]  = head[d2+j*2+513]  = ppic[j*3+1];
          head[d2+j*2+1024] = head[d2+j*2+1025] = ppic[j*3+2]; }
        d2 += 1536; }
    ((short *)(head+d1))[1] = tagsize;  if (end)  endian((short *)(head+d1+2));
    ((long *)(head+d1))[1] = numtag;  if (end)  endianl((long *)(head+d1+4));
    if (tagsize==3 && numtag==1) {
      ((short *)(head+d1))[4] = val;  if (end)  endian((short *)(head+d1+8));
      ((short *)(head+d1))[5] = 0; }
    else {
      ((long *)(head+d1))[2] = val;  if (end)  endianl((long *)(head+d1+8)); } }
  ((long *)(head+d1))[0] = 0;
  (*fwrite2)(head+8*(spec[1]!=0), 1, hsize-8*(spec[1]!=0), fptr);
  w = pwidth;
  if (pinter==2) {                       /* Convert 1-bit to 8-pixels/byte */
    w = (pwidth+7)/8;
    for (j=d1=d2=0; j<pheight; j++,d2+=w)
      for (i=0; i<pwidth; i++,d1++) {
        if (!(i&7))        ppic[d2+(i>>3)] = imask[0]*ppic[d1];
        else if (ppic[d1]) ppic[d2+(i>>3)] |= imask[i&7]; } }
  for (k=0; k<ns; k++) {
    ((long *)(head+so))[0] = (*ftell2)(fptr);
    y = k*rps;  h = rps;  if (y+h>pheight)  h = pheight-y;
    d1 = w*y*bpp;  d2 = w*h*bpp;
    if (pinter==1)  d1 += 768;
    switch (spec[3]) {
      case 0: (*fwrite2)(ppic+d1, 1, d2, fptr); break;
      case 1: if (!(dest=malloc2(w*bpp*129/128+16))) {
          perr = 3; (*fclose2)(fptr); free2(head); return(3); }
        for (j=0; j<h; j++) {
          i = rle(dest, ppic+d1+j*w*bpp, w*bpp);
          (*fwrite2)(dest, 1, i, fptr); }
        free2(dest); break;
      case 3: for (l=0; l<bpp; l++)
        for (j=0; j<h; j++)
          for (i=w-1; i>0; i--)
            ppic[d1+(i+j*w)*bpp+l] -= ppic[d1+(i-1+j*w)*bpp+l];
      case 2: if (!(dest=malloc2(j=(long)(d2*1.391+16))))
          if (!(dest=malloc2(j=lzwtiflen(ppic+d1, buf, d2)))) {
            perr = 3; (*fclose2)(fptr); free2(head); return(3); }
        i = lzwtif(dest, ppic+d1, buf, d2, j);
        (*fwrite2)(dest, 1, i, fptr);
        free2(dest);
        if (spec[3]==3 && !pfree)
          for (l=0; l<bpp; l++)
            for (j=0; j<h; j++)
              for (i=1; i<w; i++)
                ppic[d1+(i+j*w)*bpp+l] += ppic[d1+(i-1+j*w)*bpp+l]; break;
      case 5: for (l=0; l<bpp; l++)
        for (j=0; j<h; j++)
          for (i=w-1; i>0; i--)
            ppic[d1+(i+j*w)*bpp+l] -= ppic[d1+(i-1+j*w)*bpp+l];
      case 4: if (!(dest=malloc2(j=(long)(d2*1.391+16))))
          if (!(dest=malloc2(j=lzwlen(ppic+d1, buf, d2, 8)))) {
            perr = 3; (*fclose2)(fptr); free2(head); return(3); }
        i = lzw(dest, ppic+d1, buf, d2, j, 8);
        (*fwrite2)(dest, 1, i, fptr);
        free2(dest);
        if (spec[3]==5 && !pfree)
          for (l=0; l<bpp; l++)
            for (j=0; j<h; j++)
              for (i=1; i<w; i++)
                ppic[d1+(i+j*w)*bpp+l] += ppic[d1+(i-1+j*w)*bpp+l]; }
    ((long *)(head+sc))[0] = (*ftell2)(fptr)-((long *)(head+so))[0];
    if (end) {
      endianl((long *)(head+so));  endianl((long *)(head+sc)); }
    so += 4;  sc += 4; }
  if (sifd)
    sprintf(head+sifd, "%04X", (*ftell2)(fptr));
  if (!spec[1])
    (*fseek2)(fptr, 0, SEEK_SET);
  else
    (*fseek2)(fptr, zero+8, SEEK_SET);
  (*fwrite2)(head+8*(spec[1]!=0), 1, hsize-8*(spec[1]!=0), fptr);
  free2(head);
  (*fclose2)(fptr);
  return(0);
}

STATIC void scale_pic(uchar *dest, uchar *source, long srcw, long destw,
                      long simgw, long dimgw, long simgh, long dimgh, long bclr,
                      long trans)
/* Scale a 256 color image.  When scaled down, rows are dropped; when scaled
 *  up, rows are duplicated.  If the source image is not the same shape as
 *  the destination image, the edges are letterboxed.  The image's aspect
 *  ratio is maintained.  If the background color is -1, then the image's
 *  aspect ratio is not maintained, and instead the image is adjusted to fit
 *  the destination shape.
 * Enter: long dest, source: flat address values for images.
 *        int srcw, destw: width of arrays to hold images.
 *        int simgw, dimgw: width of actual image to scale.
 *        int simgh, dimgh: height of actual image to scale.
 *        int bclr: background color index for letterboxing, or -1 for non-
 *                  fixed aspect ratio.
 *        short trans: transparency color.  If specified, all values of this
 *                     color are changed to 255, and all other values are set
 *                     to zero.  To prevent this, use -1.     11/23/94-DWM */
{
  long *horz, *vert, i, j, d, w, h;
  float stepx, stepy;

  w = dimgw;  h = simgh*w/simgw;
  if (h>dimgh || w==simgw*dimgh/simgh) {
    h = dimgh;  w = simgw*h/simgh;  if (w>dimgw) w = dimgw; }
  if (bclr==-1) { w = dimgw;  h = dimgh; }
  horz = malloc2((w+h)*sizeof(long));
  if (w!=dimgw || h!=dimgh || !horz)
    for (j=0; j<dimgh; j++)
      if (trans==-1)  memset(dest+destw*j, bclr, dimgw);
      else            memset(dest+destw*j, 0, dimgw);
  if (!horz)  return;
  vert = horz+w;
  stepx = stepy = min((float)simgw/w, (float)simgh/h);
  if (bclr==-1) { stepx = (float)simgw/w;  stepy = (float)simgh/h; }
  for (i=0; i<w; i++)
    horz[i] = floor(stepx*i+(simgw-(w-1)*stepx)/2);
  for (i=0; i<h; i++)
    vert[i] = (long)(floor(stepy*i+(simgh-(h-1)*stepy)/2))*srcw;
  dest += (dimgw-w)/2+(dimgh-h)/2*destw;
  if (trans==-1)
    for (j=d=0; j<h; j++, d+=destw-w)
      for (i=0; i<w; i++, d++)
        dest[d] = source[vert[j]+horz[i]];
  else
    for (j=d=0; j<h; j++, d+=destw-w)
      for (i=0; i<w; i++, d++)
        if (source[vert[j]+horz[i]]==trans)  dest[d] = 255;
        else                                 dest[d] = 0;
  free2(horz);
}

STATIC void scale_pic_prep(long *ind, long *fac, long w, long sw,
                           float invscale, long inter)
/* Prepare an array for scaling.  The interpolation mode can be 0-nearest
 *  neighbor, 1-linear, 2-quadratic, 3-cubic, 4-quartic, 5-quintic.
 *  Additionally, the following flags can be specified:
 *   0x1C00: 0-normal, 1-neutral spiff (gamma=1), 2-dark spiff, 3-medium
 *           spiff. 4-light spiff, 5-dark center spiff, 6-medium center
 *           spiff, 7-light center spiff
 *   0x6000: 0-area data (100% fill factor), 1-area data when expanded
 *           (invscale<1), block fill mode when compressed (invscale>1),
 *           2-point data (0% fill factor).
 *   0x8000: 0-only 24-bit images are scaled, 1-all images are scaled
 * Enter: long *ind: array of dw*(inter+1) values to specify offset locations
 *                   within the unscaled data to use in calculations.
 *        long *fac: array of dw*(inter+1) values to be multiplied by the
 *                   source data.  The result should be
 *                   SUM(source[ind[j]]*fac[j],j,0->inter)>>9.
 *        long w, sw: destination data width, source data width.
 *        float invscale: inverse of the scale (i.e., sw/dw).  This does not
 *                        have to match, since the arrays may not match
 *                        precisely or may be letterboxed.
 *        long inter: interpolation mode.  See above.         11/13/00-DWM */
{
  long i, j, k, f, px, w2, w3, w4, w5, data;
  double x;

  data = (inter>>13)&3;  inter &= 0xFF;
  w2 = w+w;  w3 = w2+w;  w4 = w3+w;  w5 = w4+w;
  for (i=0; i<w; i++) {
    if (inter) {
      if (data==2 || (invscale>1 && data==1))  x = invscale*i;
      else                                     x = invscale*(i+0.5)-0.5;
      px = floor(x);  x -= px; }
    else
      px = floor(invscale*i);
    if (inter && invscale>1 && data==1) {                    /* block mode */
      for (j=0; j<=inter ||j<invscale+2; j++) {
        k = j%(inter+1);
        if (!j)                  f = 0x1000000*(1-x)/invscale;
        else if (j<invscale+x-1) f = 0x1000000/invscale;
        else if (j<invscale+x)   f = 0x1000000*(1+(x-j)/invscale);
        else                     f = 0;
        if (j<=inter || f>fac[i+k*w]) {
          ind[i+k*w] = min(px+j,sw-1);  fac[i+k*w] = f; } }
      for (j=f=0; j<=inter; j++)
        f += fac[i+j*w];
      for (j=0, k=0; j<=inter; j++) {
        fac[i+j*w] = fac[i+j*w]*512./f;  k += fac[i+j*w]; }
      fac[i+w] += (512-k);
      continue; }
    switch (inter) {
      case 0: ind[i] = px; break;
      case 1: ind[i] = max(px,0);  ind[i+w] = min(px+1,sw-1);
        fac[i] = (1-x)*512;  fac[i+w] = 512-fac[i]; break;
      case 2: ind[i] = max(px-1,0);  ind[i+w] = max(px,0);
        ind[i+w2] = min(px+1,sw-1);
        fac[i] = (x*(x-1)/2)*512;  fac[i+w] = (1-x*x)*512;
        fac[i+w2] = (x*(x+1)/2)*512; break;
      case 3: ind[i] = max(px-1,0);  ind[i+w] = max(px,0);
        ind[i+w2] = min(px+1,sw-1);  ind[i+w3] = min(px+2,sw-1);
        fac[i] = -x*(x*x-3*x+2)/6*512;  fac[i+w] = (x*x*x-2*x*x-x+2)/2*512;
        fac[i+w2] = -x*(x*x-x-2)/2*512;  fac[i+w3] = x*(x*x-1)/6*512;
        break;
      case 4: ind[i] = max(px-2,0);  ind[i+w] = max(px-1,0);
        ind[i+w2] = max(px,0);  ind[i+w3] = min(px+1,sw-1);
        ind[i+w4] = min(px+2,sw-1);
        fac[i] = x*(x*x*x-2*x*x-x+2)/24*512;
        fac[i+w] = -x*(x*x*x-x*x-4*x+4)/6*512;
        fac[i+w2] = (x*x*x*x-5*x*x+4)/4*512;
        fac[i+w3] = -x*(x*x*x+x*x-4*x-4)/6*512;
        fac[i+w4] = x*(x*x*x+2*x*x-x-2)/24*512; break;
      case 5: ind[i] = max(px-2,0);  ind[i+w] = max(px-1,0);
        ind[i+w2] = max(px,0);  ind[i+w3] = min(px+1,sw-1);
        ind[i+w4] = min(px+2,sw-1);  ind[i+w5] = min(px+3,sw-1);
        fac[i] = -x*(x*x*x*x-5*x*x*x+5*x*x+5*x-6)/120*512;
        fac[i+w] = x*(x*x*x*x-4*x*x*x-x*x+16*x-12)/24*512;
        fac[i+w2] = -(x*x*x*x*x-3*x*x*x*x-5*x*x*x+15*x*x+4*x-12)/12*512;
        fac[i+w3] = x*(x*x*x*x-2*x*x*x-7*x*x+8*x+12)/12*512;
        fac[i+w4] = -x*(x*x*x*x-x*x*x-7*x*x+x+6)/24*512;
        fac[i+w5] = x*(x*x*x*x-5*x*x+4)/120*512; } }
}

STATIC void scale_pic24(uchar *dest, uchar *source, long srcw, long destw,
                        long simgw, long dimgw, long simgh, long dimgh,
                        long bclr, long trans)
/* Scale a 24-bit color image.  The global variable Interpolate affects how
 *  scaling is performed: 0-rows are dropped or duplicated, 1-bilinear
 *  interpolation.  Note that for bilinear interpolation, when the image is
 *  scaled down an even multiple, this is the same as mode 0.
 * If the source image is not the same shape as the destination image, the
 *  edges are letterboxed.  The image's aspect ratio is maintained.  If the
 *  background color is -1, then the image's aspect ratio is not maintained,
 *  and instead the image is adjusted to fit the destination shape.  All
 *  values are passed in terms of pixels, not bytes.
 * Enter: uchar *dest, *source: address values for images.
 *        int srcw, destw: width of arrays to hold images in pixels.
 *        int simgw, dimgw: width of actual image to scale.
 *        int simgh, dimgh: height of actual image to scale.
 *        long bclr: background color RGB value for letterboxing, or -1 for
 *                   non-fixed aspect ratio.
 *        long trans: transpanency color.  If specified, all values of this
 *                    color are changed to 0xFFFFFF, and all other values are
 *                    set to zero.  To prevent this, use -1.   2/28/96-DWM */
{
  long *horz, *vert, i, j, t, a, b, c, d, w, h, inter=0;
  long *hfac, *vfac, *crop;
  long w2, w3, w4, w5, h2, h3, h4, h5, hist[256];
  double stepx, stepy, gamma;
  short transgb;
  uchar transr, *s, *pic=0;

  if (Interpolate&0x1C00) if (pic=malloc2(simgw*simgh*3)) {   /* Autospiff */
    for (j=0, s=source; j<simgh; j++, s+=srcw*3)
      memcpy(pic+simgw*3*j, s, simgw*3);
    srcw = simgw;
    source = pic;
    memset(hist, 0, 256*sizeof(long));
    for (i=t=0; i<simgw*simgh*3; i+=3) {
      hist[(pic[i]*77+pic[i+1]*151+pic[i+2]*28)>>8]++;  t++; }
    for (a=i=0; a<256; a++) {
      i += hist[a];  if (i>0.003*t)  break; }
    for (b=255, i=0; b>=0; b--) {
      i += hist[b];  if (i>0.003*t)  break; }
    if ((Interpolate&0x1C00)>=0x1400) {
      memset(hist, 0, 256*sizeof(long));
      for (j=simgh/4, t=0; j<simgh*3/4; j++)
        for (i=simgw/4; i<simgw*3/4; i++) {
          hist[(pic[j*srcw*3+i*3]*77+pic[j*srcw*3+i*3+1]*151+
                pic[j*srcw*3+i*3+2]*28)>>8]++;  t++; } }
    for (c=i=0; c<256; c++) {
      i += hist[c];  if (i>0.5*t)  break; }
    if (b-a && c<b)  switch (Interpolate&0x1C00) {
      case 0x0800: case 0x1400: gamma = log(0.35)/log((double)(c-a)/(b-a));
        break;
      case 0x0C00: case 0x1800: gamma = log(0.50)/log((double)(c-a)/(b-a));
        break;
      case 0x1000: case 0x1C00: gamma = log(0.65)/log((double)(c-a)/(b-a));
        break;
      default: gamma = 1; }
    else             gamma = 1;
    if (gamma>10)  gamma = 10;  if (gamma<0.1)  gamma = 0.1;
    for (i=0; i<256; i++) {
      if (i<=a)       hist[i] = 0;
      else if (i>=b)  hist[i] = 255;
      else            hist[i] = pow((double)(i-a)/(b-a), gamma)*255; }
    for (i=0; i<simgw*simgh*3; i++)
      pic[i] = hist[pic[i]]; }
  inter = Interpolate&0xFF;
  if (trans!=-1 || inter<0 || inter>6)  inter = 0;
  w = dimgw;  h = simgh*w/simgw;
  if (h>dimgh || w==simgw*dimgh/simgh) {
    h = dimgh;  w = simgw*h/simgh;  if (w>dimgw)  w = dimgw; }
  if (bclr==-1) { w = dimgw;  h = dimgh; }
  if (w==simgw && h==simgh)  inter = 0;
  horz = malloc2((w+h)*sizeof(long)*(inter+1)*2+2304*sizeof(long));
  if (w!=dimgw || h!=dimgh || !horz)
    for (j=0; j<dimgh; j++)
      if (trans==-1)  fill_zone24(dest+destw*j*3, bclr, dimgw);
      else            memset(dest+destw*j*3, 0, dimgw*3);
  if (!horz) {
    free2(pic);  return; }
  hfac = horz+w*(inter+1);
  vert = hfac+w*(inter+1);
  vfac = vert+h*(inter+1);
  if (inter>1) {
    crop = vfac+h*(inter+1)+1024;
    for (i=-1024; i<1280; i++)  crop[i] = min(max(0,i),255); }
  stepx = stepy = min((double)simgw/w, (double)simgh/h);
  if (bclr==-1) { stepx = (double)simgw/w;  stepy = (double)simgh/h; }
  w2 = w+w;  w3 = w2+w;  w4 = w3+w;  w5 = w4+w;
  h2 = h+h;  h3 = h2+h;  h4 = h3+h;  h5 = h4+h;
  scale_pic_prep(horz,hfac, w, simgw, stepx, inter|(Interpolate&0xFFFFFF00));
  scale_pic_prep(vert,vfac, h, simgh, stepy, inter|(Interpolate&0xFFFFFF00));
  for (i=0; i<(w*(inter+1)); i++)  horz[i] *= 3;
  for (i=0; i<(h*(inter+1)); i++)  vert[i] *= 3*srcw;
  dest += ((dimgw-w)/2+(dimgh-h)/2*destw)*3;
  if (trans!=-1) {
    transgb = ((short *)&trans)[0];
    transr = ((uchar *)&trans)[2];
    for (j=d=0; j<h; j++, d+=(destw-w)*3)
      for (i=0; i<w; i++, d+=3) {
        if (((short *)(source+vert[j]+horz[i]))[0]==transgb &&
            source[vert[j]+horz[i]+2]==transr) {
          ((short *)(dest+d))[0] = -1;  dest[d+2] = 0xFF; }
        else {
          ((short *)(dest+d))[0] = 0;  dest[d+2] = 0; } } }
  else if (!inter)
    for (j=d=0; j<h; j++, d+=(destw-w)*3)
      for (i=0; i<w; i++, d+=3) {
        ((short *)(dest+d))[0] = ((short *)(source+vert[j]+horz[i]))[0];
        dest[d+2] = source[vert[j]+horz[i]+2]; }
  else for (c=0; c<3; c++)
    for (j=0,d=c,s=source+c; j<h; j++, d+=(destw-w)*3)
      switch (inter) {
        case 1: for (i=0; i<w; i++, d+=3)
          dest[d] = ((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w])*vfac[j+h])>>18;
          break;
        case 2: for (i=0; i<w; i++, d+=3)
          dest[d] = crop[((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w]+s[vert[j]+horz[i+w2]]*hfac[i+w2])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w]+s[vert[j+h]+horz[i+w2]]*hfac[i+w2])*vfac[j+h]+
                    (s[vert[j+h2]+horz[i]]*hfac[i]+s[vert[j+h2]+horz[i+w]]*hfac[i+w]+s[vert[j+h2]+horz[i+w2]]*hfac[i+w2])*vfac[j+h2])>>18];
          break;
        case 3: for (i=0; i<w; i++, d+=3)
          dest[d] = crop[((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w]+s[vert[j]+horz[i+w2]]*hfac[i+w2]+s[vert[j]+horz[i+w3]]*hfac[i+w3])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w]+s[vert[j+h]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h]+horz[i+w3]]*hfac[i+w3])*vfac[j+h]+
                    (s[vert[j+h2]+horz[i]]*hfac[i]+s[vert[j+h2]+horz[i+w]]*hfac[i+w]+s[vert[j+h2]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h2]+horz[i+w3]]*hfac[i+w3])*vfac[j+h2]+
                    (s[vert[j+h3]+horz[i]]*hfac[i]+s[vert[j+h3]+horz[i+w]]*hfac[i+w]+s[vert[j+h3]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h3]+horz[i+w3]]*hfac[i+w3])*vfac[j+h3])>>18];
          break;
        case 4: for (i=0; i<w; i++, d+=3)
          dest[d] = crop[((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w]+s[vert[j]+horz[i+w2]]*hfac[i+w2]+s[vert[j]+horz[i+w3]]*hfac[i+w3]+s[vert[j]+horz[i+w4]]*hfac[i+w4])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w]+s[vert[j+h]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h]+horz[i+w4]]*hfac[i+w4])*vfac[j+h]+
                    (s[vert[j+h2]+horz[i]]*hfac[i]+s[vert[j+h2]+horz[i+w]]*hfac[i+w]+s[vert[j+h2]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h2]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h2]+horz[i+w4]]*hfac[i+w4])*vfac[j+h2]+
                    (s[vert[j+h3]+horz[i]]*hfac[i]+s[vert[j+h3]+horz[i+w]]*hfac[i+w]+s[vert[j+h3]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h3]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h3]+horz[i+w4]]*hfac[i+w4])*vfac[j+h3]+
                    (s[vert[j+h4]+horz[i]]*hfac[i]+s[vert[j+h4]+horz[i+w]]*hfac[i+w]+s[vert[j+h4]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h4]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h4]+horz[i+w4]]*hfac[i+w4])*vfac[j+h4])>>18];
          break;
        case 5: for (i=0; i<w; i++, d+=3)
          dest[d] = crop[((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w]+s[vert[j]+horz[i+w2]]*hfac[i+w2]+s[vert[j]+horz[i+w3]]*hfac[i+w3]+s[vert[j]+horz[i+w4]]*hfac[i+w4]+s[vert[j]+horz[i+w5]]*hfac[i+w5])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w]+s[vert[j+h]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h]+horz[i+w4]]*hfac[i+w4]+s[vert[j+h]+horz[i+w5]]*hfac[i+w5])*vfac[j+h]+
                    (s[vert[j+h2]+horz[i]]*hfac[i]+s[vert[j+h2]+horz[i+w]]*hfac[i+w]+s[vert[j+h2]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h2]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h2]+horz[i+w4]]*hfac[i+w4]+s[vert[j+h2]+horz[i+w5]]*hfac[i+w5])*vfac[j+h2]+
                    (s[vert[j+h3]+horz[i]]*hfac[i]+s[vert[j+h3]+horz[i+w]]*hfac[i+w]+s[vert[j+h3]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h3]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h3]+horz[i+w4]]*hfac[i+w4]+s[vert[j+h3]+horz[i+w5]]*hfac[i+w5])*vfac[j+h3]+
                    (s[vert[j+h4]+horz[i]]*hfac[i]+s[vert[j+h4]+horz[i+w]]*hfac[i+w]+s[vert[j+h4]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h4]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h4]+horz[i+w4]]*hfac[i+w4]+s[vert[j+h4]+horz[i+w5]]*hfac[i+w5])*vfac[j+h4]+
                    (s[vert[j+h5]+horz[i]]*hfac[i]+s[vert[j+h5]+horz[i+w]]*hfac[i+w]+s[vert[j+h5]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h5]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h5]+horz[i+w4]]*hfac[i+w4]+s[vert[j+h5]+horz[i+w5]]*hfac[i+w5])*vfac[j+h5])>>18];
        break; }
  free2(pic);
  free2(horz);
}

STATIC long set_color_table(long size, long notlast)
/* Set the color table based on the colors used.  The ColorTable array must
 *  have either 0s to indicate unused colors or 1s to indicate used values.
 * Enter: long size: number of colors to allocate.  Usually 254.  Note that
 *                   the other two colors are always black and white.
 *        long notlast: 0 if this is the final call to make the table,
 *                      otherwise this is the starting number of this set of
 *                      colors.  -1 for the final call without actually
 *                      setting the palette.
 * Exit:  long exact: 1 if all of the colors were exactly fitted to the
 *                    palette, 0 otherwise.                    10/1/95-DWM */
{
  uchar xr[256], xg[256], xb[256], nr[256], ng[256], nb[256];
  long n=2, i, best, m, c, r, g, b, exact, changed=1;

  ColorTable[0] = ColorTable[65535] = 0;
  xr[1] = xb[1] = 62;  xg[1] = 63;  nr[1] = ng[1] = nb[1] = 0;
  while (n<size+1) {
    for (i=1, best=0; i<n; i++) {
      if (xg[i]-ng[i]>best) { m = i; c = 1; best = xg[i]-ng[i]; }
      if (xr[i]-nr[i]>best) { m = i; c = 0; best = xr[i]-nr[i]; }
      if (xb[i]-nb[i]>best) { m = i; c = 2; best = xb[i]-nb[i]; } }
    xr[n] = xr[m];  xg[n] = xg[m];  xb[n] = xb[m];
    nr[n] = nr[m];  ng[n] = ng[m];  nb[n] = nb[m];
    switch (c) {
      case 0: xr[m] = ((xr[m]+nr[m])/2)&0x3E;  nr[n] = xr[m]+2; break;
      case 1: xg[m] =  (xg[m]+ng[m])/2;        ng[n] = xg[m]+1; break;
      case 2: xb[m] = ((xb[m]+nb[m])/2)&0x3E;  nb[n] = xb[m]+2; }
    for (i=0; i<2; i++, m=n) {
      for (r=nr[m]; r<=xr[m]; r+=2)
        for (g=ng[m]; g<=xg[m]; g++)
          for (b=nb[m]; b<=xb[m]; b+=2)
            if (ColorTable[(r<<10)+(g<<5)+(b>>1)]) {
              nr[m] = r;  g = b = r = 64; }
      for (g=ng[m]; g<=xg[m]; g++)
        for (r=nr[m]; r<=xr[m]; r+=2)
          for (b=nb[m]; b<=xb[m]; b+=2)
            if (ColorTable[(r<<10)+(g<<5)+(b>>1)]) {
              ng[m] = g;  g = b = r = 64; }
      for (b=nb[m]; b<=xb[m]; b+=2)
        for (g=ng[m]; g<=xg[m]; g++)
          for (r=nr[m]; r<=xr[m]; r+=2)
            if (ColorTable[(r<<10)+(g<<5)+(b>>1)]) {
              nb[m] = b;  g = b = r = 64; }
      for (r=xr[m]; r>=nr[m]; r-=2)
        for (g=xg[m]; g>=ng[m]; g--)
          for (b=xb[m]; b>=nb[m]; b-=2)
            if (ColorTable[(r<<10)+(g<<5)+(b>>1)]) {
              xr[m] = r;  g = b = r = -2; }
      for (g=xg[m]; g>=ng[m]; g--)
        for (r=xr[m]; r>=nr[m]; r-=2)
          for (b=xb[m]; b>=nb[m]; b-=2)
            if (ColorTable[(r<<10)+(g<<5)+(b>>1)]) {
              xg[m] = g;  g = b = r = -2; }
      for (b=xb[m]; b>=nb[m]; b-=2)
        for (g=xg[m]; g>=ng[m]; g--)
          for (r=xr[m]; r>=nr[m]; r-=2)
            if (ColorTable[(r<<10)+(g<<5)+(b>>1)]) {
              xb[m] = b;  g = b = r = -2; } }
    for (r=nr[n]; r<=xr[n]; r+=2)
      for (g=ng[n]; g<=xg[n]; g++)
        for (b=nb[n]; b<=xb[n]; b+=2) {
          i = (r<<10)+(g<<5)+(b>>1);
          if (ColorTable[i]<n && ColorTable[i])
            ColorTable[i] = n; }
    n++; }
  for (i=1, best=0; i<n; i++) {
    if (xg[i]-ng[i]>best)  best = xg[i]-ng[i];
    if (xr[i]-nr[i]>best)  best = xr[i]-nr[i];
    if (xb[i]-nb[i]>best)  best = xb[i]-nb[i]; }
  exact = (!best);
  memset(MasterPal, 0, size*3+6);
  MasterPal[size*3+3] = MasterPal[size*3+4] = MasterPal[size*3+5] = 63;
  ColorTable[0] = 0;  ColorTable[65535] = size+1;
  if (notlast>0)  m = (notlast - 1)*3;
  else            m = 0;
  for (i=1; i<size+1; i++) {
    MasterPal[m+i*3]   = (xr[i]+nr[i]*1.03+1)/2;
    MasterPal[m+i*3+1] = (xg[i]+ng[i]+1)/2;
    MasterPal[m+i*3+2] = (xb[i]+nb[i]*1.03+1)/2; }
  if (notlast>0) {
    for (r=i=m=0; r<64; r+=2)
      for (g=0; g<64; g++)  for (b=0; b<64; b+=2, i++)
        if (ColorTable[i])  ColorTable[i] += notlast - 1;
    return(exact); }
  for (i=1; i<n; i++) {
    nr[i] >>= 1;  xr[i] >>= 1;  nb[i] >>= 1;  xb[i] >>= 1;
    nr[i] = max(0, nr[i]-1);  xr[i] = min(31, xr[i]+1);
    ng[i] = max(0, ng[i]-1);  xg[i] = min(63, xg[i]+1);
    nb[i] = max(0, nb[i]-1);  xb[i] = min(31, xb[i]+1); }
  while (changed) {
    changed = 0;
    for (i=1; i<n; i++)
      if (nr[i]!=0xFF) {
        for (c=0, r=nr[i], g=ng[i]; g<=xg[i]; g++)
          for (b=nb[i]; b<=xb[i]; b++)
            if (!ColorTable[(r<<11)+(g<<5)+b])
              ColorTable[(r<<11)+(g<<5)+b] = c = changed = i;
        for (r++; r<xr[i]; r++) {
          for (g=ng[i], b=nb[i]; b<=xb[i]; b++)
            if (!ColorTable[(r<<11)+(g<<5)+b])
              ColorTable[(r<<11)+(g<<5)+b] = c = changed = i;
          for (g++; g<xg[i]; g++) {
            b = nb[i];  if (!ColorTable[(r<<11)+(g<<5)+b])
                          ColorTable[(r<<11)+(g<<5)+b] = c = changed = i;
            b = xb[i];  if (!ColorTable[(r<<11)+(g<<5)+b])
                          ColorTable[(r<<11)+(g<<5)+b] = c = changed = i; }
          for (b=nb[i]; b<=xb[i] && g<=xg[i]; b++)
            if (!ColorTable[(r<<11)+(g<<5)+b])
              ColorTable[(r<<11)+(g<<5)+b] = c = changed = i; }
        for (g=ng[i]; g<=xg[i] && r<=xr[i]; g++)
          for (b=nb[i]; b<=xb[i]; b++)
            if (!ColorTable[(r<<11)+(g<<5)+b])
              ColorTable[(r<<11)+(g<<5)+b] = c = changed = i;
        if (!c) { nr[i] = 0xFF; }
        else {
          nr[i] = max(0, nr[i]-1);  xr[i] = min(31, xr[i]+1);
          ng[i] = max(0, ng[i]-1);  xg[i] = min(63, xg[i]+1);
          nb[i] = max(0, nb[i]-1);  xb[i] = min(31, xb[i]+1); } } }
  ColorTable[0] = 0;
  if (Gamma!=1)
    for (i=0; i<768; i++)
      MasterPal[i] = pow((float)MasterPal[i]/63, Gamma)*63.;
  return(exact);
}

STATIC long use_palette(uchar *pal)
/* Use a palette for palettization purposes.  This stores the palette in
 *  MasterPal and sets up the references in ColorTable.
 * Enter: uchar *pal: palette to use.
 * Exit:  long exact: 0 for insufficient memory, 2 for okay.   3/14/97-DWM */
{
  uchar xr[256], xg[256], xb[256], nr[256], ng[256], nb[256];
  long i, r, g, b, c, changed=1;

  if (!ColorTable)
    if (!(ColorTable=malloc2(65536))) return(0);
  memcpy(MasterPal, pal, 768);
  memset(ColorTable, 0, 65536);
  for (i=0; i<256; i++) {
    xr[i] = nr[i] = (pal[i*3]>>3);
    xg[i] = ng[i] = (pal[i*3+1]>>2);
    xb[i] = nb[i] = (pal[i*3+2]>>3); }
  while (changed) {
    changed = 0;
    for (i=1; i<256; i++)
      if (nr[i]!=0xFF) {
        for (c=0, r=nr[i], g=ng[i]; g<=xg[i]; g++)
          for (b=nb[i]; b<=xb[i]; b++)
            if (!ColorTable[(r<<11)+(g<<5)+b])
              ColorTable[(r<<11)+(g<<5)+b] = c = changed = i;
        for (r++; r<xr[i]; r++) {
          for (g=ng[i], b=nb[i]; b<=xb[i]; b++)
            if (!ColorTable[(r<<11)+(g<<5)+b])
              ColorTable[(r<<11)+(g<<5)+b] = c = changed = i;
          for (g++; g<xg[i]; g++) {
            b = nb[i];  if (!ColorTable[(r<<11)+(g<<5)+b])
                          ColorTable[(r<<11)+(g<<5)+b] = c = changed = i;
            b = xb[i];  if (!ColorTable[(r<<11)+(g<<5)+b])
                          ColorTable[(r<<11)+(g<<5)+b] = c = changed = i; }
          for (b=nb[i]; b<=xb[i] && g<=xg[i]; b++)
            if (!ColorTable[(r<<11)+(g<<5)+b])
              ColorTable[(r<<11)+(g<<5)+b] = c = changed = i; }
        for (g=ng[i]; g<=xg[i] && r<=xr[i]; g++)
          for (b=nb[i]; b<=xb[i]; b++)
            if (!ColorTable[(r<<11)+(g<<5)+b])
              ColorTable[(r<<11)+(g<<5)+b] = c = changed = i;
        if (!c) { nr[i] = 0xFF; }
        else {
          nr[i] = max(0, nr[i]-1);  xr[i] = min(31, xr[i]+1);
          ng[i] = max(0, ng[i]-1);  xg[i] = min(63, xg[i]+1);
          nb[i] = max(0, nb[i]-1);  xb[i] = min(31, xb[i]+1); } } }
  r = nr[0];  g = ng[0];  b = nb[0];
  ColorTable[(r<<11)+(g<<5)+b] = 0;
  return(2);
}
