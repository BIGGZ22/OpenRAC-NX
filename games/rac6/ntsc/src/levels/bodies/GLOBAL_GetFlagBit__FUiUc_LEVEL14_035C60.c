/* GLOBAL_GetFlagBit__FUiUc_LEVEL14_035C60 -- LEVEL14.REL 0x035C60, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   32 octets.
 *   L'adresse est formee en deux temps : `pGV + id` d'abord, le decalage
 *   0x4E0 restant dans l'offset du chargement.
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

unsigned int GLOBAL_GetFlagBit__FUiUc(unsigned int id, unsigned char bit)
{
    return *(unsigned char *)(pGV + id + 0x4E0) & (1 << bit);
}
