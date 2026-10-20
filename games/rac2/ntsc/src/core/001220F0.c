#include "common.h"

/* libgcc2.c, module L_umoddi3 -- GCC's runtime library. `symbol_addrs.txt` names
   0x001220F0 `__umoddi3`; the retail body is the whole `__udivmoddi4` inlined with
   `rp` = &w, 1344 bytes. Same source, compiler and provenance as its sibling
   src/core/00121B20.c (L_udivdi3) -- nothing is decompiled, because Sony shipped
   the object prebuilt and it is what retail linked; read that file's header for the
   full account of the revision, the driver and the two retail facts.

   What is specific to this module:

   - `addiu $sp,$sp,-0x30` in the prologue, before the delay slot -- 48 bytes, not
     the 0x10 of L_udivdi3, because `&w` is passed and `w` gets a stack slot.
     rac1-decomp measures 0x30 for its L_umoddi3 at 0x11F4F8 too and records that
     only Sony's own compiler emits it.
   - `__clz_tab` is this module's own copy of libgcc2.c's `static const` table, at
     0x0013AF58 -- 0x100 past L_udivdi3's, exactly as in rac1's retail (0x152D18
     against 0x152C18). Both addresses are readable from the two prologues' `lui
     $v0,0x14`: `addiu $v0,$v0,-0x50A8` here.
   - `daddu $t8,$sp,$zero` in the delay slot of the first `bnez` is `rp` = &w.
   - `__udivmoddi4` is `static inline` here too and the single call site inlines;
     the non-null `rp` keeps every `rr.s.low/high` store that the L_udivdi3 module
     folded away, which is where the 144-byte difference between the two comes from.

   GPL version 2-or-later with the libgcc linking exception; the copyright and
   permission notices are the ones reproduced in src/core/001232F0.c, and the
   text is in COPYING and THIRD_PARTY_NOTICES.md at the root. */

/* --- libgcc2.c's type vocabulary (unchanged) --------------------------------------- */

typedef unsigned int UQItype __attribute__((mode(QI)));
typedef int SItype __attribute__((mode(SI)));
typedef unsigned int USItype __attribute__((mode(SI)));
typedef int DItype __attribute__((mode(DI)));
typedef unsigned int UDItype __attribute__((mode(DI)));

/* Make sure that we don't accidentally use any normal C language built-in
   type names in the first part of this file.  Instead we want to use *only*
   the type names defined above.  The following macro definitions insure
   that if we *do* accidentally use some normal C language built-in type name,
   we will get a syntax error.  */

#define char     bogus_type
#define short    bogus_type
#define int      bogus_type
#define long     bogus_type
#define unsigned bogus_type
#define float    bogus_type
#define double   bogus_type

#define BITS_PER_UNIT 8

#define SI_TYPE_SIZE (sizeof(SItype) * BITS_PER_UNIT)

/* DIstructs are pairs of SItype values in the order determined by
   LIBGCC2_WORDS_BIG_ENDIAN, 0 for the EE.  */

struct DIstruct {
    SItype low, high;
};

/* We need this union to unpack/pack DImode values, since we don't have
   any arithmetic yet.  Incoming DImode parameters are stored into the
   `ll' field, and the unpacked result is read from the struct `s'.  */

typedef union {
    struct DIstruct s;
    DItype ll;
} DIunion;

/* --- longlong.h for __mips__ ------------------------------------------------------- */

#define __BITS4          (SI_TYPE_SIZE / 4)
#define __ll_B           (1L << (SI_TYPE_SIZE / 2))
#define __ll_lowpart(t)  ((USItype)(t) % __ll_B)
#define __ll_highpart(t) ((USItype)(t) / __ll_B)

#define umul_ppmm(w1, w0, u, v)                                                                    \
    __asm__("multu %2,%3"                                                                          \
            : "=l"((USItype)(w0)), "=h"((USItype)(w1))                                             \
            : "d"((USItype)(u)), "d"((USItype)(v)))
#define UMUL_TIME 10
#define UDIV_TIME 100

/* Define this unconditionally, so it can be used for debugging.  */
#define __udiv_qrnnd_c(q, r, n1, n0, d)                                                            \
    do {                                                                                           \
        USItype __d1, __d0, __q1, __q0;                                                            \
        USItype __r1, __r0, __m;                                                                   \
        __d1 = __ll_highpart(d);                                                                   \
        __d0 = __ll_lowpart(d);                                                                    \
                                                                                                   \
        __r1 = (n1) % __d1;                                                                        \
        __q1 = (n1) / __d1;                                                                        \
        __m = (USItype)__q1 * __d0;                                                                \
        __r1 = __r1 * __ll_B | __ll_highpart(n0);                                                  \
        if (__r1 < __m) {                                                                          \
            __q1--, __r1 += (d);                                                                   \
            if (__r1 >= (d)) /* i.e. we didn't get carry when adding to __r1 */                    \
                if (__r1 < __m)                                                                    \
                    __q1--, __r1 += (d);                                                           \
        }                                                                                          \
        __r1 -= __m;                                                                               \
                                                                                                   \
        __r0 = __r1 % __d1;                                                                        \
        __q0 = __r1 / __d1;                                                                        \
        __m = (USItype)__q0 * __d0;                                                                \
        __r0 = __r0 * __ll_B | __ll_lowpart(n0);                                                   \
        if (__r0 < __m) {                                                                          \
            __q0--, __r0 += (d);                                                                   \
            if (__r0 >= (d))                                                                       \
                if (__r0 < __m)                                                                    \
                    __q0--, __r0 += (d);                                                           \
        }                                                                                          \
        __r0 -= __m;                                                                               \
                                                                                                   \
        (q) = (USItype)__q1 * __ll_B | __q0;                                                       \
        (r) = __r0;                                                                                \
    } while (0)

