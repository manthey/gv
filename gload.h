/* Specification blocks for the different file formats:  (These values are
 *  passed to the save routine and recorded in pspec[] by the load routine).
 *     +---+---+---+---+---+---+---+---+---+---+ Image#:number of image within file.  High bit set if additional images available.
 * Byte|BMP|CUR|GIF|GRO|ICO|JPG|PPM|PCX|TGA|TIF| Color:BMP:0-best,1-b&w,2-4bit,3-pal,4-24bit
 *     +---+---+---+---+---+---+---+---+---+---+       CUR,ICO:0-best,1-b&w,2-4bit, 3-pal, 4-24bit
 *   0 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |       JPG:0-best,1-8bit,2-24bit
 *     +---+---+---+---+---+---+---+---+---+---+       PCX:0-best,1-b&w,2-pal,3-24bit
 *   1 |xxx|Image #|   |Image #|           |Im#|       PPM:0-best,1-b&w,2-8bit,3-24bit
 *     +---+---+---+   +-------+-------+   +---+       TIF:0-best,1-b&w,2-8bit,3-pal,4-24bit,5-cmyk
 *   2 | Color |Trn|   |     Color     |   |Clr| Trn:1 for use transparency
 *     +---+---+---+   +---+---+-------+   +---+ Cmp:BMP:0-none,1-rle
 *   3 |Cmp|     Reserved  |Cmp|           |Cmp|     JPG:0-baseline,1-extended,2-progressive, bits:0x40-only write data,0x80-only write headers
 *     +---+---+       +---+---+           +---+     TIF:0-none,1-rle,2-lzw,3-lzw with prediction
 *   4 |       |       |       | Reserved  |   | DPI:(short)Dots per inch
 *     |  DPI  |       |  DPI  |           |DPI| Typ:0-40,1-12
 *   5 |       |       |       |           |   | Siz:CUR,ICO:0-normal,1-16x16,2-32x32
 *     +---+---+---+---+---+---+---+       +---+     GRO:0-normal,1-131x64
 *   6 |Typ|Siz|Ver|Bin|Siz|Dec|Bin|       |End| Ver:0-87a,1-89a
 *     +---+---+---+---+---+---+---+       +---+ Bin:0-ASCII,1-binary
 *   7 | R     |Int|Siz|xxx|Cus|               | Dec:0-1x1:1x1:1x1,1-2x1,2-2x2,3-4x2
 *     | e +---+---+---+---+---+           +---+ End:0-motorola endian,1-intel endian
 *   8 | s |       |   |   |   |           |   | Int:0-no interlacing,1-interlacing
 *     | e |       |   |   |   |           |   | Trans.Color:(long)transparency color on original image
 *   9 | r | Trans |   |Trn|   |           |Row| Cus:0-standard quantization tables,1-custom quantization table
 *     | v |       |Gam|   |Qal|           |   | Gam:(float)gamma value
 *   A | e | Color |   |Clr|   |           |Stp| Qal:(long)quality factor
 *     | d |       |   |   |   |           |   | Row.Stp:(long)rows per strip
 *   B |   |       |   |   |   |           |   |
 *     |   +---+---+---+---+---+           +---+
 *   C |   |   |   |   |   |   |               | Inv.Clr:(long)inverse color on original image
 *     |   |   |Dly|   |   |   |               | Dly:(short)delay in 0.01 seconds
 *   D |   |Inv|   |Num|Inv|Cus|               | Num.Shd:(long)number of shades of grey
 *     |   |   +---+   |   |   |               | Cus.Qnt:(uchar *)pointer to custom quantization table
 *   E |   |Clr|   |Shd|Clr|Qnt|               |
 *     |   |   |Rst|   |   |   |               | Rst:(short)non-zero to prevent reseting the image
 *   F |   |   |   |   |   |   |               |
 *     |   +---+---+---+---+---+               |
 *  10 |   |Hot|   |       |   |               | HotX:(short)x hot spot
 *     |   |   |Rep|       |Stp|               | Rep:(short)number of times to repeat
 *  11 |   | X |   |       |   |               | Stp:(short)number of progressive steps
 *     |   +---+---+Reserve+---+           +---+
 *  12 |   |Hot|Img|       |Img|           |Img| HotY:(short)y hot spot
 *     |   |   | # |       | # |           | # | Img#Hi:Top 9 bits of image number
 *  13 |   | Y |Hi |       |Hi |           |Hi |
 *     +---+---+---+---+---+---+---+---+---+---+
 * Byte+BMP|CUR|GIF|GRO|ICO|JPG|PPM|PCX|TGA|TIF| xxx:reserved for future expansion - set to zero
 *     +---+---+---+---+---+---+---+---+---+---+                           */

