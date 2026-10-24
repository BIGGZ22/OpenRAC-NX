/* Update__9ClankPortf -- LEVEL1.REL 0x000C5EB8, 272 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Update__9ClankPortf -- LEVEL1.REL 0x000C5EB8, 272 bytes.
 *   Body translated from the retail module.
 *   Update__9ClankPortf  (LEVEL1.REL 0x000C5EB8, 272 octets)
 *   Retail: addiu $sp,-0x20 / sd $s0 / sd $ra / swc1 $f20,0x10
 *   lw $v0,0x4($a0) / lw $s0,0xC($v0) / lw $v1,0xC($s0)
 *   beqz $v1 / addiu $v0,$zero,1 / beql $v1,$v0 / lwc1 $f0,0x10($s0)
 *   b / ld $s0
 *   lwc1 $f0,0x10($s0) / lui $at,0x3DCC / ori $at,0xCCCD / mtc1 $at,$f1
 *   sub.s $f0,$f0,$f12 / c.lt.s $f0,$f1 / nop / bc1f
 *   swc1 $f0,0x10($s0) / jal PLAYER_ShowPlayerAll__Fb / daddu $a0,$zero
 *   lwc1 $f0,0x10($s0) / mtc1 $zero,$f20 / nop / c.le.s $f0,$f20
 *   nop / bc1fl / ld $s0
 *   jal PLAYER_SetPosition__FPC4VEC3 / daddu $a0,$s0
 *   lui $v1,0x28 / lui $at,0x3F00 / mtc1 $at,$f0
 *   lw $v0,-0x2280($v1) / addiu $a0,$zero,1
 *   swc1 $f20,0xAC($v0) / addiu $v0,$v0,0xAC / swc1 $f20,0x8($v0)
 *   swc1 $f20,0x4($v0) / swc1 $f0,0x10($s0) / b / sw $a0,0xC($s0)
 *   mtc1 $zero,$f1 / sub.s $f0,$f0,$f12 / c.le.s $f0,$f1 / nop / bc1f
 *   swc1 $f0,0x10($s0) / lui $v0,0x28 / lui $a1,0x2B
 *   lw $v1,-0x2280($v0) / addiu $a1,$a1,-0x4BD8
 *   jal Teleport_Spawn__FP4MobyPC14TeleportConfig / lw $a0,0x260($v1)
 *   jal CONTROLLER_SetActiveContext__FUi / addiu $a0,$zero,2
 *   jal PLAYER_ShowPlayerAll__Fb / addiu $a0,$zero,1
 *   addiu $v0,$zero,2 / sw $v0,0xC($s0)
 *   ld $s0 / ld $ra / lwc1 $f20 / jr $ra
 *   The port state has two steps: the first one pulls the player back to the
 *   port when the timer of the state expires, the second one teleports him
 *   away.  The zero of the state lives across the calls, so it is kept in the
 *   callee-saved register $f20.
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

typedef struct ClankPort ClankPort;
typedef struct PortHolder PortHolder;
typedef struct PortState PortState;
typedef struct Player Player;
typedef struct Moby Moby;
typedef struct VEC3 VEC3;
typedef struct TeleportConfig TeleportConfig;

struct VEC3 {
    float x;
    float y;
    float z;
};

struct PortState {
    VEC3 pos;
    int state;
    float timer;
};

struct PortHolder {
    char pad_00[0xC];
    PortState *f_0C;
};

struct ClankPort {
    char pad_00[4];
    PortHolder *f_04;
};

struct Player {
    char pad_00[0xAC];
    float f_AC;
    float f_B0;
    float f_B4;
    char pad_B8[0x1A8];
    Moby *f_260;
};

extern Player *g_pActivePlayer;

/* The teleport configuration the port uses has no exported name. */
extern TeleportConfig D_002AB428;

extern void PLAYER_ShowPlayerAll__Fb(int show);
extern void PLAYER_SetPosition__FPC4VEC3(VEC3 *position);
extern void Teleport_Spawn__FP4MobyPC14TeleportConfig(Moby *moby,
                                                      TeleportConfig *config);
extern void CONTROLLER_SetActiveContext__FUi(unsigned int context);

void Update__9ClankPortf(ClankPort *self, float dt)
{
    PortState *port = self->f_04->f_0C;
    int state = port->state;

    switch (state) {
    case 0:
        {
            float v = port->timer - dt;

            port->timer = v;
            if (v < 0.1f) {
                PLAYER_ShowPlayerAll__Fb(0);
            }
            if (port->timer <= 0.0f) {
                PLAYER_SetPosition__FPC4VEC3(&port->pos);
                {
                    float *vel = &g_pActivePlayer->f_AC;

                    vel[0] = 0.0f;
                    vel[1] = 0.0f;
                    vel[2] = 0.0f;
                }
                port->state = 1;
                port->timer = 0.5f;
            }
        }
        break;

    case 1:
        {
            float v = port->timer - dt;

            port->timer = v;
            if (v <= 0.0f) {
                Teleport_Spawn__FP4MobyPC14TeleportConfig(
                    g_pActivePlayer->f_260, &D_002AB428);
                CONTROLLER_SetActiveContext__FUi(2);
                PLAYER_ShowPlayerAll__Fb(1);
                port->state = 2;
            }
        }
        break;

    default:
        break;
    }
}
