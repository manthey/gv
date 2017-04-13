#ifndef LIBRARY
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "gvlib.h"
#include "gload.h"
#define STATIC
#endif

STATIC uchar dithertbl1[2048], dithertbl3[]={0,0,1,2,3,4,5,6,7,8,9,10,12,14,16,18},
       *Mfile;
STATIC long dithernum=-1, dithertbl2[900], Mlen, Mminlen=0, Mpad=0, Mpos;

HBITMAPINFO BitmapToBMP(HBITMAP gmem, HPALETTE gpal, HDC hdc)
/* Convert a BITMAP object into a standard BMP which can be predicatbly
 *  manipulated.  This uses GetObject to obtain the bitmap.
 * Enter: HBITMAP gmem: handle to the bitmap object.
 *        HPALETTE gpal: handle to the palette object used with this bitmap.
 *                       Null for none.
 *        HDC hdc: Device context for palette.  0 for no palette or use gpal.
 * Exit:  HBITMAPINFO bmp: handle to the regular bitmap.        4/5/97-DWM */
{
  char *dest=0, *buf;
  long w, h, bits, obits, scan, pal, i;
  BITMAP data;
  HANDLE hdest;

  GetObject(gmem, sizeof(BITMAP), &data);
  w = data.bmWidth;  h = data.bmHeight;  bits = data.bmBitsPixel;
  obits = bits;
  if (hdc && bits>8)  bits = 24;
  if (bits<=8)       { scan = (w+3)&0xFFFFC;  pal = 1024; }
  else if (bits==16) { scan = (w*2+3)&0xFFFFC;  pal = 0; }
  else               { scan = (w*3+3)&0xFFFFC;  pal = 0; }
  hdest = GlobalAlloc(GHND|GMEM_DDESHARE, scan*h+pal+40);
  dest = lock2(hdest);
  if (!dest)  return(0);
  buf = ((char *)dest)+40+pal;
  ((long *)dest)[0] = 40;
  ((long *)dest)[1] = w;  ((long *)dest)[2] = h;
  ((short *)dest)[6] = 1;  ((short *)dest)[7] = bits;
  ((long *)dest)[4] = ((long *)dest)[6] = ((long *)dest)[7] = 0;
  ((long *)dest)[5] = scan*h;
  ((long *)dest)[8] = ((long *)dest)[9] = 0;
  if (gpal && pal && bits!=16) {
    GetPaletteEntries(gpal, 0, pal/4, (LPPALETTEENTRY)(dest+40));
    for (i=0; i<pal/4; i++)
      bgr_to_rgb(dest+40+i*4, 1); }
  if (!gpal && hdc && pal)
    GetDIBits(hdc, gmem, 0, h, 0, (LPBITMAPINFO)dest, DIB_RGB_COLORS);
  if (hdc)
    GetDIBits(hdc, gmem, 0, h, buf, (LPBITMAPINFO)dest, DIB_RGB_COLORS);
  else {
    GetBitmapBits(gmem, scan*h, buf);
    vertical_flip(buf, scan, h); }
//  if (obits==16 && hdc)
//    for (j=0; j<h; j++)
//      bgr_to_rgb(buf+scan*j, w);
  return(unlock2(dest));
}

STATIC uchar *bmp_to_graphic(HBITMAPINFO bmp)
/* Convert a bitmap in memory to the graphic format.
 * Enter: HBITMAPINFO bmp: handle to the bitmap.
 * Exit:  uchar *pic: a pointer to an array which was malloc2ed containing
 *                    the image.  pwidth, pheight, and pinter tell what sort
 *                    of image this is.                        2/22/97-DWM */
{
  uchar *bpic, *pic;

  bpic = lock2(bmp);
  ppart = 0;
  LoadPart = -1;
  fopen2 = mopen;  fread2 = mread;  fclose2 = mclose;
  fseek2 = mseek;  ftell2 = mtell;  fwrite2 = mwrite;
  Mpad = 14;
  pic = load_graphic(bpic);
  Mpad = 0;
  fopen2 = fopen;  fread2 = fread;  fclose2 = fclose;
  fseek2 = fseek;  ftell2 = ftell;  fwrite2 = fwrite;
  unlock2(bpic);
  return(pic);
}

#ifdef LIBRARY
LPLOGPALETTE BMPPalette(HBITMAPINFO bmp)
/* Compute the optimal palette for use with a particular bitmap.  If the
 *  bitmap is 8-bits or less, the palette of the bitmap is extracted.
 * Enter: HBITMAPINFO bmp: handle to bitmap.
 * Exit:  LPLOGPALETTE palette: Locally allocated pointer to windows style
 *                              optimal palette, or null to indicate an
 *                              error.                         3/21/97-DWM */
{
  uchar *pic;
  LOGPALETTE *lpal;
  long i;

  if (!bmp)  return(0);
  pic = bmp_to_graphic(bmp);
  if (pinter)  memcpy(MasterPal, pic, 768);
  else         palettize(pic, pwidth, pheight, 1);
  free2(pic);
  lpal = LocalAlloc(LPTR, sizeof(LOGPALETTE) + 256*sizeof(PALETTEENTRY));
  if (!lpal)
    return(0);
  lpal->palVersion = 0x300;  lpal->palNumEntries = 256;
  for (i=0; i<256; i++) {
    lpal->palPalEntry[i].peRed   = MasterPal[i*3];
    lpal->palPalEntry[i].peGreen = MasterPal[i*3+1];
    lpal->palPalEntry[i].peBlue  = MasterPal[i*3+2];
    lpal->palPalEntry[i].peFlags = 0; }
  return(lpal);
}
#endif

#ifdef LIBRARY
HANDLE BMPToGraphic(GV *gv, HBITMAPINFO bmp)
/* Convert a bitmap in memory to the graphic format.
 * Enter: GV *gv: location to store graphic width, height, and palettization.
 *        HBITMAPINFO bmp: handle to the bitmap.
 * Exit:  HANDLE graphic: GlobalAlloc'ed handle to the graphic, or 0 for
 *                        error.  graphicspec contains the error number.
 *                                                             2/22/97-DWM */
{
  uchar *pic;

  if (!bmp)  return(0);
  pic = bmp_to_graphic(bmp);
  fill_gv(gv, !pic);
  return(unlock2(pic));
}
#endif

STATIC uchar *depalettize_graphic(long w, long h, long pal, uchar *pic)
/* Convert an 8-bit graphic to a 24-bit graphic.  If the graphic is already
 *  24-bit, just copy it.
 * Enter: long w, h: size of the graphic.
 *        long pal: 0 for 24-bit source image, 1 for 8-bit source image.
 *        uchar *pic: pointer to 24-bit graphic.
 * Exit:  uchar *pic24: graphic in 24-bit format.  Null for failed (due to
 *                      memory).                               3/20/97-DWM */
{
  long siz=w*h*3, okay;
  uchar *apic;

  if (!pic)  return(0);
  if (!pal) {
    if (!(apic=malloc2(siz)))
      return(0);
    memcpy(apic, pic, siz);
    return(apic); }
  ppic = pic;  pwidth = w;  pheight = h;  pinter = pal;  pfree = 0;
  okay = best_color(1, -1, -1);
  if (okay!=0)  return(0);
  return(ppic);
}

