#ifndef LIBRARY
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "process.h"
#include "gload.h"
#define STATIC
#endif

#define itag  ((short *)tag)
#define ltag  ((long *)tag)
#define utag  ((unsigned char *)tag)
#define uitag ((unsigned short *)tag)
#define ultag ((unsigned long *)tag)

STATIC char fcheck1[]="CPBackFile";
STATIC char outcode[4096], pspec[LENPSPEC], suffix[4096];
STATIC short prefix[4096];
STATIC long LoadPart=-1, Minpwidth=0, Minpheight=0, NumPart, perr;
STATIC long MAXGSIZE, pheight, pinter, ppart=0, pwidth;
STATIC uchar imask[]={0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1}, pquant[128];
STATIC char zagtbl[]={ 0, 1, 8,16, 9, 2, 3,10, 17,24,32,25,18,11, 4, 5,
                      12,19,26,33,40,48,41,34, 27,20,13, 6, 7,14,21,28,
                      35,42,49,56,57,50,43,36, 29,22,15,23,30,37,44,51,
                      58,59,52,45,38,31,39,46, 53,60,61,54,47,55,62,63};
STATIC FILE *(*fopen2)(char *name, char *mode)=fopen;
STATIC size_t (*fread2)(void *buffer, size_t size, size_t count, FILE *fptr)=fread;
STATIC size_t (*fwrite2)(void *buffer, size_t size, size_t count,FILE *fptr)=fwrite;
STATIC int (*fclose2)(FILE *fptr)=fclose;
STATIC int (*fseek2)(FILE *fptr, long offset, int origin)=fseek;
STATIC long (*ftell2)(FILE *fptr)=ftell;
STATIC char software[TIF_MAXSOFTWARELEN], serial[TIF_MAXSERIALLEN],
       tifmodel[TIF_MAXMODELLEN];
STATIC long ccittcode[]={
          0x1000 /* 0b0001000000000000 */,  4,    0,
          0x2000 /* 0b0010000000000000 */,  3,    1,
          0x8000 /* 0b1000000000000000 */,  1,    2,
          0x6000 /* 0b0110000000000000 */,  3,   11,
          0x0c00 /* 0b0000110000000000 */,  6,   12,
          0x0600 /* 0b0000011000000000 */,  7,   13,
          0x4000 /* 0b0100000000000000 */,  3,   21,
          0x0800 /* 0b0000100000000000 */,  6,   22,
          0x0400 /* 0b0000010000000000 */,  7,   23,
          0x0200 /* 0b0000001000000000 */,  7,    3,
          0x0000 /* 0b0000000000000000 */,  7,   -1, -2,-2,-2};
STATIC long ccittwhite[]={
          0x3500 /* 0b0011010100000000 */,  8,    0,
          0x1c00 /* 0b0001110000000000 */,  6,    1,
          0x7000 /* 0b0111000000000000 */,  4,    2,
          0x8000 /* 0b1000000000000000 */,  4,    3,
          0xb000 /* 0b1011000000000000 */,  4,    4,
          0xc000 /* 0b1100000000000000 */,  4,    5,
          0xe000 /* 0b1110000000000000 */,  4,    6,
          0xf000 /* 0b1111000000000000 */,  4,    7,
          0x9800 /* 0b1001100000000000 */,  5,    8,
          0xa000 /* 0b1010000000000000 */,  5,    9,
          0x3800 /* 0b0011100000000000 */,  5,   10,
          0x4000 /* 0b0100000000000000 */,  5,   11,
          0x2000 /* 0b0010000000000000 */,  6,   12,
          0x0c00 /* 0b0000110000000000 */,  6,   13,
          0xd000 /* 0b1101000000000000 */,  6,   14,
          0xd400 /* 0b1101010000000000 */,  6,   15,
          0xa800 /* 0b1010100000000000 */,  6,   16,
          0xac00 /* 0b1010110000000000 */,  6,   17,
          0x4e00 /* 0b0100111000000000 */,  7,   18,
          0x1800 /* 0b0001100000000000 */,  7,   19,
          0x1000 /* 0b0001000000000000 */,  7,   20,
          0x2e00 /* 0b0010111000000000 */,  7,   21,
          0x0600 /* 0b0000011000000000 */,  7,   22,
          0x0800 /* 0b0000100000000000 */,  7,   23,
          0x5000 /* 0b0101000000000000 */,  7,   24,
          0x5600 /* 0b0101011000000000 */,  7,   25,
          0x2600 /* 0b0010011000000000 */,  7,   26,
          0x4800 /* 0b0100100000000000 */,  7,   27,
          0x3000 /* 0b0011000000000000 */,  7,   28,
          0x0200 /* 0b0000001000000000 */,  8,   29,
          0x0300 /* 0b0000001100000000 */,  8,   30,
          0x1a00 /* 0b0001101000000000 */,  8,   31,
          0x1b00 /* 0b0001101100000000 */,  8,   32,
          0x1200 /* 0b0001001000000000 */,  8,   33,
          0x1300 /* 0b0001001100000000 */,  8,   34,
          0x1400 /* 0b0001010000000000 */,  8,   35,
          0x1500 /* 0b0001010100000000 */,  8,   36,
          0x1600 /* 0b0001011000000000 */,  8,   37,
          0x1700 /* 0b0001011100000000 */,  8,   38,
          0x2800 /* 0b0010100000000000 */,  8,   39,
          0x2900 /* 0b0010100100000000 */,  8,   40,
          0x2a00 /* 0b0010101000000000 */,  8,   41,
          0x2b00 /* 0b0010101100000000 */,  8,   42,
          0x2c00 /* 0b0010110000000000 */,  8,   43,
          0x2d00 /* 0b0010110100000000 */,  8,   44,
          0x0400 /* 0b0000010000000000 */,  8,   45,
          0x0500 /* 0b0000010100000000 */,  8,   46,
          0x0a00 /* 0b0000101000000000 */,  8,   47,
          0x0b00 /* 0b0000101100000000 */,  8,   48,
          0x5200 /* 0b0101001000000000 */,  8,   49,
          0x5300 /* 0b0101001100000000 */,  8,   50,
          0x5400 /* 0b0101010000000000 */,  8,   51,
          0x5500 /* 0b0101010100000000 */,  8,   52,
          0x2400 /* 0b0010010000000000 */,  8,   53,
          0x2500 /* 0b0010010100000000 */,  8,   54,
          0x5800 /* 0b0101100000000000 */,  8,   55,
          0x5900 /* 0b0101100100000000 */,  8,   56,
          0x5a00 /* 0b0101101000000000 */,  8,   57,
          0x5b00 /* 0b0101101100000000 */,  8,   58,
          0x4a00 /* 0b0100101000000000 */,  8,   59,
          0x4b00 /* 0b0100101100000000 */,  8,   60,
          0x3200 /* 0b0011001000000000 */,  8,   61,
          0x3300 /* 0b0011001100000000 */,  8,   62,
          0x3400 /* 0b0011010000000000 */,  8,   63,
          0xd800 /* 0b1101100000000000 */,  5,   64,
          0x9000 /* 0b1001000000000000 */,  5,  128,
          0x5c00 /* 0b0101110000000000 */,  6,  192,
          0x6e00 /* 0b0110111000000000 */,  7,  256,
          0x3600 /* 0b0011011000000000 */,  8,  320,
          0x3700 /* 0b0011011100000000 */,  8,  384,
          0x6400 /* 0b0110010000000000 */,  8,  448,
          0x6500 /* 0b0110010100000000 */,  8,  512,
          0x6800 /* 0b0110100000000000 */,  8,  576,
          0x6700 /* 0b0110011100000000 */,  8,  640,
          0x6600 /* 0b0110011000000000 */,  9,  704,
          0x6680 /* 0b0110011010000000 */,  9,  768,
          0x6900 /* 0b0110100100000000 */,  9,  832,
          0x6980 /* 0b0110100110000000 */,  9,  896,
          0x6a00 /* 0b0110101000000000 */,  9,  960,
          0x6a80 /* 0b0110101010000000 */,  9, 1024,
          0x6b00 /* 0b0110101100000000 */,  9, 1088,
          0x6b80 /* 0b0110101110000000 */,  9, 1152,
          0x6c00 /* 0b0110110000000000 */,  9, 1216,
          0x6c80 /* 0b0110110010000000 */,  9, 1280,
          0x6d00 /* 0b0110110100000000 */,  9, 1344,
          0x6d80 /* 0b0110110110000000 */,  9, 1408,
          0x4c00 /* 0b0100110000000000 */,  9, 1472,
          0x4c80 /* 0b0100110010000000 */,  9, 1536,
          0x4d00 /* 0b0100110100000000 */,  9, 1600,
          0x6000 /* 0b0110000000000000 */,  6, 1664,
          0x4d80 /* 0b0100110110000000 */,  9, 1728,
          0x0100 /* 0b0000000100000000 */, 11, 1792,
          0x0180 /* 0b0000000110000000 */, 11, 1856,
          0x01a0 /* 0b0000000110100000 */, 11, 1920,
          0x0120 /* 0b0000000100100000 */, 12, 1984,
          0x0130 /* 0b0000000100110000 */, 12, 2048,
          0x0140 /* 0b0000000101000000 */, 12, 2112,
          0x0150 /* 0b0000000101010000 */, 12, 2176,
          0x0160 /* 0b0000000101100000 */, 12, 2240,
          0x0170 /* 0b0000000101110000 */, 12, 2304,
          0x01c0 /* 0b0000000111000000 */, 12, 2368,
          0x01d0 /* 0b0000000111010000 */, 12, 2432,
          0x01e0 /* 0b0000000111100000 */, 12, 2496,
          0x01f0 /* 0b0000000111110000 */, 12, 2560,
          0x0010 /* 0b0000000000010000 */, 12,   -1, -2,-2,-2};
STATIC long ccittblack[]={
          0x0dc0 /* 0b0000110111000000 */, 10,    0,
          0x4000 /* 0b0100000000000000 */,  3,    1,
          0xc000 /* 0b1100000000000000 */,  2,    2,
          0x8000 /* 0b1000000000000000 */,  2,    3,
          0x6000 /* 0b0110000000000000 */,  3,    4,
          0x3000 /* 0b0011000000000000 */,  4,    5,
          0x2000 /* 0b0010000000000000 */,  4,    6,
          0x1800 /* 0b0001100000000000 */,  5,    7,
          0x1400 /* 0b0001010000000000 */,  6,    8,
          0x1000 /* 0b0001000000000000 */,  6,    9,
          0x0800 /* 0b0000100000000000 */,  7,   10,
          0x0a00 /* 0b0000101000000000 */,  7,   11,
          0x0e00 /* 0b0000111000000000 */,  7,   12,
          0x0400 /* 0b0000010000000000 */,  8,   13,
          0x0700 /* 0b0000011100000000 */,  8,   14,
          0x0c00 /* 0b0000110000000000 */,  9,   15,
          0x05c0 /* 0b0000010111000000 */, 10,   16,
          0x0600 /* 0b0000011000000000 */, 10,   17,
          0x0200 /* 0b0000001000000000 */, 10,   18,
          0x0ce0 /* 0b0000110011100000 */, 11,   19,
          0x0d00 /* 0b0000110100000000 */, 11,   20,
          0x0d80 /* 0b0000110110000000 */, 11,   21,
          0x06e0 /* 0b0000011011100000 */, 11,   22,
          0x0500 /* 0b0000010100000000 */, 11,   23,
          0x02e0 /* 0b0000001011100000 */, 11,   24,
          0x0300 /* 0b0000001100000000 */, 11,   25,
          0x0ca0 /* 0b0000110010100000 */, 12,   26,
          0x0cb0 /* 0b0000110010110000 */, 12,   27,
          0x0cc0 /* 0b0000110011000000 */, 12,   28,
          0x0cd0 /* 0b0000110011010000 */, 12,   29,
          0x0680 /* 0b0000011010000000 */, 12,   30,
          0x0690 /* 0b0000011010010000 */, 12,   31,
          0x06a0 /* 0b0000011010100000 */, 12,   32,
          0x06b0 /* 0b0000011010110000 */, 12,   33,
          0x0d20 /* 0b0000110100100000 */, 12,   34,
          0x0d30 /* 0b0000110100110000 */, 12,   35,
          0x0d40 /* 0b0000110101000000 */, 12,   36,
          0x0d50 /* 0b0000110101010000 */, 12,   37,
          0x0d60 /* 0b0000110101100000 */, 12,   38,
          0x0d70 /* 0b0000110101110000 */, 12,   39,
          0x06c0 /* 0b0000011011000000 */, 12,   40,
          0x06d0 /* 0b0000011011010000 */, 12,   41,
          0x0da0 /* 0b0000110110100000 */, 12,   42,
          0x0db0 /* 0b0000110110110000 */, 12,   43,
          0x0540 /* 0b0000010101000000 */, 12,   44,
          0x0550 /* 0b0000010101010000 */, 12,   45,
          0x0560 /* 0b0000010101100000 */, 12,   46,
          0x0570 /* 0b0000010101110000 */, 12,   47,
          0x0640 /* 0b0000011001000000 */, 12,   48,
          0x0650 /* 0b0000011001010000 */, 12,   49,
          0x0520 /* 0b0000010100100000 */, 12,   50,
          0x0530 /* 0b0000010100110000 */, 12,   51,
          0x0240 /* 0b0000001001000000 */, 12,   52,
          0x0370 /* 0b0000001101110000 */, 12,   53,
          0x0380 /* 0b0000001110000000 */, 12,   54,
          0x0270 /* 0b0000001001110000 */, 12,   55,
          0x0280 /* 0b0000001010000000 */, 12,   56,
          0x0580 /* 0b0000010110000000 */, 12,   57,
          0x0590 /* 0b0000010110010000 */, 12,   58,
          0x02b0 /* 0b0000001010110000 */, 12,   59,
          0x02c0 /* 0b0000001011000000 */, 12,   60,
          0x05a0 /* 0b0000010110100000 */, 12,   61,
          0x0660 /* 0b0000011001100000 */, 12,   62,
          0x0670 /* 0b0000011001110000 */, 12,   63,
          0x03c0 /* 0b0000001111000000 */, 10,   64,
          0x0c80 /* 0b0000110010000000 */, 12,  128,
          0x0c90 /* 0b0000110010010000 */, 12,  192,
          0x05b0 /* 0b0000010110110000 */, 12,  256,
          0x0330 /* 0b0000001100110000 */, 12,  320,
          0x0340 /* 0b0000001101000000 */, 12,  384,
          0x0350 /* 0b0000001101010000 */, 12,  448,
          0x0360 /* 0b0000001101100000 */, 13,  512,
          0x0368 /* 0b0000001101101000 */, 13,  576,
          0x0250 /* 0b0000001001010000 */, 13,  640,
          0x0258 /* 0b0000001001011000 */, 13,  704,
          0x0260 /* 0b0000001001100000 */, 13,  768,
          0x0268 /* 0b0000001001101000 */, 13,  832,
          0x0390 /* 0b0000001110010000 */, 13,  896,
          0x0398 /* 0b0000001110011000 */, 13,  960,
          0x03a0 /* 0b0000001110100000 */, 13, 1024,
          0x03a8 /* 0b0000001110101000 */, 13, 1088,
          0x03b0 /* 0b0000001110110000 */, 13, 1152,
          0x03b8 /* 0b0000001110111000 */, 13, 1216,
          0x0290 /* 0b0000001010010000 */, 13, 1280,
          0x0298 /* 0b0000001010011000 */, 13, 1344,
          0x02a0 /* 0b0000001010100000 */, 13, 1408,
          0x02a8 /* 0b0000001010101000 */, 13, 1472,
          0x02d0 /* 0b0000001011010000 */, 13, 1536,
          0x02d8 /* 0b0000001011011000 */, 13, 1600,
          0x0320 /* 0b0000001100100000 */, 13, 1664,
          0x0328 /* 0b0000001100101000 */, 13, 1728,
          0x0100 /* 0b0000000100000000 */, 11, 1792,
          0x0180 /* 0b0000000110000000 */, 11, 1856,
          0x01a0 /* 0b0000000110100000 */, 11, 1920,
          0x0120 /* 0b0000000100100000 */, 12, 1984,
          0x0130 /* 0b0000000100110000 */, 12, 2048,
          0x0140 /* 0b0000000101000000 */, 12, 2112,
          0x0150 /* 0b0000000101010000 */, 12, 2176,
          0x0160 /* 0b0000000101100000 */, 12, 2240,
          0x0170 /* 0b0000000101110000 */, 12, 2304,
          0x01c0 /* 0b0000000111000000 */, 12, 2368,
          0x01d0 /* 0b0000000111010000 */, 12, 2432,
          0x01e0 /* 0b0000000111100000 */, 12, 2496,
          0x01f0 /* 0b0000000111110000 */, 12, 2560,
          0x0000 /* 0b0000000000000000 */, 11,   -1, -2,-2,-2};

STATIC void bgr_to_rgb(uchar *image, long siz)
/* Convert a packed array of BGR values to RGB values.
 * Enter: uchar *image: pointer to array to convert.
 *        long size: number of values to convert.  Each value is 3 bytes.
 *                                                             2/26/96-DWM */
{
  _asm {
          mov esi, image
          mov ecx, siz
bgrrgb1:  mov al, [esi]
          xchg al, [esi+2]
          mov [esi], al
          add esi, 03
          loop   bgrrgb1
    }
}

STATIC short endian(short *x)
/* Convert the value stored in x from big endian (Motorola format) to little
 *  endian (Intel format).
 * Enter: short *x: pointer to location to convert.  This value is changed.
 * Exit:  short val: value stored in x.                        3/17/95-DWM */
{
  uchar *y=(char *)x, z[2];

  z[0] = y[1];  z[1] = y[0];
  return(x[0]=((short *)z)[0]);
}

STATIC long endianl(long *x)
/* Convert the value stored in x from big endian (Motorola format) to little
 *  endian (Intel format).
 * Enter: long *x: pointer to location to convert.  This value is changed.
 * Exit:  long val: value stored in x.                         3/17/95-DWM */
{
  uchar *y=(char *)x, z[4];

  z[0] = y[3];  z[1] = y[2];  z[2] = y[1];  z[3] = y[0];
  return(x[0]=((long *)z)[0]);
}

STATIC void hor_flip(uchar *image, long w, long h, long pix)
/* Flip an image horizontally.
 * Enter: uchar *image: pointer to the image array to flip.
 *        long w, h: size of the image in pixels.
 *        long pix: the width of a pixel in bytes.             2/28/96-DWM */
{
  char *buf;
  long i, j, k;

  if (!(buf=malloc2(w))) {
    perr = 0x306;  return; }
  for (j=0; j<h; j++) {
    memmove(buf, image+j*w*pix, w*pix);
    for (i=0; i<w; i++)
      for (k=0; k<pix; k++)
        image[j*w*pix+i*pix+k] = buf[(w-i-1)*pix+k]; }
  free2(buf);
}

STATIC long jpeg_decode(uchar *src, long offset, long len, uchar *dchuff,
       uchar *dchlen, uchar *achuff, uchar *achlen, uchar *quant, long *dest)
/* Decode a 8x8 block from a jpeg file.
 * Enter: uchar *src: pointer to source data.
 *        long offset: offset into source data in terms of bits.
 *        long len: length of source data in bytes.
 *        uchar *dchuff: table for DC huffman conversion.
 *        uchar *dchlen: table for length of DC huffman codes.
 *        uchar *achuff: table for AC huffman conversion.
 *        uchar *achlen: table for length of AC huffman codes.
 *        uchar *quant: quantization table.
 *        long *dest: location to store resultant values in.  Note that these
 *                    values have been unquantized and unzigzaged.
 * Exit:  long offset: new offset value in bits.                3/3/96-DWM */
{
  long bit, p, neg, d=1;
  ulong val, val2;
  static short nvaltbl[]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,
                          16383,32767};
  static short pvaltbl[]={0,1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,
                          16384};

  memset(dest, 0, 64*sizeof(long));
  p = offset>>3;  bit = offset&7;
  val = (((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit))&0xFFFF;
  val2 = dchuff[val];
  bit += dchlen[val2];
  while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); }
  if (val2) {
    val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
    neg = !(val&0x8000);
    val = (val&0x7FFF)>>(16-val2);
    if (neg)  dest[0] = (val-nvaltbl[val2])*quant[0];
    else      dest[0] = (pvaltbl[val2]+val)*quant[0];
    bit += val2;
    while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); } }
  while (d<64) {
    val = (((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit))&0xFFFF;
    val2 = achuff[val];
    bit += achlen[val2];
    if (!val2)  return((p<<3)+bit);
    while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); }
    d += (val2>>4);  val2 &= 0xF;
    if (val2 && d<64) {
      val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
      neg = !(val&0x8000);
      val = (val&0x7FFF)>>(16-val2);
      if (neg)  dest[zagtbl[d]] = (val-nvaltbl[val2])*quant[d];
      else      dest[zagtbl[d]] = (pvaltbl[val2]+val)*quant[d];
      bit += val2;
      while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); } }
    d++; }
  return((p<<3)+bit);
}

STATIC long jpeg_decode_spectral(uchar *src, long offset, long len,
       uchar *dchuff, uchar *dchlen, uchar *achuff, uchar *achlen,
       uchar *quant, long *dest, long spec0, long spec1, long first,
       long succ)
