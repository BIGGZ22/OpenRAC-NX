/* fread -- RAC5 boot body, w11/s1.
 *
 * Provenance: newlib libc/stdio/fread.c (Copyright (c) 1990 The Regents of the
 * University of California), from the EE newlib snapshot in
 * D:/RAC2/master/build/sdk-compiler-build/source/ee/newlib/libc/stdio/fread.c.
 * The body is that source verbatim; only the declarations it pulls from
 * <stdio.h>/<string.h>/local.h are restated here, because the campaign's
 * prodg2953 driver runs its preprocessor without an -I path. FILE is the newlib
 * __sFILE prefix actually touched: _p at offset 0, _r at offset 4.
 * Retail: 0x01ECF508, 280 bytes. Measured with SN ProDG 3.01 ee-gcc2953
 * (GCC 2.95.3) -O2 -G0, Ps2EeAs.exe, SN ld.exe.
 */
typedef unsigned int size_t;

typedef struct __sFILE {
  unsigned char *_p;
  int _r;
} FILE;

extern void *memcpyalpha(void *, const void *, size_t);
extern int __srefill(FILE *);

size_t
fread (void *buf, size_t size, size_t count, FILE *fp)
{
  register size_t resid;
  register char *p;
  register int r;
  size_t total;

  if ((resid = count * size) == 0)
    return 0;
  if (fp->_r < 0)
    fp->_r = 0;
  total = resid;
  p = buf;
  while (resid > (r = fp->_r))
    {
      (void) memcpyalpha ((void *) p, (void *) fp->_p, (size_t) r);
      fp->_p += r;
      /* fp->_r = 0 ... done in __srefill */
      p += r;
      resid -= r;
      if (__srefill (fp))
	{
	  /* no more input: return partial result */
	  return (total - resid) / size;
	}
    }
  (void) memcpyalpha ((void *) p, (void *) fp->_p, resid);
  fp->_r -= resid;
  fp->_p += resid;
  return count;
}
