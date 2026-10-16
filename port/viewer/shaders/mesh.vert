#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Extracted level geometry, instanced: one matrix per placed object, in the
// game's axes (Z up). The projection has reversed depth.
//
// A live moby is skinned first: its vertex is the weighted sum of its joints'
// matrices from the moby's joint palette (F_j = P_j S_j, as the game's
// MobyAnimEval builds it), applied to the bind-pose vertex. The palettes are
// a float texture, four texels (the matrix's columns) per joint; `palette` is
// the instance's first matrix, -1 for an object drawn as stored.
//
// A lit tie (`lights` >= 0) colours each vertex with its instance's colour for
// the vertex's light slot, from the light texture: what LightTies computed.

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 colour;
layout(location = 4) in mat4 model;  // locations 4 to 7
layout(location = 8) in vec4 tint;
layout(location = 9) in float palette;
layout(location = 10) in uvec4 joints;
layout(location = 11) in vec4 weights;
layout(location = 12) in float light_slot;
layout(location = 13) in float lights;
layout(location = 14) in vec4 moby_light;    // set 0, set 1, cross-fade, 1 to light
layout(location = 15) in vec4 moby_ambient;  // ambient RGB / 128; w: 1 + glow RGB (mode 0x10), else 0

uniform mat4 view_projection;
uniform mat4 view;
uniform float picture_scale;  // the game's 4:3 picture's width as a part of the target's (1: all of it)
// The game's fog line (LevelScene::set_fog): F = depth x slope + offset, clamped to [z, w].
uniform vec4 fog_params;
uniform int skinning;
uniform sampler2D palette_texture;
uniform sampler2D light_texture;
// The level's 16 directional sets: colour A (w = back factor), direction A, colour B, direction B.
uniform vec4 light_sets[64];
// The metal ("shine") pass (MobyProc's metal packets): texture coordinates from the view-space
// normal, a sphere map of the level's chrome or glass picture, the vertex colour 0x70 and the
// alpha the moby's distance gives (ReRAC moby_skinning_lighting.md 6 and 9).
uniform int metal_pass;

out vec2 v_uv;
out vec4 v_colour;
out vec3 v_normal;
flat out int v_lit;
flat out int v_glow;  // a glow packet's vertex drawn in the moby's glow colour
// GS F / 255: interpolated in screen space, as the GS does.
noperspective out float v_fog;

const int kPaletteWidth = 1024;
const int kLightWidth = 1024;

mat4 joint_matrix(int index) {
    int texel = 4 * index;
    ivec2 at = ivec2(texel % kPaletteWidth, texel / kPaletteWidth);
    return mat4(
        texelFetch(palette_texture, at, 0),
        texelFetch(palette_texture, at + ivec2(1, 0), 0),
        texelFetch(palette_texture, at + ivec2(2, 0), 0),
        texelFetch(palette_texture, at + ivec2(3, 0), 0)
    );
}

