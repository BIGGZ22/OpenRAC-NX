// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Wrapped texture, sky and authored emission sampling for offline transport.
#include "enhanced/lighting/bake_material.h"

#include "enhanced/lighting/bake_math.h"

namespace openrac::lighting {
namespace {
// Keep colour and cutout queries on identical bilinear footprints and accumulation order.
struct TextureTap {
    std::size_t offset;  // RGBA byte offset of the wrapped texel
    float weight;        // bilinear contribution, accumulated in row order
};

// Repeat wrapping matches the exported materials, including UVs outside the unit square.
std::array<TextureTap, 4> texture_taps(
    const BakeTexture& texture, const BakeTriangle& triangle, float u, float v
) {
    const auto& uv = triangle.uv;
    const float s = uv[0][0] * (1 - u - v) + uv[1][0] * u + uv[2][0] * v;
    const float t = uv[0][1] * (1 - u - v) + uv[1][1] * u + uv[2][1] * v;
    const float x = (s - std::floor(s)) * static_cast<float>(texture.width) - 0.5f;
    const float y = (t - std::floor(t)) * static_cast<float>(texture.height) - 0.5f;
    const auto ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y));
    const float tx = x - std::floor(x), ty = y - std::floor(y);
    std::array<TextureTap, 4> taps{};
    std::size_t tap_index = 0;

    // Accumulate bilinear rows in the same order for colour and alpha.
    for (int j = 0; j < 2; ++j) {
        // Repeat wrapping applies independently to the two horizontal neighbours.
        for (int i = 0; i < 2; ++i) {
            const auto px = static_cast<std::uint32_t>(
                (ix + i + static_cast<int>(texture.width)) % static_cast<int>(texture.width)
            );
            const auto py = static_cast<std::uint32_t>(
                (iy + j + static_cast<int>(texture.height)) % static_cast<int>(texture.height)
            );
            taps[tap_index++] =
                {(std::size_t{py} * texture.width + px) * 4, (i ? tx : 1 - tx) * (j ? ty : 1 - ty)};
        }
    }

    return taps;
}

// Decodes an sRGB component after the material-specific tint stage.
float linear_colour(float value) {
    return value <= .04045f ? value / 12.92f : std::pow((value + .055f) / 1.055f, 2.4f);
}

// Filters colour and linear coverage without changing the traversal footprint.
std::array<float, 4> filtered_material(
    const BakeTexture& texture,
    const BakeTriangle& triangle,
    Float2 uv,
    const BakeMaterialOptions& options,
    bool authored
) {
    const auto [u, v] = uv;
    std::array<float, 4> filtered{};

    // Colour uses the same footprint as traversal coverage.
    for (const auto& tap : texture_taps(texture, triangle, u, v)) {
        // Alpha remains linear while RGB follows the material colour path.
        for (unsigned c = 0; c < 3; ++c) {
            float f = static_cast<float>(texture.rgba[tap.offset + c]) / 255;
            f = std::clamp(f * options.texture_tint[c], 0.0f, 1.0f);

            // Ordinary materials decode each tap before interpolation.
            if (!authored) {
                f = linear_colour(f);
            }

            filtered[c] += f * tap.weight;
        }

        const float alpha = static_cast<float>(texture.rgba[tap.offset + 3]) / 255;
        filtered[3] += alpha * tap.weight;
    }

    return filtered;
}

// Applies encoded authored tint to untextured reflectance before its final decode.
void tint_untextured(
    std::array<float, 4>& value,
    Float3 authored_tint,
    const BakeMaterialOptions& options,
    bool authored
) {
    // Apply encoded tint before linear decoding on authored terrain.
    for (unsigned c = 0; c < 3; ++c) {
        const float f = std::clamp(options.texture_tint[c], 0.0f, 1.0f);
        value[c] = authored ? linear_colour(std::clamp(value[c] * authored_tint[c] * f, 0.f, 1.f))
                            : value[c] * linear_colour(f);
    }
}

// Limits diffuse reflectance while keeping coverage independent of the reflectance scale.
void scale_material(std::array<float, 4>& value, float diffuse_scale) {
    // Diffuse reflectance stays within unity after material scaling.
    for (unsigned c = 0; c < 3; ++c) {
        value[c] = std::clamp(value[c] * diffuse_scale, 0.f, 1.f);
    }

    value[3] = std::clamp(value[3], 0.f, 1.f);
}

}  // namespace

namespace bake_detail {
// Intersection acceptance needs linear alpha only, never decoded RGB or emission.
float textured_opacity(const BakeScene& scene, const BakeTriangle& triangle, float u, float v) {
    const auto& texture = scene.textures.at(static_cast<std::size_t>(triangle.texture));
    float filtered = 0;

    // Coverage stays linear and skips colour decoding inside traversal.
    for (const auto& tap : texture_taps(texture, triangle, u, v)) {
        const float alpha = static_cast<float>(texture.rgba[tap.offset + 3]) / 255;
        filtered += alpha * tap.weight;
    }

    const float modulation = triangle.multipliers[0][3] * (1 - u - v)
                             + triangle.multipliers[1][3] * u + triangle.multipliers[2][3] * v;
    return std::clamp(modulation * filtered, 0.f, 1.f);
}

}  // namespace bake_detail

