// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Shared view-space smooth normals, written by the depth/normal prepass.
uniform sampler2D surface_normals;
uniform int surface_normals_enabled;

vec3 smooth_normal_at(ivec2 pixel, vec3 toward_eye) {
    vec3 normal = vec3(0, 0, 1);
    if (surface_normals_enabled != 0) {
        vec4 encoded = texelFetch(surface_normals, pixel, 0);
        // MSAA resolves average covered samples with zero background; remove coverage first.
        vec3 candidate = encoded.rgb / max(encoded.a, 1e-6) * 2.0 - 1.0;
        // Uncovered or invalid input uses a neutral view-space normal, never depth derivatives.
        if (encoded.a > 0.0 && dot(candidate, candidate) > 1e-6) {
            normal = normalize(candidate);
        }
    }
    return dot(normal, toward_eye) < 0.0 ? -normal : normal;
}
