/* LZW compression and decompression for Graphic Viewer.
 *                                                    2026-DWM
 *
 * This is a plain C implementation of the LZW codec, replacing the
 * hand-written 32-bit assembly that used to live in gload.c and gsave.c.
 * The compressed data formats are unchanged.  Two bit-order variants exist,
 * exactly as in the original code:
 *
 *   - GIF style (msb is 0): codes are packed least-significant bit first.
 *     The code width grows when the next free code passes 2^codesize, and
 *     the width is capped at 12 bits, at which point the table is cleared
 *     with a clear code (the "deferred clear" of the GIF specification).
 *
 *   - TIFF style (msb is 1): codes are packed most-significant bit first,
 *     and the code width grows one code earlier (the TIFF "early change"
 *     convention).
 *
 * The string table maps a (prefix code, final byte) pair to a code.  Codes
 * 0 to 255 are the raw bytes, the clear code and end-of-data code follow,
 * and the remaining codes name strings built while coding.  Strings are
 * kept as a chain per prefix code: head[code] is the first node of the
 * chain of strings whose prefix is that code, and each node holds the
 * string's final byte and the next node of the chain.
 *
 * The tables are static.  They are written before they are read, and the
 * chain heads are zeroed whenever the table is reset, so no stale data is
 * ever used.                                                       */

#include <stdio.h>
#include <string.h>
#include "gload.h"

#ifndef LIBRARY
#define STATIC
#endif

#define LZW_MAXCODES    4096            /* codes are at most 12 bits */

typedef struct {
  ushort next;                          /* next node of the chain, or 0 */
  uchar  chr;                           /* final byte of the string */
  uchar  pad;
} LZWNODE;

static ushort lzw_head[LZW_MAXCODES];   /* chain head per prefix code */
static LZWNODE lzw_node[LZW_MAXCODES];  /* strings, indexed by code-first */
static ushort lzw_prefix[LZW_MAXCODES]; /* decoder: prefix code per string */
static uchar  lzw_suffix[LZW_MAXCODES]; /* decoder: final byte per string */
static uchar  lzw_stack[LZW_MAXCODES];  /* decoder: string reversal buffer */

long lzw(uchar *dest, uchar *source, long len, long maxlen, short numbits);
long lzwlen(uchar *source, long len, short numbits);
long lzwtif(uchar *dest, uchar *source, long len, long maxlen);
long lzwtiflen(uchar *source, long len);
void unlzw(uchar *dest, uchar *source, long maxlen, long srclen,
           short numbits);
void unlzwtif(uchar *dest, uchar *source, long maxlen, long srclen);

STATIC long lzw_encode(uchar *dest, uchar *source, long len, long maxlen,
                       short numbits, int msb)
/* LZW compress len bytes from source.  Codes are packed LSB first if msb is
 * 0 (GIF style) or MSB first if msb is 1 (TIFF style).  Returns the number
 * of bytes that the compressed data occupies, including the end-of-data
 * code and any final partial byte.  If dest is NULL, nothing is written and
 * the length that the data would have is returned.  Encoding stops early if
 * maxlen bytes would be exceeded (in which case the data is truncated, as
 * with the original code).                                        2026-DWM */
{
  ushort clearcode, eof, firstcode, freecode, maxcode, incode, code, *link;
  uchar codesize, initcodesize, c;
  ulong acc = 0;
  long accbits = 0, bits = 0, out = 0;

  if (len < 1)
    return(0);
  clearcode = (ushort)(1 << numbits);
  eof = (ushort)(clearcode+1);
  firstcode = (ushort)(clearcode+2);
  codesize = initcodesize = (uchar)(numbits+1);
  maxcode = (ushort)((1 << codesize) - msb);
  freecode = firstcode;
  memset(lzw_head, 0, sizeof(lzw_head));

/* LZW_EMIT appends codesize bits of a code to the output.  When dest is
 * NULL nothing is stored, only counted.  The macro is reentered with the
 * label done defined below. */
#define LZW_EMIT(code) \
    { \
      if (msb) { \
        acc = (acc << codesize) | (ulong)(code); \
        accbits += codesize;  bits += codesize; \
        while (accbits >= 8) { \
          if (dest) { \
            if (out >= maxlen)  goto done; \
            dest[out] = (uchar)(acc >> (accbits-8)); } \
          out++;  accbits -= 8; } \
      } else { \
        acc |= (ulong)(code) << accbits; \
        accbits += codesize;  bits += codesize; \
        while (accbits >= 8) { \
          if (dest) { \
            if (out >= maxlen)  goto done; \
            dest[out] = (uchar)acc; } \
          out++;  acc >>= 8;  accbits -= 8; } \
      } \
    }

  LZW_EMIT(clearcode)
  incode = *source++;
  len--;
  while (len > 0) {
    c = *source++;
    len--;
    /* find the string incode followed by c in the table */
    link = &lzw_head[incode];
    code = *link;
    while (code && lzw_node[code-firstcode].chr != c) {
      link = &lzw_node[code-firstcode].next;
      code = *link;
    }
    if (code) {
      incode = code;                /* the longer string is already known */
      continue;
    }
    LZW_EMIT(incode)                /* emit the code for the current string */
    lzw_node[freecode-firstcode].chr = c;    /* add incode+c to the table */
    lzw_node[freecode-firstcode].next = 0;
    *link = freecode;
    incode = c;
    freecode++;
    if (freecode > maxcode) {       /* the code width must grow */
      if (codesize < 12) {
        codesize++;
        maxcode = (ushort)((1 << codesize) - msb);
        if (codesize == 12)         /* deferred clear, as in the original */
          maxcode--;
      } else {                      /* the 12-bit table is full: clear it */
        LZW_EMIT(clearcode)
        codesize = initcodesize;
        maxcode = (ushort)((1 << codesize) - msb);
        freecode = firstcode;
        memset(lzw_head, 0, sizeof(lzw_head));
      }
    }
  }
  LZW_EMIT(incode)                  /* emit the code for the final string */
  LZW_EMIT(eof)
  if (dest && accbits > 0 && out < maxlen) {   /* flush any partial byte */
    if (msb)
      dest[out] = (uchar)(acc << (8-accbits));
    else
      dest[out] = (uchar)acc;
    out++;
  }
  if (!dest)
    out = (bits+7) >> 3;
  return(out);
done:
  return(out);
}

