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

#include "enhanced/tools/probes/activation.h"
#include "enhanced/tools/probes/level_scene.h"
#include "viewer/json.h"

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

// Geometry queries only: this does not start the game or render images. Use
// authored normals/positions to expose coverage hidden by fallback. These are
// view-independent samples, without the fragment shader's camera-facing flip.
void report_terrain_coverage(
    const viewer::LevelData& level, const ProbeVolume& volume, const ProbeVolume* coarse = nullptr
) {
    auto unclassified = volume;
    for (auto& probe : unclassified.probes) {
        probe.valid = true;
    }
    std::size_t covered = 0, total = 0, opposed = 0, upright = 0, upright_covered = 0;
    std::size_t classification_fallbacks = 0;
    std::size_t coarse_covered = 0, combined_covered = 0, compared = 0;
    double difference_squared = 0, reference_squared = 0;
    for (const auto& instance : level.instances[static_cast<std::size_t>(viewer::Layer::Terrain)]) {
        const auto& m = instance.matrix;
        const Float3 a{m[0], m[1], m[2]}, b{m[4], m[5], m[6]}, c{m[8], m[9], m[10]};
        const auto ca = renderer::cross(b, c), cb = renderer::cross(c, a),
                   cc = renderer::cross(a, b);
        const float determinant = renderer::dot(a, ca);
        for (const auto& primitive : level.models.at(instance.model).primitives) {
            for (std::size_t i = 0; i + 2 < primitive.index_count; i += 3) {
                Float3 position{}, normal{};
                std::array<Float3, 3> points;
                for (unsigned k = 0; k < 3; ++k) {
                    const auto& vertex =
                        level.vertices.at(level.indices.at(primitive.first_index + i + k));
                    points[k] = renderer::transform_point(
                        instance.matrix,
                        {vertex.position[0], vertex.position[1], vertex.position[2]}
                    );
                    for (unsigned axis = 0; axis < 3; ++axis) {
                        position[axis] += points[k][axis] / 3;
                        if (std::abs(determinant) > 1e-12f) {
                            normal[axis] +=
                                (ca[axis] * vertex.normal[0] + cb[axis] * vertex.normal[1]
                                 + cc[axis] * vertex.normal[2])
                                / determinant;
                        }
                    }
                }
                const auto geometric = renderer::cross(
                    Float3{
                        points[1][0] - points[0][0],
                        points[1][1] - points[0][1],
                        points[1][2] - points[0][2]
                    },
                    Float3{
                        points[2][0] - points[0][0],
                        points[2][1] - points[0][1],
                        points[2][2] - points[0][2]
                    }
                );
                if (coarse) {
                    bool inside = true;
                    for (unsigned axis = 0; axis < 3; ++axis) {
                        const float coordinate =
                            (position[axis] - volume.grid.origin[axis]) / volume.grid.spacing[axis];
                        inside =
                            inside && coordinate >= 0
                            && coordinate <= static_cast<float>(volume.grid.dimensions[axis] - 1);
                    }
                    if (!inside) {
                        continue;
                    }
                }
                opposed += renderer::dot(normal, geometric) < 0;
                if (renderer::dot(normal, normal) < 1e-12f) {
                    normal = geometric;
                }
                ++total;
                const bool valid = sample_probes(volume, position, normal).has_value();
                if (!valid && sample_probes(unclassified, position, normal)) {
                    ++classification_fallbacks;
                }
                covered += valid;
                if (coarse) {
                    const auto base = sample_probes(*coarse, position, normal);
                    const auto combined = sample_probe_volumes(*coarse, &volume, position, normal);
                    coarse_covered += base.has_value();
                    combined_covered += combined.has_value();
                    if (base && combined) {
                        ++compared;
                        for (unsigned channel = 0; channel < 3; ++channel) {
                            const double delta =
                                combined->irradiance[channel] - base->irradiance[channel];
                            difference_squared += delta * delta;
                            reference_squared +=
                                double(base->irradiance[channel]) * base->irradiance[channel];
                        }
                    }
                }
                if (normal[2] > 0.5f * std::sqrt(renderer::dot(normal, normal))) {
                    ++upright;
                    upright_covered += valid;
                }
            }
        }
    }
    std::cout << "Terrain centroid coverage " << covered << '/' << total << " ("
              << (total ? 100.0 * static_cast<double>(covered) / static_cast<double>(total) : 0.0)
              << " percent); unweighted geometry samples, not screen coverage\n";
    std::cout << "Upward terrain coverage " << upright_covered << '/' << upright
              << "; winding opposes authored normal on " << opposed << '/' << total
              << " triangles\n";
    std::cout << "Fallbacks attributable to probe classification " << classification_fallbacks
              << "; diagnostic only, invalid probes remain disabled\n";
    if (coarse) {
        std::cout << "Within dense nominal bounds: " << total << " terrain centres; sparse covered "
                  << coarse_covered << "; dense covered " << covered << "; combined covered "
                  << combined_covered << "\nCombined vs sparse relative RMS irradiance difference "
                  << (reference_squared > 0 ? std::sqrt(difference_squared / reference_squared) : 0)
                  << " over " << compared
                  << " shared samples; difference is not a quality metric\n";
    }
}

