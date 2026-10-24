/* HUD_FontPrintWindowInitAutoScroll__FP20tFontPrintWindowData -- FE.REL 0x02B3C8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   HUD_FontPrintWindowInitAutoScroll__FP20tFontPrintWindowData
 *   @ FE.REL 0x02B3C8 (32 bytes)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tFontPrintWindowData tFontPrintWindowData;

struct tFontPrintWindowData {
    unsigned char reserved[0x3C];
    int field3C;
    int field40;
    unsigned char reserved44[4];
    unsigned char field48;
    unsigned char field49;
};

void HUD_FontPrintWindowInitAutoScroll(tFontPrintWindowData *data)
{
    data->field48 = 1;
    data->field49 = 0;
    data->field3C = 0x3C;
    data->field40 = 0x1E;
}
