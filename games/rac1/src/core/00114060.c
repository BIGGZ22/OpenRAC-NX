#include "common.h"
#include "structs.h"

/*
 * core_text object 0x114060-0x1144D0. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib (the SDK's libc.a): fvwrite.o (__sfvwrite) and fwalk.o
 * (_fwalk), back to back. Built with Sony's 2.9-ee (Makefile.sn,
 * EE29_CORE), like libc.a.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);

/* No user fns here.  Pesch 15apr92. */

/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* newlib's __sfvwrite, from its own source. Adapted from Lombyte (MIT)
   for PAL. */
/* newlib's FILE and uio, as __sfvwrite uses them. */
struct __sbuf_fvw {
    unsigned char *_base;
    int _size;
};
typedef struct {
    unsigned char *_p;          /* 0x00 */
    int _r;
    int _w;                     /* 0x08 */
    short _flags;               /* 0x0C */
    short _file;
    struct __sbuf_fvw _bf;      /* 0x10 */
    int _lbfsize;
    void *_cookie;              /* 0x1C */
    int (*_read)(void *, char *, int);
    int (*_write)(void *, const char *, int);   /* 0x24 */
} FILE_fvw;
struct __siov {
    const void *iov_base;
    unsigned int iov_len;
};
struct __suio {
    struct __siov *uio_iov;
    int uio_iovcnt;
    int uio_resid;
};

extern void *func_001150D4(const void *, int, unsigned int);   /* memchr */
extern void *func_001152F8(void *, const void *, unsigned int); /* memmove */
extern int func_00118928(FILE_fvw *);                           /* __swsetup */

#define _CONST const
#define FILE FILE_fvw
#define size_t unsigned int
#define BUFSIZ 1024
#define EOF (-1)
#define __SNBF 2
#define __SLBF 1
#define __SSTR 0x200
#define __SERR 0x40
#define memmove func_001152F8
#define memchr func_001150D4
#define fflush(fp) func_00113968_f(fp)
#define __swsetup func_00118928
#define cantwrite(fp) ((((fp)->_flags & 8) == 0 || (fp)->_bf._base == 0) && __swsetup(fp))

extern int func_00113968_f(FILE_fvw *) __asm__("func_00113968");

#define	MIN(a, b) ((a) < (b) ? (a) : (b))
#define	COPY(n)	  (void) memmove((void *) fp->_p, (void *) p, (size_t) (n))

#define GETIOV(extra_work) \
  while (len == 0) \
    { \
      extra_work; \
      p = iov->iov_base; \
      len = iov->iov_len; \
      iov++; \
    }

/*
 * Write some memory regions.  Return zero on success, EOF on error.
 *
 * This routine is large and unsightly, but most of the ugliness due
 * to the three different kinds of output buffering is handled here.
 */

