/* PAR_SetGroupCenter__FPC9PAR_GroupPC4VEC3PC18DRAW_RenderContext_LEVEL11_00060770 -- LEVEL11.REL 0x00060770, 40 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   (40 octets)
 *   Le groupe n'est pas utilise : la profondeur d'ecran calculee a partir du
 *   centre et du contexte de rendu est rangee dans le global du module a
 *   l'adresse 0x33E814.
 *   Preuve : chaine gnu-ee-20261015:-O2 -G0 (egalement exacte sur
 *   gnu-ee-trial-20261015:-O2 -G0), comparaison octet pour octet au retail
 *   au placement vise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct VEC3 {
    float x;
    float y;
    float z;
} VEC3;

typedef struct PAR_Group PAR_Group;
typedef struct DRAW_RenderContext DRAW_RenderContext;

extern int DRAW_GetScreenDepth__FPC4VEC3PC18DRAW_RenderContext(
    VEC3 *center,
    DRAW_RenderContext *context);

/* Global du module a l'adresse 0x33E814 (nom retail inconnu). */
extern int D_0033E814;

void PAR_SetGroupCenter__FPC9PAR_GroupPC4VEC3PC18DRAW_RenderContext(
    PAR_Group *group,
    VEC3 *center,
    DRAW_RenderContext *context)
{
    (void)group;

    D_0033E814 = DRAW_GetScreenDepth__FPC4VEC3PC18DRAW_RenderContext(center,
                                                                     context);
}
