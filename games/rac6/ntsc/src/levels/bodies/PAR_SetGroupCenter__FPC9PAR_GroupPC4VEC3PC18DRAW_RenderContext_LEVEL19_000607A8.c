/* PAR_SetGroupCenter__FPC9PAR_GroupPC4VEC3PC18DRAW_RenderContext_LEVEL19_000607A8 -- LEVEL19.REL 0x000607A8, 40 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   PAR_SetGroupCenter__FPC9PAR_GroupPC4VEC3PC18DRAW_RenderContext -- LEVEL19.REL
 *   0x0607A8, 40 bytes.
 *   The group itself is not used: the routine asks the draw context for the
 *   screen depth seen from the centre and stores the answer in the global word
 *   at 0x333B14 (an anonymous object of the module).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct PAR_Group PAR_Group;
typedef struct VEC3 VEC3;
typedef struct DRAW_RenderContext DRAW_RenderContext;

extern int D_333B14;

extern int DRAW_GetScreenDepth__FPC4VEC3PC18DRAW_RenderContext(
    VEC3 *position, DRAW_RenderContext *context);

void PAR_SetGroupCenter__FPC9PAR_GroupPC4VEC3PC18DRAW_RenderContext(
    PAR_Group *group, VEC3 *center, DRAW_RenderContext *context)
{
    (void)group;

    D_333B14 = DRAW_GetScreenDepth__FPC4VEC3PC18DRAW_RenderContext(center,
                                                                   context);
}
