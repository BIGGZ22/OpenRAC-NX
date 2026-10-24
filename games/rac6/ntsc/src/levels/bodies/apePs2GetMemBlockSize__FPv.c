/* apePs2GetMemBlockSize__FPv -- FE.REL 0x000AC9E0, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La taille du bloc est lue par le gestionnaire de tas : le mot place juste
 *   avant le bloc ($a0 - 4) est la taille, arrondie a 4 octets.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern unsigned int dl_mspace_get_block_size__17ApePs2MemHeapCodePv(void *block);

unsigned int apePs2GetMemBlockSize__FPv(void *block)
{
    return dl_mspace_get_block_size__17ApePs2MemHeapCodePv(block);
}