/* Decode a limit number of spectral components from an 8x8 block from a jpeg
 *  file.
 * Enter: uchar *src: pointer to source data.
 *        long offset: offset into source data in terms of bits.
 *        long len: length of source data in bytes.
 *        uchar *dchuff: table for DC huffman conversion.
 *        uchar *dchlen: table for length of DC huffman codes.
 *        uchar *achuff: table for AC huffman conversion.
 *        uchar *achlen: table for length of AC huffman codes.
 *        uchar *quant: quantization table.
 *        long *dest: location to store resultant values in.  Note that these
 *                    values have been unquantized and unzigzaged.
 *        long spec0, spec1: spectral component range to decode.  spec1 must
 *                           be greater than or equal to spec0.  Both must be
 *                           in the range [0,63].
 *        long first: 1 if this is the first scan that is decoded or the
 *                    first block of a subsequent scan.  0 otherwise.
 *        long succ: successive approximation.  0 for none.  Otherwise this
 *                   is a two digit hexadecimal number 0xAB, where the
 *                   current data is value/(2^A), and the new adjusted data
 *                   is value/(2^B).
 * Exit:  long offset: new offset value in bits.               1/27/01-DWM */
{
  long bit, p, neg, d=1, i, j, k;
  ulong val, val2;
  static short nvaltbl[]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,
                          16383,32767};
  static short pvaltbl[]={0,1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,
                          16384,32768};
  static long run;

  p = offset>>3;  bit = offset&7;
  if (succ) {
    if (((succ>>4)&0xF)>(succ&0xF)) {
      j = ((succ>>4)&0xF)-(succ&0xF);
      for (i=spec0; i<=spec1; i++)  dest[i] <<= j; }
    succ = (succ>>4)&0xF; }
  if (!spec0) {
    if (!succ) {
      val = (((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit))&0xFFFF;
      val2 = dchuff[val];
      bit += dchlen[val2];
      while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); }
      if (val2) {
        val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
        neg = !(val&0x8000);
        val = (val&0x7FFF)>>(16-val2);
        if (neg)  dest[0] = (val-nvaltbl[val2])*quant[0];
        else      dest[0] = (pvaltbl[val2]+val)*quant[0];
        bit += val2;
        while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); } } }
    else {
      val = (src[p]<<8)>>(8-bit);
      bit++;
      while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); }
      if (val&0x80)  dest[0]++; } }
  if (!first && run) {
    if (succ && spec0)
      for (d=spec0; d<=spec1; d++)
        if (dest[zagtbl[d]]) {
          val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
          if (val&0x8000) {
            if (dest[zagtbl[d]]>0)  dest[zagtbl[d]] += quant[d];
            else                    dest[zagtbl[d]] -= quant[d]; }
          bit++;
          while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); } }
    run--;  return((p<<3)+bit); }
  if (spec0)  d = spec0;
  run = 0;
  while (d<=spec1) {
    val = (((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit))&0xFFFF;
    val2 = achuff[val];
    bit += achlen[val2];
    while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); }
    if (!(val2&0xF) && (val2&0xF0)!=0xF0) {
      val2 >>= 4;
      val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
      val = ((val&0xFFFF)>>(16-val2))+pvaltbl[val2+1];
      bit += val2;
      while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); }
      run = val-1;
      if (succ)
        for (; d<=spec1; d++)
          if (dest[zagtbl[d]]) {
            val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
            if (val&0x8000) {
              if (dest[zagtbl[d]]>0)  dest[zagtbl[d]] += quant[d];
              else                    dest[zagtbl[d]] -= quant[d]; }
            bit++;
            while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); } }
      return((p<<3)+bit); }
    if (!succ) {
      d += (val2>>4);  val2 &= 0xF;
      if (val2 && d<=spec1) {
        val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
        neg = !(val&0x8000);
        val = (val&0x7FFF)>>(16-val2);
        if (neg)  dest[zagtbl[d]] = (val-nvaltbl[val2])*quant[d];
        else      dest[zagtbl[d]] = (pvaltbl[val2]+val)*quant[d]; }
      bit += val2;
      while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); } }
    else {
      i = (val2>>4)+1;  j = d;
      for (k=0; i && d<=spec1+1; )
        if (dest[zagtbl[d]]) { d++;  k++; }
        else                 { d++;  i--; }
      d--;
      if (val2&0xF) {
        val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
        if (val&0x8000)  dest[zagtbl[d]] = quant[d];
        else             dest[zagtbl[d]] = -quant[d];
        bit++;
        while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); } }
      for (i=0; i<k; i++) {
        while (!dest[zagtbl[j]]) j++;
        val = ((src[p]<<16)+(src[p+1]<<8)+src[p+2])>>(8-bit);
        if (val&0x8000) {
          if (dest[zagtbl[j]]>0)  dest[zagtbl[j]] += quant[j];
          else                    dest[zagtbl[j]] -= quant[j]; }
        bit++;
        while (bit>=8) { bit -= 8; p++; if (p>=len) return(len<<3); }
        j++; } }
    d++; }
  return((p<<3)+bit);
}

STATIC void jpeg_shift(uchar *dest, long *source)
/* Divide a set of 64 long signed values by 1024*1024*2, and crop the result
 *  to -128/+127.
 * Enter: uchar *dest: byte array to store results.
 *        long *source: source array of values to convert.      3/5/96-DWM */
{
  _asm {
          mov edi, dest
          mov esi, source
          mov ecx, 0x040
jshift1:  mov eax, [esi]
          sar eax, 0x13
          cmp ax, 0x80
          jl      jshift2
          mov al, 0x7F
          jmp     jshift3
jshift2:  cmp ax, 0xFF80
          jge     jshift3
          mov al, 0x80
jshift3:  mov [edi], al
          inc edi
          add esi, 0x04
          loop    jshift1
    }
}

STATIC void jpeg_shift_add(uchar *dest, long *source)
/* Divide a set of 64 long signed values by 1024*1024/2, add 128, and crop the
 *  result to 0-255.
 * Enter: uchar *dest: byte array to store results.
 *        long *source: source array of values to convert.      3/7/96-DWM */
{
  _asm {
          mov edi, dest
          mov esi, source
          mov ecx, 0x40
jshifta1: mov eax, [esi]
          sar eax, 0x13
          add ax, 0x80
          cmp ax, 0x100
          jb      jshifta3
          jge     jshifta2
          xor al, al
          jmp     jshifta3
jshifta2: mov al, 0xFF
jshifta3: mov [edi], al
          inc edi
          add esi, 0x04
          loop    jshifta1
    }
}

STATIC void jpeg_undct(long *res, char *dest, long w, long rw, long rh, long add)
/* Unquantize and unDCT a 8x8 block from a JPEG file.  The results are in the
 *  0-255 range.
 * Enter: long *res: pointer to 64 values to process.
 *        char *dest: location to store result.
 *        long w: width of the destination array.
 *        long rw: number of times to replicate values in the x direction.
 *                 Note that the total width of the destination array is
 *                 8*rw.
 *        long rh: number of times to replicate values in the y direction.
 *        long add: 0 to leave values signed, 128 to add 128 to values.
 *                                                              3/3/96-DWM */
{
  long i, j, s, d, d2, oldd, x, y, v, jd;
  uchar out[64];

  if (Minpwidth<rw || Minpwidth<rh)
  {
    for (i=0; i<64; i+=8)
      undct_1d(res+i, res+i);
    for (i=0; i<8; i++)
      undct_1d_alt(res+i, res+i);
    if (add) jpeg_shift_add(out, res);
    else     jpeg_shift(out, res);
  }
  else
  {
    j = (res[0]>>3);
    if (add)  j = min(max(j+128, 0), 255);
    else      j = min(max(j, -128), 127);
    for (i=0; i<64; i++)
      out[i] = j;
  }
  if (rw==1 && rh==1) {
    for (j=s=d=0; j<8; j++,d+=w,s+=8)
      memcpy(dest+d, out+s, 8);
    return; }
  jd = w*rh-(rw<<3);
  for (j=s=d=0; j<8; j++,d+=jd) {
    oldd = d;
    switch (rw) {
      case 1: memcpy(dest+d, out+s, 8);  s+=8;  d+=8; break;
      case 2: for (i=0; i<8; i++,s++,d+=rw)
          dest[d] = dest[d+1] = out[s]; break;
      case 4: for (i=0; i<8; i++,s++,d+=rw) {
          dest[d] = dest[d+1] = out[s];
          ((short *)(dest+d))[1] = ((short *)(dest+d))[0]; } break;
      case 8: for (i=0; i<8; i++,s++,d+=rw) {
          dest[d] = dest[d+1] = out[s];
          ((short *)(dest+d))[1] = ((short *)(dest+d))[0];
          ((long *)(dest+d))[1] = ((long *)(dest+d))[0]; } break;
      default: for (i=0; i<8; i++,s++,d+=rw) {
        v = out[s];
        for (x=0; x<rw; x++)
          dest[d+x] = v; } }
    for (y=1,d2=oldd+w; y<rh; y++,d2+=w)
      memcpy(dest+d2, dest+oldd, rw<<3); }
}

STATIC long jpeg_unstuff(uchar *jpg, long len, long start)
/* Condense a jpeg image stream so that it only has image data.  This removes
 *  markers and removes the 'stuffed' zero bytes which occur.  Basically, an
 *  0xFF byte signals a marker.  If followed by a zero, the zero is a dummy
 *  value just to claim that this isn't a marker.
 * Enter: uchar *jpg: pointer to array for source and destination.
 *        long len: length of the source array.
 *        long start: position within array to start unstuffing.
 * Exit:  long len: new length of the data.  The data is written at the
 *                  beginning of the array.                     3/3/96-DWM */
{
  long s=start, d=0, diff;
  uchar *temp;

  while (s<len) {
    temp = memchr(jpg+s, 0xFF, len-s);
    if (!temp) {
      memmove(jpg+d, jpg+s, len-s);
      d += len-s;
      return(d); }
    diff = temp-jpg-s;
    if (diff) {
      memmove(jpg+d, jpg+s, diff);
      d += diff;  s += diff; }
    if (s+1>=len) {
      jpg[d] = 0xFF;
      return(d+1); }
    if (!jpg[s+1]) {
      jpg[d] = 0xFF;  d ++;
      s += 2; }
    else if (jpg[s+1]>=0xD0 && jpg[s+1]<=0xD7) {
      jpg[d] = 0xFF;  jpg[d+1] = jpg[s+1];  d += 2;
      s += 2; }
    else if (jpg[s+1]==0xFF)
      s ++;
    else
      return(d); }
  return(d);
}

STATIC uchar *load_bmp(FILE *fptr)
/* Load in a BMP/DIB file.  Exits with values in pwidth and pheight.
 *  pinter=1 for palette images and 0 for RGB images.  If an error occurs a
 *  null is passed back.  See load_graphic for return data format.  The file
 *  pointer is closed.
 * Enter: FILE *fptr: pointer to the open BMP file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/25/96-DWM */
{
  static uchar *new, *bmp, head[54];
  static long bits, type, comp, fsize, size, i, j, s, d, flip;
  static long bitfield[6];
  long place=0, dpi;

  if (LoadPart<=0) {
    comp = fsize = 0;  flip = 1;
    (*fread2)(head, 1, 54, fptr);
    type = ((long *)(head+14))[0];
    place = ((long *)(head+10))[0];
    if (type==40) {
      pwidth = ((long *)(head+18))[0];
      pheight = ((long *)(head+18))[1];
      bits = ((short *)(head+28))[0];
      comp = ((long *)(head+30))[0];
      fsize = ((long *)(head+34))[0];
      if (((long *)(head+46))[0] && !place)
        place = 54+4*((long *)(head+46))[0]; }
    else if (type==12) {
      pwidth = ((short *)(head+18))[0];
      pheight = ((short *)(head+18))[1];
      bits = ((short *)(head+24))[0];
      (*fseek2)(fptr, -28, SEEK_CUR); }
    else {
      perr = 0x403;  (*fclose2)(fptr);  return(0); }
    if (pheight<0) {  flip = 0;  pheight *= -1; }
    if (!fsize || !comp)
      fsize = (pwidth*bits+31)/8*pheight;
    pinter = (bits<=8);
    size = pwidth*pheight;
    if (pinter)  size += 768;
    else         size *= 3;
    if (pinter)  if (size<(1<<bits)*4)  size = (1<<bits)*4;
    if (!(new=malloc2(size))) {
      perr = 0x506;  (*fclose2)(fptr);  return(0); }
    if (!(bmp=malloc2(fsize))) {
      free2(new);
      perr = 0x606;  (*fclose2)(fptr);  return(0); }
    if (pinter && size>=(1<<bits)*4) {
      if (type!=12) {
        (*fread2)(new, 4, 1<<bits, fptr);
        for (i=1; i<(1<<bits); i++)
          memmove(new+i*3, new+i*4, 3); }
      else
        (*fread2)(new, 3, 1<<bits, fptr);
      bgr_to_rgb(new, 256); }
    if (comp==3)
      (*fread2)(bitfield, 4, 3, fptr);
    if (comp!=3 || !bitfield[0] || !bitfield[1] || !bitfield[2]) {
      if (bits==32) {
        bitfield[0] = 0xFF0000;  bitfield[1] = 0xFF00;  bitfield[2] = 0xFF; }
      else {
        bitfield[0] = 0xF800;  bitfield[1] = 0x7E0;  bitfield[2] = 0x1F; } }
    if (bits==16 || bits==32)
      for (j=0; j<3; j++)
        for (i=0; i<32; i++)
          if (bitfield[j]&(1<<i))
            bitfield[j+3] = 31-i;
    if (place)
      (*fseek2)(fptr, place, SEEK_SET);
    (*fread2)(bmp, 1, fsize, fptr);
    (*fclose2)(fptr);
    j = s = 0;  d = 768;
    if (bits>8)  d = 0;
    pspec[2] = (bits==1)+2*(bits==4)+3*(bits==8)+4*(bits>8);
    pspec[3] = comp;
    dpi = (float)((long *)(head+14))[6]/39.37+0.5;
    if (dpi<=0 || dpi>32767)  dpi = 160;
    ((short *)(pspec+4))[0] = dpi;
    pspec[6] = (type==12);
    NumPart = pheight; }
  if (LoadPart>NumPart)
    j = pheight;
  switch (comp*100+bits) {
    case 1: for (; j<pheight; j++) {
        for (i=0; i<pwidth && s<fsize; i++,d++)
          new[d] = ((bmp[s+i/8]&imask[i%8])!=0);
        s += ((pwidth+31)/32)*4;
        if (LoadPart>=0) {
          LoadPart++;  j++;  return(0); } } break;
    case 4: for (; j<pheight; j++) {
        for (i=0; i<pwidth && s<fsize; i++,d++)
          if (i&1) new[d] = (bmp[s+i/2]&15);
          else     new[d] = (bmp[s+i/2]>>4);
        s += ((pwidth+7)/8)*4;
        if (LoadPart>=0) {
          LoadPart++;  j++;  return(0); } } break;
    case 8: for (; j<pheight; j++) {
        memmove(new+d, bmp+s, min(pwidth, fsize-s));
        d += pwidth;
        s += ((pwidth+3)/4)*4;
        if (LoadPart>=0) {
          LoadPart++;  j++;  return(0); } } break;
    case 16: for (; j<pheight; j++) {
        for (i=0; i<pwidth; i++) {
          new[d+i*3]   = ((((ushort *)(bmp+s+i*2))[0]&bitfield[0])<<
                          bitfield[3])>>24;
          new[d+i*3+1] = ((((ushort *)(bmp+s+i*2))[0]&bitfield[1])<<
                          bitfield[4])>>24;
          new[d+i*3+2] = ((((ushort *)(bmp+s+i*2))[0]&bitfield[2])<<
                          bitfield[5])>>24; }
        d += pwidth*3;
        s += (((pwidth*2+3))/4)*4;
        if (LoadPart>=0) {
          LoadPart++;  j++;  return(0); } } break;
    case 24: for (; j<pheight; j++) {
        memmove(new+d, bmp+s, min(pwidth*3, fsize-s));
        d += pwidth*3;
        s += (((pwidth*3+3))/4)*4;
        if (LoadPart>=0) {
          LoadPart++;  j++;  return(0); } }
      bgr_to_rgb(new, pwidth*pheight); break;
    case 32: for (; j<pheight; j++) {
        for (i=0; i<pwidth; i++) {
          new[d+i*3]   = ((((ulong *)(bmp+s+i*4))[0]&bitfield[0])<<
                          bitfield[3])>>24;
          new[d+i*3+1] = ((((ulong *)(bmp+s+i*4))[0]&bitfield[1])<<
                          bitfield[4])>>24;
          new[d+i*3+2] = ((((ulong *)(bmp+s+i*4))[0]&bitfield[2])<<
                          bitfield[5])>>24; }
        d += pwidth*3;
        s += pwidth*4;
        if (LoadPart>=0) {
          LoadPart++;  j++;  return(0); } } break;
    case 108: unbmp8rle(bmp, fsize, new+768, pwidth, pheight); break;
    case 204: unbmp4rle(bmp, fsize, new+768, pwidth, pheight); break;
    default: perr = 0x704;  free2(bmp);  free2(new);  return(0); }
  if (LoadPart>=0)  LoadPart = 0;
  free2(bmp);
  if (flip)
    vertical_flip(new+768*pinter, pwidth*(1+2*(!pinter)), pheight);
  return(new);
}

STATIC uchar *load_gif(FILE *fptr)
/* Load in a GIF file.  Exits with values in pwidth and pheight.  pinter=1,
 *  since this guaranteed to be a palletized image.  If an error occurs, a
 *  null is passed back.  See load_graphic for return data format.  The file
 *  pointer is closed.
 * Enter: FILE *fptr: pointer to the open GIF file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/25/96-DWM */
{
  uchar *new, *gif;
  long size, i;

  (*fseek2)(fptr, 0, SEEK_END);
  size = (*ftell2)(fptr);
  (*fseek2)(fptr, 0, SEEK_SET);
  if (!(gif=malloc2(size+3))) {
    perr = 0x606;  (*fclose2)(fptr);  return(0); }
  (*fread2)(gif, 1, size, fptr);
  (*fclose2)(fptr);
  MAXGSIZE = (long)((short *)gif)[3]*((short *)gif)[4];
  if (!(new=malloc2(MAXGSIZE+768))) {
    perr = 0x506;  free2(gif);  return(0); }
  pwidth = -1;
  pspec[6] = (gif[4]=='9');
  ungif(new, gif, size);
  pspec[0] = 2;
  if (ppart>=0) {
    pspec[1] |= (ppart&0x7F);
    ((short *)(pspec+0x12))[0] = (ppart>>7); }
  pspec[2] = ((pinter&2)!=0);
  ((short *)(pspec+12))[0] = pinter>>16;
  pspec[14] = ((pinter&4)!=0);
  if (pspec[2])  pspec[8] = (pinter>>8)&0xFF;
  pinter &= 1;  pspec[7] = pinter;
  free2(gif);
  if (pwidth<=0 || pheight<0) {
    perr = 0x805;  free2(new);  return(0); }
  if (pinter)
    uninterlace(new+768, 0);
  for (i=0; i<768; i++)
    new[i] = new[i]*4.048;
  pinter = 1;
  return(new);
}

STATIC uchar *load_graphic(char *name)
/* Load in a graphics file or return an error condition.  Exits with values
 *  in pwidth and pheight.  pinter=1 for palette images and 0 for RGB images.
 *  The palette is always stored in terms of RGBRGBRGB... on a scale of 0 -
 *  255.
 * Only one global variable has an effect on the loading process, ppart.
 *  This only applies to formats which support multiple images.  It contains
 *  either -1 to choose the "best" image from the file, or the part of the
 *  file to load (0 to n-1), where the file has n images.  If it is out of
 *  range, either the -1 condition is used or an error is returned.
 * If an error occurs a null is passed back and perr&0xFF contains
 *  0-unknown file type or no error
 *  1-can't read file
 *  2-CPCardBack file (didn't read)
 *  3-unknown sub format
 *  4-unknown file option
 *  5-invalid graphic size
 *  6-out of memory
 *  7-part not available.
 *  8-GVSlide file (didn't read)
 * More explicit error messages are contained in (perr>>8)
 *   0-Unknown file type or no error.
 *   1-Can't read file.
 *   2-CPCardBack file (didn't read)
 *   3-Insufficient memory to flip image horizontally.
 *   4-Invalid version number (not 0x40 or 0x12).
 *   5-Insufficient memory for graphic.
 *   6-Insufficient memory for file.
 *   7-Unrecognized compression type.
 *   8-Invalid graphic size.
 *   9-Insufficient memory for decoder.
 *   A-Unsupported compression type.
 *   B-Unsupported bits per pixel.
 *   C-Unsupported number of color channels.
 *   D-Unsupported spectral selection in scan header.
 *   E-Only four quantization tables are supported.
 *   F-Frame and scan headers do not match.
 *  10-Palette larger than supported.
 *  11-Unsupported orientation.
 *  12-Unsupported color space.
 *  13-Planar pixel format not supported.
 *  14-Too many strips in file.
 *  15-Insufficient memory for planar conversion.
 *  16-Insufficient memory for deinterlacing.
 *  17-Part not available.
 *  18-GVSlide file (didn't read)
 * Note that if LoadPart is less than zero, the entire graphic is loaded in a
 *  single call.  If LoadPart is non-negative, then the graphic is loaded in
 *  a series of calls to load_graphic.  The calling program must continue to
 *  call load_graphic(0) until LoadPart becomes equal to zero.  Only then
 *  does the call return with the success or failure of the loading process.
 *  The value NumPart contains the predicted value for how many times
 *  load_graphic will need to be called.  If LoadPart is set to a number
 *  greater than NumPart, and load_graphic called, the load process will be
 *  aborted, generally with a graphic image returned but containing invalid
 *  data.
 * The pspec array contains information about the loaded file.  The format of
 *  this array matches exactly the format that save_graphic requires.
 * Enter: char *name: name of file to attempt to load.  If LoadPart is non-
 *                    negative, pass a 0 to continue loading parts of the
 *                    graphics file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/25/96-DWM */
{
  static long i, j, scale, type;
  static uchar *new, total[18];
  static FILE *fptr, *fptr2;

  if (LoadPart<0 || name) {
    perr = 0;
    if (LoadPart>=0)  LoadPart = 0;
    NumPart = 1;
    if (!(fptr=(*fopen2)(name, "rb"))) {
      perr = 0x101;  return(0); }
    if ((*fread2)(total, 18, 1, fptr)!=1) {
      (*fclose2)(fptr);  perr = 0x101;  return(0); }              type = 0;
    if (!memcmp(total, fcheck1, 10))                              type = 1;
    if (!memcmp(total,"MM\0\x2A",4)||!memcmp(total,"II\x2A\0",4)) type = 2;
    if (!memcmp(total, "GIF8", 4))                                type = 3;
    if (toupper(total[0])=='P' && total[1]>='1' && total[1]<='9') type = 4;
    if (!memcmp(total, "BM", 2))                                  type = 5;
    if (total[0]==0x0A && total[2]==1 && total[1]<=5)             type = 6;
    if (!memcmp(total, "\xFF\xD8", 2))                            type = 8;
    if (!memcmp(total, "%%HP", 4))                                type = 9;
    if (!memcmp(total,"\0\0\1\0",4)||!memcmp(total,"\0\0\2\0",4)) type =10;
    if (!memcmp(total, "HPHP", 4))                                type =11;
    if (!memcmp(total, "GVSl", 4))                                type =12;
    if (total[1]<=1 && (total[2]&0xF7)==2 && (total[16]==24 || (total[16]==32
        && total[2]!=0xA)) && !(total[17]&0xD0))                  type = 7;
    (*fseek2)(fptr, 0, SEEK_SET);
    memset(pspec, 0, LENPSPEC);
    if (type<=1) {
      if (new=load_pil(fptr)) {
        type = 13;
      }
    }
    if (type<=1 || type==12) {
      perr = 0;  if (type==1)  perr = 0x202;
      if (type==12)  perr = 0x1808;
      (*fclose2)(fptr);  return(0); } }
  switch (type) {
    case 2:  new = load_tif(fptr); break;
    case 3:  new = load_gif(fptr); break;
    case 4:  new = load_ppm(fptr); break;
    case 5:  new = load_bmp(fptr); break;
#ifndef DOSLIB
    case 6:  new = load_pcx(fptr); break;
    case 7:  new = load_tga(fptr); break;
#endif
    case 8:  new = load_jpg(fptr); break;
#ifndef DOSLIB
    case 9:  new = load_groa(fptr);break;
    case 10: new = load_ico(fptr); break;
    case 11: new = load_grob(fptr);break;
#endif
    case 13: break;
/** Additional file formats go here **/
    default: perr = 0;  return(0); }
  if (LoadPart>0)  return(0);
  return(new);
}

