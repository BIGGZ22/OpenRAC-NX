// SPDX-License-Identifier: GPL-3.0-or-later
// Shared fragment sampler; CPU reference: lighting/probes.cpp. All coordinates
// and distance moments are in game world units. Atlas stores E^(1/5), where E
// is full irradiance, not E/pi. Sampling returns linear E.
uniform sampler2D probe_irradiance;
uniform sampler2D probe_moments;
uniform sampler2D probe_metadata;
uniform vec3 probe_origin;
uniform vec3 probe_spacing;
uniform vec3 probe_dimensions;
uniform float probe_resolution;
uniform int probe_tiles_x;
uniform float probe_gi_multiplier;
uniform vec3 probe_sunlight_scale;
// XYZ is a world-space travel direction; W enables the primary-light override.
uniform vec4 probe_sunlight_direction;
uniform int probe_terrain_albedo;
uniform float probe_normal_bias;
uniform float probe_view_bias;
uniform vec3 probe_dense_origin[2];
uniform vec3 probe_dense_spacing[2];
uniform vec3 probe_dense_dimensions[2];
uniform int probe_dense_offset[2];
uniform float probe_dense_weight[2];
uniform int probe_gpu_selection;
uniform int probe_region_count;
uniform vec2 probe_tile_extent;
uniform usamplerBuffer probe_region_masks;

layout(std140) uniform ProbeRegions {
    vec4 probe_regions[1536];
};

uniform int probe_mode;      // 0 original, 1 combined, 2 indirect, 3 direct, 4 coverage
uniform int probe_receiver;  // 0 unchanged, 1 terrain, 2 moby
uniform int probe_effect;    // additive effects stay unlit
uniform int probe_backlight;
uniform sampler2D hero_shadow_map;
uniform mat4 hero_shadow_matrix;
uniform int hero_shadow_enabled;
uniform int hero_shadow_airborne;
// Transition progress fades the platforming cue and delays rig self-shadowing until grounded.
uniform float hero_shadow_air_blend;
uniform int hero_shadow_receiver;
uniform sampler2D sun_shadow_map0;
uniform sampler2D sun_shadow_map1;
uniform sampler2D sun_shadow_map2;
uniform sampler2D sun_shadow_map3;
uniform mat4 sun_shadow_matrix[4];
uniform mat4 sun_shadow_view;
uniform vec4 sun_shadow_ends;
uniform float sun_shadow_near;
uniform int sun_shadow_enabled;
uniform int shadow_atlas_enabled;
uniform vec4 shadow_atlas_rects[5];
uniform vec4 light_sets[64];
uniform vec4 probe_point_position[8];
uniform vec4 probe_point_colour[8];

vec2 probe_oct(vec3 n) {
    n /= abs(n.x) + abs(n.y) + abs(n.z);
    vec2 p = n.xy;
    if (n.z < 0.0) {
        p = (1.0 - abs(p.yx)) * mix(vec2(-1.0), vec2(1.0), greaterThanEqual(p, vec2(0.0)));
    }
    return p * 0.5 + 0.5;
}

vec2 probe_map_uv(ivec2 tile, vec3 direction) {
    vec2 pixel =
        vec2(tile) * (probe_resolution + 2.0) + 1.0 + probe_oct(direction) * probe_resolution;
    return pixel / vec2(textureSize(probe_irradiance, 0));
}

