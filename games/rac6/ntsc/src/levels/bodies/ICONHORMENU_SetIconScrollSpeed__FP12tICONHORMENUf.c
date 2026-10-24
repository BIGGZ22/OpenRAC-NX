/* ICONHORMENU_SetIconScrollSpeed__FP12tICONHORMENUf -- LEVEL13.REL 0x0B21E0, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   8 octets.
 *   Le deuxieme parametre (float) vit dans $f12 ; l'ecriture est dans le
 *   slot de delai du jr $ra.
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
    unsigned int reserved_000[9];
    float icon_scroll_speed;
};

void ICONHORMENU_SetIconScrollSpeed__FP12tICONHORMENUf(tICONHORMENU *self,
                                                      float speed)
{
    self->icon_scroll_speed = speed;
}
