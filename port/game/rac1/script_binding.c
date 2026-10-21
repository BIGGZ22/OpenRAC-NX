/* Generated from sdk/bindings/rac1.json by sdk/tools/gen_binding.py. Do not edit. */

/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The script mods' side of the game's hook points and its fields (docs/port/MODS.md, section 11;
 * openrac/script_binding.h). */
#include <stddef.h>
#include <stdint.h>

#include "openrac/script_binding.h"

static const openrac_script_point k_points[6];

/* Pad_ProcessReport void(u32,u32,i32): One pad report into the pad's state (PadState, at pad), each poll, before the tick reads it. */
typedef void (*Pad_ProcessReport_fn)(uint32_t, uint32_t, int32_t);
static openrac_script_link k_Pad_ProcessReport_links[OPENRAC_SCRIPT_LINKS];
static const uint8_t k_Pad_ProcessReport_kinds[] = {OPENRAC_SV_U32, OPENRAC_SV_U32, OPENRAC_SV_I32};
static const char* const k_Pad_ProcessReport_names[] = {"pad", "report", "size"};
static void Pad_ProcessReport_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    (void)r;
    ((Pad_ProcessReport_fn)original)((uint32_t)v[0].i, (uint32_t)v[1].i, (int32_t)v[2].i);
}
static void Pad_ProcessReport_0(uint32_t pad, uint32_t report, int32_t size) {
    openrac_script_value v[3] = {{0, 0.0}};
    v[0].i = (int64_t)pad;
    v[1].i = (int64_t)report;
    v[2].i = (int64_t)size;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Pad_ProcessReport_links[0], v, &r)) {
        ((Pad_ProcessReport_fn)k_Pad_ProcessReport_links[0].original)(pad, report, size);
        return;
    }
}
static void Pad_ProcessReport_1(uint32_t pad, uint32_t report, int32_t size) {
    openrac_script_value v[3] = {{0, 0.0}};
    v[0].i = (int64_t)pad;
    v[1].i = (int64_t)report;
    v[2].i = (int64_t)size;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Pad_ProcessReport_links[1], v, &r)) {
        ((Pad_ProcessReport_fn)k_Pad_ProcessReport_links[1].original)(pad, report, size);
        return;
    }
}
static void Pad_ProcessReport_2(uint32_t pad, uint32_t report, int32_t size) {
    openrac_script_value v[3] = {{0, 0.0}};
    v[0].i = (int64_t)pad;
    v[1].i = (int64_t)report;
    v[2].i = (int64_t)size;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Pad_ProcessReport_links[2], v, &r)) {
        ((Pad_ProcessReport_fn)k_Pad_ProcessReport_links[2].original)(pad, report, size);
        return;
    }
}
static void Pad_ProcessReport_3(uint32_t pad, uint32_t report, int32_t size) {
    openrac_script_value v[3] = {{0, 0.0}};
    v[0].i = (int64_t)pad;
    v[1].i = (int64_t)report;
    v[2].i = (int64_t)size;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Pad_ProcessReport_links[3], v, &r)) {
        ((Pad_ProcessReport_fn)k_Pad_ProcessReport_links[3].original)(pad, report, size);
        return;
    }
}
static void Pad_ProcessReport_4(uint32_t pad, uint32_t report, int32_t size) {
    openrac_script_value v[3] = {{0, 0.0}};
    v[0].i = (int64_t)pad;
    v[1].i = (int64_t)report;
    v[2].i = (int64_t)size;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Pad_ProcessReport_links[4], v, &r)) {
        ((Pad_ProcessReport_fn)k_Pad_ProcessReport_links[4].original)(pad, report, size);
        return;
    }
}
static void Pad_ProcessReport_5(uint32_t pad, uint32_t report, int32_t size) {
    openrac_script_value v[3] = {{0, 0.0}};
    v[0].i = (int64_t)pad;
    v[1].i = (int64_t)report;
    v[2].i = (int64_t)size;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Pad_ProcessReport_links[5], v, &r)) {
        ((Pad_ProcessReport_fn)k_Pad_ProcessReport_links[5].original)(pad, report, size);
        return;
    }
}
static void Pad_ProcessReport_6(uint32_t pad, uint32_t report, int32_t size) {
    openrac_script_value v[3] = {{0, 0.0}};
    v[0].i = (int64_t)pad;
    v[1].i = (int64_t)report;
    v[2].i = (int64_t)size;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Pad_ProcessReport_links[6], v, &r)) {
        ((Pad_ProcessReport_fn)k_Pad_ProcessReport_links[6].original)(pad, report, size);
        return;
    }
}
static void Pad_ProcessReport_7(uint32_t pad, uint32_t report, int32_t size) {
    openrac_script_value v[3] = {{0, 0.0}};
    v[0].i = (int64_t)pad;
    v[1].i = (int64_t)report;
    v[2].i = (int64_t)size;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[0], &k_Pad_ProcessReport_links[7], v, &r)) {
        ((Pad_ProcessReport_fn)k_Pad_ProcessReport_links[7].original)(pad, report, size);
        return;
    }
}
static void* const k_Pad_ProcessReport_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)Pad_ProcessReport_0, (void*)Pad_ProcessReport_1, (void*)Pad_ProcessReport_2, (void*)Pad_ProcessReport_3, (void*)Pad_ProcessReport_4, (void*)Pad_ProcessReport_5, (void*)Pad_ProcessReport_6, (void*)Pad_ProcessReport_7};

