/* ICONMENU_SetIconScrollSpeed__FP9tICONMENUf -- LEVEL14.REL 0x0B2540, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   8 octets.
 *   L'ecriture du flottant est dans l'emplacement de delai du `jr $ra`.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tICONMENU {
    unsigned int reserved_00;
    unsigned int reserved_04;
    unsigned int reserved_08;
    unsigned int reserved_0C;
    unsigned int reserved_10;
    unsigned int reserved_14;
    unsigned int reserved_18;
    float scrollSpeed; /* +0x1C */
} tICONMENU;

void ICONMENU_SetIconScrollSpeed__FP9tICONMENUf(tICONMENU *self, float speed)
{
    self->scrollSpeed = speed;
}
