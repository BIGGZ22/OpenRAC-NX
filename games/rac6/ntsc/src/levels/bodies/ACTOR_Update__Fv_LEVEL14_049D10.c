/* ACTOR_Update__Fv_LEVEL14_049D10 -- LEVEL14.REL 0x049D10, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Actor Actor;
typedef struct ActorDef ActorDef;

struct ActorDef {
    char pad[0x10];
    void (*method)(Actor *);
};

struct Actor {
    Actor *prev;
    Actor *next;
    void *field_08;
    void *field_0C;
    void *field_10;
    ActorDef *def;
    void *update;
};

typedef struct ActorGlobal {
    char pad[0x1C];
    int end;
    Actor *head;
} ActorGlobal;

/* the actor table lives at 0x34A060 in the module (anonymous) */
extern ActorGlobal D_0034A060;

extern Actor *func_049C60(Actor *node);
extern void func_049C88(Actor *node);

void ACTOR_Update__Fv(void)
{
    ActorGlobal *g = &D_0034A060;
    Actor *node = g->head;

    while (node != (Actor *)&g->end) {
        if (node->def->method != 0) {
            node->def->method(node);
            func_049C88(node);
        }
        if (node->update == 0)
            node = func_049C60(node);
        else
            node = node->next;
    }
}
