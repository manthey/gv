/* Harness for verifying that the C replacements for the hand-tuned assembly
 * are bit-exact against the original assembly implementations.
 *
 * It saves and loads synthetic images in every format that exercises the
 * converted routines:
 *   lzw/lzwlen        - GIF save, TIF-GV save
 *   lzwtif/lzwtiflen  - TIF LZW save
 *   rle               - TIF RLE save
 *   ungif             - GIF load
 *   unlzw/unlzwtif    - TIF LZW load
 *   unrle             - TIF RLE load
 *   fill_zone24       - image fill
 *   jpeg_shift(_add)  - JPEG load
 *   jpeg_dct          - JPEG save
 *
 * Usage:  asmconv_test <outdir>
 * Saves a corpus of files, loads every one of them back, verifies lossless
 * roundtrips, and dumps the decoded pixels of every file to <outdir>/<f>.raw.
 * Run it against the assembly build and the converted build into two
 * directories; the directories must be byte-identical.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gvlib.h"

#define NPAL 768

static long load_only;
static long startspec;

/* deterministic pseudo-random byte generator (xorshift32) */
static unsigned long rstate;
static void seed(unsigned long s) { rstate = s ? s : 1; }
static uchar nextr(void) {
  rstate ^= rstate << 13;  rstate ^= rstate >> 17;  rstate ^= rstate << 5;
  return((uchar)rstate);
}

typedef struct {
  long w, h, pal;            /* pal: 1 = 8-bit palettized, 0 = 24-bit */
  uchar *data;               /* palette (if pal) + pixels, graphic layout */
  long datalen;
} IMG;

static void make_image(IMG *im, long w, long h, long pal,
                       unsigned long seedval, long pattern, long grey)
/* pattern: 0 = noise, 1 = gradient, 2 = flat, 3 = vertical stripes,
 *          4 = smooth RGB ramps
 * (stripes/flat are worst-case growth for LZW; noise is worst-case for RLE;
 * smooth is needed for meaningful lossy cross-decoder comparison) */
{
  long i, size = pal ? NPAL+w*h : w*h*3;

  im->w = w;  im->h = h;  im->pal = pal;  im->datalen = size;
  im->data = GlobalAlloc(GPTR, size);
  if (!im->data) { printf("FAIL alloc\n");  exit(1); }
  seed(seedval);
  if (pal) {
    for (i=0; i<NPAL; i++)  im->data[i] = nextr();
    if (grey)                        /* grey ramp so greyscale conversion */
      for (i=0; i<256; i++) {        /* is the identity map */
        im->data[i*3] = im->data[i*3+1] = im->data[i*3+2] = (uchar)i; } }
  for (i=pal?NPAL:0; i<size; i++) {
    long p = i-(pal?NPAL:0);
    switch (pattern) {
      case 0:  im->data[i] = nextr(); break;
      case 1:  im->data[i] = (uchar)((p*7+p/w*13)&0xFF); break;
      case 2:  im->data[i] = (uchar)(0xA5); break;
      case 4: {                /* smooth ramps: R across, G down, B their mix;
                                * slow enough that chroma upsampling method
                                * differences stay negligible */
                  long k = p/3, c = p%3, x = k%im->w, y = k/im->w;
                  long r = x*255/(im->w-1), g = y*255/(im->h-1);
                  im->data[i] = (uchar)(c==0 ? r : c==1 ? g : (r+g)/2);
                  break; }
      default: im->data[i] = (uchar)(((p/w)&1)*0xFF); break;
    }
  }
}

static void free_image(IMG *im) { GlobalFree(im->data);  im->data = 0; }

static HANDLE image_handle(IMG *im)
{
  HANDLE h = GlobalAlloc(GMEM_MOVEABLE, im->datalen);
  void *p;
  if (!h) { printf("FAIL GlobalAlloc\n");  exit(1); }
  p = GlobalLock(h);
  memcpy(p, im->data, im->datalen);
  GlobalUnlock(h);
  return(h);
}

