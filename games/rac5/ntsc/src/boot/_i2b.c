/* Adapted from public Newlib 1.9.0 mprec.c; retail RAC5 ABI and
 * placement measured independently. Source: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libc/stdlib/mprec.c */
/* newlib _i2b, retail 0x01ECFC20 (56 bytes):
 *   b = _Balloc(ptr, 1); b->_x[0] = x; b->_wds = 1; return b;
 */
struct _reent;
struct _Bigint {
    struct _Bigint *_next;       /* 0x00 */
    int _k;                      /* 0x04 */
    int _maxwds;                 /* 0x08 */
    int _sign;                   /* 0x0C */
    int _wds;                    /* 0x10 */
    unsigned int _x[1];          /* 0x14 */
};

extern struct _Bigint *_Balloc(struct _reent *ptr, int k);

struct _Bigint *_i2b(struct _reent *ptr, int x) {
    struct _Bigint *b = _Balloc(ptr, 1);
    b->_x[0] = x;
    b->_wds = 1;
    return b;
}

/* Upstream permission notice (also indexed in THIRD_PARTY_NOTICES.md): */
/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991 by AT&T.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/
