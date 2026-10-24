/* GU_CameraHint__FPC4VEC3PifP6PLAYER -- LEVEL10.REL 0x0001DB40, 200 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   GU_CameraHint__FPC4VEC3PifP6PLAYER -- LEVEL10.REL 0x01DB40, 200 bytes.
 *   Body translated from the retail module.
 *   GU_CameraHint__FPC4VEC3PifP6PLAYER  (LEVEL10.REL 0x01DB40, 200 octets)
 *   Retail: addiu $sp,-0x30 / sd $s0,0x0 / sd $s1,0x8 / daddu $s0,$a2
 *   sd $s2,0x10 / daddu $s1,$a1 / swc1 $f20,0x20 / daddu $s2,$a0
 *   sd $ra,0x18 / beqz $s1,.L1 / mov.s $f20,$f12
 *   lw $v0,0x0($s1) / bltz $v0,.Lend / addiu $v0,$zero,1
 *   jal CAMDRIVER_GetFramesSinceInput__Fv / nop
 *   slti $v0,$v0,0x5 / bnez $v0,.Lend / daddu $v0,$zero,$zero
 *   bnez $s0,.L2 / lui $v0,0x28 / lw $s0,0x730($v0)
 *   jal FIRSTPERSON_IsActive__FP6PLAYER / daddu $a0,$s0,$zero
 *   beqz $v0,.L3 / daddu $a0,$s0,$zero
 *   jal FIRSTPERSON_ExitState__FP6PLAYERb / daddu $a1,$zero,$zero
 *   jal CAMDRIVER_EnableControl__Fb / daddu $a0,$zero,$zero
 *   jal CAMERA_GetFov__Fv / nop / neg.s $f13,$f0
 *   daddu $a0,$s2,$zero / mov.s $f12,$f20
 *   jal CAMDRIVER_SetSecondaryTarget__FPC4VEC3ffb / addiu $a1,$zero,1
 *   beqz $s1,.Lend / daddu $v0,$zero,$zero / lw $v0,0x0($s1)
 *   addiu $v0,$v0,-1 / sw $v0,0x0($s1) / daddu $v0,$zero,$zero
 *   ld $s0 / ld $s1 / ld $s2 / ld $ra / lwc1 $f20,0x20($sp)
 *   jr $ra / addiu $sp,$sp,0x30
 *   The hint is only given when the camera has not been moved for five frames;
 *   the counter of the hint is decremented at each call.  When no player is
 *   given, the one of the game is used (the global read at 0x280730).  The
 *   value of the hint lives across the calls, so it is kept in the
 *   callee-saved register $f20.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the route gprfirst (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this
 *   source to the retail bytes of that placement, byte for byte.  The program-
 *   level gate is still pending, so the body is proved, not promoted.
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

typedef struct PLAYER PLAYER;

extern int CAMDRIVER_GetFramesSinceInput__Fv(void);
extern int FIRSTPERSON_IsActive__FP6PLAYER(PLAYER *player);
extern void FIRSTPERSON_ExitState__FP6PLAYERb(PLAYER *player, int state);
extern void CAMDRIVER_EnableControl__Fb(int enable);
extern float CAMERA_GetFov__Fv(void);
extern void CAMDRIVER_SetSecondaryTarget__FPC4VEC3ffb(VEC3 *pos, float a,
                                                       float b, int c);

/* The player of the game; the module reads it at 0x280730. */
extern PLAYER *pPlayer;

int GU_CameraHint__FPC4VEC3PifP6PLAYER(VEC3 *camPos, int *counter, float value,
                                        PLAYER *player)
{
    int frames;
    float fov;

    if (counter != 0) {
        if (*counter < 0)
            return 1;
    }

    frames = CAMDRIVER_GetFramesSinceInput__Fv();
    if (frames < 5)
        return 0;

    if (player == 0)
        player = pPlayer;

    if (FIRSTPERSON_IsActive__FP6PLAYER(player))
        FIRSTPERSON_ExitState__FP6PLAYERb(player, 0);

    CAMDRIVER_EnableControl__Fb(0);
    fov = CAMERA_GetFov__Fv();
    CAMDRIVER_SetSecondaryTarget__FPC4VEC3ffb(camPos, value, -fov, 1);

    if (counter != 0)
        *counter = *counter - 1;

    return 0;
}
