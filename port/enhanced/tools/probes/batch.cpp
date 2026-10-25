// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Persistent single-level baking and shared CLI diagnostics/output.
#include "enhanced/tools/probes/batch.h"

#include <algorithm>
#include <cfenv>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "common/json.h"
#include "enhanced/tools/probes/report.h"

namespace fs = std::filesystem;

namespace openrac::lighting {
namespace {
// Restore the caller's arithmetic mode even if read-back diagnostics reject a volume.
class RuntimeRounding {
public:
    RuntimeRounding() {
        if (std::fegetenv(&m_previous) != 0 || std::fesetround(FE_TOWARDZERO) != 0) {
            throw std::runtime_error("cannot set runtime probe verification rounding");
        }
    }

    ~RuntimeRounding() { std::fesetenv(&m_previous); }

    RuntimeRounding(const RuntimeRounding&) = delete;
    RuntimeRounding& operator=(const RuntimeRounding&) = delete;

private:
    std::fenv_t m_previous{};  // restored before the next grid is traced
};

// Fixed names prevent planner input from redirecting writes to unrelated assets.
struct BatchJob {
    fs::path path;
    ProbeGrid grid;
};

// Reject malformed vector settings before any volume output is written.
template <std::size_t N>
std::array<float, N> plan_vector(const json::Value& value) {
    if (!value.is_array() || value.items().size() != N) {
        throw std::runtime_error("invalid probe batch vector");
    }

    std::array<float, N> result{};

    for (std::size_t i = 0; i < N; ++i) {
        if (!value[i].is_number() || !std::isfinite(value[i].number())) {
            throw std::runtime_error("nonfinite probe batch vector");
        }

        result[i] = static_cast<float>(value[i].number());
    }

    return result;
}

// Parse all grids first so bad metadata or existing outputs cannot produce a partial fresh bake.
std::vector<BatchJob> batch_jobs(
    const BakeSceneExport& exported, const ProbeGrid& sparse, const ProbeBatchOptions& options
) {
    json::Value plan;
    std::string error;
    constexpr std::size_t kMaximumRegions = 512;  // runtime dense-region limit

    if (!json::parse(options.plan, plan, error) || plan["version"].number() != 1
        || plan["scene_hash"].string() != std::to_string(exported.scene_hash)
        || !plan["regions"].is_array() || plan["regions"].items().size() > kMaximumRegions) {
        throw std::runtime_error("invalid or stale probe batch plan: " + error);
    }

    std::vector<BatchJob> jobs{{options.output_directory / "sparse.orpg", sparse}};

    for (const auto& region : plan["regions"].items()) {
        const auto bounds = plan_vector<6>(region["bounds"]);
        const auto spacing = plan_vector<3>(region["spacing"]);
        auto grid = bounded_probe_grid(
            exported,
            {bounds[0], bounds[1], bounds[2]},
            {bounds[3], bounds[4], bounds[5]},
            spacing,
            sparse.map_resolution
        );

        if (!region["max_distance"].is_number()) {
            throw std::runtime_error("probe batch moment distance missing");
        }

        grid.max_distance = static_cast<float>(region["max_distance"].number());
        grid.game_version = sparse.game_version;
        grid.level = sparse.level;
        grid.settings_hash = probe_bake_settings_hash(grid, options.settings);
        grid.validate();
        jobs.push_back(
            {options.output_directory / std::format("region-{:03}.orpg", jobs.size() - 1), grid}
        );
    }

    for (const auto& job : jobs) {
        for (const auto& suffix : {"", ".txt", ".tmp", ".txt.tmp"}) {
            const auto path = job.path.string() + suffix;
            if (!fs::exists(path)) {
                continue;
            }
            // Resuming: a write that was cut off is thrown away; a finished grid is checked later.
            if (options.resume && std::string_view(suffix).ends_with(".tmp")) {
                fs::remove(path);
            } else if (!options.resume) {
                throw std::runtime_error("probe batch output already exists");
            }
        }
    }

    return jobs;
}

// A grid a stopped bake already wrote, with its report: its bytes, or nothing when either file
// is missing.
std::optional<std::vector<std::uint8_t>> finished_bytes(const BatchJob& job) {
    if (!fs::exists(job.path) || !fs::exists(job.path.string() + ".txt")) {
        return std::nullopt;
    }
    std::ifstream input(job.path, std::ios::binary);
    std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>()
    );
    if (!input.good() && !input.eof()) {
        return std::nullopt;
    }
    return bytes;
}

// Read the actual file back and verify exactly the bytes produced by this grid.
ProbeVolume verify_batch_file(const BatchJob& job, const std::vector<std::uint8_t>& encoded) {
    std::ifstream input(job.path, std::ios::binary | std::ios::ate);

    if (!input || input.tellg() != static_cast<std::streamoff>(encoded.size())) {
        throw std::runtime_error("probe batch read-back size mismatch");
    }

    std::vector<std::uint8_t> bytes(encoded.size());
    input.seekg(0);

    if (!input.read(
            reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())
        )
        || bytes != encoded) {
        throw std::runtime_error("probe batch read-back bytes differ");
    }

