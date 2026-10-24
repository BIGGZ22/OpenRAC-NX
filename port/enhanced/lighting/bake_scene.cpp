// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Scene validation, Embree ownership and cutout filtering for offline bakes.
#include "enhanced/lighting/bake_scene.h"

#include "enhanced/lighting/bake_material.h"
#include "enhanced/lighting/bake_math.h"

namespace openrac::lighting::bake_detail {
namespace {
// ---- Validation ----

// Cache alpha extremes once for each source texture.
struct TextureAlpha {
    bool opaque = true;
    bool transparent = true;
};

// Reject settings that could create invalid rays or unbounded allocation requests.
void validate_settings(const BakeSettings& settings, std::size_t triangle_count) {
    // Overrides must form a finite ray even when supplied directly by native callers.
    if (settings.sunlight_direction) {
        unit(*settings.sunlight_direction);
    }

    // Reject invalid per-channel sun correction before preparing transport.
    if (!finite(settings.sunlight_scale)
        || std::any_of(settings.sunlight_scale.begin(), settings.sunlight_scale.end(), [](float v) {
               return v < 0 || v > 4;
           })) {
        throw std::runtime_error("sunlight_scale requires three finite values between 0 and 4");
    }

    // Bound sampling and relocation settings before allocating worker data.
    if (settings.rays_per_probe < 16 || settings.bounces > 32 || !std::isfinite(settings.ray_bias)
        || !std::isfinite(settings.sky_intensity) || settings.sky_intensity < 0
        || !std::isfinite(settings.sky_weight) || settings.sky_weight < 0 || settings.sky_weight > 1
        || settings.ray_bias <= 0 || !std::isfinite(settings.distance_exponent)
        || settings.distance_exponent < 1 || settings.distance_exponent > 256
        || !std::isfinite(settings.min_inside_fraction) || settings.min_inside_fraction <= 0.5f
        || settings.min_inside_fraction > 1 || settings.relocation_iterations > 32
        || !std::isfinite(settings.relocation_max_fraction) || settings.relocation_max_fraction < 0
        || settings.relocation_max_fraction > 0.45f || triangle_count > 0xffffffffu / 3) {
        throw std::runtime_error("invalid probe bake settings");
    }
}

// Linear colours must be finite and nonnegative; reflectance also stays within unity.
void validate_colour(Float3 c, bool bounded) {
    // Reflectance is bounded; emitted radiance may exceed unity.
    if (!finite(c) || *std::min_element(c.begin(), c.end()) < 0
        || (bounded && *std::max_element(c.begin(), c.end()) > 1)) {
        throw std::runtime_error("invalid linear bake colour");
    }
}

// Rejects nonfinite interpolation coordinates before Embree or texture traversal consumes them.
void validate_triangle_coordinates(const BakeTriangle& triangle) {
    // Wrapped UV arithmetic still requires finite coordinates.
    for (auto uv : triangle.uv) {
        // A nonfinite UV cannot select a valid texel footprint.
        if (!std::isfinite(uv[0]) || !std::isfinite(uv[1])) {
            throw std::runtime_error("invalid bake UV");
        }
    }

    // Embree requires finite triangle positions.
    for (auto p : triangle.vertices) {
        // Reject invalid positions before uploading acceleration buffers.
        if (!finite(p)) {
            throw std::runtime_error("invalid bake vertex");
        }
    }
}

// Check material indices, authored tints and vertices before traversal can reference them.
void validate_triangle(
    const BakeTriangle& triangle, const BakeScene& input, const BakeSettings& settings
) {
    // Validate authored tints only when the albedo path consumes them.
    if (settings.terrain_vertex_albedo && triangle.terrain_prelighting) {
        // Every vertex contributes to interpolation, even outside the probe grid.
        for (auto tint : triangle.authored_colours) {
            validate_colour(tint, false);
        }
    }

    // Diffuse scaling cannot create negative or super-unit reflectance.
    if (!std::isfinite(triangle.diffuse_scale) || triangle.diffuse_scale < 0
        || triangle.diffuse_scale > 1) {
        throw std::runtime_error("invalid bake diffuse scale");
    }

    validate_colour(triangle.diffuse, true);
    validate_colour(triangle.emission, false);

    // Emission colours are consumed only by the authored brightness path.
    if (triangle.emission_vertex_colour) {
        // All emitting vertices must have finite, nonnegative colours.
        for (const auto& tint : triangle.emission_colours) {
            validate_colour(tint, false);
        }
    }

    // Emission indices must reference retained source textures.
    if (triangle.emission_texture < -1
        || (triangle.emission_texture >= 0
            && static_cast<std::size_t>(triangle.emission_texture) >= input.textures.size())) {
        throw std::runtime_error("invalid bake emission texture");
    }

    // GS colour multipliers may exceed unity but must remain finite.
    for (const auto& multiplier : triangle.multipliers) {
        validate_colour({multiplier[0], multiplier[1], multiplier[2]}, false);

        // Coverage interpolation requires finite, nonnegative alpha.
        if (!std::isfinite(multiplier[3]) || multiplier[3] < 0) {
            throw std::runtime_error("invalid bake vertex alpha");
        }
    }

    // Cutout references and thresholds must be valid before the callback runs.
    if (triangle.texture < -1
        || (triangle.texture >= 0
            && static_cast<std::size_t>(triangle.texture) >= input.textures.size())
        || !std::isfinite(triangle.alpha_cutoff) || triangle.alpha_cutoff < 0
        || triangle.alpha_cutoff > 1) {
        throw std::runtime_error("invalid bake material");
    }

    validate_triangle_coordinates(triangle);
}
}  // namespace

// ---- Embree acceleration ----

void Scene::check_error() const {
    // Embree errors belong to the worker that performed the query.
    if (rtcGetDeviceError(device) != RTC_ERROR_NONE) {
        throw std::runtime_error("Embree probe bake failed");
    }
}

void Scene::filter_alpha(const RTCFilterFunctionNArguments* args) {
    const auto& scene = *static_cast<const Scene*>(args->geometryUserPtr);

    // Embree may evaluate several candidates in one filter invocation.
    for (unsigned lane = 0; lane < args->N; ++lane) {
        // Inactive lanes must retain their original rejection state.
        if (args->valid[lane] == 0) {
            continue;
        }

        const auto primitive = scene.triangle_ids[1][RTCHitN_primID(args->hit, args->N, lane)];
        const auto& triangle = scene.input.triangles[primitive];

        // Reject transparent candidates without restarting traversal.
        if (textured_opacity(
                scene.input,
                triangle,
                RTCHitN_u(args->hit, args->N, lane),
                RTCHitN_v(args->hit, args->N, lane)
            )
            < triangle.alpha_cutoff) {
            // Reject this candidate without restarting traversal. Both query
            // types search past holes without relying on surface winding.
            args->valid[lane] = 0;
        }
    }
}

// Retain source indices and emitter weights while classifying traversal geometry.
void Scene::collect_triangles() {
    std::vector<TextureAlpha> texture_alpha;

    // Classify texture extrema once rather than scanning them per triangle.
    for (const auto& texture : input.textures) {
        auto& alpha = texture_alpha.emplace_back();

        // RGBA byte alpha determines guaranteed opaque and invisible cases.
        for (std::size_t i = 3; i < texture.rgba.size(); i += 4) {
            alpha.opaque = alpha.opaque && texture.rgba[i] == 255;
            alpha.transparent = alpha.transparent && texture.rgba[i] == 0;
        }
    }

    // Keep source order stable for emitter selection and material lookup.
    for (std::size_t i = 0; i < input.triangles.size(); ++i) {
        const auto& triangle = input.triangles[i];
        // The half-alpha margin keeps bilinear rounding away from the cutoff.
        const bool opaque =
            triangle.texture < 0
            || (triangle.alpha_cutoff <= .5f
                && texture_alpha.at(static_cast<std::size_t>(triangle.texture)).opaque
                && triangle.multipliers[0][3] == 1 && triangle.multipliers[1][3] == 1
                && triangle.multipliers[2][3] == 1);
        const bool transparent =
            triangle.texture >= 0 && triangle.alpha_cutoff > 0
            && (texture_alpha.at(static_cast<std::size_t>(triangle.texture)).transparent
                || std::all_of(
                    triangle.multipliers.begin(),
                    triangle.multipliers.end(),
                    [](const auto& colour) { return colour[3] == 0; }
                ));

        // Zero-cutoff and untextured faces retain their existing acceptance rules.
        if (!transparent) {
            triangle_ids[opaque ? 0 : 1].push_back(static_cast<std::uint32_t>(i));
        }

        // Preserve emitter weights and random choices even for invisible emitters.
        const float strength =
            *std::max_element(triangle.emission.begin(), triangle.emission.end());

        // Only positive emission contributes area-weighted selection entries.
        if (strength > 0) {
            const auto a = add(triangle.vertices[1], mul(triangle.vertices[0], -1));
            const auto b = add(triangle.vertices[2], mul(triangle.vertices[0], -1));
            const auto normal = cross(a, b);
            const float twice_area = std::sqrt(dot(normal, normal));

            // Degenerate triangles have no emitting area or usable normal.
            if (twice_area > 1e-12f) {
                emission_weight += double(twice_area) * .5 * strength;
                emitters.push_back(
                    {i, emission_weight, twice_area * .5f, mul(normal, 1 / twice_area)}
                );
            }
        }
    }
}

// Upload one opaque or cutout group with independent Embree ownership.
void Scene::build_geometry(unsigned group) {
    const auto& source_ids = triangle_ids[group];

    // Empty groups must not create an invalid zero-sized geometry.
    if (source_ids.empty()) {
        return;
    }

    RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

    // Release partially created geometry if buffer allocation fails.
    try {
        check_error();
        auto* vertices = static_cast<Float3*>(rtcSetNewGeometryBuffer(
            geometry,
            RTC_BUFFER_TYPE_VERTEX,
            0,
            RTC_FORMAT_FLOAT3,
            sizeof(Float3),
            source_ids.size() * 3
        ));
        using Index = std::array<std::uint32_t, 3>;
        auto* indices = static_cast<Index*>(rtcSetNewGeometryBuffer(
            geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Index), source_ids.size()
        ));
        check_error();

        // Source IDs preserve material identity after opaque/cutout partitioning.
        for (std::size_t i = 0; i < source_ids.size(); ++i) {
            // Each triangle uploads three independent source vertices.
            for (unsigned j = 0; j < 3; ++j) {
                vertices[3 * i + j] = input.triangles[source_ids[i]].vertices[j];
            }

            const auto first = static_cast<std::uint32_t>(i * 3);
            indices[i] = {first, first + 1, first + 2};
        }

        // Opaque geometry bypasses the coverage callback entirely.
        if (group == 1) {
            rtcSetGeometryUserData(geometry, this);
            rtcSetGeometryIntersectFilterFunction(geometry, filter_alpha);
            rtcSetGeometryOccludedFilterFunction(geometry, filter_alpha);
        }

        rtcCommitGeometry(geometry);
        rtcAttachGeometryByID(handle, geometry, group);
        check_error();
    } catch (...) {
        // Only successfully created geometry needs release during failure cleanup.
        if (geometry) {
            rtcReleaseGeometry(geometry);
        }

        throw;
    }

