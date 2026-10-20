/* from src/core/00124568.c: 148 bytes, 148 compiled; two store orders differ */
/*
 * Builds the two cached handles the rest of the unit reads. The function is
 * void: the `addiu $v0,$zero,1` and `addiu $v1,$zero,1` pairs in retail are the
 * value the buffer stores use (buf[1] = buf[2] = 1), not a return value - the
 * epilogue's $v0 is left holding the last address materialisation, so there is
 * no `return 1` to write. The first of the three calls has its result dropped;
 * the second feeds both caches, which is why retail stores the same $v0 twice.
 *
 * With the unit's SDK profile (`sdk -O2 -G2 -ffunction-sections`) this body is
 * 148 bytes and agrees with retail on the control flow (`beq` to the setup,
 * `bne` to the tail, `b` + `addiu $v0,$zero,1` into the setup, the duplicated
 * `lui $s0,0x13` block), on the three calls, on the relocations and on every
 * `nop`. What differs is the order of two pairs of adjacent stores:
 *
 *   retail   +0x44  sw $v0, 0x4($sp)   then the call's delay slot +0x50
 *                   sw $v0, 0x8($sp), and later +0x5C sw $v0, 0x63A8($s1)
 *                   before +0x60 sw $v0, 0x63AC($s0)
 *   this cc1 the same four stores with buf[2] before buf[1], and the $s0 store
 *                   (0x63AC) before the $s1 one, with the next call's
 *                   `daddu $a0,$sp,$zero` between them
 *
 * Wording tried for the buffer writes (declaration order, `buf[2] = 1` written
 * before/after `buf[1]`, the statement moved across the first call) and for the
 * two caches (direct, through a local): all give one of the two orders, and the
 * wrong one is always the same, so the choice is the scheduler's, not a
 * statement order. `-O2 -G2` without `sdk` is worse (136 bytes).
 */
extern int D_001363A8;

extern int D_001363AC;

extern int D_001363A0;

extern int D_001363B0;

extern int func_0011AC20(void *);

void func_00124780(void) {
    int buf[8];

    if (D_001363A8 != -1 && D_001363AC != -1) {
        return;
    }
    buf[5] = 0;
    buf[1] = 1;
    buf[2] = 1;
    func_0011AC20(buf);
    D_001363A8 = func_0011AC20(buf);
    D_001363AC = D_001363A8;
    buf[2] = 0;
    D_001363A0 = func_0011AC20(buf);
    D_001363B0 = 0;
}
