#include "common.h"

/* Unpack an IEEE double into the runtime's own parts: the sign, the kind
   (2 zero, 3 normal, 4 infinity, 0 signalling NaN, 1 quiet NaN) and, for
   every kind but the zero, the fraction left-justified in the top bits of a
   64-bit word with the implicit one at 2^60. */
typedef struct {
    int kind;
    unsigned int sign;
    int exponent;
    unsigned long fraction;
} DoubleParts;

void func_00122760(double *value, DoubleParts *parts) {
    unsigned long bits = *(unsigned long *) value;
    unsigned int exponent = (unsigned int) (bits >> 52) & 0x7FF;
    unsigned long fraction = bits & 0xFFFFFFFFFFFFFUL;

    parts->sign = (unsigned int) (bits >> 63);
    if (exponent == 0) {
        parts->kind = 2;
        return;
    }
    if (exponent == 0x7FF) {
        if (fraction == 0) {
            parts->kind = 4;
            return;
        }
        if (fraction & 0x8000000000000UL) {
            parts->kind = 1;
        } else {
            parts->kind = 0;
        }
        parts->fraction = fraction;
        return;
    }
    parts->fraction = (fraction << 8) | 0x1000000000000000UL;
    parts->exponent = (int) exponent - 1023;
    parts->kind = 3;
}
/* Near miss: 152 bytes against retail's 156, body instruction for instruction.
   Every instruction matches from +0x38 to the end; the 4-byte gap is a
   standalone `nop` retail has at +0x34, between the exponent==0 return and the
   `bnel` on exponent != 0x7FF (retail also aligns that branch to 8 bytes).
   One more difference: the three stores of the normal path land in different
   slots -- retail stores the exponent before `jr` and the kind in its delay
   slot, this candidate the other way round. Same output under the SDK and the
   game compiler, and under `if (fraction != 0)` and `if (fraction == 0)`. */
