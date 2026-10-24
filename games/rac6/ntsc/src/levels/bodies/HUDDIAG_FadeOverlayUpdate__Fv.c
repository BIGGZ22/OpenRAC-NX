/* HUDDIAG_FadeOverlayUpdate__Fv -- FE.REL 0x037BD8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   HUDDIAG_FadeOverlayUpdate__Fv @ FE.REL 0x037BD8 (32 bytes)
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

extern tFADEOVERLAY D_001B29E0[];
extern void FADEOVERLAYOBJ_Update__FP12tFADEOVERLAY(tFADEOVERLAY *overlay);

void HUDDIAG_FadeOverlayUpdate(void)
{
    FADEOVERLAYOBJ_Update__FP12tFADEOVERLAY(D_001B29E0);
}