int func_00114060(FILE *fp, struct __suio *uio)
{
  register size_t len;
  register _CONST char *p;
  register struct __siov *iov;
  register int w, s;
  char *nl;
  int nlknown, nldist;

  if ((len = uio->uio_resid) == 0)
    return 0;

  /* make sure we can write */
  if (cantwrite (fp))
    return EOF;

  iov = uio->uio_iov;
  len = 0;
  if (fp->_flags & __SNBF)
    {
      /*
       * Unbuffered: write up to BUFSIZ bytes at a time.
       */
      do
	{
	  GETIOV (;);
	  w = (*fp->_write) (fp->_cookie, p, MIN (len, BUFSIZ));
	  if (w <= 0)
	    goto err;
	  p += w;
	  len -= w;
	}
      while ((uio->uio_resid -= w) != 0);
    }
  else if ((fp->_flags & __SLBF) == 0)
    {
      /*
       * Fully buffered: fill partially full buffer, if any,
       * and then flush.  If there is no partial buffer, write
       * one _bf._size byte chunk directly (without copying).
       *
       * String output is a special case: write as many bytes
       * as fit, but pretend we wrote everything.  This makes
       * snprintf() return the number of bytes needed, rather
       * than the number used, and avoids its write function
       * (so that the write function can be invalid).
       */
      do
	{
	  GETIOV (;);
	  w = fp->_w;
	  if (fp->_flags & __SSTR)
	    {
	      if (len < w)
		w = len;
	      COPY (w);		/* copy MIN(fp->_w,len), */
	      fp->_w -= w;
	      fp->_p += w;
	      w = len;		/* but pretend copied all */
	    }
	  else if (fp->_p > fp->_bf._base && len > w)
	    {
	      /* fill and flush */
	      COPY (w);
	      /* fp->_w -= w; *//* unneeded */
	      fp->_p += w;
	      if (fflush (fp))
		goto err;
	    }
	  else if (len >= (w = fp->_bf._size))
	    {
	      /* write directly */
	      w = (*fp->_write) (fp->_cookie, p, w);
	      if (w <= 0)
		goto err;
	    }
	  else
	    {
	      /* fill and done */
	      w = len;
	      COPY (w);
	      fp->_w -= w;
	      fp->_p += w;
	    }
	  p += w;
	  len -= w;
	}
      while ((uio->uio_resid -= w) != 0);
    }
  else
    {
      /*
       * Line buffered: like fully buffered, but we
       * must check for newlines.  Compute the distance
       * to the first newline (including the newline),
       * or `infinity' if there is none, then pretend
       * that the amount to write is MIN(len,nldist).
       */
      nlknown = 0;
      do
	{
	  GETIOV (nlknown = 0);
	  if (!nlknown)
	    {
	      nl = memchr ((void *) p, '\n', len);
	      nldist = nl ? nl + 1 - p : len + 1;
	      nlknown = 1;
	    }
	  s = MIN (len, nldist);
	  w = fp->_w + fp->_bf._size;
	  if (fp->_p > fp->_bf._base && s > w)
	    {
	      COPY (w);
	      /* fp->_w -= w; */
	      fp->_p += w;
	      if (fflush (fp))
		goto err;
	    }
	  else if (s >= (w = fp->_bf._size))
	    {
	      w = (*fp->_write) (fp->_cookie, p, w);
	      if (w <= 0)
		goto err;
	    }
	  else
	    {
	      w = s;
	      COPY (w);
	      fp->_w -= w;
	      fp->_p += w;
	    }
	  if ((nldist -= w) == 0)
	    {
	      /* copied the newline: flush and forget */
	      if (fflush (fp))
		goto err;
	      nlknown = 0;
	    }
	  p += w;
	  len -= w;
	}
      while ((uio->uio_resid -= w) != 0);
    }
  return 0;

err:
  fp->_flags |= __SERR;
  return EOF;
}

#undef _CONST
#undef FILE
#undef size_t
#undef BUFSIZ
#undef EOF
#undef __SNBF
#undef __SLBF
#undef __SSTR
#undef __SERR
#undef memmove
#undef memchr
#undef fflush
#undef __swsetup
#undef cantwrite
#undef MIN
#undef COPY
#undef GETIOV

typedef struct { char pad0[0xC]; short _flags; char pad1[0x58 - 0xE]; } Fwalk_FILE;
typedef struct Fwalk_glue {
    struct Fwalk_glue *_next;
    int _niobs;
    Fwalk_FILE *_iobs;
} Fwalk_glue;
typedef struct { char pad[0x1D8]; Fwalk_glue __sglue; } Fwalk_reent;

/* newlib's _fwalk (findfp.c), verbatim: call function on every FILE in
   use (_flags != 0) in every glue block of ptr->__sglue (+0x1D8), or-ing
   the results. It returns int; the shared preamble's void declaration is
   gone from this file. */
int func_00114438(Fwalk_reent *ptr, int (*function)()) {
    register Fwalk_FILE *fp;
    register int n, ret = 0;
    register Fwalk_glue *g;

    for (g = &ptr->__sglue; g != 0; g = g->_next)
        for (fp = g->_iobs, n = g->_niobs; --n >= 0; fp++)
            if (fp->_flags != 0)
                ret |= (*function)(fp);
    return ret;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_001144CC);
