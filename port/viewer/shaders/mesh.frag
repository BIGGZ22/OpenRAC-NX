#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Texture times vertex colour, alpha-tested for cutout materials, with a
// fixed sun so shapes read where the export has no lighting. The editor's PNGs
// carry alpha from 0 to 255 (it doubles the chip's 0..0x80), so the cutout
// threshold is plain 0.5. A vertex colour of 1.0 is the GS's 0x80: the product
// is the GS's MODULATE, (texel x colour) >> 7, clamped like the chip's.

in vec2 v_uv;
in vec4 v_colour;
in vec3 v_normal;
in vec3 v_world_position;
in vec4 v_probe_multiplier;
flat in vec4 v_probe_selector;
flat in int v_lit;
flat in int v_glow;
in vec3 v_script_emission;
in vec3 v_emission_colour;
noperspective in float v_fog;

uniform sampler2D tex_T0;
uniform vec4 material_colour;
uniform int cutout;
uniform int lighting;
uniform int glow_pass;  // mobys: 0 the solid faces, 1 the glow faces (blended)
// Cutout materials in two passes, as the GS draws them (alpha test GEQUAL 0x80, failing pixels
// written as colour only): 0 the opaque texels, with depth; 1 the translucent ones, blended.
uniform int translucent_pass;
// Mobys MobyProc blends (an explosion's flash, a fading pickup): every face and texel, its alpha
// the texel's times the moby's (the tint), blended by the pass.
uniform int effect_pass;
// The metal pass: the chrome or glass texel times the vertex colour, its alpha the texel's times the
// shine, blended over the moby (mesh.vert; ALPHA 0x44 as the moby's own).
uniform int metal_pass;
uniform vec4 fog_colour;  // FOGCOL; w = 1: fog on
uniform mat4 view;
#ifdef OPENRAC_ENHANCED_RENDERING
uniform int gtao_enabled;
uniform vec3 material_emission;
uniform sampler2D emission_texture;
uniform int emission_mask;
uniform int emission_vertex_colour;
uniform float material_diffuse_scale;
uniform sampler2D gtao_visibility;
#endif

layout(location = 0) out vec4 out_colour;

// From above and to one side, in game axes (Z up).
const vec3 kSun = vec3(0.36, 0.48, 0.80);

#ifdef OPENRAC_ENHANCED_RENDERING
// Match bake_material: repeat wrapping, decode sRGB before bilinear filtering.
// These level textures are stored as RGBA8 for the original GS colour path.
vec3 emission_texel(bool authored_tint) {
    ivec2 size = textureSize(emission_texture, 0);
    vec2 p = fract(v_uv) * vec2(size) - 0.5;
    ivec2 at = ivec2(floor(p));
    vec2 f = fract(p);
    vec3 result = vec3(0.0);
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            ivec2 q = ((at + ivec2(x, y)) % size + size) % size;
            float weight = (x == 0 ? 1.0 - f.x : f.x) * (y == 0 ? 1.0 - f.y : f.y);
            vec3 srgb = texelFetch(emission_texture, q, 0).rgb;
            if (authored_tint) {
                srgb = clamp(srgb * v_emission_colour, vec3(0.0), vec3(1.0));
            }
            result += probe_linear(srgb) * weight;
        }
    }
    return result;
}

vec3 surface_emission(vec4 texel) {
    vec3 emitted = probe_linear(texel.rgb) * v_script_emission;
    if (any(greaterThan(material_emission, vec3(0.0)))) {
        vec3 radiance = emission_texel(emission_vertex_colour != 0);
        float coverage = 1.0;
        if (emission_mask == 1) {
            vec3 mask = emission_vertex_colour != 0 ? emission_texel(false) : radiance;
            float chroma = (mask.g - max(mask.r, mask.b)) / max(mask.g, 1e-5);
            coverage = smoothstep(0.25, 0.5, chroma);
        }
        emitted += material_emission * radiance * coverage;
    }
    return emitted;
}

