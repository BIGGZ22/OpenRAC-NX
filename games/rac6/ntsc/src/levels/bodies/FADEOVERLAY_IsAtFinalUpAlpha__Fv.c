/* FADEOVERLAY_IsAtFinalUpAlpha__Fv -- FE.REL 0x027528, 32 bytes.
 *
 * Forwards the question to the global fade overlay object.
 *
 * Evidence:
 *   L'appelee prend l'adresse de l'instance globale de tFADEOVERLAY
 *   (adresse module 0x144C40, deduite des fixups HI16/LO16).
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

extern tFADEOVERLAY g_FADEOVERLAY;

extern int FADEOVERLAYOBJ_IsAtFinalUpAlpha__FP12tFADEOVERLAY(tFADEOVERLAY *self);

void FADEOVERLAY_IsAtFinalUpAlpha__Fv(void)
{
    FADEOVERLAYOBJ_IsAtFinalUpAlpha__FP12tFADEOVERLAY(&g_FADEOVERLAY);
}