#ifndef DOSLIB
STATIC uchar *load_groa(FILE *fptr)
/* Load in an ASCII grob (HP calculator) file.  Exits with values in pwidth
 *  and pheight.  pinter=1, since this is a 2 color image.  If an error
 *  occurs a null is passed back.  See load_graphic for return data format.
 *  The file pointer is closed.
 * Enter: FILE *fptr: pointer to the open grob file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/26/96-DWM */
{
  uchar *new;
  long size=17, i, j, k, d, t;
  char mask[]={16,32,64,128,1,2,4,8};
  char header[50], *h;

  (*fread2)(header, 1, 50, fptr);
  if (h=strstr(header, "GROB"))
    size = h-header+4;
  (*fseek2)(fptr, size, SEEK_SET);
  pwidth = pheight = 0;
  fscanf(fptr, "%d%*c%d", &pwidth, &pheight);
  if (pwidth<=0 || pheight<=0) {
    perr = 0x805;  (*fclose2)(fptr);  return(0); }
  pinter = 1;
  size = pwidth*pheight+768;
  if (!(new=malloc2(size))) {
    perr = 0x506;  (*fclose2)(fptr);  return(0); }
  memset(new, 0, size);
  new[3] = new[4] = new[5] = 255;
  pspec[0] = 3;
  ((float *)(pspec+8))[0] = 1;
  pspec[12] = 2;  if (!(pheight%64))  pspec[12] = pheight/64+1;
  d = 768;
  for (j=0; j<pheight; j++)
    for (i=0; i<pwidth; i+=8) {
      fscanf(fptr, "%2x", &t);
      for (k=0; k<8 && i+k<pwidth; k++, d++)
        if (!(t&mask[k]))
          new[d] = 1; }
  (*fclose2)(fptr);
  return(new);
}
#endif

#ifndef DOSLIB
STATIC uchar *load_grob(FILE *fptr)
/* Load in a binary grob (HP calculator) file.  Exits with values in pwidth
 *  and pheight.  pinter=1, since this is a 2 color image.  If an error
 *  occurs a null is passed back.  See load_graphic for return data format.
 *  The file pointer is closed.
 * Enter: FILE *fptr: pointer to the open grob file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/26/96-DWM */
{
  uchar *new;
  long size=17, i, j, k, d, t;
  uchar mask[]={1,2,4,8,16,32,64,128};
  char header[50];

  (*fread2)(header, 1, 18, fptr);
  pheight = ((uchar *)(header+13))[0]+((uchar *)(header+14))[0]*256;
  pwidth = (((uchar *)(header+15))[0]+((uchar *)(header+16))[0]*256)>>4;
  if (((short *)(header+8))[0]!=0x2B1E)  pwidth = 0;
  if (pwidth<=0 || pheight<=0) {
    perr = 0x805;  (*fclose2)(fptr);  return(0); }
  pinter = 1;
  size = pwidth*pheight+768;
  if (!(new=malloc2(size))) {
    perr = 0x506;  (*fclose2)(fptr);  return(0); }
  memset(new, 0, size);
  new[3] = new[4] = new[5] = 255;
  pspec[0] = 3;
  pspec[6] = 1;
  ((float *)(pspec+8))[0] = 1;
  pspec[12] = 2;  if (!(pheight%64))  pspec[12] = pheight/64+1;
  d = 768;
  for (j=0; j<pheight; j++)
    for (i=0; i<pwidth; i+=8) {
      (*fread2)(&t, 1, 1, fptr);
      for (k=0; k<8 && i+k<pwidth; k++, d++)
        if (!(t&mask[k]))
          new[d] = 1; }
  (*fclose2)(fptr);
  return(new);
}
#endif

#ifndef DOSLIB
STATIC uchar *load_ico(FILE *fptr)
/* Load in an ICO/CUR file.  This is essentially a BMP file with an extra
 *  header.  Exits with values in pwidth and pheight.  pinter=1 for palette
 *  images and 0 for RGB images.  If an error occurs a null is passed back.
 *  See load_graphic for return data format.  The file pointer is closed.
 * Enter: FILE *fptr: pointer to the open ICO/CUR file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/25/96-DWM */
{
  uchar *new, *bmp, head[54];
  long bits, type, comp, fsize, size, i, j, k, s, d, start, clrs;

  comp = fsize = 0;
  (*fread2)(head, 1, 6, fptr);
  pspec[0] = -((short *)head)[1]*3+7;
  d = ((short *)(head+4))[0];
  if (ppart>=0 && ppart<d)  d = ppart+1;
  ((uchar *)pspec)[1] |= 0x80*(ppart+1<((short *)(head+4))[0])*(ppart<127);
  for (k=d; k; ) {
    (*fseek2)(fptr, 6, SEEK_SET);
    for (i=s=0; i<k; i++) {
      (*fread2)(head, 1, 16, fptr);
      j = head[0]*head[1]*((ushort *)(head+2))[0];
      if (j>=s || ppart>=0) {
        pspec[1] = (pspec[1]&0x80)|i;
        s = j;  start = ((long *)(head+12))[0]; } }
    k = 0;
    (*fseek2)(fptr, start, SEEK_SET);
    (*fread2)(head+14, 1, 40, fptr);
    type = ((long *)(head+14))[0];
    if (type==40) {
      pwidth = ((long *)(head+18))[0];
      pheight = ((long *)(head+18))[1];
      bits = ((short *)(head+28))[0];
      comp = ((long *)(head+30))[0];
      fsize = ((long *)(head+34))[0]; }
    else if (type==12) {
      (*fseek2)(fptr, start+12, SEEK_SET);
      pwidth = ((short *)(head+18))[0];
      pheight = ((short *)(head+18))[1];
      bits = ((short *)(head+24))[0]; }
    else {
      d--;  if (ppart>=0) d = 0;
      k = d; } }
  ((short *)(pspec+16))[0] = ((short *)head)[2];
  ((short *)(pspec+16))[1] = ((short *)head)[3];
  if (!d) {
    perr = 0x403;  (*fclose2)(fptr);  return(0); }
  pheight /= 2;
  if (!fsize || !comp)
    fsize = (pwidth*bits+31)/8*pheight+(pwidth+31)/8*pheight;
  pinter = (bits!=24);
  clrs = 1<<bits;
  size = pwidth*pheight;
  if (pinter)  size += 768;
  else         size *= 3;
  if (!(new=malloc2(size))) {
    perr = 0x506;  (*fclose2)(fptr);  return(0); }
  if (!(bmp=malloc2(fsize))) {
    free2(new);
    perr = 0x606;  (*fclose2)(fptr);  return(0); }
  if (pinter && size>=min(clrs, 256)*4) {
    (*fread2)(new, 4, min(clrs, 256), fptr);
    for (i=1; i<clrs && i<256; i++)
      memmove(new+i*3, new+i*4, 3);
    bgr_to_rgb(new, 256);
    if (clrs<256)
      new[clrs*3] = new[clrs*3+1] = new[clrs*3+2] = 160;
    if (clrs<=128)
      new[clrs*6-3] = new[clrs*6-2] = new[clrs*6-1] = 95;
    for (i=clrs*3+3; i<clrs*6-3 && i<768; i++)
      new[i] = (new[i-3*clrs]+64)%256; }
  (*fread2)(bmp, 1, fsize, fptr);
  (*fclose2)(fptr);
  pspec[2] = (bits==1)+2*(bits==4);
  ((short *)(pspec+4))[0] = ((long *)(head+14))[6]/39.37;
  pspec[8] = clrs;  pspec[12] = clrs*2-1;
  j = s = 0;  d = 768;
  if (comp*100+bits==24)  d = 0;
  switch (comp*100+bits) {
    case 1: for (; j<pheight; j++) {
        for (i=0; i<pwidth && s<fsize; i++,d++)
          new[d] = ((bmp[s+i/8]&imask[i%8])!=0);
        s += ((pwidth+31)/32)*4; } break;
    case 4: for (; j<pheight; j++) {
        for (i=0; i<pwidth && s<fsize; i++,d++)
          if (i&1) new[d] = (bmp[s+i/2]&15);
          else     new[d] = (bmp[s+i/2]>>4);
        s += ((pwidth+7)/8)*4; } break;
    case 8: for (; j<pheight; j++) {
        memmove(new+d, bmp+s, min(pwidth, fsize-s));
        d += pwidth;
        s += ((pwidth+3)/4)*4; } break;
    case 24: for (; j<pheight; j++) {
        memmove(new+d, bmp+s, min(pwidth*3, fsize-s));
        d += pwidth*3;
        s += (((pwidth*3+3))/4)*4; }
      bgr_to_rgb(new, pwidth*pheight); break;
    case 108: unbmp8rle(bmp, fsize, new+768, pwidth, pheight); break;
    case 204: unbmp4rle(bmp, fsize, new+768, pwidth, pheight); break;
    default: perr = 0x704;  free2(bmp);  free2(new);  return(0); }
  if (!comp && bits<=7)
    for (j=0, d=768; j<pheight; j++) {
      for (i=0; i<pwidth && s<fsize; i++,d++)
        if ((bmp[s+i/8]&imask[i%8])!=0)
          new[d] += clrs;
      s += ((pwidth+31)/32)*4; }
  free2(bmp);
  vertical_flip(new+768*pinter, pwidth*(1+2*(!pinter)), pheight);
  return(new);
}
#endif

STATIC uchar *load_jpg(FILE *fptr)
/* Load in a JPEG file.  Exits with values in pwidth and pheight.  pinter=1
 *  for palette (greyscale) images and 0 for RGB images.  If an error occurs
 *  a null is passed back.  See load_graphic for return data format.  The
 *  file pointer is closed.  This file reader mallocs a work array and an
 *  initial file array prior to the final image array, so there may be memory
 *  fragmentation.
 * This only reads baseline JPEG files with either 1, 3, or 4 components
 *  (greyscale, YCbCr color, CMYK, or YCCK).  Also, the scan header and the
 *  frame header must list the components in the same order.
 * Enter: FILE *fptr: pointer to the open JPG file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/25/96-DWM */
{
  static uchar *new, *jpg, *temp, *temp2, *huff, *hufflen, *quant, *dres;
  static uchar *crop, *dres2, *dres3;
  static long res[64], dy, fsize, s, d, val, restart, h, i, j, k, l, r, w;
  static long mw, mh, start, size, com[28], rr[8], ncom, hl[16], dcbase[4];
  static long x, y, Y, Cb, Cr, mw8, mh8, *ytbl, C, M, K, ycck;
  static long *ytbl2, *ytbl3, *ytbl4, lastr, rnum, orient;
  static long ln[16]={32768,16384,8192,4096,2048,1024,512,256,128,64,32,16,8,4,2,1};
  long part=ppart, tiflen, tifend, taglen, tagnum;
  unsigned char *tag;

  if (LoadPart<=0) {
    if (part<0)  part = 0;
    restart = 0;
    ycck = 0;
    orient = 0;
    (*fseek2)(fptr, 0, SEEK_END);
    fsize = (*ftell2)(fptr);                 /*   Workspace:              */
    (*fseek2)(fptr, 0, SEEK_SET);            /* Huffman tables:  64k * 8  */
    if (!(jpg=malloc2(fsize))) {             /* Huffman lengths: 256 * 8  */
      perr = 0x606;  (*fclose2)(fptr);       /* Quant. tables:    64 * 4  */
      return(0); }                           /* Decoded values:   64 * 24 */
    (*fread2)(jpg, 1, fsize, fptr);          /* Crop table:      768      */
    (*fclose2)(fptr);                        /* YCbCr tables:   1024 * 4  */
    if (!(huff=malloc2(65536*8+256*8+64*4+64*32+768+1024*4))) {
      perr = 0x906;  free2(jpg);  return(0); }
    hufflen = huff+65536*8;  quant = hufflen+256*8;  dres = quant+64*4;
    crop = dres+64*32;  ytbl = (long *)(crop+768);
    dres2 = dres+512;  dres3 = dres2+512;
    ytbl2 = ytbl+256;  ytbl3 = ytbl2+256;  ytbl4 = ytbl3+256;
    pspec[1] = part&0x7F;  ((short *)(pspec+0x12))[0] = (part>>7);
    s = 0;        /* Read header information, including tables, size, etc. */
    while (s<fsize) {
      val = jpg[s]*256+jpg[s+1];
      switch (val) {
        case 0xFFC1: pspec[3] = 1;                /* Extended frame header */
        case 0xFFC0: if (jpg[s+4]!=8) {           /* Baseline frame header */
            perr = 0xB04;  free2(jpg);  free2(huff);  return(0); }
          if (part<0) break;
          pheight = jpg[s+5]*256+jpg[s+6];
          pwidth  = jpg[s+7]*256+jpg[s+8];
          if (jpg[s+9]!=1 && jpg[s+9]!=3 && jpg[s+9]!=4) {
            perr = 0xC04;  free2(jpg);  free2(huff);  return(0); }
          ncom = jpg[s+9];
          for (i=0; i<ncom; i++) {
            com[i*7] = jpg[s+10+i*3];
            com[i*7+1] = jpg[s+11+i*3]>>4;
            com[i*7+2] = jpg[s+11+i*3]&0xF;
            com[i*7+3] = jpg[s+12+i*3]; } break;
        case 0xFFC2:                           /* Progressive frame header */
          /* huff and jpg are freed by load_jpg_progress */
          return(load_jpg_progress(jpg, fsize, huff));
        case 0xFFC3: case 0xFFC5: case 0xFFC6: case 0xFFC7: case 0xFFC9:
        case 0xFFCA: case 0xFFCB: case 0xFFCD: case 0xFFCE: case 0xFFCF:
          free2(jpg);  free2(huff);  perr = 0xA03;  return(0);
        case 0xFFC4: j = jpg[s+2]*256 + jpg[s+3] + s + 2;/* Huffman tables */
          if (part<0) break;
          h = s + 4;
          while (h<j) {
            temp = huff+65536*((jpg[h]>>4)*4+(jpg[h]&0xF));
            temp2 = hufflen+256*((jpg[h]>>4)*4+(jpg[h]&0xF));
            for (i=0; i<16; i++)
              hl[i] = jpg[h+1+i];
            h += 17;
            for (l=1; l<=16; l++)
              for (i=0; i<hl[l-1]; i++, h++) {
                temp2[jpg[h]] = l;
                memset(temp, jpg[h], ln[l-1]);
                temp += ln[l-1]; } } break;
        case 0xFFDA: if (part<0) {                          /* Scan header */
            pspec[1] |= 0x80; break; }
          part--;
          if (part!=-1) break;
          start = s+2+jpg[s+2]*256+jpg[s+3];
          for (i=0; i<jpg[s+4]; i++) {
            com[i*7+4] = jpg[s+5+i*2];
            com[i*7+5] = jpg[s+6+i*2]>>4;
            com[i*7+6] = (jpg[s+6+i*2]&0xF)+4; }/* +4 to match huffman pos */
          if (jpg[s+5+i*2] || jpg[s+7+i*2] || jpg[s+6+i*2]!=63) {
            perr = 0xD04;  free2(jpg);  free2(huff);  return(0); } break;
        case 0xFFDB: j = (jpg[s+2]*256+jpg[s+3]-2)/65;    /* Quant. tables */
          if (part<0) break;
          for (i=0; i<j; i++) {
            k = jpg[s+4+i*65];
            if (k>=4) {
              perr = 0xE04;  free2(jpg);  free2(huff);  return(0); }
            memcpy(quant+k*64, jpg+s+5+i*65, 64);
            if (k<2)
              memcpy(pquant+k*64, quant+k*64, 64); } break;
        case 0xFFDC: if (part<0) break;                      /* Num. lines */
          pheight = jpg[s+4]*256+jpg[s+5]; break;
        case 0xFFDD: if (part<0) break;                     /* Restart int */
          restart = jpg[s+4]*256+jpg[s+5]; break;
        case 0xFFD9: break;                                /* End of image */
        case 0xFFEE: /* Adobe color space marker */
          if (jpg[s+2]*256+jpg[s+3]>=14)
            if (!memcmp(jpg+s+4, "Adobe", 5) && jpg[s+15])
              ycck = 1;
          break;
        case 0xFFE1: /* Application Data.  Includes EXIF data. */
          taglen = jpg[s+2]*256+jpg[s+3];
          if (taglen<16)  break;
          if (memcmp(jpg+s+4, "Exif", 4) || (memcmp(jpg+s+10, "II*\0", 4) && memcmp(jpg+s+10, "MM\0*", 4)))  break;
          tifend = (jpg[s+10]=='M');
          if (tifend)
            tiflen = endianl((long *)(jpg+s+14));
          else
            tiflen = ((long *)(jpg+s+14))[0];
          if (8+tiflen+2>taglen)  break;
          if (tifend)
             tagnum = endian((short *)(jpg+s+10+tiflen));
          else
             tagnum = ((short *)(jpg+s+10+tiflen))[0];
          tag = malloc2(taglen);
          if (!tag)  break;
          reorder_tif(tag, jpg+s+10+tiflen+2, tagnum, tifend, 0);
          for (i=0; i<tagnum; i++)
            switch (uitag[i*14]) {
              case 0x112: orient = ((ltag[i*7+3]-1)&7)+1; break;
              default: break; }
          free2(tag);
          break;
        default: if (jpg[s]==0xFF && jpg[s+1]!=0 && jpg[s+1]!=0xDA) break;
          temp = memchr(jpg+s+1, 0xFF, fsize-s-1);
          if (!temp)  s = fsize;
          else        s = temp-jpg; continue; }
      if (val==0xFFD9 && part<=-1) break;               // had been just <1
      if ((val>=0xFFC0 && val<=0xFFCF) || val==0xFFDA || val==0xFFDB ||
          val==0xFFDA || (val>=0xFFE0 && val<=0xFFEF) || val==0xFFFE)
                                            s += 2 + jpg[s+2]*256 + jpg[s+3];
      else if (val==0xFF01 || (val>=0xFFD0 && val<=0xFFD9))  s += 2;
      else if (val==0xFFDF)                                  s += 5;
      else if (val==0xFFDC || val==0xFFDD)                   s += 6;
      else                                                   s++; }
    for (i=mw=mh=0; i<ncom; i++) {/* Check if scan and frame headers match */
      if (ncom==1)  com[i*7+1] = com[i*7+2] = 1;
      if (com[i*7+1]>mw)  mw = com[i*7+1];
      if (com[i*7+2]>mh)  mh = com[i*7+2];
      if (com[i*7]!=com[i*7+4] || com[i*7+1]<=0 || com[i*7+2]<=0) {
        perr = 0xF04;  free2(jpg);  free2(huff);  return(0); } }
    if (pwidth<=0 || pheight<=0 || mw*mh>8) {    /* Set up new image array */
      perr = 0x804;  free2(jpg);  free2(huff);  return(0); }
    pspec[0] = 5;
    ((short *)(pspec+4))[0] = jpg[0xE]*256+jpg[0xF];
    pspec[6] = (mw==2)+(mh==2)+2*(mw==4);
    pspec[8] = quant[58];
    ((long *)(pspec+0xC))[0] = (long)(pquant);
    mw8 = mw*8;  mh8 = mh*8;
    for (i=0; i<ncom; i++) {
      rr[i]   = mw/com[i*7+1];
      rr[i+4] = mh/com[i*7+2]; }
    w = ((pwidth +mw8-1)/(mw8))*mw8;
    h = ((pheight+mh8-1)/(mh8))*mh8;
    size = w*h*(1+2*(ncom!=1))+768*(ncom==1);  pinter = (ncom==1);
    if (!(new=malloc2(size))) {
      perr = 0x506;  free2(jpg);  free2(huff);  return(0); }
    if (ncom==1)
      for (i=0; i<256; i++)
        new[i*3] = new[i*3+1] = new[i*3+2] = i;
    else
      for (i=0; i<256; i++) {
        crop[i] = 0;  crop[i+256] = i;  crop[i+512] = 255;
        ytbl[i]     = (((i-128)*359+128)>>8)+256;
        ytbl[i+256] = (i-128)*88+128-65536;
        ytbl[i+512] = (i-128)*183;
        ytbl[i+768] = (((i-128)*454+128)>>8)+256; }
    dy = 3*(w-mw8);
    fsize = jpeg_unstuff(jpg, fsize, start);         /* Unstuff zero bytes */
    r = dcbase[0] = dcbase[1] = dcbase[2] = 0;                   /* Decode */
    y=s=x=0;  lastr = rnum = 0;
    NumPart = h*w/mh8/mw8;
    if (Minpwidth && Minpheight && (Minpwidth*8<w || Minpheight*8<h))
      Minpwidth = min(w, h)/(max(Minpwidth, Minpheight)*8);
    else
      Minpwidth = 0; }
  if (LoadPart>NumPart)
    y = h;
  for (; y<h; y+=mh8, x=0)  for (; x<w; x+=mw8, r++) {
    if (r && r==restart) {
      r = dcbase[0] = dcbase[1] = dcbase[2] = dcbase[3] = 0;
      s = (s+7)&0xFFFFFFF8;
      if (jpg[s>>3]==0xFF && jpg[(s>>3)+1]==0xD0+rnum) {
        s += 16;  lastr = s;  rnum = (rnum+1)%8; }
      else {
        for (i=(lastr>>3); i<fsize-1; i++)
          if (jpg[i]==0xFF && jpg[i+1]==0xD0+rnum) {
            for (j=i+1; j<fsize; j++)
              if (jpg[j]==0xFF && (jpg[j+1]&0xD8)==0xD0) {
                if (jpg[j+1]==0xD0+rnum)  i = j;
                break; }
            s = (i+2)<<3;  lastr = s;  rnum = (rnum+1)%8; break; } } }
    for (k=0; k<ncom; k++)
      for (j=0,l=k*512; j<com[k*7+2]; j++,l+=7*mw8)
        for (i=0; i<com[k*7+1]; i++, l+=(rr[k]<<3)) {
          s = jpeg_decode(jpg, s, fsize, huff+(com[k*7+5]<<16),
                        hufflen+(com[k*7+5]<<8), huff+(com[k*7+6]<<16),
                        hufflen+(com[k*7+6]<<8), quant+(com[k*7+3]<<6), res);
          dcbase[k] = res[0] = (dcbase[k]+res[0]);
          jpeg_undct(res, dres+l, mw8, rr[k], rr[k+4], 128); }
    if (ncom==1)
      for (j=l=0; j<mh8; j++,l+=mw8)
        memcpy(new+768+x+(y+j)*w, dres+l, mw8);
    else if (ncom==3) {
      d = (x+y*w)*3;
      for (j=l=0; j<mh8; j++, d+=dy) {
        for (i=0; i<mw8; i++,l++,d+=3) {
          Y = dres[l];  Cb = dres2[l];  Cr = dres3[l];
          new[d]   = crop[ytbl[Cr]+Y];
          new[d+1] = crop[Y-((ytbl2[Cb]+ytbl3[Cr])>>8)];
          new[d+2] = crop[ytbl4[Cb]+Y]; } } }
    else {
      d = (x+y*w)*3;
      for (j=l=0; j<mh8; j++, d+=dy) {
        for (i=0; i<mw8; i++,l++,d+=3) {
          /* CMYK should be simple, but in some cases it didn't work, whereas,
           * this does, which may be what is called YCCK. */
          if (!ycck)
          {
            M = dres[l];       Y = dres[l+512];
            C = dres[l+1024];  K = 255-dres[l+1536];
          }
          else
          {
            Y = dres[l];  Cb = dres2[l];  Cr = dres3[l];
            C = crop[ytbl[Cr]+Y];
            M = crop[Y-((ytbl2[Cb]+ytbl3[Cr])>>8)];
            Y = crop[ytbl4[Cb]+Y];
            K = 255-dres[l+1536];
          }
          new[d]   = max(0, 255-C*(255-K)/255-K);
          new[d+1] = max(0, 255-M*(255-K)/255-K);
          new[d+2] = max(0, 255-Y*(255-K)/255-K); } } }
    if (LoadPart>=0) {
      LoadPart++;  x+=mw8, r++;
      if (x>=w) { y+=mh8; x=0; }
      return(0); } }
  if (LoadPart>=0)  LoadPart = 0;
  free2(huff);
  free2(jpg);
  Minpwidth = Minpheight = 0;
  if (pwidth!=w) {
    if (pinter) {
      for (i=1; i<pheight; i++)
        memmove(new+768+i*pwidth, new+768+i*w, pwidth);
      size = 768+pwidth*pheight; }
    else {
      for (i=1; i<pheight; i++)
        memmove(new+i*pwidth*3, new+i*w*3, pwidth*3);
      size = pwidth*pheight*3; }
    if (jpg=realloc2(new, size))
      new = jpg; }
  new = reorient(new, orient);
  return(new);
}

