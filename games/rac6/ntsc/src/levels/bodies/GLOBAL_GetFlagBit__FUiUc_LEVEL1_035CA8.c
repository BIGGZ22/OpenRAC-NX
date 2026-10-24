/* GLOBAL_GetFlagBit__FUiUc_LEVEL1_035CA8 -- LEVEL1.REL 0x035CA8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le drapeau est l'octet de la table globale situee a 0x4E0 du bloc designe
 *   par le pointeur global pGV ; l'index est un decalage en octets depuis le
 *   debut de ce bloc.  Le bit teste est le bit "bit" de cet octet.
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

unsigned char GLOBAL_GetFlagBit__FUiUc(unsigned int index, unsigned char bit)
{
    unsigned char *p = (unsigned char *)pGV + index;

    return p[0x4E0] & (unsigned char)(1u << bit);
}
