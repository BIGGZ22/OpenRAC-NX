/* CHEAT_IsActive__FUi_LEVEL1_038C88 -- LEVEL1.REL 0x038C88, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail: reads the global pGV, tests the bit mask at offset 0x8E0 of the
 *   object it points to against the bit of the cheat index.
 *   The bit mask is built in a local: that keeps the compiler from folding
 *   the test into a single bit extraction (the retail keeps the full mask).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GameVars {
    unsigned char field_000[0x8E0];
    unsigned int active_cheats;
} GameVars;

extern GameVars *pGV;

int CHEAT_IsActive__FUi(unsigned int cheat)
{
    unsigned int bit = 1u << cheat;

    return (pGV->active_cheats & bit) != 0;
}