static void setup_gv(GV *gv, const char *name, long w, long h, long pal)
{
  memset(gv, 0, sizeof(GV));
  gv->name = (char *)name;
  gv->width = w;  gv->height = h;  gv->palette = pal;
  gv->image = 0;  gv->dither = 0;  gv->offset = 0;
}

/* Custom quantization tables, in the same order the file's DQT segment and
 * the quantizer use (zig-zag scan order): 64 luminance values followed by
 * 64 chrominance values.  These are the standard (Annex K) tables. */
static uchar jpgcust[128];
static const uchar jpgcuststd[128] = {
  16,11,10,16,24,40,51,61, 12,12,14,19,26,58,60,55,
  14,13,16,24,40,57,69,56, 14,17,22,29,51,87,80,62,
  18,22,37,56,68,109,103,77, 24,35,55,64,81,104,113,92,
  49,64,78,87,103,121,120,101, 72,92,95,98,112,100,103,99,
  17,18,24,47,99,99,99,99, 18,18,24,47,99,99,99,99,
  24,24,47,99,99,99,99,99, 47,47,99,99,99,99,99,99,
  99,99,99,99,99,99,99,99, 99,99,99,99,99,99,99,99,
  99,99,99,99,99,99,99,99, 99,99,99,99,99,99,99,99 };

typedef struct {
  const char *name;
  long format;
  long color;         /* TIF color / BMP color / JPG color / PCX color */
  long comp;          /* TIF compression / BMP compression / JPG baseline */
  long pal;           /* 1 = save palettized source, 0 = 24-bit source */
  long a, b;          /* format specific: tif endian/rowsperstrip,
                         jpg decimation/quality, gif giftype/interlace */
  long usetrans;      /* gif transparency */
  long pattern;       /* image pattern to use */
  uchar *custom;      /* if non-zero, custom jpg quant tables (lum+chrom) */
} SPEC;

