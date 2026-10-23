// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Cached directions, probe integration and worker lifetime for offline level baking.
#include "enhanced/lighting/bake.h"

#include <atomic>
#include <cfenv>
#include <condition_variable>
#include <exception>
#include <map>
#include <mutex>
#include <thread>

#include "common/joining_thread.h"
#include "enhanced/lighting/bake_math.h"
#include "enhanced/lighting/bake_transport.h"

namespace openrac::lighting {
namespace {
using bake_detail::add;
using bake_detail::dot;
using bake_detail::kPi;
using bake_detail::mul;
using bake_detail::Random;
using bake_detail::Scene;

// Tracing and explicit emission feed the same portable integration records.
using bake_detail::EmissionSample;
using bake_detail::RaySample;

// ---- Direction tables ----

// Generate the regular 2x2 footprint for reuse by all probes and volumes of this resolution.
std::vector<bake_detail::DirectionBlock> integration_directions(std::uint32_t resolution) {
    constexpr auto kLanes = bake_detail::kIntegrationLanes;
    const auto texels = std::size_t{resolution} * resolution;
    std::vector<bake_detail::DirectionBlock> directions((texels + kLanes - 1) / kLanes);

    for (std::uint32_t y = 0; y < resolution; ++y) {
        for (std::uint32_t x = 0; x < resolution; ++x) {
            const auto texel = std::size_t{y} * resolution + x;
            auto& footprint = directions[texel / kLanes];
            const auto lane = texel % kLanes;

            for (unsigned sample = 0; sample < bake_detail::kIntegrationDirections; ++sample) {
                const unsigned column = sample % kProbeTexelSamplesPerAxis;
                const unsigned row = sample / kProbeTexelSamplesPerAxis;
                const float u = (static_cast<float>(column) + 0.5f)
                                / static_cast<float>(kProbeTexelSamplesPerAxis);
                const float v = (static_cast<float>(row) + 0.5f)
                                / static_cast<float>(kProbeTexelSamplesPerAxis);
                const auto direction = oct_decode(
                    {(static_cast<float>(x) + u) / static_cast<float>(resolution),
                     (static_cast<float>(y) + v) / static_cast<float>(resolution)}
                );
                footprint.x[sample][lane] = direction[0];
                footprint.y[sample][lane] = direction[1];
                footprint.z[sample][lane] = direction[2];
            }
        }
    }

    return directions;
}

// Probe-independent part of a primary ray; azimuth rotation stays specific to each probe.
struct PrimaryDirection {
    float z, radius, angle;
};

// Small immutable tables shared across volumes with the same resolution and rounding mode.
struct BakeDirections {
    static constexpr unsigned kPlacementPairs = 32;  // paired occupancy rays over a hemisphere
    std::vector<PrimaryDirection> primary;
    std::array<std::array<Float3, 2>, kPlacementPairs> placement;
    std::vector<bake_detail::DirectionBlock> texels;

    BakeDirections(std::uint32_t rays, std::uint32_t resolution)
        : texels(integration_directions(resolution)) {
        primary.reserve(rays);

        for (std::uint32_t i = 0; i < rays; ++i) {
            const float z = 1 - 2 * (static_cast<float>(i) + 0.5f) / static_cast<float>(rays);
            primary.push_back(
                {z, std::sqrt(std::max(0.0f, 1 - z * z)), static_cast<float>(i) * 2.39996323f}
            );
        }

        for (unsigned i = 0; i < kPlacementPairs; ++i) {
            const float z = (static_cast<float>(i) + 0.5f) / kPlacementPairs;
            const float r = std::sqrt(1 - z * z);
            const float angle = static_cast<float>(i) * 2.39996323f;
            const Float3 direction{r * std::cos(angle), r * std::sin(angle), z};
            placement[i] = {mul(direction, 1.0f), mul(direction, -1.0f)};
        }
    }
};

}  // namespace

// ---- Prepared baker ----

// The scene borrows its immutable source; the owning export outlives this implementation.
struct ProbeBaker::Impl {
    BakeSettings settings;
    Scene scene;
    std::mutex direction_mutex;  // protects table creation; workers only read immutable entries
    std::map<std::pair<std::uint32_t, int>, std::unique_ptr<const BakeDirections>> direction_cache;

    Impl(const BakeScene& input, const BakeSettings& options) : settings(options), scene(input) {}

