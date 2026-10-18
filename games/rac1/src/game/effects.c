#include "common.h"
#include "records.h"
#include "structs.h"

/*
 * effects.cpp in the original source; text 0x1EDFF8-0x1EE9F8.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];

typedef struct { u8 pad_0[0x10]; f32 pos[4]; u8 flags; } FlareMoby;
typedef struct { FlareMoby *moby; s32 state; f32 intensity; s32 pad_c; s16 alpha[16]; s16 tex[16]; s32 color[16]; f32 dist[16]; f32 scale[16]; } LensFlare;
typedef struct { u8 pad_0[0x8]; s32 cx; s32 cy; s32 ox; s32 oy; } ScreenInfo;
extern u8 D_00187080_1edff8[] __asm__("D_00187180");
extern ScreenInfo D_0013E600_1edff8 __asm__("D_0013E600");
extern LensFlare D_00187300_1edff8 __asm__("D_00187400");
extern f32 func_001F9D10_1edff8(void *, void *) __asm__("func_001F9D10");
extern f32 ConvertIntegerToFloat(s32) __asm__("func_001FA888");
extern s32 truncate_float_to_s32(f32) __asm__("func_001FA898");
extern float AbsoluteFloat(float input) __asm__("func_001F9B88");
extern void project_to_screen(f32 *, void *) __asm__("func_001F2418");
extern s64 get_effect_texture(s32) __asm__("func_001F4868");
extern void draw_textured_quad(s32, s32, s32, s32, s32, s32, s32, s32, s64, s64) __asm__("func_001F5800");
/* Lens flare: projects the flare moby to the screen and fills the per-element alpha, texture, colour, distance and scale tables. Adapted from Lombyte (MIT) for PAL: rendering/effects/fun_001edc50.c, FUN_001edc50. */
void LensFlareUpdate(void) {
    f32 scr[4];
    f32 ctr[4];
    f32 d[4];
    f32 off[4];
    f32 at[4];
    f32 dist;
    f32 fade;
    f32 fx;
    f32 fy;
    f32 half;
    s32 i;
    s32 size;
    s64 tex;
    s32 rgba;
    s32 w;
    s32 x;
    s32 y;

    if (D_00187300_1edff8.state != 1 || (D_00187300_1edff8.moby->flags & 0x80)) {
        D_00187300_1edff8.state = 0;
        return;
    }
    if (D_00187300_1edff8.intensity <= 0.0f) {
        return;
    }
    dist = func_001F9D10_1edff8(D_00187080_1edff8, D_00187300_1edff8.moby->pos);
    ctr[0] = ConvertIntegerToFloat(D_0013E600_1edff8.cx);
    ctr[1] = ConvertIntegerToFloat(D_0013E600_1edff8.cy);
    project_to_screen(scr, D_00187300_1edff8.moby->pos);
    scr[0] = (scr[0] - ConvertIntegerToFloat(D_0013E600_1edff8.ox)) * 0.0625f;
    scr[1] = (scr[1] - ConvertIntegerToFloat(D_0013E600_1edff8.oy)) * 0.0625f;
    d[0] = ctr[0] - scr[0];
    d[1] = ctr[1] - scr[1];
    for (i = 0; i < 16; i++) {
        tex = get_effect_texture(D_00187300_1edff8.tex[i] + 6);
        size = (i != 0) ? 0x20 : 0x40;
        off[0] = d[0] * D_00187300_1edff8.dist[i];
        off[1] = d[1] * D_00187300_1edff8.dist[i];
        at[0] = ctr[0] + off[0];
        at[1] = ctr[1] + off[1];
        fade = dist - 5.0f;
        if (fade > 1.0f) {
            fade = 1.0f;
        } else if (fade < 0.0f) {
            fade = 0.0f;
        }
        fx = AbsoluteFloat(d[0]) / ctr[0] * -5.0f + 5.5f;
        if (fx > 1.0f) {
            fx = 1.0f;
        } else if (fx < 0.0f) {
            fx = 0.0f;
        }
        fy = AbsoluteFloat(d[1]) / ctr[1] * -5.0f + 5.5f;
        if (fy > 1.0f) {
            fy = 1.0f;
        } else if (fy < 0.0f) {
            fy = 0.0f;
        }
        fade = fade * (fx * fy);
        rgba = (truncate_float_to_s32(fade * D_00187300_1edff8.alpha[i]) << 24) | (D_00187300_1edff8.color[i] & 0xFFFFFF);
        w = truncate_float_to_s32(size * D_00187300_1edff8.scale[i]);
        half = w >> 1;
        x = truncate_float_to_s32(at[0] - half);
        y = truncate_float_to_s32(at[1] - half);
        draw_textured_quad(x, y, w, w, 0, 0, size, size, rgba, tex);
    }
    D_00187300_1edff8.intensity = 0.0f;
}