/* Hero_UpdateMotionInput void(): The stick read into the hero, early in his update each tick. */
typedef void (*Hero_UpdateMotionInput_fn)(void);
static openrac_script_link k_Hero_UpdateMotionInput_links[OPENRAC_SCRIPT_LINKS];
static const uint8_t k_Hero_UpdateMotionInput_kinds[] = {OPENRAC_SV_VOID};
static const char* const k_Hero_UpdateMotionInput_names[] = {NULL};
static void Hero_UpdateMotionInput_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    (void)v;
    (void)r;
    ((Hero_UpdateMotionInput_fn)original)();
}
static void Hero_UpdateMotionInput_0(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Hero_UpdateMotionInput_links[0], v, &r)) {
        ((Hero_UpdateMotionInput_fn)k_Hero_UpdateMotionInput_links[0].original)();
        return;
    }
}
static void Hero_UpdateMotionInput_1(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Hero_UpdateMotionInput_links[1], v, &r)) {
        ((Hero_UpdateMotionInput_fn)k_Hero_UpdateMotionInput_links[1].original)();
        return;
    }
}
static void Hero_UpdateMotionInput_2(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Hero_UpdateMotionInput_links[2], v, &r)) {
        ((Hero_UpdateMotionInput_fn)k_Hero_UpdateMotionInput_links[2].original)();
        return;
    }
}
static void Hero_UpdateMotionInput_3(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Hero_UpdateMotionInput_links[3], v, &r)) {
        ((Hero_UpdateMotionInput_fn)k_Hero_UpdateMotionInput_links[3].original)();
        return;
    }
}
static void Hero_UpdateMotionInput_4(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Hero_UpdateMotionInput_links[4], v, &r)) {
        ((Hero_UpdateMotionInput_fn)k_Hero_UpdateMotionInput_links[4].original)();
        return;
    }
}
static void Hero_UpdateMotionInput_5(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Hero_UpdateMotionInput_links[5], v, &r)) {
        ((Hero_UpdateMotionInput_fn)k_Hero_UpdateMotionInput_links[5].original)();
        return;
    }
}
static void Hero_UpdateMotionInput_6(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Hero_UpdateMotionInput_links[6], v, &r)) {
        ((Hero_UpdateMotionInput_fn)k_Hero_UpdateMotionInput_links[6].original)();
        return;
    }
}
static void Hero_UpdateMotionInput_7(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[1], &k_Hero_UpdateMotionInput_links[7], v, &r)) {
        ((Hero_UpdateMotionInput_fn)k_Hero_UpdateMotionInput_links[7].original)();
        return;
    }
}
static void* const k_Hero_UpdateMotionInput_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)Hero_UpdateMotionInput_0, (void*)Hero_UpdateMotionInput_1, (void*)Hero_UpdateMotionInput_2, (void*)Hero_UpdateMotionInput_3, (void*)Hero_UpdateMotionInput_4, (void*)Hero_UpdateMotionInput_5, (void*)Hero_UpdateMotionInput_6, (void*)Hero_UpdateMotionInput_7};

