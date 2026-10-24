// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <cstring>
#include <numbers>

#include "enhanced/tests/gpu_probe_fixture.h"
#include "enhanced/tests/probe_fixture.h"
#include "enhanced/viewer/scene_lighting.h"
#include "renderer/gl.h"
#include "scene/hero_shadow.h"
#include "scene/level_renderer.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

using namespace openrac;
using namespace openrac::gl;

// A finite transition reaches both endpoints and reverses continuously at a known intermediate phase.
void test_shadow_direction_transition() {
    scene::ShadowTransition transition;
    CHECK(scene::hero_shadow_blend(false, 10, transition) == 0);
    CHECK(scene::hero_shadow_blend(true, 10, transition) == 0);
    const float halfway = scene::hero_shadow_blend(true, 10.1, transition);
    // Cubic ease-out reaches 1 - (1 - 0.5)^3 = 0.875 halfway through take-off.
    CHECK(std::abs(halfway - .875f) < 1e-5f);
    const auto middle = scene::hero_shadow_projection({0, 0, 2}, {1, 0, -1}, halfway);
    CHECK(middle.direction[0] > 0 && middle.direction[0] < .7f);
    CHECK(middle.direction[2] < -.7f && middle.direction[2] > -1);
    CHECK(scene::hero_shadow_blend(true, 10.2, transition) == 1);
    CHECK(scene::hero_shadow_blend(false, 10.2, transition) == 1);
    const float landing = scene::hero_shadow_blend(false, 10.3, transition);
    CHECK(std::abs(landing - .125f) < 1e-5f);
    CHECK(scene::hero_shadow_blend(true, 10.3, transition) == landing);
    const float reversed = scene::hero_shadow_blend(true, 10.4, transition);
    CHECK(reversed > landing && reversed < 1);
    CHECK(scene::hero_shadow_blend(true, 10.5, transition) == 1);
    CHECK(scene::hero_shadow_blend(false, 20, transition) == 0);
    CHECK(scene::hero_shadow_blend(true, 5, transition) == 1);
}

// Splitting elapsed time into display frames preserves the direction at the same real time.
void test_shadow_transition_frame_rate_independence() {
    scene::ShadowTransition transition;
    CHECK(scene::hero_shadow_blend(false, 1, transition) == 0);
    CHECK(scene::hero_shadow_blend(true, 1, transition) == 0);
    const float single_frame = scene::hero_shadow_blend(true, 1.09, transition);
    transition = {};
    CHECK(scene::hero_shadow_blend(false, 1, transition) == 0);
    CHECK(scene::hero_shadow_blend(true, 1, transition) == 0);
    float many_frames = 0;
    // Nine 10-ms intervals reach the same 90-ms take-off time.
    for (int n = 1; n <= 9; ++n) {
        many_frames = scene::hero_shadow_blend(true, 1 + n * .01, transition);
    }
    CHECK(std::abs(single_frame - many_frames) < 1e-5f);
}

