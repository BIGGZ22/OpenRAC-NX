/* GLOBALVARS_TemporarilyOverrideGlobalGameVolume__Ff -- FE.REL 0x00009400, 124 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   GLOBALVARS_TemporarilyOverrideGlobalGameVolume__Ff -- FE.REL 0x009400, 124 bytes.
 *   Body translated from the retail module.
 *   GLOBALVARS_TemporarilyOverrideGlobalGameVolume__Ff  (FE.REL 0x009400, 124 octets)
 *   Retail: addiu $sp,-0x20 / sd $s0 / lui $s0,0xF / sd $ra / swc1 $f21 / swc1 $f20
 *   lw $v0,0x4D8($s0) / mov.s $f20,$f12 / lui $at,0x437F / mtc1 $at,$f21
 *   lwc1 $f12,0x20($v0) / cvt.s.w / div.s $f12,$f12,$f21
 *   jal SOUND_SetGlobalVolume__Ff / mul.s $f12,$f12,$f20
 *   lw $v0,0x4D8($s0) / daddu $a0,$zero,$zero
 *   lwc1 $f12,0x1C($v0) / cvt.s.w / div.s $f12,$f12,$f21
 *   jal SNDSTREAM_SetGlobalVolume__Ff11STREAM_TYPE / mul.s $f12,$f12,$f20
 *   lui $v1,0xF / addiu $v0,$zero,5
 *   ld $s0 / ld $ra / lwc1 $f21 / lwc1 $f20
 *   sw $v0,0x4DC($v1) / jr $ra
 *   The two volumes of the GLOBALVARS structure are divided by 255 and
 *   multiplied by the given factor; the multiplier lives across the two
 *   calls, so it is kept in the callee-saved register $f20.
 *   Placement: module-relative offset of the disc module; the other
 *   placements of the same body are listed in config/levels/bodies.tsv.
 *   Proof: the gprfirst route (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this
 *   source to the retail bytes of that placement, byte for byte.  The program-
 *   level gate is still pending, so the body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GLOBALVARS GLOBALVARS;

struct GLOBALVARS {
    int pad_00[7];
    int musicVolume;                /* 0x1C */
    int fxVolume;                   /* 0x20 */
};

extern GLOBALVARS *pGV;

/* The retail module keeps this word next to pGV; it has no exported name. */
extern int D_000F04DC;

extern void SOUND_SetGlobalVolume__Ff(float volume);
extern void SNDSTREAM_SetGlobalVolume__Ff11STREAM_TYPE(float volume,
                                                       int streamType);

void GLOBALVARS_TemporarilyOverrideGlobalGameVolume__Ff(float volume)
{
    SOUND_SetGlobalVolume__Ff((float)pGV->fxVolume / 255.0f * volume);
    SNDSTREAM_SetGlobalVolume__Ff11STREAM_TYPE((float)pGV->musicVolume
                                                / 255.0f * volume, 0);
    D_000F04DC = 5;
}