/* Hero_TurnToHeading void(i32,f32,f32,f32): Eases the hero's facing toward his move yaw. */
typedef void (*Hero_TurnToHeading_fn)(int32_t, float, float, float);
static openrac_script_link k_Hero_TurnToHeading_links[OPENRAC_SCRIPT_LINKS];
static const uint8_t k_Hero_TurnToHeading_kinds[] = {OPENRAC_SV_I32, OPENRAC_SV_F32, OPENRAC_SV_F32, OPENRAC_SV_F32};
static const char* const k_Hero_TurnToHeading_names[] = {"mode", "accel", "damping", "limit"};
static void Hero_TurnToHeading_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    (void)r;
    ((Hero_TurnToHeading_fn)original)((int32_t)v[0].i, (float)v[1].f, (float)v[2].f, (float)v[3].f);
}
static void Hero_TurnToHeading_0(int32_t mode, float accel, float damping, float limit) {
    openrac_script_value v[4] = {{0, 0.0}};
    v[0].i = (int64_t)mode;
    v[1].f = (double)accel;
    v[2].f = (double)damping;
    v[3].f = (double)limit;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[2], &k_Hero_TurnToHeading_links[0], v, &r)) {
        ((Hero_TurnToHeading_fn)k_Hero_TurnToHeading_links[0].original)(mode, accel, damping, limit);
        return;
    }
}
static void Hero_TurnToHeading_1(int32_t mode, float accel, float damping, float limit) {
    openrac_script_value v[4] = {{0, 0.0}};
    v[0].i = (int64_t)mode;
    v[1].f = (double)accel;
    v[2].f = (double)damping;
    v[3].f = (double)limit;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[2], &k_Hero_TurnToHeading_links[1], v, &r)) {
        ((Hero_TurnToHeading_fn)k_Hero_TurnToHeading_links[1].original)(mode, accel, damping, limit);
        return;
    }
}
static void Hero_TurnToHeading_2(int32_t mode, float accel, float damping, float limit) {
    openrac_script_value v[4] = {{0, 0.0}};
    v[0].i = (int64_t)mode;
    v[1].f = (double)accel;
    v[2].f = (double)damping;
    v[3].f = (double)limit;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[2], &k_Hero_TurnToHeading_links[2], v, &r)) {
        ((Hero_TurnToHeading_fn)k_Hero_TurnToHeading_links[2].original)(mode, accel, damping, limit);
        return;
    }
}
static void Hero_TurnToHeading_3(int32_t mode, float accel, float damping, float limit) {
    openrac_script_value v[4] = {{0, 0.0}};
    v[0].i = (int64_t)mode;
    v[1].f = (double)accel;
    v[2].f = (double)damping;
    v[3].f = (double)limit;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[2], &k_Hero_TurnToHeading_links[3], v, &r)) {
        ((Hero_TurnToHeading_fn)k_Hero_TurnToHeading_links[3].original)(mode, accel, damping, limit);
        return;
    }
}
static void Hero_TurnToHeading_4(int32_t mode, float accel, float damping, float limit) {
    openrac_script_value v[4] = {{0, 0.0}};
    v[0].i = (int64_t)mode;
    v[1].f = (double)accel;
    v[2].f = (double)damping;
    v[3].f = (double)limit;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[2], &k_Hero_TurnToHeading_links[4], v, &r)) {
        ((Hero_TurnToHeading_fn)k_Hero_TurnToHeading_links[4].original)(mode, accel, damping, limit);
        return;
    }
}
static void Hero_TurnToHeading_5(int32_t mode, float accel, float damping, float limit) {
    openrac_script_value v[4] = {{0, 0.0}};
    v[0].i = (int64_t)mode;
    v[1].f = (double)accel;
    v[2].f = (double)damping;
    v[3].f = (double)limit;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[2], &k_Hero_TurnToHeading_links[5], v, &r)) {
        ((Hero_TurnToHeading_fn)k_Hero_TurnToHeading_links[5].original)(mode, accel, damping, limit);
        return;
    }
}
static void Hero_TurnToHeading_6(int32_t mode, float accel, float damping, float limit) {
    openrac_script_value v[4] = {{0, 0.0}};
    v[0].i = (int64_t)mode;
    v[1].f = (double)accel;
    v[2].f = (double)damping;
    v[3].f = (double)limit;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[2], &k_Hero_TurnToHeading_links[6], v, &r)) {
        ((Hero_TurnToHeading_fn)k_Hero_TurnToHeading_links[6].original)(mode, accel, damping, limit);
        return;
    }
}
static void Hero_TurnToHeading_7(int32_t mode, float accel, float damping, float limit) {
    openrac_script_value v[4] = {{0, 0.0}};
    v[0].i = (int64_t)mode;
    v[1].f = (double)accel;
    v[2].f = (double)damping;
    v[3].f = (double)limit;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[2], &k_Hero_TurnToHeading_links[7], v, &r)) {
        ((Hero_TurnToHeading_fn)k_Hero_TurnToHeading_links[7].original)(mode, accel, damping, limit);
        return;
    }
}
static void* const k_Hero_TurnToHeading_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)Hero_TurnToHeading_0, (void*)Hero_TurnToHeading_1, (void*)Hero_TurnToHeading_2, (void*)Hero_TurnToHeading_3, (void*)Hero_TurnToHeading_4, (void*)Hero_TurnToHeading_5, (void*)Hero_TurnToHeading_6, (void*)Hero_TurnToHeading_7};

