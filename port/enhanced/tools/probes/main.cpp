// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cfenv>
#include <charconv>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "common/json.h"
#include "enhanced/tools/probes/activation.h"
#include "enhanced/tools/probes/batch.h"
#include "enhanced/tools/probes/level_scene.h"

namespace fs = std::filesystem;
using namespace openrac;
using namespace openrac::lighting;

namespace {
unsigned integer(std::string_view text) {
    unsigned out = 0;
    auto result = std::from_chars(text.data(), text.data() + text.size(), out);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
        throw std::runtime_error("invalid integer option");
    }
    return out;
}

float real(const std::string& text) {
    std::size_t end = 0;
    const float value = std::stof(text, &end);
    if (end != text.size() || !std::isfinite(value)) {
        throw std::runtime_error("invalid float option");
    }
    return value;
}

void usage() {
    std::cout << "Offline Embree sparse probe bake (no game window)\n"
              << "openrac-probe-bake --level-dir DIR [--inspect | --output FILE | --verify FILE]\n"
              << "  --print-profile (resolved grid/quality defaults as JSON, no scene load)\n"
              << "  --batch-output DIR (prepare once, then read one JSON region plan on stdin)\n"
              << "  --spacing 128 (or X,Y,Z) --resolution 6 --rays 1024 --bounces 3 --seed 1\n"
              << "  --threads 0 (all CPU threads; 1 for serial baking, same quality)\n"
              << "  --light-set 0 --environment 0.1 (uniform fallback radiance)\n"
              << "  --sky level|uniform --sky-intensity 1 (linear radiance multiplier)\n"
              << "  --bounds minX,minY,minZ,maxX,maxY,maxZ (optional local volume)\n"
              << "  --max-distance N (visibility range; default 1.5 times cell diagonal)\n"
              << "  --compare-coarse FILE (with --verify LOCAL_FILE)\n"
              << "Defaults: per-level probe-bake.json; explicit options override its settings.\n"
              << "Existing output files are never overwritten. Bakes stay local.\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        fs::path directory, output, verify, compare_coarse, batch_output;
        bool inspect = false, game_rounding = false, print_profile = false;
        // Resolve the directory first: explicit CLI overrides win in any order.
        for (int i = 1; i + 1 < argc; ++i) {
            if (std::string_view(argv[i]) == "--level-dir") {
                directory = argv[++i];
            } else if (std::string_view(argv[i]) == "--batch-output") {
                batch_output = argv[++i];
            }
        }
        const auto source_snapshot =
            batch_output.empty() || directory.empty() ? 0 : probe_input_hash(directory);
        const auto profile =
            directory.empty() ? LevelBakeProfile{} : load_level_bake_profile(directory);
        Float3 spacing = profile.sparse_spacing;
        Float3 environment = profile.activation.environment;
        unsigned resolution = profile.map_resolution, light_set = profile.activation.light_set;
        BakeSettings settings = profile.activation.bake;
        unsigned threads = 0;
        std::optional<std::array<float, 6>> bounds;
        std::optional<float> max_distance;
        for (int i = 1; i < argc; ++i) {
            const std::string_view argument = argv[i];
            if (argument == "--help") {
                usage();
                return 0;
            }
            if (argument == "--print-profile") {
                print_profile = true;
                continue;
            }
            if (argument == "--inspect") {
                inspect = true;
                continue;
            }
            if (argument == "--game-rounding") {
                game_rounding = true;
                continue;
            }
            if (i + 1 >= argc) {
                throw std::runtime_error("option needs a value");
            }
            const std::string value = argv[++i];
            if (argument == "--level-dir") {
                directory = value;
            } else if (argument == "--output") {
                output = value;
            } else if (argument == "--batch-output") {
                batch_output = value;
            } else if (argument == "--verify") {
                verify = value;
            } else if (argument == "--compare-coarse") {
                compare_coarse = value;
            } else if (argument == "--max-distance") {
                max_distance = real(value);
            } else if (argument == "--spacing") {
                if (value.find(',') == std::string::npos) {
                    spacing.fill(real(value));
                } else {
                    std::istringstream stream(value);
                    std::string component;
                    for (float& n : spacing) {
                        if (!std::getline(stream, component, ',')) {
                            throw std::runtime_error("spacing needs X,Y,Z");
                        }
                        n = real(component);
                    }
                    if (std::getline(stream, component, ',') || value.back() == ',') {
                        throw std::runtime_error("spacing needs exactly three coordinates");
                    }
                }
            } else if (argument == "--bounds") {
                std::array<float, 6> parsed{};
                std::istringstream stream(value);
                std::string component;
                for (float& n : parsed) {
                    if (!std::getline(stream, component, ',')) {
                        throw std::runtime_error("bounds needs six comma-separated coordinates");
                    }
                    n = real(component);
                }
                if (std::getline(stream, component, ',')) {
                    throw std::runtime_error("bounds needs exactly six coordinates");
                }
                bounds = parsed;
            } else if (argument == "--environment") {
                environment.fill(real(value));
            } else if (argument == "--sky") {
                if (value != "level" && value != "uniform") {
                    throw std::runtime_error("sky must be level or uniform");
                }
                settings.use_level_sky = value == "level";
            } else if (argument == "--sky-intensity") {
                settings.sky_intensity = real(value);
                if (settings.sky_intensity < 0) {
                    throw std::runtime_error("negative sky intensity");
                }
            } else if (argument == "--resolution") {
                resolution = integer(value);
            } else if (argument == "--light-set") {
                light_set = integer(value);
            } else if (argument == "--rays") {
                settings.rays_per_probe = integer(value);
            } else if (argument == "--bounces") {
                settings.bounces = integer(value);
            } else if (argument == "--threads") {
                threads = integer(value);
            } else if (argument == "--seed") {
                settings.seed = integer(value);
            } else {
                throw std::runtime_error("unknown bake option");
            }
        }
        if (directory.empty()
            || (!inspect && !print_profile && output.empty() && verify.empty()
                && batch_output.empty())) {
            usage();
            return 1;
        }
        if (print_profile) {
            // Machine-readable defaults for region planners: one C++ resolver
            // owns schema validation and default values for all consumers.
            std::cout << std::setprecision(9)
                      << "{\"version\":1,\"sky_intensity\":" << settings.sky_intensity
                      << ",\"sky_weight\":" << settings.sky_weight << ",\"sunlight_scale\":["
                      << settings.sunlight_scale[0] << ',' << settings.sunlight_scale[1] << ','
                      << settings.sunlight_scale[2] << ']' << ",\"terrain_vertex_albedo\":"
                      << (settings.terrain_vertex_albedo ? "true" : "false")
                      << ",\"probe_format_version\":" << kProbeFormatVersion
                      << ",\"bake_version\":" << kProbeBakeVersion << ",\"sparse_spacing\":["
                      << spacing[0] << ',' << spacing[1] << ',' << spacing[2]
                      << "],\"dense_spacing\":[" << profile.dense_spacing[0] << ','
                      << profile.dense_spacing[1] << ',' << profile.dense_spacing[2]
                      << "],\"resolution\":" << resolution
                      << ",\"rays\":" << settings.rays_per_probe
                      << ",\"bounces\":" << settings.bounces << ",\"sky\":\""
                      << (settings.use_level_sky ? "level" : "uniform") << "\""
                      << ",\"light_set\":" << light_set << ",\"environment\":[" << environment[0]
                      << ',' << environment[1] << ',' << environment[2] << ']'
                      << ",\"seed\":" << settings.seed << ",\"ray_bias\":" << settings.ray_bias
                      << ",\"distance_exponent\":" << settings.distance_exponent
                      << ",\"min_inside_fraction\":" << settings.min_inside_fraction
                      << ",\"relocation_iterations\":" << settings.relocation_iterations
                      << ",\"relocation_max_fraction\":" << settings.relocation_max_fraction
                      << "}\n";
            return 0;
        }
        if (!batch_output.empty()
            && (inspect || !verify.empty() || !output.empty() || bounds || max_distance
                || !compare_coarse.empty() || game_rounding)) {
            throw std::runtime_error("batch mode cannot be combined with a single-grid operation");
        }
        if (!compare_coarse.empty() && verify.empty()) {
            throw std::runtime_error("--compare-coarse requires --verify LOCAL_FILE");
        }
        if (!output.empty()
            && (fs::exists(output) || fs::exists(output.string() + ".txt")
                || fs::exists(output.string() + ".tmp"))) {
            throw std::runtime_error("output exists; choose a new bake path");
        }
        json::Value manifest;
        std::string error;
        if (!json::parse_file((directory / "manifest.json").string(), manifest, error)) {
            throw std::runtime_error(error);
        }
        const std::string game = manifest["game"].string(), version = manifest["version"].string();
        if (game != "rac1" || (version != "pal" && version != "ntsc")) {
            throw std::runtime_error("expected a RAC1 PAL or NTSC export");
        }
        viewer::LevelData level;
        if (game_rounding) {
            std::fesetround(FE_TOWARDZERO);
        }
        std::cout << "Loading extracted level " << directory.string() << std::endl;
        if (!viewer::load_level(directory, level, error)) {
            throw std::runtime_error(error);
        }
        if (level.level < 0) {
            throw std::runtime_error("level identity missing");
        }
        if (!verify.empty()) {
            ProbeActivation expected;
            expected.bake = settings;
            expected.light_set = light_set;
            expected.environment = environment;
            const auto volume = load_level_probes(verify, level, expected, true);
            std::cout << "Probe asset verified: " << volume.grid.game_version << " level "
                      << volume.grid.level << ", " << volume.probes.size()
                      << " probes; scene and settings match\n";
            if (compare_coarse.empty()) {
                report_terrain_coverage(level, volume, nullptr, std::cout);
            } else {
                const auto coarse = load_level_probes(compare_coarse, level, expected, true);
                validate_probe_pair(coarse, volume);
                report_terrain_coverage(level, volume, &coarse, std::cout);
            }
            return 0;
        }
        const auto start = std::chrono::steady_clock::now();
        auto exported = export_probe_scene(level, light_set, environment);
        ProbeGrid grid;
        if (bounds) {
            grid = bounded_probe_grid(
                exported,
                {(*bounds)[0], (*bounds)[1], (*bounds)[2]},
                {(*bounds)[3], (*bounds)[4], (*bounds)[5]},
                spacing,
                resolution
            );
        } else {
            grid = sparse_probe_grid(exported, spacing, resolution);
        }
        if (max_distance) {
            grid.max_distance = *max_distance;
            grid.validate();
        }
        grid.game_version = game + "-" + version;
        grid.level = static_cast<std::uint32_t>(level.level);
        grid.settings_hash = probe_bake_settings_hash(grid, settings);
        std::ostringstream report;
        report << describe_probe_bake(level, exported, grid, settings, light_set);
        std::cout << report.str() << std::flush;
        if (!batch_output.empty()) {
            std::cout << "Probe batch ready\n" << std::flush;
            ProbeBatchOptions options;
            options.output_directory = batch_output;
            options.settings = settings;
            options.light_set = light_set;
            options.threads = threads;
            options.source_snapshot = source_snapshot;
            if (!std::getline(std::cin, options.plan)) {
                throw std::runtime_error("probe batch plan missing");
            }
            run_probe_batch(level, exported, grid, options, std::cout);
            return 0;
        }
        if (inspect) {
            return 0;
        }
        // Free the loader's copies of meshes/textures before building the BVH.
        level = viewer::LevelData{};
        auto last = start;
        const auto volume = bake_probes(
            grid,
            exported.scene,
            settings,
            [&](std::size_t done, std::size_t total) {
                const auto now = std::chrono::steady_clock::now();
                if (done == 0 || done == total || now - last >= std::chrono::seconds(5)) {
                    std::cout << "Baked " << done << '/' << total << " probes\n" << std::flush;
                    last = now;
                }
            },
            threads
        );
        const auto encoded = encode_probes(volume);
        // Check the asset that will be used by the renderer before publishing.
        const auto decoded = decode_gpu_probes(encoded);
        if (!probe_identity_matches(
                decoded, grid.game_version, grid.level, grid.scene_hash, grid.settings_hash
            )) {
            throw std::runtime_error("probe asset round-trip identity mismatch");
        }
        const auto valid =
            std::count_if(volume.probes.begin(), volume.probes.end(), [](const Probe& p) {
                return p.valid;
            });
        const auto relocated =
            std::count_if(volume.probes.begin(), volume.probes.end(), [](const Probe& p) {
                return p.offset != Float3{};
            });
        const auto seconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        report << "relocated_probes " << relocated << "\nvalid_probes " << valid
               << "\ninvalid_probes " << volume.probes.size() - static_cast<std::size_t>(valid)
               << "\nbake_seconds " << seconds << "\nasset_bytes " << encoded.size() << '\n';
        write_bake_file(output, encoded);
        const auto text = report.str();
        write_bake_file(
            output.string() + ".txt",
            std::span(reinterpret_cast<const std::uint8_t*>(text.data()), text.size())
        );
        std::cout << "Saved " << output.string() << " (" << valid << '/' << volume.probes.size()
                  << " valid probes, " << seconds << " s)\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "probe bake: " << error.what() << '\n';
        return 1;
    }
}
