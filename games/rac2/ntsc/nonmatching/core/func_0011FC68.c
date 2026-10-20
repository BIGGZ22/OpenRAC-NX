/* from src/core/0011FA70.c: the recovered libgcc2.c source is instruction-exact for 394 of
   retail's 443 instructions, but this cc1 places six nops differently and schedules the
   tail differently (candidate 1788 bytes against retail's 1772); the 256-byte __clz_tab
   also needs .rodata at 0x13AC58, which tools/build.py cannot place */
/*
 * libgcc libgcc2.c: __divdi3, the L_divdi3 member (GPL version 2-or-later with
 * the libgcc linking exception; the source is the public archive the qualified
 * compiler is built from, gnu-ee-binutils-gcc-1.1, gcc/libgcc2.c with
 * gcc/longlong.h).
 *
 * Checked as `sdk -O2 -G2 -ffunction-sections`: 1788 bytes against 1772. Non-nop
 * mnemonic streams are identical for the first 394 of 443 instructions and diverge only
 * from the umul_ppmm site (this cc1 emits mflo before mfhi, retail mfhi before mflo) and
 * in the closing __negdi2. Nops: retail 9, candidate 6 - owed to __udiv_qrnnd_c's
 * correction block, where retail puts `__r1 -= __m` in a beql delay slot (nullified on the
 * fall-through) and this cc1 keeps a straight-line copy, 7 instructions longer.
 *
 * For __mips__ longlong.h defines only umul_ppmm, so udiv_qrnnd falls back to
 * __udiv_qrnnd_c with UDIV_NEEDS_NORMALIZATION 1 and count_leading_zeros to the
 * table form; retail shows both: the 16-bit-digit divide at every udiv_qrnnd,
 * and `lbu` from __clz_tab. That table is this unit's own 256-byte copy, at
 * 0x13AC58 in retail (`lui $v0,0x14 ; addiu $v0,$v0,-0x53A8`); __moddi3 has its
 * own at 0x13AD58, so they are not shared. The entry points keep the names the
 * compiler's own sources carry: __negdi2 and __udivmoddi4 are the static
   inline helpers libgcc2.c only gives those names to under L_divdi3. */