STATIC uchar *load_jpg_progress(uchar *jpg, long fsize, uchar *huff)
/* Load in the first image in a progressive JPEG file.  Exits with values in
 *  pwidth and pheight.  pinter=1 for palette (greyscale) images and 0 for
 *  RGB images.  If an error occurs a null is passed back.  See load_graphic
 *  for return data format.
 * Enter: uchar *jpg: pointer to a memory image of the JPEG file.
 *        long fsize: length of the file.
 *        uchar *huff: work memory pointer.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        1/27/01-DWM */
{
  uchar *new, *temp, *temp2, *hufflen, *quant, *dres, *crop, *dres2, *dres3;
  long dy, s, d, val, restart, h, i, j, k, l, r, w, firstspec;
  long mw, mh, start, size, com[28], rr[8], ncom, hl[16], dcbase[4];
  long x, y, Y, Cb, Cr, mw8, mh8, *ytbl, succ, C, M, K, ycck;
  long *ytbl2, *ytbl3, *ytbl4, lastr, rnum;
  static long ln[16]={32768,16384,8192,4096,2048,1024,512,256,128,64,32,16,8,4,2,1};
  long part=0, spec0, spec1, scom0, scom1, so;
  long snum=0, spos;
  /* There is some bug such that when these are not static, something gets
   * corrupted.  I haven't hunted it down. */
  static long sstart[JPG_MAXPROGRESSIVE], slen[JPG_MAXPROGRESSIVE];
  static long shead[JPG_MAXPROGRESSIVE], shuff[JPG_MAXPROGRESSIVE];
  long *decode, *dest;

  NumPart = 1;
  restart = 0;
  ycck = 0;
  hufflen = huff+65536*8;  quant = hufflen+256*8;  dres = quant+64*4;
  crop = dres+64*32;  ytbl = (long *)(crop+768);
  dres2 = dres+512;  dres3 = dres2+512;
  ytbl2 = ytbl+256;  ytbl3 = ytbl2+256;  ytbl4 = ytbl3+256;
  pspec[1] = 0;
  s = 0;        /* Read header information, including tables, size, etc. */
  while (s<fsize) {
    val = jpg[s]*256+jpg[s+1];
    switch (val) {
      case 0xFFC0: case 0xFFC1: case 0xFFC3: case 0xFFC5: case 0xFFC6:
      case 0xFFC7: case 0xFFC9: case 0xFFCA: case 0xFFCB: case 0xFFCD:
      case 0xFFCE: case 0xFFCF:
        free2(jpg);  free2(huff);  perr = 0xA03;  return(0);
      case 0xFFC2: pspec[3] = 2;               /* Progressive frame header */
        if (jpg[s+4]!=8) {
          perr = 0xB04;  free2(jpg);  free2(huff);  return(0); }
        if (part<0) break;
        pheight = jpg[s+5]*256+jpg[s+6];
        pwidth  = jpg[s+7]*256+jpg[s+8];
        if (jpg[s+9]!=1 && jpg[s+9]!=3 && jpg[s+9]!=4) {
          perr = 0xC04;  free2(jpg);  free2(huff);  return(0); }
        ncom = jpg[s+9];
        for (i=0; i<ncom; i++) {
          com[i*7] = jpg[s+10+i*3];
          com[i*7+1] = jpg[s+11+i*3]>>4;
          com[i*7+2] = jpg[s+11+i*3]&0xF;
          com[i*7+3] = jpg[s+12+i*3]; } break;
      case 0xFFC4: j = jpg[s+2]*256 + jpg[s+3] + s + 2;  /* Huffman tables */
        if (snum)  shuff[snum-1] = s;
        if (part<0) break;
        h = s + 4;
        while (h<j) {
          temp = huff+65536*((jpg[h]>>4)*4+(jpg[h]&0xF));
          temp2 = hufflen+256*((jpg[h]>>4)*4+(jpg[h]&0xF));
          for (i=0; i<16; i++)
            hl[i] = jpg[h+1+i];
          h += 17;
          for (l=1; l<=16; l++)
            for (i=0; i<hl[l-1]; i++, h++) {
              temp2[jpg[h]] = l;
              memset(temp, jpg[h], ln[l-1]);
              temp += ln[l-1]; } } break;
      case 0xFFDA: if (!part) {                             /* Scan header */
          start = s+2+jpg[s+2]*256+jpg[s+3];
          for (i=0; i<jpg[s+4]; i++) {
            com[i*7+4] = jpg[s+5+i*2];
            com[i*7+5] = jpg[s+6+i*2]>>4;
            com[i*7+6] = (jpg[s+6+i*2]&0xF)+4; }
          if (jpg[s+5+i*2]) {
            perr = 0xD04;  free2(jpg);  free2(huff);  return(0); } }
        part--;
        if (snum==JPG_MAXPROGRESSIVE) {
          perr = 0xD04;  free2(jpg);  free2(huff);  return(0); }
        shead[snum] = s;
        sstart[snum] = s+2+jpg[s+2]*256+jpg[s+3];
        if (snum)
          slen[snum-1]=sstart[snum]-sstart[snum-1]-(2+jpg[s+2]*256+jpg[s+3]);
        slen[snum] = fsize-sstart[snum];  shuff[snum] = 0;
        snum++; break;
      case 0xFFDB: j = (jpg[s+2]*256+jpg[s+3]-2)/65;      /* Quant. tables */
        if (part<0) break;
        for (i=0; i<j; i++) {
          k = jpg[s+4+i*65];
          if (k>=4) {
            perr = 0xE04;  free2(jpg);  free2(huff);  return(0); }
          memcpy(quant+k*64, jpg+s+5+i*65, 64);
          if (k<2)
            memcpy(pquant+k*64, quant+k*64, 64); } break;
      case 0xFFDC: if (part<0) break;                        /* Num. lines */
        pheight = jpg[s+4]*256+jpg[s+5]; break;
      case 0xFFDD: if (part<0) break;                       /* Restart int */
        restart = jpg[s+4]*256+jpg[s+5]; break;
      case 0xFFD9: break;                                  /* End of image */
      case 0xFFEE: /* Adobe color space marker */
        if (jpg[s+2]*256+jpg[s+3]>=14)
          if (!memcmp(jpg+s+4, "Adobe", 5) && jpg[s+15])
            ycck = 1;
        break;
      default: if (jpg[s]==0xFF && jpg[s+1]!=0 && jpg[s+1]!=0xDA) break;
        temp = memchr(jpg+s+1, 0xFF, fsize-s-1);
        if (!temp)  s = fsize;
        else        s = temp-jpg; continue; }
    if (val==0xFFD9 && part<=-1) break;        // had been just <-1
    if (val<0xFFD0 || val>0xFFD9)  s += 2 + jpg[s+2]*256 + jpg[s+3];
    else if (val!=0xFFFF)          s += 2;
    else if (val==0xFFDF)                                  s += 5;
    else if (val==0xFFDC || val==0xFFDD)                   s += 6;
    else                           s ++; }
  for (i=mw=mh=0; i<ncom; i++) {  /* Check if scan and frame headers match */
    if (ncom==1)  com[i*7+1] = com[i*7+2] = 1;
    if (com[i*7+1]>mw)  mw = com[i*7+1];
    if (com[i*7+2]>mh)  mh = com[i*7+2];
    if (com[i*7]!=com[i*7+4] || com[i*7+1]<=0 || com[i*7+2]<=0) {
      perr = 0xF04;  free2(jpg);  free2(huff);  return(0); } }
  if (pwidth<=0 || pheight<=0 || mw*mh>8) {      /* Set up new image array */
    perr = 0x804;  free2(jpg);  free2(huff);  return(0); }
  pspec[0] = 5;
  ((short *)(pspec+4))[0] = jpg[0xE]*256+jpg[0xF];
  pspec[6] = (mw==2)+(mh==2)+2*(mw==4);
  pspec[8] = quant[58];
  ((long *)(pspec+0xC))[0] = (long)(pquant);
  mw8 = mw*8;  mh8 = mh*8;
  for (i=0; i<ncom; i++) {
    rr[i]   = mw/com[i*7+1];
    rr[i+4] = mh/com[i*7+2]; }
  w = ((pwidth +mw8-1)/(mw8))*mw8;
  h = ((pheight+mh8-1)/(mh8))*mh8;
  size = w*h*(1+2*(ncom!=1))+768*(ncom==1);  pinter = (ncom==1);
  if (!(new=malloc2(size))) {
    perr = 0x506;  free2(jpg);  free2(huff);  return(0); }
  if (!(decode=malloc2(w*h*ncom*4))) {
    perr = 0x506;  free2(new);  free2(jpg);  free2(huff);  return(0); }
  memset(decode, 0, w*h*ncom*4);
  if (ncom==1)
    for (i=0; i<256; i++)
      new[i*3] = new[i*3+1] = new[i*3+2] = i;
  else
    for (i=0; i<256; i++) {
      crop[i] = 0;  crop[i+256] = i;  crop[i+512] = 255;
      ytbl[i]     = (((i-128)*359+128)>>8)+256;
      ytbl[i+256] = (i-128)*88+128-65536;
      ytbl[i+512] = (i-128)*183;
      ytbl[i+768] = (((i-128)*454+128)>>8)+256; }
  dy = 3*(w-mw8);
  for (i=0; i<snum; i++) {
    if (shuff[i])  slen[i] = min(slen[i],shuff[i]-sstart[i]);
    slen[i] = jpeg_unstuff(jpg+sstart[i], slen[i], 0);  }
  Minpwidth = Minpheight = 0;
  for (spos=0; spos<snum; spos++) {                              /* Decode */
    if (spos)  if (shuff[spos-1]) {
      j = jpg[shuff[spos-1]+2]*256+jpg[shuff[spos-1]+3]+shuff[spos-1]+2;
      h = shuff[spos-1] + 4;
      while (h<j) {
        temp = huff+65536*((jpg[h]>>4)*4+(jpg[h]&0xF));
        temp2 = hufflen+256*((jpg[h]>>4)*4+(jpg[h]&0xF));
        for (i=0; i<16; i++)
          hl[i] = jpg[h+1+i];
        h += 17;
        for (l=1; l<=16; l++)
          for (i=0; i<hl[l-1]; i++, h++) {
            temp2[jpg[h]] = l;
            memset(temp, jpg[h], ln[l-1]);
            temp += ln[l-1]; } }
      h = ((pheight+mh8-1)/(mh8))*mh8; }
    s = shead[spos];  scom0 = 0;
    for (i=0; i<ncom; i++)
      if (jpg[s+5]==com[i*7+4])  scom0 = i;
    for (i=0; i<jpg[s+4]; i++) {
      if (com[(i+scom0)*7+4]!=jpg[s+5+i*2])  break;
      com[(i+scom0)*7+5] = jpg[s+6+i*2]>>4;
      com[(i+scom0)*7+6] = (jpg[s+6+i*2]&0xF)+4; }
    scom1 = scom0+i;  so = (i==1);  if (scom0)  so = 0;
    succ = jpg[s+i*2+7];
    if (i!=jpg[s+4])  break;
    spec0 = jpg[s+i*2+5];  spec1 = jpg[s+i*2+6];
    r = dcbase[0] = dcbase[1] = dcbase[2] = 0;
    s = sstart[spos]*8;  lastr = rnum = 0;  firstspec = 1;
    for (y=0; y<h; y+=mh8*!so+8*so)
      for (x=0; x<w*!so+pwidth*so; x+=mw8*!so+8*so, r++) {
        if (r && r==restart) {
          r = dcbase[0] = dcbase[1] = dcbase[2] = dcbase[3] = 0;
          s = (s+7)&0xFFFFFFF8;
          if (jpg[s>>3]==0xFF && jpg[(s>>3)+1]==0xD0+rnum) {
            s += 16;  lastr = s;  rnum = (rnum+1)%8; }
          else {
            for (i=(lastr>>3); i<fsize-1; i++)
              if (jpg[i]==0xFF && jpg[i+1]==0xD0+rnum) {
                for (j=i+1; j<fsize; j++)
                  if (jpg[j]==0xFF && (jpg[j+1]&0xD8)==0xD0) {
                    if (jpg[j+1]==0xD0+rnum)  i = j;
                    break; }
                s = (i+2)<<3;  lastr = s;  rnum = (rnum+1)%8; break; } } }
        for (k=scom0; k<scom1; k++)
          for (j=0; j<com[k*7+2]*!so+so; j++)
            for (i=0; i<com[k*7+1]*!so+so; i++) {
              dest = decode+(((x/8)+i+((y/8)+j)*(w/8))*ncom+k)*64;
              s = jpeg_decode_spectral(jpg, s, fsize, huff+(com[k*7+5]<<16),
                            hufflen+(com[k*7+5]<<8), huff+(com[k*7+6]<<16),
                            hufflen+(com[k*7+6]<<8), quant+(com[k*7+3]<<6),
                            dest, spec0, spec1, firstspec, succ);
              if (spos)  firstspec = 0;
              if (!spec0 && !(succ&0xF0))
                dcbase[k] = dest[0] = (dcbase[k]+dest[0]); } } }
  for (y=0; y<h; y+=mh8)  for (x=0; x<w; x+=mw8, r++) {
    for (k=0; k<ncom; k++)
      for (j=0,l=k*512; j<com[k*7+2]; j++,l+=7*mw8)
        for (i=0; i<com[k*7+1]; i++, l+=(rr[k]<<3)) {
          dest = decode+(((x/8)+i+((y/8)+j)*(w/8))*ncom+k)*64;
          jpeg_undct(dest, dres+l, mw8, rr[k], rr[k+4], 128); }
    if (ncom==1)
      for (j=l=0; j<mh8; j++,l+=mw8)
        memcpy(new+768+x+(y+j)*w, dres+l, mw8);
    else if (ncom==3) {
      d = (x+y*w)*3;
      for (j=l=0; j<mh8; j++, d+=dy) {
        for (i=0; i<mw8; i++,l++,d+=3) {
          Y = dres[l];  Cb = dres2[l];  Cr = dres3[l];
          new[d]   = crop[ytbl[Cr]+Y];
          new[d+1] = crop[Y-((ytbl2[Cb]+ytbl3[Cr])>>8)];
          new[d+2] = crop[ytbl4[Cb]+Y]; } } }
    else {
      d = (x+y*w)*3;
      for (j=l=0; j<mh8; j++, d+=dy) {
        for (i=0; i<mw8; i++,l++,d+=3) {
          /* CMYK should be simple, but in some cases it didn't work, whereas,
           * this does, which may be what is called YCCK. */
          if (!ycck)
          {
            M = dres[l];       Y = dres[l+512];
            C = dres[l+1024];  K = 255-dres[l+1536];
          }
          else
          {
            Y = dres[l];  Cb = dres2[l];  Cr = dres3[l];
            C = crop[ytbl[Cr]+Y];
            M = crop[Y-((ytbl2[Cb]+ytbl3[Cr])>>8)];
            Y = crop[ytbl4[Cb]+Y];
            K = 255-dres[l+1536];
          }
          new[d]   = max(0, 255-C*(255-K)/255-K);
          new[d+1] = max(0, 255-M*(255-K)/255-K);
          new[d+2] = max(0, 255-Y*(255-K)/255-K); } } } }
  free2(decode);
  free2(huff);
  free2(jpg);
  if (pwidth!=w) {
    if (pinter) {
      for (i=1; i<pheight; i++)
        memmove(new+768+i*pwidth, new+768+i*w, pwidth);
      size = 768+pwidth*pheight; }
    else {
      for (i=1; i<pheight; i++)
        memmove(new+i*pwidth*3, new+i*w*3, pwidth*3);
      size = pwidth*pheight*3; }
    if (jpg=realloc2(new, size))
      new = jpg; }
  return(new);
}

#ifndef DOSLIB
STATIC uchar *load_pcx(FILE *fptr)
/* Load in a PCX/PCC file.  Exits with values in pwidth and pheight.
 *  pinter=1 for palette images and 0 for RGB images.  If an error occurs a
 *  null is passed back.  See load_graphic for return data format.  The file
 *  pointer is closed.
 * Enter: FILE *fptr: pointer to the open PCX file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/26/96-DWM */
{
  uchar head[128], *new, *pcx;
  long scan, bits, size, i, j, d, s, k, fsize, w;

  pspec[0] = 7;
  (*fseek2)(fptr, 0, SEEK_END);
  fsize = (*ftell2)(fptr)-128;
  (*fseek2)(fptr, 0, SEEK_SET);
  (*fread2)(head, 1, 128, fptr);
  pwidth  = ((short *)(head+8))[0] -((short *)(head+4))[0]+1;
  pheight = ((short *)(head+10))[0]-((short *)(head+6))[0]+1;
  scan = ((short *)(head+66))[0];
  bits = head[3];  pinter = (head[65]==1);
  if (bits>1 && bits<8)  bits = 0;
  if (bits==1 && head[65]<=4) { bits = head[65];  pinter = 1; }
  if ((bits!=8 && !pinter) || pwidth<=0 || pheight<=0 || (bits!=8 &&
      bits>4)) {
    perr = 0xB03;  (*fclose2)(fptr);  return(0); }
  size = pwidth*pheight;
  if (pinter)  size += 768;
  else         size *= 3;
  if (bits>1 && bits<8)  size = 768+pwidth*pheight+pwidth;
  if (!(new=malloc2(size))) {
    perr = 0x506;  (*fclose2)(fptr);  return(0); }
  if (!(pcx=malloc2(fsize))) {
    free2(new);
    perr = 0x606;  (*fclose2)(fptr);  return(0); }
  (*fread2)(pcx, 1, fsize, fptr);
  (*fclose2)(fptr);
  for (i=j=0; i<256; i++,j+=3)
    new[j] = new[j+1] = new[j+2] = i;
  if (bits==1)
    new[3] = new[4] = new[5] = 255;
  if (bits<=4 && (head[1]==2 || head[1]>=4) && (head[68]==1 || head[68]==2 ||
      bits!=1))
    memcpy(new, head+16, 48);
  d = 768*pinter;
  if (bits<8) { w = pwidth;  pwidth = (w*bits+7)/8; }
  for (s=i=0; s<fsize && d<size;)
    if ((pcx[s]&0xC0)==0xC0 && s<fsize-1) {
      for (k=0; k<(pcx[s]&0x3F) && d<size; k++,i++) {
        if (i==scan)    i = 0;
        if (i<pwidth) { new[d] = pcx[s+1];  d++; } }
      s += 2; }
    else {
      if (i==scan)    i = 0;
      if (i<pwidth) { new[d] = pcx[s];  d++; }
      s++; i++; }
  if (bits==1) {
    for (j=pheight-1; j>=0; j--)
      for (i=w-1; i>=0; i--)
        new[j*w+i+768] = ((new[768+j*pwidth+(i/8)]&imask[i%8])!=0);
    pwidth = w; }
  if (bits>1 && bits<=4) {
    d = 768+w;
    for (j=pheight-1; j>=0; j--)
      for (i=0; i<w; i++) {
        new[j*w+i+d] = ((new[768+j*bits*scan+(i/8)]&imask[i%8])!=0);
        new[j*w+i+d] |= ((new[768+(j*bits+1)*scan+(i/8)]&imask[i%8])!=0)*2;
        if (bits>2)
          new[j*w+i+d]|=((new[768+(j*bits+2)*scan+(i/8)]&imask[i%8])!=0)*4;
        if (bits>3)
          new[j*w+i+d]|=((new[768+(j*bits+3)*scan+(i/8)]&imask[i%8])!=0)*8; }
    pwidth = w;
    memmove(new+768, new+d, pwidth*pheight); }
  if (bits>4 && pinter && head[1]>=4) {
    while (pcx[s]!=0x0C && s<fsize)  s++;
    if (s+769<=fsize)
      memcpy(new, pcx+s+1, 768); }
  if (!pinter)
    plane_to_rgb(new, pwidth, pheight);
  free2(pcx);
  return(new);
}
#endif