/* Hero_SetPlanarVelocity void(f32): The hero's velocity at his ground speed along an angle; an angle above pi means his facing. */
typedef void (*Hero_SetPlanarVelocity_fn)(float);
static openrac_script_link k_Hero_SetPlanarVelocity_links[OPENRAC_SCRIPT_LINKS];
static const uint8_t k_Hero_SetPlanarVelocity_kinds[] = {OPENRAC_SV_F32};
static const char* const k_Hero_SetPlanarVelocity_names[] = {"angle"};
static void Hero_SetPlanarVelocity_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    (void)r;
    ((Hero_SetPlanarVelocity_fn)original)((float)v[0].f);
}
static void Hero_SetPlanarVelocity_0(float angle) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].f = (double)angle;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[3], &k_Hero_SetPlanarVelocity_links[0], v, &r)) {
        ((Hero_SetPlanarVelocity_fn)k_Hero_SetPlanarVelocity_links[0].original)(angle);
        return;
    }
}
static void Hero_SetPlanarVelocity_1(float angle) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].f = (double)angle;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[3], &k_Hero_SetPlanarVelocity_links[1], v, &r)) {
        ((Hero_SetPlanarVelocity_fn)k_Hero_SetPlanarVelocity_links[1].original)(angle);
        return;
    }
}
static void Hero_SetPlanarVelocity_2(float angle) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].f = (double)angle;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[3], &k_Hero_SetPlanarVelocity_links[2], v, &r)) {
        ((Hero_SetPlanarVelocity_fn)k_Hero_SetPlanarVelocity_links[2].original)(angle);
        return;
    }
}
static void Hero_SetPlanarVelocity_3(float angle) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].f = (double)angle;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[3], &k_Hero_SetPlanarVelocity_links[3], v, &r)) {
        ((Hero_SetPlanarVelocity_fn)k_Hero_SetPlanarVelocity_links[3].original)(angle);
        return;
    }
}
static void Hero_SetPlanarVelocity_4(float angle) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].f = (double)angle;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[3], &k_Hero_SetPlanarVelocity_links[4], v, &r)) {
        ((Hero_SetPlanarVelocity_fn)k_Hero_SetPlanarVelocity_links[4].original)(angle);
        return;
    }
}
static void Hero_SetPlanarVelocity_5(float angle) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].f = (double)angle;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[3], &k_Hero_SetPlanarVelocity_links[5], v, &r)) {
        ((Hero_SetPlanarVelocity_fn)k_Hero_SetPlanarVelocity_links[5].original)(angle);
        return;
    }
}
static void Hero_SetPlanarVelocity_6(float angle) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].f = (double)angle;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[3], &k_Hero_SetPlanarVelocity_links[6], v, &r)) {
        ((Hero_SetPlanarVelocity_fn)k_Hero_SetPlanarVelocity_links[6].original)(angle);
        return;
    }
}
static void Hero_SetPlanarVelocity_7(float angle) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].f = (double)angle;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[3], &k_Hero_SetPlanarVelocity_links[7], v, &r)) {
        ((Hero_SetPlanarVelocity_fn)k_Hero_SetPlanarVelocity_links[7].original)(angle);
        return;
    }
}
static void* const k_Hero_SetPlanarVelocity_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)Hero_SetPlanarVelocity_0, (void*)Hero_SetPlanarVelocity_1, (void*)Hero_SetPlanarVelocity_2, (void*)Hero_SetPlanarVelocity_3, (void*)Hero_SetPlanarVelocity_4, (void*)Hero_SetPlanarVelocity_5, (void*)Hero_SetPlanarVelocity_6, (void*)Hero_SetPlanarVelocity_7};