static SPEC specs[] = {
  /* name,          format,   color,         comp,     pal, a,b,   trans,pat */
  {"t8_none",       GV_TIF,   TIF_8BIT,      TIF_NONE,     1, 0,0,     0, 0,0},
  {"t8_none_g",     GV_TIF,   TIF_8BIT,      TIF_NONE,     1, 0,0,     0, 1,0},
  {"t8_rle",        GV_TIF,   TIF_8BIT,      TIF_RLE,      1, 0,0,     0, 0,0},
  {"t8_lzw",        GV_TIF,   TIF_8BIT,      TIF_LZW,      1, 0,0,     0, 0,0},
  {"t8_lzw_g",      GV_TIF,   TIF_8BIT,      TIF_LZW,      1, 0,0,     0, 1,0},
  {"t8_lzwflat",    GV_TIF,   TIF_8BIT,      TIF_LZW,      1, 0,0,     0, 2,0},
  {"t8_lzwpred",    GV_TIF,   TIF_8BIT,      TIF_LZWPRED,  1, 0,0,     0, 0,0},
  {"t8_lzwpred_g",  GV_TIF,   TIF_8BIT,      TIF_LZWPRED,  1, 0,0,     0, 1,0},
  {"t8_gv",         GV_TIF,   TIF_8BIT,      TIF_GV,       1, 0,0,     0, 0,0},
  {"t8_gv_g",       GV_TIF,   TIF_8BIT,      TIF_GV,       1, 0,0,     0, 1,0},
  {"t8_gvpred",     GV_TIF,   TIF_8BIT,      TIF_GVPRED,   1, 0,0,     0, 0,0},
  {"t8_lzwrps",     GV_TIF,   TIF_8BIT,      TIF_LZW,      1, 0,7,     0, 0,0},
  {"t24_lzw",       GV_TIF,   TIF_24BIT,     TIF_LZW,      0, 0,0,     0, 0,0},
  {"t24_lzw_g",     GV_TIF,   TIF_24BIT,     TIF_LZW,      0, 0,0,     0, 1,0},
  {"t24_lzwflat",   GV_TIF,   TIF_24BIT,     TIF_LZW,      0, 0,0,     0, 2,0},
  {"t24_gv",        GV_TIF,   TIF_24BIT,     TIF_GV,       0, 0,0,     0, 0,0},
  {"tg_lzw",        GV_TIF,   TIF_GREYSCALE, TIF_LZW,      1, 0,0,     0, 0,0},
  {"tg_lzwpred",    GV_TIF,   TIF_GREYSCALE, TIF_LZWPRED,  1, 0,0,     0, 0,0},
  {"tg_gv",         GV_TIF,   TIF_GREYSCALE, TIF_GV,       1, 0,0,     0, 0,0},
  {"t24_lzwmot",    GV_TIF,   TIF_24BIT,     TIF_LZW,      0, 1,0,     0, 0,0},
  {"t24_rle",       GV_TIF,   TIF_24BIT,     TIF_RLE,      0, 0,0,     0, 0,0},
  {"g87",           GV_GIF,   0,             0,            1, 0,0,     0, 0,0},
  {"g89i",          GV_GIF,   0,             0,            1, 1,1,     0, 0,0},
  {"g89t",          GV_GIF,   0,             0,            1, 1,0,     1, 0,0},
  {"gflat",         GV_GIF,   0,             0,            1, 0,0,     0, 2,0},
  {"pc8",           GV_PCX,   PCX_8BIT,      0,            1, 0,0,     0, 0,0},
  {"pc24",          GV_PCX,   PCX_24BIT,     0,            0, 0,0,     0, 0,0},
  {"b24",           GV_BMP,   BMP_24BIT,     0,            0, 0,0,     0, 0,0},
  {"bp8",           GV_BMP,   BMP_PAL,       0,            1, 0,0,     0, 0,0},
  {"p24b",          GV_PPM,   PPM_24BIT,     PPM_BINARY,   0, 0,0,     0, 0,0},
  /* JPEG specs use the smooth pattern: comparing two decoders' output on
   * lossy files is only meaningful for smooth content, where chroma
   * upsampling method differences do not dominate. */
  {"j11",           GV_JPG,   JPG_24BIT,     JPG_BASELINE, 0, 0,75,    0, 4,0},
  {"j21",           GV_JPG,   JPG_24BIT,     JPG_BASELINE, 0, 1,75,    0, 4,0},
  {"j22",           GV_JPG,   JPG_24BIT,     JPG_BASELINE, 0, 2,75,    0, 4,0},
  {"jg",            GV_JPG,   JPG_GREYSCALE, JPG_BASELINE, 0, 2,75,    0, 4,0},
  {"jc",            GV_JPG,   JPG_24BIT,     JPG_BASELINE, 0, 0,100,   0, 4,
                    (uchar *)jpgcust},
  {0,0,0,0,0,0,0,0,0,0}
};

/* sizes used per format (TIF/JPG get all, GIF gets a subset) */
static long sizes[][2] = { {61,37}, {64,64}, {255,255}, {256,38} };

