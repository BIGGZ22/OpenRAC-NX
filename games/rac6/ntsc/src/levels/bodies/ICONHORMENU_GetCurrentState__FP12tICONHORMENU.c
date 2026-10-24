/* ICONHORMENU_GetCurrentState__FP12tICONHORMENU -- LEVEL13.REL 0x0B3730, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   8 octets.
 *   La valeur de retour vit dans le slot de delai du jr $ra : le champ est
 *   lu apres la preparation du retour.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tICONHORMENU tICONHORMENU;

struct tICONHORMENU {
    unsigned int reserved_000[50];
    unsigned int current_state;
};

unsigned int ICONHORMENU_GetCurrentState__FP12tICONHORMENU(tICONHORMENU *self)
{
    unsigned int state;

    state = self->current_state;
    return state;
}