// probes.glsl is inserted after the version line by probe_mesh_fragment().
#ifdef OPENRAC_EARLY_LIGHTING
bool early_surface(
    vec3 eye_position,
    vec3 normal_view,
    out vec4 result,
    out bool covered,
    out float hero_visibility
);
#endif
// This replaces RGB only. Alpha tests, fading and blend equations stay in
// the existing path, as do explicitly unlit, additive, metal and glow passes.
vec3 probe_surface(vec4 texel, vec3 original) {
    if (probe_receiver == 0 || v_glow != 0 || probe_effect != 0 || metal_pass != 0
        || (probe_receiver == 2 && v_probe_selector.w > 1.5)) {
        return original;
    }
    if (probe_mode == 0 && sun_shadow_enabled == 0 && hero_shadow_enabled == 0) {
        return original;
    }
    vec3 geometric = cross(dFdx(v_world_position), dFdy(v_world_position));
    vec3 n = probe_unit(dot(v_normal, v_normal) > 1e-12 ? v_normal : geometric);
    if (dot(n, n) < 0.5) {
        return original;
    }
    // These surfaces render double-sided. Orient the shading normal toward
    // the eye, independently of winding (some terrain normals point inward).
    vec3 eye_position = (view * vec4(v_world_position, 1.0)).xyz;
    if (dot(mat3(view) * n, -eye_position) < 0.0) {
        n = -n;
    }
#ifdef OPENRAC_EARLY_LIGHTING
    bool cached_coverage;
    vec4 cached_lighting;
    float hero_visibility;
    early_surface(
        eye_position, mat3(view) * n, cached_lighting, cached_coverage, hero_visibility
    );
    if (!hero_shadow_applies(n)) {
        hero_visibility = 1.0;
    }
    float sun_visibility = cached_lighting.a;
#else
    float sun_visibility = sun_shadow_visibility(v_world_position, n, n);
    float hero_visibility = hero_shadow_visibility(v_world_position, n);
#endif
    // Either projection casts a normal shadow in sunlight, including beyond cascade coverage.
    float visibility = min(sun_visibility, hero_visibility);
    // Only the airborne footprint adds a platforming cue inside existing cascade shade.
    float hero_cue = 1.0;
    if (hero_shadow_receiver == 0) {
        hero_cue = 1.0 - 0.45 * hero_shadow_air_blend * (1.0 - sun_visibility) * (1.0 - hero_visibility);
    }
    // Without a probe asset, retain the original authored lighting and apply
    // the same shadow floor. This also covers holes in probe coverage.
    vec3 emitted = surface_emission(texel);
    // Legacy prelighting may already contain authored brightness. Preserve that
    // component while shadowing the remaining diffuse, without adding it twice.
    vec3 fallback = probe_display(
        max(probe_linear(original) - emitted, vec3(0.0)) * mix(0.1, 1.0, visibility) * hero_cue
        + emitted
    );
    if (probe_mode == 0) {
        return fallback;
    }
    vec3 indirect;
    // Use the same smooth receiver normal as the depth/normal prepass and early lighting.
    vec3 toward_camera = transpose(mat3(view)) * (-eye_position);
    vec3 surface_bias = probe_surface_bias(n, toward_camera);
#ifdef OPENRAC_EARLY_LIGHTING
    indirect = cached_lighting.rgb;
    bool covered = cached_coverage;
#else
    bool covered = sample_probe_irradiance(v_world_position, n, surface_bias, indirect);
#endif
    if (probe_mode == 4) {
        return covered ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 1.0);
    }
    // Diagnostic views must not silently substitute original lighting:
    // indirect is black without coverage, while direct is independent of GI.
    if (!covered && probe_mode == 2) {
        return vec3(0.0);
    }
    if (!covered && probe_mode != 3) {
        return fallback;
    }
    // Artistic GI exposure; keep the bake and direct-light energy unchanged.
    indirect *= probe_gi_multiplier;
    // Only opaque diffuse GI receives AO. Blended/effect surfaces do not have
    // a matching prepass depth; direct light, emission and metal stay intact.
#ifndef OPENRAC_EARLY_LIGHTING
    if (gtao_enabled != 0 && translucent_pass == 0 && effect_pass == 0) {
        ivec2 pixel = clamp(ivec2(gl_FragCoord.xy), ivec2(0), textureSize(gtao_visibility, 0) - 1);
        indirect *= texelFetch(gtao_visibility, pixel, 0).r;
    }
