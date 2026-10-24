/* GADGET_GetDataDef__FUi_LEVEL13_033580 -- LEVEL13.REL 0x033580, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   64 octets.
 *   L'ordre des additions compte : (offset + sous_index * 4) est calcule
 *   avant l'ajout de la table, et le +0x10 reste dans le decalage du load.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GADGET_ENTRY GADGET_ENTRY;

struct GADGET_ENTRY {
    unsigned char reserved_000[0x5C];
    unsigned int sub_index;
    unsigned char reserved_060[0x14];
};

extern GADGET_ENTRY GADGET_table[];

unsigned int GADGET_GetDataDef__FUi(unsigned int index)
{
    unsigned int offset;
    unsigned int sub_index;

    if (index < 40) {
        offset = index * 0x74;
        sub_index = GADGET_table[index].sub_index;
        return *(unsigned int *)(((unsigned char *)GADGET_table
                                  + (offset + sub_index * 4)) + 0x10);
    }
    return 0;
}