void write_new(const fs::path& path, std::span<const std::uint8_t> bytes) {
    const fs::path temporary = path.string() + ".tmp";
    if (fs::exists(path) || fs::exists(temporary)) {
        throw std::runtime_error("output or temporary file already exists");
    }
    if (!path.parent_path().empty()) {
        fs::create_directories(path.parent_path());
    }
    std::ofstream file(temporary, std::ios::binary);
    if (!file
        || !file.write(
            reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())
        )) {
        throw std::runtime_error("cannot write probe output");
    }
    file.close();
    if (!file) {
        throw std::runtime_error("cannot close probe output");
    }
    fs::rename(temporary, path);
}
}  // namespace

int main(int argc, char** argv) {
    try {
        fs::path directory, output, verify, compare_coarse;
        bool inspect = false, game_rounding = false, print_profile = false;
        // Resolve the directory first: explicit CLI overrides win in any order.
        for (int i = 1; i + 1 < argc; ++i) {
            if (std::string_view(argv[i]) == "--level-dir") {
                directory = argv[++i];
            }
        }
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
        if (directory.empty() || (!inspect && !print_profile && output.empty() && verify.empty())) {
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
        if (!compare_coarse.empty() && verify.empty()) {
            throw std::runtime_error("--compare-coarse requires --verify LOCAL_FILE");
        }
        if (!output.empty()
            && (fs::exists(output) || fs::exists(output.string() + ".txt")
                || fs::exists(output.string() + ".tmp"))) {
            throw std::runtime_error("output exists; choose a new bake path");
        }
        viewer::json::Value manifest;
        std::string error;
        if (!viewer::json::parse_file((directory / "manifest.json").string(), manifest, error)) {
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
                report_terrain_coverage(level, volume);
            } else {
                const auto coarse = load_level_probes(compare_coarse, level, expected, true);
                validate_probe_pair(coarse, volume);
                report_terrain_coverage(level, volume, &coarse);
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
        std::array<std::size_t, 2> emission_masks{};
        std::size_t vertex_emitters = 0;
        for (const auto& triangle : exported.scene.triangles) {
            if (*std::max_element(triangle.emission.begin(), triangle.emission.end()) <= 0) {
                continue;
            }
            ++emission_masks.at(static_cast<unsigned>(triangle.emission_mask));
            vertex_emitters += triangle.emission_vertex_colour ? 1 : 0;
        }
        std::ostringstream report;
        report << "probe_format_version " << kProbeFormatVersion << "\nbake_version "
               << grid.bake_version << "\ngame_version " << grid.game_version << "\nlevel "
               << grid.level << "\nstatic_instances terrain " << exported.instances[0] << " ties "
               << exported.instances[1] << " shrubs " << exported.instances[2] << " mobys "
               << exported.instances[3] << "\nskipped_mobys " << exported.skipped_mobys
               << "\nskipped_glow_triangles " << exported.skipped_glow << "\ntriangles "
               << exported.scene.triangles.size() << "\nemissive_triangles "
               << exported.emissive_triangles << "\nbake_surfaces " << exported.surface_instances
               << "\ntextures " << exported.scene.textures.size() << "\nskipped_blended_primitives "
               << exported.skipped_blended << "\nautomatic_liquid_triangles "
               << exported.automatic_liquid_triangles << "\nskipped_degenerate_triangles "
               << exported.skipped_degenerate << "\nlight_set " << light_set << "\nanalytic_lights "
               << exported.scene.suns.size() << "\nenvironment_linear_radiance " << environment[0]
               << ' ' << environment[1] << ' ' << environment[2] << "\nenvironment_source "
               << (settings.use_level_sky && !exported.scene.sky_radiance.empty()
                       ? "level sky panorama"
                       : "uniform fallback")
               << "\nsky_resolution " << exported.scene.sky_width << ' '
               << exported.scene.sky_height << "\nsky_intensity " << settings.sky_intensity
               << "\npoint_emitters " << exported.scene.point_emitters.size()
               << "\nemission_masks none " << emission_masks[0] << " green " << emission_masks[1]
               << "\nemission_vertex_colour_triangles " << vertex_emitters << "\nsky_model "
               << (1 - settings.sky_weight) << " * primary directional colour + "
               << settings.sky_weight << " * sky_intensity * sky"
               << "\nprimary_directional_colour " << exported.scene.primary_directional_colour[0]
               << ' ' << exported.scene.primary_directional_colour[1] << ' '
               << exported.scene.primary_directional_colour[2]
               << "\ntexture_colour sRGB decoded to linear; original vertex lighting excluded"
               << "\nemission two-sided linear radiance; area/power next-event sampling at probes "
                  "and bounces"
               << "\nprobe_origin " << grid.origin[0] << ' ' << grid.origin[1] << ' '
               << grid.origin[2] << "\nprobe_dimensions " << grid.dimensions[0] << ' '
               << grid.dimensions[1] << ' ' << grid.dimensions[2] << "\nprobe_spacing "
               << grid.spacing[0] << ' ' << grid.spacing[1] << ' ' << grid.spacing[2]
               << "\nprobe_count " << grid.probe_count() << "\nmap_resolution " << resolution
               << "\nmaximum_moment_distance " << grid.max_distance << "\nrays_per_probe "
               << settings.rays_per_probe << "\nsurface_bounces " << settings.bounces << "\nseed "
               << settings.seed << "\nscene_hash " << grid.scene_hash << "\nsettings_hash "
               << grid.settings_hash << "\nmap_and_metadata_bytes "
               << grid.probe_count() * (16 + 20 * grid.texels_per_probe()) << '\n';
        report << "relocation_iterations " << settings.relocation_iterations
               << "\nrelocation_max_spacing_fraction " << settings.relocation_max_fraction
               << "\nplacement paired ray-crossing parity; closed shells treated as solids"
               << "\nminimum_inside_fraction " << settings.min_inside_fraction << '\n';
        // Resolved shared definitions, rather than a second material classifier.
        // Keep the actual colours/masks in the local bake audit alongside counts.
        for (std::size_t i = 0; i < level.materials.size(); ++i) {
            const auto& material = level.materials[i];
            if (*std::max_element(material.emission.begin(), material.emission.end()) <= 0) {
                continue;
            }
            report << "emission_material " << i << " linear_rgb " << material.emission[0] << ' '
                   << material.emission[1] << ' ' << material.emission[2] << " mask "
                   << (material.emission_mask == EmissionMask::Green ? "green" : "none")
                   << " vertex_colour " << material.emission_vertex_colour << " diffuse_scale "
                   << material.diffuse_scale << " texture ";
            if (material.emission_image >= 0) {
                report << std::quoted(
                    level.image_paths.at(static_cast<std::size_t>(material.emission_image))
                );
            } else {
                report << "implicit-white";
            }
            report << '\n';
        }
        std::cout << report.str() << std::flush;
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
        write_new(output, encoded);
        const auto text = report.str();
        write_new(
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