    rtcReleaseGeometry(geometry);
}

Scene::Scene(const BakeScene& source, const std::optional<Float3>& sunlight_direction)
    : input(source) {
    collect_triangles();

    // Normalise authored sun directions once for all probe grids.
    for (const auto& sun : input.suns) {
        // Secondary fill retains its authored direction.
        const auto direction =
            sun.primary ? sunlight_direction.value_or(sun.direction) : sun.direction;
        to_suns.push_back(mul(unit(direction), -1));
    }

    device = rtcNewDevice(nullptr);

    // Device creation may fail before a scene can own it.
    if (!device) {
        throw std::runtime_error("could not create Embree device");
    }

    // Release the device and scene together if preparation fails.
    try {
        handle = rtcNewScene(device);
        check_error();
        rtcSetSceneFlags(handle, RTC_SCENE_FLAG_ROBUST);

        // Opaque and cutout groups have separate traversal filters.
        for (unsigned group = 0; group < triangle_ids.size(); ++group) {
            build_geometry(group);
        }

        // Point emitters follow surface emitters in the original selection order.
        for (std::size_t i = 0; i < input.point_emitters.size(); ++i) {
            const auto& light = input.point_emitters[i];
            const float power = *std::max_element(light.intensity.begin(), light.intensity.end());

            // Zero-power point emitters cannot contribute to transport.
            if (power > 0) {
                emission_weight += power;
                emitters.push_back({input.triangles.size() + i, emission_weight, 0, {}});
            }
        }

        rtcCommitScene(handle);
        check_error();
    } catch (...) {
        // Scene creation can fail before returning an owned handle.
        if (handle) {
            rtcReleaseScene(handle);
        }

        rtcReleaseDevice(device);
        throw;
    }
}

