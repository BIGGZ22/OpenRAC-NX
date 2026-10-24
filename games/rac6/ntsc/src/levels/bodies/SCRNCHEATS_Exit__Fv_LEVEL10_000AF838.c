/* SCRNCHEATS_Exit__Fv_LEVEL10_000AF838 -- LEVEL10.REL 0x000AF838, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le menu des triches quitte l'ecran en jouant le son 7.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef enum HUDSOUND {
    HUDSOUND_7 = 7
} HUDSOUND;

extern void HUDSOUND_Play__F9eHUDSOUND(HUDSOUND sound);

void SCRNCHEATS_Exit__Fv(void)
{
    HUDSOUND_Play__F9eHUDSOUND(HUDSOUND_7);
}
