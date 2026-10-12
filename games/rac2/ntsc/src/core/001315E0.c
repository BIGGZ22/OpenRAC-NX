#include "common.h"
#include "structs.h"

extern signed char r1_D_001331D8[];

extern char *func_00131540(void);

extern void r1_func_00118CF0(void *);

extern short r1_D_001331D0 NOT_SDA;

extern unsigned char r1_D_001331D6[];

extern void r1_func_00118DC0(void *, int, int);

extern int r1_func_00118DC0_i(void *, int, int) __asm__("r1_func_00118DC0");

extern int func_00131730(int);

extern int func_00131760(int);

/* rac1-decomp src/core/0012CC90.c: func_0012D340 */
int func_001315E0(void) {
    if (r1_D_001331D8[0] == 0) {
        func_00131540();
    }
    return r1_D_001331D8[4] == 0x54;
}

INCLUDE_ASM("asm/core", func_00131620);

INCLUDE_ASM("asm/core", func_00131668);

/* rac1-decomp src/core/0012CC90.c: func_0012D3F0 */
/* `r` assigned in both arms and returned once: a direct `return
   r1_D_001331D0;` loads straight into $v0 and moves the address to $v1. */
int func_00131670(void) {
    unsigned int local;
    unsigned int bits;
    int r;

    if (func_001315E0() != 0) {
        r = r1_D_001331D0;
    } else {
        r1_func_00118CF0(&local);
        bits = (local >> 13) & 7;
        if (bits == 0) {
            r = 0x21C;
        } else {
            r = (int)local >> 21;
        }
    }
    return r;
}

INCLUDE_ASM("asm/core", func_001316C0);

/* rac1-decomp src/core/0012CC90.c: func_0012D448 */
/* One return, and r1_func_00118DC0 returns int. */
int func_001316C8(void) {
    unsigned char buf[16];
    int r;

    if (func_001315E0() != 0) {
        r = r1_D_001331D6[0];
    } else {
        r1_func_00118CF0(buf);
        if (((*(unsigned int *)buf >> 13) & 7) == 0) {
            r = 0;
        } else {
            r1_func_00118DC0_i(buf + 4, 1, 1);
            r = (buf[4] >> 4) & 1;
        }
    }
    return r;
}

/* rac1-decomp src/core/0012CC90.c: func_0012D4B0 */
int func_00131730(int arg0) {
    unsigned int v = arg0 & 0xFF;
    return ((v / 10) * 6 + v) & 0xFF;
}

/* rac1-decomp src/core/0012CC90.c: func_0012D4E0 */
/*
 * REVERTED (size mismatch: ours 28 bytes, retail 32). Semantics certain
 * -- BCD byte to binary:
 *
 *   int func_00131760(int arg0) {
 *       unsigned int v = arg0 & 0xFF;
 *       return (v - (v >> 4) * 6) & 0xFF;
 *   }
 *
 * (`unsigned` matters: it gives retail's `srl`, not `sra`.) Everything
 * matches except the multiply: retail uses the generic two-operand
 * `mult $0,$3,$4` followed by `mflo $3`, while this compiler uses the
 * EE three-operand `mult $v1,$v1,$a0` which writes the result directly
 * and needs no mflo -- one instruction fewer, hence 4 bytes short. That
 * is an ISA/codegen choice, not something the source can steer.
 */
/* BCD byte to binary. The product goes through an unsigned char: combine
   folds the multiply into that narrowing copy and re-recognises it as the
   generic LO-destination multiply, which is retail's `mult $0,a,b` +
   `mflo` (the plain int form picks the R5900 three-operand mult). */
/* The product goes through an unsigned char local: that gives retail's
   generic mult $0 + mflo instead of the three-operand mult. */
int func_00131760(int arg0) {
    unsigned char v = arg0;
    unsigned char t = (v >> 4) * 6;
    return (unsigned char)(v - t);
}
