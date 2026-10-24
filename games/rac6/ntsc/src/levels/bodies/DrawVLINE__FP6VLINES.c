/* DrawVLINE__FP6VLINES -- LEVEL10.REL 0x000630E0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Une VLINE porte deux sommets et un parametre en 0x0C ; elle est rendue
 *   par DrawLines.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct UBE_VERT UBE_VERT;
typedef struct VLINES {
    unsigned char unknown_000[0x0C];
    unsigned int param;
} VLINES;

extern void DrawLines__FPC9PUBE_VERTUiUi(UBE_VERT *verts, unsigned int count,
                                        unsigned int param);

void DrawVLINE__FP6VLINES(VLINES *lines)
{
    DrawLines__FPC9PUBE_VERTUiUi((UBE_VERT *)lines, 2, lines->param);
}