extern int func_001F4868(int);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001FA748(float, float);
extern void func_001F5E60(float, float, float, float, float, int, int, int, int, int, int, int,
                           float, float);

/*
 * Spawns a burst of particle effects around (arg1, arg2) using the source
 * object's unk10 (a size, scaled by 40.0f) and unk18 (looked up through
 * func_001F4868 to get a handle passed on to func_001F5E60). unk2C picks
 * the pattern: 0 -- a fan of unk26 particles, stepping the angle (unk1C)
 * by unk28 each time (func_001FA748 adds and wraps the angle); 1 -- four
 * particles offset from the point along and across the current angle;
 * 2 -- a single particle. Matching the case order (1, then 0 vs
 * negative, then 2 vs default) needed a real switch so gcc's own
 * binary-search lowering picked the same compare order as retail.
 *
 * The 40.0f constant is materialized once, before anything else (retail
 * loads it into $f24 before even the first call), because it is live
 * across every switch arm. The four offsets in case 1 live in a local
 * array, not scalars: retail spills them to fixed stack slots (0x0, 0x4,
 * 0x10, 0x14 with an 8-byte gap) instead of extra saved float registers.
 */
void DrawFlareSprites(void *arg0, float arg1, float arg2)
{
    float forty = 40.0f;
    char *p = (char *)arg0;
    int handle = GetEffectTex(*(int *)(p + 0x18));
    int mode = *(int *)(p + 0x2C);
    float angle = *(float *)(p + 0x1C);
    float k;
    int i;

    switch (mode) {
    case 0:
        for (i = 0; i < *(short *)(p + 0x26); i++) {
            func_001F5E60(arg1, arg2, forty * *(float *)(p + 0x10),
                          forty * *(float *)(p + 0x10), angle,
                          0x3F, 0x3F, handle, 0xFFFFF3, *(int *)(p + 0x14), 0, 0,
                          0.0f, 0.0f);
            angle = FastAddRots(angle, *(float *)(p + 0x28));
        }
        break;
    case 1: {
        float tmp[6];

        tmp[0] = FastSin(angle) * forty * *(float *)(p + 0x10);
        tmp[1] = FastCos(angle) * forty * *(float *)(p + 0x10);
        tmp[4] = FastCos(angle) * forty * *(float *)(p + 0x10);
        tmp[5] = FastSin(angle) * -forty * *(float *)(p + 0x10);

        k = *(float *)(p + 0x10) * forty;
        func_001F5E60(arg1, arg2, k, k, angle,
                      0x3F, 0x3F, handle, 0xFFFFF3, *(int *)(p + 0x14), 0, 0,
                      0.0f, 0.0f);

        k = *(float *)(p + 0x10);
        k *= forty;
        func_001F5E60(arg1 + tmp[4], arg2 + tmp[5], k, k, angle,
                      0x3F, 0x3F, handle, 0xFFFFF3, *(int *)(p + 0x14), 1, 0,
                      0.0f, 0.0f);

        k = *(float *)(p + 0x10);
        k *= forty;
        func_001F5E60(arg1 - tmp[0], arg2 - tmp[1], k, k, angle,
                      0x3F, 0x3F, handle, 0xFFFFF3, *(int *)(p + 0x14), 0, 1,
                      0.0f, 0.0f);

        k = *(float *)(p + 0x10);
        k *= forty;
        func_001F5E60(arg1 + tmp[4] - tmp[0], arg2 + tmp[5] - tmp[1], k, k, angle,
                      0x3F, 0x3F, handle, 0xFFFFF3, *(int *)(p + 0x14), 1, 1,
                      0.0f, 0.0f);
        break;
    }
    case 2:
        k = *(float *)(p + 0x10);
        k *= forty;
        func_001F5E60(arg1, arg2, k, k, angle,
                      0x3F, 0x3F, handle, 0xFFFFF3, *(int *)(p + 0x14), 0, 0,
                      0.5f, 0.5f);
        break;
    }
}

LINKER_REMNANT("asm/remnants/text", func_001EE6D0);

/* The queue of glows to draw this frame: 0x30-byte records, the count
   at +0xC0. */
typedef struct {
    char pad00[0x20];
    unsigned char *moby;    /* 0x20 */
    short onScreen;         /* 0x24 */
    char pad26[0xA];
} GlowSlot;

typedef struct {
    GlowSlot slot[4];
    int count;              /* 0xC0 */
} GlowQueue;

extern GlowQueue D_00189400;
extern int D_001414D4;
extern int D_0013E600[];
extern float func_001FA888(int);
extern void func_001F2418(float *, void *);
extern void func_001EE3B0(void *, float, float);

