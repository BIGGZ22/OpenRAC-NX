/* GLOBALVARS_UpdateGameVolumeFromTempSettings__Fv -- FE.REL 0x00009598, 96 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   GLOBALVARS_UpdateGameVolumeFromTempSettings__Fv -- FE.REL 0x009598, 96 bytes.
 *   Body translated from the retail module.
 *   GLOBALVARS_UpdateGameVolumeFromTempSettings__Fv  (FE.REL 0x009598, 96 octets)
 *   Retail: addiu $sp,-0x20 / sd $s0,0x0 / lui $s0,0xF / sd $ra,0x8
 *   swc1 $f20,0x10 / lw $v0,0x4D8($s0) / lui $at,0x437F / mtc1 $at,$f20
 *   lwc1 $f12,0x20($v0) / cvt.s.w $f12,$f12 / div.s $f12,$f12,$f20
 *   jal SOUND_SetGlobalVolume__Ff / nop
 *   lw $v0,0x4D8($s0) / lwc1 $f12,0x1C($v0) / cvt.s.w / div.s
 *   jal SNDSTREAM_SetGlobalVolume__Ff11STREAM_TYPE
 *   daddu $a0,$zero,$zero
 *   The two volumes live in the GLOBALVARS structure as integers (0..255) and
 *   are divided by 255 to become the 0..1 floats the sound system expects.
 *   $f20 (255.0f) is live across the first call: it is saved in the frame.
 *   Placement: module-relative offset of the disc module; the other
 *   placements of the same body are listed in config/levels/bodies.tsv.
 *   Proof: the route gprfirst + contextual division padding
 *   (gnu-ee-20261015-gprfirst + divpad-contextuel:-O2 -G0) compiles this source
 *   to the retail bytes of that placement, byte for byte.  The program-level
 *   gate is still pending, so the body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gprfirst-divctx:-O2 -G0) compiles this source to the
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

extern void SOUND_SetGlobalVolume__Ff(float volume);
extern void SNDSTREAM_SetGlobalVolume__Ff11STREAM_TYPE(float volume,
                                                       int streamType);

void GLOBALVARS_UpdateGameVolumeFromTempSettings__Fv(void)
{
    SOUND_SetGlobalVolume__Ff((float)pGV->fxVolume / 255.0f);
    SNDSTREAM_SetGlobalVolume__Ff11STREAM_TYPE((float)pGV->musicVolume / 255.0f,
                                                0);
}