#ifdef LIBRARY
HBITMAPINFO DepalettizeBMP(HBITMAPINFO bmp)
/* Convert any bitmap to 24-bits.
 * Enter: HBITMAPINFO bmp: handle to bitmap.
 * Exit:  HBITMAPINFO bmp24: 24-bit bitmap.  Null for failed (due to memory).
 *                                                             2/22/97-DWM */
{
  uchar *pic, *apic;
  HBITMAPINFO bmp24;
  long w, h, pal;

  if (!bmp)  return(0);
  pic = bmp_to_graphic(bmp);
  if (!pic)  return(0);
  w = pwidth;  h = pheight;  pal = pinter;
  if (!pal) {
    bmp24 = graphic_to_bmp(w, h, pal, pic, 0);
    free2(pic);
    return(bmp24); }
  apic = depalettize_graphic(w, h, pal, pic);
  free2(pic);
  if (!apic)
    return(0);
  bmp24 = graphic_to_bmp(w, h, 0, apic, 0);
  free2(apic);
  return(bmp24);
}
#endif

#ifdef LIBRARY
HANDLE DepalettizeGraphic(long w, long h, long pal, HANDLE hpic)
/* Convert an 8-bit graphic to a 24-bit graphic.  If the graphic is already
 *  24-bit, just copy it.
 * Enter: long w, h: size of the graphic.
 *        long pal: 0 for 24-bit image, 1 for an 8-bit image.
 *        HANDLE hpic: handle to 24-bit graphic.
 * Exit:  HANDLE hpic8: palettized graphic in 8-bit format.  Null for failed
 *                     (due to memory).                        2/22/97-DWM */
{
  uchar *pic, *apic;

  if (!hpic)  return(0);
  pic = lock2(hpic);
  apic = depalettize_graphic(w, h, pal, pic);
  unlock2(pic);
  if (!apic)
    return(0);
  return(unlock2(apic));
}
#endif

#ifdef LIBRARY
STATIC void fill_gv(GV *gv, long err)
/* Fill the specification structure with information about the loaded file or
 *  about any errors which have occured.
 * Enter: GV *gv: specification block to modify.
 *        long err: 0 if no error occurred, 1 to retrieve load error code, 2
 *                  for save error code.                       1/19/97-DWM */
{
  long loaderr[]={GV_UNKNOWN,GV_READERROR,GV_UNKNOWN,GV_LOWMEMORY,GV_BADBMPTYPE,
       GV_LOWMEMORY,GV_LOWMEMORY,GV_UNKNOWNCOMPRESSION,GV_INVALIDSIZE,GV_LOWMEMORY,
       GV_UNSUPPORTEDCOMPRESSION,GV_UNSUPPORTEDBITSPERPIXEL,GV_UNSUPPORTEDCOLOR,
       GV_BADSPECTRAL,GV_TOOMANYQUANTIZATION,GV_BADSCANHEADER,GV_PALETTETOOBIG,
       GV_UNSUPPORTEDORIENTATION,GV_UNSUPPORTEDCOLORSPACE,GV_PLANARFORMAT,GV_TOOMANYSTRIPS,
       GV_LOWMEMORY,GV_LOWMEMORY,GV_NOSUCHIMAGE,GV_UNKNOWN};
  long saveerr[]={-1,GV_UNKNOWN,GV_CANTWRITE,GV_SAVELOWMEMORY,GV_CANTAPPEND,GV_APPENDDOESNTMATCH,
       -1,GV_ENDIANDOESNTMATCH};

  gv->format = pspec[0];
  gv->image = pspec[1]&0x7F;
  gv->width = pwidth;
  gv->height = pheight;
  gv->palette = pinter;
  gv->offset = 0;
  gv->error = 0;
  if (err==1)  gv->error = loaderr[perr>>8];
  if (err==2)  gv->error = saveerr[perr];
  switch (gv->format) {
    case GV_BMP:
      gv->fs.bmp.color = pspec[2];
      gv->fs.bmp.compression = pspec[3];
      gv->fs.bmp.dpi = ((unsigned short *)(pspec+4))[0];
      gv->fs.bmp.bmptype = pspec[6]; break;
    case GV_CUR:
      gv->fs.cur.image = pspec[1]&0x7F;
      gv->fs.cur.moreimages = (pspec[1]&0x80)!=0;
      gv->fs.cur.color = pspec[2];
      gv->fs.cur.dpi = ((unsigned short *)(pspec+4))[0];
      gv->fs.cur.cursize = pspec[6];
      gv->fs.cur.transparent = ((long *)(pspec+8))[0];
      gv->fs.cur.inverse = ((long *)(pspec+0xC))[0];
      gv->fs.cur.hotx = ((short *)(pspec+0x10))[0];
      gv->fs.cur.hoty = ((short *)(pspec+0x12))[0]; break;
    case GV_GIF:
      gv->fs.gif.image = (pspec[1]&0x7F)+(((short *)(pspec+0x12))[0]<<7);
      gv->image = (pspec[1]&0x7F)+(((short *)(pspec+0x12))[0]<<7);
      gv->fs.gif.moreimages = (pspec[1]&0x80)!=0;
      gv->fs.gif.giftype = pspec[6];
      gv->fs.gif.interlace = pspec[7];
      gv->fs.gif.usetrans = pspec[2];
      gv->fs.gif.transparent = ((long *)(pspec+8))[0];
      gv->fs.gif.delay = ((short *)(pspec+0xC))[0];
      gv->fs.gif.repeat = ((short *)(pspec+0x10))[0];
      gv->fs.gif.keep = ((short *)(pspec+0xE))[0]; break;
    case GV_GRO:
      gv->fs.gro.grobtype = pspec[6];
      gv->fs.gro.grobsize = pspec[7];
      gv->fs.gro.numshades = ((long *)(pspec+0xC))[0];
      gv->fs.gro.gamma = ((float *)(pspec+8))[0]; break;
    case GV_ICO:
      gv->fs.ico.image = pspec[1]&0x7F;
      gv->fs.ico.moreimages = (pspec[1]&0x80)!=0;
      gv->fs.ico.color = pspec[2];
      gv->fs.ico.dpi = ((unsigned short *)(pspec+4))[0];
      gv->fs.ico.icosize = pspec[6];
      gv->fs.ico.transparent = ((long *)(pspec+8))[0];
      gv->fs.ico.inverse = ((long *)(pspec+0xC))[0]; break;
    case GV_JPG:
      gv->fs.jpg.image = (pspec[1]&0x7F)+(((short *)(pspec+0x12))[0]<<7);
      gv->image = (pspec[1]&0x7F)+(((short *)(pspec+0x12))[0]<<7);
      gv->fs.jpg.moreimages = (pspec[1]&0x80)!=0;
      gv->fs.jpg.color = pspec[2];
      gv->fs.jpg.compression = pspec[3];
      gv->fs.jpg.dpi = ((unsigned short *)(pspec+4))[0];
      gv->fs.jpg.decimation = pspec[6];
      gv->fs.jpg.quality = ((long *)(pspec+8))[0];
      gv->fs.jpg.usecustom = pspec[7];
      gv->fs.jpg.customquant = ((char **)(pspec+0xC))[0];
      gv->fs.jpg.spectralsteps = ((short *)(pspec+0x10))[0];
      gv->fs.jpg.succesivesteps = ((short *)(pspec+0x12))[0]; break;
    case GV_PPM:
      gv->fs.ppm.color = pspec[2];
      gv->fs.ppm.ppmtype = pspec[6]; break;
    case GV_PCX:
      gv->fs.pcx.color = pspec[2]; break;
    case GV_TIF:
      strcpy(gv->fs.tif.software, software);
      strcpy(gv->fs.tif.serial, serial);
      strcpy(gv->fs.tif.model, tifmodel);
      gv->fs.tif.image = (pspec[1]&0x7F)+(((short *)(pspec+0x12))[0]<<7);
      gv->image = (pspec[1]&0x7F)+(((short *)(pspec+0x12))[0]<<7);
      gv->fs.tif.moreimages = (pspec[1]&0x80)!=0;
      gv->fs.tif.color = pspec[2];
      gv->fs.tif.compression = pspec[3];
      gv->fs.tif.dpi = ((unsigned short *)(pspec+4))[0];
      gv->fs.tif.endian = pspec[6];
      gv->fs.tif.rowsperstrip = ((long *)(pspec+8))[0]; }
}
#endif