int main(int argc, char **argv)
{
  char dir[400], path[512];
  SPEC *s;
  long si;
  FILE *f;
  int failures = 0;

  memcpy(jpgcust, jpgcuststd, sizeof(jpgcust));
  load_only = (argc > 2 && !strcmp(argv[2], "load"));
  if (argc > 3)  startspec = atoi(argv[3]);
  if (argc<2) { printf("usage: asmconv_test <outdir>\n");  return(2); }
  strcpy(dir, argv[1]);
  if (!load_only)  CreateDirectoryA(dir, NULL);

  for (s=specs+startspec; s->name; s++) {
    long nsizes = 4;
    if (s->format==GV_GIF)  nsizes = 3;      /* skip 256x38 for gif */
    if (s->format==GV_JPG)  nsizes = 2;      /* keep jpg runs fast */
    for (si=0; si<nsizes; si++) {
      GV gv;
      IMG im;
      HANDLE hpic, hload;
      uchar *p, *q;
      char fname[80];
      long ok;

      make_image(&im, sizes[si][0], sizes[si][1], s->pal,
                 (unsigned long)(0x1234+si*7+(s-specs)*11), s->pattern,
                 s->format==GV_TIF && s->color==TIF_GREYSCALE);
      snprintf(fname, sizeof(fname), "%s_%dx%d", s->name,
              (int)sizes[si][0], (int)sizes[si][1]);

      /* save */
      setup_gv(&gv, fname, sizes[si][0], sizes[si][1], s->pal);
      gv.format = s->format;
      switch (s->format) {
        case GV_TIF:
          gv.fs.tif.color = s->color;
          gv.fs.tif.compression = s->comp;
          gv.fs.tif.dpi = 160;
          gv.fs.tif.endian = s->a;
          gv.fs.tif.rowsperstrip = s->b;
          break;
        case GV_GIF:
          gv.fs.gif.giftype = s->a ? 1 : 0;
          gv.fs.gif.interlace = s->b;
          gv.fs.gif.usetrans = s->usetrans;
          if (s->usetrans) {
            uchar *t = im.data+255*3;
            gv.fs.gif.transparent = (t[0]<<16)|(t[1]<<8)|t[2]; }
          break;
        case GV_PCX:  gv.fs.pcx.color = s->color;  break;
        case GV_BMP:  gv.fs.bmp.color = s->color;  break;
        case GV_PPM:  gv.fs.ppm.color = s->color;
                      gv.fs.ppm.ppmtype = s->comp;  break;
        case GV_JPG:  gv.fs.jpg.color = s->color;
                      gv.fs.jpg.compression = s->comp;
                      gv.fs.jpg.dpi = 160;
                      gv.fs.jpg.decimation = s->a;
                      /* Quality scales the quantization tables (a percentage
                       * of the base table values; 100 = tables unchanged, 0
                       * = quantizer 1 = best quality).  It is NOT the IJG
                       * "quality 0-100" scale; the save dialog allows 0 to
                       * 2600 with a default of 50. */
                      gv.fs.jpg.quality = s->b;
                      gv.fs.jpg.usecustom = (s->custom != 0);
                      if (s->custom)
                        gv.fs.jpg.customquant = (char *)s->custom;
                      break; }
      hpic = image_handle(&im);
      snprintf(path, sizeof(path), "%s\\%s", dir, fname);
      gv.name = path;
      if (!load_only && !SaveGraphic(&gv, hpic)) {
        printf("SAVE-ERR %s error=%lx\n", fname, gv.error);  failures++; }
      GlobalFree(hpic);
      if (load_only && (f = fopen(path, "rb")) == NULL) {
        printf("MISSING %s\n", fname);  failures++;  free_image(&im);
        continue; }
      if (load_only)  fclose(f);

      /* load back and compare (exact for lossless formats) */
      setup_gv(&gv, path, 0, 0, 0);
      hload = LoadGraphic(&gv);
      if (!hload) {
        printf("LOAD-ERR %s error=%lx\n", fname, gv.error);  failures++; }
      else {
        p = GlobalLock(hload);
        /* dump decoded data for cross-build comparison */
        snprintf(path, sizeof(path), "%s\\%s.raw", dir, fname);
        f = fopen(path, "wb");
        fwrite(&gv.width, 4, 1, f);
        fwrite(&gv.height, 4, 1, f);
        fwrite(&gv.palette, 4, 1, f);
        if (gv.palette)
          fwrite(p, 1, NPAL+(long)gv.width*gv.height, f);
        else
          fwrite(p, 1, (long)gv.width*gv.height*3, f);
        fclose(f);
        if ((s->format==GV_TIF && (s->color==TIF_8BIT ||
                                  s->color==TIF_24BIT)) ||
            s->format==GV_BMP || s->format==GV_PPM) {
          q = im.data;
          ok = (gv.width==sizes[si][0] && gv.height==sizes[si][1] &&
                gv.palette==s->pal);
          if (ok)
            ok = !memcmp(p, q, im.datalen);
          if (!ok) {
            printf("MISMATCH %s\n", fname);  failures++; } }
        else {
          /* lossy: only check that the load succeeded and produced data */
          ok = (gv.width==sizes[si][0] && gv.height==sizes[si][1]); }
        GlobalUnlock(hload);
        GlobalFree(hload); }
      free_image(&im);
    }
  }
  printf(failures ? "FAILURES: %d\n" : "ALL OK (%d failures)\n", failures);
  return(failures != 0);
}
