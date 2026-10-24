/* RATCHET_ActivateShield__FP7RATCHETP4Moby -- LEVEL14.REL 0x112280, 8 bytes.
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

typedef struct RATCHET RATCHET;
typedef struct Moby Moby;

struct RATCHET {
    unsigned char reserved_000[0xB98];
    Moby *shield;
};

void RATCHET_ActivateShield__FP7RATCHETP4Moby(RATCHET *self, Moby *shield)
{
    self->shield = shield;
}
