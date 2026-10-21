/* Generated from sdk/bindings/test.json by sdk/tools/gen_binding.py. Do not edit. */

/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The script mods' side of the game's hook points and its fields (docs/port/MODS.md, section 11;
 * openrac/script_binding.h). */
#include <stddef.h>
#include <stdint.h>

#include "openrac/script_binding.h"

static const openrac_script_point k_points[2];

/* Test_Add i32(i32):  */
typedef int32_t (*Test_Add_fn)(int32_t);
static openrac_script_link k_Test_Add_links[OPENRAC_SCRIPT_LINKS];
static const uint8_t k_Test_Add_kinds[] = {OPENRAC_SV_I32};
static const char* const k_Test_Add_names[] = {"x"};
static void Test_Add_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    r->i = ((Test_Add_fn)original)((int32_t)v[0].i);
}
static int32_t Test_Add_0(int32_t x) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)x;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Test_Add_links[0], v, &r)) {
        return ((Test_Add_fn)k_Test_Add_links[0].original)(x);
    }
    return (int32_t)r.i;
}
static int32_t Test_Add_1(int32_t x) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)x;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Test_Add_links[1], v, &r)) {
        return ((Test_Add_fn)k_Test_Add_links[1].original)(x);
    }
    return (int32_t)r.i;
}
static int32_t Test_Add_2(int32_t x) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)x;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Test_Add_links[2], v, &r)) {
        return ((Test_Add_fn)k_Test_Add_links[2].original)(x);
    }
    return (int32_t)r.i;
}
static int32_t Test_Add_3(int32_t x) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)x;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Test_Add_links[3], v, &r)) {
        return ((Test_Add_fn)k_Test_Add_links[3].original)(x);
    }
    return (int32_t)r.i;
}
static int32_t Test_Add_4(int32_t x) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)x;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Test_Add_links[4], v, &r)) {
        return ((Test_Add_fn)k_Test_Add_links[4].original)(x);
    }
    return (int32_t)r.i;
}
static int32_t Test_Add_5(int32_t x) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)x;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Test_Add_links[5], v, &r)) {
        return ((Test_Add_fn)k_Test_Add_links[5].original)(x);
    }
    return (int32_t)r.i;
}
static int32_t Test_Add_6(int32_t x) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)x;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Test_Add_links[6], v, &r)) {
        return ((Test_Add_fn)k_Test_Add_links[6].original)(x);
    }
    return (int32_t)r.i;
}
static int32_t Test_Add_7(int32_t x) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)x;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Test_Add_links[7], v, &r)) {
        return ((Test_Add_fn)k_Test_Add_links[7].original)(x);
    }
    return (int32_t)r.i;
}
static void* const k_Test_Add_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)Test_Add_0, (void*)Test_Add_1, (void*)Test_Add_2, (void*)Test_Add_3, (void*)Test_Add_4, (void*)Test_Add_5, (void*)Test_Add_6, (void*)Test_Add_7};

/* Test_Scale f32(f32,u32):  */
typedef float (*Test_Scale_fn)(float, uint32_t);
static openrac_script_link k_Test_Scale_links[OPENRAC_SCRIPT_LINKS];
static const uint8_t k_Test_Scale_kinds[] = {OPENRAC_SV_F32, OPENRAC_SV_U32};
static const char* const k_Test_Scale_names[] = {"x", "by"};
static void Test_Scale_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    r->f = ((Test_Scale_fn)original)((float)v[0].f, (uint32_t)v[1].i);
}
static float Test_Scale_0(float x, uint32_t by) {
    openrac_script_value v[2] = {{0, 0.0}};
    v[0].f = (double)x;
    v[1].i = (int64_t)by;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Test_Scale_links[0], v, &r)) {
        return ((Test_Scale_fn)k_Test_Scale_links[0].original)(x, by);
    }
    return (float)r.f;
}
static float Test_Scale_1(float x, uint32_t by) {
    openrac_script_value v[2] = {{0, 0.0}};
    v[0].f = (double)x;
    v[1].i = (int64_t)by;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Test_Scale_links[1], v, &r)) {
        return ((Test_Scale_fn)k_Test_Scale_links[1].original)(x, by);
    }
    return (float)r.f;
}
static float Test_Scale_2(float x, uint32_t by) {
    openrac_script_value v[2] = {{0, 0.0}};
    v[0].f = (double)x;
    v[1].i = (int64_t)by;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Test_Scale_links[2], v, &r)) {
        return ((Test_Scale_fn)k_Test_Scale_links[2].original)(x, by);
    }
    return (float)r.f;
}
static float Test_Scale_3(float x, uint32_t by) {
    openrac_script_value v[2] = {{0, 0.0}};
    v[0].f = (double)x;
    v[1].i = (int64_t)by;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Test_Scale_links[3], v, &r)) {
        return ((Test_Scale_fn)k_Test_Scale_links[3].original)(x, by);
    }
    return (float)r.f;
}
static float Test_Scale_4(float x, uint32_t by) {
    openrac_script_value v[2] = {{0, 0.0}};
    v[0].f = (double)x;
    v[1].i = (int64_t)by;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Test_Scale_links[4], v, &r)) {
        return ((Test_Scale_fn)k_Test_Scale_links[4].original)(x, by);
    }
    return (float)r.f;
}
static float Test_Scale_5(float x, uint32_t by) {
    openrac_script_value v[2] = {{0, 0.0}};
    v[0].f = (double)x;
    v[1].i = (int64_t)by;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Test_Scale_links[5], v, &r)) {
        return ((Test_Scale_fn)k_Test_Scale_links[5].original)(x, by);
    }
    return (float)r.f;
}
static float Test_Scale_6(float x, uint32_t by) {
    openrac_script_value v[2] = {{0, 0.0}};
    v[0].f = (double)x;
    v[1].i = (int64_t)by;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Test_Scale_links[6], v, &r)) {
        return ((Test_Scale_fn)k_Test_Scale_links[6].original)(x, by);
    }
    return (float)r.f;
}
static float Test_Scale_7(float x, uint32_t by) {
    openrac_script_value v[2] = {{0, 0.0}};
    v[0].f = (double)x;
    v[1].i = (int64_t)by;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Test_Scale_links[7], v, &r)) {
        return ((Test_Scale_fn)k_Test_Scale_links[7].original)(x, by);
    }
    return (float)r.f;
}
static void* const k_Test_Scale_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)Test_Scale_0, (void*)Test_Scale_1, (void*)Test_Scale_2, (void*)Test_Scale_3, (void*)Test_Scale_4, (void*)Test_Scale_5, (void*)Test_Scale_6, (void*)Test_Scale_7};

static const openrac_script_point k_points[2] = {
    {"Test_Add", "i32(i32)", 1, k_Test_Add_kinds, k_Test_Add_names, OPENRAC_SV_I32, k_Test_Add_links, k_Test_Add_thunks, Test_Add_call},
    {"Test_Scale", "f32(f32,u32)", 2, k_Test_Scale_kinds, k_Test_Scale_names, OPENRAC_SV_F32, k_Test_Scale_links, k_Test_Scale_thunks, Test_Scale_call},
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
};

const openrac_script_binding* openrac_game_script_binding(void) {
    return &k_binding;
}