    auto volume = decode_probes(bytes);
    const auto& grid = job.grid;

    if (!probe_identity_matches(
            volume, grid.game_version, grid.level, grid.scene_hash, grid.settings_hash
        )) {
        throw std::runtime_error("probe batch identity mismatch");
    }

    return volume;
}
}  // namespace

void run_probe_batch(
    const scene::LevelData& level,
    const BakeSceneExport& exported,
    const ProbeGrid& sparse_grid,
    const ProbeBatchOptions& options,
    std::ostream& output
) {
    const auto jobs = batch_jobs(exported, sparse_grid, options);

    if (probe_input_hash(level.dir) != options.source_snapshot) {
        throw std::runtime_error("source level changed during planning; no batch started");
    }

    ProbeBaker baker(exported.scene, options.settings, options.threads);
    std::optional<ProbeVolume> coarse;

    // Progress over the whole level, in probes: every grid's, the ones kept counted as done.
    std::size_t total = 0, done = 0, shown = 0;
    for (const auto& job : jobs) {
        total += job.grid.probe_count();
    }
    output << "Probe batch probes " << total << '\n' << std::flush;
    const auto report_done = [&](std::size_t now) {
        // Each half per cent at most: a level has up to a few million probes.
        if (total > 0 && (now == total || (now - shown) * 200 >= total)) {
            output << "Probes " << now << '/' << total << '\n' << std::flush;
            shown = now;
        }
    };

    for (const auto& job : jobs) {
        const auto start = std::chrono::steady_clock::now();
        if (options.resume) {
            if (const auto bytes = finished_bytes(job)) {
                try {
                    RuntimeRounding rounding;
                    auto checked = verify_batch_file(job, *bytes);
                    if (checked.grid.settings_hash
                        != probe_bake_settings_hash(checked.grid, options.settings)) {
                        throw std::runtime_error("kept probe grid has other settings");
                    }
                    if (coarse) {
                        validate_probe_pair(*coarse, checked);
                    } else {
                        coarse = std::move(checked);
                    }
                    output << "Kept " << job.path.filename().string() << '\n'
                           << "Verified " << job.path.filename().string() << '\n' << std::flush;
                    done += job.grid.probe_count();
                    report_done(done);
                    continue;
                } catch (const std::exception&) {
                    // Not this plan's grid, or damaged: baked again below.
                }
            }
            // What is left of a grid the stop cut short (written, its report not yet): baked again.
            fs::remove(job.path);
            fs::remove(job.path.string() + ".txt");
        }
        output << "Baking " << job.path.filename().string() << '\n' << std::flush;
        const auto volume = baker.bake(
            job.grid,
            [&](std::size_t baked, std::size_t) { report_done(done + baked); },
            options.threads
        );
        done += job.grid.probe_count();
        report_done(done);
        const auto encoded = encode_probes(volume);
        write_bake_file(job.path, encoded);
        {
            RuntimeRounding rounding;
            auto checked = verify_batch_file(job, encoded);

            if (checked.grid.settings_hash
                != probe_bake_settings_hash(checked.grid, options.settings)) {
                throw std::runtime_error("probe batch settings differ under runtime rounding");
            }

            if (coarse) {
                validate_probe_pair(*coarse, checked);
            }

            report_terrain_coverage(level, checked, coarse ? &*coarse : nullptr, output);

            if (!coarse) {
                coarse = std::move(checked);
            }
        }
        auto report =
            describe_probe_bake(level, exported, job.grid, options.settings, options.light_set);
        const auto valid =
            std::count_if(volume.probes.begin(), volume.probes.end(), [](const Probe& probe) {
                return probe.valid;
            });
        const auto relocated =
            std::count_if(volume.probes.begin(), volume.probes.end(), [](const Probe& probe) {
                return probe.offset != Float3{};
            });
        report += std::format(
            "relocated_probes {}\nvalid_probes {}\ninvalid_probes {}\n"
            "bake_seconds {}\nasset_bytes {}\nverified_runtime_rounding 1\n",
            relocated,
            valid,
            volume.probes.size() - static_cast<std::size_t>(valid),
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count(),
            encoded.size()
        );
        write_bake_file(
            job.path.string() + ".txt",
            std::span(reinterpret_cast<const std::uint8_t*>(report.data()), report.size())
        );
        output << "Verified " << job.path.filename().string() << '\n' << std::flush;
    }

    if (probe_input_hash(level.dir) != options.source_snapshot) {
        throw std::runtime_error(
            "source level changed during the bake; no manifest was registered"
        );
    }

    output << "Probe batch complete\n" << std::flush;
}

}  // namespace openrac::lighting
