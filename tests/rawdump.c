/* rawdump -- load a graphics file with the GV library and dump the raw
 * decoded data (width, height, palette flag, then palette+pixels or RGB)
 * to stdout or a file, for comparison against other decoders. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "gvlib.h"

int main(int argc, char **argv) {
  GV gv;
  HANDLE h;
  uchar *p;
  FILE *f;
  long n;

  if (argc < 3) {
    printf("usage: rawdump <file> <outfile>\n");
    return (2);
  }
  memset(&gv, 0, sizeof(GV));
  gv.name = argv[1];
  h = LoadGraphic(&gv);
  if (!h) {
    printf("LOAD ERROR %lx\n", gv.error);
    return (1);
  }
  p = GlobalLock(h);
  f = fopen(argv[2], "wb");
  fwrite(&gv.width, 4, 1, f);
  fwrite(&gv.height, 4, 1, f);
  fwrite(&gv.palette, 4, 1, f);
  n = gv.palette ? 768 + (long)gv.width * gv.height
                 : (long)gv.width * gv.height * 3;
  fwrite(p, 1, n, f);
  fclose(f);
  GlobalUnlock(h);
  GlobalFree(h);
  return (0);
}
