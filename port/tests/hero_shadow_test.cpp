// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <cstring>
#include <numbers>

#include "enhanced/tests/gpu_probe_fixture.h"
#include "enhanced/tests/probe_fixture.h"
#include "renderer/gl.h"
#include "scene/hero_shadow.h"
#include "scene/level_renderer.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

using namespace openrac;
using namespace openrac::gl;

void cpu() {
    std::vector<std::uint8_t> ram(0x1000);
    CHECK(!scene::hero_airborne(ram, 0));
    CHECK(!scene::hero_airborne(ram, 0xf00));
    CHECK(scene::hero_airborne(ram, 0x100) == false);
    // A special ground-moby hit must not select the airborne projection.
    ram[0x100 + 0x30a] = 1;
    CHECK(scene::hero_airborne(ram, 0x100) == false);
    ram[0x100 + 0x30a] = 0;
    ram[0x100 + 0x30e] = 22;
    CHECK(scene::hero_airborne(ram, 0x100) == true);
    // Zero vertical velocity at the apex must remain airborne.
    std::fill(ram.begin() + 0x100 + 0xe0, ram.begin() + 0x100 + 0xf0, 0);
    CHECK(scene::hero_airborne(ram, 0x100) == true);
    ram[0x100 + 0x30e] = 0;
    CHECK(scene::hero_airborne(ram, 0x100) == false);  // landing
    CHECK(!scene::hero_airborne(std::span(ram).first(0x40f), 0x100));
    const auto ground = scene::hero_shadow_projection({0, 0, 2}, {1, 0, -1}, false);
    const auto air = scene::hero_shadow_projection({0, 0, 2}, {1, 0, -1}, true);
    CHECK(ground.direction[0] > 0.7f && ground.direction[2] < -0.7f);
    CHECK((air.direction == renderer::Vec3{0, 0, -1}));
    const auto projected = renderer::transform_point(ground.matrix, {2, 0, 0});
    const auto caster = renderer::transform_point(ground.matrix, {0, 0, 2});
    CHECK(std::abs(projected[0] - caster[0]) < 1e-5f);
    CHECK(std::abs(projected[1] - caster[1]) < 1e-5f);
    CHECK(caster[2] > projected[2]);
    const auto vertical = renderer::transform_point(air.matrix, {0, 0, 0});
    CHECK(std::abs(vertical[0]) < 1e-5f && std::abs(vertical[1]) < 1e-5f);
}

