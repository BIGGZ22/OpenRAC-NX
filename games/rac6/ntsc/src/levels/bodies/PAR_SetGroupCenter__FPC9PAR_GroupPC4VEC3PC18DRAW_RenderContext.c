/* PAR_SetGroupCenter__FPC9PAR_GroupPC4VEC3PC18DRAW_RenderContext -- LEVEL10.REL 0x000607A8, 40 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le centre du groupe est replace a la profondeur ecran du point donne ;
 *   la profondeur obtenue est rangee dans le global 0x327414.
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
typedef struct VEC3 {
    float x;
    float y;
    float z;
} VEC3;
typedef struct DRAW_RenderContext DRAW_RenderContext;

extern unsigned int g_par_screen_depth;

extern unsigned int DRAW_GetScreenDepth__FPC4VEC3PC18DRAW_RenderContext(
    VEC3 *pos, DRAW_RenderContext *rctx);

void PAR_SetGroupCenter__FPC9PAR_GroupPC4VEC3PC18DRAW_RenderContext(
    PAR_Group *group, VEC3 *center, DRAW_RenderContext *rctx)
{
    (void)group;
    g_par_screen_depth =
        DRAW_GetScreenDepth__FPC4VEC3PC18DRAW_RenderContext(center, rctx);
}
