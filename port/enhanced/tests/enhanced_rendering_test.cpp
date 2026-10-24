// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <filesystem>
#include <string_view>

#include "enhanced/renderer/gtao.h"
#include "enhanced/tests/gpu_probe_fixture.h"
#include "enhanced/tests/probe_fixture.h"
#include "enhanced/viewer/early_lighting.h"
#include "enhanced/viewer/probe_lighting.h"
#include "enhanced/viewer/scene_lighting.h"
#include "scene/level_renderer.h"
#include "scene_shaders.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

using namespace openrac;

// --expect-off: switched off at start (OPENRAC_ENHANCED=0, the setting's default) or on OpenGL,
// nothing of enhanced rendering may be made, so the game draws as a build without it.
// --expect-build: switched on, Vulkan has it when the build compiled it in.
int main(int argc, char** argv) {
    const bool expect_off = argc > 1 && std::string_view(argv[1]) == "--expect-off";
    test::GlContext context;
    std::string error;
    if (!context.open(error)) {
        return test::kSkip;
    }
#if defined(OPENRAC_ENHANCED_RENDERING) && OPENRAC_ENHANCED_RENDERING
    const bool expected = !expect_off && gl::is_vulkan();
#else
    const bool expected = false;
#endif
    CHECK(gl::enhanced_rendering_available() == expected);
    if (expect_off) {
        CHECK(!gl::enhanced_rendering_available());
    }
    if (!expected) {
        // Explicit API requests must not bypass the build/backend boundary.
        auto volume = lighting::ProbeVolume::allocate({});
        lighting::ProbeLighting probes;
        CHECK(!probes.upload(openrac::test::gpu_probes(volume), error));
        CHECK(!probes.active());
        CHECK(!probes.compute_selection());
        scene::LevelScene scene;
        lighting::use_scene_lighting(scene);
        CHECK(!scene.enable_sun_shadow(true, error));
        CHECK(!scene.enable_hero_shadow(true, error));
        CHECK(!scene.sun_shadow_enabled() && !scene.hero_shadow_enabled());
        CHECK(!lighting::EarlyLighting::supported());
        renderer::Gtao ao;
        CHECK(!ao.render({}, {}));
        CHECK(ao.visibility_texture() == 0 && ao.depth_texture() == 0);
        // No atlas/shadow source is injected into the compatibility shader.
        const auto fragment = lighting::probe_mesh_fragment(scene::shaders::mesh_frag);
        CHECK(fragment.find("uniform sampler2D probe_irradiance") == std::string::npos);
        CHECK(fragment.find("uniform sampler2D sun_shadow_map0") == std::string::npos);
    }
    if (expected) {
        lighting::ProbeLighting probes;
        CHECK(!probes.upload(lighting::ProbeVolume::allocate({}), error)
        );  // No runtime conversion fallback.
        renderer::Renderer renderer;
        renderer.set_ambient_occlusion(std::make_unique<renderer::Gtao>());
        scene::LevelScene scene;
        lighting::use_scene_lighting(scene);
        scene::LevelData level;
        for (const auto& p :
             {renderer::Vec3{-1, -1, 0}, renderer::Vec3{1, -1, 0}, renderer::Vec3{0, 1, 0}}) {
            scene::Vertex vertex{};
            std::copy(p.begin(), p.end(), vertex.position);
            vertex.normal[2] = 1;
            std::fill(std::begin(vertex.colour), std::end(vertex.colour), 1.0f);
            level.vertices.push_back(vertex);
        }
        level.indices = {0, 1, 2};
        scene::Model model;
        model.min = {-1, -1, 0};
        model.max = {1, 1, 0};
        model.primitives.push_back({0, 3, 0});
        level.models.push_back(model);
        level.materials.push_back({});
        level.instances[static_cast<std::size_t>(scene::Layer::Terrain)].push_back({});
        CHECK(scene.upload(level, renderer.textures(), error));
        CHECK(!scene.probe_data_available());
        {
            // A level without a bake under the player's probe folder keeps the original lighting.
            lighting::set_probe_root(std::filesystem::temp_directory_path() / "openrac-no-probes");
            level.dir = "level_05";
            lighting::ProbeLighting unbaked;
            unbaked.load(level);
            CHECK(!unbaked.has_data() && !unbaked.active());
            lighting::set_probe_root({});
        }
        CHECK(!scene.enable_sun_shadow(true, error));
        CHECK(!scene.enable_hero_shadow(true, error));
        CHECK(renderer.add(
            std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Terrain), error
        ));
        CHECK(renderer.init(error));
        renderer::FrameBuffer target;
        CHECK(target.create(32, 32, error));
        renderer::FrameInput frame;
        frame.camera.position = {0, 0, 4};
        frame.camera.view = renderer::look_along(frame.camera.position, {0, 0, -1}, {0, 1, 0});
        frame.camera.projection = renderer::perspective_reversed(1, 1, .1f, 100);
        const renderer::RenderTarget rt{target.id(), 32, 32};
        renderer.render(frame, rt);
        const auto plain = target.read_rgba();
        const auto draws = renderer.last_stats().draw_calls;
        frame.gtao = true;
        renderer.render(frame, rt);
        CHECK(target.read_rgba() == plain);
        CHECK(renderer.last_stats().draw_calls == draws);  // No AO passes without probe data.

        CHECK(test::shadow_probe_fixture(scene, error));
        CHECK(scene.probe_data_available());
        CHECK(scene.enable_sun_shadow(true, error));
        CHECK(!scene.needs_early_lighting(frame));
        frame.sun_pixel_shadow = true;
        CHECK(scene.needs_early_lighting(frame));
        frame.sun_pixel_shadow = false;
        scene.set_probe_lighting(false, 1);
        CHECK(!scene.needs_early_lighting(frame));  // Allocated maps do not request a pass.
        CHECK(scene.enable_hero_shadow(true, error));
        frame.hero_pixel_shadow = true;
        CHECK(scene.needs_early_lighting(frame));  // Hero-only shadows still use compute.
        frame.hero_pixel_shadow = false;
        CHECK(!scene.needs_early_lighting(frame));
        scene.release();
        CHECK(!scene.probe_data_available());
        CHECK(!scene.enable_sun_shadow(true, error));
        renderer.release();
        target.release();
    }
    return test::result();
}
