/* Generated from sdk/bindings/test.json by sdk/tools/gen_binding.py. Do not edit. */

/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The script mods' side of the game's hook points, their names and its fields (docs/port/MODS.md,
 * sections 4 and 11; openrac/script_binding.h). */
#include <stddef.h>
#include <stdint.h>

#include "openrac/script_binding.h"

static const openrac_script_point k_points[2];

/* Shape S0: i32(i32) (x). */
typedef int32_t (*S0_fn)(int32_t);
static const uint8_t k_S0_kinds[] = {OPENRAC_SV_I32};
static const char* const k_S0_names[] = {"x"};
static void S0_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    r->i = ((S0_fn)original)((int32_t)v[0].i);
}
#define S0_THUNK(N, I, K) static int32_t N##_##K(int32_t x) { \
    openrac_script_value v[1] = {{0, 0.0}}; \
    v[0].i = (int64_t)x; \
    openrac_script_value r = {0, 0.0}; \
    if (!openrac_script_dispatch(&k_points[I], &k_##N##_links[K], v, &r)) { \
        return ((S0_fn)k_##N##_links[K].original)(x); \
    } \
    return (int32_t)r.i; \
}
#define S0_POINT(N, I) static openrac_script_link k_##N##_links[OPENRAC_SCRIPT_LINKS]; S0_THUNK(N, I, 0) S0_THUNK(N, I, 1) S0_THUNK(N, I, 2) S0_THUNK(N, I, 3) S0_THUNK(N, I, 4) S0_THUNK(N, I, 5) S0_THUNK(N, I, 6) S0_THUNK(N, I, 7) static void* const k_##N##_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)N##_0, (void*)N##_1, (void*)N##_2, (void*)N##_3, (void*)N##_4, (void*)N##_5, (void*)N##_6, (void*)N##_7};

/* Shape S1: f32(f32,u32) (x, by). */
typedef float (*S1_fn)(float, uint32_t);
static const uint8_t k_S1_kinds[] = {OPENRAC_SV_F32, OPENRAC_SV_U32};
static const char* const k_S1_names[] = {"x", "by"};
static void S1_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    r->f = ((S1_fn)original)((float)v[0].f, (uint32_t)v[1].i);
}
#define S1_THUNK(N, I, K) static float N##_##K(float x, uint32_t by) { \
    openrac_script_value v[2] = {{0, 0.0}}; \
    v[0].f = (double)x; \
    v[1].i = (int64_t)by; \
    openrac_script_value r = {0, 0.0}; \
    if (!openrac_script_dispatch(&k_points[I], &k_##N##_links[K], v, &r)) { \
        return ((S1_fn)k_##N##_links[K].original)(x, by); \
    } \
    return (float)r.f; \
}
#define S1_POINT(N, I) static openrac_script_link k_##N##_links[OPENRAC_SCRIPT_LINKS]; S1_THUNK(N, I, 0) S1_THUNK(N, I, 1) S1_THUNK(N, I, 2) S1_THUNK(N, I, 3) S1_THUNK(N, I, 4) S1_THUNK(N, I, 5) S1_THUNK(N, I, 6) S1_THUNK(N, I, 7) static void* const k_##N##_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)N##_0, (void*)N##_1, (void*)N##_2, (void*)N##_3, (void*)N##_4, (void*)N##_5, (void*)N##_6, (void*)N##_7};

S0_POINT(Test_Add, 0) /* test.add */
S1_POINT(Test_Scale, 1) /* test.scale */

static const openrac_script_point k_points[2] = {
    {"test.add", "i32(i32)", 1, k_S0_kinds, k_S0_names, OPENRAC_SV_I32, k_Test_Add_links, k_Test_Add_thunks, S0_call},
    {"test.scale", "f32(f32,u32)", 2, k_S1_kinds, k_S1_names, OPENRAC_SV_F32, k_Test_Scale_links, k_Test_Scale_thunks, S1_call},
};

static const openrac_script_alias k_aliases[5] = {
    {"Old_Add", "test.add"},
    {"Test_Add", "test.add"},
    {"Test_Scale", "test.scale"},
    {"func_00100000", "test.add"},
    {"func_00100100", "test.scale"},
};

static const char* const k_discs[] = {"one"};
static const uint32_t k_field_0[] = {0x00100000u}; /* counter */
static const uint32_t k_field_1[] = {0x00100004u}; /* nested.value */
static const openrac_script_field k_fields[2] = {
    {"counter", OPENRAC_SV_I32, 0, k_field_0},
    {"nested.value", OPENRAC_SV_F32, 1, k_field_1},
};

static const openrac_script_binding k_binding = {
    "test", 1, k_discs, 2, k_points, 2, k_fields, NULL,
    5, k_aliases,
};

const openrac_script_binding* openrac_game_script_binding(void) {
    return &k_binding;
}
