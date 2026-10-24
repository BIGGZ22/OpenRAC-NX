/* GADGET_GetGadgetNameInfo__FUiUi_LEVEL18_034408 -- LEVEL18.REL 0x034408, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   addiu $sp,$sp,-0x10 ; sd $ra,0x0($sp) ; jal func_0334D8 ; nop
 *   ld $ra,0x0($sp) ; addiu $v0,$v0,0x10 ; jr $ra ; addiu $sp,$sp,0x10
 *   L'appel recupere une table de noms et l'appelant renvoie le pointeur
 *   decale de 0x10 octets (l'en-tete de la table).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_0334D8(unsigned int gadget, unsigned int index);

int GADGET_GetGadgetNameInfo__FUiUi(unsigned int gadget, unsigned int index)
{
    return func_0334D8(gadget, index) + 0x10;
}