#include "mem.h"

#define uchar  unsigned char
#define uint   unsigned short
#define ushort unsigned short
#define ulong  unsigned long

#define LENPSPEC    20

#define JPG_MAXPROGRESSIVE   25

#define TIF_MAXMODELLEN      50
#define TIF_MAXSOFTWARELEN  100
#define TIF_MAXSERIALLEN     50

#ifndef _WINDOWS_
  #define HANDLE void *
  #define DWORD long
  #define LPVOID void *
  #define WINAPI
#endif

#define UNDCT1   139  /* sqrt(2)cos(3pi/8) */
#define UNDCT2   473  /* sqrt(2)(cos(3pi/8)+sin(3pi/8)) */
#define UNDCT3   196  /* sqrt(2)(sin(3pi/8)-cos(3pi/8)) */
#define UNDCT4   301  /* sqrt(2)cos(3pi/16) */
#define UNDCT5    76  /* sqrt(2)(-cos(pi/16)+cos(3pi/16)+cos(5pi/16)-cos(7pi/16)) */
#define UNDCT6   526  /* sqrt(2)(cos(pi/16)+cos(3pi/16)-cos(5pi/16)+cos(7pi/16)) */
#define UNDCT7   787  /* sqrt(2)(cos(pi/16)+cos(3pi/16)+cos(5pi/16)-cos(7pi/16)) */
#define UNDCT8   384  /* sqrt(2)(cos(pi/16)+cos(3pi/16)-cos(5pi/16)-cos(7pi/16)) */
#define UNDCT9  -230  /* sqrt(2)(cos(7pi/16)-cos(3pi/16)) */
#define UNDCT10 -656  /* sqrt(2)(-cos(pi/16)-cos(3pi/16)) */
#define UNDCT11 -502  /* sqrt(2)(-cos(5pi/16)-cos(3pi/16)) */
#define UNDCT12 -100  /* sqrt(2)(cos(5pi/16)-cos(3pi/16)) */

long   append_curico    (FILE *fptr, char *spec);
long   append_gif       (FILE *fptr, char *spec);
long   append_jpg       (FILE *fptr, char *spec);
long   append_tif       (FILE *fptr, char *spec);
long   best_color       (long allowed, long trans, long inver);
void   bgr_to_rgb       (uchar *image, long size);
void   bmp4rle          (uchar *src, long w, FILE *fptr, uchar *buf);
void   bmp8rle          (uchar *src, long w, FILE *fptr, uchar *buf);
void   dct_1d           (char *src, long *dest);
void   dct_1d_alt       (long *src, long *dest);
short  endian           (short *x);
long   endianl          (long *x);
void   fill_zone24      (uchar *dest, long bclr, long count);
void   find_next        (void);
long   free_left        (long all);
void   hor_flip         (uchar *image, long w, long h, long pix);
void   join_names       (char *text);
void   jpeg_dct         (char *src, long w, long *div, long channel,
                         long *huff);
long   jpeg_decode      (uchar *src, long offset, long len, uchar *dchuff,
                         uchar *dchlen, uchar *achuff, uchar *achlen,
                         uchar *quant, long *dest);
long   jpeg_decode_spectral(uchar *src, long offset, long len, uchar *dchuff,
                         uchar *dchlen, uchar *achuff, uchar *achlen,
                         uchar *quant, long *dest, long spec0, long spec1,
                         long first, long succ);
void   jpeg_shift       (uchar *dest, long *source);
void   jpeg_shift_add   (uchar *dest, long *source);
void   jpeg_undct       (long *res, char *dest, long w, long rw, long rh,
                         long add);
long   jpeg_unstuff     (uchar *jpg, long len, long start);
uchar *load_bmp         (FILE *fptr);
uchar *load_gif         (FILE *fptr);
uchar *load_graphic     (char *name);
uchar *load_groa        (FILE *fptr);
uchar *load_grob        (FILE *fptr);
uchar *load_ico         (FILE *fptr);
uchar *load_jpg         (FILE *fptr);
uchar *load_jpg_progress(uchar *jpg, long fsize, uchar *huff);
uchar *load_pcx         (FILE *fptr);
uchar *load_pil         (FILE *fptr);
uchar *load_ppm         (FILE *fptr);
uchar *load_tga         (FILE *fptr);
uchar *load_tif         (FILE *fptr);
long   lzw              (void *dest, void *source, uchar *buf28k, long len,
                         long maxlen, short numbits);
long   lzwlen           (void *source, uchar *buf28k, long len,
                         short numbits);
long   lzwtif           (void *dest, void *source, uchar *buf28k, long len,
                         long maxlen);
