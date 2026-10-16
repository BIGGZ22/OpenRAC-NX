#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x116FA0-0x1188C8. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib's vfprintf.c (the SDK's libc.a member vfprintf.o): __sprint,
 * __sbprintf, vfprintf, _vfprintf_r, cvt and exponent. Built with Sony's
 * 2.9-ee (Makefile.sn, EE29_CORE), like libc.a; newlib's own text for
 * each function compiles to retail under it.
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
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");
extern int func_00112468(int *errOut, int arg1);

extern int func_00114060(int, void *);

/* newlib's stdio types (sys/reent.h, 1999-2000), under local names. */
struct __sbuf_16FA0 {
    unsigned char *_base;
    int _size;
};
struct _reent_16FA0 {
    char _pad[0x38];
    int __sdidinit;             /* 0x38 */
};
typedef struct {
    unsigned char *_p;          /* 0x00 */
    int _r;
    int _w;
    short _flags;               /* 0x0C */
    short _file;
    struct __sbuf_16FA0 _bf;    /* 0x10 */
    int _lbfsize;
    void *_cookie;              /* 0x1C */
    int (*_read)(void *, char *, int);
    int (*_write)(void *, const char *, int);
    long (*_seek)(void *, long, int);
    int (*_close)(void *);
    struct __sbuf_16FA0 _ub;    /* 0x30 */
    unsigned char *_up;
    int _ur;
    unsigned char _ubuf[3];
    unsigned char _nbuf[1];
    struct __sbuf_16FA0 _lb;    /* 0x44 */
    int _blksize;
    int _offset;
    struct _reent_16FA0 *_data; /* 0x54 */
} FILE_16FA0;
struct __siov_16FA0 {
    void *iov_base;
    int iov_len;
};
struct __suio_16FA0 {
    struct __siov_16FA0 *uio_iov;
    int uio_iovcnt;
    int uio_resid;
};

/* __sprint, newlib's text: flush the uio through __sfvwrite
   (func_00114060) if it holds anything. */
int func_00116FA0(FILE_16FA0 *fp, struct __suio_16FA0 *uio) {
    register int err;

    if (uio->uio_resid == 0) {
        uio->uio_iovcnt = 0;
        return (0);
    }
    err = func_00114060((int)fp, uio);
    uio->uio_resid = 0;
    uio->uio_iovcnt = 0;
    return (err);
}

/* vfprintf and fflush return int; the declarations in scope above say
   void. */
extern int func_001170A0_f(FILE_16FA0 *, const char *, void *) __asm__("func_001170A0");
extern int func_00113968_f(FILE_16FA0 *) __asm__("func_00113968");

/* __sbprintf, newlib's text: printf to an unbuffered file through a
   temporary 1 KB buffer (BUFSIZ), then copy the error status back. */
int func_00116FE8(FILE_16FA0 *fp, const char *fmt, void *ap) {
    int ret;
    FILE_16FA0 fake;
    unsigned char buf[1024];

    /* copy the important variables */
    fake._data = fp->_data;
    fake._flags = fp->_flags & ~0x0002; /* __SNBF */
    fake._file = fp->_file;
    fake._cookie = fp->_cookie;
    fake._write = fp->_write;

    /* set up the buffer */
    fake._bf._base = fake._p = buf;
    fake._bf._size = fake._w = sizeof(buf);
    fake._lbfsize = 0;  /* not actually used, but Just In Case */

    /* do the work, then copy any error status */
    ret = func_001170A0_f(&fake, fmt, ap);
    if (ret >= 0 && func_00113968_f(&fake))
        ret = -1;       /* EOF */
    if (fake._flags & 0x0040)   /* __SERR */
        fp->_flags |= 0x0040;
    return (ret);
}

extern void func_00113AE0(void *);
extern int func_00117118_i(struct _reent_16FA0 *, FILE_16FA0 *, const char *, void *)
    __asm__("func_00117118");

