/* KillEffect__4MaskP4Moby -- LEVEL18.REL 0x0C4618, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail : jr $ra ; sw $zero, 0x54($a0)
 *   Le drapeau "effet tue" est un int a l'offset 0x54 de Mask ; le second
 *   argument (Moby *) n'est pas utilise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Mask {
    unsigned char field_00[0x54];
    unsigned int effect_killed;
} Mask;

void KillEffect__4MaskP4Moby(Mask *self, void *moby)
{
    (void)moby;
    self->effect_killed = 0;
}
