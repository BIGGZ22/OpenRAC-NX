/* Update__14ClankSuperKickf -- LEVEL1.REL 0x000C4BB8, 164 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Update__14ClankSuperKickf -- LEVEL1.REL 0x000C4BB8, 164 bytes.
 *   Body translated from the retail module.
 *   Update__14ClankSuperKickf  (LEVEL1.REL 0x000C4BB8, 164 octets)
 *   Retail: addiu $sp,-0x30 / sd $s0 / sd $s1 / sd $ra / swc1 $f20
 *   lw $v0,0x4($a0) / lw $s1,0x10($v0)
 *   jal Update__5Punchf / lwc1 $f20,0x1D8($s1)
 *   lw $v1,0x264($s1) / addiu $v0,$zero,-3 / swc1 $f20,0x1D8($s1)
 *   and $v1,$v1,$v0 / sw $v1,0x264($s1)
 *   lbu $v0,-0x2248($a0) / beqz $v0 / addiu $s0,$s1,0x1154
 *   jal AnimTranslator_GetFrameTrans__FP14AnimTranslator
 *   lwc1 $f1,0x4($v0) / lwc1 $f0,0x34($s1) / lw $v0,0x260($s1)
 *   add.s $f0,$f0,$f1 / swc1 $f0,0x34($s1)
 *   ldl $v1,0x37($s1) / ldr $v1,0x30($s1) / ldl $a1,0x3F($s1)
 *   ldr $a1,0x38($s1) / sdl $v1,0x37($v0) / sdr $v1,0x30($v0)
 *   sdl $a1,0x3F($v0) / sdr $a1,0x38($v0)
 *   jal AnimTranslator_ClearFrameTrans__FP14AnimTranslator
 *   ld $s0 / ld $s1 / ld $ra / lwc1 $f20 / jr $ra
 *   The kick state calls the base punch update first, then restores the
 *   animation scale it had read before the call: that float lives across the
 *   call, so it is kept in the callee-saved register $f20.
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

typedef struct ClankSuperKick ClankSuperKick;
typedef struct AnimBody AnimBody;
typedef struct AnimHolder AnimHolder;
typedef struct FrameTrans FrameTrans;
typedef struct AnimTranslator AnimTranslator;

/* The sixteen bytes the state copies are not eight-byte aligned in the
 * object: the retail moves them with the unaligned word pair (ldl/ldr,
 * sdl/sdr), so the block is packed. */
typedef struct __attribute__((packed)) AnimBlock {
    long long lo;
    long long hi;
} AnimBlock;

struct AnimBody {
    char pad_00[0x30];
    union {
        AnimBlock block;
        struct {
            char pad_00[4];
            float f_34;
        } mid;
    } u_30;
    char pad_40[0x198];
    float f_1D8;
    char pad_1DC[0x84];
    AnimBody *f_260;
    int f_264;
};

struct AnimHolder {
    char pad_00[0x10];
    AnimBody *f_10;
};

struct ClankSuperKick {
    char pad_00[4];
    AnimHolder *f_04;
};

struct FrameTrans {
    char pad_00[4];
    float f_04;
};

extern unsigned char g_player_inPrimaryUpdate;

extern void Update__5Punchf(ClankSuperKick *self, float dt);
extern FrameTrans *AnimTranslator_GetFrameTrans__FP14AnimTranslator(
    AnimTranslator *translator);
extern void AnimTranslator_ClearFrameTrans__FP14AnimTranslator(
    AnimTranslator *translator);

void Update__14ClankSuperKickf(ClankSuperKick *self, float dt)
{
    AnimBody *body = self->f_04->f_10;
    float scale = body->f_1D8;

    Update__5Punchf(self, dt);

    body->f_1D8 = scale;
    body->f_264 &= ~2;

    if (g_player_inPrimaryUpdate) {
        AnimTranslator *translator = (AnimTranslator *)((char *)body + 0x1154);
        FrameTrans *trans = AnimTranslator_GetFrameTrans__FP14AnimTranslator(
            translator);
        AnimBody *other = body->f_260;

        body->u_30.mid.f_34 += trans->f_04;
        other->u_30.block = body->u_30.block;
        AnimTranslator_ClearFrameTrans__FP14AnimTranslator(translator);
    }
}
