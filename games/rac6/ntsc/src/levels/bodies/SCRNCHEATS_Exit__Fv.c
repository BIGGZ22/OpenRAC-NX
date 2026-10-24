/* SCRNCHEATS_Exit__Fv -- LEVEL14.REL 0x000B0A20, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   SCRNCHEATS_Exit__Fv - LEVEL14.REL 0x000B0A20, 28 bytes.
 *   The exit sound is the seventh of the HUD sound enum.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef unsigned int uint;

extern void HUDSOUND_Play__F9eHUDSOUND(uint sound);

void SCRNCHEATS_Exit__Fv(void)
{
    HUDSOUND_Play__F9eHUDSOUND(7);
}
