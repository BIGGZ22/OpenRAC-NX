/* DRAW_AddStageRenderCallback__F10DRAW_LayerPF10DRAW_LayerPv_vPv -- LEVEL13.REL 0x04F818, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   32 octets.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct DRAW_Layer DRAW_Layer;
typedef void (*tDRAW_LayerCallback)(DRAW_Layer *layer, void *user_data);

void DRAW_AddStageRenderCallback__F10DRAW_LayerPF10DRAW_LayerPv_vPv(
    DRAW_Layer *layer, tDRAW_LayerCallback callback, void *user_data)
{
    callback(layer, user_data);
}
