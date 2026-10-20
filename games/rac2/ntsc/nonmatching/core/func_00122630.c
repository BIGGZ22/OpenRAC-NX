/* from src/core/001213B8.c: 304 bytes against retail's 300, one instruction too
 * many, and nothing else differs.
 *
 * libgcc dp-bit.o: __pack_d, transcribed from GCC's fp-bit.c in its double
 * configuration (the single-precision build of the same file is vendored and
 * matched in src/core/001232F0.c; body-writing rules: NGARDS 8, GARDROUND
 * 0x7f, GARDMASK 0xff, GARDMSB 0x80, EXPMAX 0x7ff, QUIET_NAN
 * 0x8000000000000LL, EXPBIAS 1023, FRACBITS 52, NORMAL_EXPMIN -1022,
 * IMPLICIT_2 1<<61). Every instruction matches except the materialisation of
 * the exponent field's clear mask 0x800FFFFFFFFFFFFF at +0xF0:
 *
 *   retail   lui  $v0,0x800F ; ori $v0,$v0,0xFFFF ; dsll 16 ; ori 0xFFFF ;
 *            dsll 16 ; ori 0xFFFF                        (6 instructions)
 *   ours     ori  $v0,$zero,0x800F ; dsll 16 ; ori 0xFFFF ; dsll 16 ;
 *            ori 0xFFFF ; dsll 16 ; ori 0xFFFF           (7 instructions)
 *
 * Same value, one instruction more. This is the assembler, not the source:
 * cc1 emits one 64-bit immediate and gas's load_register expands it. The
 * qualified tree carries patch 0052 (r5900-dli-retail-general), whose rule
 * materialises a 64-bit constant top-down in 16-bit fields; for this value
 * the highest field is 0x800F at bit 48, giving the ori-first form above.
 * Retail's form is what the *unpatched* generic path below that rule
 * produces: it loads hi32 = 0x800FFFFF with lui+ori, then dsll 16/ori for
 * each remaining 16-bit field, which for lo32 = 0xFFFFFFFF is exactly
 * retail's six instructions.
 *
 * What would unblock it: a narrow exception in patch 0052's guard for
 * constants whose low 32 bits are all ones (or one that restores the generic
 * tail when the low half is 0xffffffff), a maintainer-side change to
 * tools/compiler. Both qualified profiles (-O2 -G0 -ffunction-sections and
 * sdk -O2 -G2 -ffunction-sections) produce the same 304 bytes here, so the
 * profile is not the lever.
 *
 * Verified: tools/check.py func_00122630 --diff: boot: 304 bytes, expected
 * 300; exactly one differing instruction.
 */

typedef float DFtype __attribute__ ((mode (DF)));

typedef unsigned int UHItype __attribute__ ((mode (HI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));

typedef UDItype fractype;
typedef USItype halffractype;
typedef DFtype FLO_type;

typedef enum
{
  CLASS_SNAN,
  CLASS_QNAN,
  CLASS_ZERO,
  CLASS_NUMBER,
  CLASS_INFINITY
} fp_class_type;

typedef struct
{
  fp_class_type class;
  unsigned int sign;
  int normal_exp;

  union
    {
      fractype ll;
      halffractype l[2];
    } fraction;
} fp_number_type;

typedef union
{
  FLO_type value;
  fractype value_raw;

  struct
    {
      fractype fraction:52  __attribute__ ((packed));
      unsigned int exp: 11  __attribute__ ((packed));
      unsigned int sign:1 __attribute__ ((packed));
    }
  bits;
}
FLO_union_type;

static __inline__ int fp_pack_isnan(fp_number_type *number) {
    return number->class == CLASS_SNAN || number->class == CLASS_QNAN;
}
static __inline__ int fp_pack_isinf(fp_number_type *number) {
    return number->class == CLASS_INFINITY;
}
static __inline__ int fp_pack_iszero(fp_number_type *number) {
    return number->class == CLASS_ZERO;
}

FLO_type func_00122630(fp_number_type *src) {
    FLO_union_type dst;
    fractype fraction = src->fraction.ll;
    int sign = src->sign;
    int exp = 0;

    if (fp_pack_isnan(src)) {
        exp = 2047;
        fraction |= 0x8000000000000LL;
    } else if (fp_pack_isinf(src)) {
        exp = 2047;
        fraction = 0;
    } else if (fp_pack_iszero(src)) {
        exp = 0;
        fraction = 0;
    } else if (fraction == 0) {
        exp = 0;
    } else {
        if (src->normal_exp < -1022) {
            int shift = -1022 - src->normal_exp;
            exp = 0;
            if (shift > 56)
                fraction = 0;
            else
                fraction >>= shift;
            fraction >>= 8;
        } else if (src->normal_exp > 1023) {
            exp = 2047;
            fraction = 0;
        } else {
            exp = src->normal_exp + 1023;
            if ((fraction & 255) == 128) {
                if (fraction & 256)
                    fraction += 128;
            } else {
                fraction += 127;
            }
            if (fraction >= ((fractype)1 << 61)) {
                fraction >>= 1;
                exp += 1;
            }
            fraction >>= 8;
        }
    }
    dst.bits.fraction = fraction;
    dst.bits.exp = exp;
    dst.bits.sign = sign;
    return dst.value;
}