STATIC uchar *load_pil(FILE *fptr)
/* Check if the open file pointer can be read via an external process using
 *  Python's PIL library.  If so, read it.
 * Enter: FILE *fptr: pointer to the open PPM file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.  Null if the image cannot be read by
 *                    PIL.                                        4/3/17-DWM */
{
  long destlen, offset;
  uchar *dest;

  /* The command uses -u to prevent buffering and ensure stdin and stdout are
   * in binary mode.  This copies memory far too many times. */
  char *command = "python -c \"\n"
    "import six, sys, PIL.Image\n"
    "import os, msvcrt\n"
    "msvcrt.setmode(sys.stdin.fileno(), os.O_BINARY)\n"
    "msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)\n"
    "inp = six.BytesIO(sys.stdin.read())\n"
    "buf = six.BytesIO()\n"
    "PIL.Image.open(inp).convert('RGB').save(buf, format='PPM')\n"
    "sys.stdout.write(buf.getvalue())\"";

  dest = pipe_file_to_command(command, fptr, fread2, 512*1024*1024, &destlen);
  if (!dest) {
    perr = ProcessError;
    return 0;
  }
  sscanf(dest, "P6%ld%ld%*d%*c%n", &pwidth, &pheight, &offset);
  if (destlen-offset < pwidth * pheight * 3) {
    free2(dest);
    return 0;
  }
  memmove(dest, dest+offset, destlen-offset);
  pinter = 0;
  return dest;
}

STATIC uchar *load_ppm(FILE *fptr)
/* Load in a PBM/PGM/PNM/PPM file.  Exits with values in pwidth and pheight.
 *  pinter=1 for palette images and 0 for RGB images.  If an error occurs a
 *  null is passed back.  See load_graphic for return data format.  The file
 *  pointer is closed.
 * Enter: FILE *fptr: pointer to the open PPM file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/25/96-DWM */
{
  uchar *new, *ppm, total[2];
  long type, scale=1, i, j, size;
  float low=0, high=0;

  (*fread2)(total, 1, 2, fptr);
  type = total[1]-'1';
  pspec[0] = 6;
  pspec[6] = (type>=3);
  pwidth = ppm_number(fptr, 0);
  pheight = ppm_number(fptr, 0);
  if (type==7 || type==8) {
    low  = ppm_float_number(fptr, 0, 0);
    high = ppm_float_number(fptr, 0, 0); }
  else if (type!=0 && type!=3)
    scale = ppm_number(fptr, 0);
  if (pwidth<=0 || pheight<=0 || scale<=0) {
    perr = 0x805;  (*fclose2)(fptr);  return(0); }
  size = pwidth*pheight;
  new = malloc2(size*(1+2*((type%3)==2))+768*((type%3)<2));
  if (!new) {
    perr = 0x506;  (*fclose2)(fptr);  return(0); }
  ppm = new;  pinter = 0;
  if ((type%3)<2) {
    for (i=0; i<256; i++)
      new[i*3] = new[i*3+1] = new[i*3+2] = i;
    ppm = new+768;  pinter = 1; }
  if (type<3)
    for (i=0; i<size*(1+2*(type==2)); i++)
      ppm[i] = ppm_number(fptr, scale);
  else if (type<6) {
    (*fseek2)(fptr, 1, SEEK_CUR);
    if (type==3) {
      (*fread2)(ppm+size-(size+7)/8, 1, (size+7)/8, fptr);
      for (i=0, j=size-(size+7)/8; i<size; i++)
        ppm[i] = ((imask[i%8]&ppm[j+i/8])!=0)*255; }
    else {
      (*fread2)(ppm, 1+2*(type==5), size, fptr);
      for (i=0; i<size*(1+2*(type==5)) && scale!=255; i++)
        ppm[i] = ppm[i]*255/scale; } }
  else
    for (i=0; i<size*(1+2*(type==2)); i++)
      ppm[i] = ppm_float_number(fptr, low, high);
  (*fclose2)(fptr);
  return(new);
}

#ifndef DOSLIB
STATIC uchar *load_tga(FILE *fptr)
/* Load in a 24-bit or 32-bit Targa file.  Exits with values in pwidth and
 *  pheight.  pinter=0, since this is an RGB image.  If an error occurs a
 *  null is passed back.  See load_graphic for return data format.  The file
 *  pointer is closed.
 * Enter: FILE *fptr: pointer to the open TGA file.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/26/96-DWM */
{
  uchar head[18], *new, *tga, temp;
  long top, size, trle, fsize, s=0, d=0, val, i, bits;

  pspec[0] = 8;
  (*fseek2)(fptr, 0, SEEK_END);
  fsize = (*ftell2)(fptr);
  (*fseek2)(fptr, 0, SEEK_SET);
  (*fread2)(head, 1, 18, fptr);
  pwidth  = ((short *)(head+12))[0];
  pheight = ((short *)(head+14))[0];
  pinter = 0;
  bits = head[16];
  top = head[17];
  size = pwidth*pheight*3;
  trle = (head[2]==0x0A);
  (*fseek2)(fptr, 0x18+head[1]*((head[7]+7)/8)*((short *)(head+5))[0]+head[0],
        SEEK_SET);
  if (bits==32)
    (*fseek2)(fptr, 0x12, SEEK_SET);
  if (!(new=malloc2(size))) {
    perr = 0x506;  (*fclose2)(fptr);  return(0); }
  if (!trle) {
    if (bits==24)
      (*fread2)(new, 1, size, fptr);
    else
      for (i=0; i<pwidth*pheight; i++) {
        (*fread2)(new+i*3, 1, 3, fptr);
        (*fread2)(&temp, 1, 1, fptr); }
    (*fclose2)(fptr);
    if (!top || bits==32)
      vertical_flip(new, pwidth*3, pheight);
    bgr_to_rgb(new, (long)pwidth*pheight);
    return(new); }
  fsize -= (*ftell2)(fptr);
  if (!(tga=malloc2(fsize))) {
    free2(new);
    perr = 0x606;  (*fclose2)(fptr);  return(0); }
  (*fread2)(tga, 1, fsize, fptr);
  (*fclose2)(fptr);
  while (d<size && s<fsize) {
    val = tga[s];  s++;
    if (val<=127)
      for (i=0; i<(val+1)*3 && d<size && s<fsize; i++,s++,d++)
         new[d] = tga[s];
    else if (s+3<=fsize) {
      for (i=0; i<((val&0x7F)+1)*3 && d+2<size; i+=3,d+=3) {
        new[d] = tga[s];  new[d+1] = tga[s+1];  new[d+2] = tga[s+2]; }
      s += 3; } }
  free2(tga);
  if (!top)
    vertical_flip(new, pwidth*3, pheight);
  bgr_to_rgb(new, pwidth*pheight);
  return(new);
}
#endif

STATIC uchar *load_tif(FILE *fptr)
/* Load an image that is stored as a TIFF file.  The size of the image is
 *  stored in pwidth, pheight.  pinter = 1 if this is a greyscale or
 *  palettized image, pinter = 0 if this is an RGB image.
 * Enter: FILE *fptr: pointer to file to read.  This is guaranteed to be
 *                    closed before the routine returns.
 * Exit:  char *image: pointer to image which the palettized bitmap or the
 *                     RGB image, as mentioned above.  0 indicates an error.
 *                                                             2/21/95-DWM */
{
  short ifd=0, ifd2=0, orient=0;
  long end, temp, temp2, i, j, pic[8]={8,1,1,1,0,0,-1,1}, part=ppart;
  long trydcs=1, *offset=0, *count, onum;
  char *Grey, *tag;

  (*fseek2)(fptr, 0, SEEK_SET);
  pinter = 1;
  if (!(tag=(char *)malloc2(35008))) {
    perr = 0x906;  return(0); }
  memset(tag, 0, 35008);
  (*fread2)(tag+7168, 0x18, 1, fptr);
  end = (tag[7168]=='M');
  if (end)  endianl(ltag+1793);
  (*fseek2)(fptr, ltag[1793], SEEK_SET);
  if (part<0) part = 0;
  pspec[1] = part&0x7F;  ((short *)(pspec+0x12))[0] = (part>>7);
  do {
    (*fread2)(&ifd, 2, 1, fptr);
    if (end)  endian(&ifd);
    if (ifd>256)  ifd = 256;
    (*fread2)(tag+7168, 12, ifd, fptr);
    (*fread2)(&temp, 4, 1, fptr);
    if (end)  endianl(&temp);
    if (part && !temp) { part = 0;  trydcs = 0; }
    if (part)
      (*fseek2)(fptr, temp, SEEK_SET);
    else if (temp)
      pspec[1] |= 0x80;
    part--; }
  while (part>=0);
  reorder_tif(tag, tag+7168, ifd, end, fptr);
  for (i=0; i<256; i++)
    utag[10240+i*3] = utag[10240+i*3+1] = utag[10240+i*3+2] = i;
  memset(software, 0, TIF_MAXSOFTWARELEN);
  memset(serial, 0, TIF_MAXSERIALLEN);
  memset(tifmodel, 0, TIF_MAXMODELLEN);
  for (i=0; i<ifd; i++)
    switch (uitag[i*14]) {
      case 0x100: pwidth = ltag[i*7+3]; break;
      case 0x101: pheight = ltag[i*7+3]; break;
      case 0x102: for (j=0; j<ltag[i*7+1] && j<4; j++)
          if (ltag[i*7+3+j]!=ltag[i*7+3] || ltag[i*7+3]>23) {
            perr=0xB04; free2(tag); free2(offset);
            (*fclose2)(fptr);  return(0); }
        pic[0] = ltag[i*7+3]; break;
      case 0x103: pic[1] = ltag[i*7+3]; break;
      case 0x106: pic[2] = ltag[i*7+3];
        if (pic[2]==32803) {                        /* For DCS 330 cameras */
          pic[2] = 1;  pinter = 1; }
        if (pic[2]<0 || pic[2]>=6) {
          perr = 0x1204; free2(tag); free2(offset);
          (*fclose2)(fptr);  return(0); }
        if (pic[2]==2 || pic[2]>=5)  pinter = 0; break;
      case 0x10A: if (ltag[i*7+3]==2)  pic[4] = 1; break;
      case 0x110: if (ltag[i*7+1]<=4)  break;
        (*fseek2)(fptr, ltag[i*7+2], SEEK_SET);
        (*fread2)(tifmodel, 1, min(ltag[i*7+1], TIF_MAXMODELLEN-1), fptr);
        break;
      case 0x112: orient = ((ltag[i*7+3]-1)&7)+1;
        if (orient!=1 && orient!=3 && orient!=6 && orient!=8) {
          perr = 0x1104;  free2(tag); free2(offset);
          (*fclose2)(fptr);  return(0); }
        break;
      case 0x115: pic[7] = ltag[i*7+3]; break;
      case 0x116: pic[6] = ltag[i*7+3]; break;
      case 0x117: temp = 0;
      case 0x111: if (itag[i*14]==0x111)  temp = ltag[i*7+1];
        if (!offset) {
          onum = ltag[i*7+1];
          if (!(offset=malloc2(onum*2*sizeof(long)))) {
            perr = 0x1404; free2(tag); (*fclose2)(fptr);  return(0); }
          count = offset+onum; }
        else if (ltag[i*7+1]!=onum) {
          perr = 0x1404; free2(tag); free2(offset);
          (*fclose2)(fptr);  return(0); }
        if (ltag[i*7+1]<=4)
          memcpy(offset+temp, ltag+i*7+3, ltag[i*7+1]*4);
        else {
          (*fseek2)(fptr, ltag[i*7+2], SEEK_SET);
          (*fread2)(offset+temp, itag[i*14+1]-(itag[i*14+1]==3), ltag[i*7+1],
                fptr);
          switch (itag[i*14+1]) {
            case 1: for (j=ltag[i*7+1]-1; j>=0; j--)
              offset[temp+j] = ((unsigned char *)(offset+temp))[j]; break;
            case 3: for (j=ltag[i*7+1]-1; j>=0; j--) {
              if (end)  endian(((unsigned short *)(offset+temp))+j);
              offset[temp+j] = ((unsigned short *)(offset+temp))[j]; } break;
            case 4: if (end)  for (j=0; j<ltag[i*7+1]; j++)
              endianl(offset+temp+j); } } break;
      case 0x11A: case 0x11B: if (ltag[i*7+4])
        ((short *)(pspec+6))[0] = ltag[i*7+3]/ltag[i*7+4]; break;
      case 0x11C: if (ltag[i*7+3]!=1) {
          perr = 0x1304;  free2(tag);  free2(offset);
          (*fclose2)(fptr);  return(0); }
        break;
      case 0x124: if (!(ltag[i*7+3]&3))  break;
        perr = 0xA04; free2(tag); free2(offset); (*fclose2)(fptr); return(0);
      case 0x125: if (!(ltag[i*7+3]&2))  break;
        perr = 0xA04;  free2(tag);  free2(offset);
        (*fclose2)(fptr);  return(0);
      case 0x131: case 0x868A:             /* 868A is actually GEOTIFF tag */
        if (ltag[i*7+1]<=4)  break;
        (*fseek2)(fptr, ltag[i*7+2], SEEK_SET);
        (*fread2)(software, 1, min(ltag[i*7+1], TIF_MAXSOFTWARELEN-1), fptr);
        break;
      case 0x13D: pic[3] = ltag[i*7+3]; break;
      case 0x140: if (ltag[i*7+1]>768)  {
          perr = 0x1004; free2(tag); free2(offset);
          (*fclose2)(fptr);  return(0); }
        (*fseek2)(fptr, ltag[i*7+2], SEEK_SET);
        (*fread2)(tag+7168, itag[i*14+1]-(itag[i*14+1]==3), ltag[i*7+1], fptr);
        temp = ltag[i*7+1]/3;  temp2 = 10240;
        switch (itag[i*14+1]) {
          case 1: for (j=0; j<temp; j++) {
            utag[temp2+j*3]   = utag[7168+j];
            utag[temp2+j*3+1] = utag[7168+j+temp];
            utag[temp2+j*3+2] = utag[7168+j+2*temp]; } break;
          case 3: if (end) for (j=0; j<ltag[i*7+1]; j++)
              endian(uitag+3584+j);
            for (j=0; j<temp; j++) {
              utag[temp2+j*3]   = uitag[3584+j]>>8;
              utag[temp2+j*3+1] = uitag[3584+j+temp]>>8;
              utag[temp2+j*3+2] = uitag[3584+j+2*temp]>>8; } break;
          case 4: if (end) for (j=0; j<ltag[i*7+1]; j++)
              endianl(ltag+1792+j);
            for (j=0; j<temp; j++) {
              utag[temp2+j*3]   = ultag[1792+j]>>8;
              utag[temp2+j*3+1] = ultag[1792+j+temp]>>8;
              utag[temp2+j*3+2] = ultag[1792+j+2*temp]>>8; } }
        pic[5] = 1; break;
      case 0x14A: if (ltag[i*7+1]>4 || i>=ifd || !trydcs) break;
        (*fseek2)(fptr, ltag[i*7+2+ltag[i*7+1]], SEEK_SET);
        (*fread2)(&ifd2, 2, 1, fptr);
        if (end)  endian(&ifd2);
        pic[5] = 2;                                    /* DCS color camera */
        if (!ifd2) {
          (*fseek2)(fptr, ltag[i*7+3], SEEK_SET);
          (*fread2)(&ifd2, 2, 1, fptr);
          if (end)  endian(&ifd2);
          pic[5] = 0; }                                  /* DCS b&w camera */
        if (ifd+ifd2>256)  ifd2 = 256-ifd;
        (*fread2)(tag+7168+12*ifd, 12, ifd2, fptr);
        reorder_tif(tag+28*ifd, tag+7168+12*ifd, ifd2, end, fptr);
        pspec[1] |= 0x80;
        ifd += ifd2;  ifd2 = 0; break;
      case 0x827D: if (ltag[i*7+1]<=4)  break;
        (*fseek2)(fptr, ltag[i*7+2], SEEK_SET);
        (*fread2)(serial, 1, min(ltag[i*7+1], TIF_MAXSERIALLEN-1), fptr); }
  pspec[0] = 9;
  pspec[3] = (pic[1]==32773)+2*(pic[2]==5)+(pic[2]==5&&pic[3]);
  if (pic[1]==3 || pic[1]==4)  pspec[3] = 3;
  pspec[6] = 1-end;
  ((long *)(pspec+8))[0] = pic[6];
  if (!pwidth || !pheight) {
    perr = 0x805;  free2(tag);  free2(offset);  (*fclose2)(fptr);
    return(0); }
  if (pinter)
    temp = pwidth*pheight+768;
  else
    temp = pwidth*pheight*3;
  if (!(Grey=(char *)malloc2(temp))) {
    perr = 0x506;  free2(tag);  free2(offset);
    (*fclose2)(fptr);  return(0); }
  if (pinter)
    memcpy(Grey, utag+10240, 768);
  temp = read_tif(Grey+768*pinter, offset, count, pic, pwidth,
                  pheight, fptr);
  (*fclose2)(fptr);
  free2(tag);  free2(offset);
  /** Reenable the following line to accept only valid tiff files **
  if (temp) { perr = 0x101;  free2(Grey);  return(0); }
   ** See above **/
  Grey = reorient(Grey, orient);
  return(Grey);
}

STATIC long lzw(uchar *dest, uchar *source, uchar *buf, long len, long maxlen,
                short numbits)
/* Compress a set of data using the LZW compression scheme.  Note that if the
 *  destination buffer is filled, there will not be an eof code written.  In
 *  the worst case, the LZW routine will produce output which is 1.391 times
 *  the size of the original input.
 * Enter: long dest: absolute memory location for compressed data.
 *        long source: memory location for data to compress.
 *        char *buf: 28k of work space.
 *        long len: length of data to compress.  Must be at least 2 bytes.
 *        long maxlen: length of output buffer.  Must be at least 8 bytes.
 *        int numbits: number of bits to begin.
 * Exit:  long len: length of compressed data, including eof. 12/11/94-DWM */
{
  uchar codesize, initcodesize;
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
lzw1:     mov [edx], eax
          add edx, 0x04
          loop    lzw1
          mov ecx, maxlen
          and ecx, 0x03
          add ecx, 0x04
lzw2:     mov [edx], al
          inc edx
          loop    lzw2
          mov eax, dest                   ;almost the same beginning as unlzw
          sub eax, 0x04
          add maxlen, eax
          mov al, BYTE PTR numbits
          mov cl, al
          mov dx, 0x01
          shl dx, cl
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
lzw3:     mov edi, eax                                    ;add code to output
          shl eax, cl
          or  [edx], eax
          add cl, codesize
          xor eax, eax
          mov al, cl
          shr ax, 0x03
          add edx, eax
          and cl, 0x07
          cmp edx, maxlen
          jge     lzw14
          cmp di, eof
          je      lzw14
          cmp di, clearcode
          jne     lzw7
          mov al, initcodesize                                   ;clear codes
          mov codesize, al
          mov ax, initmaxcode
          mov maxcode, ax
          mov ax, firstfree
          mov freecode, ax
          xor eax, eax
          xor ebx, ebx
lzw4:     mov [esi+ebx], eax
          add bx, 0x04
          cmp bx, 0x2000
          jne     lzw4
          mov nextlink, bx
          jmp     lzw7
lzw5:     mov ax, [esi+edi+1]                         ;continue building code
          mov incode, ax
          jmp     lzw7
lzw6:     mov ax, [esi+ebx+1]
          mov incode, ax
lzw7:     xor eax, eax                                       ;build next code
          cmp len, 0x00
          je      lzw12
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
lzw8:     test bx, bx
          je      lzw10
          cmp [esi+ebx], al
          je      lzw6
          xor edi, edi
          mov di, [esi+ebx+3]
          test edi, edi
          je      lzw9
          cmp [esi+edi], al
          je      lzw5
          mov bx, [esi+edi+3]
          jmp     lzw8
lzw9:     mov edi, ebx
lzw10:    mov bx, nextlink
          mov [esi+edi+3], bx
          add nextlink, 0x05
          mov [esi+ebx], al
          xor edi, edi
          mov di, freecode
          mov [esi+ebx+1], edi
          mov di, incode
          mov incode, ax
          shl edi, cl
          or  [edx], edi
          add cl, codesize
          mov ax, cx
          shr ax, 0x03
          add edx, eax
          and cl, 0x07
          cmp edx, maxlen
          jge     lzw14
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jle     lzw7
          inc codesize
          shl maxcode, 0x01
          cmp codesize, 0x0C
          jl      lzw7
          jg      lzw11
          sub maxcode, 0x01
          jmp     lzw7
lzw11:    dec codesize
          mov ax, clearcode
          jmp     lzw3
lzw12:    mov ax, incode                                     ;write last code
          shl eax, cl
          or  [edx], eax
          add cl, codesize
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jle     lzw13
          cmp codesize, 0x0C
          jne     lzw13
          inc codesize
lzw13:    xor eax, eax
          mov ax, eof
          jmp     lzw3
lzw14:    xor eax, eax                               ;return total lzw length
          mov al, cl
          add al, 0x07
          shr al, 0x03
          add eax, edx
          sub eax, dest
          mov finlen, eax
    }
  return(finlen);
}

