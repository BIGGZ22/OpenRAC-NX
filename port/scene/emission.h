// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The level's emitted light: the mask that keeps a texture's glowing parts, and the reviewed
// materials that emit (kAuthoredEmissions), part of the scene's level data. load_level applies the
// table to a level's materials (level.cpp); the probe baker reads the same materials, so the
// runtime and the bake agree (enhanced/lighting/bake.h). Restructure package P2-39b moved it from
// enhanced/lighting/, where scene/level.h included it against the layering.

#pragma once

#include <algorithm>
#include <array>
#include <string_view>

namespace openrac::scene {
// Applied to the filtered linear emission texture, in both bake.cpp and mesh.frag.
enum class EmissionMask : unsigned {
    None,
    Green
};

inline float emission_mask(EmissionMask mask, const std::array<float, 3>& rgb) {
    if (mask == EmissionMask::None) {
        return 1;
    }
    const float chroma = (rgb[1] - std::max(rgb[0], rgb[2])) / std::max(rgb[1], 1e-5f);
    const float t = std::clamp((chroma - .25f) / .25f, 0.0f, 1.0f);
    return t * t * (3 - 2 * t);
}

struct AuthoredEmission {
    unsigned level;
    std::string_view texture;
    float intensity;
    EmissionMask mask;
    bool vertex_colour = false;  // GS colour modulation before sRGB decoding
    float diffuse_scale = 1;
};

// Reviewed RAC1 NTSC materials; never infer emission from ordinary baked brightness.
// The same material definition feeds the runtime renderer and scene exporter.
inline constexpr AuthoredEmission kAuthoredEmissions[] = {
    {3, "textures/moby_0178.png", 4, EmissionMask::None},
    {9, "textures/probe_gaspar_lava.png", 1, EmissionMask::None, true, 0},
    {10, "textures/moby_0140.png", 1, EmissionMask::Green},
    {10, "textures/terrain_0012.png", 1, EmissionMask::None, true, 0},
    {10, "textures/tie_0048.png", 1, EmissionMask::Green},
    {10, "textures/terrain_0059.png", 1, EmissionMask::Green},
};
}  // namespace openrac::scene
