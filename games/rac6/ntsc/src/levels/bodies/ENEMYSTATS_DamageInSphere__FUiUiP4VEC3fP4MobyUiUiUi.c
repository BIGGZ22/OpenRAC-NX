/* ENEMYSTATS_DamageInSphere__FUiUiP4VEC3fP4MobyUiUiUi -- LEVEL1.REL 0x00162718, 176 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ENEMYSTATS_DamageInSphere__FUiUiP4VEC3fP4MobyUiUiUi -- LEVEL1.REL 0x00162718,
 *   176 bytes.
 *   Body translated from the retail module.
 *   Evidence (retail, LEVEL1.REL 0x00162718):
 *   addiu $sp,-0x50 / lui $v0,0x3B / sd $s1 / daddu $s1,$a2 / sd $s2 / sd $s3
 *   daddu $s2,$a3 / sd $s4 / daddu $s3,$t0 / sd $s5 / daddu $s4,$t1
 *   swc1 $f20,0x40($sp) / daddu $s5,$t2 / lw $a2,0x166C($v0) / sd $s0 / sd $ra
 *   jal ENEMYSTATS_GetAttackData__FUiUiUi / mov.s $f20,$f12
 *   jal GLOBALVARS_IsInReplayMode__Fv / daddu $s0,$v0,$zero
 *   addiu $v1,$zero,4 / lw $t2,0xC($s0) / movz $v1,$zero,$v0
 *   lw $t1,0x10($s0) / addu $s0,$s0,$v1 / daddu $a0,$s1 / lwc1 $f13,0x0($s0)
 *   mov.s $f12,$f20 / daddu $a1,$s2 / daddu $a2,$s3 / daddu $a3,$s4
 *   daddu $t0,$s5 / jal GU_DamageInSphere / daddu $t3,$zero,$zero
 *   The attack data of the moby type is a table of four-byte words; the float
 *   of the table is taken at index 0, or at index 1 when the game is in replay
 *   mode.  The damage type and the damage strength are the third and the
 *   fourth words of the table.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
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

typedef struct Moby Moby;
typedef int MobyDamageType;
typedef int MobyDamageStrength;

/* The global of the module that holds the third argument of the attack
 * data lookup. */
extern unsigned int D_003B166C;

extern void *ENEMYSTATS_GetAttackData__FUiUiUi(unsigned int mobyType,
                                                unsigned int attackType,
                                                unsigned int index);
extern int GLOBALVARS_IsInReplayMode__Fv(void);
extern void GU_DamageInSphere__FPC4VEC3ffP4MobyUiUiUi14MobyDamageType18MobyDamageStrengthUi(
    VEC3 *pos, float radius, float damage, Moby *moby, unsigned int damageId,
    unsigned int damageFlags, unsigned int damageAmount, MobyDamageType type,
    MobyDamageStrength strength, unsigned int extra);

void ENEMYSTATS_DamageInSphere__FUiUiP4VEC3fP4MobyUiUiUi(
    unsigned int mobyType, unsigned int attackType, VEC3 *pos, float damage,
    Moby *moby, unsigned int damageId, unsigned int damageFlags,
    unsigned int damageAmount)
{
    unsigned int *data = (unsigned int *)ENEMYSTATS_GetAttackData__FUiUiUi(
        mobyType, attackType, D_003B166C);
    unsigned int off = GLOBALVARS_IsInReplayMode__Fv() ? 4 : 0;
    unsigned int *slot = (unsigned int *)((char *)data + off);

    GU_DamageInSphere__FPC4VEC3ffP4MobyUiUiUi14MobyDamageType18MobyDamageStrengthUi(
        pos, damage, *(float *)slot, moby, damageId, damageFlags, damageAmount,
        data[4], data[3], 0);
}