long   lzwtiflen        (void *source, uchar *buf28k, long len);
int    mclose           (FILE *fptr);
FILE  *mopen            (char *name, char *mode);
FILE  *mopen2           (char *name, char *mode);
void   move             (long flags, long x, long y, long clear);
size_t mread            (void *buffer, size_t size, size_t count,
                         FILE *fptr);
int    mseek            (FILE *fptr, long offset, int origin);
long   mtell            (FILE *fptr);
size_t mwrite           (void *buffer, size_t size, size_t count,
                         FILE *fptr);
long   palettize        (uchar *image, long w, long h, long scale);
uchar *palettize_graphic(long w, long h, long pinter, uchar *pic,
                         long dither, uchar *pal);
void   plane_to_rgb     (uchar *image, long w, long h);
float  ppm_float_number (FILE *fptr, float low, float high);
long   ppm_number       (FILE *fptr, long scale);
void   preview_insert   (long x, long y, long sel);
void   preview_move     (long flags, long x, long y, long clear);
long   read_tif         (uchar *grey, long *count, long *offset, long *pic,
                         long x, long y, FILE *fptr);
void   read_tif_dcs     (uchar *grey, long x, long y, long ax);
void   reduce_res       (HWND hwnd, long all);
void   rename_file      (void);
void   reorder_tif      (uchar *dest, uchar *tag, long num, long end,
                         FILE *fptr);
uchar *reorient         (uchar *pic, int orient);
void   reverse_bitorder (uchar *buf, long ln);
long   rle              (uchar *output, uchar *input, ushort ln);
long   save_bmp         (FILE *fptr, char *spec);
long   save_curico      (FILE *fptr, char *spec);
long   save_gif         (FILE *fptr, char *spec);
long   save_graphic     (char *name, uchar *image, char *spec, long free);
long   save_grob        (FILE *fptr, char *spec);
long   save_jpg         (FILE *fptr, char *spec);
long   save_pcx         (FILE *fptr, char *spec);
long   save_ppm         (FILE *fptr, char *spec);
long   save_tga         (FILE *fptr, char *spec);
long   save_tif         (FILE *fptr, char *spec);
void   scale_pic        (uchar *dest, uchar *source, long srcw, long destw,
                         long simgw, long dimgw, long simgh, long dimgh,
                         long bclr, long trans);
void   scale_pic_prep   (long *ind, long *fac, long w, long sw,
                         float invscale, long inter);
void   scale_pic24      (uchar *dest, uchar *source, long srcw, long destw,
                         long simgw, long dimgw, long simgh, long dimgh,
                         long bclr, long trans);
long   set_color_table  (long size, long notlast);
void   slide_cat_select (long cat);
void   unbmp4rle        (uchar *source, long len, uchar *dest, long w,
                         long h);
void   unbmp8rle        (uchar *source, long len, uchar *dest, long w,
                         long h);
long   unccitt_mode2    (uchar *output, uchar *input, long ln, long srclen,
                         long bitorder, long w);
long   unccitt_t4       (uchar *output, uchar *input, long ln, long srclen,
                         long bitorder, long w);
long   unccitt_t6       (uchar *output, uchar *input, long ln, long srclen,
                         long bitorder, long w, long h);
void   undct_1d         (long *src, long *dest);
void   undct_1d_alt     (long *src, long *dest);
void   ungif            (void *dest, void *source, long len);
void   uninterlace      (char *addr, long rev);
void   unlzw            (void *dest, void *source, long maxlen, long srclen,
                         short numbits);
void   unlzwtif         (void *dest, void *source, long maxlen, long srclen);
long   unrle            (void *dest, void *source, long len, long srclen);
long   use_palette      (uchar *pal);
void   vertical_flip    (uchar *image, long w, long h);

#ifndef LIBRARY
extern char fcheck1[], outcode[], pspec[], software[], suffix[], zagtbl[];
extern float Gamma;
extern long Dither, Interpolate, LoadPart, MAXGSIZE, Minpheight, Minpwidth,
       NumPart, perr, pfree, pheight, pinter, ppart, pwidth, UniformPal;
extern short prefix[];
extern uchar *ColorTable, imask[], jpghead[], MasterPal[], *ppic, pquant[],
       winpal[];
extern FILE *(*fopen2)(char *name, char *mode);
extern size_t (*fread2)(void *buffer, size_t size, size_t count, FILE *fptr);
extern size_t (*fwrite2)(void *buffer, size_t size, size_t count,FILE *fptr);
extern int (*fclose2)(FILE *fptr);
extern int (*fseek2)(FILE *fptr, long offset, int origin);
extern long (*ftell2)(FILE *fptr);
#endif
