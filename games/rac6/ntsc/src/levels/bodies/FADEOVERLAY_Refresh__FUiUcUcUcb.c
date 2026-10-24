/* FADEOVERLAY_Refresh__FUiUcUcUcb -- FE.REL 0x0274B0, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   FADEOVERLAY_Refresh__FUiUcUcUcb  (FE.REL 0x0274B0, 64 bytes)
 *   Retail:  daddu $t2,$a0,$zero / daddu $t1,$t0,$zero / addiu $sp,-0x10
 *   andi $v0,$a1,0xFF / andi $v1,$a2,0xFF / andi $t0,$a3,0xFF
 *   lui $a0,0x14 / sd $ra / addiu $a0,$a0,0x4C40 / daddu $a1,$t2
 *   daddu $a2,$v0 / jal FADEOVERLAYOBJ_Refresh / daddu $a3,$v1
 *   ld $ra / jr $ra / addiu $sp,0x10
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tFADEOVERLAY tFADEOVERLAY;

extern char D_00144C40[];

extern void FADEOVERLAYOBJ_Refresh__FP12tFADEOVERLAYUiUcUcUcb(tFADEOVERLAY *overlay,
                                                                 unsigned int id,
                                                                 unsigned char red,
                                                                 unsigned char green,
                                                                 unsigned char blue,
                                                                 int flag);

void FADEOVERLAY_Refresh__FUiUcUcUcb(unsigned int id, unsigned char red,
                                     unsigned char green, unsigned char blue,
                                     int flag)
{
    FADEOVERLAYOBJ_Refresh__FP12tFADEOVERLAYUiUcUcUcb((tFADEOVERLAY *)&D_00144C40,
                                                      id, red, green, blue,
                                                      flag);
}
