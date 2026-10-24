// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/tools/probes/activation.h"

#include <cfenv>
#include <cmath>
#include <fstream>
#include <stdexcept>

#include "common/json.h"

namespace openrac::lighting {
namespace {
void apply_profile(const std::filesystem::path& path, LevelBakeProfile& profile) {
    if (!std::filesystem::exists(path)) {
        return;
    }
    json::Value root;
    std::string error;
    const auto fail = [&](const std::string& detail) {
        throw std::runtime_error(path.string() + ": " + detail);
    };
    if (!json::parse_file(path.string(), root, error)) {
        fail(error);
    }
    if (!root.is_object() || !root["version"].is_number() || root["version"].number() != 1) {
        fail("expected profile object with version 1");
    }
    auto number = [&](const json::Value& value, double minimum, double maximum) {
        const auto n = value.number();
        if (!value.is_number() || !std::isfinite(n) || n < minimum || n > maximum) {
            fail("number outside permitted range");
        }
        return static_cast<float>(n);
    };
    auto integer = [&](const json::Value& value, unsigned minimum, unsigned maximum) {
        const auto n = value.number();
        if (!value.is_number() || !std::isfinite(n) || std::floor(n) != n || n < minimum
            || n > maximum) {
            fail("expected integer in permitted range");
        }
        return static_cast<unsigned>(n);
    };
    auto vector = [&](const json::Value& value, float minimum) {
        if (!value.is_array() || value.items().size() != 3) {
            fail("expected three coordinates");
        }
        Float3 result{};
        for (unsigned i = 0; i < 3; ++i) {
            result[i] = number(value[i], minimum, 1e6);
        }
        return result;
    };
    auto& activation = profile.activation;
    auto& bake = activation.bake;
    for (const auto& [key, value] : root.members()) {
        if (key == "version") {
            continue;
        }
        if (key == "sky") {
            if (value.string() != "level" && value.string() != "uniform") {
                fail("sky must be level or uniform");
            }
            bake.use_level_sky = value.string() == "level";
        } else if (key == "sky_intensity") {
            bake.sky_intensity = number(value, 0, 1e6);
        } else if (key == "sky_weight") {
            bake.sky_weight = number(value, 0, 1);
        } else if (key == "sunlight_scale") {
            bake.sunlight_scale = vector(value, 0);
            for (float v : bake.sunlight_scale) {
                if (v > 4) {
                    fail("sunlight_scale must not exceed 4");
                }
            }
        } else if (key == "terrain_vertex_albedo") {
            if (value.type() != json::Value::Type::Bool) {
                fail("terrain_vertex_albedo requires a boolean");
            }
            bake.terrain_vertex_albedo = value.boolean();
        } else if (key == "rays") {
            bake.rays_per_probe = integer(value, 16, 1048576);
        } else if (key == "bounces") {
            bake.bounces = integer(value, 0, 32);
        } else if (key == "seed") {
            bake.seed = integer(value, 0, UINT32_MAX);
        } else if (key == "light_set") {
            activation.light_set = integer(value, 0, 255);
        } else if (key == "environment") {
            activation.environment = vector(value, 0);
        } else if (key == "sparse_spacing") {
            profile.sparse_spacing = vector(value, 0.001f);
        } else if (key == "dense_spacing") {
            profile.dense_spacing = vector(value, 0.001f);
        } else if (key == "resolution") {
            profile.map_resolution = integer(value, 2, 64);
        } else if (key == "ray_bias") {
            bake.ray_bias = number(value, 0.000001, 1e6);
        } else if (key == "distance_exponent") {
            bake.distance_exponent = number(value, 1, 256);
        } else if (key == "min_inside_fraction") {
            bake.min_inside_fraction = number(value, 0.500001, 1);
        } else if (key == "relocation_iterations") {
            bake.relocation_iterations = integer(value, 0, 32);
        } else if (key == "relocation_max_fraction") {
            bake.relocation_max_fraction = number(value, 0, 0.45);
        } else {
            fail("unknown setting: " + key);
        }
    }
}
}  // namespace

LevelBakeProfile load_level_bake_profile(
    const std::filesystem::path& directory, const std::filesystem::path& overrides
) {
    // The game runs with FE_TOWARDZERO. Canonical profile values must match the
    // offline baker, including the float bits used by the settings fingerprint.
    struct Rounding {
        int previous = std::fegetround();

        Rounding() { std::fesetround(FE_TONEAREST); }

        ~Rounding() { std::fesetround(previous); }
    } rounding;

    LevelBakeProfile profile;
    apply_profile(directory / "probe-bake.json", profile);
    if (!overrides.empty()) {
        apply_profile(overrides / "probe-bake.json", profile);
    }
    return profile;
}

ProbeVolume load_level_probes(
    const std::filesystem::path& path,
    const scene::LevelData& level,
    const ProbeActivation& expected,
    bool expand_linear_maps
) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        throw std::runtime_error("cannot open probe asset");
    }
    const auto size = input.tellg();
    if (size <= 0 || size > 512 * 1024 * 1024) {
        throw std::runtime_error("unsupported probe asset size");
    }
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    input.seekg(0);
    if (!input.read(reinterpret_cast<char*>(bytes.data()), size)) {
        throw std::runtime_error("cannot read probe asset");
    }
    auto volume = expand_linear_maps ? decode_probes(bytes) : decode_gpu_probes(std::move(bytes));
    json::Value manifest;
    std::string error;
    if (!json::parse_file((level.dir / "manifest.json").string(), manifest, error)) {
        throw std::runtime_error(error);
    }
    const auto version = manifest["game"].string() + "-" + manifest["version"].string();
    if ((version != "rac1-pal" && version != "rac1-ntsc") || volume.grid.game_version != version
        || level.level < 0 || volume.grid.level != static_cast<std::uint32_t>(level.level)) {
        throw std::runtime_error("probe game version or level does not match loaded scene");
    }
    if (volume.grid.settings_hash != probe_bake_settings_hash(volume.grid, expected.bake)) {
        throw std::runtime_error(
            "probe bake settings differ; check OPENRAC_PROBE_RAYS and rebake if needed"
        );
    }
    const auto exported =
        export_probe_scene(level, expected.light_set, expected.environment, false);
    if (volume.grid.scene_hash != exported.scene_hash) {
        throw std::runtime_error("stale probe scene or different lighting inputs; rebake this level"
        );
    }
    return volume;
}
}  // namespace openrac::lighting