/*
 * vfprintf, newlib's text: CHECK_INIT (fault in the reent from
 * _impure_ptr, D_0012F86C, and run __sinit, func_00113AE0, if it has not
 * been), then return _vfprintf_r's result.
 *
 * CHECK_INIT has to stay the macro's `do { ... } while (0)`: its loop
 * notes are what leave retail's `nop` after the _data store and give the
 * argument registers retail's $17/$18 roles. Without the wrapper 2.9-ee
 * is one instruction short (116/120). (Under 2.95.3 the old field-access
 * spelling was the right size but 20/120, with $17/$18 swapped.) It
 * returns the callee's value, so 2.9-ee keeps the call.
 */
int func_001170A0(FILE_16FA0 *fp, const char *fmt0, void *ap) {
    do {
        if ((fp)->_data == 0)
            (fp)->_data = (struct _reent_16FA0 *)D_0012F86C;
        if (!(fp)->_data->__sdidinit)
            func_00113AE0((fp)->_data);
    } while (0);
    return func_00117118_i(fp->_data, fp, fmt0, ap);
}

extern struct _reent_16FA0 *vfprintf_reent_117118[] __asm__("D_0012F86C") __attribute__((section(".data")));
extern s32 vfprintf_mbmax_117118[] __asm__("D_0012F870") __attribute__((section(".data")));
extern const char vfprintf_blanks_117118[] __asm__("D_001525E0");
extern const char vfprintf_zeroes_117118[] __asm__("D_001525F0");
extern char **localeconv(void) __asm__("func_001144F0");
extern s32 _mbtowc_r(struct _reent_16FA0 *, s32 *, const char *, s32, s32 *) __asm__("func_00115098");
extern s32 __swsetup(FILE_16FA0 *) __asm__("func_00118928");
extern s32 __sbprintf(FILE_16FA0 *, const char *, char *) __asm__("func_00116FE8");
extern s32 __sprint(FILE_16FA0 *, struct __suio_16FA0 *) __asm__("func_00116FA0");
extern s32 isinf(f64) __asm__("func_00116168");
extern s32 isnan(f64) __asm__("func_001161B0");
extern char *cvt(struct _reent_16FA0 *, f64, s32, s32, char *, s32 *, s32, s32 *) __asm__("func_00118630");
extern s32 exponent(char *, s32, s32) __asm__("func_001187E0");
extern void *memchr(const void *, s32, u32) __asm__("func_001150D4");
extern u32 strlen(const char *) __asm__("func_00116810");
extern char vfprintf_string_00152600[] __asm__("D_00152600");
extern char vfprintf_string_00152608[] __asm__("D_00152608");
extern char vfprintf_string_00152610[] __asm__("D_00152610");
extern char vfprintf_string_00152628[] __asm__("D_00152628");
extern char vfprintf_string_00152630[] __asm__("D_00152630");
extern char vfprintf_string_00152648[] __asm__("D_00152648");
extern char vfprintf_string_00152668[] __asm__("D_00152668");
extern char vfprintf_string_00152670[] __asm__("D_00152670");
typedef char *va_list;
/* _vfprintf_r, adapted from newlib 1999 through Lombyte.
 * The varargs cursor advances in eight-byte slots on the EE ABI.
 * PRINT/PAD/FLUSH are expanded here to keep their scope local; the build
 * compiles this C directly. See THIRD_PARTY_NOTICES.md.
 */
