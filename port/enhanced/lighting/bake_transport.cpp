// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Sun/emitter visibility, surface bounces and winding-independent occupancy.
#include "enhanced/lighting/bake_transport.h"

#include "enhanced/lighting/bake_math.h"

namespace openrac::lighting::bake_detail {
namespace {
// Cosine-weighted continuation uses only the incoming ray and geometric normal.
Float3 hemisphere(Float3 n, Random& random) {
    const float u = random.next(), angle = 2 * kPi * random.next();
    const float r = std::sqrt(u), z = std::sqrt(1 - u);
    const auto tangent = unit(cross(std::abs(n[2]) < 0.99f ? Float3{0, 0, 1} : Float3{0, 1, 0}, n));
    const auto bitangent = cross(n, tangent);
    return add(
        add(mul(tangent, r * std::cos(angle)), mul(bitangent, r * std::sin(angle))), mul(n, z)
    );
}

// A power-weighted emitter choice with its discrete selection probability.
struct SelectedEmitter {
    const Scene::Emitter& emitter;  // Borrowed from the immutable acceleration scene.
    double probability;             // Probability before point/area conversion.
};

// Samples inverse-square point lighting over a finite visibility segment.
EmissionSample sample_point_emitter(
    const Scene& scene, const SelectedEmitter& selected, Float3 origin, float bias
) {
    const auto& light =
        scene.input.point_emitters.at(selected.emitter.triangle - scene.input.triangles.size());
    const auto delta = add(light.position, mul(origin, -1));
    const float squared = dot(delta, delta), distance = std::sqrt(squared);

    // Near-coincident endpoints cannot form a stable visibility interval.
    if (distance <= 4 * bias) {
        return {};
    }

    const auto direction = mul(delta, 1 / distance);

    // Stop at any accepted blocker before the sampled light.
    if (scene.occluded(origin, direction, bias, distance - 2 * bias)) {
        return {};
    }

    return {
        direction, mul(light.intensity, static_cast<float>(1 / (squared * selected.probability)))
    };
}

// Samples a two-sided triangle with uniform area barycentrics.
EmissionSample sample_area_emitter(
    const Scene& scene, const SelectedEmitter& selected, Float3 origin, float bias, Random& random
) {
    const double pdf_area = selected.probability / selected.emitter.area;
    const auto& triangle = scene.input.triangles[selected.emitter.triangle];
    const float root = std::sqrt(random.next());
    const float u = root * (1 - random.next()), v = root - u;
    Float3 position{};

    // Barycentric sampling is uniform over the selected triangle area.
    for (unsigned c = 0; c < 3; ++c) {
        position[c] = triangle.vertices[0][c] * (1 - u - v) + triangle.vertices[1][c] * u
                      + triangle.vertices[2][c] * v;
    }

    const auto delta = add(position, mul(origin, -1));
    const float squared = dot(delta, delta), distance = std::sqrt(squared);

    // Reject near-coincident or transparent emitter samples.
    if (distance <= 4 * bias
        || bake_material(scene.input, triangle, u, v)[3] < triangle.alpha_cutoff) {
        return {};
    }

    const auto direction = mul(delta, 1 / distance);
    const float cosine = std::abs(dot(selected.emitter.normal, direction));

    // Two-sided emitters use the absolute geometric receiving angle.
    if (cosine <= 0 || scene.occluded(origin, direction, bias, distance - 2 * bias)) {
        return {};
    }

    return {
        direction,
        mul(bake_emission(scene.input, triangle, u, v),
            static_cast<float>(double(cosine) / (double(squared) * pdf_area)))
    };
}

// Counts accepted crossings and retains the nearest relocation exit, independent of winding.
bool crossing_parity(
    const Scene& scene, Float3 position, Float3 ray, float bias, Placement& result
) {
    bool odd = false;
    float near = bias;

    // Continue until the unbounded ray escapes or exceeds the crossing limit.
    for (unsigned crossings = 0;; ++crossings) {
        const auto hit = scene.trace(position, ray, near, std::numeric_limits<float>::infinity());

        // Escaping rays complete this side of the parity vote.
        if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
            break;
        }

        // Pathological overlapping geometry must not trap the occupancy query.
        if (crossings >= 4096) {
            throw std::runtime_error("probe occupancy ray exceeds crossing limit");
        }

        odd = !odd;

        // The nearest accepted crossing supplies a relocation candidate.
        if (hit.ray.tfar < result.nearest) {
            result.nearest = hit.ray.tfar;
            result.exit_direction = ray;
        }

        // Treat coincident faces as one crossing; layers thinner than the bias remain unresolved.
        near = std::nextafter(hit.ray.tfar + bias, std::numeric_limits<float>::infinity());
    }

