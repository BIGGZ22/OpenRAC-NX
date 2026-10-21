/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * A game's binding for script mods (docs/port/MODS.md, section 11), written by
 * sdk/tools/gen_binding.py from sdk/bindings/<game>.json into port/game/<game>/host/script_binding.c:
 * for each hook point, OPENRAC_SCRIPT_LINKS C functions of its exact signature, one for each script
 * mod that may take it (a slot of the hook point's chain holds a plain function, so each place in the
 * chain needs a function of its own), which hand their arguments to the script runtime
 * (game/common/mod_scripts.cpp) as openrac_script_value and call what they go on to when no script
 * handles the call; and the game state exposed to scripts by name (fields), with each disc's address.
 * A game without one gets game/common/script_binding_none.c: no script mod loads there.
 */
#ifndef OPENRAC_SCRIPT_BINDING_H
#define OPENRAC_SCRIPT_BINDING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* How many script mods may take one hook point. */
#define OPENRAC_SCRIPT_LINKS 8

/* The kinds of the signatures (MODS.md, section 4). */
enum {
    OPENRAC_SV_VOID = 0,
    OPENRAC_SV_I8,
    OPENRAC_SV_U8,
    OPENRAC_SV_I16,
    OPENRAC_SV_U16,
    OPENRAC_SV_I32,
    OPENRAC_SV_U32,
    OPENRAC_SV_I64,
    OPENRAC_SV_U64,
    OPENRAC_SV_F32
};

/* An argument or a result: integers in i (sign-extended), floats in f. */
typedef struct openrac_script_value {
    int64_t i;
    double f;
} openrac_script_value;

/* One place in a hook point's chain that a script mod holds. */
typedef struct openrac_script_link {
    void* original; /* what it goes on to: the game's function, or a mod's loaded earlier */
    void* runtime;  /* the script runtime's own state for it; null while the place is free */
} openrac_script_link;

/* Calls `original` (of the hook point's signature) with args, putting its result in *result. */
typedef void (*openrac_script_call_fn)(void* original, const openrac_script_value* args,
                                       openrac_script_value* result);

typedef struct openrac_script_point {
    const char* name;      /* the hook point's, as hostgen's hooks.c and the SDK name it */
    const char* signature; /* "void(i32,f32,f32,f32)" */
    int arg_count;
    const uint8_t* arg_kinds; /* OPENRAC_SV_* of each argument */
    const char* const* arg_names;
    uint8_t result_kind;
    openrac_script_link* links;   /* OPENRAC_SCRIPT_LINKS of them */
    void* const* thunks;          /* the C function of each link */
    openrac_script_call_fn call;  /* calls a function of this signature */
} openrac_script_point;

/* A piece of game state exposed by name: at `address` on disc `discs[k]` (0: not on that disc). */
typedef struct openrac_script_field {
    const char* path; /* "hero.position.x" */
    uint8_t kind;     /* OPENRAC_SV_* */
    uint8_t readonly;
    const uint32_t* addresses; /* one per binding disc, in the binding's order */
} openrac_script_field;

/* From the behaviour catalogue: a moby class's pvars, at offsets from the address its moby keeps. */
typedef struct openrac_script_pvar {
    const char* name;
    uint8_t kind; /* OPENRAC_SV_* */
    uint8_t readonly;
    uint32_t offset;
} openrac_script_pvar;

typedef struct openrac_script_class {
    const char* name;
    int32_t number; /* the class's number, as a moby keeps it */
    int pvar_count;
    const openrac_script_pvar* pvars;
} openrac_script_class;

/* Where a moby keeps its class number (of kind class_kind) and its pvars' address. */
typedef struct openrac_script_moby {
    uint32_t class_offset;
    uint8_t class_kind;
    uint32_t pvars_offset;
    int class_count;
    const openrac_script_class* classes;
} openrac_script_moby;

typedef struct openrac_script_binding {
    const char* game;
    int disc_count;
    const char* const* discs; /* "pal", "ntsc" */
    int point_count;
    const openrac_script_point* points;
    int field_count;
    const openrac_script_field* fields;
    const openrac_script_moby* moby; /* null until the game's behaviour catalogue is in */
} openrac_script_binding;

/* The game's binding, or null for a game with none. */
const openrac_script_binding* openrac_game_script_binding(void);

/* The runtime's side, called by every link's function: 1 when a script handled the call (and put
 * the result in *result), 0 when the link's original is to be called as it is (no script there, or
 * its mod stopped). */
int openrac_script_dispatch(const openrac_script_point* point, openrac_script_link* link,
                            const openrac_script_value* args, openrac_script_value* result);

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_SCRIPT_BINDING_H */