/* FollowCamera_ApplyOverrides void(u32): The follow camera's per-tick requests from the hero's state; the camera moby's update calls it before it places the eye. */
typedef void (*FollowCamera_ApplyOverrides_fn)(uint32_t);
static openrac_script_link k_FollowCamera_ApplyOverrides_links[OPENRAC_SCRIPT_LINKS];
static const uint8_t k_FollowCamera_ApplyOverrides_kinds[] = {OPENRAC_SV_U32};
static const char* const k_FollowCamera_ApplyOverrides_names[] = {"camera"};
static void FollowCamera_ApplyOverrides_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    (void)r;
    ((FollowCamera_ApplyOverrides_fn)original)((uint32_t)v[0].i);
}
static void FollowCamera_ApplyOverrides_0(uint32_t camera) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)camera;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[4], &k_FollowCamera_ApplyOverrides_links[0], v, &r)) {
        ((FollowCamera_ApplyOverrides_fn)k_FollowCamera_ApplyOverrides_links[0].original)(camera);
        return;
    }
}
static void FollowCamera_ApplyOverrides_1(uint32_t camera) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)camera;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[4], &k_FollowCamera_ApplyOverrides_links[1], v, &r)) {
        ((FollowCamera_ApplyOverrides_fn)k_FollowCamera_ApplyOverrides_links[1].original)(camera);
        return;
    }
}
static void FollowCamera_ApplyOverrides_2(uint32_t camera) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)camera;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[4], &k_FollowCamera_ApplyOverrides_links[2], v, &r)) {
        ((FollowCamera_ApplyOverrides_fn)k_FollowCamera_ApplyOverrides_links[2].original)(camera);
        return;
    }
}
static void FollowCamera_ApplyOverrides_3(uint32_t camera) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)camera;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[4], &k_FollowCamera_ApplyOverrides_links[3], v, &r)) {
        ((FollowCamera_ApplyOverrides_fn)k_FollowCamera_ApplyOverrides_links[3].original)(camera);
        return;
    }
}
static void FollowCamera_ApplyOverrides_4(uint32_t camera) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)camera;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[4], &k_FollowCamera_ApplyOverrides_links[4], v, &r)) {
        ((FollowCamera_ApplyOverrides_fn)k_FollowCamera_ApplyOverrides_links[4].original)(camera);
        return;
    }
}
static void FollowCamera_ApplyOverrides_5(uint32_t camera) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)camera;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[4], &k_FollowCamera_ApplyOverrides_links[5], v, &r)) {
        ((FollowCamera_ApplyOverrides_fn)k_FollowCamera_ApplyOverrides_links[5].original)(camera);
        return;
    }
}
static void FollowCamera_ApplyOverrides_6(uint32_t camera) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)camera;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[4], &k_FollowCamera_ApplyOverrides_links[6], v, &r)) {
        ((FollowCamera_ApplyOverrides_fn)k_FollowCamera_ApplyOverrides_links[6].original)(camera);
        return;
    }
}
static void FollowCamera_ApplyOverrides_7(uint32_t camera) {
    openrac_script_value v[1] = {{0, 0.0}};
    v[0].i = (int64_t)camera;
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[4], &k_FollowCamera_ApplyOverrides_links[7], v, &r)) {
        ((FollowCamera_ApplyOverrides_fn)k_FollowCamera_ApplyOverrides_links[7].original)(camera);
        return;
    }
}
static void* const k_FollowCamera_ApplyOverrides_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)FollowCamera_ApplyOverrides_0, (void*)FollowCamera_ApplyOverrides_1, (void*)FollowCamera_ApplyOverrides_2, (void*)FollowCamera_ApplyOverrides_3, (void*)FollowCamera_ApplyOverrides_4, (void*)FollowCamera_ApplyOverrides_5, (void*)FollowCamera_ApplyOverrides_6, (void*)FollowCamera_ApplyOverrides_7};