#ifdef LIBRARY
STATIC void fill_spec(GV *gv)
/* Fill the internal array with information from the specification structure.
 * Enter: GV *gv: specification block to use as source.        2/22/97-DWM */
{
  memset(pspec, 0, LENPSPEC);
  pspec[0] = gv->format;
  pspec[1] = (gv->image!=0);
  pwidth = gv->width;
  pheight = gv->height;
  pinter = gv->palette;
  Dither = gv->dither;
  software[0] = 0;  serial[0] = 0;  tifmodel[0] = 0;
  switch (gv->format) {
    case GV_BMP:
      pspec[2] = gv->fs.bmp.color;
      pspec[3] = gv->fs.bmp.compression;
      ((unsigned short *)(pspec+4))[0] = gv->fs.bmp.dpi;
      gv->fs.bmp.bmptype = pspec[6]; break;
    case GV_CUR:
      pspec[2] = gv->fs.cur.color;
      ((unsigned short *)(pspec+4))[0] = gv->fs.cur.dpi;
      pspec[6] = gv->fs.cur.cursize;
      ((long *)(pspec+8))[0] = gv->fs.cur.transparent;
      ((long *)(pspec+0xC))[0] = gv->fs.cur.inverse;
      ((short *)(pspec+0x10))[0] = gv->fs.cur.hotx;
      ((short *)(pspec+0x12))[0] = gv->fs.cur.hoty; break;
    case GV_GIF:
      pspec[6] = gv->fs.gif.giftype;
      pspec[7] = gv->fs.gif.interlace;
      pspec[2] = gv->fs.gif.usetrans;
      ((long *)(pspec+8))[0] = gv->fs.gif.transparent;
      ((short *)(pspec+0xC))[0] = gv->fs.gif.delay;
      ((short *)(pspec+0xE))[0] = gv->fs.gif.keep;
      ((short *)(pspec+0x10))[0] = gv->fs.gif.repeat; break;
    case GV_GRO:
      pspec[6] = gv->fs.gro.grobtype;
      pspec[7] = gv->fs.gro.grobsize;
      ((long *)(pspec+0xC))[0] = gv->fs.gro.numshades;
      ((float *)(pspec+8))[0] = gv->fs.gro.gamma; break;
    case GV_ICO:
      pspec[2] = gv->fs.ico.color;
      ((unsigned short *)(pspec+4))[0] = gv->fs.ico.dpi;
      pspec[6] = gv->fs.ico.icosize;
      ((long *)(pspec+8))[0] = gv->fs.ico.transparent;
      ((long *)(pspec+0xC))[0] = gv->fs.ico.inverse; break;
    case GV_JPG:
      pspec[2] = gv->fs.jpg.color;
      pspec[3] = gv->fs.jpg.compression;
      ((unsigned short *)(pspec+4))[0] = gv->fs.jpg.dpi;
      pspec[6] = gv->fs.jpg.decimation;
      ((long *)(pspec+8))[0] = gv->fs.jpg.quality;
      pspec[7] = gv->fs.jpg.usecustom;
      ((char **)(pspec+0xC))[0] = gv->fs.jpg.customquant;
      ((short *)(pspec+0x10))[0] = gv->fs.jpg.spectralsteps;
      ((short *)(pspec+0x12))[0] = gv->fs.jpg.succesivesteps; break;
    case GV_PPM:
      pspec[2] = gv->fs.ppm.color;
      pspec[6] = gv->fs.ppm.ppmtype; break;
    case GV_PCX:
      pspec[2] = gv->fs.pcx.color; break;
    case GV_TIF:
      strcpy(software, gv->fs.tif.software);
      strcpy(serial, gv->fs.tif.serial);
      strcpy(tifmodel, gv->fs.tif.model);
      pspec[2] = gv->fs.tif.color;
      pspec[3] = gv->fs.tif.compression;
      ((unsigned short *)(pspec+4))[0] = gv->fs.tif.dpi;
      pspec[6] = gv->fs.tif.endian;
      ((long *)(pspec+8))[0] = gv->fs.tif.rowsperstrip; }
}
#endif

STATIC HBITMAPINFO graphic_to_bmp(long w, long h, long pal, uchar *pic,
                                  long free)
/* Convert a graphic into a 1, 8, or 24-bit per pixel bitmap.  A 1-bit per
 *  pixel bitmap is only created from a palettized graphic with all pixels
 *  having a value of 0 or 1.  The bitmap is never compressed.
 * Enter: long w, h: size of the graphic.
 *        long pal: 0 for RGB, 1 for palettized.
 *        uchar *pic: pointer to graphic.
 *        long free: 0 to keep graphic, 1 to free pic.
 * Exit:  HBITMAPINFO bmp: GlobalAlloc'ed bitmap.              1/19/97-DWM */
{
  fopen2 = mopen;  fread2 = mread;  fclose2 = mclose;
  fseek2 = mseek;  ftell2 = mtell;  fwrite2 = mwrite;
  memset(pspec, 0, LENPSPEC);
  ((short *)(pspec+4))[0] = 160;
  pwidth = w;  pheight = h;  pinter = pal;
  Mpad = 14;  Mminlen = w*h*(3-2*(pal!=0))+40+1024*(pal!=0);
  if (save_graphic(0, pic, pspec, free)) {
    fopen2 = fopen;  fread2 = fread;  fclose2 = fclose;
    fseek2 = fseek;  ftell2 = ftell;  fwrite2 = fwrite;
    Mpad = 0;  Mminlen = 0;
    return(0); }
  Mpad = 0;  Mminlen = 0;
  fopen2 = fopen;  fread2 = fread;  fclose2 = fclose;
  fseek2 = fseek;  ftell2 = ftell;  fwrite2 = fwrite;
  return(unlock2(Mfile));
}

