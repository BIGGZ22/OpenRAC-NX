/* Spotlight_RayCheck__FPC4VEC3T0fP4Moby -- LEVEL1.REL 0x00116318, 152 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Spotlight_RayCheck__FPC4VEC3T0fP4Moby -- LEVEL1.REL 0x00116318, 152 bytes.
 *   Body translated from the retail module.
 *   Evidence (retail disassembly, 0x00116318..0x001163AC):
 *   addiu $sp,$sp,-0x50 / daddu $a3,$a1 / sd $s0,0x30($sp) / daddu $a1,$sp
 *   swc1 $f20,0x40($sp) / daddu $s0,$a2 / mov.s $f20,$f12 / daddu $a2,$a0
 *   sd $ra,0x38($sp) / addiu $a0,$sp,0x20 / sw $zero,0x20($sp)
 *   addiu $t0,$zero,0x28 / daddu $t1,$zero / daddu $t2,$zero
 *   jal MOBYCOLL_CollideRayWithWorld / daddu $t3,$zero
 *   beqz $v0 / lw $v1,0x14($sp) / addiu $v0,$zero,2 / bne $v1,$v0
 *   lwc1 $f0,0x20($sp) / lw $v0,0x10($sp) / beq $v0,$s0 / addiu $v0,$zero,1
 *   lui $at,0x3E99 / ori $at,$at,0x999A / mtc1 $at,$f1 / sub.s $f0,$f20,$f0
 *   c.le.s $f0,$f1 / nop / bc1f / daddu $v0,$zero,$zero / addiu $v0,$zero,1
 *   ld $s0 / ld $ra / lwc1 $f20 / jr $ra / addiu $sp,$sp,0x50
 *   The routine throws a ray of length `f` from `pos` and asks the world
 *   collision for the surface it meets; the collision answer is written in the
 *   local surface block (sp+0x00) and the hit distance in the local float
 *   (sp+0x20).  The general purpose registers are saved before the floating
 *   point one: the retail frame order (sd $s0, sd $ra, then swc1 $f20).
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

typedef struct Moby {
    char pad_00[0x80];
} Moby;

typedef struct CollSurfaceInfo {
    char pad_00[0x10];
    Moby *f_10;
    int f_14;
    char pad_18[0x08];
} CollSurfaceInfo;

extern int MOBYCOLL_CollideRayWithWorld__FPfP15CollSurfaceInfoPC4VEC3T2fiiiUi(
    float *outDistance, CollSurfaceInfo *outInfo, VEC3 *origin, void *owner,
    float length, int flags, int arg7, int arg8, unsigned int arg9);

int Spotlight_RayCheck__FPC4VEC3T0fP4Moby(VEC3 *origin, void *owner, float length,
                                          Moby *moby)
{
    CollSurfaceInfo info;
    float distance = 0.0f;

    int hit = MOBYCOLL_CollideRayWithWorld__FPfP15CollSurfaceInfoPC4VEC3T2fiiiUi(
        &distance, &info, origin, owner, length, 0x28, 0, 0, 0);

    if (hit != 0 && !(info.f_14 == 2 && info.f_10 == moby)) {
        if (!(length - distance <= 0.3f))
            return 0;
    }
    return 1;
}