bool sample_probe_grid(
    vec3 p,
    vec3 n,
    vec3 surface_bias,
    vec3 origin,
    vec3 spacing,
    vec3 dimensions,
    int index_offset,
    bool preserve_occlusion,
    out vec3 irradiance,
    out float confidence
) {
    irradiance = vec3(0.0);
    confidence = 0.0;
    vec3 coordinate = (p - origin) / spacing;
    if (any(lessThan(coordinate, vec3(0.0))) || any(greaterThan(coordinate, dimensions - 1.0))) {
        return false;
    }
    vec3 receiver = p + surface_bias;
    // Select and interpolate the same biased receiver used for visibility.
    // Keep volume membership and the behind-surface guard at the real surface.
    coordinate = clamp((receiver - origin) / spacing, vec3(0.0), dimensions - 1.0);
    ivec3 cell = min(ivec3(floor(coordinate)), ivec3(dimensions) - 2);
    vec3 fraction = coordinate - vec3(cell);
    float total = 0.0;
    float support = 0.0;
    float valid_support = 0.0;
    float facing_support = 0.0;
    for (int corner = 0; corner < 8; ++corner) {
        ivec3 high = ivec3(corner & 1, (corner >> 1) & 1, (corner >> 2) & 1);
        vec3 f = mix(1.0 - fraction, fraction, vec3(high));
        float weight = f.x * f.y * f.z;
        ivec3 at = cell + high;
        int index = index_offset + at.x + int(dimensions.x) * (at.y + int(dimensions.y) * at.z);
        ivec2 tile = ivec2(index % probe_tiles_x, index / probe_tiles_x);
#ifndef OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE
        if (weight <= 0.0) {
            continue;
        }
#endif
        vec4 metadata = texelFetch(probe_metadata, tile, 0);
#ifdef OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE
        if (weight <= 0.0) {
            continue;
        }
#endif
        vec3 position = origin + vec3(at) * spacing + metadata.xyz;
        vec3 delta = receiver - position;
        float distance = length(delta);
        if (distance > 1e-6) {
            vec3 direction = delta / distance;
            float wrap = max(0.05, (1.0 - dot(direction, n)) * 0.5);
            weight *= wrap * wrap;
        }
        support += weight;
        if (metadata.w < 0.5) {
            continue;
        }
        valid_support += weight;
        // Reject probes behind the surface even when angularly filtered
        // depth moments miss a thin floor. Coplanar probes retain support.
        weight *= smoothstep(-0.1, 0.0, dot(position - p, n));
        facing_support += weight;
#ifndef OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE
        // Coverage counters above must include valid but blocked probes.
        // Their zero weight cannot contribute light, so skip both atlas reads.
        if (weight <= 0.0) {
            continue;
        }
#endif
        if (distance > 1e-6) {
            vec2 moments = texture(probe_moments, probe_map_uv(tile, delta / distance)).xy;
            if (distance > moments.x) {
                float variance = max(0.0, moments.y - moments.x * moments.x);
                float difference = distance - moments.x;
                float denominator = variance + difference * difference;
                float visibility = denominator > 0.0 ? variance / denominator : 0.0;
                weight *= visibility * visibility * visibility;
            }
        }
#ifndef OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE
        if (weight <= 0.0) {
            continue;
        }
#endif
        // Partially decode to sqrt(E) for interpolation between probes.
        vec3 encoded = texture(probe_irradiance, probe_map_uv(tile, n)).rgb;
        irradiance += pow(max(encoded, vec3(0.0)), vec3(2.5)) * weight;
        total += weight;
    }
    if (valid_support <= 0.0) {
        return false;
    }
    confidence = clamp(valid_support / support, 0.0, 1.0);
    // Occluded probes must not pull in brighter fallback lighting. Only
    // missing probes affect cross-volume coverage blending.
    if (total <= 1e-8) {
        irradiance = vec3(0.0);
        return preserve_occlusion;
    }
    irradiance /= total;
    irradiance *= irradiance;  // Return to linear before visibility and volume fades.
    if (preserve_occlusion) {
        irradiance *= smoothstep(0.0, 0.1, total / facing_support);
    }
    return true;
}

uint probe_candidate_word(ivec2 pixel, int word) {
    if (probe_gpu_selection == 2 || any(lessThan(pixel, ivec2(0)))
        || any(greaterThanEqual(pixel, ivec2(probe_tile_extent)))) {
        return ~0u;
    }
    ivec2 tile = pixel / 16;
    int tiles_x = (int(probe_tile_extent.x) + 15) / 16;
    return texelFetch(
               probe_region_masks,
               129 + (tile.y * tiles_x + tile.x) * 4 + word
    )
        .r;
}

