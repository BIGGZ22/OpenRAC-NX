/* GLOBAL_GetFlagBitU32__FUiUi_LEVEL1_035D88 -- LEVEL1.REL 0x035D88, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Meme table globale que GLOBAL_GetFlagBit, lue cette fois sur un mot ;
 *   l'index reste un decalage en octets depuis le debut du bloc pGV.
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

unsigned int GLOBAL_GetFlagBitU32__FUiUi(unsigned int index, unsigned int bit)
{
    unsigned int base = index + (unsigned int)pGV;

    return *(unsigned int *)(base + 0x4E0) & (1u << bit);
}
