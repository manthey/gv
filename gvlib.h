/* Header file for Windows interface routines for graphics load and save   */
/*  routines.  See other documentation for details.                        */
/*                                    Copyright 1996-2017 by David Manthey */

#ifndef GVLIBHEADER                            /* Prevent double inclusion */
#define GVLIBHEADER 1

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

#pragma pack(4)

#define HBITMAPINFO HANDLE
#define uchar unsigned char


struct BMP {
  long color, compression, dpi, bmptype; };

#define BMP_BEST        0
#define BMP_BW          1
#define BMP_4BIT        2
#define BMP_PAL         3
#define BMP_24BIT       4

#define BMP_NONE        0
#define BMP_RLE         1

#define BMP_TYPE40      0
#define BMP_TYPE12      1

struct CUR {
  long image, moreimages;
  long color, dpi, cursize, transparent, inverse, hotx, hoty; };

#define CUR_BEST        0
#define CUR_BW          1
#define CUR_4BIT        2
#define CUR_PAL         3
#define CUR_24BIT       4

#define CUR_NORMAL      0
#define CUR_16X16       1
#define CUR_32X32       2

struct GIF {
  long image, moreimages;
  long giftype, interlace, usetrans, transparent, delay, repeat, keep; };

#define GIF_TYPE87      0
#define GIF_TYPE89      1

struct GRO {
  long grobtype, grobsize, numshades;
  float gamma; };

#define GRO_ASCII       0
#define GRO_BINARY      1

#define GRO_NORMAL      0
#define GRO_131x64      1

struct ICO {
  long image, moreimages;
  long color, dpi, icosize, transparent, inverse; };

#define ICO_BEST        0
#define ICO_BW          1
#define ICO_4BIT        2
#define ICO_PAL         3
#define ICO_24BIT       4

#define ICO_NORMAL      0
#define ICO_16X16       1
#define ICO_32X32       2

struct JPG {
  long image, moreimages;
  long color, compression, dpi, decimation, quality, usecustom;
  char *customquant;
  long spectralsteps, succesivesteps; };

#define JPG_BEST        0
#define JPG_GREYSCALE   1
#define JPG_24BIT       2

#define JPG_BASELINE    0
#define JPG_EXTENDED    1
#define JPG_PROGRESSIVE 2
#define JPG_COMPLETE    0x00
#define JPG_ONLYDATA    0x40
#define JPG_ONLYHEADER  0x80

#define JPG_1X1TO1X1    0
#define JPG_2X1TO1X1    1
#define JPG_2X2TO1X1    2
#define JPG_4X2TO1X1    3

struct PPM {
  long color, ppmtype; };

#define PPM_BEST        0
#define PPM_BW          1
#define PPM_GREYSCALE   2
#define PPM_24BIT       3

#define PPM_ASCII       0
#define PPM_BINARY      1

struct PCX {
  long color; };

#define PCX_BEST        0
#define PCX_BW          1
#define PCX_8BIT        2
#define PCX_24BIT       3

struct TGA {
  long reserved; };

#define TIF_MAXMODELLEN      50
#define TIF_MAXSOFTWARELEN  100
#define TIF_MAXSERIALLEN     50

struct TIF {
  long image, moreimages;
  long color, compression, dpi, endian, rowsperstrip;
  char model[TIF_MAXMODELLEN], serial[TIF_MAXSERIALLEN];
  char software[TIF_MAXSOFTWARELEN]; };

#define TIF_BEST        0
#define TIF_BW          1
#define TIF_GREYSCALE   2
#define TIF_8BIT        3
#define TIF_24BIT       4
#define TIF_CYMK        5

#define TIF_NONE        0
#define TIF_RLE         1
#define TIF_LZW         2
#define TIF_LZWPRED     3
#define TIF_GV          4
#define TIF_GVPRED      5

#define TIF_MOTOROLA    0
#define TIF_INTEL       1


typedef struct GV {
  char *name;
  long format, image, dither, width, height, palette, offset;
  union formatspec {
    struct BMP bmp;
    struct CUR cur;
    struct GIF gif;
    struct GRO gro;
    struct ICO ico;
    struct JPG jpg;
    struct PPM ppm;
    struct PCX pcx;
    struct TGA tga;
    struct TIF tif; } fs;
  FILE *(*fopenb)(char *name, char *mode);
  size_t (*freadb)(void *buffer, size_t size, size_t count, FILE *fptr);
  size_t (*fwriteb)(void *buffer, size_t size, size_t count, FILE *fptr);
  int (*fcloseb)(FILE *fptr);
  int (*fseekb)(FILE *fptr, long offset, int origin);
  long (*ftellb)(FILE *fptr);
  long error; } GV;

#define GV_BMP          0
#define GV_CUR          1
#define GV_GIF          2
#define GV_GRO          3
#define GV_ICO          4
#define GV_JPG          5
#define GV_PPM          6
#define GV_PCX          7
#define GV_TGA          8
#define GV_TIF          9