    // Rounding is part of the key so reuse cannot change a later bake's arithmetic results.
    const BakeDirections& directions(std::uint32_t resolution) {
        const std::lock_guard lock(direction_mutex);
        auto& cached = direction_cache[{resolution, std::fegetround()}];

        if (!cached) {
            cached = std::make_unique<BakeDirections>(settings.rays_per_probe, resolution);
        }

        return *cached;
    }
};

ProbeBaker::ProbeBaker(const BakeScene& input, const BakeSettings& settings) {
    bake_detail::validate_bake_scene(input, settings);
    m_impl = std::make_unique<Impl>(input, settings);
}

ProbeBaker::~ProbeBaker() = default;

// ---- Probe workers ----

ProbeVolume ProbeBaker::bake(
    const ProbeGrid& grid,
    const std::function<void(std::size_t, std::size_t)>& progress,
    std::uint32_t thread_count
) const {
    grid.validate();
    const auto& settings = m_impl->settings;

    // A grid's moment range must still leave room for the configured ray bias.
    if (settings.ray_bias >= grid.max_distance) {
        throw std::runtime_error("invalid probe bake settings");
    }

    const auto& scene = m_impl->scene;

    if (progress) {
        progress(0, grid.probe_count());
    }

    auto out = ProbeVolume::allocate(grid);

    const auto resolution = grid.map_resolution, side = resolution + 2;
    const auto texel_count = std::size_t{resolution} * resolution;
    const auto& directions = m_impl->directions(resolution);
    const auto bake_probe = [&](std::size_t probe,
                                std::vector<RaySample>& rays,
                                std::vector<EmissionSample>& emission) {
        // Seek a nearby exit from geometric occupancy before baking. Keep
        // the nominal cell fixed and cap displacement to avoid moving a
        // probe into another cell. Unreachable placements remain invalid.
        const float relocation_limit =
            settings.relocation_max_fraction
            * *std::min_element(grid.spacing.begin(), grid.spacing.end());
        auto placement = classify(scene, out.position(probe), settings, directions.placement);

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
            placement = classify(scene, out.position(probe), settings, directions.placement);
        }

        out.probes[probe].valid = !placement.inside;
        const auto origin = out.position(probe);
        Random random{settings.seed ^ (static_cast<std::uint32_t>(probe) * 747796405u)};
        const float rotation = random.next() * 2 * kPi;
        // Separate sequence: adding emitters cannot perturb first-hit moment rays.
        Random emission_random{
            settings.seed ^ (static_cast<std::uint32_t>(probe) * 747796405u) ^ 0x9e3779b9u
        };

        for (auto& light : emission) {
            light = sample_emitter(scene, origin, settings.ray_bias, emission_random);
        }

        for (std::uint32_t i = 0; i < settings.rays_per_probe; ++i) {
            const auto& base = directions.primary[i];
            const float angle = base.angle + rotation;
            const Float3 direction{
                base.radius * std::cos(angle), base.radius * std::sin(angle), base.z
            };
            auto hit = scene.trace(
                origin, direction, settings.ray_bias, std::numeric_limits<float>::infinity()
            );
            // Distant geometry still blocks GI; only the stored visibility distance is capped.
            const float distance = std::min(hit.ray.tfar, grid.max_distance);
            rays[i] = {direction, radiance(scene, settings, hit, random), distance};
        }

        for (std::size_t block = 0; block < directions.texels.size(); ++block) {
            const auto samples = bake_detail::integrate_probe_block(
                directions.texels[block],
                rays,
                emission,
                settings.distance_exponent,
                grid.max_distance
            );

            for (std::size_t lane = 0; lane < bake_detail::kIntegrationLanes; ++lane) {
                const auto texel = block * bake_detail::kIntegrationLanes + lane;

                // Odd resolutions leave inactive lanes in the final block; never write them into gutters.
                if (texel >= texel_count) {
                    break;
                }

                const auto at = probe * grid.texels_per_probe() + (texel / resolution + 1) * side
                                + texel % resolution + 1;
                out.irradiance[at] = samples[lane].irradiance;
                out.distance_moments[at] = samples[lane].moments;
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
            scene.check_error();
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

    scene.check_error();
    update_probe_gutters(out);
    out.validate();
    return out;
}

ProbeVolume bake_probes(
    const ProbeGrid& grid,
    const BakeScene& scene,
    const BakeSettings& settings,
    const std::function<void(std::size_t, std::size_t)>& progress,
    std::uint32_t thread_count
) {
    grid.validate();
    return ProbeBaker(scene, settings).bake(grid, progress, thread_count);
}

}  // namespace openrac::lighting