    return odd;
}

// Geometric receiving frame and linear diffuse reflectance at one accepted hit.
struct BounceSurface {
    Float3 position;              // Unbiased surface position.
    Float3 normal;                // Geometric normal facing the incoming ray.
    std::array<float, 4> albedo;  // Linear reflectance and coverage.
};

// Reconstructs a two-sided surface without consulting authored shading normals.
BounceSurface bounce_surface(
    const Scene& acceleration, const BakeSettings& settings, const RTCRayHit& hit
) {
    const Float3 direction{hit.ray.dir_x, hit.ray.dir_y, hit.ray.dir_z};
    const Float3 origin{hit.ray.org_x, hit.ray.org_y, hit.ray.org_z};
    auto normal = unit({hit.hit.Ng_x, hit.hit.Ng_y, hit.hit.Ng_z});

    // Face the geometric normal toward the incoming ray, independent of winding.
    if (dot(normal, direction) > 0) {
        normal = mul(normal, -1);
    }

    return {
        add(origin, mul(direction, hit.ray.tfar)),
        normal,
        bake_material(
            acceleration.input,
            acceleration.triangle(hit.hit),
            hit.hit.u,
            hit.hit.v,
            {{1, 1, 1}, settings.terrain_vertex_albedo}
        )
    };
}

// Accumulated radiance and diffuse throughput for a single continuation path.
struct BounceLighting {
    const Scene& acceleration;     // Immutable scene, owned by the prepared baker.
    const BakeSettings& settings;  // Immutable settings for this bake.
    Float3 result{};               // Radiance in the primary ray direction.
    Float3 throughput{1, 1, 1};    // Reflectance product of preceding surfaces.

    // Adds escaped sky radiance through the preceding surface reflectance.
    void add_environment(Float3 direction) {
        const auto environment = sample_bake_environment(acceleration.input, direction, settings);

        // Throughput carries the material response of previous bounces.
        for (unsigned c = 0; c < 3; ++c) {
            result[c] += throughput[c] * environment[c];
        }
    }

    // Adds visible emitters without counting emission hit by continuation rays a second time.
    void add_emitters(const BounceSurface& surface, Random& random) {
        // An empty emitter list consumes no transport random samples.
        if (acceleration.emitters.empty()) {
            return;
        }

        const auto light = sample_emitter(
            acceleration,
            add(surface.position, mul(surface.normal, settings.ray_bias)),
            settings.ray_bias,
            random
        );
        const float cosine = std::max(0.0f, dot(surface.normal, light.direction));

        // Convert the visible emitter estimate to diffuse reflected radiance.
        for (unsigned c = 0; c < 3; ++c) {
            result[c] +=
                throughput[c] * surface.albedo[c] * light.radiance_weight[c] * cosine / kPi;
        }
    }

    // Adds unblocked sunlight with the same per-level RGB correction used at runtime.
    void add_sunlight(const BounceSurface& surface) {
        const auto& scene = acceleration.input;

        // Each authored sun contributes independently to the receiving surface.
        for (std::size_t i = 0; i < scene.suns.size(); ++i) {
            const auto& sun = scene.suns[i];
            const auto to_sun = acceleration.to_suns[i];
            const float cosine = std::max(0.0f, dot(surface.normal, to_sun));

            // Back-facing sunlight contributes no diffuse reflection.
            if (cosine <= 0) {
                continue;
            }

            // Geometry blocks sunlight even beyond the stored moment range.
            if (acceleration.occluded(
                    add(surface.position, mul(surface.normal, settings.ray_bias)),
                    to_sun,
                    settings.ray_bias,
                    std::numeric_limits<float>::infinity()
                )) {
                continue;
            }

            // Primary sunlight uses the same per-level RGB scale as runtime lighting.
            for (unsigned c = 0; c < 3; ++c) {
                const float correction = sun.primary ? settings.sunlight_scale[c] : 1.0f;
                result[c] += throughput[c] * surface.albedo[c] * sun.irradiance[c] * correction
                             * cosine / kPi;
            }
        }
    }