/* Hero_AdvanceAnimation void(): Ratchet's animation moved on by his animation speed, once a tick after his state's update. */
typedef void (*Hero_AdvanceAnimation_fn)(void);
static openrac_script_link k_Hero_AdvanceAnimation_links[OPENRAC_SCRIPT_LINKS];
static const uint8_t k_Hero_AdvanceAnimation_kinds[] = {OPENRAC_SV_VOID};
static const char* const k_Hero_AdvanceAnimation_names[] = {NULL};
static void Hero_AdvanceAnimation_call(void* original, const openrac_script_value* v, openrac_script_value* r) {
    (void)v;
    (void)r;
    ((Hero_AdvanceAnimation_fn)original)();
}
static void Hero_AdvanceAnimation_0(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[5], &k_Hero_AdvanceAnimation_links[0], v, &r)) {
        ((Hero_AdvanceAnimation_fn)k_Hero_AdvanceAnimation_links[0].original)();
        return;
    }
}
static void Hero_AdvanceAnimation_1(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[5], &k_Hero_AdvanceAnimation_links[1], v, &r)) {
        ((Hero_AdvanceAnimation_fn)k_Hero_AdvanceAnimation_links[1].original)();
        return;
    }
}
static void Hero_AdvanceAnimation_2(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[5], &k_Hero_AdvanceAnimation_links[2], v, &r)) {
        ((Hero_AdvanceAnimation_fn)k_Hero_AdvanceAnimation_links[2].original)();
        return;
    }
}
static void Hero_AdvanceAnimation_3(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[5], &k_Hero_AdvanceAnimation_links[3], v, &r)) {
        ((Hero_AdvanceAnimation_fn)k_Hero_AdvanceAnimation_links[3].original)();
        return;
    }
}
static void Hero_AdvanceAnimation_4(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[5], &k_Hero_AdvanceAnimation_links[4], v, &r)) {
        ((Hero_AdvanceAnimation_fn)k_Hero_AdvanceAnimation_links[4].original)();
        return;
    }
}
static void Hero_AdvanceAnimation_5(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[5], &k_Hero_AdvanceAnimation_links[5], v, &r)) {
        ((Hero_AdvanceAnimation_fn)k_Hero_AdvanceAnimation_links[5].original)();
        return;
    }
}
static void Hero_AdvanceAnimation_6(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[5], &k_Hero_AdvanceAnimation_links[6], v, &r)) {
        ((Hero_AdvanceAnimation_fn)k_Hero_AdvanceAnimation_links[6].original)();
        return;
    }
}
static void Hero_AdvanceAnimation_7(void) {
    openrac_script_value v[1] = {{0, 0.0}};
    openrac_script_value r = {0, 0.0};
    if (!openrac_script_dispatch(&k_points[5], &k_Hero_AdvanceAnimation_links[7], v, &r)) {
        ((Hero_AdvanceAnimation_fn)k_Hero_AdvanceAnimation_links[7].original)();
        return;
    }
}
static void* const k_Hero_AdvanceAnimation_thunks[OPENRAC_SCRIPT_LINKS] = {(void*)Hero_AdvanceAnimation_0, (void*)Hero_AdvanceAnimation_1, (void*)Hero_AdvanceAnimation_2, (void*)Hero_AdvanceAnimation_3, (void*)Hero_AdvanceAnimation_4, (void*)Hero_AdvanceAnimation_5, (void*)Hero_AdvanceAnimation_6, (void*)Hero_AdvanceAnimation_7};

