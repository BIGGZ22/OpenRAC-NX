/* GLOBAL_GetFlagBitU32__FUiUi_LEVEL11_035D80 -- LEVEL11.REL 0x035D80, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Variante 32 bits : le masque est applique a un mot lu a l'octet "index"
 *   de la table des drapeaux.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GV GV;

struct GV {
    unsigned char unknown_000[0x4E0];
    unsigned char flags[1];
};

extern GV *pGV;

unsigned int GLOBAL_GetFlagBitU32__FUiUi(unsigned int index, unsigned int bit)
{
    unsigned int mask = 1u << bit;

    return *(unsigned int *)(pGV->flags + index) & mask;
}
