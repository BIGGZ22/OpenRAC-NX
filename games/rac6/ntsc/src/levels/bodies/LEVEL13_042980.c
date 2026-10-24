/* LEVEL13_042980 -- LEVEL13.REL 0x042980, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   32 octets.
 *   La table des symboles du module ne contient aucune entrée pour cette
 *   adresse (elle n'est exportee ni importee) : le nom ci-dessous est
 *   descriptif, il ne pretend pas etre le nom retail.
 *   L'index est calcule dans une variable locale : le -1 reste alors dans
 *   le calcul (addiu) au lieu d'etre replie dans le decalage du load, et
 *   l'ordre des écritures fixe l'affectation des registres.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ITEM_TABLE ITEM_TABLE;

struct ITEM_TABLE {
    unsigned int reserved_000[3];
    unsigned char *rows;
};

unsigned char GetItem__FR13ITEM_TABLEUiUi(ITEM_TABLE *self,
                                          unsigned int row,
                                          unsigned int column)
{
    unsigned int index;

    index = 3 * row - 1;
    index = index + column;
    return self->rows[index];
}
