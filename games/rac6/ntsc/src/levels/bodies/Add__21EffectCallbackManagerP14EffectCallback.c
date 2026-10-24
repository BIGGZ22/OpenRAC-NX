/* Add__21EffectCallbackManagerP14EffectCallback -- LEVEL13.REL 0x03F0E0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   32 octets.
 *   La recharge de la tete est chargee avant l'ecriture "cb->suivant = NULL"
 *   : l'ordre source met "cb->precedent = tete" en premier, l'ordonnanceur
 *   echange ensuite les deux ecritures.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct EffectCallback EffectCallback;
typedef struct EffectCallbackManager EffectCallbackManager;

struct EffectCallback {
    EffectCallback *previous;
    EffectCallback *next;
};

struct EffectCallbackManager {
    EffectCallback *head;
};

void Add__21EffectCallbackManagerP14EffectCallback(EffectCallbackManager *self,
                                                   EffectCallback *callback)
{
    if (self->head)
        self->head->next = callback;
    callback->previous = self->head;
    callback->next = 0;
    self->head = callback;
}
