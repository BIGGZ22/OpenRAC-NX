/* ACTOR_PostUpdate__Fv_LEVEL10_049DD0 -- LEVEL10.REL 0x049DD0, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Meme liste chainee que ACTOR_Update__Fv (0x049D50), mais la methode
 *   appelee est celle situee a l'offset 0x14 de la definition de l'acteur
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
typedef struct ActorDefPost ActorDefPost;

struct ActorDefPost {
    char pad[0x14];
    void (*method)(Actor *);
};

struct Actor {
    Actor *prev;
    Actor *next;
    void *field_08;
    void *field_0C;
    void *field_10;
    ActorDefPost *def;
    void *update;
};

typedef struct ActorGlobal {
    char pad[0x1C];
    int end;
    Actor *head;
} ActorGlobal;

/* the actor table lives at 0x321560 in the module (anonymous) */
extern ActorGlobal D_00321560;

extern Actor *func_049CA0(Actor *node);
extern void func_049CC8(Actor *node);

void ACTOR_PostUpdate__Fv(void)
{
    ActorGlobal *g = &D_00321560;
    Actor *node = g->head;

    while (node != (Actor *)&g->end) {
        if (node->def->method != 0) {
            node->def->method(node);
            func_049CC8(node);
        }
        if (node->update == 0)
            node = func_049CA0(node);
        else
            node = node->next;
    }
}