STATIC void lzw_decode(uchar *dest, uchar *source, long maxlen, long srclen,
                       short numbits, int msb)
/* LZW decompress source into dest, writing at most maxlen bytes.  Codes are
 * read LSB first if msb is 0 (GIF style) or MSB first if msb is 1 (TIFF
 * style).  Decoding stops at the end-of-data code, when the input runs out,
 * or when maxlen bytes have been written.  Invalid codes (greater than the
 * next free code) stop the decoding as well.                      2026-DWM */
{
  ushort clearcode, eof, firstcode, freecode, maxcode, oldcode, incode, code,
         codemask;
  uchar codesize, initcodesize, finchar;
  ulong acc = 0;
  long accbits = 0, in = 0, out = 0, sp;

  clearcode = (ushort)(1 << numbits);
  eof = (ushort)(clearcode+1);
  firstcode = (ushort)(clearcode+2);
  codesize = initcodesize = (uchar)(numbits+1);
  codemask = (ushort)((1 << codesize) - 1);
  maxcode = (ushort)((1 << codesize) - msb);
  freecode = firstcode;
  oldcode = 0;                      /* only used if the stream omits the
                                       initial clear code */
  finchar = 0;

/* LZW_GETCODE reads the next codesize-bit code into code, or jumps to done
 * if the input is exhausted. */
#define LZW_GETCODE() \
    { \
      while (accbits < codesize) { \
        if (in >= srclen)  goto done; \
        if (msb) { \
          acc = (acc << 8) | source[in]; \
        } else { \
          acc |= (ulong)source[in] << accbits; \
        } \
        accbits += 8; \
        in++; \
      } \
      if (msb) { \
        code = (ushort)((acc >> (accbits-codesize)) & codemask); \
        accbits -= codesize; \
        acc &= (1UL << accbits) - 1; \
      } else { \
        code = (ushort)(acc & codemask); \
        acc >>= codesize; \
        accbits -= codesize; \
      } \
    }

  for (;;) {
    LZW_GETCODE()
    if (code == eof)
      break;
    if (code == clearcode) {        /* reset the string table */
      codesize = initcodesize;
      codemask = (ushort)((1 << codesize) - 1);
      maxcode = (ushort)((1 << codesize) - msb);
      freecode = firstcode;
      LZW_GETCODE()                 /* the next code starts a new string */
      if (code == eof)
        break;
      oldcode = code;
      finchar = (uchar)(code & (clearcode-1));
      if (out >= maxlen)
        goto done;
      dest[out++] = finchar;
      continue;
    }
    if (code > freecode)            /* corrupt stream */
      break;
    incode = code;
    sp = 0;
    if (code == freecode) {         /* the "KwKwK" case: the code names the
                                       string being defined right now */
      lzw_stack[sp++] = finchar;
      code = oldcode;
    }
    while (code >= clearcode) {     /* walk the string back to its root */
      lzw_stack[sp++] = lzw_suffix[code];
      code = lzw_prefix[code];
    }
    finchar = (uchar)code;
    lzw_stack[sp++] = finchar;
    while (sp > 0) {                /* emit the string in forward order */
      if (out >= maxlen)
        goto done;
      dest[out++] = lzw_stack[--sp];
    }
    /* add the string oldcode followed by finchar to the table */
    lzw_suffix[freecode] = finchar;
    lzw_prefix[freecode] = oldcode;
    oldcode = incode;
    freecode++;
    if (freecode >= maxcode && codesize < 12) {
      codesize++;
      codemask = (ushort)((1 << codesize) - 1);
      maxcode = (ushort)((1 << codesize) - msb);
    }
  }
done:
  return;
}

long lzw(uchar *dest, uchar *source, long len, long maxlen, short numbits)
/* Compress data using LZW, GIF style (LSB first).  See lzw_encode. */
{
  return(lzw_encode(dest, source, len, maxlen, numbits, 0));
}

long lzwlen(uchar *source, long len, short numbits)
/* Compute the length that data compressed with lzw would have. */
{
  return(lzw_encode(0, source, len, 0x7FFFFFFF, numbits, 0));
}

long lzwtif(uchar *dest, uchar *source, long len, long maxlen)
/* Compress data using LZW, TIFF style (MSB first, early change). */
{
  return(lzw_encode(dest, source, len, maxlen, 8, 1));
}

long lzwtiflen(uchar *source, long len)
/* Compute the length that data compressed with lzwtif would have. */
{
  return(lzw_encode(0, source, len, 0x7FFFFFFF, 8, 1));
}

void unlzw(uchar *dest, uchar *source, long maxlen, long srclen,
           short numbits)
/* Decompress LZW data, GIF style (LSB first).  See lzw_decode. */
{
  lzw_decode(dest, source, maxlen, srclen, numbits, 0);
}

void unlzwtif(uchar *dest, uchar *source, long maxlen, long srclen)
/* Decompress LZW data, TIFF style (MSB first, early change). */
{
  lzw_decode(dest, source, maxlen, srclen, 8, 1);
}
