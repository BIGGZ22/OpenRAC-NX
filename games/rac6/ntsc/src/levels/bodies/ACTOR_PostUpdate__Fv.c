/* ACTOR_PostUpdate__Fv -- LEVEL1.REL 0x049DD8, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   128 octets.
 *   Meme corps que ACTOR_Update__Fv (0x049D58), avec la methode de la
 *   definition d'acteur lue a 0x14 au lieu de 0x10 : le second pointeur de
 *   methode de la definition est appele.
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
    void (*postMethod)(Actor *);
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

/* the actor table lives at 0x31F560 in the module (anonymous) */
extern ActorGlobal D_0031F560;

extern Actor *func_049CA8(Actor *node);
extern void func_049CD0(Actor *node);

void ACTOR_PostUpdate__Fv(void)
{
    ActorGlobal *g = &D_0031F560;
    Actor *node = g->head;

    while (node != (Actor *)&g->end) {
        if (node->def->postMethod != 0) {
            node->def->postMethod(node);
            func_049CD0(node);
        }
        if (node->update == 0)
            node = func_049CA8(node);
        else
            node = node->next;
    }
}
