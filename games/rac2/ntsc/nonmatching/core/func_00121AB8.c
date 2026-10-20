/* func_00121AB8 = __muldi3 (libgcc2.c, module L_muldi3), 96 bytes, boot.

   Verbatim libgcc2.c source plus the union vocabulary, built with the SDK's
   unmodified Sony EE-GCC 2.9-ee through its driver (config/cflags.txt
   "sdk -O2 -G2 -ffunction-sections"). This replaces a hand-written synthesis
   whose note blamed the compiler - "both compilers turn a*b into a jal
   __muldi3" was the *game* compiler's behaviour, not this module's.

   Measured 2026-10-15: same size, same instruction sequence. The residue is
   one allocation rotation and one scheduling move - our `(s32)(u >> 32)`
   lands in $v0 (retail $a2), the `mult1` accumulator follows it, `mflo`/`mfhi`
   come out $a3/$a0 against retail's $a0/$v0, and the 0xFFFF mask materialises
   between the `mult1` and the `multu` where retail keeps it after the `or`.
   A split `w.s.high += A; w.s.high += B;` overshoots to 28 instructions and
   crosses into func_00121B20, so the single `+=` stays verbatim.

   Provenance: libgcc2.c and longlong.h from the GCC revision rac1-decomp
   pinned (31cf01446d, 1999-09-09), the L_muldi3 module only. GPL 2-or-later
   with the libgcc linking exception; notices as in src/core/001232F0.c. */

/* --- libgcc2.c's type vocabulary (unchanged) --------------------------------------- */

typedef int SItype __attribute__((mode(SI)));
typedef unsigned int USItype __attribute__((mode(SI)));
typedef int DItype __attribute__((mode(DI)));
typedef unsigned int UDItype __attribute__((mode(DI)));

#define BITS_PER_UNIT 8

#define SI_TYPE_SIZE (sizeof(SItype) * BITS_PER_UNIT)

struct DIstruct {
    SItype low, high;
};

typedef union {
    struct DIstruct s;
    DItype ll;
} DIunion;

/* --- longlong.h, the generic __umulsidi3 (this port has no machine one) ------------ */

#define __umulsidi3(u, v) ((UDItype)(USItype)(u) * (USItype)(v))

/* --- libgcc2.c, the L_muldi3 part (renamed: __muldi3 is func_00121AB8) ------------- */

DItype func_00121AB8(DItype u, DItype v) {
    DIunion w;
    DIunion uu, vv;

    uu.ll = u,
    vv.ll = v;

    w.ll = __umulsidi3(uu.s.low, vv.s.low);
    w.s.high += ((USItype)uu.s.low * (USItype)vv.s.high
                 + (USItype)uu.s.high * (USItype)vv.s.low);

    return w.ll;
}
