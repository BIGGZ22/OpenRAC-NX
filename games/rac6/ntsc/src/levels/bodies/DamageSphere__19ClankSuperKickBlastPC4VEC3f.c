/* DamageSphere__19ClankSuperKickBlastPC4VEC3f -- LEVEL1.REL 0x000C4CF8, 224 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   DamageSphere__19ClankSuperKickBlastPC4VEC3f -- LEVEL1.REL 0x000C4CF8, 224 bytes.
 *   Body translated from the retail module.
 *   DamageSphere__19ClankSuperKickBlastPC4VEC3f  (LEVEL1.REL 0x000C4CF8, 224 octets)
 *   Retail: addiu $sp,-0xB0 / sd $s0,0x80 / swc1 $f21,0xA8 / daddu $s0,$a1
 *   swc1 $f20,0xA0 / mov.s $f21,$f12 / sd $s1,0x88 / sd $s2,0x90
 *   sd $ra,0x98 / lui $at,0x4000 / mtc1 $at,$f20
 *   lw $v0,0x4($a0) / addiu $a0,$zero,0x27
 *   jal GADGET_PlayerHasGadget__FUi / lw $s2,0x10($v0)
 *   beqz $v0 / nop / lui $at,0x4040 / mtc1 $at,$f20
 *   jal PLAYER_GetBaseMeleeDamage__FP6PLAYER / daddu $a0,$s2
 *   mul.s $f20,$f20,$f0 / daddu $a2,$s0 / mov.s $f12,$f21
 *   daddu $a0,$sp / addiu $a1,$zero,0x20 / addiu $a3,$zero,0x69
 *   addiu $t0,$zero,0x1 / ori $t1,$zero,0xAE3E
 *   jal MOBYCOLL_FindMobysWithSphere__FPP4MobyiPC4VEC3fiii25MobyFormCollisionCategory
 *   addiu $t2,$zero,0x1 / blez $v0 / daddu $s1,$v0
 *   daddu $s0,$sp / lw $a0,0x0($s0) / lw $v0,0x40($a0)
 *   lw $v0,0x20($v0) / beqz $v0 / mov.s $f12,$f20
 *   addiu $a1,$zero,0x2 / addiu $a2,$zero,0x3
 *   addiu $a3,$s2,0x30 / addiu $t0,$s2,0x20 / jalr $v0
 *   daddu $t1,$a0 / addiu $s1,$s1,-0x1 / bnez $s1 / addiu $s0,$s0,0x4
 *   ld $s0 / ld $s1 / ld $s2 / ld $ra / lwc1 $f21 / lwc1 $f20 / jr $ra
 *   The blast sphere damages every moby of the collision category around the
 *   given point.  The multiplier is 2, or 3 when the player carries the gadget
 *   0x27, and it scales the base melee damage of the player; both the
 *   multiplier and the radius live across the two calls, so they are kept in
 *   the callee-saved registers $f20 and $f21.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the gprfirst route (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this
 *   source to the retail bytes of that placement, byte for byte.  The program-
 *   level gate is still pending, so the body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ClankSuperKickBlast ClankSuperKickBlast;
typedef struct Player Player;
typedef struct PlayerHolder PlayerHolder;
typedef struct Moby Moby;
typedef struct MobyType MobyType;
typedef struct VEC3 VEC3;

struct VEC3 {
    float x;
    float y;
    float z;
};

struct Player {
    char pad_00[0x20];
    void *f_20;
    char pad_24[0xC];
    void *f_30;
};

struct PlayerHolder {
    char pad_00[0x10];
    Player *f_10;
};

struct ClankSuperKickBlast {
    char pad_00[4];
    PlayerHolder *f_04;
};

struct Moby {
    char pad_00[0x40];
    MobyType *f_40;
};

struct MobyType {
    char pad_00[0x20];
    void (*f_20)(float damage, Moby *moby, int kind, int count, void *a,
                 void *b, Moby *self);
};

extern int GADGET_PlayerHasGadget__FUi(unsigned int gadget);
extern float PLAYER_GetBaseMeleeDamage__FP6PLAYER(Player *player);
extern int MOBYCOLL_FindMobysWithSphere__FPP4MobyiPC4VEC3fiii25MobyFormCollisionCategory(
    Moby **found, int maxCount, VEC3 *center, float radius, int kind,
    int count, int flags, int category);

void DamageSphere__19ClankSuperKickBlastPC4VEC3f(ClankSuperKickBlast *self,
                                                 VEC3 *center, float radius)
{
    Moby *found[32];
    Player *player = self->f_04->f_10;
    float damage;
    int count;
    int i;

    damage = GADGET_PlayerHasGadget__FUi(0x27) ? 3.0f : 2.0f;
    damage = damage * PLAYER_GetBaseMeleeDamage__FP6PLAYER(player);

    count =
        MOBYCOLL_FindMobysWithSphere__FPP4MobyiPC4VEC3fiii25MobyFormCollisionCategory(
            found, 32, center, radius, 0x69, 1, 0xAE3E, 1);

    for (i = 0; i < count; i++) {
        Moby *moby = found[i];

        if (moby->f_40->f_20) {
            moby->f_40->f_20(damage, moby, 2, 3, &player->f_30,
                             &player->f_20, moby);
        }
    }
}