ivec2 probe_choose_regions(vec3 p) {
    ivec2 best = ivec2(-1);
    vec2 scores = vec2(0);
    int count = int(texelFetch(probe_region_masks, 0).r);
    for (int word = 0; word < (count + 31) / 32; ++word) {
#ifdef OPENRAC_PROBE_PIXEL
        uint bits = probe_candidate_word(OPENRAC_PROBE_PIXEL, word);
#else
        uint bits = probe_candidate_word(ivec2(gl_FragCoord.xy), word);
#endif
        while (bits != 0u) {
            int slot = word * 32 + findLSB(bits);
            bits &= bits - 1u;
            if (slot >= count) {
                continue;
            }
            int id = int(texelFetch(probe_region_masks, 1 + slot).r);
            vec3 step = probe_regions[id * 3 + 1].xyz;
            vec3 c = (p - probe_regions[id * 3].xyz) / step;
            vec3 edge = min(c, probe_regions[id * 3 + 2].xyz - 1.0 - c);
            float influence = smoothstep(0.0, 1.0, min(edge.x, min(edge.y, edge.z)));
            float score = influence / (step.x * step.y * step.z);
            if (score > scores.x || (score > 0.0 && score == scores.x && id < best.x)) {
                best.y = best.x;
                scores.y = scores.x;
                best.x = id;
                scores.x = score;
            } else if (score > scores.y || (score > 0.0 && score == scores.y && id < best.y)) {
                best.y = id;
                scores.y = score;
            }
        }
    }
    return best;
}

bool sample_probe_irradiance(vec3 p, vec3 n, vec3 surface_bias, out vec3 irradiance) {
    vec3 locals = vec3(0.0);
    float total = 0.0;
    float confidence = 0.0;
    ivec2 chosen = ivec2(-1);
    if (probe_gpu_selection != 0) {
        chosen = probe_choose_regions(p);
    }
    for (int slot = 0; slot < 2; ++slot) {
        int id = chosen[slot];
        if (probe_gpu_selection != 0 && id < 0) {
            continue;
        }
        vec3 origin = probe_dense_origin[slot];
        float weight = probe_dense_weight[slot];
        vec3 spacing = probe_dense_spacing[slot];
        vec3 dimensions = probe_dense_dimensions[slot];
        int offset = probe_dense_offset[slot];
        if (probe_gpu_selection != 0) {
            origin = probe_regions[id * 3].xyz;
            weight = 1.0;
            spacing = probe_regions[id * 3 + 1].xyz;
            dimensions = probe_regions[id * 3 + 2].xyz;
            offset = int(probe_regions[id * 3].w);
        }
        if (weight <= 0.0) {
            continue;
        }
        vec3 local;
        bool local_valid = sample_probe_grid(
            p, n, surface_bias, origin, spacing, dimensions, offset, true, local, confidence
        );
        if (local_valid) {
            vec3 coordinate = (p - origin) / spacing;
            vec3 edge = min(coordinate, dimensions - 1.0 - coordinate);
            float blend = smoothstep(0.0, 1.0, min(edge.x, min(edge.y, edge.z)))
                          * smoothstep(0.0, 1.0, confidence) * weight;
            locals += local * blend;
            total += blend;
        }
    }
    if (total >= 1.0) {
        irradiance = locals / total;
        return true;
    }
    bool base_valid = sample_probe_grid(
        p,
        n,
        surface_bias,
        probe_origin,
        probe_spacing,
        probe_dimensions,
        0,
        true,
        irradiance,
        confidence
    );
    if (total > 0.0) {
        if (!base_valid) {
            irradiance = locals / total;
            return true;
        }
        irradiance = mix(irradiance, locals / total, min(total, 1.0));
    }
    return base_valid;
}

// Reference/fixture entry point retains the original normal-only bias.
bool sample_probe_irradiance(vec3 p, vec3 n, out vec3 irradiance) {
    return sample_probe_irradiance(p, n, n * 0.02, irradiance);
}

vec3 probe_surface_bias(vec3 normal, vec3 toward_camera) {
    float squared = dot(toward_camera, toward_camera);
    vec3 view_direction = squared > 1e-12 ? toward_camera * inversesqrt(squared) : vec3(0.0);
    return probe_normal_bias * normal + probe_view_bias * view_direction;
}

vec3 probe_unit(vec3 v) {
    float squared = dot(v, v);
    return squared > 1e-12 ? v * inversesqrt(squared) : vec3(0.0);
}

// Interpolate comparison results, never raw depths: a nearest-depth PCF
// kernel otherwise jumps a whole texel when the receiver moves slightly.
float shadow_bilinear_rect(sampler2D map, ivec4 rect, vec3 position, float bias, vec2 slope) {
    ivec2 size = rect.zw;
    vec2 pixel = position.xy * vec2(size) - 0.5;
    ivec2 base = ivec2(floor(pixel));
    vec2 f = fract(pixel);
    float result = 0.0;
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            ivec2 at = base + ivec2(x, y);
            vec2 weight = mix(1.0 - f, f, vec2(x, y));
            bool outside = any(lessThan(at, ivec2(0))) || any(greaterThanEqual(at, size));
            float receiver = position.z + dot(slope, (vec2(at) + 0.5) / vec2(size) - position.xy);
            float visibility = 1.0;
            if (!outside) {
                visibility = receiver + bias >= texelFetch(map, rect.xy + at, 0).r ? 1.0 : 0.0;
            }
            result += visibility * weight.x * weight.y;
        }
    }
    return result;
}

