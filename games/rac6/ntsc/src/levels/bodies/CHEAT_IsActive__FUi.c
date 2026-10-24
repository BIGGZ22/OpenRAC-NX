/* CHEAT_IsActive__FUi -- LEVEL13.REL 0x038C80, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   32 octets.
 *   Le masque est produit dans une variable locale : ecrit ainsi, le
 *   compilateur ne peut pas transformer (x & (1 << n)) en (x >> n) & 1.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct CHEAT_TABLE CHEAT_TABLE;

struct CHEAT_TABLE {
    unsigned int reserved_000[568];      /* 0x8E0 */
    unsigned int active_cheats;
};

extern CHEAT_TABLE *CHEAT_table;

unsigned int CHEAT_IsActive__FUi(unsigned int cheat_number)
{
    unsigned int mask;

    mask = 1u << cheat_number;
    return (CHEAT_table->active_cheats & mask) != 0;
}