#endif
    vec4 selector = probe_receiver == 1 ? vec4(0.0) : v_probe_selector;
    vec3 direct = probe_direct(v_world_position, n, selector, visibility);
    vec3 illumination = probe_mode == 2
                            ? indirect / 3.14159265359
                            : (probe_mode == 3 ? direct : direct + indirect / 3.14159265359);
    vec3 albedo = probe_linear(texel.rgb) * material_colour.rgb * v_probe_multiplier.rgb;
    if (probe_terrain_albedo != 0) {
        // Preserve the GS modulation as an artistic material tint, then decode once.
        albedo = probe_linear(clamp(texel.rgb * v_colour.rgb * material_colour.rgb, 0.0, 1.0));
    }
    if (probe_mode != 2) {
        illumination *= hero_cue;
    }
    // Emission is independent of GI exposure, AO and shadow visibility.
    return probe_display(
        albedo * illumination * material_diffuse_scale + (probe_mode == 1 ? emitted : vec3(0.0))
    );
}

#else
vec3 probe_surface(vec4 texel, vec3 original) {
    return original;
}
#endif

// OPENRAC_UPSCALED_TEXTURE_COLOURS_SOURCE

void main() {
#ifdef OPENRAC_OPAQUE_COLOUR
    if (opaque_solid != 0) {
#endif
#if defined(OPENRAC_SOLID) || defined(OPENRAC_OPAQUE_COLOUR)
    // The static solid variant contains no discard. The Vulkan colour-batch
    // variant selects this original solid path or the original general path
    // per draw, preserving their texture/derivative and alpha-test ordering.
    // It only runs with reused EQUAL depth and depth writes disabled.
    vec4 texel = sample_upscaled_texture_colours(v_uv);
    vec4 c = texel * v_colour * material_colour;
    float light = 1.0;
    if (lighting != 0 && v_lit == 0) {
        light = 0.55 + 0.45 * abs(dot(normalize(v_normal), kSun));
    }
    vec3 rgb = min(c.rgb * light, vec3(1.0));
    rgb = probe_surface(texel, rgb);
    if (fog_colour.w > 0.5) {
        rgb = mix(fog_colour.rgb, rgb, v_fog);
    }
    out_colour = vec4(rgb, 1.0);
#ifdef OPENRAC_OPAQUE_COLOUR
    return;
    }
#endif
#endif
#ifndef OPENRAC_SOLID
    if (metal_pass != 0) {
        vec4 m = sample_upscaled_texture_colours(v_uv) * v_colour;
        vec3 mrgb = min(m.rgb, vec3(1.0));
        if (fog_colour.w > 0.5) {
            mrgb = mix(fog_colour.rgb, mrgb, v_fog);
        }
        out_colour = vec4(mrgb, clamp(m.a, 0.0, 1.0));
        return;
    }
    if (effect_pass == 0 && v_glow != glow_pass) {
        discard;
    }
    vec4 texel = sample_upscaled_texture_colours(v_uv);
    vec4 c = texel * v_colour * material_colour;
    if (effect_pass != 0) {
        vec3 e = min(c.rgb, vec3(1.0));
        e = probe_surface(texel, e);
        if (fog_colour.w > 0.5) {
            e = mix(fog_colour.rgb, e, v_fog);
        }
        out_colour = vec4(e, clamp(c.a, 0.0, 1.0));
        return;
    }
    // The glow pass blends every texel at its alpha (the Gadgetron hologram's lettering is mostly
    // part-transparent texels); only the solid pass leaves a cutout's part-transparent texels out.
    if (cutout != 0 && translucent_pass == 0 && glow_pass == 0 && c.a < 0.99) {
        discard;
    }
    if (translucent_pass != 0 && (cutout == 0 || c.a >= 0.99 || c.a < 0.02)) {
        discard;
    }
    float light = 1.0;
    if (lighting != 0 && v_lit == 0) {
        // Double-sided: a face lit from behind is lit too.
        light = 0.55 + 0.45 * abs(dot(normalize(v_normal), kSun));
    }
    vec3 rgb = min(c.rgb * light, vec3(1.0));
    rgb = probe_surface(texel, rgb);
    // The GS's fog, after the texture: RGB only.
    if (fog_colour.w > 0.5) {
        rgb = mix(fog_colour.rgb, rgb, v_fog);
    }
    out_colour = vec4(rgb, v_glow != 0 || translucent_pass != 0 ? clamp(c.a, 0.0, 1.0) : 1.0);
#endif
}