typedef int SItype __attribute__ ((mode (SI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef int DItype __attribute__ ((mode (DI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));
typedef int word_type __attribute__ ((mode (__word__)));

struct DIstruct { SItype low, high; };
typedef union {
    struct DIstruct s;
    DItype ll;
} DIunion;

#ifndef SI_TYPE_SIZE
#define SI_TYPE_SIZE 32
#endif
#define __BITS4 (SI_TYPE_SIZE / 4)
#define __ll_B (1L << (SI_TYPE_SIZE / 2))
#define __ll_lowpart(t) ((USItype) (t) % __ll_B)
#define __ll_highpart(t) ((USItype) (t) / __ll_B)

static const unsigned char __clz_tab[] = {
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

#define count_leading_zeros(count, x)                                       \
    do {                                                                    \
        USItype __xr = (x);                                                 \
        USItype __a;                                                        \
                                                                            \
        __a = __xr < ((USItype) 1 << 2 * __BITS4)                           \
              ? (__xr < ((USItype) 1 << __BITS4) ? 0 : __BITS4)             \
              : (__xr < ((USItype) 1 << 3 * __BITS4) ? 2 * __BITS4 : 3 * __BITS4); \
                                                                            \
        (count) = SI_TYPE_SIZE - (__clz_tab[__xr >> __a] + __a);            \
    } while (0)

#define sub_ddmmss(sh, sl, ah, al, bh, bl)                                  \
    do {                                                                    \
        USItype __x;                                                        \
        __x = (al) - (bl);                                                  \
        (sh) = (ah) - (bh) - (__x > (al));                                  \
        (sl) = __x;                                                         \
    } while (0)

#define umul_ppmm(w1, w0, u, v)                                             \
    do {                                                                    \
        DIunion __w;                                                        \
        __w.ll = (UDItype) (USItype) (u) * (UDItype) (USItype) (v);         \
        (w1) = __w.s.high;                                                  \
        (w0) = __w.s.low;                                                   \
    } while (0)

#define udiv_qrnnd(q, r, n1, n0, d)                                         \
    do {                                                                    \
        USItype __d1, __d0, __q1, __q0;                                     \
        USItype __r1, __r0, __m;                                            \
        __d1 = __ll_highpart (d);                                           \
        __d0 = __ll_lowpart (d);                                            \
                                                                            \
        __r1 = (n1) % __d1;                                                 \
        __q1 = (n1) / __d1;                                                 \
        __m = (USItype) __q1 * __d0;                                        \
        __r1 = __r1 * __ll_B | __ll_highpart (n0);                          \
        if (__r1 < __m) {                                                   \
            __q1--, __r1 += (d);                                            \
            if (__r1 >= (d))                                                \
                if (__r1 < __m)                                             \
                    __q1--, __r1 += (d);                                    \
        }                                                                   \
        __r1 -= __m;                                                        \
                                                                            \
        __r0 = __r1 % __d1;                                                 \
        __q0 = __r1 / __d1;                                                 \
        __m = (USItype) __q0 * __d0;                                        \
        __r0 = __r0 * __ll_B | __ll_lowpart (n0);                           \
        if (__r0 < __m) {                                                   \
            __q0--, __r0 += (d);                                            \
            if (__r0 >= (d))                                                \
                if (__r0 < __m)                                             \
                    __q0--, __r0 += (d);                                    \
        }                                                                   \
        __r0 -= __m;                                                        \
                                                                            \
        (q) = (USItype) __q1 * __ll_B | __q0;                               \
        (r) = __r0;                                                         \
    } while (0)

static inline DItype __negdi2 (DItype u) {
    DIunion w;
    DIunion uu;

    uu.ll = u;

    w.s.low = -uu.s.low;
    w.s.high = -uu.s.high - ((USItype) w.s.low > 0);

    return w.ll;
}

static inline UDItype __udivmoddi4 (UDItype n, UDItype d, UDItype *rp) {
    DIunion ww;
    DIunion nn, dd;
    DIunion rr;
    USItype d0, d1, n0, n1, n2;
    USItype q0, q1;
    USItype b, bm;

    nn.ll = n;
    dd.ll = d;

    d0 = dd.s.low;
    d1 = dd.s.high;
    n0 = nn.s.low;
    n1 = nn.s.high;

    if (d1 == 0) {
        if (d0 > n1) {
            /* 0q = nn / 0D */

            count_leading_zeros (bm, d0);

            if (bm != 0) {
                /* Normalize, i.e. make the most significant bit of the
                   denominator set.  */

                d0 = d0 << bm;
                n1 = (n1 << bm) | (n0 >> (SI_TYPE_SIZE - bm));
                n0 = n0 << bm;
            }

            udiv_qrnnd (q0, n0, n1, n0, d0);
            q1 = 0;

            /* Remainder in n0 >> bm.  */
        } else {
            /* qq = NN / 0d */

            if (d0 == 0)
                d0 = 1 / d0;	/* Divide intentionally by zero.  */

            count_leading_zeros (bm, d0);

            if (bm == 0) {
                /* From (n1 >= d0) /\ (the most significant bit of d0 is set),
                   conclude (the most significant bit of n1 is set) /\ (the
                   leading quotient digit q1 = 1).

                   This special case is necessary, not an optimization.
                   (Shifts counts of SI_TYPE_SIZE are undefined.)  */

                n1 -= d0;
                q1 = 1;
            } else {
                /* Normalize.  */

                b = SI_TYPE_SIZE - bm;

                d0 = d0 << bm;
                n2 = n1 >> b;
                n1 = (n1 << bm) | (n0 >> b);
                n0 = n0 << bm;

                udiv_qrnnd (q1, n1, n2, n1, d0);
            }

            /* n1 != d0...  */

            udiv_qrnnd (q0, n0, n1, n0, d0);

            /* Remainder in n0 >> bm.  */
        }

        if (rp != 0) {
            rr.s.low = n0 >> bm;
            rr.s.high = 0;
            *rp = rr.ll;
        }
    } else {
        if (d1 > n1) {
            /* 00 = nn / DD */

            q0 = 0;
            q1 = 0;

            /* Remainder in n1n0.  */
            if (rp != 0) {
                rr.s.low = n0;
                rr.s.high = n1;
                *rp = rr.ll;
            }
        } else {
            /* 0q = NN / dd */

            count_leading_zeros (bm, d1);
            if (bm == 0) {
                /* From (n1 >= d1) /\ (the most significant bit of d1 is set),
                   conclude (the most significant bit of n1 is set) /\ (the
                   quotient digit q0 = 0 or 1).

                   This special case is necessary, not an optimization.  */

                /* The condition on the next line takes advantage of that
                   n1 >= d1 (true due to program flow).  */
                if (n1 > d1 || n0 >= d0) {
                    q0 = 1;
                    sub_ddmmss (n1, n0, n1, n0, d1, d0);
                } else
                    q0 = 0;

                q1 = 0;

                if (rp != 0) {
                    rr.s.low = n0;
                    rr.s.high = n1;
                    *rp = rr.ll;
                }
            } else {
                USItype m1, m0;
                /* Normalize.  */

                b = SI_TYPE_SIZE - bm;

                d1 = (d1 << bm) | (d0 >> b);
                d0 = d0 << bm;
                n2 = n1 >> b;
                n1 = (n1 << bm) | (n0 >> b);
                n0 = n0 << bm;

                udiv_qrnnd (q0, n1, n2, n1, d1);
                umul_ppmm (m1, m0, q0, d0);

                if (m1 > n1 || (m1 == n1 && m0 > n0)) {
                    q0--;
                    sub_ddmmss (m1, m0, m1, m0, d1, d0);
                }

                q1 = 0;

                /* Remainder in (n1n0 - m1m0) >> bm.  */
                if (rp != 0) {
                    sub_ddmmss (n1, n0, n1, n0, m1, m0);
                    rr.s.low = (n1 << b) | (n0 >> bm);
                    rr.s.high = n1 >> bm;
                    *rp = rr.ll;
                }
            }
        }
    }

    ww.s.low = q0;
    ww.s.high = q1;
    return ww.ll;
}

DItype func_0011FC68 (DItype u, DItype v) {
    word_type c = 0;
    DIunion uu, vv;
    DItype w;

    uu.ll = u;
    vv.ll = v;

    if (uu.s.high < 0)
        c = ~c,
            uu.ll = __negdi2 (uu.ll);
    if (vv.s.high < 0)
        c = ~c,
            vv.ll = __negdi2 (vv.ll);

    w = __udivmoddi4 (uu.ll, vv.ll, (UDItype *) 0);
    if (c)
        w = __negdi2 (w);

    return w;
}