#ifdef LIBRARY
HANDLE GraphicPalette(long w, long h, long pal, HANDLE hpic)
/* Compute the optimal palette for use with a particular graphic.  If the
 *  graphic is 8-bits, the palette of the graphic is extracted.
 * Enter: long w, h: size of the graphic.
 *        long pal: 0 for RGB, 1 for palettized.
 *        HANDLE hpic: handle to graphic.
 * Exit:  HANDLE palette: handle to 768 bytes RGBRGB palette, or null to
 *                        indicate an error.                   3/21/97-DWM */
{
  uchar *pic;
  uchar *pal2;

  if (!hpic)  return(0);
  pic = lock2(hpic);
  if (pal)  memcpy(MasterPal, pic, 768);
  else      palettize(pic, w, h, 1);
  unlock2(pic);
  if (!(pal2=malloc2(768)))
    return(0);
  memcpy(pal2, MasterPal, 768);
  return(unlock2(pal2));
}
#endif

#ifdef LIBRARY
HBITMAPINFO GraphicToBMP(long w, long h, long pal, HANDLE hpic)
/* Convert a graphic into a 1, 8, or 24-bit per pixel bitmap.  A 1-bit per
 *  pixel bitmap is only created from a palettized graphic with all pixels
 *  having a value of 0 or 1.  The bitmap is never compressed.
 * Enter: long w, h: size of the graphic.
 *        long pal: 0 for RGB, 1 for palettized.
 *        HANDLE hpic: handle to graphic.
 * Exit:  HBITMAPINFO bmp: GlobalAlloc'ed bitmap.              2/22/97-DWM */
{
  char *pic;
  HBITMAPINFO bmp;

  if (!hpic)  return(0);
  pic = lock2(hpic);
  bmp = graphic_to_bmp(w, h, pal, pic, 0);
  unlock2(pic);
  return(bmp);
}
#endif

#ifdef LIBRARY
HBITMAPINFO LoadBMP(GV *gv)
/* Load a graphic into memory and create a 1-bit, 8-bit, or 24-bit BMP out of
 *  it.
 * Enter: GV *gv: specification block for loading.  See complete
 *                documentation for gruesome details.
 * Exit:  HBITMAPINFO *bmp: GlobalAlloc'ed handle of bitmap, or NULL for
 *                          error.                             1/19/97-DWM */
{
  uchar *pic;
  HBITMAPINFO bmp;

  ppart = gv->image;
  LoadPart = -1;
  if (gv->fopenb)   fopen2 = gv->fopenb;   else fopen2 = fopen;
  if (gv->freadb)   fread2 = gv->freadb;   else fread2 = fread;
  if (gv->fcloseb)  fclose2 = gv->fcloseb; else fclose2 = fclose;
  if (gv->fseekb)   fseek2 = gv->fseekb;   else fseek2 = fseek;
  if (gv->ftellb)   ftell2 = gv->ftellb;   else ftell2 = ftell;
  if (fopen2==fopen && gv->name)  if (!gv->name[0]) {
    fopen2 = mopen2;  fread2 = mread;  fclose2 = mclose;
    fseek2 = mseek;  ftell2 = mtell;  fwrite2 = mwrite;
    Mlen = ((long *)(gv->name+1))[1];
    gv->name = (char *)(((long *)(gv->name+1))[0]); }
  pic = load_graphic(gv->name);
  fopen2 = fopen;  fread2 = fread;  fclose2 = fclose;
  fseek2 = fseek;  ftell2 = ftell;  fwrite2 = fwrite;
  fill_gv(gv, !pic);
  if (!pic)  return(0);
  bmp = graphic_to_bmp(gv->width, gv->height, gv->palette, pic, 1);
  if (!bmp)  gv->error = GV_LOWMEMORY;
  return(bmp);
}
#endif

#ifdef LIBRARY
HANDLE LoadGraphic(GV *gv)
/* Load a graphic into memory.
 * Enter: GV *gv: specification block for loading.  See complete
 *                documentation for gruesome details.
 * Exit:  HANDLE *pic: GlobalAlloc'ed handle of graphic, or NULL for error.
 *                                                             1/19/97-DWM */
{
  uchar *pic;

  ppart = gv->image;
  LoadPart = -1;
  if (gv->fopenb)   fopen2 = gv->fopenb;   else fopen2 = fopen;
  if (gv->freadb)   fread2 = gv->freadb;   else fread2 = fread;
  if (gv->fcloseb)  fclose2 = gv->fcloseb; else fclose2 = fclose;
  if (gv->fseekb)   fseek2 = gv->fseekb;   else fseek2 = fseek;
  if (gv->ftellb)   ftell2 = gv->ftellb;   else ftell2 = ftell;
  if (fopen2==fopen && gv->name)  if (!gv->name[0]) {
    fopen2 = mopen2;  fread2 = mread;  fclose2 = mclose;
    fseek2 = mseek;  ftell2 = mtell;  fwrite2 = mwrite;
    Mlen = ((long *)(gv->name+1))[1];
    gv->name = (char *)(((long *)(gv->name+1))[0]); }
  pic = load_graphic(gv->name);
  fopen2 = fopen;  fread2 = fread;  fclose2 = fclose;
  fseek2 = fseek;  ftell2 = ftell;  fwrite2 = fwrite;
  fill_gv(gv, !pic);
  if (!pic)  return(0);
  return(unlock2(pic));
}
#endif

STATIC int mclose(FILE *fptr)
/* Close a memory file.  This function does absolutely nothing, and is
 *  provided for compatibility only.
 * Enter: FILE *fptr: dummy variable.
 * Exit:  int failed: 0 for okay (always).                     2/22/97-DWM */
{
  return(0);
}

STATIC FILE *mopen(char *name, char *mode)
/* Open a "file" in memory for reading or writing.  Although this accepts the
 *  same parameters as fopen, they are not handled identically.  Only one
 *  memory file can be open at a time.  Any call to mopen will abandon a
 *  previous memory file.  The current memory file's pointer is always
 *  located in Mfile.
 * Enter: char *name: This is a pointer to a malloc2ed memory array used as
 *                    the file.  If null, the file will be "created" using
 *                    malloc2.
 *        char *mode: ignored.  The mode is always capable of reading and
 *                    writing, and is always binary.
 * Exit:  FILE *file: Used only for error return.  0 for error, other for
 *                    okay.                                    2/22/97-DWM */
{
  if (name) {
    Mfile = name;
    Mlen = size2(name)+Mpad; }
  else {
    if (Mminlen>0)  Mfile = malloc2(Mminlen);
    else            Mfile = malloc2(1);
    if (!Mfile)  return(0);
    Mlen = 0;
    if (Mminlen>0)  Mlen = Mminlen; }
  Mpos = 0;
  return((FILE *)-1);
}

