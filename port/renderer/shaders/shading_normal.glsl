// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Keeps two-sided smooth normals on the visible face without grazing-angle sign changes.
vec3 orient_shading_normal(vec3 normal, vec3 geometric, vec3 toward_eye) {
    vec3 eye = normalize(toward_eye);
    geometric = normalize(geometric);
    // Winding and authored normal signs may both be reversed.
    geometric *= dot(geometric, eye) < 0.0 ? -1.0 : 1.0;
    normal = normalize(normal);
    normal *= dot(normal, geometric) < 0.0 ? -1.0 : 1.0;
    // A smooth normal can lean behind the eye on a visible triangle. Clip that lean instead
    // of flipping it through the surface. The margin covers RGBA8 rounding at partial MSAA coverage.
    const float minimum_facing = 0.08;
    return normalize(normal + eye * max(0.0, minimum_facing - dot(normal, eye)));
}
