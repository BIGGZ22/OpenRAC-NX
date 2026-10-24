/* GLOBAL_GetFlagBit__FUiUc_LEVEL10_035CA0 -- LEVEL10.REL 0x035CA0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le drapeau "index" est un octet a l'offset 0x4E0 de l'entree de la table
 *   globale designee par pGV (0x255030) ; le bit teste est "bit".
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern unsigned char *pGV;

unsigned int GLOBAL_GetFlagBit__FUiUc(unsigned int index, unsigned char bit)
{
    unsigned char *flags = pGV + index;

    return flags[0x4E0] & (1u << bit);
}
