/* fflush -- RAC5 boot body, w11/s1.
 *
 * Provenance: newlib libc/stdio/fflush.c (Copyright (c) 1990 The Regents of the
 * University of California), EE newlib snapshot
 * D:/RAC2/master/build/sdk-compiler-build/source/ee/newlib/libc/stdio/fflush.c.
 * Body verbatim; the FILE/_reent prefixes and the CHECK_INIT macro it uses are
 * restated from sys/reent.h and stdio/local.h of the same snapshot, with the
 * offsets retail reads: _flags 0x0C, _bf._base 0x10, _data 0x54, __sdidinit
 * 0x38. Retail: 0x01ECF288, 376 bytes. Toolchain: SN ProDG 3.01 ee-gcc2953
 * (GCC 2.95.3) -O2 -G0, Ps2EeAs.exe, SN ld.exe.
 */
typedef unsigned int size_t;

struct __sbuf {
  unsigned char *_base;
  int _size;
};

typedef struct __sFILE {
  unsigned char *_p;
  int _r;
  int _w;
  short _flags;
  short _file;
  struct __sbuf _bf;
  int _lbfsize;
  void *_cookie;
  int (*_read) (void *, char *, int);
  int (*_write) (void *, const char *, int);
  long (*_seek) (void *, long, int);
  int (*_close) (void *);
  struct __sbuf _ub;
  unsigned char *_up;
  int _ur;
  unsigned char _ubuf[3];
  unsigned char _nbuf[1];
  struct __sbuf _lb;
  int _blksize;
  int _offset;
  struct _reent *_data;
} FILE;

struct _reent {
  int _errno;
  FILE *_stdin;
  FILE *_stdout;
  FILE *_stderr;
  int _inc;
  char _emergency[25];
  int _current_category;
  const char *_current_locale;
  int __sdidinit;
};

extern struct _reent *_impure_ptr;
extern void __sinit (struct _reent *);
extern int _fwalk (struct _reent *, int (*) (FILE *));

#define _REENT _impure_ptr
#define CHECK_INIT(fp) \
  do \
    { \
      if ((fp)->_data == 0) \
	(fp)->_data = _REENT; \
      if (!(fp)->_data->__sdidinit) \
	__sinit ((fp)->_data); \
    } \
  while (0)

#define NULL ((void *) 0)
#define __SLBF 0x0001
#define __SNBF 0x0002
#define __SWR  0x0008
#define __SERR 0x0040
#define EOF (-1)

int
fflush (FILE *fp)
{
  register unsigned char *p;
  register int n, t;

  if (fp == NULL)
    return _fwalk (_REENT, fflush);

  CHECK_INIT (fp);

  t = fp->_flags;
  if ((t & __SWR) == 0 || (p = fp->_bf._base) == NULL)
    return 0;
  n = fp->_p - p;		/* write this much */

  /*
   * Set these immediately to avoid problems with longjmp
   * and to allow exchange buffering (via setvbuf) in user
   * write function.
   */
  fp->_p = p;
  fp->_w = t & (__SLBF | __SNBF) ? 0 : fp->_bf._size;

  while (n > 0)
    {
      t = (*fp->_write) (fp->_cookie, (char *) p, n);
      if (t <= 0)
	{
	  fp->_flags |= __SERR;
	  return EOF;
	}
      p += t;
      n -= t;
    }
  return 0;
}
