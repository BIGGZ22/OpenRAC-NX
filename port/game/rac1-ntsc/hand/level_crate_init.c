/* Carried from port/game/rac1-pal/hand/level_crate_init.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native-only reconstruction of PAL 002D1168..002D19E8. Not a PS2 match.
 * The three group passes establish support links, propagate platform/list
 * state from the bottom of each stack, then visit each pair of crates.
 * Layouts below are the EE layouts; hostgen converts their pointers to gaddr.
 */
typedef struct CrateInitMoby CrateInitMoby;

typedef struct CrateInitVars {
    unsigned char pad00[0xA0];
    CrateInitMoby *above, *below;
    int unkA8;
    unsigned int flags;
    float height;
    unsigned char padB4[12];
    int list;
    unsigned short stamp;
    short trigger;
    int unkC8;
    unsigned int inherited;
    float platform_position[4], platform_rotation[4];
    CrateInitMoby* platform;
    float platform_height;
    int saved_threshold;
} CrateInitVars;

struct CrateInitMoby {
    unsigned char pad00[16];
    float position[4];
    unsigned char state, group, pad22[14];
    unsigned char collision, pad31[5];
    unsigned short distance;
    unsigned char pad38[8];
    float rotation[4];
    unsigned char pad50[40];
    CrateInitVars* vars;
    unsigned char pad7C[42];
    short oclass;
    unsigned short stamp;
    unsigned char padAA[6];
    unsigned char saved_index, padB1[47];
    float up[4];
};

typedef struct CrateInitHit {
    unsigned char pad00[24];
    CrateInitMoby* moby;
    unsigned char pad1C[12];
    float z;
} CrateInitHit;

extern CrateInitHit crate_hit __asm__("D_L00_00173E40");
extern unsigned char crate_save[] __asm__("D_0014161B");
extern int* crate_lists[] __asm__("D_L00_001B04B0");
extern int crate_random(int) __asm__("func_00213260");
extern float crate_angle(float, float) __asm__("func_001FA580");
extern void crate_destroy(CrateInitMoby*) __asm__("func_0020C828");
extern int crate_raycast(float*, float*, int, CrateInitMoby*, int) __asm__("func_L00_001EFC70");
extern int crate_is_crate(CrateInitMoby*) __asm__("func_L00_0025E3B8");
extern int crate_is_platform(CrateInitMoby*) __asm__("func_L00_0025C338");
extern int crate_relative(int, CrateInitMoby*, float*, float*, float*, float*) __asm__(
    "func_L00_00260668"
);
extern int crate_first(CrateInitMoby**, int, int, int) __asm__("func_L00_002591D0");
extern int crate_next(CrateInitMoby**, CrateInitMoby*, int, int) __asm__("func_L00_002592B8");
extern void crate_add(float*, float*, float*) __asm__("func_001F9A10");
extern float crate_distance(float*, float*) __asm__("func_001F9B48");
extern void crate_trap(void) __asm__("func_001F97B0");

static void crate_copy(float* out, const float* in) {
    int i;
    for (i = 0; i < 4; ++i) {
        out[i] = in[i];
    }
}

/* Returns false only when the saved counter removes this crate. */
static int crate_prepare(CrateInitMoby* m) {
    CrateInitVars* v = m->vars;
    if (m->oclass == 0x1F4 && crate_random(2) != 0 && m->rotation[0] == 0.0f
        && m->rotation[1] == 0.0f) {
        m->rotation[2] = crate_angle(m->rotation[2], 1.5707963705062866f);
    }
    if (m->oclass == 0x1F5 && m->saved_index != 255
        && ((int*)(crate_save + 0xD875))[m->saved_index] < v->saved_threshold) {
        crate_destroy(m);
        return 0;
    }
    v->flags |= 1;
    v->stamp = m->stamp;
    v->height = 1.0f;
    return 1;
}

static int crate_probe(CrateInitMoby* m, float height, CrateInitMoby* ignore) {
    float from[4], to[4];
    crate_copy(from, m->position);
    crate_copy(to, m->position);
    from[2] += height;
    to[2] = 0.1f;
    return crate_raycast(from, to, 2, ignore, 0);
}

static void crate_attach(CrateInitMoby* m, CrateInitHit* hit) {
    CrateInitVars* v = m->vars;
    /* Retail's first argument can be an uninitialized stack slot here.
     * 002616E0 never reads a0 (confirmed in its complete retail body). */
    crate_relative(
        0, hit->moby, m->position, m->rotation, v->platform_position, v->platform_rotation
    );
    v->platform_height = 0.0f;
    v->platform = hit->moby;
}