static const openrac_script_point k_points[6] = {
    {"Pad_ProcessReport", "void(u32,u32,i32)", 3, k_Pad_ProcessReport_kinds, k_Pad_ProcessReport_names, OPENRAC_SV_VOID, k_Pad_ProcessReport_links, k_Pad_ProcessReport_thunks, Pad_ProcessReport_call},
    {"Hero_UpdateMotionInput", "void()", 0, k_Hero_UpdateMotionInput_kinds, k_Hero_UpdateMotionInput_names, OPENRAC_SV_VOID, k_Hero_UpdateMotionInput_links, k_Hero_UpdateMotionInput_thunks, Hero_UpdateMotionInput_call},
    {"Hero_TurnToHeading", "void(i32,f32,f32,f32)", 4, k_Hero_TurnToHeading_kinds, k_Hero_TurnToHeading_names, OPENRAC_SV_VOID, k_Hero_TurnToHeading_links, k_Hero_TurnToHeading_thunks, Hero_TurnToHeading_call},
    {"Hero_SetPlanarVelocity", "void(f32)", 1, k_Hero_SetPlanarVelocity_kinds, k_Hero_SetPlanarVelocity_names, OPENRAC_SV_VOID, k_Hero_SetPlanarVelocity_links, k_Hero_SetPlanarVelocity_thunks, Hero_SetPlanarVelocity_call},
    {"FollowCamera_ApplyOverrides", "void(u32)", 1, k_FollowCamera_ApplyOverrides_kinds, k_FollowCamera_ApplyOverrides_names, OPENRAC_SV_VOID, k_FollowCamera_ApplyOverrides_links, k_FollowCamera_ApplyOverrides_thunks, FollowCamera_ApplyOverrides_call},
    {"Hero_AdvanceAnimation", "void()", 0, k_Hero_AdvanceAnimation_kinds, k_Hero_AdvanceAnimation_names, OPENRAC_SV_VOID, k_Hero_AdvanceAnimation_links, k_Hero_AdvanceAnimation_thunks, Hero_AdvanceAnimation_call},
};

static const char* const k_discs[] = {"pal", "ntsc"};
static const uint32_t k_field_0[] = {0x0013F4D0u, 0x0013F3D0u}; /* hero.position.x */
static const uint32_t k_field_1[] = {0x0013F4D4u, 0x0013F3D4u}; /* hero.position.y */
static const uint32_t k_field_2[] = {0x0013F4D8u, 0x0013F3D8u}; /* hero.position.z */
static const uint32_t k_field_3[] = {0x0013F4E8u, 0x0013F3E8u}; /* hero.facing */
static const uint32_t k_field_4[] = {0x0013F5D0u, 0x0013F4D0u}; /* hero.move_yaw */
static const uint32_t k_field_5[] = {0x001414D4u, 0x001413D4u}; /* hero.state */
static const uint32_t k_field_6[] = {0x0015EE98u, 0x0015ED98u}; /* bolts */
static const openrac_script_field k_fields[7] = {
    {"hero.position.x", OPENRAC_SV_F32, 0, k_field_0},
    {"hero.position.y", OPENRAC_SV_F32, 0, k_field_1},
    {"hero.position.z", OPENRAC_SV_F32, 0, k_field_2},
    {"hero.facing", OPENRAC_SV_F32, 0, k_field_3},
    {"hero.move_yaw", OPENRAC_SV_F32, 0, k_field_4},
    {"hero.state", OPENRAC_SV_I32, 1, k_field_5},
    {"bolts", OPENRAC_SV_I32, 0, k_field_6},
};

static const openrac_script_binding k_binding = {
    "rac1", 2, k_discs, 6, k_points, 7, k_fields, NULL,
};

const openrac_script_binding* openrac_game_script_binding(void) {
    return &k_binding;
}
