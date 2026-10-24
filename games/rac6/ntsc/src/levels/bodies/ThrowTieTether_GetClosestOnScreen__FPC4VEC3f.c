/* ThrowTieTether_GetClosestOnScreen__FPC4VEC3f -- LEVEL1.REL 0x0010F978, 256 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ThrowTieTether_GetClosestOnScreen__FPC4VEC3f -- LEVEL1.REL 0x0010F978,
 *   256 bytes.
 *   Body translated from the retail module.
 *   Evidence (retail disassembly, 0x0010F978..0x0010FA74):
 *   addiu $sp,$sp,-0x50 / lui $a1,0x27 / sd $s1,0x8($sp) / lui $v1,0x27
 *   lw $s1,-0x34E4($v1) / lw $v0,-0x34E0($a1) / sd $s2 / sd $s3
 *   daddu $s2,$a0 / daddu $s3,$zero / swc1 $f20,0x40($sp) / daddu $s5,$a1
 *   sd $s0 / mul.s $f20,$f12,$f12 / sd $s4 / sltu $v0,$s1,$v0 / beqz $v0
 *   addiu $s4,$zero,1 / addiu $s0,$s1,0x30 / lw $v0,0x34($s0) / andi $v0,0x431
 *   bne $v0,$s4 / jal GU_IsOnScreen / beqz $v0 / lwc1 ... / sub.s / mul.s
 *   add.s / c.le.s $f0,$f20 / bc1tl / daddu $s3,$s1 / addiu $s1,0x80
 *   sltu $v0,$s1,$v0 / bnez / addiu $s0,0x80 / daddu $v0,$s3 / ld ... / jr $ra
 *   The routine walks the list of tether cables (two global words: the cursor
 *   at 0x26FCB1C and its limit at 0x26FCB20), keeps the ones that are flagged
 *   and on screen, and returns the last one whose distance to `pos` is not
 *   inside the squared radius.  The general purpose registers are saved before
 *   the floating point one ($f20 carries the squared radius across the call).
 *   Placement: module-relative offset of the disc module.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct VEC3 {
    float x;
    float y;
    float z;
} VEC3;

typedef struct ThrowTieTetherCable {
    char pad_00[0x30];
    VEC3 pos;
    char pad_3C[0x28];
    unsigned int flags;
    char pad_68[0x18];
} ThrowTieTetherCable;

extern void *D_26CB1C;
extern void *D_26CB20;

extern int GU_IsOnScreen__FPC4VEC3(VEC3 *pos);

void *ThrowTieTether_GetClosestOnScreen__FPC4VEC3f(VEC3 *pos, float radius)
{
    ThrowTieTetherCable *cable = (ThrowTieTetherCable *)D_26CB1C;
    void *best = 0;
    float radius2 = radius * radius;

    while (cable < (ThrowTieTetherCable *)D_26CB20) {
        if ((cable->flags & 0x431) == 1) {
            VEC3 *at = &cable->pos;

            if (GU_IsOnScreen__FPC4VEC3(at)) {
                float dx = pos->x - at->x;
                float dy = pos->y - at->y;
                float dz = pos->z - at->z;
                float distance2 = dx * dx + dy * dy + dz * dz;

                if (distance2 <= radius2)
                    best = cable;
            }
        }
        cable++;
    }
    return best;
}