// Retained comparison path: nine bilinear taps, 36 depth reads.
float shadow_pcf_reference_rect(sampler2D map, ivec4 rect, vec3 position, float bias, vec2 slope, bool tent) {
    vec2 texel = 1.0 / vec2(rect.zw);
    float result = 0.0, total = 0.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            vec2 offset = vec2(x, y) * texel;
            float weight = tent ? float((2 - abs(x)) * (2 - abs(y))) : 1.0;
            result +=
                weight
                * shadow_bilinear_rect(map, rect, position + vec3(offset, dot(slope, offset)), bias, slope);
            total += weight;
        }
    }
    return result / total;
}

// Convolve the three kernel weights with the bilinear weights on each axis.
// The 4x4 path reads each unique depth once, including at borders.
float shadow_pcf_unique_rect(sampler2D map, ivec4 rect, vec3 position, float bias, vec2 slope, bool tent) {
    ivec2 size = rect.zw;
    vec2 pixel = position.xy * vec2(size) - 0.5;
    ivec2 base = ivec2(floor(pixel)) - 1;
    vec2 f = fract(pixel);
    vec4 wx = vec4(1.0 - f.x, 1.0, 1.0, f.x);
    vec4 wy = vec4(1.0 - f.y, 1.0, 1.0, f.y);
    if (tent) {
        wx.y += 1.0 - f.x;
        wx.z += f.x;
        wy.y += 1.0 - f.y;
        wy.z += f.y;
    }
#ifdef OPENRAC_SHADOW_GATHER
    // Four 2x2 gathers cover exactly the same 4x4 texels. GLSL gather
    // component order is (low x/high y, high/high, high/low, low/low).
    vec2 inverse_size = 1.0 / vec2(textureSize(map, 0));
    vec4 depths[4];
    depths[0] = textureGather(map, (vec2(rect.xy + base) + vec2(1.0, 1.0)) * inverse_size, 0);
    depths[1] = textureGather(map, (vec2(rect.xy + base) + vec2(3.0, 1.0)) * inverse_size, 0);
    depths[2] = textureGather(map, (vec2(rect.xy + base) + vec2(1.0, 3.0)) * inverse_size, 0);
    depths[3] = textureGather(map, (vec2(rect.xy + base) + vec2(3.0, 3.0)) * inverse_size, 0);
#endif
    float result = 0.0;
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            ivec2 at = base + ivec2(x, y);
            float visibility = 1.0;
            if (all(greaterThanEqual(at, ivec2(0))) && all(lessThan(at, size))) {
                vec2 centre = (vec2(at) + 0.5) / vec2(size);
                float receiver = position.z + dot(slope, centre - position.xy);
#ifdef OPENRAC_SHADOW_GATHER
                int component = (y & 1) != 0 ? (x & 1) : 3 - (x & 1);
                float depth = depths[(y >> 1) * 2 + (x >> 1)][component];
#else
                float depth = texelFetch(map, rect.xy + at, 0).r;
#endif
                visibility = receiver + bias >= depth ? 1.0 : 0.0;
            }
            result += visibility * wx[x] * wy[y];
        }
    }
    return result / (tent ? 16.0 : 9.0);
}

float shadow_pcf_kernel_rect(sampler2D map, ivec4 rect, vec3 position, float bias, vec2 slope, bool tent) {
#ifdef OPENRAC_SHADOW_FILTER_REFERENCE
    return shadow_pcf_reference_rect(map, rect, position, bias, slope, tent);
#else
    return shadow_pcf_unique_rect(map, rect, position, bias, slope, tent);
#endif
}