Float3 sample_bake_environment(
    const BakeScene& scene, Float3 direction, const BakeSettings& settings
) {
    // Absent or disabled sky data uses the uniform scene environment.
    if (!settings.use_level_sky || scene.sky_radiance.empty()) {
        return scene.environment;
    }

    direction = bake_detail::unit(direction);
    float longitude = std::atan2(direction[0], direction[1]) / (2 * bake_detail::kPi);
    longitude -= std::floor(longitude);
    const float x = longitude * static_cast<float>(scene.sky_width) - 0.5f;
    const float y = std::clamp(
        std::acos(std::clamp(direction[2], -1.0f, 1.0f)) / bake_detail::kPi
                * static_cast<float>(scene.sky_height)
            - 0.5f,
        0.0f,
        static_cast<float>(scene.sky_height - 1)
    );
    const int ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y));
    const float fx = x - std::floor(x), fy = y - static_cast<float>(iy);
    Float3 result{};

    // Clamp latitude at the poles while wrapping longitude.
    for (int j = 0; j < 2; ++j) {
        // Both longitude neighbours contribute to the sky sample.
        for (int i = 0; i < 2; ++i) {
            const auto sx = static_cast<std::uint32_t>(
                (ix + i + static_cast<int>(scene.sky_width)) % static_cast<int>(scene.sky_width)
            );
            const auto sy = std::min(static_cast<std::uint32_t>(iy + j), scene.sky_height - 1);
            const float weight = (i ? fx : 1 - fx) * (j ? fy : 1 - fy) * settings.sky_intensity;
            const auto& pixel = scene.sky_radiance[std::size_t{sy} * scene.sky_width + sx];

            // Scale sky radiance before mixing it with the authored light colour.
            for (unsigned c = 0; c < 3; ++c) {
                result[c] += pixel[c] * weight;
            }
        }
    }

    // The ambient colour mix is independent of direct-sun correction.
    for (unsigned c = 0; c < 3; ++c) {
        result[c] = settings.sky_weight * result[c]
                    + (1 - settings.sky_weight) * scene.primary_directional_colour[c];
    }

    return result;
}

std::array<float, 4> bake_material(
    const BakeScene& scene,
    const BakeTriangle& triangle,
    float u,
    float v,
    const BakeMaterialOptions& options
) {
    const bool authored = options.terrain_vertex_albedo && triangle.terrain_prelighting;
    Float3 authored_tint{1, 1, 1};

    // Authored terrain tint follows the runtime albedo path.
    if (authored) {
        // Interpolate the artistic tint before colour decoding.
        for (unsigned c = 0; c < 3; ++c) {
            authored_tint[c] = triangle.authored_colours[0][c] * (1 - u - v)
                               + triangle.authored_colours[1][c] * u
                               + triangle.authored_colours[2][c] * v;
        }
    }

    std::array<float, 4> value{triangle.diffuse[0], triangle.diffuse[1], triangle.diffuse[2], 1};

    // GS multipliers affect colour and coverage before filtering.
    for (unsigned c = 0; c < 4; ++c) {
        value[c] *= triangle.multipliers[0][c] * (1 - u - v) + triangle.multipliers[1][c] * u
                    + triangle.multipliers[2][c] * v;
    }

    // Untextured surfaces still retain authored tint and GS modulation.
    if (triangle.texture < 0) {
        tint_untextured(value, authored_tint, options, authored);
        scale_material(value, triangle.diffuse_scale);
        return value;
    }

    const auto& texture = scene.textures.at(static_cast<std::size_t>(triangle.texture));
    const auto filtered = filtered_material(texture, triangle, {u, v}, options, authored);

    // Combine filtered texels with interpolated vertex modulation.
    for (unsigned c = 0; c < 4; ++c) {
        const float product = value[c] * filtered[c];
        value[c] = product;

        // Authored tint is applied before the final decode.
        if (authored && c < 3) {
            // Match the original terrain's encoded texture filtering and GS modulation.
            value[c] = linear_colour(std::clamp(product * authored_tint[c], 0.f, 1.f));
        }
    }

    scale_material(value, triangle.diffuse_scale);
    return value;
}

Float3 bake_emission(const BakeScene& scene, const BakeTriangle& triangle, float u, float v) {
    BakeTriangle mask{};
    mask.diffuse = {1, 1, 1};
    mask.texture = triangle.emission_texture;
    mask.uv = triangle.uv;
    Float3 tint{1, 1, 1};

    // Authored bright colours can modulate emission independently of albedo.
    if (triangle.emission_vertex_colour) {
        // Interpolate emission colour over the same triangle barycentrics.
        for (unsigned c = 0; c < 3; ++c) {
            tint[c] = triangle.emission_colours[0][c] * (1 - u - v)
                      + triangle.emission_colours[1][c] * u + triangle.emission_colours[2][c] * v;
        }
    }

    const auto radiance = bake_material(scene, mask, u, v, {tint});
    float coverage = 1;

    // Masks isolate luminous parts of an otherwise ordinary texture.
    if (triangle.emission_mask != EmissionMask::None) {
        const auto texture =
            triangle.emission_vertex_colour ? bake_material(scene, mask, u, v) : radiance;
        coverage = emission_mask(triangle.emission_mask, {texture[0], texture[1], texture[2]});
    }

    return {
        triangle.emission[0] * radiance[0] * coverage,
        triangle.emission[1] * radiance[1] * coverage,
        triangle.emission[2] * radiance[2] * coverage
    };
}

}  // namespace openrac::lighting
