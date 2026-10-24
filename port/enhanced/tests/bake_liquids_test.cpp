// SPDX-License-Identifier: GPL-3.0-or-later
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "enhanced/tools/probes/level_scene.h"
#include "enhanced/tools/probes/liquids.h"
#include "tests/check.h"

using namespace openrac;

namespace {
using Bytes = std::vector<std::uint8_t>;

template <class T>
void put(Bytes& out, std::size_t at, T value) {
    if (out.size() < at + sizeof(T)) {
        out.resize(at + sizeof(T));
    }
    std::memcpy(out.data() + at, &value, sizeof(T));
}

void section(Bytes& overlay, unsigned address, const Bytes& data) {
    const auto at = overlay.size();
    put(overlay, at, address);
    put(overlay, at + 4, unsigned(data.size()));
    put(overlay, at + 8, 1u);
    put(overlay, at + 12, 0u);
    overlay.insert(overlay.end(), data.begin(), data.end());
}
}  // namespace

int main() {
    // Synthetic authored record, separate colour and strip sections. No retail data.
    Bytes record(64), colours(512), order, overlay, index(2048), core(5120);
    put(record, 0, 10.0f);
    put(record, 4, 20.0f);
    put(record, 8, 5.0f);
    put(record, 16, 2.0f);
    put(record, 20, 3.0f);
    put(record, 24, std::uint16_t(7));
    put(record, 26, std::uint16_t(7));
    put(record, 0x34, 0x200000u);
    put(record, 0x3c, std::uint16_t(44));
    put(record, 0x3e, std::uint8_t(16));
    std::vector<std::uint16_t> strip;
    for (unsigned row = 0; row < 7; ++row) {
        if (row) {
            strip.push_back(strip.back());
            strip.push_back(std::uint16_t(row * 64));
        }
        for (unsigned col = 0; col < 8; ++col) {
            strip.push_back(std::uint16_t((row * 8 + col) * 8));
            strip.push_back(std::uint16_t(((row + 1) * 8 + col) * 8));
        }
    }
    CHECK(strip.size() == 124);
    for (unsigned k = 0; k < strip.size(); ++k) {
        put(order, k * 2, strip[k]);
        put(colours, 16 + k * 4, 0x00204080u);
    }
    section(overlay, 0x1fc740, record);
    section(overlay, 0x200000, colours);
    section(overlay, 0x210000, order);
    put(index, 0x58, 60u);
    put(index, 0x5c, 0x100u);
    put(index, 0x68, 0u);
    for (unsigned frame = 44; frame < 60; ++frame) {
        put(index, 0x100 + frame * 16, 0u);
        put(index, 0x104 + frame * 16, 1024u);
        put(index, 0x108 + frame * 16, 64u);
        put(index, 0x10c + frame * 16, 64u);
    }
    put(core, 4, 0x804080ffu);
    std::fill(core.begin() + 1024, core.end(), 1);
    const auto triangles = lighting::gaspar_liquid_triangles(overlay, index, core);
    CHECK(triangles.size() == 98);
    float max_x = 0, max_y = 0;
    for (const auto& triangle : triangles) {
        CHECK((triangle.diffuse == lighting::Float3{}));
        CHECK(triangle.emission_vertex_colour);
        for (unsigned i = 0; i < 3; ++i) {
            const auto& p = triangle.vertices[i];
            CHECK(p[2] == 5);
            max_x = std::max(max_x, p[0]);
            max_y = std::max(max_y, p[1]);
            const auto& e = triangle.emission_colours[i];
            CHECK(std::abs(e[0] - 1) < 1e-5f);
            CHECK(std::abs(e[1] - 128.0f / 255 * .5f) < 1e-5f);
            CHECK(std::abs(e[2] - 64.0f / 255 * .25f) < 1e-5f);
        }
    }
    CHECK(max_x == 24 && max_y == 41);
    auto animated_core = core;
    animated_core.resize(9216);  // second texture is black
    auto animated_index = index;
    for (unsigned frame = 52; frame < 60; ++frame) {
        put(animated_index, 0x104 + frame * 16, 5120u);
    }
    const auto animated = lighting::gaspar_liquid_triangles(overlay, animated_index, animated_core);
    const float half_linear_white = 1.055f * std::pow(.5f, 1 / 2.4f) - .055f;
    CHECK(std::abs(animated[0].emission_colours[0][0] - half_linear_white) < 1e-5f);
    const auto rejects = [&](const Bytes& o, const Bytes& c) {
        bool rejected = false;
        try {
            (void)lighting::gaspar_liquid_triangles(o, index, c);
        } catch (const std::exception&) {
            rejected = true;
        }
        CHECK(rejected);
    };
    rejects(overlay, {});  // truncated palette/texture data
    auto ambiguous = overlay;
    section(ambiguous, 0x220000, order);
    rejects(ambiguous, core);
    auto invalid = overlay;
    invalid.resize(invalid.size() - 1);
    rejects(invalid, core);
    // Runtime identity checks hash raw sources but never analyze them. The
    // emitted triangles exist only in the bake scene and never in draw lists.
    namespace fs = std::filesystem;
    const auto root =
        fs::temp_directory_path()
        / ("openrac-liquid-"
           + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(root / "bake-inputs");
    const auto write = [&](const char* name, const Bytes& data) {
        std::ofstream output(root / "bake-inputs" / name, std::ios::binary);
        output.write(reinterpret_cast<const char*>(data.data()), std::streamsize(data.size()));
    };
    write("overlay.bin", overlay);
    write("core_index.bin", index);
    write("core_data.bin", core);
    {
        std::ofstream manifest(root / "manifest.json");
        manifest << R"({"game":"rac1","version":"ntsc","level":9,
          "bake_inputs":{"liquid":"rac1-ntsc-gaspar-v1"}})";
    }
    scene::LevelData level;
    level.dir = root;
    level.level = 9;
    level.light_sets.resize(256);
    level.light_sets[0] = 1;
    level.light_sets[6] = -1;
    level.materials.push_back({});
    level.vertices.resize(3);
    level.vertices[1].position[0] = 1;
    level.vertices[2].position[1] = 1;
    level.indices = {0, 1, 2};
    scene::Model model;
    model.primitives.push_back({0, 3, 0, -1});
    level.models.push_back(model);
    scene::Instance instance;
    instance.matrix = renderer::identity();
    level.instances[std::size_t(scene::Layer::Terrain)].push_back(instance);
    const auto baked = lighting::export_probe_scene(level, 0, {});
    const auto identity = lighting::export_probe_scene(level, 0, {}, false);
    CHECK(baked.automatic_liquid_triangles == 98 && baked.scene.triangles.size() == 99);
    CHECK(identity.automatic_liquid_triangles == 0 && identity.scene.triangles.size() == 1);
    CHECK(baked.scene_hash == identity.scene_hash);
    CHECK(level.bake_surfaces.empty() && level.vertices.size() == 3);
    write("core_data.bin", {0});  // fingerprinting succeeds without decoding this invalid source
    CHECK(lighting::export_probe_scene(level, 0, {}, false).scene_hash != identity.scene_hash);
    bool invalid_source = false;
    try {
        (void)lighting::export_probe_scene(level, 0, {});
    } catch (const std::exception&) {
        invalid_source = true;
    }
    CHECK(invalid_source);
    // Old prepared emission surfaces take precedence, without a second emitter.
    renderer::Rgba8Image white;
    white.width = 1;
    white.height = 1;
    white.pixels = {255, 255, 255, 255};
    level.images.push_back(white);
    level.image_paths.push_back((root / "textures/probe_gaspar_lava.png").string());
    level.materials[0].image = 0;
    level.bake_surfaces.push_back(instance);
    CHECK(lighting::export_probe_scene(level, 0, {}).automatic_liquid_triangles == 0);
    for (const auto* name : {"overlay.bin", "core_index.bin", "core_data.bin"}) {
        fs::remove(root / "bake-inputs" / name);
    }
    fs::remove(root / "bake-inputs");
    fs::remove(root / "manifest.json");
    fs::remove(root);
    return test::result();
}