STATIC long lzwlen(uchar *source, uchar *buf, long len, short numbits)
/* Compute the length that data compressed with the lzw compression scheme
 *  will end up being.  No actual data is written.
 * Enter: long source: memory location for data to compress.
 *        char *buf: 28k of work space.
 *        long len: length of data to compress.  Must be at least 2 bytes.
 *        int numbits: number of bits to begin.
 * Exit:  long len: length of compressed data, including eof. 12/11/94-DWM */
{
  uchar codesize, initcodesize;
  ushort clearcode, eof, firstfree, freecode, maxcode, initmaxcode, incode,
         nextlink;
  long finlen;

  _asm {
          xor ebx, ebx
          mov al, BYTE PTR numbits
          mov cl, al
          mov dx, 0x01
          shl dx, cl
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
lzw3:     mov edi, eax                                    ;add code to output
          shl eax, cl
          add cl, codesize
          xor eax, eax
          mov al, cl
          shr ax, 0x03
          add edx, eax
          and cl, 0x07
          cmp di, eof
          je      lzw14
          cmp di, clearcode
          jne     lzw7
          mov al, initcodesize                                   ;clear codes
          mov codesize, al
          mov ax, initmaxcode
          mov maxcode, ax
          mov ax, firstfree
          mov freecode, ax
          xor eax, eax
          xor ebx, ebx
lzw4:     mov [esi+ebx], eax
          add bx, 0x04
          cmp bx, 0x2000
          jne     lzw4
          mov nextlink, bx
          jmp     lzw7
lzw5:     mov ax, [esi+edi+1]                         ;continue building code
          mov incode, ax
          jmp     lzw7
lzw6:     mov ax, [esi+ebx+1]
          mov incode, ax
lzw7:     xor eax, eax                                       ;build next code
          cmp len, 0x00
          je      lzw12
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
lzw8:     test bx, bx
          je      lzw10
          cmp [esi+ebx], al
          je      lzw6
          xor edi, edi
          mov di, [esi+ebx+3]
          test edi, edi
          je      lzw9
          cmp [esi+edi], al
          je      lzw5
          mov bx, [esi+edi+3]
          jmp     lzw8
lzw9:     mov edi, ebx
lzw10:    mov bx, nextlink
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
          jle     lzw7
          inc codesize
          shl maxcode, 0x01
          cmp codesize, 0x0C
          jl      lzw7
          jg      lzw11
          sub maxcode, 0x01
          jmp     lzw7
lzw11:    dec codesize
          mov ax, clearcode
          jmp     lzw3
lzw12:    mov ax, incode                                     ;write last code
          add cl, codesize
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jle     lzw13
          cmp codesize, 0x0C
          jne     lzw13
          inc codesize
lzw13:    mov ax, eof
          jmp     lzw3
lzw14:    xor eax, eax                               ;return total lzw length
          mov al, cl
          add al, 0x07
          shr al, 0x03
          add eax, edx
          mov finlen, eax
    }
  return(finlen);
}

STATIC void plane_to_rgb(uchar *image, long w, long h)
/* Convert a file in the format of the red,green,and blue planes (organized
 *  on a line by line basis) to RGB triplets.
 * Enter: uchar *image: pointer to imgae to convert.
 *        long w, h: size of image in pixels.                  2/26/96-DWM */
{
  uchar *buf;
  long i, j;

  if (!(buf=malloc2(w*3))) { perr = 0x1506; return; }
  for (j=0; j<h; j++) {
    for (i=0; i<w; i++) {
      buf[i*3]   = image[j*3*w+i];
      buf[i*3+1] = image[j*3*w+i+w];
      buf[i*3+2] = image[j*3*w+i+w+w]; }
    memmove(image+j*3*w, buf, w*3); }
  free2(buf);
}

STATIC float ppm_float_number(FILE *fptr, float low, float high)
/* Read in the next ASCII number in a PPM file.  This ignores comments and
 *  scales the value to a range of 0-255 if required.
 * Enter: FILE *fptr: pointer to open file.  The file pointer is moved.
 *        long low, high: both 0 for return the actual number read, otherwise
 *                        these are the minimum and maximum values which are
 *                        scaled to 0 to 255.
 * Exit:  float num: the number read/scaled.  -1 if the file has ended.
 *                                                             2/25/96-DWM */
{
  float num, val;

  while (!(val=fscanf(fptr, "%f", &num)))
    fscanf(fptr, "%*[^\n\r]%*c");
  if (val<0)  return(-1);
  if (low==high)  return(num);
  if ((num<low && low<high) || (num>low && low>high))  return(0);
  if ((num>high && low<high) || (num<high && low>high))  return(255);
  return((num-low)*255/(high-low));
}

STATIC long ppm_number(FILE *fptr, long scale)
/* Read in the next ASCII number in a PPM file.  This ignores comments and
 *  scales the value to a range of 0-255 if required.
 * Enter: FILE *fptr: pointer to open file.  The file pointer is moved.
 *        long scale: 0 for return the actual number read, otherwise this is
 *                    the 'maximum' value which is scaled to 255.
 * Exit:  long num: the number read/scaled.  -1 if the file has ended.
 *                                                             2/25/96-DWM */
{
  long num, val;

  while (!(val=fscanf(fptr, "%d", &num)))
    fscanf(fptr, "%*[^\n\r]%*c");
  if (val<0)  return(-1);
  if (!scale)  return(num);
  return(num*255/scale);
}

STATIC long read_tif(uchar *grey, long *count, long *offset, long *pic,
                     long x, long y, FILE *fptr)
/* Read in a TIFF image.  Memory must already have been allocated for the
 *  final image, but work memory is temporarily reserved as needed.
 * Enter: char *grey: location to store image.  This is either a palette
 *                    based image or an RGB image.
 *        long *count, *offset: arrays containing location and length (count)
 *                              of image strips in the file.
 *        long *pic: array of eight values: 0-bits/pixel, 1-TIFF compression
 *                   type, 2-TIFF photometric interpretation, 3-differencing
 *                   predictor (2 if used), 4-set if bit order reversed, 5-1
 *                   if *pal contains a palette or 2 for dcs image, 6-number
 *                   of image rows per image strip, 7-number of samples per
 *                   pixel.
 *        long x, y: size of image to read.
 *        FILE *fptr: open TIFF file to read picture data.
 * Exit:  long error: 0 for okay, otherwise error number.      3/20/95-DWM */
{
  long i, j, k, l, s, temp[5], bit, sl, cl, ul;
  long w=1+2*(!pinter);
  uchar *cdata=0, *udata=0, *cur, *dest=grey;
  long mask[]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};

  if (!pic[0])                          pic[0] = 1;
  if (!pic[1])                          pic[1] = 1;
  if (!pic[6] || pic[6]>y || pic[6]<0)  pic[6] = y;
  if (!pic[7])                          pic[7] = 1+2*(pic[2]==2||pic[2]>=5);
  if (!count[0]) {
    if (pic[1]==1)  count[0] = pic[7]*y*((x*pic[0]+7)/8);
    else { (*fseek2)(fptr, 0, SEEK_END);  count[0] = (*ftell2)(fptr)-offset[0]; } }
  memset(grey, 0, x*y*w);
  if (pic[0]==8 && pic[1]==1 && pic[2]<=3 && pic[2]!=2 && pic[3]!=2 &&
      pic[6]==y && pic[7]==1) {                             /* Simple case */
    (*fseek2)(fptr, offset[0], SEEK_SET);
    (*fread2)(grey, x*w, y, fptr); }
  else for (j=s=0; j<y; j+=pic[6], s++) {
    memset(grey+j*x*w, 0, x*min(pic[6],y-j)*w);
    sl = pic[7]*min(pic[6], y-j)*((x*pic[0]+7)/8);
    (*fseek2)(fptr, offset[s], SEEK_SET);
    if (count[s]+3>=(y-j)*((x*pic[0]+7)/8) || pic[0]<8) {
      if (!(cdata=(char *)malloc2(count[s]+3)))  return(1);
      cl = 1; }
    else { cdata = dest;  cl = 0; }
    udata = 0;  ul = 0;
    if (pic[1]!=1) {
      if (!(udata=(char *)malloc2(sl+3))) {
        if (cl) free2(cdata);  return(1); }
      ul = 1; }
    count[s] = (*fread2)(cdata, 1, count[s], fptr);
    if (count[s]<=0) {
      if (cl) free2(cdata);  if (ul) free2(udata);  return(1); }
    switch (pic[1]) {
      case 1: udata = cdata;  cdata = 0; ul = cl; break;   /* Uncompressed */
      case 2: unccitt_mode2(udata, cdata, sl, count[s], pic[4], x); break;
      case 3: unccitt_t4(udata, cdata, sl, count[s], pic[4], x); break;
      case 4: unccitt_t6(udata, cdata, sl, count[s], pic[4], x, min(pic[6], y-j)); break;
      case 5: unlzwtif(udata, cdata, sl, count[s]); break;
      case 32773: unrle(udata, cdata, sl, count[s]); break;
      case 0xA5AE: unlzw(udata, cdata, sl, count[s], 8); break;
      default: if (cl) free2(cdata);  free2(udata);  return(1); }
    if (cdata) { if (cl) free2(cdata);  cdata = 0; }
    if (pic[3]==2) {                             /* Differencing predictor */
      if (pic[0]!=8) {
        if (ul) free2(udata);  return(1); }
      for (l=0; l<pic[7]; l++)
        for (k=0; k<pic[6] && k<y-j; k++)
          for (i=1; i<x; i++)
            udata[(i+k*x)*pic[7]+l] += udata[(i-1+k*x)*pic[7]+l]; }
    cur = udata;  bit = 0;
    for (k=0; k<pic[6] && k<y-j; k++) {
      for (i=0; i<x; i++) {
        for (l=0; l<pic[7]; l++) {
          if (l<4) {
            if (!pic[4]) switch (pic[0]) {
              case 1: temp[l] = ((cur[0]&imask[bit])!=0); break;
              case 8: temp[l] = cur[0]; break;
              default: temp[l] = ((unsigned long *)cur)[0];
                endianl(temp+l);
                temp[l] = (temp[l]>>(32-bit-pic[0]))&mask[pic[0]]; }
            else
              temp[l] = (((unsigned long *)cur)[0]>>bit)&mask[pic[0]]; }
          bit += pic[0];  while (bit>=8) { cur++;  bit-=8; } }
        switch (pic[2]) {
          case 0: case 1: case 4:              /* Greyscale & transparancy */
            if (pic[0]>8)  temp[0] >>= (pic[0]-8);
            if (pic[0]<8)  {
              for (l=0; l<8-pic[0]; l+=pic[0])
                temp[0] = (temp[0]<<pic[0])|(temp[0]&mask[pic[0]]);
              if (l>8-pic[0])
                temp[0] >>= (l-(8-pic[0])); }
            dest[0] = temp[0]; break;
          case 2: dest[0] = temp[0];  dest[1] = temp[1];            /* RGB */
            dest[2] = temp[2];  dest += 2; break;
          case 3: dest[0] = temp[0]; break;                     /* Palette */
          case 5: dest[0] = max(0, 255-temp[0]-temp[3]);           /* CMYK */
            dest[1] = max(0, 255-temp[1]-temp[3]);
            dest[2] = max(0, 255-temp[2]-temp[3]);  dest += 2; break;
          default: if (cl) free2(cdata);  if (ul) free2(udata);
            return(1); }
        dest++; }
      if (bit && pic[1]!=3) { cur++;  bit = 0; } }
    if (ul) { free2(udata);  udata = 0; } }
  if (pic[5]==2) read_tif_dcs(grey, x, y, x);           /* DCS color image */
  if (!pic[2])                                            /* White is zero */
    for (i=0; i<x*y*w; i++)
      grey[i] = 255-grey[i];
  return(0);
}

void read_tif_dcs(uchar *grey, long x, long y, long ax)
/* Convert an image from the raw DCS values to greyscale values on a scale of
 *  0 to 255.  This is done by ignoring the red and blue component values.
 * Enter: char *grey: pointer to image.
 *        long x, y: size of image.
 *        short ax: width of image in memory.                    3/21/95-DWM */
{
  long i, j, s, min=255, max=0, val;
  uchar table[1024], table2[256];
  double range;

  for (j=0; j<y; j++)
    for (i=2, s=j*ax+2; i<x-2; i++, s++)
      if (!((i+j)&1)) {
        if (grey[s]>max && grey[s]<255)  max = grey[s];
        if (grey[s]<min && grey[s])      min = grey[s]; }
  range = 0.25/(max-min+1);  min *= 4;
  for (i=0; i<1024; i++) {
    if (i>min)  val = 255.*pow(((double)i-min)*range, 0.6);
    else        val = 0;
    if (val<0)    val = 0;
    if (val>255)  val = 255;
    table[i] = val; }
  for (i=0; i<256; i++)
    table2[i] = table[(i<<2)+(i>>6)];
  for (j=1; j<y-1; j++) {
    for (i=3, s=j*ax+3; i<x-3; i++,s++) {
      if ((i+j)&1)
        grey[s-ax-3] = table[grey[s-1]+grey[s+1]+grey[s-ax]+grey[s+ax]];
      else
        grey[s-ax-3] = table2[grey[s]]; }
    memset(grey+(j-1)*ax+x-6, 0, ax-x+6); }
  memset(grey+(y-2)*ax, 0, ax*2);
}

STATIC void reorder_tif(uchar *dest, uchar *tag, long num, long end, FILE *fptr)
/* Change a set of tif records from either endian to little endian.  Also
 *  convert all tags so that all numbers are stored as longs.  The new field
 *  layout becomes Tag|Type|Num|Address|Long1|Long2|Long3|Long4, where tag
 *  and type are shorts, and all other values are longs.  If there are more
 *  than four items, the address and type must be used to reference them.
 * Enter: char *dest: location to store transformed tags.
 *        char *tag: pointer to array containing tags.
 *        long num: number of tags to convert.
 *        long end: 0 for little endian, 1 for big endian.
 *        FILE *fptr: pointer to open TIFF file.               3/20/95-DWM */
{
  unsigned long i, j, *ldest=(long *)dest;
  unsigned short *idest=(short *)dest, itemp[4];
  uchar temp[4];

  for (i=0; i<(unsigned long)num; i++) {
    if (end) {
      endian(itag+6*i);    endianl(ltag+3*i+1);
      endian(itag+6*i+1);  endianl(ltag+3*i+2); }
    idest[i*14]   = itag[i*6];    ldest[i*7+1]  = ltag[i*3+1];
    idest[i*14+1] = itag[i*6+1];  ldest[i*7+2]  = ltag[i*3+2];
    switch (idest[14*i+1]) {
      case 6:  case 8:  case 9:  case 10: idest[14*i+1] -= 5; break;
      case 1:  case 3:  case 4:  case 5:  case 11:  case 12:  break;
      default: idest[14*i+1] = 1; }
    switch (idest[14*i+1]) {
      case 1: if (ldest[i*7+1]<=4) {
          if (end)  endianl(ldest+i*7+2);
          ldest[i*7+3] = dest[i*28+8];   ldest[i*7+4] = dest[i*28+9];
          ldest[i*7+5] = dest[i*28+10];  ldest[i*7+6] = dest[i*28+11];
          ldest[i*7+2] = 0; break; }
        if (!fptr)  break;
        (*fseek2)(fptr, ldest[i*7+2], SEEK_SET);
        (*fread2)(temp, 4, 1, fptr);
        ldest[i*7+3] = temp[0];  ldest[i*7+4] = temp[1];
        ldest[i*7+5] = temp[2];  ldest[i*7+6] = temp[3]; break;
      case 3: if (ldest[i*7+1]<=2) {
          if (end) { endianl(ldest+i*7+2);
            endian(idest+i*14+4);  endian(idest+i*14+5); }
          ldest[i*7+3] = idest[i*14+4];  ldest[i*7+4] = idest[i*14+5];
          ldest[i*7+2] = ldest[i*7+5] = ldest[i*7+6] = 0; break; }
        if (!fptr)  break;
        (*fseek2)(fptr, ldest[i*7+2], SEEK_SET);
        (*fread2)(itemp, 4, 2, fptr);
        if (end) { for (j=0; j<4; j++)  endian(itemp+j); }
        ldest[i*7+3] = itemp[0];  ldest[i*7+4] = itemp[1];
        ldest[i*7+5] = itemp[2];  ldest[i*7+6] = itemp[3]; break;
      case 5: ldest[i*7+1] *= 2;
      case 4: if (ldest[i*7+1]<=1) {
          ldest[i*7+3] = ldest[i*7+2];
          ldest[i*7+2]=ldest[i*7+4]=ldest[i*7+5]=ldest[i*7+6] = 0; break; }
        if (!fptr)  break;
        (*fseek2)(fptr, ldest[i*7+2], SEEK_SET);
        (*fread2)(ldest+i*7+3, 4, 4, fptr);
        if (end)  for (j=0; j<4; j++)  endianl(ldest+i*7+3+j); }
    if (idest[14*i+1]==5)
      ldest[i*7+1] /= 2; }
}

STATIC uchar *reorient(uchar *pic, int orient)
/* Rotate and/or flip an image based on the TIFF-style orientation value.  This
 *  is a value where 1-upright, 2-flipped horizontally, 3-upside down, 4-
 *  upside down and flipped, 5-rotated right and flipped, 6-rotated left, 7-
 *  rotated left and flipped, 8-rotated right.  The image details are stored in
 *  pwidth, pheight, and pinter.  The width and height may be reversed.
 * Enter: uchar *pic: the buffer containing the current picture.
 *        int orient: the orientation value (see above).
 * Exit:  uchar *pic: the upright picture.  This may be the input buffer, or
 *                    the input buffer will have been freed and this is a new
 *                    buffer.                                  10/11/13-DWM */
{
   uchar *new, p, p3[3];
   long i, j, s, d, cw;

   if (orient<=1 || orient>8) /* nothing to do */
      return pic;
   if (orient==2 || orient==3 || orient==5 || orient==7) /* flip horiz. */
   {
      if (pinter)
         for (j=0; j<pheight; j++)
            for (i=0, d=768+j*pwidth; i<pwidth/2; i++)
            {
               p = pic[d+i];
               pic[d+i] = pic[d+pwidth-1-i];
               pic[d+pwidth-1-i] = p;
            }
      else
         for (j=0; j<pheight; j++)
            for (i=0, d=j*pwidth*3; i<pwidth/2; i++)
            {
               memcpy(p3, pic+d+i*3, 3);
               memcpy(pic+d+i*3, pic+d+(pwidth-1-i)*3, 3);
               memcpy(pic+d+(pwidth-1-i)*3, p3, 3);
            }
      if (orient==2)
         return pic;
   }
   if (orient==3 || orient==4) /* flip vertically */
   {
      if (!(new=malloc2(pwidth*3)))
      {
          perr = 0x506;  free2(pic);  return(0);
      }
      d = (3-2*pinter)*pwidth;
      for (j=0; j<pheight/2; j++)
      {
         memcpy(new, pic+768*pinter+j*d, d);
         memcpy(pic+768*pinter+j*d, pic+768*pinter+(pheight-j-1)*d, d);
         memcpy(pic+768*pinter+(pheight-j-1)*d, new, d);
      }
      free2(new);
      return pic;
   }
   cw = ((orient==6 || orient==7)?1:-1);
   if (!(new=malloc2(pwidth*pheight*(3-2*pinter)+768*pinter)))
   {
       perr = 0x506;  free2(pic);  return(0);
   }
   if (pinter)
   {
      memcpy(new, pic, 768);
      for (j=0, s=768; j<pheight; j++)
      {
         d = 768;
         if (cw>0)  d += pheight-1-j;
         else       d += pheight*(pwidth-1)+j;
         for (i=0; i<pwidth; i++, s++, d+=pheight*cw)
            new[d] = pic[s];
      }
   }
   else
   {
      for (j=s=0; j<pheight; j++)
      {
         if (cw>0)  d = pheight-1-j;
         else       d = pheight*(pwidth-1)+j;
         d *= 3;
         for (i=0; i<pwidth; i++, s+=3, d+=pheight*cw*3)
            memcpy(new+d, pic+s, 3);
      }
   }
   i = pwidth;  pwidth = pheight;  pheight = i;
   free2(pic);
   return new;
}

STATIC void reverse_bitorder(uchar *buf, long ln)
/* Reverse the bitorder of a block of bytes.
 * Enter: uchar *buf: array to reverse.
 *        long ln: number of bytes to reverse.                12/29/00-DWM */
{
  long tbl[256], i, j;

  for (i=0; i<256; i++) {
    tbl[i] = 0;
    for (j=0; j<8; j++)
      if (i&(1<<j))  tbl[i] |= (0x80>>j); }
  for (i=0; i<ln; i++)
    buf[i] = tbl[buf[i]];
}

STATIC void unbmp4rle(uchar *source, long len, uchar *dest, long w, long h)
/* Decompress a BMP 4-bit/pixel 'rle' encoded string to an image.
 * Enter: char *source: pointer to compressed data.
 *        long len: length of compressed data.
 *        char *dest: destination to store data.
 *        long w, h: size of destination image.                2/26/96-DWM */
{
  long d=0, s=0, val, size=w*h, i, line=0;

  memset(dest, 0, size);
  do {
   val = source[s];
   if (val) {
     for (i=0; i<val && d<size; i++,d++)
       if (i&1)  dest[d] = (source[s+1]&15);
       else      dest[d] = (source[s+1]>>4);
     s += 2; }
   else if (source[s+1]>=3) {
     for (i=0; i<source[s+1] && s+2+i/2<len && d<size; i++,d++)
       if (i&1)  dest[d] = (source[s+2+i/2]&15);
       else      dest[d] = (source[s+2+i/2]>>4);
     s += 2 + ((i+3)/4)*2; }
   else switch (source[s+1]) {
     case 0: if (d>=line*w+w*2)  line = d/w;
       line++;  d = line*w;  s+= 2;  if (d>=size)  return; break;
     case 1:
       return;
     case 2: d = d+source[s+2]+w*source[s+4]/2;
       if (d>=size) return;
       line = source[s+4];  s += 4; } }
  while (s<len-1 && d<size);
}

STATIC void unbmp8rle(uchar *source, long len, uchar *dest, long w, long h)
/* Decompress a BMP 8-bit/pixel 'rle' encoded string to an image.
 * Enter: char *source: pointer to compressed data.
 *        long len: length of compressed data.
 *        char *dest: destination to store data.
 *        long w, h: size of destination image.                2/26/96-DWM */
{
  long d=0, s=0, val, size=w*h, i, line=0;

  memset(dest, 0, size);
  do {
   val = source[s];
   if (val) {
     for (i=0; i<val && d<size; i++,d++)
       dest[d] = source[s+1];
     s += 2; }
   else if (source[s+1]>=3) {
     for (i=0; i<source[s+1] && s+2+i<len && d<size; i++,d++)
       dest[d] = source[s+2+i];
     s += 2 + ((i+1)/2)*2; }
   else switch (source[s+1]) {
     case 0: if (d>=line*w+w*2)  line = d/w;
       line++;  d = line*w;  s += 2;  if (d>size)  return;  break;
     case 1: return;
     case 2: d = d+source[s+2]+w*source[s+4];
       if (d>=size) return;
       line = source[s+4];  s += 4; } }
  while (s<len-1 && d<size);
}

STATIC void unccitt_prep_bits(uchar *input, long *s, long *sb, long *cur,
                         long srclen, long bitorder)