/* The EE has no udiv_qrnnd instruction, so longlong.h falls back to the C
   expansion and asks for the normalization step.  */

#define UDIV_NEEDS_NORMALIZATION 1
#define udiv_qrnnd               __udiv_qrnnd_c

/* If we don't have count_leading_zeros, here is a default one.  The table is
   libgcc2.c's `static const UQItype __clz_tab[]`; this module's copy is the one
   at 0x0013AF58 in retail, referenced by address so this object holds no copy.  */

#define __clz_tab D_0013AF58

extern const UQItype D_0013AF58[];

#define count_leading_zeros(count, x)                                                              \
    do {                                                                                           \
        USItype __xr = (x);                                                                        \
        USItype __a;                                                                               \
                                                                                                   \
        if (SI_TYPE_SIZE <= 32) {                                                                  \
            __a = __xr < ((USItype)1 << 2 * __BITS4)                                               \
                      ? (__xr < ((USItype)1 << __BITS4) ? 0 : __BITS4)                             \
                      : (__xr < ((USItype)1 << 3 * __BITS4) ? 2 * __BITS4 : 3 * __BITS4);          \
        } else {                                                                                   \
            for (__a = SI_TYPE_SIZE - 8; __a > 0; __a -= 8)                                        \
                if (((__xr >> __a) & 0xff) != 0)                                                   \
                    break;                                                                         \
        }                                                                                          \
                                                                                                   \
        (count) = SI_TYPE_SIZE - (__clz_tab[__xr >> __a] + __a);                                   \
    } while (0)

#define add_ssaaaa(sh, sl, ah, al, bh, bl)                                                         \
    do {                                                                                           \
        USItype __x;                                                                               \
        __x = (al) + (bl);                                                                         \
        (sh) = (ah) + (bh) + (__x < (al));                                                         \
        (sl) = __x;                                                                                \
    } while (0)

#define sub_ddmmss(sh, sl, ah, al, bh, bl)                                                         \
    do {                                                                                           \
        USItype __x;                                                                               \
        __x = (al) - (bl);                                                                         \
        (sh) = (ah) - (bh) - (__x > (al));                                                         \
        (sl) = __x;                                                                                \
    } while (0)

/* --- libgcc2.c, the L_udivmoddi4 part (L_umoddi3 defines it too) ------------------ */

static inline UDItype __udivmoddi4(UDItype n, UDItype d, UDItype *rp) {
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

    /* UDIV_NEEDS_NORMALIZATION is 1: the `!UDIV_NEEDS_NORMALIZATION' arm is not
       compiled.  Unlike the L_udivdi3 module, `rp' is not known to be null, so
       every remainder store below is live.  */

    if (d1 == 0) {
        if (d0 > n1) {
            /* 0q = nn / 0D */

            count_leading_zeros(bm, d0);

            if (bm != 0) {
                /* Normalize, i.e. make the most significant bit of the
                   denominator set.  */

                d0 = d0 << bm;
                n1 = (n1 << bm) | (n0 >> (SI_TYPE_SIZE - bm));
                n0 = n0 << bm;
            }

            udiv_qrnnd(q0, n0, n1, n0, d0);
            q1 = 0;

            /* Remainder in n0 >> bm.  */
        } else {
            /* qq = NN / 0d */

            if (d0 == 0)
                d0 = 1 / d0; /* Divide intentionally by zero.  */

            count_leading_zeros(bm, d0);

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

                udiv_qrnnd(q1, n1, n2, n1, d0);
            }

            /* n1 != d0...  */

            udiv_qrnnd(q0, n0, n1, n0, d0);

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

            count_leading_zeros(bm, d1);
            if (bm == 0) {
                /* From (n1 >= d1) /\ (the most significant bit of d1 is set),
                   conclude (the most significant bit of n1 is set) /\ (the
                   quotient digit q0 = 0 or 1).

                   This special case is necessary, not an optimization.  */

                /* The condition on the next line takes advantage of that
                   n1 >= d1 (true due to program flow).  */
                if (n1 > d1 || n0 >= d0) {
                    q0 = 1;
                    sub_ddmmss(n1, n0, n1, n0, d1, d0);
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

                udiv_qrnnd(q0, n1, n2, n1, d1);
                umul_ppmm(m1, m0, q0, d0);

                if (m1 > n1 || (m1 == n1 && m0 > n0)) {
                    q0--;
                    sub_ddmmss(m1, m0, m1, m0, d1, d0);
                }

                q1 = 0;

                /* Remainder in (n1n0 - m1m0) >> bm.  */
                if (rp != 0) {
                    sub_ddmmss(n1, n0, n1, n0, m1, m0);
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

/* --- libgcc2.c, the L_umoddi3 part (renamed: __umoddi3 is func_001220F0) ---------- */

UDItype __udivmoddi4();

UDItype func_001220F0(UDItype u, UDItype v) {
    UDItype w;

    (void)__udivmoddi4(u, v, &w);

    return w;
}