static void crate_advance_list(CrateInitVars* v, int** lists) {
    int *list, state, i;
    if (v->list == -1) {
        return;
    }
    list = lists[v->list];
    state = list[7];
    if (list[0] == 0 || state == 1 || state == 3 || state == 0x111) {
        v->list = -1;
    } else if ((unsigned int)(state - 0x11) < 0x100) {
        if (state - 0x10 < list[0]) {
            list[7] = state + 1;
        } else {
            v->list = -1;
        }
    } else {
        list[7] = 0x11;
        /* Entry zero aliases the state just written: retain that retail order. */
        for (i = 0; i < lists[v->list][0]; ++i) {
            lists[v->list][i * 4 + 7] = 0;
        }
    }
}

void FUN_L00_002cfcb8(CrateInitMoby* self) {
    CrateInitMoby *cur, *base, *other, *support;
    CrateInitVars *v, *under;
    int group = self->group;
    self->vars->flags |= 4;
    if (group == 255) {
        v = self->vars;
        if (!crate_prepare(self)) {
            return;
        }
        if (!crate_probe(self, 0.5f, 0)) {
            v->below = 0;
        } else if (!crate_hit.moby) {
            self->position[2] = crate_hit.z;
            v->below = 0;
        } else if (!crate_is_crate(crate_hit.moby)) {
            v->below = 0;
            v->flags |= 8;
            if (crate_is_platform(crate_hit.moby)) {
                self->distance = 0x7F80;
                crate_attach(self, &crate_hit);
            }
        }
        return;
    }

    for (crate_first(&cur, group, 0, 0); cur; crate_next(&cur, cur, 0, 0)) {
        v = cur->vars;
        if (!crate_is_crate(cur) || !crate_prepare(cur)) {
            continue;
        }
        if (!crate_probe(cur, 0.5f, 0)) {
            v->below = 0;
            continue;
        }
        support = crate_hit.moby;
        if (!support) {
            cur->position[2] = crate_hit.z;
            v->below = 0;
        } else if (
            !crate_is_crate(support) && support->oclass != 0x128 && support->oclass != 0x129
        ) {
            v->below = 0;
            cur->collision = 255;
            v->flags |= 8;
        } else if (!support->vars || support->group != cur->group) {
            v->below = 0;
        } else if (crate_is_crate(support)) {
            v->below = support;
            crate_copy(cur->position, support->position);
            crate_add(cur->position, cur->position, support->up);
            under = support->vars;
            if (crate_is_crate(cur)) {
                under->above = cur;
            } else {
                crate_trap();
                under->above = 0;
            }
        } else {
            v->inherited = 0;
            v->below = 0;
            cur->position[2] = crate_hit.z;
        }
    }

    for (crate_first(&base, group, 0, 0); base; crate_next(&base, base, 0, 0)) {
        v = base->vars;
        if (!crate_is_crate(base) || v->below) {
            continue;
        }
        /* cur is NULL after pass one, and after every completed stack walk.
         * Retail passes that value as the ray's exclusion argument. */
        if (crate_probe(base, v->height * 0.5f, cur) && crate_hit.moby
            && crate_is_platform(crate_hit.moby)) {
            if (cur) {
                cur->distance = 0x7F80;
            }
            crate_attach(base, &crate_hit);
            v->flags |= 8;
        }
        for (cur = base; cur; cur = v->above) {
            v = cur->vars;
            under = v->below ? v->below->vars : 0;
            if (under) {
                v->inherited = under->inherited;
                if (under->platform) {
                    cur->collision = 255;
                    v->flags |= 8;
                    crate_copy(v->platform_position, under->platform_position);
                    crate_copy(v->platform_rotation, under->platform_rotation);
                    v->platform = under->platform;
                    v->platform_height = under->platform_height + 1.0f;
                }
            }
            if ((v->trigger != 0 || cur->oclass == 0x1F5) && v->above) {
                crate_advance_list(v, crate_lists);
            }
        }
    }

    for (crate_first(&cur, group, 0, 0); cur; crate_next(&cur, cur, 0, 0)) {
        if (!crate_is_crate(cur)) {
            continue;
        }
        for (crate_next(&other, cur, 0, 0); other; crate_next(&other, other, 0, 0)) {
            if (crate_is_crate(other)) {
                /* The retail call's return value is deliberately unused. */
                crate_distance(cur->position, other->position);
            }
        }
    }
}
