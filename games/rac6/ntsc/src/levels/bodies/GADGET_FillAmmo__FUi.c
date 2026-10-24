/* GADGET_FillAmmo__FUi -- FE.REL 0x00C3A0, 64 bytes.
 *
 * Fills the ammunition of a gadget instance with the ammunition of its definition.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GADGETDATA_s GADGETDATA_s;
typedef struct GADGETDEF_s GADGETDEF_s;

struct GADGETDATA_s {
    unsigned char pad[0x60];
    unsigned int m_ammo;
};

struct GADGETDEF_s {
    unsigned char pad[0x40];
    unsigned int m_ammo;
};

extern GADGETDATA_s *GADGET_GetData__FUi(unsigned int index);
extern GADGETDEF_s *GADGET_GetDataDef__FUi(unsigned int index);

void GADGET_FillAmmo__FUi(unsigned int index)
{
    GADGETDATA_s *data = GADGET_GetData__FUi(index);
    unsigned int ammo = GADGET_GetDataDef__FUi(index)->m_ammo;
    data->m_ammo = ammo;
}