/* Read in a group of bits so that they can be compared to the ccitt tables.
 * Enter: uchar *input: array containing source data.
 *        long *s: byte offset within source; modified.
 *        long *sb: bit offset within byte of source; modified.
 *        long *cur: current value; modified.
 *        long srclen: length of source in bytes.
 *        long bitorder: 0 for first bit is in the high bit of a byte, 1 if
 *                       the first bit is in the low bit.     12/29/00-DWM */
{
  static uchar rev[256];
  static long fillrev=0;
  long i;

  if (bitorder && !fillrev) {
    for (i=0; i<256; i++)
      rev[i] = ((i&1)<<7)|((i&2)<<5)|((i&4)<<3)|((i&8)<<1)|((i&16)>>1)|
               ((i&32)>>3)|((i&64)>>5)|((i&128)>>7);
    fillrev = 1; }
  if (sb[0]>=8) { s[0] += (sb[0]>>3);  sb[0] &= 7; }
  if (s[0]>=srclen)  return;
  if (s[0]<srclen-2)
    cur[0] = (input[s[0]]<<16)|(input[s[0]+1]<<8)|(input[s[0]+2]);
  else {
    cur[0] = (input[s[0]]<<16);
    if (s[0]<srclen-1)  cur[0] |= (input[s[0]+1]<<8); }
  if (bitorder) {
    ((uchar *)cur)[0] = rev[((uchar *)cur)[0]];
    ((uchar *)cur)[1] = rev[((uchar *)cur)[1]];
    ((uchar *)cur)[2] = rev[((uchar *)cur)[2]]; }
  cur[0] = (cur[0]>>(8-sb[0]))&0xFFFF;
}

STATIC long unccitt_mode2(uchar *output, uchar *input, long ln, long srclen,
                          long bitorder, long w)
/* Uncompress data using the CCITT T4 encoding scheme.  The size of the
 *  output is unpredicatable.  Decompression ends when len characters have
 *  been converted or six EOL codes in a row are sent.  If the first code is
 *  an EOL code, it is ignored.
 * Enter: char *output: pointer to expanded data.
 *        char *input: pointer to data to decompress.
 *        long len: maximum length of output.  Required.
 *        long srclen: maximum length of input.
 *        long bitorder: 0 for first bit is in the high bit of a byte, 1 if
 *                       the first bit is in the low bit.
 *        long w: width of a line in pixels.
 * Exit:  long len: length of CCITT data stream used.          10/4/00-DWM */
{
  uchar rev[256];
  long i, j, first=1, eol=0, s=0, sb=0, d=0, db=0, white=1, cur, p1=0, w8;
  static long mask[]={0x0000,0x8000,0xC000,0xE000,0xF000,0xF800,0xFC00,
      0xFE00,0xFF00,0xFF80,0xFFC0,0xFFE0,0xFFF0,0xFFF8,0xFFFC,0xFFFE,0xFFFF};

  memset(output, 0, ln);
  if (bitorder)
    for (i=0; i<256; i++)
      rev[i] = ((i&1)<<7)|((i&2)<<5)|((i&4)<<3)|((i&8)<<1)|((i&16)>>1)|
               ((i&32)>>3)|((i&64)>>5)|((i&128)>>7);
  w8 = ((w+7)/8)*8;
  while (1) {
    if (sb>=8) { s += (sb>>3);  sb &= 7; }
    if (s>=srclen)  break;
    if (s<srclen-2)
      cur = (input[s]<<16)|(input[s+1]<<8)|(input[s+2]);
    else {
      cur = (input[s]<<16);  if (s<srclen-1)  cur |= (input[s+1]<<8); }
    if (bitorder) {
      ((uchar *)(&cur))[0] = rev[((uchar *)(&cur))[0]];
      ((uchar *)(&cur))[1] = rev[((uchar *)(&cur))[1]];
      ((uchar *)(&cur))[2] = rev[((uchar *)(&cur))[2]]; }
    cur = (cur>>(8-sb))&0xFFFF;
    if (white) {
      for (i=0; ccittwhite[i+2]!=-2; i+=3)
        if (ccittwhite[i]==(cur&mask[ccittwhite[i+1]]))
          break;
      if (ccittwhite[i+2]==-2) { sb++;  continue; }
      sb += ccittwhite[i+1];
      if (ccittwhite[i+2]==-1) {
        eol++;  if (eol==6)  break;
        if (first) { first = eol = 0;  continue; }
        db = ((((d<<3)+db)+(w8-1))/w8)*w8;
        d = (db>>3);  db &= 7;
        p1 = 0;
        if (d>=ln)  break;
        continue; }
      first = 0;
      db += ccittwhite[i+2];  p1 += ccittwhite[i+2];
      if (db>=8) { d += (db>>3);  db &= 7; }  if (d>=ln)  break;
      if (ccittwhite[i+2]<64)  white = 0;
      if (p1==w && ccittwhite[i+2]<64) {
        sb = ((sb+7)/8)*8;
        if (db) { d++;  db = 0; }  p1 = 0;
        white = 1;
        continue; } }
    else {
      for (i=0; ccittblack[i+2]!=-2; i+=3)
        if (ccittblack[i]==(cur&mask[ccittblack[i+1]]))
          break;
      if (ccittblack[i+2]==-2) { sb++;  continue; }
      sb += ccittblack[i+1];
      if (ccittblack[i+2]==-1) {
        eol++;  white = 1;
        db = ((((d<<3)+db)+(w8-1))/w8)*w8;
        d = (db>>3);  db &= 7;
        p1 = 0;
        if (d>=ln)  break;
        continue; }
      for (j=ccittblack[i+2]; j>0; j--) {
        if (!db && j>=8) {
          output[d] = 0xFF;  d++;  p1 += 8;  if (d>=ln)  break;
          j -= 7;  continue; }
        output[d] |= imask[db];
        db++;  p1++;  if (db==8) { d++;  db = 0; if (d>=ln)  break; } }
      if (d>=ln) break;
      if (ccittblack[i+2]<64)  white = 1;
      if (p1==w && ccittblack[i+2]<64) {
        sb = ((sb+7)/8)*8;
        if (db) { d++;  db = 0; }  p1 = 0;
        white = 1;
        continue; } }
    if (((d<<3)+db)==((((d<<3)+db)+(w8-1))/w8)*w8 && white)
      sb = ((sb+7)&0xFF8); }
  if (sb)  s++;
  if (bitorder)  reverse_bitorder(output, ln);
  return(s);
}

STATIC long unccitt_t4(uchar *output, uchar *input, long ln, long srclen,
                       long bitorder, long w)
/* Uncompress data using the CCITT T4 encoding scheme.  The size of the
 *  output is unpredicatable.  Decompression ends when len characters have
 *  been converted or six EOL codes in a row are sent.  If the first code is
 *  an EOL code, it is ignored.
 * Enter: char *output: pointer to expanded data.
 *        char *input: pointer to data to decompress.
 *        long len: maximum length of output.  Required.
 *        long srclen: maximum length of input.
 *        long bitorder: 0 for first bit is in the high bit of a byte, 1 if
 *                       the first bit is in the low bit.
 *        long w: width of a line in pixels.
 * Exit:  long len: length of CCITT data stream used.          10/4/00-DWM */
{
  uchar rev[256];
  long i, j, first=1, eol=0, s=0, sb=0, d=0, db=0, white=1, cur;
  static long mask[]={0x0000,0x8000,0xC000,0xE000,0xF000,0xF800,0xFC00,
      0xFE00,0xFF00,0xFF80,0xFFC0,0xFFE0,0xFFF0,0xFFF8,0xFFFC,0xFFFE,0xFFFF};

  memset(output, 0, ln);
  if (bitorder)
    for (i=0; i<256; i++)
      rev[i] = ((i&1)<<7)|((i&2)<<5)|((i&4)<<3)|((i&8)<<1)|((i&16)>>1)|
               ((i&32)>>3)|((i&64)>>5)|((i&128)>>7);
  while (1) {
    if (sb>=8) { s += (sb>>3);  sb &= 7; }
    if (s>=srclen)  break;
    if (s<srclen-2)
      cur = (input[s]<<16)|(input[s+1]<<8)|(input[s+2]);
    else {
      cur = (input[s]<<16);  if (s<srclen-1)  cur |= (input[s+1]<<8); }
    if (bitorder) {
      ((uchar *)(&cur))[0] = rev[((uchar *)(&cur))[0]];
      ((uchar *)(&cur))[1] = rev[((uchar *)(&cur))[1]];
      ((uchar *)(&cur))[2] = rev[((uchar *)(&cur))[2]]; }
    cur = (cur>>(8-sb))&0xFFFF;
    if (white) {
      for (i=0; ccittwhite[i+2]!=-2; i+=3)
        if (ccittwhite[i]==(cur&mask[ccittwhite[i+1]]))
          break;
      if (ccittwhite[i+2]==-2) { sb++;  continue; }
      sb += ccittwhite[i+1];
      if (ccittwhite[i+2]==-1) {
        eol++;  if (eol==6)  break;
        if (first) { first = eol = 0;  continue; }
        db = ((((d<<3)+db)+(w-1))/w)*w;
        d = (db>>3);  db &= 7;
        if (d>=ln)  break;
        continue; }
      first = 0;
      db += ccittwhite[i+2];
      if (db>=8) { d += (db>>3);  db &= 7; }  if (d>=ln)  break;
      if (ccittwhite[i+2]<64)  white = 0; }
    else {
      for (i=0; ccittblack[i+2]!=-2; i+=3)
        if (ccittblack[i]==(cur&mask[ccittblack[i+1]]))
          break;
      if (ccittblack[i+2]==-2) { sb++;  continue; }
      sb += ccittblack[i+1];
      if (ccittblack[i+2]==-1) {
        eol++;  white = 1;
        db = ((((d<<3)+db)+(w-1))/w)*w;
        d = (db>>3);  db &= 7;
        if (d>=ln)  break;
        continue; }
      for (j=ccittblack[i+2]; j>0; j--) {
        if (!db && j>=8) {
          output[d] = 0xFF;  d++;  if (d>=ln)  break;
          j -= 7;  continue; }
        output[d] |= imask[db];
        db++;  if (db==8) { d++;  db = 0; if (d>=ln)  break; } }
      if (d>=ln) break;
      if (ccittblack[i+2]<64)  white = 1; } }
  if (sb)  s++;
  if (bitorder)  reverse_bitorder(output, ln);
  return(s);
}

STATIC long unccitt_t6(uchar *output, uchar *input, long ln, long srclen,
                       long bitorder, long w, long h)
/* Uncompress data using the CCITT T6 encoding scheme.  The size of the
 *  output is unpredicatable.  Decompression ends when len characters have
 *  been converted or six EOL codes in a row are sent.  If the first code is
 *  an EOL code, it is ignored.
 * Enter: char *output: pointer to expanded data.
 *        char *input: pointer to data to decompress.
 *        long len: maximum length of output.  Required.
 *        long srclen: maximum length of input.
 *        long bitorder: 0 for first bit is in the high bit of a byte, 1 if
 *                       the first bit is in the low bit.
 *        long w, h: size of the image in pixels.
 * Exit:  long len: length of CCITT data stream used.          10/4/00-DWM */
{
  long i, j, k, x, ox, rx, orx, rl, s=0, sb=0, d, db, cur;
  long *run, *oldrun;
  static long mask[]={0x0000,0x8000,0xC000,0xE000,0xF000,0xF800,0xFC00,
      0xFE00,0xFF00,0xFF80,0xFFC0,0xFFE0,0xFFF0,0xFFF8,0xFFFC,0xFFFE,0xFFFF};

  run = malloc2(w*4*sizeof(long));  oldrun = run+w*2;
  if (!run)  return(0);
  memset(run, 0, w*4*sizeof(long));
  oldrun[0] = w;
  memset(output, 0, ln);
  for (j=0; j<h; j++) {
    x = rx = 0;  rl = 0;  ox = oldrun[0];  orx = 1;
    while (x<w) {
      unccitt_prep_bits(input, &s, &sb, &cur, srclen, bitorder);
      for (i=0; i<ccittcode[i+2]!=-2; i+=3)
        if (ccittcode[i]==(cur&mask[ccittcode[i+1]]))
          break;
      if (ccittcode[i+2]==-2) { sb++;  continue; }
      sb += ccittcode[i+1];
      switch (ccittcode[i+2]) {
        case 0: if (rx)  while (ox<=x && ox<w) {    /* Skip a previous run */
            ox += oldrun[orx]+oldrun[orx+1];  orx += 2; }
          ox += oldrun[orx];  orx++;
          rl += ox-x;  x = ox;
          ox += oldrun[orx];  orx++;
          break;
        case 1: if (!(rx&1)) {                            /* Read two runs */
            while (1) {
              unccitt_prep_bits(input, &s, &sb, &cur, srclen, bitorder);
              for (i=0; ccittwhite[i+2]!=-2; i+=3)
                if (ccittwhite[i]==(cur&mask[ccittwhite[i+1]]))
                  break;
              if (ccittwhite[i+2]<0) { sb++;  continue; }
              sb += ccittwhite[i+1];
              rl += ccittwhite[i+2];  x += ccittwhite[i+2];
              if (ccittwhite[i+2]<64) break; }
            run[rx] = rl;  rx++;  rl = 0;
            while (1) {
              unccitt_prep_bits(input, &s, &sb, &cur, srclen, bitorder);
              for (i=0; ccittblack[i+2]!=-2; i+=3)
                if (ccittblack[i]==(cur&mask[ccittblack[i+1]]))
                  break;
              if (ccittblack[i+2]<0) { sb++;  continue; }
              sb += ccittblack[i+1];
              rl += ccittblack[i+2];  x += ccittblack[i+2];
              if (ccittblack[i+2]<64) break; }
            run[rx] = rl;  rx++;  rl = 0; }
          else {
            while (1) {
              unccitt_prep_bits(input, &s, &sb, &cur, srclen, bitorder);
              for (i=0; ccittblack[i+2]!=-2; i+=3)
                if (ccittblack[i]==(cur&mask[ccittblack[i+1]]))
                  break;
              if (ccittblack[i+2]<0) { sb++;  continue; }
              sb += ccittblack[i+1];
              rl += ccittblack[i+2];  x += ccittblack[i+2];
              if (ccittblack[i+2]<64) break; }
            run[rx] = rl;  rx++;  rl = 0;
            while (1) {
              unccitt_prep_bits(input, &s, &sb, &cur, srclen, bitorder);
              for (i=0; ccittwhite[i+2]!=-2; i+=3)
                if (ccittwhite[i]==(cur&mask[ccittwhite[i+1]]))
                  break;
              if (ccittwhite[i+2]<0) { sb++;  continue; }
              sb += ccittwhite[i+1];
              rl += ccittwhite[i+2];  x += ccittwhite[i+2];
              if (ccittwhite[i+2]<64) break; }
            run[rx] = rl;  rx++;  rl = 0; }
          if (rx)  while (ox<=x && ox<w) {
            ox += oldrun[orx]+oldrun[orx+1];  orx += 2; } break;
        case 2: if (rx)  while (ox<=x && ox<w) {  /* Repeat a previous run */
            ox += oldrun[orx]+oldrun[orx+1];  orx += 2; }
          run[rx] = ox-x+rl;  rx++;  rl = 0;  x = ox;
          ox += oldrun[orx];  orx++;
          break;
        case 11: case 12: case 13:     /* Start a run further to the right */
          if (rx)  while (ox<=x && ox<w) {
            ox += oldrun[orx]+oldrun[orx+1];  orx += 2; }
          run[rx] = ox-x+ccittcode[i+2]-10+rl;  rx++;  rl = 0;
          x = ox+ccittcode[i+2]-10;  ox += oldrun[orx];  orx++;
          break;
        case 21: case 22: case 23:      /* Start a run further to the left */
          if (rx)  while (ox<=x && ox<w) {
            ox += oldrun[orx]+oldrun[orx+1];  orx += 2; }
          run[rx] = ox-x-ccittcode[i+2]+20+rl;  rx++;  rl = 0;
          x = ox-ccittcode[i+2]+20;  orx--;  ox -= oldrun[orx];
          break;
        case 3: run[rx] = w-x+rl;  rx++;  rl = 0;             /* extension */
          break;
        case -1: run[rx] = w-x+rl;  rx++;  rl = 0;          /* end of line */
          break;
        default: ; } }
    if (rl) {
      sb++;
      run[rx] = rl;  rx++;  rl = 0; }
    d = j*((w+7)>>3);  db = 0;
    for (i=0; i<rx; i++) {
      db += run[i];
      if (db>=8) { d += (db>>3);  db &= 7; }
      i++;
      if (d>=ln || i>=rx) break;
      for (k=run[i]; k>0; k--) {
        if (!db && k>=8) {
          output[d] = 0xFF;  d++;  if (d>=ln)  break;
          k -= 7;  continue; }
        output[d] |= imask[db];
        db++;  if (db==8) { d++;  db = 0; if (d>=ln)  break; } }
      if (d>=ln) break; }
    if (d>=ln) break;
    run[rx] = run[rx+1] = run[rx+2] = run[rx+3] = 0;
    memcpy(oldrun, run, (rx+4)*sizeof(long)); }
  free2(run);
  if (sb)  s++;
  if (bitorder)  reverse_bitorder(output, ln);
  return(s);
}

#ifdef slowdct
STATIC void undct_1d(long *src, long *dest)
/* Perform a one dimensional inverse discrete-cosine transformation on a set
 *  of 8 values.  The values returned from this routine have been scaled up
 *  by 1024 (i.e., they must be divided by 1024 to get the correct values).
 * Enter: long *src: source array of 8 values.
 *        long *dest: destination array for 8 values.  This may be the same
 *                    as the source array.                      3/5/96-DWM */
{
  #define UNDCT1 502
  #define UNDCT2 473
  #define UNDCT3 426
  #define UNDCT4 362
  #define UNDCT5 284
  #define UNDCT6 196
  #define UNDCT7 100

  long a04, a11, a13, a15, a17, a22, a26, a31, a33, a35, a37, a44, a51, a53;
  long a55, a57, a62, a66, a71, a73, a75, a77, b0, b1, b2, b3, b4, b5, b6;
  long b7, b8, b9, b10, b11;

  if (!src[1] && !src[2] && !src[3] && !src[4] && !src[5] && !src[6] &&
      !src[7]) {
    if (!src[0])  return(0);
    dest[0] = src[0]*UNDCT4;
    for (b4=1; b4<8; b4++)  dest[b4] = dest[0];
    return(0); }
  a04 = src[0]*UNDCT4;  a11 = src[1]*UNDCT1;  a13 = src[1]*UNDCT3;
  a15 = src[1]*UNDCT5;  a17 = src[1]*UNDCT7;  a22 = src[2]*UNDCT2;
  a26 = src[2]*UNDCT6;  a31 = src[3]*UNDCT1;  a33 = src[3]*UNDCT3;
  a35 = src[3]*UNDCT5;  a37 = src[3]*UNDCT7;  a44 = src[4]*UNDCT4;
  a51 = src[5]*UNDCT1;  a53 = src[5]*UNDCT3;  a55 = src[5]*UNDCT5;
  a57 = src[5]*UNDCT7;  a62 = src[6]*UNDCT2;  a66 = src[6]*UNDCT6;
  a71 = src[7]*UNDCT1;  a73 = src[7]*UNDCT3;  a75 = src[7]*UNDCT5;
  a77 = src[7]*UNDCT7;
  b0 = a04+a44;  b1 = a04-a44;  b2 = a22+a66;  b3 = a26-a62;  b4 = b0+b2;
  b5 = b1+b3;  b6 = b1-b3;  b7 = b0-b2;  b8 = a11+a33+a55+a77;
  b9 = a13-a37-a51-a75;  b10 = a15-a31+a57+a73;  b11 = a17-a35+a53-a71;
  dest[0] = b4+b8;  dest[1] = b5+b9;  dest[2] = b6+b10;  dest[3] = b7+b11;
  dest[4] = b7-b11;  dest[5] = b6-b10;  dest[6] = b5-b9;  dest[7] = b4-b8;
}

STATIC void undct_1d_alt(long *src, long *dest)
/* Just like undct_1d, except values are only stored at every eighth point in
 *  the array.  Note that there is no early out on this routine, since this
 *  routine is operated second, and therefore is much less likely to have all
 *  zero parameters.
 * Enter: long *src: source array of 57 fixed values.  Only every 8th value
 *                   is used.
 *        long *dest: destination array for 57 fixed values.  This may be the
 *                    same as the source array.                 3/3/96-DWM */
{
  long a04, a11, a13, a15, a17, a22, a26, a31, a33, a35, a37, a44, a51, a53;
  long a55, a57, a62, a66, a71, a73, a75, a77, b0, b1, b2, b3, b4, b5, b6;
  long b7, b8, b9, b10, b11;

  a04 = src[0]*UNDCT4;  a11 = src[8]*UNDCT1;  a13 = src[8]*UNDCT3;
  a15 = src[8]*UNDCT5;  a17 = src[8]*UNDCT7;  a22 = src[16]*UNDCT2;
  a26 = src[16]*UNDCT6;  a31 = src[24]*UNDCT1;  a33 = src[24]*UNDCT3;
  a35 = src[24]*UNDCT5;  a37 = src[24]*UNDCT7;  a44 = src[32]*UNDCT4;
  a51 = src[40]*UNDCT1;  a53 = src[40]*UNDCT3;  a55 = src[40]*UNDCT5;
  a57 = src[40]*UNDCT7;  a62 = src[48]*UNDCT2;  a66 = src[48]*UNDCT6;
  a71 = src[56]*UNDCT1;  a73 = src[56]*UNDCT3;  a75 = src[56]*UNDCT5;
  a77 = src[56]*UNDCT7;
  b0 = a04+a44;  b1 = a04-a44;  b2 = a22+a66;  b3 = a26-a62;  b4 = b0+b2;
  b5 = b1+b3;  b6 = b1-b3;  b7 = b0-b2;  b8 = a11+a33+a55+a77;
  b9 = a13-a37-a51-a75;  b10 = a15-a31+a57+a73;  b11 = a17-a35+a53-a71;
  dest[0] = b4+b8;  dest[8] = b5+b9;  dest[16] = b6+b10;  dest[24] = b7+b11;
  dest[32] = b7-b11;  dest[40] = b6-b10;  dest[48] = b5-b9;  dest[56] = b4-b8;
}
#endif