STATIC FILE *mopen2(char *name, char *mode)
/* Open a "file" in memory for reading or writing.  Although this accepts the
 *  same parameters as fopen, they are not handled identically.  Only one
 *  memory file can be open at a time.  Any call to mopen will abandon a
 *  previous memory file.  The current memory file's pointer is always
 *  located in Mfile.  The length of the file is already stored in Mlen.
 * Enter: char *name: This is a pointer to a malloc2ed memory array used as
 *                    the file.  If null, the file will be "created" using
 *                    malloc2.
 *        char *mode: ignored.  The mode is always capable of reading and
 *                    writing, and is always binary.
 * Exit:  FILE *file: Used only for error return.  0 for error, other for
 *                    okay.                                    2/22/97-DWM */
{
  if (name)
    Mfile = name;
  else
    return(0);
  Mpos = 0;
  return((FILE *)-1);
}

STATIC size_t mread(char *buffer, size_t size, size_t count, FILE *fptr)
/* Read data from a memory buffer.  This mimicks the fread function,
 *  including EOF behavior.
 * Enter: void *buffer: location to store data.
 *        size_t size: size of each memory item to read.
 *        size_t count: number of memory items to read.
 *        FILE *fptr: dummy varibale for compatibility.
 * Exit:  size_t num: number of memory items actually read.    2/22/97-DWM */
{
  long len;
  char bm[]="BM\0\0\0\0\0\0\0\0\0\0\0\0";

  len = Mlen-Mpos;
  if (size*count > (size_t)len)
    count = len/size;
  len = size*count;
  if (Mpos-Mpad<0 && len+Mpos-Mpad>0) {
    memcpy(buffer, bm, min(14, Mpad-Mpos));
    buffer += (Mpad-Mpos);  len -= (Mpad-Mpos);  Mpos = Mpad; }
  else if (Mpos-Mpad<0) {
    memcpy(buffer, bm, min(14, len));  Mpos += len;  len = 0; }
  if (len) {
    memcpy(buffer, Mfile+Mpos-Mpad, len);
    Mpos += len; }
  return(count);
}

STATIC int mseek(FILE *fptr, long offset, int origin)
/* Change the file pointer of a memory file.
 * Enter: FILE *fptr: dummy pointer.
 *        long offset: distance to move the file pointer.
 *        int origin: how to move the file pointer.  Either SEEK_CUR,
 *                    SEEK_END, or SEEK_SET.
 * Exit:  int error: 0 for okay, 1 for attempted to position pointer before
 *                   file.                                     2/22/97-DWM */
{
  switch (origin) {
    case SEEK_CUR: Mpos += offset; break;
    case SEEK_END: Mpos = Mlen+offset; break;
    case SEEK_SET: Mpos = offset; }
  if (Mpos<0) { Mpos = 0;  return(1); }
  if (Mpos>Mlen)  Mpos = Mlen;
  return(0);
}

STATIC long mtell(FILE *fptr)
/* Report the location of the file pointer for a memory file.
 * Enter: FILE *fptr: dummy pointer.
 * Exit:  long pos: position of file pointer.                  2/22/97-DWM */
{
  return(Mpos);
}

STATIC size_t mwrite(char *buffer, size_t size, size_t count, FILE *fptr)
/* Write data to a memory buffer.  This mimicks the fwrite function,
 *  including EOF behavior.
 * Enter: void *buffer: location to store data.
 *        size_t size: size of each memory item to read.
 *        size_t count: number of memory items to read.
 *        FILE *fptr: dummy varibale for compatibility.
 * Exit:  size_t num: number of memory items actually read.    2/22/97-DWM */
{
  long len;
  uchar *file;

  len = Mlen-Mpos;
  if (size*count > (size_t)len) {
    file = realloc2(Mfile, Mpos+size*count-Mpad);
    if (!file)
      count = len/size;
    else {
      Mfile = file;
      Mlen = Mpos+size*count; } }
  len = size*count;
  if (Mpos-Mpad<0 && len+Mpos-Mpad>0) {
    buffer += (Mpad-Mpos);  len -= (Mpad-Mpos);  Mpos = Mpad; }
  else if (Mpos-Mpad<0) {
    Mpos += len;  len = 0; }
  if (len) {
    memcpy(Mfile+Mpos-Mpad, buffer, len);
    Mpos += len; }
  return(count);
}

STATIC uchar *palettize_graphic(long w, long h, long pinter, uchar *pic,
                                long dither, uchar *pal)
/* Reduce a 24-bit graphic to 8-bits, dithering it if desired.  Or, convert
 *  an 8-bit graphic from one palette to another.  This is done with
 *  dithering, if so specified.
 * Enter: long w, h: size of the graphic.
 *        long pinter: 0 for 24-bit source image, 1 for 8-bit source image.
 *        uchar *pic: pointer to 24-bit graphic.
 *        long dither: 0 for no dither, 1 for dithering.
 *        uchar *pal: null for use optimized palette, -1 for use uniform
 *                    palette, otherwise this a pointer to the palette to be
 *                    used.  If null and the source image is 8-bit, the image
 *                    is copied to a new memory area.
 * Exit:  uchar *pic8: palettized graphic in 8-bit format.  Null for failed
 *                     (due to memory).                        2/22/97-DWM */
{
  uchar *apic, conv[256];
  long i, j, exact, siz=w*h;

  if (!pal && pinter) {
    if (!(apic=malloc2(768+siz)))
      return(0);
    memcpy(apic, pic, 768+siz);
    return(apic); }
  Dither = dither;
  if (dithernum<0 && Dither) {
    for (i=0; i<2048; i++)  dithertbl1[i] = dithertbl3[rand()%16]+22;
    for (i=0; i<256+22*2; i++) {
      j = i-22;  if (j<0) j = 0;  if (j>255) j =255;
      dithertbl2[i]     = (j&0xF8)<<8;
      dithertbl2[i+300] = (j&0xFC)<<3;
      dithertbl2[i+600] =  j      >>3; }
    dithernum = 0; }
  if ((!pal && !pinter) || ((long)pal)==-1) {
    UniformPal = (((long)pal)==-1);
    if (!(exact=palettize(pic, w, h, 1))) {
      UniformPal = 0;
      return(0); }
    UniformPal = 0; }
  else if (pal)
    exact = use_palette(pal);
  else
    exact = use_palette(pic);
  if (!(apic=malloc2(768+siz)))
    return(0);
  if (!Dither || exact==1) {
    if (!pinter)
      for (i=j=0; i<siz; i++, j+=3)
        apic[i+768] = ColorTable[((pic[j]&0xF8)<<8)+((pic[j+1]&0xFC)<<3)+
                                 (pic[j+2]>>3)];
    else {
      for (i=j=0; i<256; i++, j+=3)
        conv[i] = ColorTable[((pic[j]&0xF8)<<8)+((pic[j+1]&0xFC)<<3)+
                             (pic[j+2]>>3)];
      for (i=0; i<siz; i++)
        apic[i+768] = conv[pic[i+768]]; } }
  else {
    for (i=j=0; i<siz; i++, j+=3) {
      if (pinter)  j = pic[i+768]*3;
      apic[i+768] = ColorTable[dithertbl2[pic[j]+dithertbl1[dithernum]]+
                    dithertbl2[pic[j+1]+dithertbl1[dithernum+1]+300]+
                    dithertbl2[pic[j+2]+dithertbl1[dithernum+2]+600]];
      dithernum+=3;  if (dithernum>2040) dithernum = rand()%100; } }
  memcpy(apic, MasterPal, 768);
  return(apic);
}

