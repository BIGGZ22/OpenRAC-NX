/* sceGsSyncVCallback -- FE.REL 0x000C6F98, 160 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le contexte GS porte deux mots : le callback de synchronisation verticale
 *   (0x08) et l'identifiant du gestionnaire d'interruption (0x0C). Un callback
 *   nul retire le gestionnaire ; un callback non nul enregistre le nouveau
 *   gestionnaire (interruption 2, Priorite -1). Si un gestionnaire existait
 *   deja, il est retire avant l'enregistrement ; sinon le retrait est saute
 *   (la branche "beql" du retail). La valeur rendue est l'ancien callback.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GS_CONTEXT {
    short x;
    short y;
    short z;
    short imr;
    int callback;
    int intcHandler;
} GS_CONTEXT;

extern GS_CONTEXT *sceGsGetGParam(void);
extern void DisableIntc(int intc);
extern void EnableIntc(int intc);
extern void RemoveIntcHandler(int intc, int handler);
extern int AddIntcHandler(int intc, int handler, int arg);

int sceGsSyncVCallback(int callback)
{
    GS_CONTEXT *ctx = sceGsGetGParam();
    int old = ctx->callback;

    if (callback == 0) {
        DisableIntc(2);
        RemoveIntcHandler(2, ctx->intcHandler);
        ctx->intcHandler = 0;
        ctx->callback = 0;
    } else {
        if (old == 0) {
            ctx->callback = callback;
        } else {
            DisableIntc(2);
            RemoveIntcHandler(2, ctx->intcHandler);
            ctx->callback = callback;
        }
        ctx->intcHandler = AddIntcHandler(2, callback, -1);
        EnableIntc(2);
    }

    return old;
}
