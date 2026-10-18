/* _b2d -- David M. Gay's mprec b2d, adapted to bare types.
 * Provenance: newlib libc/stdlib/mprec.c `b2d` (Pack_32, IEEE_MC68k word
 * order); measured compiler SN ProDG 3.01 ee-gcc2953 (GCC 2.95.3) +
 * Ps2EeAs 2.0, -O2 -G0. */
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;

typedef struct _Bigint {
    struct _Bigint *_next;
    s32 _k, _maxwds, _sign, _wds;
    u32 _x[1];
} _Bigint;

struct _reent;

typedef union {
    double d;
    u32 i[2];
} double_union;

#define word0(x) (x.i[1])
#define word1(x) (x.i[0])
#define Exp_1 ((u32)0x3ff00000L)
#define Ebits 11

extern s32 _hi0bits(u32);

double _b2d(_Bigint *a, s32 *e)
{
    u32 *xa, *xa0, w, y, z;
    s32 k;
    double_union d;

    xa0 = a->_x;
    xa = xa0 + a->_wds;
    y = *--xa;
    k = _hi0bits(y);
    *e = 32 - k;
    if (k < Ebits) {
        word0(d) = Exp_1 | y >> (Ebits - k);
        w = xa > xa0 ? *--xa : 0;
        word1(d) = y << ((32 - Ebits) + k) | w >> (Ebits - k);
        goto ret_d;
    }
    z = xa > xa0 ? *--xa : 0;
    if ((k -= Ebits) != 0) {
        word0(d) = Exp_1 | y << k | z >> (32 - k);
        y = xa > xa0 ? *--xa : 0;
        word1(d) = z << k | y >> (32 - k);
    } else {
        word0(d) = Exp_1 | y;
        word1(d) = z;
    }
ret_d:
    return d.d;
}
