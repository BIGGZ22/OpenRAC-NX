/* _d2b -- David M. Gay's mprec d2b, adapted to bare types.
 * Provenance: newlib libc/stdlib/mprec.c `d2b` (Pack_32, IEEE, no
 * Sudden_Underflow); measured compiler SN ProDG 3.01 ee-gcc2953
 * (GCC 2.95.3) + Ps2EeAs 2.0, -O2 -G0. */
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef unsigned int ULong;

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

#define P 53
#define Bias 1023
#define Exp_shift 20
#define Exp_msk1 0x100000
#define Frac_mask 0xfffff

extern _Bigint *_Balloc(struct _reent *, int);
extern int _lo0bits(ULong *);
extern int _hi0bits(ULong);

_Bigint *_d2b(struct _reent *ptr, double _d, int *e, int *bits)
{
    double_union d;
    _Bigint *b;
    int de, i, k;
    ULong *x, y, z;
#define d0 word0(d)
#define d1 word1(d)

    d.d = _d;
    b = _Balloc(ptr, 1);
    x = b->_x;

    z = d0 & Frac_mask;
    d0 &= 0x7fffffff;
    if ((de = (int)(d0 >> Exp_shift)) != 0)
        z |= Exp_msk1;
    if ((y = d1) != 0) {
        if ((k = _lo0bits(&y)) != 0) {
            x[0] = y | z << 32 - k;
            z >>= k;
        } else
            x[0] = y;
        i = b->_wds = (x[1] = z) ? 2 : 1;
    } else {
        k = _lo0bits(&z);
        x[0] = z;
        i = b->_wds = 1;
        k += 32;
    }
    if (de) {
        *e = de - Bias - (P - 1) + k;
        *bits = P - k;
    } else {
        *e = de - Bias - (P - 1) + 1 + k;
        *bits = 32 * i - _hi0bits(x[i - 1]);
    }
    return b;
#undef d0
#undef d1
}
