/* HUDFXCYCLE_Reset__FP11tHUDCYCLEFX -- FE.REL 0x035160, 8 bytes.
 *
 * Resets the cycle effect of the HUD to 0.
 *
 * Evidence:
 *   Le corps met a zero le mot de l'offset 8 de l'argument ; l'ecriture est
 *   placee dans l'emplacement de delai du jr par l'ordonnanceur.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tHUDCYCLEFX tHUDCYCLEFX;

struct tHUDCYCLEFX {
    unsigned char pad[0x08];
    int m_field;
};

void HUDFXCYCLE_Reset(tHUDCYCLEFX *fx)
{
    fx->m_field = 0;
}