Scene::~Scene() {
    rtcReleaseScene(handle);
    rtcReleaseDevice(device);
}

// ---- Source validation ----

// Validate materials, lighting and dimensions before any ray can use them.
void validate_bake_scene(const BakeScene& input, const BakeSettings& settings) {
    validate_settings(settings, input.triangles.size());
    validate_colour(input.environment, false);
    validate_colour(input.primary_directional_colour, false);

    // Point-light falloff requires finite positions.
    for (const auto& light : input.point_emitters) {
        // Reject invalid light positions before any visibility ray is created.
        if (!finite(light.position)) {
            throw std::runtime_error("invalid bake point emitter position");
        }

        validate_colour(light.intensity, false);
    }

    // Sky dimensions and pixel count must describe the same panorama.
    if ((!input.sky_radiance.empty() || input.sky_width || input.sky_height)
        && (!input.sky_width || !input.sky_height || input.sky_width > 16384
            || input.sky_height > 16384
            || input.sky_radiance.size() != std::size_t{input.sky_width} * input.sky_height)) {
        throw std::runtime_error("invalid bake sky panorama");
    }

    // Every sky pixel contributes to environment filtering.
    for (const auto& pixel : input.sky_radiance) {
        validate_colour(pixel, false);
    }

    // Validate texture storage before any material lookup or callback.
    for (const auto& texture : input.textures) {
        // RGBA allocation size must match finite bounded dimensions.
        if (!texture.width || !texture.height || texture.width > 65536 || texture.height > 65536
            || texture.rgba.size() != std::size_t{texture.width} * texture.height * 4) {
            throw std::runtime_error("invalid bake texture");
        }
    }

    // Validate each material before building the corresponding geometry.
    for (const auto& triangle : input.triangles) {
        validate_triangle(triangle, input, settings);
    }

    // Authored sunlight requires valid radiance and a nonzero direction.
    for (const auto& sun : input.suns) {
        validate_colour(sun.irradiance, false);
        unit(sun.direction);
    }
}
}  // namespace openrac::lighting::bake_detail
