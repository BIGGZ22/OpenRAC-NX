/* func_00148A98 -- SCUS_976.23 0x00148A98, 24 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00148A98 (24 octets)
 *   Getter de l'entier stocke a self+0xC8, ecrit dans *out si out != NULL.
 *   Renvoie 0 dans les deux cas (le retour est un daddu 64 bits).
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_00148A98(void *self, int *out)
{
    if (out)
        *out = *(int *)((char *)self + 0xC8);
    return 0;
}
