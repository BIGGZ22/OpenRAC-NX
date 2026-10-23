// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/lighting/bake.h"
// Public Embree 4 API, Apache-2.0; see THIRD_PARTY_NOTICES.md at the repo root.
#include <algorithm>
#include <atomic>
#include <cfenv>
#include <cmath>
#include <condition_variable>
#include <exception>
#include <limits>
#include <mutex>
#include <numbers>
#include <stdexcept>
#include <thread>

#include "common/joining_thread.h"

#include <embree4/rtcore.h>

namespace openrac::lighting {
namespace {
constexpr float pi = std::numbers::pi_v<float>;

float dot(Float3 a, Float3 b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Float3 add(Float3 a, Float3 b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

Float3 mul(Float3 a, float s) {
    return {a[0] * s, a[1] * s, a[2] * s};
}

Float3 cross(Float3 a, Float3 b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

Float3 unit(Float3 a) {
    const auto length = std::sqrt(dot(a, a));
    if (!std::isfinite(length) || length <= 0) {
        throw std::runtime_error("invalid bake vector");
    }
    return mul(a, 1 / length);
}

bool finite(Float3 v) {
    return std::all_of(v.begin(), v.end(), [](float x) { return std::isfinite(x); });
}

void check_error(RTCDevice device) {
    if (rtcGetDeviceError(device) != RTC_ERROR_NONE) {
        throw std::runtime_error("Embree probe bake failed");
    }
}

struct Scene {
    const BakeScene& input;
    RTCDevice device = nullptr;
    RTCScene handle = nullptr;

    struct Emitter {
        std::size_t triangle;
        double cumulative;
        float area;
        Float3 normal;
    };

    std::vector<Emitter> emitters;
    double emission_weight = 0;

    explicit Scene(const BakeScene& source) : input(source) {
        device = rtcNewDevice(nullptr);
        if (!device) {
            throw std::runtime_error("could not create Embree device");
        }
        try {
            handle = rtcNewScene(device);
            check_error(device);
            rtcSetSceneFlags(handle, RTC_SCENE_FLAG_ROBUST);
            if (!input.triangles.empty()) {
                RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
                try {
                    check_error(device);
                    auto* vertices = static_cast<Float3*>(rtcSetNewGeometryBuffer(
                        geometry,
                        RTC_BUFFER_TYPE_VERTEX,
                        0,
                        RTC_FORMAT_FLOAT3,
                        sizeof(Float3),
                        input.triangles.size() * 3
                    ));
                    using Index = std::array<std::uint32_t, 3>;
                    auto* indices = static_cast<Index*>(rtcSetNewGeometryBuffer(
                        geometry,
                        RTC_BUFFER_TYPE_INDEX,
                        0,
                        RTC_FORMAT_UINT3,
                        sizeof(Index),
                        input.triangles.size()
                    ));
                    check_error(device);
                    for (std::size_t i = 0; i < input.triangles.size(); ++i) {
                        const auto& triangle = input.triangles[i];
                        const float strength =
                            *std::max_element(triangle.emission.begin(), triangle.emission.end());
                        if (strength > 0) {
                            const auto a = add(triangle.vertices[1], mul(triangle.vertices[0], -1));
                            const auto b = add(triangle.vertices[2], mul(triangle.vertices[0], -1));
                            const auto normal = cross(a, b);
                            const float twice_area = std::sqrt(dot(normal, normal));
                            if (twice_area > 1e-12f) {
                                emission_weight += double(twice_area) * .5 * strength;
                                emitters.push_back(
                                    {i,
                                     emission_weight,
                                     twice_area * .5f,
                                     mul(normal, 1 / twice_area)}
                                );
                            }
                        }
                        for (unsigned j = 0; j < 3; ++j) {
                            vertices[3 * i + j] = input.triangles[i].vertices[j];
                        }
                        const auto first = static_cast<std::uint32_t>(i * 3);
                        indices[i] = {first, first + 1, first + 2};
                    }
                    rtcCommitGeometry(geometry);
                    rtcAttachGeometry(handle, geometry);
                    check_error(device);
                } catch (...) {
                    if (geometry) {
                        rtcReleaseGeometry(geometry);
                    }
                    throw;
                }
                rtcReleaseGeometry(geometry);
            }
            for (std::size_t i = 0; i < input.point_emitters.size(); ++i) {
                const auto& light = input.point_emitters[i];
                const float power =
                    *std::max_element(light.intensity.begin(), light.intensity.end());
                if (power > 0) {
                    emission_weight += power;
                    emitters.push_back({input.triangles.size() + i, emission_weight, 0, {}});
                }
            }
            rtcCommitScene(handle);
            check_error(device);
        } catch (...) {
            if (handle) {
                rtcReleaseScene(handle);
            }
            rtcReleaseDevice(device);
            throw;
        }
    }

    ~Scene() {
        rtcReleaseScene(handle);
        rtcReleaseDevice(device);
    }

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    RTCRayHit trace(Float3 origin, Float3 direction, float near, float far) const {
        RTCRayHit hit{};
        hit.ray.org_x = origin[0];
        hit.ray.org_y = origin[1];
        hit.ray.org_z = origin[2];
        hit.ray.dir_x = direction[0];
        hit.ray.dir_y = direction[1];
        hit.ray.dir_z = direction[2];
        hit.ray.tnear = near;
        hit.ray.tfar = far;
        hit.ray.mask = 0xffffffffu;
        hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        for (;;) {
            hit.ray.tfar = far;
            hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
            rtcIntersect1(handle, &hit);
            if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
                break;
            }
            const auto& triangle = input.triangles[hit.hit.primID];
            if (triangle.texture < 0
                || bake_material(input, triangle, hit.hit.u, hit.hit.v)[3]
                       >= triangle.alpha_cutoff) {
                break;
            }
            // A cutout texel is not a first hit, a shadow blocker, or a bounce.
            hit.ray.tnear = std::nextafter(hit.ray.tfar, std::numeric_limits<float>::infinity());
            if (hit.ray.tnear >= far) {
                hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
                hit.ray.tfar = far;
                break;
            }
        }
        return hit;
    }
};

struct Random {
    std::uint32_t state;

    float next() {
        state = state * 1664525u + 1013904223u;
        return static_cast<float>(state >> 8) / 16777216.0f;
    }
};

struct EmissionSample {
    Float3 direction{}, radiance_weight{};
};

// Area/power selection and uniform triangle barycentrics. The returned value
// is Le / pdf(solid angle); multiply by the receiving cosine and average.
// Two-sided emission deliberately does not depend on authored normals/winding.
EmissionSample sample_emitter(const Scene& scene, Float3 origin, float bias, Random& random) {
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
    if (it == scene.emitters.end()) {
        it = scene.emitters.end() - 1;
    }
    const double previous = it == scene.emitters.begin() ? 0 : (it - 1)->cumulative;
    const double probability = (it->cumulative - previous) / scene.emission_weight;
    if (it->area == 0) {
        const auto& light =
            scene.input.point_emitters.at(it->triangle - scene.input.triangles.size());
        const auto delta = add(light.position, mul(origin, -1));
        const float squared = dot(delta, delta), distance = std::sqrt(squared);
        if (distance <= 4 * bias) {
            return {};
        }
        const auto direction = mul(delta, 1 / distance);
        if (scene.trace(origin, direction, bias, distance - 2 * bias).hit.geomID
            != RTC_INVALID_GEOMETRY_ID) {
            return {};
        }
        return {direction, mul(light.intensity, static_cast<float>(1 / (squared * probability)))};
    }
    const double pdf_area = probability / it->area;
    const auto& triangle = scene.input.triangles[it->triangle];
    const float root = std::sqrt(random.next());
    const float u = root * (1 - random.next()), v = root - u;
    Float3 position{};
    for (unsigned c = 0; c < 3; ++c) {
        position[c] = triangle.vertices[0][c] * (1 - u - v) + triangle.vertices[1][c] * u
                      + triangle.vertices[2][c] * v;
    }
    const auto delta = add(position, mul(origin, -1));
    const float squared = dot(delta, delta), distance = std::sqrt(squared);
    if (distance <= 4 * bias
        || bake_material(scene.input, triangle, u, v)[3] < triangle.alpha_cutoff) {
        return {};
    }
    const auto direction = mul(delta, 1 / distance);
    const float cosine = std::abs(dot(it->normal, direction));
    if (cosine <= 0
        || scene.trace(origin, direction, bias, distance - 2 * bias).hit.geomID
               != RTC_INVALID_GEOMETRY_ID) {
        return {};
    }
    return {
        direction,
        mul(bake_emission(scene.input, triangle, u, v),
            static_cast<float>(double(cosine) / (double(squared) * pdf_area)))
    };
}

struct Placement {
    bool inside = false;
    float nearest = std::numeric_limits<float>::infinity();
    Float3 exit_direction{};
};

Placement classify(const Scene& scene, Float3 position, const BakeSettings& settings) {
    // A ray and its reverse must both see odd crossing counts to vote inside.
    // An open floor gives disagreeing parities, regardless of its winding.
    // Trace beyond the moment range: it is not an occupancy boundary.
    constexpr unsigned pairs = 32;
    Placement result;
    unsigned inside = 0;
    for (unsigned i = 0; i < pairs; ++i) {
        const float z = (static_cast<float>(i) + 0.5f) / pairs;
        const float r = std::sqrt(1 - z * z);
        const float angle = static_cast<float>(i) * 2.39996323f;
        const Float3 direction{r * std::cos(angle), r * std::sin(angle), z};
        bool odd[2]{};
        for (unsigned side = 0; side < 2; ++side) {
            const auto ray = mul(direction, side == 0 ? 1.0f : -1.0f);
            float near = settings.ray_bias;
            for (unsigned crossings = 0;; ++crossings) {
                const auto hit =
                    scene.trace(position, ray, near, std::numeric_limits<float>::infinity());
                if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
                    break;
                }
                if (crossings >= 4096) {
                    throw std::runtime_error("probe occupancy ray exceeds crossing limit");
                }
                odd[side] = !odd[side];
                if (hit.ray.tfar < result.nearest) {
                    result.nearest = hit.ray.tfar;
                    result.exit_direction = ray;
                }
                // Treat coincident faces as one crossing and avoid re-hitting
                // shared edges. Layers thinner than ray_bias are unresolved.
                near = std::nextafter(
                    hit.ray.tfar + settings.ray_bias, std::numeric_limits<float>::infinity()
                );
            }
        }
        inside += odd[0] && odd[1] ? 1u : 0u;
    }
    result.inside = static_cast<float>(inside) / pairs >= settings.min_inside_fraction;
    return result;
}

Float3 hemisphere(Float3 n, Random& random) {
    const float u = random.next(), angle = 2 * pi * random.next();
    const float r = std::sqrt(u), z = std::sqrt(1 - u);
    const auto tangent = unit(cross(std::abs(n[2]) < 0.99f ? Float3{0, 0, 1} : Float3{0, 1, 0}, n));
    const auto bitangent = cross(n, tangent);
    return add(
        add(mul(tangent, r * std::cos(angle)), mul(bitangent, r * std::sin(angle))), mul(n, z)
    );
}

Float3 radiance(
    const Scene& acceleration,
    const BakeScene& scene,
    const BakeSettings& settings,
    Float3 origin,
    Float3 direction,
    Random& random
) {
    // The finite moment range is a storage convention, not a scene clipping
    // distance. Geometry beyond it still blocks the sky and analytic sun.
    constexpr float far = std::numeric_limits<float>::infinity();
    Float3 result{}, throughput{1, 1, 1};
    for (std::uint32_t bounce = 0; bounce <= settings.bounces; ++bounce) {
        const auto hit = acceleration.trace(origin, direction, settings.ray_bias, far);
        if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
            const auto environment = sample_bake_environment(scene, direction, settings);
            for (unsigned c = 0; c < 3; ++c) {
                result[c] += throughput[c] * environment[c];
            }
            break;
        }
        // Last depth still permits an escape to the environment above, but
        // no extra surface response beyond the configured surface bounce count.
        if (bounce == settings.bounces) {
            break;
        }
        auto n = unit({hit.hit.Ng_x, hit.hit.Ng_y, hit.hit.Ng_z});
        if (dot(n, direction) > 0) {
            n = mul(n, -1);
        }
        const auto p = add(origin, mul(direction, hit.ray.tfar));
        const auto albedo = bake_material(
            scene,
            scene.triangles.at(hit.hit.primID),
            hit.hit.u,
            hit.hit.v,
            {1, 1, 1},
            settings.terrain_vertex_albedo
        );
        if (!acceleration.emitters.empty()) {
            const auto light = sample_emitter(
                acceleration, add(p, mul(n, settings.ray_bias)), settings.ray_bias, random
            );
            const float cosine = std::max(0.0f, dot(n, light.direction));
            for (unsigned c = 0; c < 3; ++c) {
                result[c] += throughput[c] * albedo[c] * light.radiance_weight[c] * cosine / pi;
            }
        }
        // Emission reached by this continuation ray is excluded: explicit
        // emitter sampling at every preceding vertex (including the probe)
        // already estimates that path. This avoids double counting, without MIS.
        for (const auto& sun : scene.suns) {
            const auto to_sun = mul(unit(sun.direction), -1);
            const float cosine = std::max(0.0f, dot(n, to_sun));
            if (cosine <= 0) {
                continue;
            }
            const auto blocker = acceleration.trace(
                add(p, mul(n, settings.ray_bias)), to_sun, settings.ray_bias, far
            );
            if (blocker.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
                continue;
            }
            for (unsigned c = 0; c < 3; ++c) {
                const float correction = sun.primary ? settings.sunlight_scale[c] : 1.0f;
                result[c] +=
                    throughput[c] * albedo[c] * sun.irradiance[c] * correction * cosine / pi;
            }
        }
        for (unsigned c = 0; c < 3; ++c) {
            throughput[c] *= albedo[c];
        }
        origin = add(p, mul(n, settings.ray_bias));
        direction = hemisphere(n, random);
    }
    return result;
}
}  // namespace

Float3 sample_bake_environment(
    const BakeScene& scene, Float3 direction, const BakeSettings& settings
) {
    if (!settings.use_level_sky || scene.sky_radiance.empty()) {
        return scene.environment;
    }
    direction = unit(direction);
    float longitude = std::atan2(direction[0], direction[1]) / (2 * pi);
    longitude -= std::floor(longitude);
    const float x = longitude * static_cast<float>(scene.sky_width) - 0.5f;
    const float y = std::clamp(
        std::acos(std::clamp(direction[2], -1.0f, 1.0f)) / pi * static_cast<float>(scene.sky_height)
            - 0.5f,
        0.0f,
        static_cast<float>(scene.sky_height - 1)
    );
    const int ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y));
    const float fx = x - std::floor(x), fy = y - static_cast<float>(iy);
    Float3 result{};
    for (int j = 0; j < 2; ++j) {
        for (int i = 0; i < 2; ++i) {
            const auto sx = static_cast<std::uint32_t>(
                (ix + i + static_cast<int>(scene.sky_width)) % static_cast<int>(scene.sky_width)
            );
            const auto sy = std::min(static_cast<std::uint32_t>(iy + j), scene.sky_height - 1);
            const float weight = (i ? fx : 1 - fx) * (j ? fy : 1 - fy) * settings.sky_intensity;
            const auto& pixel = scene.sky_radiance[std::size_t{sy} * scene.sky_width + sx];
            for (unsigned c = 0; c < 3; ++c) {
                result[c] += pixel[c] * weight;
            }
        }
    }
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
    Float3 texture_tint,
    bool terrain_vertex_albedo
) {
    const bool authored = terrain_vertex_albedo && triangle.terrain_prelighting;
    Float3 authored_tint{1, 1, 1};
    if (authored) {
        for (unsigned c = 0; c < 3; ++c) {
            authored_tint[c] = triangle.authored_colours[0][c] * (1 - u - v)
                               + triangle.authored_colours[1][c] * u
                               + triangle.authored_colours[2][c] * v;
        }
    }
    const auto linear = [](float f) {
        return f <= .04045f ? f / 12.92f : std::pow((f + .055f) / 1.055f, 2.4f);
    };
    std::array<float, 4> value{triangle.diffuse[0], triangle.diffuse[1], triangle.diffuse[2], 1};
    for (unsigned c = 0; c < 4; ++c) {
        value[c] *= triangle.multipliers[0][c] * (1 - u - v) + triangle.multipliers[1][c] * u
                    + triangle.multipliers[2][c] * v;
    }
    if (triangle.texture < 0) {
        for (unsigned c = 0; c < 3; ++c) {
            const float f = std::clamp(texture_tint[c], 0.0f, 1.0f);
            value[c] = authored ? linear(std::clamp(value[c] * authored_tint[c] * f, 0.f, 1.f))
                                : value[c] * linear(f);
        }
        for (unsigned c = 0; c < 3; ++c) {
            value[c] = std::clamp(value[c] * triangle.diffuse_scale, 0.f, 1.f);
        }
        value[3] = std::clamp(value[3], 0.f, 1.f);
        return value;
    }
    const auto& texture = scene.textures.at(static_cast<std::size_t>(triangle.texture));
    const auto& uv = triangle.uv;
    const float s = uv[0][0] * (1 - u - v) + uv[1][0] * u + uv[2][0] * v;
    const float t = uv[0][1] * (1 - u - v) + uv[1][1] * u + uv[2][1] * v;
    const float x = (s - std::floor(s)) * static_cast<float>(texture.width) - 0.5f;
    const float y = (t - std::floor(t)) * static_cast<float>(texture.height) - 0.5f;
    const auto ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y));
    const float tx = x - std::floor(x), ty = y - std::floor(y);
    std::array<float, 4> filtered{};
    for (int j = 0; j < 2; ++j) {
        for (int i = 0; i < 2; ++i) {
            const auto px = static_cast<std::uint32_t>(
                (ix + i + static_cast<int>(texture.width)) % static_cast<int>(texture.width)
            );
            const auto py = static_cast<std::uint32_t>(
                (iy + j + static_cast<int>(texture.height)) % static_cast<int>(texture.height)
            );
            const auto at = (std::size_t{py} * texture.width + px) * 4;
            const float weight = (i ? tx : 1 - tx) * (j ? ty : 1 - ty);
            for (unsigned c = 0; c < 4; ++c) {
                float f = static_cast<float>(texture.rgba[at + c]) / 255;
                if (c < 3) {
                    f = std::clamp(f * texture_tint[c], 0.0f, 1.0f);
                    if (!authored) {
                        f = linear(f);
                    }
                }
                filtered[c] += f * weight;
            }
        }
    }
    for (unsigned c = 0; c < 4; ++c) {
        const float product = value[c] * filtered[c];
        value[c] = product;
        if (authored && c < 3) {
            // Match the original terrain's encoded texture filtering and GS modulation.
            value[c] = linear(std::clamp(product * authored_tint[c], 0.f, 1.f));
        }
        if (c < 3) {
            value[c] = std::clamp(value[c] * triangle.diffuse_scale, 0.f, 1.f);
        } else {
            value[c] = std::clamp(value[c], 0.f, 1.f);
        }
    }
    return value;
}

