/* ICONMENU_SetNumberVisibleIconSlots__FP9tICONMENUUi -- LEVEL13.REL 0x0B15E0, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   8 octets.
 *   L'ecriture vit dans le slot de delai du jr $ra : la valeur du parametre
 *   est la derniere instruction produite.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tICONMENU tICONMENU;

struct tICONMENU {
    unsigned int reserved_000;
    unsigned int reserved_004;
    unsigned int number_visible_icon_slots;
};

void ICONMENU_SetNumberVisibleIconSlots__FP9tICONMENUUi(tICONMENU *self,
                                                       unsigned int count)
{
    self->number_visible_icon_slots = count;
}