#define GV_UNKNOWN                  0x101
#define GV_READERROR                0x102
#define GV_LOWMEMORY                0x103
#define GV_BADBMPTYPE               0x104
#define GV_UNKNOWNCOMPRESSION       0x105
#define GV_INVALIDSIZE              0x106
#define GV_UNSUPPORTEDCOMPRESSION   0x107
#define GV_UNSUPPORTEDBITSPERPIXEL  0x108
#define GV_UNSUPPORTEDCOLOR         0x109
#define GV_BADSPECTRAL              0x10A
#define GV_TOOMANYQUANTIZATION      0x10B
#define GV_BADSCANHEADER            0x10C
#define GV_PALETTETOOBIG            0x10D
#define GV_UNSUPPORTEDORIENTATION   0x10E
#define GV_UNSUPPORTEDCOLORSPACE    0x10F
#define GV_PLANARFORMAT             0x110
#define GV_TOOMANYSTRIPS            0x111
#define GV_NOSUCHIMAGE              0x112

#define GV_UNSUPPORTEDFORMAT        0x201
#define GV_CANTWRITE                0x202
#define GV_SAVELOWMEMORY            0x203
#define GV_CANTAPPEND               0x204
#define GV_APPENDDOESNTMATCH        0x205
#define GV_ENDIANDOESNTMATCH        0x206

#define GV_NODITHER                     0
#define GV_DITHER                       1
#define GV_DITHERIF24BIT                2

#define GV_OPTIMALPALETTE               0
#define GV_UNIFORMPALETTE      (HANDLE)-1

#define GV_REPLICATE                    0
#define GV_BILINEAR                     1
#define GV_BIQUADRATIC                  2
#define GV_BICUBIC                      3
#define GV_BIQUARTIC                    4
#define GV_BIQUINTIC                    5
#define GV_NOSPIFF                      0
#define GV_NEUTRALSPIFF            0x0400
#define GV_DARKSPIFF               0x0800
#define GV_MEDIUMSPIFF             0x0C00
#define GV_BRIGHTSPIFF             0x1000
#define GV_DARKCENTERSPIFF         0x1400
#define GV_MEDIUMCENTERSPIFF       0x1800
#define GV_BRIGHTCENTERSPIFF       0x1C00
#define GV_AREADATA                     0
#define GV_BLOCKDATA               0x2000
#define GV_POINTDATA               0x4000
#define GV_INTERPOLATE24BIT             0
#define GV_INTERPOLATEALL          0x8000

uchar       *bmp_to_graphic    (HBITMAPINFO bmp);
uchar       *depalettize_graphic(long w, long h, long pal, uchar *pic);
void         fill_gv           (GV *gv, long err);
void         fill_spec         (GV *gv);
HBITMAPINFO  graphic_to_bmp    (long w, long h, long pal, uchar *pic,
                                long free);
uchar       *palettize_graphic (long w, long h, long pinter, uchar *pic,
                                long dither, uchar *pal);
uchar       *scale_graphic     (long w, long h, long pal, uchar *pic, long nw,
                                long nh, long color);
void         scale_pic_grey    (uchar *dest, uchar *source, long srcw,
                                long destw, long simgw, long dimgw, long simgh,
                                long dimgh, long bclr);

HBITMAPINFO  LoadBMP           (GV *graphicspec);
long         SaveBMP           (GV *graphicspec, HBITMAPINFO bmp);
HBITMAPINFO  DepalettizeBMP    (HBITMAPINFO bmp);
HBITMAPINFO  PalettizeBMP      (HBITMAPINFO bmp, long dither,
                                LPLOGPALETTE palette);
LPLOGPALETTE BMPPalette        (HBITMAPINFO bmp);
HBITMAPINFO  ScaleBMP          (HBITMAPINFO bmp, long newwidth,
                                long newheight, long letterboxcolor,
                                long interpolation);

HANDLE       LoadGraphic       (GV *graphicspec);
long         SaveGraphic       (GV *graphicspec, HANDLE graphic);
HANDLE       DepalettizeGraphic(long w, long h, long pal, HANDLE hpic);
HANDLE       PalettizeGraphic  (long width, long height, long palettized,
                                HANDLE graphic, long dither, HANDLE palette);
HANDLE       GraphicPalette    (long width, long height, long palettized,
                                HANDLE graphic);
HANDLE       ScaleGraphic      (long width, long height, long palettized,
                                HANDLE hpic, long newwidth, long newheight,
                                long letterboxcolor, long interpolation);

HANDLE       BMPToGraphic      (GV *graphicspec, HBITMAPINFO bmp);
HBITMAPINFO  GraphicToBMP      (long width, long height, long palettized,
                                HANDLE graphic);
HBITMAPINFO  BitmapToBMP       (HBITMAP bitmap, HPALETTE palette, HDC hdc);

#endif                                  /* End of prevent double inclusion */
