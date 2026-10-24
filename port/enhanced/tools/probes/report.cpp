// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Coverage diagnostics and descriptions shared by offline bake commands.
#include "enhanced/tools/probes/report.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace openrac::lighting {
// Geometry queries only: this does not start the game or render images. Use
// authored normals/positions to expose coverage hidden by fallback. These are
// view-independent samples, without the fragment shader's camera-facing flip.
void report_terrain_coverage(
    const viewer::LevelData& level,
    const ProbeVolume& volume,
    const ProbeVolume* coarse,
    std::ostream& output
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

    output << "Terrain centroid coverage " << covered << '/' << total << " ("
           << (total ? 100.0 * static_cast<double>(covered) / static_cast<double>(total) : 0.0)
           << " percent); unweighted geometry samples, not screen coverage\n";
    output << "Upward terrain coverage " << upright_covered << '/' << upright
           << "; winding opposes authored normal on " << opposed << '/' << total << " triangles\n";
    output << "Fallbacks attributable to probe classification " << classification_fallbacks
           << "; diagnostic only, invalid probes remain disabled\n";

    if (coarse) {
        output << "Within dense nominal bounds: " << total << " terrain centres; sparse covered "
               << coarse_covered << "; dense covered " << covered << "; combined covered "
               << combined_covered << "\nCombined vs sparse relative RMS irradiance difference "
               << (reference_squared > 0 ? std::sqrt(difference_squared / reference_squared) : 0)
               << " over " << compared << " shared samples; difference is not a quality metric\n";
    }
}

void write_bake_file(const fs::path& path, std::span<const std::uint8_t> bytes) {
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

// Describe the same transport inputs for standalone and persistent bakes.
std::string describe_probe_bake(
    const viewer::LevelData& level,
    const BakeSceneExport& exported,
    const ProbeGrid& grid,
    const BakeSettings& settings,
    unsigned light_set
) {
    const auto& environment = exported.scene.environment;
    const unsigned resolution = grid.map_resolution;
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
           << (settings.use_level_sky && !exported.scene.sky_radiance.empty() ? "level sky panorama"
                                                                              : "uniform fallback")
           << "\nsky_resolution " << exported.scene.sky_width << ' ' << exported.scene.sky_height
           << "\nsky_intensity " << settings.sky_intensity << "\npoint_emitters "
           << exported.scene.point_emitters.size() << "\nemission_masks none " << emission_masks[0]
           << " green " << emission_masks[1] << "\nemission_vertex_colour_triangles "
           << vertex_emitters << "\nsky_model " << (1 - settings.sky_weight)
           << " * primary directional colour + " << settings.sky_weight << " * sky_intensity * sky"
           << "\nprimary_directional_colour " << exported.scene.primary_directional_colour[0] << ' '
           << exported.scene.primary_directional_colour[1] << ' '
           << exported.scene.primary_directional_colour[2]
           << "\ntexture_colour sRGB decoded to linear; original vertex lighting excluded"
           << "\nemission two-sided linear radiance; area/power next-event sampling at probes "
              "and bounces"
           << "\nprobe_origin " << grid.origin[0] << ' ' << grid.origin[1] << ' ' << grid.origin[2]
           << "\nprobe_dimensions " << grid.dimensions[0] << ' ' << grid.dimensions[1] << ' '
           << grid.dimensions[2] << "\nprobe_spacing " << grid.spacing[0] << ' ' << grid.spacing[1]
           << ' ' << grid.spacing[2] << "\nprobe_count " << grid.probe_count()
           << "\nmap_resolution " << resolution << "\nmaximum_moment_distance " << grid.max_distance
           << "\ntexel_samples_per_axis " << kProbeTexelSamplesPerAxis << "\nrays_per_probe "
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

    return report.str();
}

}  // namespace openrac::lighting