s32 func_00117118(struct _reent_16FA0 *data, FILE_16FA0 *fp, const char *fmt0, va_list ap)
{
 register char *fmt;
 register int ch;
 register int n, m;
 register char *cp;
 register struct __siov_16FA0 *iovp;
 register int flags;
 int ret;
 int width;
 int prec;
 char sign;
 s32 wc;
 char *decimal_point = *localeconv();
 char softsign;
 double _double;
 int expt;
 int expsize;
 int ndig;
 char expstr[7];
 u64 _uquad;
 enum { OCT, DEC, HEX } base;
 int dprec;
 int realsz;
 int size;
 char *xdigs;
 struct __suio_16FA0 uio;
 struct __siov_16FA0 iov[8];
 char buf[348];
 char ox[2];
        int state = 0;
 if (((((fp)->_flags & 8) == 0 || (fp)->_bf._base == 0) && __swsetup(fp)))
  return ((-1));
 if ((fp->_flags & (2|8|16)) == (2|8) &&
     fp->_file >= 0)
  return (__sbprintf(fp, fmt0, ap));
 fmt = (char *)fmt0;
 uio.uio_iov = iovp = iov;
 uio.uio_resid = 0;
 uio.uio_iovcnt = 0;
 ret = 0;
 for (;;) {
         cp = fmt;
         while ((n = _mbtowc_r(vfprintf_reent_117118[0], &wc, fmt, vfprintf_mbmax_117118[0], &state)) > 0) {
   fmt += n;
   if (wc == '%') {
    fmt--;
    break;
   }
  }
  if ((m = fmt - cp) != 0) {
   { iovp->iov_base = (cp); iovp->iov_len = (m); uio.uio_resid += (m); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
   ret += m;
  }
  if (n <= 0)
   goto done;
  fmt++;
  flags = 0;
  dprec = 0;
  width = 0;
  prec = -1;
  sign = '\0';
rflag: ch = *fmt++;
reswitch: switch (ch) {
  case ' ':
   if (!sign)
    sign = ' ';
   goto rflag;
  case '#':
   flags |= 0x001;
   goto rflag;
  case '*':
   if ((width = (*(int *)((ap += 8) - 8))) >= 0)
    goto rflag;
   width = -width;
  case '-':
   flags |= 0x004;
   goto rflag;
  case '+':
   sign = '+';
   goto rflag;
  case '.':
   if ((ch = *fmt++) == '*') {
    n = (*(int *)((ap += 8) - 8));
    prec = n < 0 ? -1 : n;
    goto rflag;
   }
   n = 0;
   while (((unsigned)((ch) - '0') <= 9)) {
    n = 10 * n + ((ch) - '0');
    ch = *fmt++;
   }
   prec = n < 0 ? -1 : n;
   goto reswitch;
  case '0':
   flags |= 0x080;
   goto rflag;
  case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
   n = 0;
   do {
    n = 10 * n + ((ch) - '0');
    ch = *fmt++;
   } while (((unsigned)((ch) - '0') <= 9));
   width = n;
   goto reswitch;
  case 'L':
   flags |= 0x008;
   goto rflag;
  case 'h':
   flags |= 0x040;
   goto rflag;
  case 'l':
   if (*fmt == 'l') {
    fmt++;
    flags |= 0x020;
   } else {
    flags |= 0x010;
   }
   goto rflag;
  case 'q':
   flags |= 0x020;
   goto rflag;
  case 'c':
   *(cp = buf) = (*(int *)((ap += 8) - 8));
   size = 1;
   sign = '\0';
   break;
  case 'D':
   flags |= 0x010;
  case 'd':
  case 'i':
   _uquad = (flags&0x010 ? (*(s64 *)((ap += 8) - 8)) : flags&0x040 ? (long)(short)(*(int *)((ap += 8) - 8)) : (long)(*(int *)((ap += 8) - 8)));
   if ((s64)_uquad < 0)
   {
    _uquad = -_uquad;
    sign = '-';
   }
   base = DEC;
   goto number;
  case 'e':
  case 'E':
  case 'f':
  case 'g':
  case 'G':
   if (prec == -1) {
    prec = 6;
   } else if ((ch == 'g' || ch == 'G') && prec == 0) {
    prec = 1;
   }
   if (flags & 0x008) {
    _double = (double) (*(long double *)((ap += 8) - 8));
   } else {
    _double = (*(double *)((ap += 8) - 8));
   }
   if (isinf(_double)) {
    if (_double < 0)
     sign = '-';
    cp = vfprintf_string_00152600;
    size = 3;
    break;
   }
   if (isnan(_double)) {
    cp = vfprintf_string_00152608;
    size = 3;
    break;
   }
   flags |= 0x100;
   cp = cvt(data, _double, prec, flags, &softsign,
    &expt, ch, &ndig);
   if (ch == 'g' || ch == 'G') {
    if (expt <= -4 || expt > prec)
     ch = (ch == 'g') ? 'e' : 'E';
    else
     ch = 'g';
   }
   if (ch <= 'e') {
    --expt;
    expsize = exponent(expstr, expt, ch);
    size = expsize + ndig;
    if (ndig > 1 || flags & 0x001)
     ++size;
   } else if (ch == 'f') {
    if (expt > 0) {
     size = expt;
     if (prec || flags & 0x001)
      size += prec + 1;
    } else
     size = prec + 2;
   } else if (expt >= ndig) {
    size = expt;
    if (flags & 0x001)
     ++size;
   } else
    size = ndig + (expt > 0 ?
     1 : 2 - expt);
   if (softsign)
    sign = '-';
   break;
  case 'n':
   if (flags & 0x010)
    *((s64 *)(*(void **)((ap += 8) - 8))) = ret;
   else if (flags & 0x040)
    *((short *)(*(void **)((ap += 8) - 8))) = ret;
   else
    *((int *)(*(void **)((ap += 8) - 8))) = ret;
   continue;
  case 'O':
   flags |= 0x010;
  case 'o':
   _uquad = (flags&0x010 ? (*(u64 *)((ap += 8) - 8)) : flags&0x040 ? (unsigned long)(unsigned short)(*(int *)((ap += 8) - 8)) : (unsigned long)(*(unsigned int *)((ap += 8) - 8)));
   base = OCT;
   goto nosign;
  case 'p':
   _uquad = (long)(int)((void *)(*(void **)((ap += 8) - 8)));
   base = HEX;
   xdigs = vfprintf_string_00152610;
   flags |= 0x002;
   ch = 'x';
   goto nosign;
  case 's':
   if ((cp = ((char *)(*(void **)((ap += 8) - 8)))) == 0)
    cp = vfprintf_string_00152628;
   if (prec >= 0) {
    char *p = memchr(cp, 0, prec);
    if (p != 0) {
     size = p - cp;
     if (size > prec)
      size = prec;
    } else
     size = prec;
   } else
    size = strlen(cp);
   sign = '\0';
   break;
  case 'U':
   flags |= 0x010;
  case 'u':
   _uquad = (flags&0x010 ? (*(u64 *)((ap += 8) - 8)) : flags&0x040 ? (unsigned long)(unsigned short)(*(int *)((ap += 8) - 8)) : (unsigned long)(*(unsigned int *)((ap += 8) - 8)));
   base = DEC;
   goto nosign;
  case 'X':
   xdigs = vfprintf_string_00152630;
   goto hex;
  case 'x':
   xdigs = vfprintf_string_00152610;
hex: _uquad = (flags&0x010 ? (*(u64 *)((ap += 8) - 8)) : flags&0x040 ? (unsigned long)(unsigned short)(*(int *)((ap += 8) - 8)) : (unsigned long)(*(unsigned int *)((ap += 8) - 8)));
   base = HEX;
   if (flags & 0x001 && _uquad != 0)
    flags |= 0x002;
nosign: sign = '\0';
number: if ((dprec = prec) >= 0)
    flags &= ~0x080;
   cp = buf + 348;
   if (_uquad != 0 || prec != 0) {
    switch (base) {
    case OCT:
     do {
      *--cp = ((_uquad & 7) + '0');
      _uquad >>= 3;
     } while (_uquad);
     if (flags & 0x001 && *cp != '0')
      *--cp = '0';
     break;
    case DEC:
     while (_uquad >= 10) {
      *--cp = ((_uquad % 10) + '0');
      _uquad /= 10;
     }
     *--cp = ((_uquad) + '0');
     break;
    case HEX:
     do {
      *--cp = xdigs[_uquad & 15];
      _uquad >>= 4;
     } while (_uquad);
     break;
    default:
     cp = vfprintf_string_00152648;
     size = strlen(cp);
     goto skipsize;
    }
   }
   size = buf + 348 - cp;
  skipsize:
   break;
  default:
   if (ch == '\0')
    goto done;
   cp = buf;
   *cp = ch;
   size = 1;
   sign = '\0';
   break;
  }
  realsz = dprec > size ? dprec : size;
  if (sign)
   realsz++;
  else if (flags & 0x002)
   realsz+= 2;
  if ((flags & (0x004|0x080)) == 0)
   { if ((n = (width - realsz)) > 0) { while (n > 16) { { iovp->iov_base = (vfprintf_blanks_117118); iovp->iov_len = (16); uio.uio_resid += (16); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; n -= 16; } { iovp->iov_base = (vfprintf_blanks_117118); iovp->iov_len = (n); uio.uio_resid += (n); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; } };
  if (sign) {
   { iovp->iov_base = (&sign); iovp->iov_len = (1); uio.uio_resid += (1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
  } else if (flags & 0x002) {
   ox[0] = '0';
   ox[1] = ch;
   { iovp->iov_base = (ox); iovp->iov_len = (2); uio.uio_resid += (2); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
  }
  if ((flags & (0x004|0x080)) == 0x080)
   { if ((n = (width - realsz)) > 0) { while (n > 16) { { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (16); uio.uio_resid += (16); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; n -= 16; } { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (n); uio.uio_resid += (n); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; } };
  { if ((n = (dprec - size)) > 0) { while (n > 16) { { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (16); uio.uio_resid += (16); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; n -= 16; } { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (n); uio.uio_resid += (n); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; } };
  if ((flags & 0x100) == 0) {
   { iovp->iov_base = (cp); iovp->iov_len = (size); uio.uio_resid += (size); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
  } else {
   if (ch >= 'f') {
    if (_double == 0) {
     { iovp->iov_base = (vfprintf_string_00152668); iovp->iov_len = (1); uio.uio_resid += (1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
     if (expt < ndig || (flags & 0x001) != 0) {
      { iovp->iov_base = (decimal_point); iovp->iov_len = (1); uio.uio_resid += (1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
      { if ((n = (ndig - 1)) > 0) { while (n > 16) { { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (16); uio.uio_resid += (16); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; n -= 16; } { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (n); uio.uio_resid += (n); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; } };
     }
    } else if (expt <= 0) {
     { iovp->iov_base = (vfprintf_string_00152668); iovp->iov_len = (1); uio.uio_resid += (1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
     { iovp->iov_base = (decimal_point); iovp->iov_len = (1); uio.uio_resid += (1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
     { if ((n = (-expt)) > 0) { while (n > 16) { { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (16); uio.uio_resid += (16); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; n -= 16; } { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (n); uio.uio_resid += (n); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; } };
     { iovp->iov_base = (cp); iovp->iov_len = (ndig); uio.uio_resid += (ndig); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
    } else if (expt >= ndig) {
     { iovp->iov_base = (cp); iovp->iov_len = (ndig); uio.uio_resid += (ndig); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
     { if ((n = (expt - ndig)) > 0) { while (n > 16) { { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (16); uio.uio_resid += (16); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; n -= 16; } { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (n); uio.uio_resid += (n); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; } };
     if (flags & 0x001)
      { iovp->iov_base = (vfprintf_string_00152670); iovp->iov_len = (1); uio.uio_resid += (1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
    } else {
     { iovp->iov_base = (cp); iovp->iov_len = (expt); uio.uio_resid += (expt); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
     cp += expt;
     { iovp->iov_base = (vfprintf_string_00152670); iovp->iov_len = (1); uio.uio_resid += (1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
     { iovp->iov_base = (cp); iovp->iov_len = (ndig-expt); uio.uio_resid += (ndig-expt); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
    }
   } else {
    if (ndig > 1 || flags & 0x001) {
     ox[0] = *cp++;
     ox[1] = '.';
     { iovp->iov_base = (ox); iovp->iov_len = (2); uio.uio_resid += (2); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
                                       if (_double) {
      { iovp->iov_base = (cp); iovp->iov_len = (ndig-1); uio.uio_resid += (ndig-1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
     } else
      { if ((n = (ndig - 1)) > 0) { while (n > 16) { { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (16); uio.uio_resid += (16); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; n -= 16; } { iovp->iov_base = (vfprintf_zeroes_117118); iovp->iov_len = (n); uio.uio_resid += (n); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; } };
    } else
     { iovp->iov_base = (cp); iovp->iov_len = (1); uio.uio_resid += (1); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
    { iovp->iov_base = (expstr); iovp->iov_len = (expsize); uio.uio_resid += (expsize); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } };
   }
  }
  if (flags & 0x004)
   { if ((n = (width - realsz)) > 0) { while (n > 16) { { iovp->iov_base = (vfprintf_blanks_117118); iovp->iov_len = (16); uio.uio_resid += (16); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; n -= 16; } { iovp->iov_base = (vfprintf_blanks_117118); iovp->iov_len = (n); uio.uio_resid += (n); iovp++; if (++uio.uio_iovcnt >= 8) { if (__sprint(fp, &uio)) goto error; iovp = iov; } }; } };
  ret += width > realsz ? width : realsz;
  { if (uio.uio_resid && __sprint(fp, &uio)) goto error; uio.uio_iovcnt = 0; iovp = iov; };
 }
done:
 { if (uio.uio_resid && __sprint(fp, &uio)) goto error; uio.uio_iovcnt = 0; iovp = iov; };
error:
 return (((fp)->_flags & 64) ? (-1) : ret);
}

/* newlib's `union double_union` over the little-endian word order:
   word0 (the sign/exponent word) is i[1]. */
union double_union_16FA0 {
    double d;
    unsigned int i[2];
};
extern char *func_001126D8_d(void *, double, int, int, int *, int *, char **)
    __asm__("func_001126D8");

/* cvt, newlib's text: convert a double for the %e/%f/%g conversions
   through _dtoa_r (func_001126D8), padding trailing zeros unless %g
   without the ALT flag. */
char *func_00118630(void *data, double value, int ndigits, int flags,
                    char *sign, int *decpt, int ch, int *length) {
    int mode, dsgn;
    char *digits, *bp, *rve;
    union double_union_16FA0 tmp;

    if (ch == 'f') {
        mode = 3;               /* ndigits after the decimal point */
    } else {
        /* To obtain ndigits after the decimal point for the 'e'
         * and 'E' formats, round to ndigits + 1 significant
         * figures.
         */
        if (ch == 'e' || ch == 'E') {
            ndigits++;
        }
        mode = 2;               /* ndigits significant digits */
    }

    tmp.d = value;
    if (tmp.i[1] & 0x80000000) { /* this will check for < 0 and -0.0 */
        value = -value;
        *sign = '-';
    } else
        *sign = '\000';
    digits = func_001126D8_d(data, value, mode, ndigits, decpt, &dsgn, &rve);
    if ((ch != 'g' && ch != 'G') || flags & 0x001) { /* ALT */
        bp = digits + ndigits;
        if (ch == 'f') {
            if (*digits == '0' && value)
                *decpt = -ndigits + 1;
            bp += *decpt;
        }
        if (value == 0) /* kludge for __dtoa irregularity */
            rve = bp;
        while (rve < bp)
            *rve++ = '0';
    }
    *length = rve - digits;
    return (digits);
}

/* exponent, newlib's text, with its MAXEXP of 308: the 0x140 frame and
   `t = expbuf + 308` are retail's. */
int func_001187E0(char *p0, int exp, int fmtch) {
    register char *p, *t;
    char expbuf[308];

    p = p0;
    *p++ = fmtch;
    if (exp < 0) {
        exp = -exp;
        *p++ = '-';
    }
    else
        *p++ = '+';
    t = expbuf + 308;
    if (exp > 9) {
        do {
            *--t = (exp % 10) + '0';
        } while ((exp /= 10) > 9);
        *--t = exp + '0';
        for (; t < expbuf + 308; *p++ = *t++);
    }
    else {
        *p++ = '0';
        *p++ = exp + '0';
    }
    return (p - p0);
}

LINKER_REMNANT("asm/remnants/core_text", func_001188C0);