void cpu() {
    std::vector<std::uint8_t> ram(0x1000);
    CHECK(!scene::hero_airborne(ram, 0));
    CHECK(!scene::hero_airborne(ram, 0xf00));
    CHECK(scene::hero_airborne(ram, 0x100) == false);
    // A special ground-moby hit must not select the airborne projection.
    ram[0x100 + 0x30a] = 1;
    CHECK(scene::hero_airborne(ram, 0x100) == false);
    ram[0x100 + 0x30a] = 0;
    // A one- or two-update gap in ground contact must not start a take-off transition.
    ram[0x100 + 0x30e] = 1;
    CHECK(scene::hero_airborne(ram, 0x100) == false);
    ram[0x100 + 0x30e] = 2;
    CHECK(scene::hero_airborne(ram, 0x100) == false);
    ram[0x100 + 0x30e] = 3;
    CHECK(scene::hero_airborne(ram, 0x100) == true);
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
    lighting::use_scene_lighting(scene);
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
    CHECK(
        lighting::scene_lighting(scene)->set_probe_volume(openrac::test::gpu_probes(probes), error)
    );
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
    // Begin take-off from the current value before checking its vertical endpoint.
    renderer.render(input, {frame.id(), 64, 64});
    input.seconds = .2;
    renderer.render(input, {frame.id(), 64, 64});
    auto airborne = frame.read_rgba();
    CHECK(pixel(32, airborne) + 10 < pixel(32, original));    // directly under Ratchet
    CHECK(pixel(32, airborne) < pixel(32, original) * 0.7f);  // normal shadow on sunlit ground
    CHECK(std::abs(int(pixel(48, airborne)) - int(pixel(48, original))) < 3);
    CHECK(std::abs(int(pixel(16, airborne)) - int(pixel(16, original))) < 3);
    // Equipment may be submitted first but the body's transform must still anchor the map.
    auto attachment = hero;
    attachment.matrix = renderer::translation({-2, 0, 0});
    hero.hero_shadow_anchor = true;
    scene.set_instances(scene::Layer::Mobys, {attachment, hero});
    renderer.render(input, {frame.id(), 64, 64});
    const auto rig_shadow = frame.read_rgba();
    CHECK(pixel(16, rig_shadow) + 10 < pixel(16, original));
    CHECK(pixel(32, rig_shadow) + 10 < pixel(32, original));
    // Attached equipment must not leave a second, sun-direction shadow while the rig projects down.
    attachment.matrix = renderer::translation({-3, 0, 0});
    scene.set_instances(scene::Layer::Mobys, {attachment, hero});
    CHECK(scene.enable_sun_shadow(true, error));
    input.sun_pixel_shadow = true;
    renderer.render(input, {frame.id(), 64, 64});
    const auto rig_cascades = frame.read_rgba();
    CHECK(pixel(8, rig_cascades) + 10 < pixel(8, original));
    CHECK(std::abs(int(pixel(24, rig_cascades)) - int(pixel(24, original))) < 3);
    CHECK(scene.enable_sun_shadow(false, error));
    input.sun_pixel_shadow = false;
    // The depth pass must use the same skinned pose as the visible hero.
    hero.palette = 0;
    scene.set_palette({renderer::translation({1, 0, 0})});
    scene.set_instances(scene::Layer::Mobys, {hero, other});
    renderer.render(input, {frame.id(), 64, 64});
    airborne = frame.read_rgba();
    CHECK(pixel(40, airborne) + 10 < pixel(40, original));
    CHECK(std::abs(int(pixel(32, airborne)) - int(pixel(32, original))) < 3);
    // A world caster covers the same receiver as Ratchet. His separate map
    // grounded projection behaves as a normal shadow, without multiplying existing cascade shade.
    hero.palette = -1;
    auto blocker = other;
    blocker.matrix = renderer::translation({-4, 0, 4});
    scene.set_instances(scene::Layer::Mobys, {hero, blocker});
    CHECK(test::shadow_probe_fixture(scene, error));
    CHECK(scene.enable_sun_shadow(true, error));
    input.sun_pixel_shadow = true;
    input.hero_airborne = false;
    input.seconds += .2;  // Complete landing before checking ordinary grounded overlap.
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
    CHECK(ratio > .97f && ratio < 1.03f);
    CHECK(std::abs(int(pixel(16, overlap)) - int(pixel(16, world_shadow))) < 3);
    // Move the world blocker over the airborne footprint; its separate map darkens existing shade.
    blocker.matrix = renderer::translation({-4, 0, 2});
    scene.set_instances(scene::Layer::Mobys, {hero, blocker});
    input.hero_airborne = true;
    input.hero_pixel_shadow = false;
    input.seconds += .2;  // Complete take-off before checking the full overlap multiplier.
    renderer.render(input, {frame.id(), 64, 64});
    const auto airborne_world = frame.read_rgba();
    input.hero_pixel_shadow = true;
    renderer.render(input, {frame.id(), 64, 64});
    const auto airborne_overlap = frame.read_rgba();
    const float airborne_ratio =
        linear(pixel(32, airborne_overlap)) / linear(pixel(32, airborne_world));
    CHECK(airborne_ratio > .52f && airborne_ratio < .58f);
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
    CHECK(
        lighting::scene_lighting(scene)->set_probe_volume(openrac::test::gpu_probes(probes), error)
    );
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
    test_shadow_direction_transition();
    test_shadow_transition_frame_rate_independence();
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