void main() {
    vec4 p = vec4(position, 1.0);
    vec3 n = normal;
    float total = weights.x + weights.y + weights.z + weights.w;
    if (skinning != 0 && palette >= 0.0 && total > 0.0) {
        int base = int(palette + 0.5);
        mat4 skin = weights.x * joint_matrix(base + int(joints.x))
                  + weights.y * joint_matrix(base + int(joints.y))
                  + weights.z * joint_matrix(base + int(joints.z))
                  + weights.w * joint_matrix(base + int(joints.w));
        p = vec4((skin * p).xyz, 1.0);
        n = mat3(skin) * n;
    }
    vec4 world = model * p;
    gl_Position = view_projection * world;
    // A moby of the 2D layer (a page menu's frame object, moby_light.w = 2) on a target wider than
    // the game's picture: cut at the picture's sides, where the television's edge cut it.
    bool in_picture = moby_light.w > 1.5 && picture_scale < 1.0;
    gl_ClipDistance[0] = in_picture ? picture_scale * gl_Position.w - gl_Position.x : 1.0;
    gl_ClipDistance[1] = in_picture ? picture_scale * gl_Position.w + gl_Position.x : 1.0;
    // Camera depth in the game's raw units (game units x 1024); the view looks down -Z.
    float depth = -(view * world).z * 1024.0;
    v_fog = trunc(clamp(depth * fog_params.x + fog_params.y, fog_params.z, fog_params.w)) / 255.0;
    v_uv = uv;
    v_colour = colour * tint;
    v_lit = 0;
    v_glow = 0;
    // A lit tie's vertex takes its instance's colour for its light slot, as VU1 does
    // (0x80 = 1.0 under the GS's MODULATE).
    if (lights >= 0.0 && light_slot >= 0.0) {
        int texel = int(lights + 0.5) + int(light_slot + 0.5);
        v_colour = texelFetch(light_texture, ivec2(texel % kLightWidth, texel / kLightWidth), 0)
                 * (255.0 / 128.0) * tint;
        v_lit = 1;
    }
    // A live moby, lit as MobyProc and its VU0 pass light it: its set's two directional
    // lights (or two sets cross-faded) on the skinned normal, with the leaky back clamp
    // max(d, -|K| d), plus its ambient colour, times the vertex's multiplier (all 0x80 = 1.0).
    if (moby_light.w > 0.5) {
        int s0 = (int(moby_light.x + 0.5) & 15) * 4;
        int s1 = (int(moby_light.y + 0.5) & 15) * 4;
        float t = moby_light.z;
        vec4 ca = light_sets[s0], cb = light_sets[s0 + 2];
        vec3 da = light_sets[s0 + 1].xyz, db = light_sets[s0 + 3].xyz;
        if (t > 0.0) {
            ca = mix(ca, light_sets[s1], t);
            cb = mix(cb, light_sets[s1 + 2], t);
            da = normalize(mix(da, light_sets[s1 + 1].xyz, t));
            db = normalize(mix(db, light_sets[s1 + 3].xyz, t));
        }
        vec3 nw = normalize(mat3(model) * n);
        float fa = dot(-da, nw);
        float fb = dot(-db, nw);
        fa = max(fa, -abs(ca.w) * fa);
        fb = max(fb, -abs(cb.w) * fb);
        vec3 lit = moby_ambient.rgb + ca.rgb * fa + cb.rgb * fb;
        v_colour = vec4(min(lit * colour.rgb, vec3(255.0 / 128.0)), colour.a) * tint;
        // A glow packet's vertex (slot -2) of a moby whose mode has 0x10: its glow RGB, unlit,
        // with no multiplier (MobyProc's glow list overwrites the lit colours).
        if (light_slot < -1.5 && moby_ambient.w > 0.5) {
            int glow = int(moby_ambient.w - 1.0 + 0.5);
            v_colour = vec4(float(glow & 255), float((glow >> 8) & 255), float((glow >> 16) & 255), 128.0)
                     / 128.0 * tint;
            v_glow = 1;
        }
        v_lit = 1;
    }
    v_normal = mat3(model) * n;
    if (metal_pass != 0) {
        // st = (e.xy + 1) / 2, e the normal in camera space (x right, y down: t grows downward).
        vec3 e = normalize(mat3(view) * normalize(mat3(model) * n));
        v_uv = vec2(e.x * 0.5 + 0.5, 0.5 - e.y * 0.5);
        // The shine alpha: full within 8 game units of the camera, none past 24 (MobyProc's gate with
        // the default shine distance 0x18: min(((0x18 << 10) - depth) >> 7, 0x80), depth in raw units).
        float centre = -(view * model[3]).z;
        float shine = clamp((24.0 - centre) / 16.0, 0.0, 1.0);
        v_colour = vec4(vec3(112.0 / 128.0), shine);
        // MobyProc's pull toward the camera (GS Z + 1000 Q): a little nearer, so the shine is not
        // lost to the moby's own surface under it (reversed depth: larger is nearer).
        gl_Position.z += 1.0e-4 * gl_Position.w;
        v_lit = 1;
        v_glow = 0;
    }
}