#ifdef LIBRARY
HBITMAPINFO PalettizeBMP(HBITMAPINFO bmp, long dither, LPLOGPALETTE palette)
/* Convert any bitmap to 8-bits (or less).  If it is already 8 bits or less,
 *  and an optimal palette is specified, the bitmap is just copied to a new
 *  memory area.  If it is 8 bits or less and a palette is specified, the
 *  bitmap is converted to the new palette.
 * Enter: HBITMAPINFO bmp: handle to bitmap.
 *        long dither: 0 for no dithering, 1 for dither, 2 to only dither
 *                     24-bit images.
 *        LOGPALETTE palette: windows style palette to use, or null to use
 *                            optimal palette, of -1 to use uniform palette.
 * Exit:  HBITMAPINFO bmp8: palettized bitmap.  Null for failed (due to
 *                          memory).                           2/22/97-DWM */
{
  uchar *pic, *apic, pal[768], *pal2;
  HBITMAPINFO bmp8;
  long w, h, i;

  if (!bmp)  return(0);
  pic = bmp_to_graphic(bmp);
  if (!pic)  return(0);
  w = pwidth;  h = pheight;
  if (pinter && !palette) {
    bmp8 = graphic_to_bmp(w, h, 1, pic, 0);
    free2(pic);
    return(bmp8); }
  if (!palette)                        pal2 = 0;
  else if (palette==(LPLOGPALETTE)-1)  pal2 = (uchar *)-1;
  else {
    pal2 = pal;
    memset(pal, 0, 768);
    for (i=0; i<palette->palNumEntries && i<256; i++) {
      pal[i*3]   = palette->palPalEntry[i].peRed;
      pal[i*3+1] = palette->palPalEntry[i].peGreen;
      pal[i*3+2] = palette->palPalEntry[i].peBlue; } }
  if (dither==2)  dither = 1-pinter;
  apic = palettize_graphic(w, h, pinter, pic, dither, pal2);
  free2(pic);
  if (!apic)
    return(0);
  bmp8 = graphic_to_bmp(w, h, 1, apic, 0);
  free2(apic);
  return(bmp8);
}
#endif

#ifdef LIBRARY
HANDLE PalettizeGraphic(long w, long h, long pinter, HANDLE hpic,
                        long dither, HANDLE hpal)
/* Reduce a 24-bit graphic to 8-bits, or convert an 8-bit graphic to a
 *  different palette.
 * Enter: long w, h: size of the graphic.
 *        long pinter: 0 for 24-bit image, 1 for an 8-bit image.
 *        HANDLE hpic: handle to 24-bit graphic.
 *        long dither: 0 for no dithering, 1 for dither, 2 to only dither
 *                     24-bit images.
 *        HANDLE hpal: handle to palette to use.  Null for optimal palette,
 *                     -1 for uniform palette.
 * Exit:  HANDLE hpic8: palettized graphic in 8-bit format.  Null for failed
 *                     (due to memory).                        2/22/97-DWM */
{
  uchar *pic, *apic, *pal;

  if (!hpic)  return(0);
  pic = lock2(hpic);
  if (hpal!=(HANDLE)-1)  pal = lock2(hpal);
  else                   pal = (uchar *)-1;
  if (dither==2)  dither = 1-pinter;
  apic = palettize_graphic(w, h, pinter, pic, dither, pal);
  unlock2(pic);
  if (hpal!=(HANDLE)-1)  unlock2(pal);
  if (!apic)
    return(0);
  return(unlock2(apic));
}
#endif

#ifdef LIBRARY
long SaveBMP(GV *gv, HBITMAPINFO bmp)
/* Save a bitmap to a file.
 * Enter: GV *gv: specification block for saving.  See complete
 *                documentation for gruesome details.
 *        HBITMAPINFO bmp: GlobalAlloc'ed handle of graphic.
 * Exit:  long okay: 0 for error (see gv->error), or 1 for okay.
 *                                                             2/22/97-DWM */
{
  uchar *pic;
  long err;

  if (!bmp)  return(0);
  pic = bmp_to_graphic(bmp);
  if (!pic) {
    gv->error = GV_SAVELOWMEMORY;
    return(0); }
  gv->width = pwidth;  gv->height = pheight;  gv->palette = pinter;
  if (gv->fopenb)   fopen2 = gv->fopenb;   else fopen2 = fopen;
  if (gv->freadb)   fread2 = gv->freadb;   else fread2 = fread;
  if (gv->fcloseb)  fclose2 = gv->fcloseb; else fclose2 = fclose;
  if (gv->fseekb)   fseek2 = gv->fseekb;   else fseek2 = fseek;
  if (gv->ftellb)   ftell2 = gv->ftellb;   else ftell2 = ftell;
  fill_spec(gv);
  err = save_graphic(gv->name, pic, pspec, 0);
  free2(pic);
  fopen2 = fopen;  fread2 = fread;  fclose2 = fclose;
  fseek2 = fseek;  ftell2 = ftell;  fwrite2 = fwrite;
  fill_gv(gv, (err!=0)*2);
  return(!err);
}
#endif

#ifdef LIBRARY
long SaveGraphic(GV *gv, HANDLE hpic)
/* Save a graphic to a file.
 * Enter: GV *gv: specification block for saving.  See complete
 *                documentation for gruesome details.
 *        HANDLE hpic: GlobalAlloc'ed handle of graphic.
 * Exit:  long okay: 0 for error (see gv->error), or 1 for okay.
 *                                                             2/22/97-DWM */
{
  uchar *pic;
  long err;

  if (!hpic)  return(0);
  pic = lock2(hpic);
  if (gv->fopenb)   fopen2 = gv->fopenb;   else fopen2 = fopen;
  if (gv->fwriteb)  fwrite2 = gv->fwriteb; else fwrite2 = fwrite;
  if (gv->fcloseb)  fclose2 = gv->fcloseb; else fclose2 = fclose;
  if (gv->fseekb)   fseek2 = gv->fseekb;   else fseek2 = fseek;
  if (gv->ftellb)   ftell2 = gv->ftellb;   else ftell2 = ftell;
  fill_spec(gv);
  err = save_graphic(gv->name, pic+gv->offset, pspec, 0);
  unlock2(pic);
  fopen2 = fopen;  fread2 = fread;  fclose2 = fclose;
  fseek2 = fseek;  ftell2 = ftell;  fwrite2 = fwrite;
  fill_gv(gv, !err);
  return(!err);
}
#endif