ivec4 shadow_map_rect(sampler2D map, int view) {
    return shadow_atlas_enabled != 0 ? ivec4(shadow_atlas_rects[view])
                                    : ivec4(0, 0, textureSize(map, 0));
}
float shadow_bilinear(sampler2D map, vec3 p, float bias, vec2 slope) {
    return shadow_bilinear_rect(map, ivec4(0, 0, textureSize(map, 0)), p, bias, slope);
}
float shadow_pcf_reference(sampler2D map, vec3 p, float bias, vec2 slope, bool tent) {
    return shadow_pcf_reference_rect(map, ivec4(0, 0, textureSize(map, 0)), p, bias, slope, tent);
}
float shadow_pcf_unique(sampler2D map, vec3 p, float bias, vec2 slope, bool tent) {
    return shadow_pcf_unique_rect(map, ivec4(0, 0, textureSize(map, 0)), p, bias, slope, tent);
}
float shadow_pcf_kernel(sampler2D map, vec3 p, float bias, vec2 slope, bool tent) {
    return shadow_pcf_kernel_rect(map, ivec4(0, 0, textureSize(map, 0)), p, bias, slope, tent);
}

bool hero_shadow_applies(vec3 n) {
    return hero_shadow_enabled != 0
           && ((hero_shadow_airborne == 0 && hero_shadow_air_blend <= 0.0)
               || (hero_shadow_receiver == 0 && n.z >= 0.1));
}

// Resolve visibility independently of the receiving object. Early lighting
// stores this once; the surface shader still excludes Ratchet while airborne.
float hero_shadow_sample(vec3 p, vec3 n) {
    if (hero_shadow_enabled == 0) {
        return 1.0;
    }
    vec4 clip = hero_shadow_matrix * vec4(p + n * 0.01, 1.0);
    vec3 position = clip.xyz / clip.w * 0.5 + 0.5;
    if (any(lessThan(position, vec3(0.0))) || any(greaterThan(position, vec3(1.0)))) {
        return 1.0;
    }
    return shadow_pcf_kernel_rect(hero_shadow_map, shadow_map_rect(hero_shadow_map, 4), position, 0.00025, vec2(0.0), false);
}

float hero_shadow_visibility(vec3 p, vec3 n) {
    return hero_shadow_applies(n) ? hero_shadow_sample(p, n) : 1.0;
}

// The sun cascades' normal offset and depth bias, in texels of the cascade sampled.
const float kSunNormalOffsetTexels = 1.0;
const float kSunDepthBiasTexels = 0.5;

float sun_shadow_pcf_rect(sampler2D map, ivec4 rect, mat4 matrix, vec3 p, vec3 n, vec3 plane_normal) {
    // The orthographic rows are scaled orthogonal light-space axes.
    vec3 row_x = vec3(matrix[0].x, matrix[1].x, matrix[2].x);
    vec3 row_y = vec3(matrix[0].y, matrix[1].y, matrix[2].y);
    vec3 row_z = vec3(matrix[0].z, matrix[1].z, matrix[2].z);
    // One of this cascade's texels in world units: a far cascade's are thirty times the nearest's.
    // The normal offset and depth bias follow it, so the far cascades do not shadow their own
    // receivers in speckles and stair steps; the nearest keeps its fixed 0.025 and 0.01.
    float texel = 2.0 / (length(row_x) * float(rect.z));
    vec3 position =
        (matrix * vec4(p + n * max(0.025, kSunNormalOffsetTexels * texel), 1.0)).xyz * 0.5 + 0.5;
    if (any(lessThan(position, vec3(0.0))) || any(greaterThan(position, vec3(1.0)))) {
        return 1.0;
    }
    // Receiver-plane depth follows the geometric face across PCF taps.
    vec3 plane = vec3(
        dot(row_x, plane_normal) / dot(row_x, row_x),
        dot(row_y, plane_normal) / dot(row_y, row_y),
        dot(row_z, plane_normal) / dot(row_z, row_z)
    );
    vec2 slope =
        abs(plane.z) > 1e-5 ? clamp(-plane.xy / plane.z, vec2(-2.0), vec2(2.0)) : vec2(0.0);
    // A world-space bias (half a texel of the cascade, at least 0.01) avoids detachment growing
    // with whole-level depth.
    float bias = length(row_z) * 0.5 * max(0.01, kSunDepthBiasTexels * texel);
    return shadow_pcf_kernel_rect(map, rect, position, bias, slope, true);
}

float sun_shadow_pcf(sampler2D map, mat4 matrix, vec3 p, vec3 n, vec3 normal) {
    return sun_shadow_pcf_rect(map, ivec4(0, 0, textureSize(map, 0)), matrix, p, n, normal);
}