STATIC void undct_1d(long *src, long *dest)
/* Perform a one dimensional inverse discrete-cosine transformation on a set
 *  of 8 values.  The values returned from this routine have been scaled up
 *  by 512*sqrt(2) (i.e., they must be divided by 512*sqrt(2) to get the
 *  correct values).
 * Enter: long *src: source array of 8 values.
 *        long *dest: destination array for 8 values.  This may be the same
 *                    as the source array.                      3/5/96-DWM */
{
  long t0, t1, t2, t3, t10, t11, t12, t13, z1, z2, z3, z4, z5;

  if (!src[1] && !src[2] && !src[3] && !src[4] && !src[5] && !src[6] &&
      !src[7]) {
    if (!src[0])  return;
    dest[0] = src[0]<<8;
    for (t0=1; t0<8; t0++)  dest[t0] = dest[0];
    return; }
  z1 = (src[2]+src[6])*UNDCT1;
  t2 = z1-src[6]*UNDCT2;  t3 = z1+src[2]*UNDCT3;
  t0 = (src[0]+src[4])<<8;  t1 = (src[0]-src[4])<<8;
  t10 = t0+t3;  t13 = t0-t3;  t11 = t1+t2;  t12 = t1-t2;
  z1 = src[7]+src[1];  z2 = src[5]+src[3];  z3 = src[7]+src[3];
  z4 = src[5]+src[1];  z5 = (z3+z4)*UNDCT4;  t0 = src[7]*UNDCT5;
  t1 = src[5]*UNDCT6;  t2 = src[3]*UNDCT7;  t3 = src[1]*UNDCT8;
  z1 *= UNDCT9;  z2 *= UNDCT10;  z3 = z3*UNDCT11+z5;  z4 = z4*UNDCT12+z5;
  t0 += z1+z3;  t1 += z2+z4;  t2 += z2+z3;  t3 += z1+z4;
  dest[0] = t10+t3;  dest[7] = t10-t3;  dest[1] = t11+t2;  dest[6] = t11-t2;
  dest[2] = t12+t1;  dest[5] = t12-t1;  dest[3] = t13+t0;  dest[4] = t13-t0;
}

STATIC void undct_1d_alt(long *src, long *dest)
/* Just like undct_1d, except values are only stored at every eighth point in
 *  the array.  Note that there is no early out on this routine, since this
 *  routine is operated second, and therefore is much less likely to have all
 *  zero parameters.
 * Enter: long *src: source array of 57 fixed values.  Only every 8th value
 *                   is used.
 *        long *dest: destination array for 57 fixed values.  This may be the
 *                    same as the source array.                 3/3/96-DWM */
{
  long t0, t1, t2, t3, t10, t11, t12, t13, z1, z2, z3, z4, z5;

  z1 = (src[16]+src[48])*UNDCT1;
  t2 = z1-src[48]*UNDCT2;  t3 = z1+src[16]*UNDCT3;
  t0 = (src[0]+src[32])<<8;  t1 = (src[0]-src[32])<<8;
  t10 = t0+t3;  t13 = t0-t3;  t11 = t1+t2;  t12 = t1-t2;
  z1 = src[56]+src[8];  z2 = src[40]+src[24];  z3 = src[56]+src[24];
  z4 = src[40]+src[8];  z5 = (z3+z4)*UNDCT4;  t0 = src[56]*UNDCT5;
  t1 = src[40]*UNDCT6;  t2 = src[24]*UNDCT7;  t3 = src[8]*UNDCT8;
  z1 *= UNDCT9;  z2 *= UNDCT10;  z3 = z3*UNDCT11+z5;  z4 = z4*UNDCT12+z5;
  t0 += z1+z3;  t1 += z2+z4;  t2 += z2+z3;  t3 += z1+z4;
  dest[0]  = t10+t3;  dest[56] = t10-t3;
  dest[8]  = t11+t2;  dest[48] = t11-t2;
  dest[16] = t12+t1;  dest[40] = t12-t1;
  dest[24] = t13+t0;  dest[32] = t13-t0;
}

STATIC void ungif(uchar *dest, uchar *source, long srclen)
/* Decode any GIF file.  The file must be completely stored in memory, and
 *  there must be sufficient space to store the uncompressed image, including
 *  space for a color map (768 bytes).  The original data is modified.  Note
 *  that interlaced images will still be interlaced.  Three global values are
 *  set: pwidth, pheight, and pinter.  Bit 0 of pinter is 0 if the image is
 *  not interlaced, and 1 if it is.  Bit 1 of pinter is 1 if a transparent
 *  color is specified.  Bit 2 of pinter is 0 if the image is reset between
 *  frames, 1 if it persists.  Bits 8-15 contain the transparent color.  Bits
 *  16-31 contain the delay time in 1/100ths of a second.
 * Enter: long dest: pointer to absolute memory location for final image.
 *        long source: pointer to source informtaion.
 *        long srclen: length of source data.                   5/1/94-DWM */
{
  uchar bitspixel, background;
  ushort totalx, totaly, xstart, ystart, xlength, ylength, xxend, yend,
         bitmask, part;
  long palsub;

  _asm {
          mov eax, ppart
          mov part, ax
          xor eax, eax                                     ;read first header
          xor ecx, ecx
          mov pinter, eax
          mov pwidth, 0x0A
          mov pheight, 0x0A
          mov palsub, eax
          mov esi, source
          add srclen, esi
          mov eax, [esi]
          cmp eax, 0x38464947
          jne     ungif18
          mov ax, [esi+6]
          mov totalx, ax
          mov ax, [esi+8]
          mov totaly, ax
          mov al, [esi+0x0A]
          and al, 0x07
          inc al
          mov bitspixel, al
          mov al, [esi+0x0B]
          mov background,al
          cmp BYTE PTR [esi+0x0C], 00      ;pixel aspect ratio
//          jne     ungif18
          mov cl, bitspixel                                    ;read colormap
          mov ax, 0x01
          shl ax, cl
          mov cx, ax
          dec ax
          mov bitmask, ax
          mov bl, [esi+0x0A]
          add esi, 0x0D
          mov edi, dest
          add dest, 0x0300
          test bl, 0x80
          je      ungif2
          mov WORD PTR palsub, 0x0300
ungif1:   mov eax, [esi]
          shr eax, 0x02
          and eax, 0x3F3F3F
          mov [edi], eax
          add edi, 0x03
          add esi, 0x03
          loop    ungif1
ungif2:   cmp BYTE PTR [esi], 0x2C                      ;check for extensions
          je      ungif6
          cmp BYTE PTR [esi], 0x21                       ;record transparency
          jne     ungif5
          cmp part, 0x00
          jg      ungif3
          cmp BYTE PTR [esi+1], 0xF9
          jne     ungifb
          mov ax, [esi+4]
          shl eax, 0x10
          mov pinter, eax
          xor eax, eax
          mov al, [esi+3]
          and al, 0x04
          or  pinter, eax
          xor eax, eax
          test BYTE PTR [esi+3], 0x01
          je      ungif3
          mov ah,  [esi+6]
          mov al, 0x02
          mov WORD PTR pinter, ax
          jmp     ungif3
ungifb:   cmp BYTE PTR [esi+1], 0xFF
          jne     ungif3
          cmp DWORD PTR [esi+2], 0x54454E0B
          jne     ungif3
          mov ax, [esi+0x10]
          mov WORD PTR [pspec+0x10], ax
ungif3:   add esi, 0x02
          cmp esi, srclen
          jae     ungif18
ungif4:   xor ebx, ebx
          mov bl, BYTE PTR [esi]
          inc bx
          add esi, ebx
          cmp esi, srclen
          jae     ungif18
          cmp bl, 0x01
          jne     ungif4
          jmp     ungif2
ungif5:   inc esi
          cmp esi, srclen
          jae     ungif18
          jmp     ungif2
ungif6:   cmp part, 0x00                                     ;ignore an image
          jle     ungif7
          dec part
          test BYTE PTR [esi+9], 0x80
          je      ungifa
          mov cl, [esi+9]
          and cl, 0x07
          inc cl
          mov eax, 0x01
          shl eax, cl
          add esi, eax
          add esi, eax
          add esi, eax
ungifa:   add esi, 0x0B
          cmp esi, srclen
          jae     ungif18
          jmp     ungif4
ungif7:   mov ax, [esi+1]                                       ;read header2
          mov xstart, ax
          mov bx, [esi+3]
          mov ystart, bx
          mov cx, [esi+5]
          mov xlength, cx
          mov WORD PTR pwidth, cx
          mov dx, [esi+7]
          mov ylength, dx
          mov WORD PTR pheight, dx
          add ax, cx
          dec ax
          mov xxend, ax
          add bx, dx
          dec bx
          mov yend, bx
          test BYTE PTR [esi+9], 0x40
          je      ungif8
          inc pinter
ungif8:   test BYTE PTR [esi+9], 0x80                    ;read image colormap
          je      ungif10
          mov cl, BYTE PTR [esi+9]
          and cl, 0x07
          inc cl
          mov ax, 0x01
          shl ax, cl
          mov cx, ax
          mov edi, dest
          sub edi, 0x0300
ungif9:   mov eax, [esi+0x0A]
          shr eax, 0x02
          and eax, 0x3F3F3F
          mov [edi], eax
          add edi, 0x03
          add esi, 0x03
          cmp esi, srclen
          jae     ungif18
          loop    ungif9
ungif10:  mov al, [esi+0x0A]       ;determine starting number of bits / pixel
          mov bitspixel, al
          add esi, 0x0B                          ;elminate block length bytes
          cmp esi, srclen
          jae     ungif17
          mov source, esi
          mov edi, esi
          xor ch, ch
ungif11:  mov cl, [esi]
          cmp cl, 0x00
          je      ungif13
          inc esi
          cmp esi, srclen
          jae     ungif17
ungif12:  mov al, [esi]
          mov [edi], al
          inc esi
          cmp esi, srclen
          jae     ungif17
          inc edi
          loop    ungif12
          jmp     ungif11
ungif13:  inc esi                                ;check for additional images
          cmp esi, srclen
          jae     ungif17
ungif14:  cmp BYTE PTR [esi], 0x2C
          je      ungif16
          cmp BYTE PTR [esi], 0x21                       ;record transparency
          jne     ungif17
          add esi, 0x02
          cmp esi, srclen
          jae     ungif17
ungif15:  xor ebx, ebx
          mov bl, BYTE PTR [esi]
          inc bx
          add esi, ebx
          cmp esi, srclen
          jae     ungif17
          cmp bl, 0x01
          jne     ungif15
          jmp     ungif14
ungif16:  or  [pspec+1], 0x80
ungif17:  xor eax, eax                                      ;decompress image
          mov al, bitspixel
          push eax
          sub edi, source
          push edi
          push MAXGSIZE
          push source
          push dest
          call    unlzw
          add esp, 0x14
ungif18:
    }
}

STATIC void uninterlace(char *addr, long rev)
/* Uninterlace the specified gif.  The image is of size pwidth by pheight.
 *  Interlacing is by the standard GIF method.
 * Enter: char *addr: pointer to gif record, excluding palette.
 *        long rev: 0 to deinterlace, 1 to interlace.          12/6/94-DWM */
{
  char *buf;
  short *cur, i, j=0;

  if (!(buf=malloc2(pwidth+pheight*2))) { perr = 0x1606;  return; }
  cur = (short *)(buf+pwidth);
  for (i=0; i<pheight; i+=8, j++)  if (!rev) cur[j] = i;  else cur[i] = j;
  for (i=4; i<pheight; i+=8, j++)  if (!rev) cur[j] = i;  else cur[i] = j;
  for (i=2; i<pheight; i+=4, j++)  if (!rev) cur[j] = i;  else cur[i] = j;
  for (i=1; i<pheight; i+=2, j++)  if (!rev) cur[j] = i;  else cur[i] = j;
  for (i=0; i<pheight; i++)
    while (cur[i]!=i) {
      memmove(buf, addr+(long)pwidth*i, pwidth);
      memmove(addr+(long)pwidth*i, addr+(long)pwidth*cur[i], pwidth);
      memmove(addr+(long)pwidth*cur[i], buf, pwidth);
      j = cur[cur[i]];
      cur[cur[i]] = cur[i];
      cur[i] = j; }
  free2(buf);
}

STATIC void unlzw(uchar *dest, uchar *source, long maxlen, long srclen,
             short numbits)
/* LZW decode data until the specified number of bytes have been created or
 *  an eof code is encountered.  This routine only works on LZW data which
 *  was created from 8 bit samples and has a maximum code length of 12 bits.
 *  Even through the data must have been created from 8 bit samples, it may
 *  have had fewer significant bits, hence the numbits parameter.
 * Enter: long dest: pointer to absolute memory location for unlzwed data.
 *        long source: pointer to source informtaion.
 *        long maxlen: maximum size of destination.
 *        long srclen: number of bytes in source.
 *        int numbits: bit size of data to decompact.         12/10/94-DWM */
{
  uchar codesize, initcodesize, pos;
  ushort clearcode, eof, firstfree, freecode, maxcode, bitmask, codemask,
         initcodemask, initmaxcode, oldcode, incode, finchar;

  _asm {
          xor ebx, ebx
          mov eax, dest
          add maxlen, eax
          mov al, BYTE PTR numbits
          mov cl, al
          mov dx, 0x01
          shl dx, cl
          mov clearcode, dx
          mov cx, dx
          mov bitmask, dx
          dec bitmask
          inc dx
          mov eof, dx
          inc dx
          mov firstfree, dx
          mov freecode, dx
          inc al
          mov codesize, al
          mov initcodesize, al
          shl cx, 0x01
          mov maxcode, cx
          mov initmaxcode, cx
          mov edx, dest                                     ;begin LZW decode
          mov esi, source
          add srclen, esi
          mov pos, 0x00
          mov ax, bitmask
          shl ax, 0x01
          inc ax
          mov codemask, ax
          mov initcodemask, ax
          xor edi, edi
unlzw1:   mov eax, [esi]                                      ;read next code
          mov cl, pos
          shr eax, cl
          add cl, codesize
          cmp cl, 0x08
          jl      unlzw2
          sub cl, 0x08
          inc esi
          cmp esi, srclen
          jge     unlzw7
          cmp cl, 0x08
          jl      unlzw2
          sub cl, 0x08
          inc esi
          cmp esi, srclen
          jge     unlzw7
unlzw2:   mov pos, cl
          and ax, codemask
          cmp ax, eof
          je      unlzw7
          cmp ax, clearcode
          jne     unlzw3
          mov al, initcodesize                                   ;clear codes
          mov codesize, al
          mov ax, initcodemask
          mov codemask, ax
          mov ax, initmaxcode
          mov maxcode, ax
          mov ax, firstfree
          mov freecode, ax
          mov eax, [esi]
          mov cl, pos
          shr eax, cl
          add cl, codesize
          mov pos, cl
          and ax, codemask
          mov oldcode, ax
          and ax, bitmask
          mov finchar, ax
          mov [edx], al
          inc edx
          cmp edx, maxlen
          jb      unlzw1
          jmp     unlzw7
unlzw3:   mov incode, ax                                        ;process code
          cmp ax, freecode
          jl      unlzw4
          mov ax, oldcode
          mov bx, finchar
          mov [outcode+edi], bl
          inc edi
unlzw4:   cmp ax, bitmask
          jle     unlzw5
          mov bx, ax
          mov cl, [suffix+ebx]
          mov [outcode+edi], cl
          inc edi
          shl bx, 0x01
          mov ax, [prefix+ebx]
          jmp     unlzw4
unlzw5:   mov finchar, ax
          mov [outcode+edi], al
          inc edi
unlzw6:   mov al, [outcode+edi-1]
          mov [edx], al
          inc edx
          cmp edx, maxlen
          je      unlzw7
          dec edi
          jne     unlzw6
          mov bx, freecode
          mov ax, finchar
          mov [suffix+ebx], al
          shl bx, 0x01
          mov cx, oldcode
          mov [prefix+ebx], cx
          mov bx, incode
          mov oldcode, bx
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jl      unlzw1
          cmp codesize, 0x0C
          jge     unlzw1
          inc codesize
          shl codemask, 0x01
          inc codemask
          shl maxcode, 0x01
          jmp     unlzw1
unlzw7:
    }
}

STATIC void unlzwtif(uchar *dest, uchar *source, long maxlen, long srclen)
/* TIFF LZW decode data until the specified number of bytes have been created
 *  or an eof code is encountered.  This routine only works on LZW data which
 *  was created from 8 bit samples and has a maximum code length of 12 bits.
 *  This routine only works on TIFF style LZW data.
 * Enter: long dest: pointer to absolute memory location for unlzwed data.
 *        long source: pointer to source informtaion.
 *        long maxlen: maximum size of destination.
 *        long srclen: length of input data.                  12/10/94-DWM */
{
  uchar codesize, initcodesize, pos;
  ushort clearcode, eof, firstfree, freecode, maxcode, bitmask, codemask,
         initcodemask, initmaxcode, oldcode, incode, finchar;

  _asm {
          xor ebx, ebx
          mov eax, dest
          add maxlen, eax
          mov al, 0x8
          mov dx, 0x0100
          mov clearcode, dx
          mov cx, dx
          mov bitmask, dx
          dec bitmask
          inc dx
          mov eof, dx
          inc dx
          mov firstfree, dx
          mov freecode, dx
          inc al
          mov codesize, al
          mov initcodesize, al
          shl cx, 0x01
          dec cx                               ;Remove line for GIF style LZW
          mov maxcode, cx
          mov initmaxcode, cx
          mov edx, dest                                     ;begin LZW decode
          mov esi, source
          add srclen, esi
          mov pos, 0x00
          mov ax, bitmask
          shl ax, 0x01
          inc ax
          mov codemask, ax
          mov initcodemask, ax
          xor edi, edi
unlzwt1:  mov eax, [esi]                                      ;read next code
          mov ch, al                 ;These 7 lines are the same as bswap eax
          mov cl, ah
          shl ecx, 0x10
          shr eax, 0x10
          mov ch, al
          mov cl, ah
          mov eax, ecx
          mov cl, 0x20
          sub cl, pos
          sub cl, codesize
          shr eax, cl            ;Remove from bswap to here for GIF style LZW
          mov cl, pos      ;Add shr eax, cl after this line for GIF style LZW
          add cl, codesize
          cmp cl, 0x08
          jl      unlzwt2
          sub cl, 0x08
          inc esi
          cmp esi, srclen
          jge     unlzwt7
          cmp cl, 0x08
          jl      unlzwt2
          sub cl, 0x08
          inc esi
          cmp esi, srclen
          jge     unlzwt7
unlzwt2:  mov pos, cl
          and ax, codemask
          cmp ax, eof
          je      unlzwt7
          cmp ax, clearcode
          jne     unlzwt3
          mov al, initcodesize                                   ;clear codes
          mov codesize, al
          mov ax, initcodemask
          mov codemask, ax
          mov ax, initmaxcode
          mov maxcode, ax
          mov ax, firstfree
          mov freecode, ax
          mov eax, [esi]
          mov ch, al                 ;These 7 lines are the same as bswap eax
          mov cl, ah
          shl ecx, 0x10
          shr eax, 0x10
          mov ch, al
          mov cl, ah
          mov eax, ecx
          mov cl, 0x20
          sub cl, pos
          sub cl, codesize
          shr eax, cl            ;Remove from bswap to here for GIF style LZW
          mov cl, pos      ;Add shr eax, cl after this line for GIF style LZW
          add cl, codesize
          mov pos, cl
          and ax, codemask
          mov oldcode, ax
          and ax, bitmask
          mov finchar, ax
          mov [edx], al
          inc edx
          cmp edx, maxlen
          jl      unlzwt1
          jmp     unlzwt7
unlzwt3:  mov incode, ax                                        ;process code
          cmp ax, freecode
          jl      unlzwt4
          mov ax, oldcode
          mov bx, finchar
          mov [outcode+edi], bl
          inc edi
unlzwt4:  cmp ax, bitmask
          jle     unlzwt5
          mov bx, ax
          mov cl, [suffix+ebx]
          mov [outcode+edi], cl
          inc edi
          shl bx, 0x01
          mov ax, [prefix+ebx]
          jmp     unlzwt4
unlzwt5:  mov finchar, ax
          mov [outcode+edi], al
          inc edi
unlzwt6:  mov al, [outcode+edi-1]
          mov [edx], al
          inc edx
          cmp edx, maxlen
          je      unlzwt7
          dec edi
          jne     unlzwt6
          mov bx, freecode
          mov ax, finchar
          mov [suffix+ebx], al
          shl bx, 0x01
          mov cx, oldcode
          mov [prefix+ebx], cx
          mov bx, incode
          mov oldcode, bx
          inc freecode
          mov bx, freecode
          cmp bx, maxcode
          jl      unlzwt1
          cmp codesize, 0x0C
          jge     unlzwt1
          inc codesize
          shl codemask, 0x01
          inc codemask
          shl maxcode, 0x01
          inc maxcode                          ;Remove line for GIF style LZW
          jmp     unlzwt1
unlzwt7:
    }
}

STATIC long unrle(uchar *output, uchar *input, long ln, long srclen)
/* Uncompress data using a run-length encoding scheme.  The size of the
 *  output is unpredicatable.  Decompression ends when len characters have
 *  been converted or an end of RLE code (0x80) is reached.
 * Enter: char *output: pointer to expanded data.
 *        char *input: pointer to data to decompress.
 *        long len: maximum length of output.  0 for use end of RLE code.
 *        long srclen: maximum length of input.
 * Exit:  long len: length of rle code used.                  11/30/92-DWM */
{
  long finlen;

  _asm {
          mov edi, input
          mov esi, output
          mov ecx, esi
          add ecx, ln
          cmp ecx, esi
          jne     unrle1
          mov ecx, 0xFFFFFFFF
unrle1:   mov al, [edi]
          inc edi
          dec srclen
          jl      unrle6
          cmp al, 0x80
          ja      unrle3
          je      unrle6
          inc al
unrle2:   mov ah, [edi]
          inc edi
          dec srclen
          jl      unrle6
          mov [esi], ah
          inc esi
          cmp esi, ecx
          jae     unrle6
          dec al
          jne     unrle2
          jmp     unrle5
unrle3:   mov ah, [edi]
          inc edi
          dec srclen
          jl      unrle6
          dec al
unrle4:   mov [esi], ah
          inc esi
          cmp esi, ecx
          jae     unrle6
          inc al
          jne     unrle4
unrle5:   cmp esi, ecx
          jb      unrle1
unrle6:   mov eax, edi
          sub eax, input
          mov finlen, eax
    }
  return(finlen);
}

STATIC void vertical_flip(uchar *image, long w, long h)
/* Flip an image vertically.
 * Enter: uchar *image: pointer to the image array to flip.
 *        long w, h: size of the image.  Width is the size of the scan line,
 *                   not the number of pixels.                 2/26/96-DWM */
{
  char buf[16384];
  long i;

  if (w>16384)  return;
  for (i=0; i<h/2; i++) {
    memmove(buf, image+i*w, w);
    memmove(image+i*w, image+(h-1-i)*w, w);
    memmove(image+(h-1-i)*w, buf, w); }
}