STATIC uchar *scale_graphic(long w, long h, long pal, uchar *pic, long nw,
                            long nh, long color)
/* Scale a graphic image to a new size, letterboxing it if needed.  The image
 *  is interpolated based on the Interpolate variable.
 * Enter: long w, h: size of original graphic.
 *        long pal: palettization of original graphic.  The output is this
 *                  same palettization.
 *        uchar *pic: pointer to the graphic.
 *        long nw, nh: size of the graphic to create.  The output is
 *                     always this size, which may require letterboxing.
 *        long color: 0xRRGGBB color used for letterboxing.
 * Exit:  uchar *newpic: pointer to scaled graphic.  Null for failed.
 *                                                             3/12/97-DWM */
{
  uchar *newpic;
  long i, clr, best=1024, val, w2, h2;

  w2 = nw;  h2 = h*nw/w;
  if (h2>nh || w*nh/h==w2) { h2 = nh;  w2 = w*nh/h;  if (w2>nw)  w2 = nw; }
  if (color==-1) { w2 = nw;  h2 = nh; }
  if (pal) {
    if (!(newpic=malloc2(nw*nh+768)))  return(0);
    memcpy(newpic, pic, 768);
    for (i=0; i<256; i++) {
      val = abs(pic[i*3]  -((uchar *)(&color))[2])+
            abs(pic[i*3+1]-((uchar *)(&color))[1])+
            abs(pic[i*3+2]-((uchar *)(&color))[0]);
      if (val<best) {
        best = val;  clr = i; } }
    if (color==-1)  clr = -1;
    if (color!=-1)
      memset(newpic+768, clr, nw*nh);
    for (i=0; i<256; i++)
      if (pic[i*3]!=i || pic[i*3+1]!=i || pic[i*3+2]!=i)  break;
    if (i!=256)
      scale_pic(newpic+768+((nh-h2)/2)*nw+(nw-w2)/2, pic+768, w, nw, w, w2,
                h, h2, clr, -1);
    else
      scale_pic_grey(newpic+768+((nh-h2)/2)*nw+(nw-w2)/2, pic+768, w, nw, w,
                     w2, h, h2, clr); }
  else {
    if (!(newpic=malloc2(nw*nh*3)))  return(0);
    if (color!=-1)
      fill_zone24(newpic, color, nw*nh);
    scale_pic24(newpic+(((nh-h2)/2)*nw+(nw-w2)/2)*3, pic, w, nw, w, w2,
                h, h2, color, -1); }
  return(newpic);
}

STATIC void scale_pic_grey(uchar *dest, uchar *source, long srcw, long destw,
                           long simgw, long dimgw, long simgh, long dimgh,
                           long bclr)
/* Scale an 8-bit greyscale image.  The global variable Interpolate affects
 *  how scaling is performed: 0-rows are dropped or duplicated, 1-bilinear
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
 *        long bclr: 8-bit background color for letterboxing, or -1 for non-
 *                   fixed aspect ratio.                      12/20/03-DWM */
{
  long *horz, *vert, i, j, t, a, b, c, d, w, h, inter=0;
  long *hfac, *vfac, *crop;
  long w2, w3, w4, w5, h2, h3, h4, h5, hist[256];
  double stepx, stepy, gamma;
  uchar *s, *pic=0;

  if (Interpolate&0x1C00) if (pic=malloc2(simgw*simgh)) {     /* Autospiff */
    for (j=0, s=source; j<simgh; j++, s+=srcw)
      memcpy(pic+simgw*j, s, simgw);
    srcw = simgw;
    source = pic;
    memset(hist, 0, 256*sizeof(long));
    for (i=t=0; i<simgw*simgh; i++) {
      hist[pic[i]]++;  t++; }
    for (a=i=0; a<256; a++) {
      i += hist[a];  if (i>0.003*t)  break; }
    for (b=255, i=0; b>=0; b--) {
      i += hist[b];  if (i>0.003*t)  break; }
    if ((Interpolate&0x1C00)>=0x1400) {
      memset(hist, 0, 256*sizeof(long));
      for (j=simgh/4, t=0; j<simgh*3/4; j++)
        for (i=simgw/4; i<simgw*3/4; i++) {
          hist[pic[j*srcw+i]]++;  t++; } }
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
    for (i=0; i<simgw*simgh; i++)
      pic[i] = hist[pic[i]]; }
  inter = Interpolate&0xFF;
  if (inter<0 || inter>6)  inter = 0;
  w = dimgw;  h = simgh*w/simgw;
  if (h>dimgh || w==simgw*dimgh/simgh) {
    h = dimgh;  w = simgw*h/simgh;  if (w>dimgw)  w = dimgw; }
  if (bclr==-1) { w = dimgw;  h = dimgh; }
  if (w==simgw && h==simgh)  inter = 0;
  horz = malloc2((w+h)*sizeof(long)*(inter+1)*2+2304*sizeof(long));
  if (w!=dimgw || h!=dimgh || !horz)
    for (j=0; j<dimgh; j++)
      memset(dest+destw*j, bclr, dimgw);
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
  for (i=0; i<(h*(inter+1)); i++)  vert[i] *= srcw;
  dest += ((dimgw-w)/2+(dimgh-h)/2*destw);
  if (!inter)
    for (j=d=0; j<h; j++, d+=destw-w)
      for (i=0; i<w; i++, d++)
        dest[d] = source[vert[j]+horz[i]];
  else
    for (j=d=0,s=source; j<h; j++, d+=destw-w)
      switch (inter) {
        case 1: for (i=0; i<w; i++, d++)
          dest[d] = ((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w])*vfac[j+h])>>18;
          break;
        case 2: for (i=0; i<w; i++, d++)
          dest[d] = crop[((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w]+s[vert[j]+horz[i+w2]]*hfac[i+w2])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w]+s[vert[j+h]+horz[i+w2]]*hfac[i+w2])*vfac[j+h]+
                    (s[vert[j+h2]+horz[i]]*hfac[i]+s[vert[j+h2]+horz[i+w]]*hfac[i+w]+s[vert[j+h2]+horz[i+w2]]*hfac[i+w2])*vfac[j+h2])>>18];
          break;
        case 3: for (i=0; i<w; i++, d++)
          dest[d] = crop[((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w]+s[vert[j]+horz[i+w2]]*hfac[i+w2]+s[vert[j]+horz[i+w3]]*hfac[i+w3])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w]+s[vert[j+h]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h]+horz[i+w3]]*hfac[i+w3])*vfac[j+h]+
                    (s[vert[j+h2]+horz[i]]*hfac[i]+s[vert[j+h2]+horz[i+w]]*hfac[i+w]+s[vert[j+h2]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h2]+horz[i+w3]]*hfac[i+w3])*vfac[j+h2]+
                    (s[vert[j+h3]+horz[i]]*hfac[i]+s[vert[j+h3]+horz[i+w]]*hfac[i+w]+s[vert[j+h3]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h3]+horz[i+w3]]*hfac[i+w3])*vfac[j+h3])>>18];
          break;
        case 4: for (i=0; i<w; i++, d++)
          dest[d] = crop[((s[vert[j]+horz[i]]*hfac[i]+s[vert[j]+horz[i+w]]*hfac[i+w]+s[vert[j]+horz[i+w2]]*hfac[i+w2]+s[vert[j]+horz[i+w3]]*hfac[i+w3]+s[vert[j]+horz[i+w4]]*hfac[i+w4])*vfac[j]+
                    (s[vert[j+h]+horz[i]]*hfac[i]+s[vert[j+h]+horz[i+w]]*hfac[i+w]+s[vert[j+h]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h]+horz[i+w4]]*hfac[i+w4])*vfac[j+h]+
                    (s[vert[j+h2]+horz[i]]*hfac[i]+s[vert[j+h2]+horz[i+w]]*hfac[i+w]+s[vert[j+h2]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h2]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h2]+horz[i+w4]]*hfac[i+w4])*vfac[j+h2]+
                    (s[vert[j+h3]+horz[i]]*hfac[i]+s[vert[j+h3]+horz[i+w]]*hfac[i+w]+s[vert[j+h3]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h3]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h3]+horz[i+w4]]*hfac[i+w4])*vfac[j+h3]+
                    (s[vert[j+h4]+horz[i]]*hfac[i]+s[vert[j+h4]+horz[i+w]]*hfac[i+w]+s[vert[j+h4]+horz[i+w2]]*hfac[i+w2]+s[vert[j+h4]+horz[i+w3]]*hfac[i+w3]+s[vert[j+h4]+horz[i+w4]]*hfac[i+w4])*vfac[j+h4])>>18];
          break;
        case 5: for (i=0; i<w; i++, d++)
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