void gpu(bool perspective = false) {
    scene::LevelData level;
    level.light_sets.resize(256);
    level.light_sets[0] = level.light_sets[1] = level.light_sets[2] = 1;
    level.light_sets[4] = 1;
    level.light_sets[6] = -1;
    const renderer::Vec3 points[] = {
        {-4, -4, 0}, {4, -4, 0}, {4, 4, 0}, {-4, 4, 0}, {-.5f, -.5f, 2}, {.5f, -.5f, 2}, {0, .5f, 2}
    };
    for (const auto& p : points) {
        scene::Vertex vertex{};
        std::copy(p.begin(), p.end(), vertex.position);
        vertex.normal[2] = 1;
        for (float& c : vertex.colour) {
            c = 1;
        }
        vertex.light_slot = -1;
        vertex.weights[0] = 1;
        level.vertices.push_back(vertex);
    }
    level.indices = {0, 1, 2, 0, 2, 3, 4, 5, 6};
    level.materials.push_back({});
    scene::Model plane, hero_model;
    plane.primitives.push_back({0, 6, 0});
    hero_model.primitives.push_back({6, 3, 0});
    hero_model.joints = 1;
    level.models = {plane, hero_model};
    level.instances[static_cast<std::size_t>(scene::Layer::Terrain)].push_back({});
    scene::Instance hero, other;
    hero.model = other.model = 1;
    hero.hero_shadow_caster = true;
    hero.moby_light[3] = 1;
    other.matrix = renderer::translation({-2, 0, 0});
    scene::LevelScene scene;
    renderer::Renderer renderer;
    std::string error;
    CHECK(scene.upload(level, renderer.textures(), error));
    scene.set_instances(scene::Layer::Mobys, {hero, other});
    lighting::ProbeGrid grid;
    grid.origin = {-8, -8, -8};
    grid.spacing = {16, 16, 16};
    auto probes = lighting::ProbeVolume::allocate(grid);
    for (auto& p : probes.probes) {
        p.valid = true;
    }
    for (auto& e : probes.irradiance) {
        e.fill(0.2f * std::numbers::pi_v<float>);
    }
    CHECK(scene.set_probe_volume(openrac::test::gpu_probes(probes), error));
    CHECK(test::shadow_probe_fixture(scene, error));
    CHECK(scene.enable_hero_shadow(true, error));
    CHECK(
        renderer.add(std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Terrain), error)
    );
    CHECK(renderer.init(error));
    renderer::FrameBuffer frame;
    CHECK(frame.create(64, 64, error));
    renderer::FrameInput input;
    input.camera.position = {0, 0, 4};
    input.camera.view = renderer::look_along(input.camera.position, {0, 0, -1}, {0, 1, 0});
    if (perspective) {
        input.camera.projection =
            renderer::perspective_reversed(std::numbers::pi_v<float> / 2, 1, .1f, 72);
        CHECK(scene.needs_early_lighting(input));
    } else {
        input.camera.projection = renderer::identity();
        input.camera.projection[0] = input.camera.projection[5] = .25f;
        input.camera.projection[10] = 2.f / 72;
        input.camera.projection[14] = 1;
    }
    auto pixel = [&](int x, const std::vector<std::uint8_t>& rgba) {
        return rgba[(32 * 64 + x) * 4];
    };
    renderer.render(input, {frame.id(), 64, 64});
    const auto original = frame.read_rgba();
    input.hero_pixel_shadow = true;
    renderer.render(input, {frame.id(), 64, 64});
    const auto grounded = frame.read_rgba();
    CHECK(pixel(48, grounded) + 40 < pixel(48, original));  // diagonal sunlight projection
    CHECK(
        std::abs(int(pixel(16, grounded)) - int(pixel(16, original))) < 3
    );  // other caster excluded
    input.hero_airborne = true;
    renderer.render(input, {frame.id(), 64, 64});
    auto airborne = frame.read_rgba();
    CHECK(pixel(32, airborne) + 10 < pixel(32, original));    // directly under Ratchet
    CHECK(pixel(32, airborne) > pixel(32, original) * 0.7f);  // bounded platforming cue
    CHECK(std::abs(int(pixel(48, airborne)) - int(pixel(48, original))) < 3);
    CHECK(std::abs(int(pixel(16, airborne)) - int(pixel(16, original))) < 3);
    // The depth pass must use the same skinned pose as the visible hero.
    hero.palette = 0;
    scene.set_palette({renderer::translation({1, 0, 0})});
    scene.set_instances(scene::Layer::Mobys, {hero, other});
    renderer.render(input, {frame.id(), 64, 64});
    airborne = frame.read_rgba();
    CHECK(pixel(40, airborne) + 10 < pixel(40, original));
    CHECK(std::abs(int(pixel(32, airborne)) - int(pixel(32, original))) < 3);
    // A world caster covers the same receiver as Ratchet. His separate map
    // should make that footprint 15% darker in linear light, even in shade.
    hero.palette = -1;
    auto blocker = other;
    blocker.matrix = renderer::translation({-4, 0, 4});
    scene.set_instances(scene::Layer::Mobys, {hero, blocker});
    CHECK(test::shadow_probe_fixture(scene, error));
    CHECK(scene.enable_sun_shadow(true, error));
    input.sun_pixel_shadow = true;
    input.hero_airborne = false;
    input.hero_pixel_shadow = false;
    renderer.render(input, {frame.id(), 64, 64});
    const auto world_shadow = frame.read_rgba();
    input.hero_pixel_shadow = true;
    renderer.render(input, {frame.id(), 64, 64});
    const auto overlap = frame.read_rgba();
    const auto linear = [](std::uint8_t byte) {
        const float v = byte / 255.f;
        return v <= .04045f ? v / 12.92f : std::pow((v + .055f) / 1.055f, 2.4f);
    };
    const float ratio = linear(pixel(48, overlap)) / linear(pixel(48, world_shadow));
    CHECK(ratio > .82f && ratio < .88f);
    CHECK(std::abs(int(pixel(16, overlap)) - int(pixel(16, world_shadow))) < 3);
    CHECK(scene.enable_sun_shadow(false, error));
    input.sun_pixel_shadow = false;
    input.hero_airborne = true;
    // Transparent caster texels must leave the map clear, not cast a solid
    // rectangle. Reuse the scene address so the existing receiver remains valid.
    scene.release();
    renderer::Rgba8Image transparent(1, 1);
    transparent.set_texel(0, 0, 0x00ffffffu);
    level.images.push_back(transparent);
    level.image_paths.push_back("fixture/transparent.png");
    scene::Material alpha;
    alpha.image = 0;
    alpha.cutout = true;
    level.materials.push_back(alpha);
    level.models[1].primitives[0].material = 1;
    hero.palette = -1;
    CHECK(scene.upload(level, renderer.textures(), error));
    scene.set_instances(scene::Layer::Mobys, {hero, other});
    CHECK(scene.set_probe_volume(openrac::test::gpu_probes(probes), error));
    CHECK(test::shadow_probe_fixture(scene, error));
    CHECK(scene.enable_hero_shadow(true, error));
    renderer.render(input, {frame.id(), 64, 64});
    const auto transparent_shadow = frame.read_rgba();
    CHECK(std::abs(int(pixel(32, transparent_shadow)) - int(pixel(32, original))) < 3);
    CHECK(glGetError() == GL_NO_ERROR);
    renderer.clear_renderers();
    scene.release();
    frame.release();
}

int main(int argc, char**) {
    cpu();
    if (argc > 1) {
        test::GlContext context;
        std::string why;
        if (!context.open(why)) {
            std::printf("SKIP: %s\n", why.c_str());
            return test::kSkip;
        }
        gpu();
        gpu(true);
    }
    return test::result();
}