Float3 bake_emission(const BakeScene& scene, const BakeTriangle& triangle, float u, float v) {
    BakeTriangle mask{};
    mask.diffuse = {1, 1, 1};
    mask.texture = triangle.emission_texture;
    mask.uv = triangle.uv;
    Float3 tint{1, 1, 1};
    if (triangle.emission_vertex_colour) {
        for (unsigned c = 0; c < 3; ++c) {
            tint[c] = triangle.emission_colours[0][c] * (1 - u - v)
                      + triangle.emission_colours[1][c] * u + triangle.emission_colours[2][c] * v;
        }
    }
    const auto radiance = bake_material(scene, mask, u, v, tint);
    float coverage = 1;
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

ProbeVolume bake_probes(
    const ProbeGrid& grid,
    const BakeScene& input,
    const BakeSettings& settings,
    const std::function<void(std::size_t, std::size_t)>& progress,
    std::uint32_t thread_count
) {
    grid.validate();
    if (!finite(settings.sunlight_scale)
        || std::any_of(settings.sunlight_scale.begin(), settings.sunlight_scale.end(), [](float v) {
               return v < 0 || v > 4;
           })) {
        throw std::runtime_error("sunlight_scale requires three finite values between 0 and 4");
    }
    if (settings.rays_per_probe < 16 || settings.bounces > 32 || !std::isfinite(settings.ray_bias)
        || !std::isfinite(settings.sky_intensity) || settings.sky_intensity < 0
        || !std::isfinite(settings.sky_weight) || settings.sky_weight < 0 || settings.sky_weight > 1
        || settings.ray_bias <= 0 || settings.ray_bias >= grid.max_distance
        || !std::isfinite(settings.distance_exponent) || settings.distance_exponent < 1
        || settings.distance_exponent > 256 || !std::isfinite(settings.min_inside_fraction)
        || settings.min_inside_fraction <= 0.5f || settings.min_inside_fraction > 1
        || settings.relocation_iterations > 32 || !std::isfinite(settings.relocation_max_fraction)
        || settings.relocation_max_fraction < 0 || settings.relocation_max_fraction > 0.45f
        || input.triangles.size() > 0xffffffffu / 3) {
        throw std::runtime_error("invalid probe bake settings");
    }
    auto colour = [](Float3 c, bool bounded) {
        if (!finite(c) || *std::min_element(c.begin(), c.end()) < 0
            || (bounded && *std::max_element(c.begin(), c.end()) > 1)) {
            throw std::runtime_error("invalid linear bake colour");
        }
    };
    colour(input.environment, false);
    colour(input.primary_directional_colour, false);
    for (const auto& light : input.point_emitters) {
        if (!finite(light.position)) {
            throw std::runtime_error("invalid bake point emitter position");
        }
        colour(light.intensity, false);
    }
    if ((!input.sky_radiance.empty() || input.sky_width || input.sky_height)
        && (!input.sky_width || !input.sky_height || input.sky_width > 16384
            || input.sky_height > 16384
            || input.sky_radiance.size() != std::size_t{input.sky_width} * input.sky_height)) {
        throw std::runtime_error("invalid bake sky panorama");
    }
    for (const auto& pixel : input.sky_radiance) {
        colour(pixel, false);
    }
    for (const auto& texture : input.textures) {
        if (!texture.width || !texture.height || texture.width > 65536 || texture.height > 65536
            || texture.rgba.size() != std::size_t{texture.width} * texture.height * 4) {
            throw std::runtime_error("invalid bake texture");
        }
    }
    for (const auto& triangle : input.triangles) {
        if (settings.terrain_vertex_albedo && triangle.terrain_prelighting) {
            for (auto tint : triangle.authored_colours) {
                colour(tint, false);
            }
        }
        if (!std::isfinite(triangle.diffuse_scale) || triangle.diffuse_scale < 0
            || triangle.diffuse_scale > 1) {
            throw std::runtime_error("invalid bake diffuse scale");
        }
        colour(triangle.diffuse, true);
        colour(triangle.emission, false);
        if (triangle.emission_vertex_colour) {
            for (const auto& tint : triangle.emission_colours) {
                colour(tint, false);
            }
        }
        if (triangle.emission_texture < -1
            || (triangle.emission_texture >= 0
                && static_cast<std::size_t>(triangle.emission_texture) >= input.textures.size())) {
            throw std::runtime_error("invalid bake emission texture");
        }
        for (const auto& multiplier : triangle.multipliers) {
            colour({multiplier[0], multiplier[1], multiplier[2]}, false);
            if (!std::isfinite(multiplier[3]) || multiplier[3] < 0) {
                throw std::runtime_error("invalid bake vertex alpha");
            }
        }
        if (triangle.texture < -1
            || (triangle.texture >= 0
                && static_cast<std::size_t>(triangle.texture) >= input.textures.size())
            || !std::isfinite(triangle.alpha_cutoff) || triangle.alpha_cutoff < 0
            || triangle.alpha_cutoff > 1) {
            throw std::runtime_error("invalid bake material");
        }
        for (auto uv : triangle.uv) {
            if (!std::isfinite(uv[0]) || !std::isfinite(uv[1])) {
                throw std::runtime_error("invalid bake UV");
            }
        }
        for (auto p : triangle.vertices) {
            if (!finite(p)) {
                throw std::runtime_error("invalid bake vertex");
            }
        }
    }
    for (const auto& sun : input.suns) {
        colour(sun.irradiance, false);
        unit(sun.direction);
    }
    Scene scene(input);
    if (progress) {
        progress(0, grid.probe_count());
    }
    auto out = ProbeVolume::allocate(grid);

    struct RaySample {
        Float3 direction, radiance;
        float distance;
    };

    const auto resolution = grid.map_resolution, side = resolution + 2;
    const auto bake_probe = [&](std::size_t probe,
                                std::vector<RaySample>& rays,
                                std::vector<EmissionSample>& emission) {
        // Seek a nearby exit from geometric occupancy before baking. Keep
        // the nominal cell fixed and cap displacement to avoid moving a
        // probe into another cell. Unreachable placements remain invalid.
        const float relocation_limit =
            settings.relocation_max_fraction
            * *std::min_element(grid.spacing.begin(), grid.spacing.end());
        auto placement = classify(scene, out.position(probe), settings);
        for (std::uint32_t iteration = 0;
             placement.inside && iteration < settings.relocation_iterations;
             ++iteration) {
            const auto candidate =
                add(out.probes[probe].offset,
                    mul(placement.exit_direction, placement.nearest + 4 * settings.ray_bias));
            if (dot(candidate, candidate) > relocation_limit * relocation_limit) {
                break;
            }
            out.probes[probe].offset = candidate;
            placement = classify(scene, out.position(probe), settings);
        }
        out.probes[probe].valid = !placement.inside;
        const auto origin = out.position(probe);
        Random random{settings.seed ^ (static_cast<std::uint32_t>(probe) * 747796405u)};
        const float rotation = random.next() * 2 * pi;
        // Separate sequence: adding emitters cannot perturb first-hit moment rays.
        Random emission_random{
            settings.seed ^ (static_cast<std::uint32_t>(probe) * 747796405u) ^ 0x9e3779b9u
        };
        for (auto& light : emission) {
            light = sample_emitter(scene, origin, settings.ray_bias, emission_random);
        }
        for (std::uint32_t i = 0; i < settings.rays_per_probe; ++i) {
            const float z =
                1
                - 2 * (static_cast<float>(i) + 0.5f) / static_cast<float>(settings.rays_per_probe);
            const float r = std::sqrt(std::max(0.0f, 1 - z * z));
            const float angle = static_cast<float>(i) * 2.39996323f + rotation;
            const Float3 direction{r * std::cos(angle), r * std::sin(angle), z};
            const auto hit = scene.trace(origin, direction, settings.ray_bias, grid.max_distance);
            rays[i] = {
                direction, radiance(scene, input, settings, origin, direction, random), hit.ray.tfar
            };
        }
        for (std::uint32_t y = 0; y < resolution; ++y) {
            for (std::uint32_t x = 0; x < resolution; ++x) {
                const auto direction = oct_decode(
                    {(static_cast<float>(x) + 0.5f) / static_cast<float>(resolution),
                     (static_cast<float>(y) + 0.5f) / static_cast<float>(resolution)}
                );
                Float3 e{};
                double weight = 0, first = 0, second = 0;
                for (const auto& ray : rays) {
                    const float cosine = std::max(0.0f, dot(direction, ray.direction));
                    for (unsigned c = 0; c < 3; ++c) {
                        e[c] += ray.radiance[c] * cosine;
                    }
                    const double w =
                        std::pow(static_cast<double>(cosine), settings.distance_exponent);
                    weight += w;
                    first += w * ray.distance;
                    second += w * ray.distance * ray.distance;
                }
                const auto at = probe * grid.texels_per_probe() + (y + 1) * side + x + 1;
                out.irradiance[at] = mul(e, 4 * pi / static_cast<float>(settings.rays_per_probe));
                for (const auto& light : emission) {
                    const float cosine = std::max(0.0f, dot(direction, light.direction));
                    for (unsigned c = 0; c < 3; ++c) {
                        out.irradiance[at][c] +=
                            light.radiance_weight[c] * cosine / static_cast<float>(emission.size());
                    }
                }
                if (weight > 1e-20) {
                    out.distance_moments[at] =
                        {static_cast<float>(first / weight), static_cast<float>(second / weight)};
                }
            }
        }
    };

    // Each probe has its own seed and disjoint output storage. Scheduling must
    // not change the ray sequence or the floating-point accumulation order.
    if (thread_count == 0) {
        thread_count = std::max(1u, std::thread::hardware_concurrency());
    }
    thread_count =
        static_cast<std::uint32_t>(std::min<std::size_t>(thread_count, out.probes.size()));
    std::fenv_t environment;
    if (std::fegetenv(&environment) != 0) {
        throw std::runtime_error("could not read bake floating-point environment");
    }
    std::atomic<std::size_t> next{0};
    std::atomic<bool> stopped{false};
    std::mutex mutex;
    std::condition_variable changed;
    std::size_t completed = 0;
    std::exception_ptr failure;
    const auto worker = [&] {
        try {
            if (thread_count > 1 && std::fesetenv(&environment) != 0) {
                throw std::runtime_error("could not set bake floating-point environment");
            }
            std::vector<RaySample> rays(settings.rays_per_probe);
            std::vector<EmissionSample> emission(
                scene.emitters.empty() ? 0 : settings.rays_per_probe
            );
            while (!stopped.load(std::memory_order_relaxed)) {
                const auto probe = next.fetch_add(1, std::memory_order_relaxed);
                if (probe >= out.probes.size()) {
                    break;
                }
                bake_probe(probe, rays, emission);
                if (thread_count == 1) {
                    if (progress) {
                        progress(probe + 1, out.probes.size());
                    }
                } else {
                    {
                        std::lock_guard lock(mutex);
                        ++completed;
                    }
                    changed.notify_one();
                }
            }
            // Embree records device errors per calling thread.
            check_error(scene.device);
        } catch (...) {
            {
                std::lock_guard lock(mutex);
                if (!failure) {
                    failure = std::current_exception();
                }
            }
            stopped.store(true, std::memory_order_relaxed);
            changed.notify_one();
        }
    };
    if (thread_count == 1) {
        worker();
    } else {
        // Keep callbacks on the calling thread, including exception handling.
        // Joining on every exit keeps the scene and output alive for workers.
        std::vector<JoiningThread> workers;
        try {
            for (std::uint32_t i = 0; i < thread_count; ++i) {
                workers.emplace_back(worker);
            }
            std::size_t reported = 0;
            while (reported < out.probes.size()) {
                std::unique_lock lock(mutex);
                changed.wait(lock, [&] { return failure || completed > reported; });
                if (failure) {
                    break;
                }
                reported = completed;
                lock.unlock();
                if (progress) {
                    progress(reported, out.probes.size());
                }
            }
        } catch (...) {
            stopped.store(true, std::memory_order_relaxed);
            throw;
        }
    }
    if (failure) {
        std::rethrow_exception(failure);
    }
    check_error(scene.device);
    update_probe_gutters(out);
    out.validate();
    return out;
}
}  // namespace openrac::lighting
