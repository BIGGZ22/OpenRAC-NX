/* ACTOR_Update__Fv_LEVEL11_049D40 -- LEVEL11.REL 0x049D40, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ...
 *   Le tableau d'acteurs est une liste chainee : la fin est marquee par
 *   l'adresse (tableau + 0x1C).
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

/* the actor table lives at 0x338960 in the module (anonymous) */
extern ActorGlobal D_00338960;

extern Actor *func_049C90(Actor *node);
extern void func_049CB8(Actor *node);

void ACTOR_Update__Fv(void)
{
    ActorGlobal *g = &D_00338960;
    Actor *node = g->head;

    while (node != (Actor *)&g->end) {
        if (node->def->method != 0) {
            node->def->method(node);
            func_049CB8(node);
        }
        if (node->update == 0)
            node = func_049C90(node);
        else
            node = node->next;
    }
}
