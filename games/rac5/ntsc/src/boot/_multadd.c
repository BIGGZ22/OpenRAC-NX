/* _multadd -- RAC5 boot body, w11/s1.
 *
 * Provenance: the old BSD/newlib dtoa.c "multiply by m and add a" routine
 * (David M. Gay; the revision whose Bigint is _next/_k/_maxwds/_sign/_wds/_x
 * and whose internals keep the leading underscore). No copy of that revision
 * exists in the RAC1/RAC2 corpora, so the body is reconstructed from the retail
 * disassembly of 0x01ECF898..0x01ECF9A0: 16-bit half-word multiply, carry in
 * the 4th argument, _Balloc(_k+1)+memcpy(wds+2 words from _sign)+_Bfree on
 * growth. Retail calls the plain memcpy at 0x01EC614C, not memcpyalpha.
 * Retail: 0x01ECF898, interval 264 bytes. Toolchain: SN ProDG 3.01 ee-gcc2953
 * (GCC 2.95.3) -O2 -G0, Ps2EeAs.exe, SN ld.exe.
 */
struct _reent;

typedef struct _Bigint {
  struct _Bigint *_next;
  int _k;
  int _maxwds;
  int _sign;
  int _wds;
  unsigned int _x[1];
} Bigint;

extern Bigint *_Balloc (struct _reent *, int);
extern void _Bfree (struct _reent *, Bigint *);
extern void *memcpy (void *, const void *, unsigned int);

#define __ULong unsigned int

Bigint *
_multadd (struct _reent *ptr, Bigint *b, int m, int a)
{
  int i, wds, c;
  __ULong *x, y;
  __ULong xi, z;
  Bigint *b1;

  wds = b->_wds;
  x = b->_x;
  i = 0;
  do
    {
      xi = *x;
      y = (xi & 0xffff) * m + a;
      xi >>= 16;
      c = (int) (y >> 16);
      z = (xi * m) + c;
      *x++ = (z << 16) + (y & 0xffff);
      c = (int) (z >> 16);
      a = c;
    }
  while (++i < wds);
  if (a)
    {
      if (wds >= b->_maxwds)
	{
	  b1 = _Balloc (ptr, b->_k + 1);
	  memcpy (&b1->_sign, &b->_sign, b->_wds * sizeof (__ULong) + 2 * sizeof (__ULong));
	  _Bfree (ptr, b);
	  b = b1;
	}
      b->_x[wds++] = a;
      b->_wds = wds;
    }
  return b;
}
