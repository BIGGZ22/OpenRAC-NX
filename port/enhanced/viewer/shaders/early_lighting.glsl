// SPDX-License-Identifier: GPL-3.0-or-later
uniform int early_lighting_enabled;
uniform samplerBuffer early_values;
uniform usamplerBuffer early_guides;
uniform mat4 early_projection;
uniform vec2 early_extent;

vec3 early_decode_normal(uint bits) {
    vec2 xy = vec2(bits & 255u, (bits >> 8u) & 255u) / 255.0 * 2.0 - 1.0;
    vec3 n = vec3(xy, 1.0 - abs(xy.x) - abs(xy.y));
    float t = clamp(-n.z, 0.0, 1.0);
    n.xy += mix(vec2(t), vec2(-t), greaterThanEqual(n.xy, vec2(0)));
    return normalize(n);
}

// A single reconstruction path for every opaque receiver. Relative plane and
// normal agreement chooses the represented surface even when depth quantization
// shifts every candidate slightly; no pixel evaluates probes or cascades again.
bool early_surface(
    vec3 eye_position,
    vec3 normal_view,
    out vec4 result,
    out bool covered,
    out float hero_visibility
) {
    result = vec4(0, 0, 0, 1);
    covered = false;
    hero_visibility = 1.0;
    if (early_lighting_enabled == 0 || translucent_pass != 0 || effect_pass != 0) {
        return false;
    }
    ivec2 half_extent = (ivec2(early_extent) + 1) / 2;
    vec2 coordinate = gl_FragCoord.xy * 0.5 - 0.5;
    ivec2 begin = ivec2(floor(coordinate));
    vec2 fraction = fract(coordinate);
    vec4 samples[4];
    float scores[4], spatial[4], hero[4];
    int coverage[4];
    float best = 1e30;
    int best_coverage = 0;
    float footprint =
        max(-eye_position.z, 0.0) * 2.0 / (abs(early_projection[1][1]) * early_extent.y);
    // The GL-compatible vertex projection and D24 depth both contribute
    // reconstruction error. Compare candidates relatively, not to a hard cutoff.
    float tolerance = 0.002 + max(-eye_position.z, 0.0) * 0.0002 + footprint * 0.1;
    for (int i = 0; i < 4; ++i) {
        int x = i & 1, y = i >> 1;
        ivec2 cell = clamp(begin + ivec2(x, y), ivec2(0), half_extent - 1);
        int index = cell.y * half_extent.x + cell.x;
        uint bits = texelFetch(early_guides, index * 2 + 1).r;
        scores[i] = 1e30;
        spatial[i] = 0.0;
        coverage[i] = 0;
        samples[i] = vec4(0);
        hero[i] = 1.0;
        if ((bits & (1u << 19u)) == 0u) {
            continue;
        }
        vec3 n = early_decode_normal(bits);
        uint selected = (bits >> 16u) & 3u;
        ivec2 pixel = cell * 2 + ivec2(selected & 1u, selected >> 1u);
        float depth = uintBitsToFloat(texelFetch(early_guides, index * 2).r);
        float distance = early_projection[3][2] / (depth * 2.0 + (early_projection[2][2] - 1.0));
        vec2 ndc = (vec2(pixel) + 0.5) / early_extent * 2.0 - 1.0;
        vec3 p = vec3(
                     (ndc + early_projection[2].xy)
                         / vec2(early_projection[0][0], early_projection[1][1]),
                     -1.0
                 )
                 * distance;
        vec3 delta = p - eye_position;
        float plane_error =
            max(abs(dot(delta, normal_view)), abs(dot(delta, n)) - length(delta) * 0.02);
        scores[i] = plane_error / tolerance + 8.0 * (1.0 - clamp(dot(n, normal_view), -1.0, 1.0));
        spatial[i] =
            max(0.001,
                (x == 0 ? 1.0 - fraction.x : fraction.x) * (y == 0 ? 1.0 - fraction.y : fraction.y)
            );
        coverage[i] = (bits & (1u << 18u)) != 0u ? 1 : 0;
        samples[i] = texelFetch(early_values, index);
        hero[i] = float((bits >> 20u) & 255u) / 255.0;
        samples[i].rgb = (exp2(samples[i].rgb * 20.0) - 1.0) / 16.0;
        if (scores[i] < best) {
            best = scores[i];
            best_coverage = coverage[i];
        }
    }
    result = vec4(0);
    hero_visibility = 0.0;
    float total = 0.0;
    for (int i = 0; i < 4; ++i) {
        float difference = min(32.0, max(0.0, scores[i] - best) * 8.0);
        float squared = difference * difference;
        float weight = coverage[i] == best_coverage ? spatial[i] / (1.0 + squared * squared) : 0.0;
        result += samples[i] * weight;
        hero_visibility += hero[i] * weight;
        total += weight;
    }
    result = total > 0.0 ? result / total : vec4(0, 0, 0, 1);
    hero_visibility = total > 0.0 ? hero_visibility / total : 1.0;
    covered = best_coverage != 0;
    return true;
}
