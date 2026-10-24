/* GADGET_GetDataDef__FUi -- FE.REL 0x00B978, 64 bytes.
 *
 * Returns the data definition a gadget points at, or 0 when the index is out of range.
 *
 * Evidence:
 *   Table de 0x28 enregistrements de 0x74 octets a l'adresse module 0xF3B28 ;
 *   chaque enregistrement porte a l'offset 0x10 un tableau de 19 pointeurs et
 *   a l'offset 0x5C l'index de l'entree active.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GADGETDEF_s GADGETDEF_s;

struct GADGETDEF_s {
    unsigned char pad0[0x10];
    GADGETDEF_s *entries[19];
    int which;
    unsigned char pad1[0x14];
};

extern GADGETDEF_s g_GadgetDefs[0x28];

GADGETDEF_s *GADGET_GetDataDef__FUi(unsigned int index)
{
    if (index < 0x28)
        return g_GadgetDefs[index].entries[g_GadgetDefs[index].which];
    return 0;
}