#ifdef LIBRARY
HBITMAPINFO ScaleBMP(HBITMAPINFO bmp, long neww, long newh, long color,
                     long inter)
/* Scale a bitmap to a new size, letterboxing it if needed.  If the source
 *  bitmap is a multiple of 4 pixels wide, and the destination bitmap is also
 *  a multiple of 4 pixels wide, and the source bitmap is uncompressed and 24
 *  bits per pixel, then a faster routine is invoked.
 * Enter: HBITMAPINFO bmp: handle to bitmap to scale.
 *        long neww, newh: size of the bitmap to create.  The output is
 *                         always this size, which may require letterboxing.
 *        long color: 0xBBGGRR color used for letterboxing, or -1 for non-
 *                    fixed aspect ratio.
 *        long inter: interpolation scheme.  0-none, 1-bilinear.
 * Exit:  HBITMAPINFO newbmp: handle to scaled bitmap.  Null for failed.
 *                                                             3/12/97-DWM */
{
  uchar *pic, *apic;
  long w, h, pal, color2, ret, i, w2, h2;
  LPBITMAPINFOHEADER b, b2;

  if (!bmp)  return(0);
  b = lock2(bmp);
  if (!(neww&3) && b->biSize==40)
    if (!(b->biWidth&3) && b->biWidth && b->biHeight && b->biPlanes==1 &&
        b->biSizeImage==b->biWidth*3*b->biHeight && b->biBitCount==24 &&
        color!=-1) {
      if (!(b2=malloc2(40+neww*newh*3))) {
        unlock2(b);  return(0); }
      memcpy(b2, b, 40);
      b2->biWidth = neww;  b2->biHeight = newh;  b2->biSizeImage = neww*newh*3;
      w = b->biWidth;  h = b->biHeight;
      w2 = neww;  h2 = h*neww/w;
      if (h2>newh || w*newh/h==w2) {
        h2 = newh;  w2 = w*newh/h;  if (w2>neww)  w2 = neww; }
      fill_zone24(((uchar *)b2)+40, color, neww*newh);
      Interpolate = inter;
      scale_pic24(((uchar *)b2)+40+(((newh-h2)/2)*neww+(neww-w2)/2)*3,
                  ((uchar *)b)+40, w, neww, w, w2, h, h2, color, -1); //
      Interpolate = 0;
      unlock2(b);
      return(unlock2(b2)); }
  unlock2(bmp);
  pic = bmp_to_graphic(bmp);
  if (!pic)  return(0);
  w = pwidth;  h = pheight;  pal = pinter;
  if (pal && (inter&GV_INTERPOLATEALL))
    for (i=0; i<256; i++)
      if (pic[i*3]!=i || pic[i*3+1]!=i || pic[i*3+2]!=i)  break;
  if (pinter && (inter&GV_INTERPOLATEALL) && i==256) {
    ppic = pic;  pfree = 1;
    ret = best_color(1, -1, -1);
    apic = ppic;
    if (ret<0)  return(0);
    pic = apic;  pal = 0; }
  color2 = (color>>16)+(color&0xFF00)+((color&0xFF)<<16);
  Interpolate = inter;
  if (color==-1)  color2 = -1;
  apic = scale_graphic(w, h, pal, pic, neww, newh, color2);
  Interpolate = 0;
  free2(pic);
  if (!apic)
    return(0);
  bmp = graphic_to_bmp(neww, newh, pal, apic, 0);
  free2(apic);
  return(bmp);
}
#endif

#ifdef LIBRARY
HANDLE ScaleGraphic(long w, long h, long pal, HANDLE hpic, long neww,
                    long newh, long color, long inter)
/* Scale a graphic image to a new size, letterboxing it if needed.
 * Enter: long w, h: size of original graphic.
 *        long pal: palettization of original graphic.  The output is this
 *                  same palettization.
 *        HANDLE hpic: handle to memory containing the graphic.
 *        long neww, newh: size of the graphic to create.  The output is
 *                         always this size, which may require letterboxing.
 *        long color: 0xRRGGBB color used for letterboxing, or -1 for non-
 *                    fixed aspect ratio.
 *        long inter: interpolation scheme.  0-none, 1-bilinear.
 * Exit:  HANDLE newhpic: handle to scaled graphic.  Null for failed.
 *                                                             3/12/97-DWM */
{
  char *pic, *pic2;
  long fr=0, ret, i;

  if (!hpic)  return(0);
  pic = lock2(hpic);
  if (pal && (inter&GV_INTERPOLATEALL))
    for (i=0; i<256; i++)
      if (pic[i*3]!=i || pic[i*3+1]!=i || pic[i*3+2]!=i)  break;
  if (pal && (inter&GV_INTERPOLATEALL) && i==256) {
    ppic = pic;  pfree = 0;  pwidth = w;  pheight = h;  pinter = pal;
    ret = best_color(1, -1, -1);
    pic2 = ppic;
    if (ret<0)  return(0);
    unlock2(pic);
    pic = pic2;  pal = 0;  fr = 1; }
  Interpolate = inter;
  pic2 = scale_graphic(w, h, pal, pic, neww, newh, color);
  Interpolate = 0;
  if (!fr)  unlock2(pic);
  else      free2(pic);
  return(unlock2(pic2));
}
#endif