/* Draws every queued glow whose moby is live (not in state 0xFE/0xFD):
   at its projected screen position (func_001F2418, then relative to the
   D_0013E600 offset in 1/16 units) or, for off-screen ones, at the
   screen centre; then empties the queue. Nothing in level 0x72. */
void DrawQueuedGlows(void) {
    float centre[4];
    float pos[4];
    int i;

    if (D_001414D4 == 0x72) {
        D_00189400.count = 0;
    }
    if (D_00189400.count != 0) {
        centre[0] = IntToFloat(D_0013E600[2]);
        centre[1] = IntToFloat(D_0013E600[3]);
        for (i = 0; i < D_00189400.count; i++) {
            GlowSlot *s = &D_00189400.slot[i];

            if (s->moby == 0 || s->moby[0x20] == 0xFE || s->moby[0x20] == 0xFD) {
                continue;
            }
            if (s->onScreen != 0) {
                projectWorldPoint(pos, s);
                pos[0] = (pos[0] - IntToFloat(D_0013E600[4])) * 0.0625f;
                pos[1] = (pos[1] - IntToFloat(D_0013E600[5])) * 0.0625f;
            } else {
                qcopy(pos, centre);
            }
            DrawFlareSprites(s, pos[0], pos[1]);
        }
        D_00189400.count = 0;
    }
}

LINKER_REMNANT("asm/remnants/text", func_001EE850);

struct CameraEnvironmentRegion {
    u8 pad0[0x50];
    s32 flags;
    s32 negative_color;
    s32 positive_color;
    f32 negative_depth_start;
    f32 negative_value_start;
    f32 negative_depth_end;
    f32 negative_value_end;
    f32 positive_depth_start;
    f32 positive_value_start;
    f32 positive_depth_end;
    f32 positive_value_end;
    u8 pad7C[4];
};
extern struct CameraEnvironmentRegion D_0019AEC0[];
/* A fog preset: an RGB byte triple and four floats (as src/game/draw.c reads it). */
typedef struct {
    unsigned char r, g, b, pad;
    float f[4];
} FogPreset;
extern FogPreset D_0015F584 MACRO_ADDR;
extern s32 func_00213A78(void *, f32 *, s32 *);
extern s32 func_001FA898(f32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/update_camera_environment_from_regions.c,
 * update_camera_environment_from_regions; the fog preset written as the struct src/game/draw.c reads. */
void Camera_UpdateEnvironment(void *position) {
    f32 blend;
    s32 region_index;
    struct CameraEnvironmentRegion *region;
    u32 positive_weight;
    u32 negative_weight;
    s32 negative_color;
    s32 positive_color;
    f32 inverse_blend;
    u32 red;
    u32 green;
    u32 positive_red;
    u32 negative_red;
    u32 positive_green;
    u32 negative_green;
    u32 positive_blue;
    u32 negative_blue;

    if (func_00213A78(position, &blend, &region_index) == 0) {
        return;
    }
    region = &D_0019AEC0[region_index];
    if (!(region->flags & 2)) {
        return;
    }
    positive_weight = truncate_float_to_s32(blend * 255.0f);
    negative_weight = 255 - positive_weight;
    inverse_blend = 1.0f - blend;
    positive_color = region->positive_color;
    negative_color = region->negative_color;
    positive_red = (positive_color & 0xFF) * positive_weight;
    positive_green = ((positive_color >> 8) & 0xFF) * positive_weight;
    positive_blue = (positive_color >> 16) & 0xFF;
    negative_red = (negative_color & 0xFF) * negative_weight;
    negative_green = ((negative_color >> 8) & 0xFF) * negative_weight;
    negative_blue = (negative_color >> 16) & 0xFF;
    red = (s32)(positive_red + negative_red) >> 8;
    green = (s32)(positive_green + negative_green) >> 8;
    D_0015F584.b = (s32)(positive_blue * positive_weight + negative_blue * negative_weight) >> 8;
    D_0015F584.r = red;
    D_0015F584.g = green;
    D_0015F584.f[0] =
        (region->positive_depth_start * blend + region->negative_depth_start * inverse_blend) *
        1024.0f;
    D_0015F584.f[1] =
        (region->positive_depth_end * blend + region->negative_depth_end * inverse_blend) * 1024.0f;
    D_0015F584.f[2] = 255.0f - (region->positive_value_start * blend +
                                   region->negative_value_start * inverse_blend) *
                                      255.0f;
    D_0015F584.f[3] =
        255.0f -
        (region->positive_value_end * blend + region->negative_value_end * inverse_blend) * 255.0f;
}

LINKER_REMNANT("asm/remnants/text", func_001EE9E8);