    // Carries diffuse reflectance into the next continuation ray.
    void reflect(const BounceSurface& surface) {
        // Each channel carries its own reflectance through the continuation.
        for (unsigned c = 0; c < 3; ++c) {
            throughput[c] *= surface.albedo[c];
        }
    }
};

}  // namespace

// Area/power selection and uniform triangle barycentrics. The returned value
// is Le / pdf(solid angle); multiply by the receiving cosine and average.
// Two-sided emission deliberately does not depend on authored normals/winding.
EmissionSample sample_emitter(const Scene& scene, Float3 origin, float bias, Random& random) {
    // A scene without emitters consumes no emitter random samples.
    if (scene.emitters.empty()) {
        return {};
    }

    const double choice = random.next() * scene.emission_weight;
    auto it = std::upper_bound(
        scene.emitters.begin(),
        scene.emitters.end(),
        choice,
        [](double value, const Scene::Emitter& emitter) { return value < emitter.cumulative; }
    );

    // Rounding at the cumulative upper bound selects the final emitter.
    if (it == scene.emitters.end()) {
        it = scene.emitters.end() - 1;
    }

    const double previous = it == scene.emitters.begin() ? 0 : (it - 1)->cumulative;
    const SelectedEmitter selected{*it, (it->cumulative - previous) / scene.emission_weight};

    // Point lights use inverse-square falloff instead of an area PDF.
    if (it->area == 0) {
        return sample_point_emitter(scene, selected, origin, bias);
    }

    return sample_area_emitter(scene, selected, origin, bias, random);
}

Placement classify(
    const Scene& scene,
    Float3 position,
    const BakeSettings& settings,
    std::span<const std::array<Float3, 2>> directions
) {
    // A ray and its reverse must both see odd crossing counts to vote inside.
    // An open floor gives disagreeing parities, regardless of its winding.
    // Trace beyond the moment range: it is not an occupancy boundary.
    Placement result;
    unsigned inside = 0;

    // Paired crossing counts distinguish closed solids from open sheets.
    for (const auto& pair : directions) {
        // Evaluate both sides even when the first parity is even; both update the nearest exit.
        const bool forward = crossing_parity(scene, position, pair[0], settings.ray_bias, result);
        const bool reverse = crossing_parity(scene, position, pair[1], settings.ray_bias, result);
        inside += forward && reverse ? 1u : 0u;
    }

    result.inside = static_cast<float>(inside) / static_cast<float>(directions.size())
                    >= settings.min_inside_fraction;
    return result;
}

// Consume the primary hit in place; subsequent bounces overwrite it with full-scene hits.
Float3 radiance(
    const Scene& acceleration, const BakeSettings& settings, RTCRayHit& hit, Random& random
) {
    // The finite moment range is a storage convention, not a scene clipping distance.
    constexpr float kFar = std::numeric_limits<float>::infinity();
    Float3 direction{hit.ray.dir_x, hit.ray.dir_y, hit.ray.dir_z};
    BounceLighting lighting{acceleration, settings};

    // The final depth can escape to the sky without adding another surface bounce.
    for (std::uint32_t bounce = 0; bounce <= settings.bounces; ++bounce) {
        // Unblocked rays collect directional environment radiance.
        if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
            lighting.add_environment(direction);
            break;
        }

        // The final depth accepts sky escape but no further surface response.
        if (bounce == settings.bounces) {
            break;
        }

        const auto surface = bounce_surface(acceleration, settings, hit);
        lighting.add_emitters(surface, random);
        lighting.add_sunlight(surface);
        lighting.reflect(surface);
        const auto origin = add(surface.position, mul(surface.normal, settings.ray_bias));
        direction = hemisphere(surface.normal, random);
        hit = acceleration.trace(origin, direction, settings.ray_bias, kFar);
    }

    return lighting.result;
}

}  // namespace openrac::lighting::bake_detail