float sun_cascade_visibility(int cascade, vec3 p, vec3 n, vec3 plane_normal) {
    if (cascade == 0) {
        return sun_shadow_pcf_rect(sun_shadow_map0, shadow_map_rect(sun_shadow_map0, 0), sun_shadow_matrix[0], p, n, plane_normal);
    }
    if (cascade == 1) {
        return sun_shadow_pcf_rect(sun_shadow_map1, shadow_map_rect(sun_shadow_map1, 1), sun_shadow_matrix[1], p, n, plane_normal);
    }
    if (cascade == 2) {
        return sun_shadow_pcf_rect(sun_shadow_map2, shadow_map_rect(sun_shadow_map2, 2), sun_shadow_matrix[2], p, n, plane_normal);
    }
    return sun_shadow_pcf_rect(sun_shadow_map3, shadow_map_rect(sun_shadow_map3, 3), sun_shadow_matrix[3], p, n, plane_normal);
}

float sun_shadow_visibility(vec3 p, vec3 n, vec3 plane_normal) {
    if (sun_shadow_enabled == 0) {
        return 1.0;
    }
    float distance = -(sun_shadow_view * vec4(p, 1.0)).z;
    if (distance < sun_shadow_near || distance >= sun_shadow_ends.w) {
        return 1.0;
    }
    int cascade = distance < sun_shadow_ends.x
                      ? 0
                      : (distance < sun_shadow_ends.y ? 1 : (distance < sun_shadow_ends.z ? 2 : 3));
    float visibility = sun_cascade_visibility(cascade, p, n, plane_normal);
    float start = cascade == 0 ? sun_shadow_near : sun_shadow_ends[cascade - 1];
    float blend =
        smoothstep(mix(start, sun_shadow_ends[cascade], 0.9), sun_shadow_ends[cascade], distance);
    float next =
        blend > 0.0 && cascade < 3 ? sun_cascade_visibility(cascade + 1, p, n, plane_normal) : 1.0;
    return mix(visibility, next, blend);
}

vec3 probe_direct(vec3 p, vec3 n, vec4 selector, float visibility) {
    int a = (int(selector.x + 0.5) & 15) * 4;
    int b = (int(selector.y + 0.5) & 15) * 4;
    float blend = clamp(selector.z, 0.0, 1.0);
    vec3 direct = vec3(0.0);
    for (int light = 0; light < 2; ++light) {
        int offset = light * 2;
        vec4 colour = mix(light_sets[a + offset], light_sets[b + offset], blend);
        vec3 direction =
            probe_unit(mix(light_sets[a + offset + 1].xyz, light_sets[b + offset + 1].xyz, blend));
        // All authored primary sets share the override; secondary fill keeps its direction.
        if (light == 0 && probe_sunlight_direction.w != 0.0) {
            direction = probe_sunlight_direction.xyz;
        }
        float d = dot(-direction, n);
        float response = probe_backlight != 0 ? max(d, -abs(colour.w) * d) : max(d, 0.0);
        // Both the primary sun and secondary directional fill retain 10%.
        vec3 correction = light == 0 ? probe_sunlight_scale : vec3(1.0);
        direct += max(colour.rgb, vec3(0.0)) * correction * response * mix(0.1, 1.0, visibility);
    }
    for (int i = 0; i < 8; ++i) {
        vec3 delta = probe_point_position[i].xyz - p;
        float radius = probe_point_position[i].w;
        float distance = length(delta);
        if (radius <= 0.0 || distance >= radius || distance <= 1e-6) {
            continue;
        }
        float d = dot(delta / distance, n);
        float response =
            probe_backlight != 0 ? max(d, -abs(probe_point_colour[i].w) * d) : max(d, 0.0);
        direct += max(probe_point_colour[i].rgb, vec3(0.0)) * response * (1.0 - distance / radius);
    }
    return direct;
}

vec3 probe_linear(vec3 c) {
    c = max(c, vec3(0.0));
    return mix(pow((c + 0.055) / 1.055, vec3(2.4)), c / 12.92, lessThanEqual(c, vec3(0.04045)));
}

vec3 probe_display(vec3 c) {
    c = max(c, vec3(0.0));
    return clamp(
        mix(1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, c * 12.92, lessThanEqual(c, vec3(0.0031308))),
        0.0,
        1.0
    );
}
