// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <numbers>

#include "common/stats.h"
#include "enhanced/renderer/gtao.h"
#include "enhanced/tests/gpu_probe_fixture.h"
#include "enhanced/tests/probe_fixture.h"
#include "enhanced/viewer/scene_lighting.h"
#include "renderer/gl.h"
#if defined(OPENRAC_VULKAN)
#include "renderer/vulkan/vkgl.h"
#endif
#include "renderer/renderer.h"
#include "scene/level_renderer.h"
#include "tests/check.h"
#include "tests/gpu_draw_reference.h"
#include "tests/renderer_gl_context.h"

namespace {
using namespace openrac;

// The game has an early HUD-bucket background before its world renderers.
// A bucket-order shortcut must not run AO on that still-empty depth buffer.
class BeforeWorld : public renderer::BucketRenderer {
public:
    BeforeWorld() : BucketRenderer("before world", renderer::Bucket::Hud) {}

    void render(const renderer::FrameInput&, renderer::RenderState& state) override {
        CHECK(!state.depth_prepass && !state.reuse_prepass_depth);
    }
};

scene::LevelData corner(bool wall) {
    scene::LevelData data;
    const renderer::Vec3 points[] = {
        {-4, -4, 0}, {4, -4, 0}, {4, 4, 0}, {-4, 4, 0}, {0, -3, 0}, {0, 3, 0}, {0, 3, 3}, {0, -3, 3}
    };
    for (std::size_t i = 0; i < std::size(points); ++i) {
        scene::Vertex vertex{};
        std::copy(points[i].begin(), points[i].end(), vertex.position);
        vertex.normal[i < 4 ? 2 : 0] = 1;
        std::fill(std::begin(vertex.colour), std::end(vertex.colour), 1.0f);
        vertex.light_slot = -1;
        data.vertices.push_back(vertex);
    }
    data.indices = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
    scene::Model model;
    model.min = {-4, -4, 0};
    model.max = {4, 4, 3};
    model.primitives.push_back({0, wall ? 12u : 6u, 0});
    data.models.push_back(model);
    data.materials.push_back({});
    data.instances[static_cast<std::size_t>(scene::Layer::Terrain)].push_back({});
    return data;
}

void check_scene(bool wall, int samples, renderer::GtaoQuality quality) {
    auto level = corner(wall);
    renderer::Renderer renderer;
    renderer.set_ambient_occlusion(std::make_unique<renderer::Gtao>());
    scene::LevelScene scene;
    lighting::use_scene_lighting(scene);
    std::string error;
    CHECK(scene.upload(level, renderer.textures(), error));
    CHECK(test::shadow_probe_fixture(scene, error));
    CHECK(renderer.add(std::make_unique<BeforeWorld>(), error));
    CHECK(
        renderer.add(std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Terrain), error)
    );
    CHECK(renderer.init(error));
    renderer::FrameBuffer frame;
    bool modes_differ = false;
    for (const auto size : {std::pair{128, 96}, std::pair{131, 99}, std::pair{192, 96}}) {
        const auto [width, height] = size;
        CHECK(frame.create(width, height, error, samples));
        renderer::FrameInput input;
        input.gtao_settings.quality = quality;
        input.camera.view = renderer::look_along({4, -6, 4}, {-4, 6, -3.5f}, {0, 0, 1});
        input.camera.projection =
            renderer::perspective_reversed(1.0f, float(width) / float(height), 0.1f, 100);
        input.clear_colour = {0.1f, 0.2f, 0.3f, 1};
        renderer.render(input, {frame.id(), width, height});
        const auto plain = frame.read_rgba();
        input.gtao = true;
        renderer.render(input, {frame.id(), width, height});
        const auto ao = frame.read_rgba();
        if (samples == 1) {
            // Both paths must preserve the same pixels, including subsequent
            // depth-tested colour passes and changes to the borrowed depth.
            renderer::FrameBuffer direct;
            CHECK(direct.create(width, height, error, 1, true));
            renderer::RenderTarget target{direct.id(), width, height};
            target.depth_texture = direct.depth_texture();
            renderer.render(input, target);
            CHECK(direct.read_rgba() == ao);
            const auto saved_view = input.camera.view;
            input.camera.view = renderer::look_along({3, -5, 3}, {-3, 5, -2.5f}, {0, 0, 1});
            renderer.render(input, target);
            input.camera.view = saved_view;
            renderer.render(input, target);
            CHECK(direct.read_rgba() == ao);
            renderer::Gtao evaluation;
            CHECK(evaluation.render(input, target, false, true));
            CHECK(evaluation.depth_texture() == direct.depth_texture());
            evaluation.release();
            CHECK(evaluation.depth_texture() == 0);
            // Releasing GTAO must not delete the borrowed framebuffer texture.
            renderer.render(input, target);
            CHECK(direct.read_rgba() == ao);
            direct.release();
        }
        CHECK(renderer.last_stats().draw_calls == 2);
        int darkened = 0;
        for (std::size_t i = 0; i < plain.size(); i += 4) {
            CHECK(
                ao[i] <= plain[i] + 1 && ao[i + 1] <= plain[i + 1] + 1
                && ao[i + 2] <= plain[i + 2] + 1
            );
            CHECK(ao[i + 3] == plain[i + 3]);
            if (plain[i] == 26 && plain[i + 1] == 51 && plain[i + 2] == 76) {
                CHECK(ao[i] == plain[i] && ao[i + 1] == plain[i + 1] && ao[i + 2] == plain[i + 2]);
            }
            if (plain[i] > ao[i] + 4) {
                ++darkened;
            }
        }
        std::printf(
            "GTAO wall=%d samples=%d %dx%d: %d contact pixels\n",
            wall,
            samples,
            width,
            height,
            darkened
        );
        CHECK(wall ? darkened > 20 : darkened == 0);
        // No frame-index jitter or history: identical static scenes stay identical.
        for (int n = 0; n < 5; ++n) {
            input.frame += 11;
            renderer.render(input, {frame.id(), width, height});
            CHECK(frame.read_rgba() == ao);
        }
        // Camera movement must not leave previous-frame AO in the result.
        const auto original_view = input.camera.view;
        input.camera.view = renderer::look_along({3, -5, 3}, {-3, 5, -2.5f}, {0, 0, 1});
        renderer.render(input, {frame.id(), width, height});
        input.camera.view = original_view;
        renderer.render(input, {frame.id(), width, height});
        CHECK(frame.read_rgba() == ao);
        // Switching an existing renderer must select the other compiled horizon
        // shader immediately, and switching back must restore the exact image.
        input.gtao_settings.quality = quality == renderer::GtaoQuality::Performance
                                          ? renderer::GtaoQuality::Quality
                                          : renderer::GtaoQuality::Performance;
        renderer.render(input, {frame.id(), width, height});
        modes_differ |= frame.read_rgba() != ao;
        input.gtao_settings.quality = quality;
        renderer.render(input, {frame.id(), width, height});
        CHECK(frame.read_rgba() == ao);
        if (wall && samples == 1) {
            input.gtao_settings.strength = 0.0f;
            renderer.render(input, {frame.id(), width, height});
            CHECK(frame.read_rgba() == plain);
            input.gtao_settings.strength = renderer::GtaoSettings{}.strength * 0.5f;
            renderer.render(input, {frame.id(), width, height});
            const auto weaker = frame.read_rgba();
            input.gtao_settings.strength = 2.0f;
            renderer.render(input, {frame.id(), width, height});
            const auto stronger = frame.read_rgba();
            for (std::size_t i = 0; i < ao.size(); ++i) {
                CHECK(weaker[i] >= ao[i] && weaker[i] <= plain[i]);
                CHECK(stronger[i] <= ao[i]);
            }
            input.gtao_settings.strength = 1.0f;
            input.gtao_settings.half_resolution = false;
            input.gtao_settings.slices = 3;
            input.gtao_settings.steps = 6;
            renderer.render(input, {frame.id(), width, height});
            const auto full = frame.read_rgba();
            CHECK(full.size() == plain.size());
            CHECK(full != plain);
            input.gtao_settings = renderer::GtaoSettings{};
            input.gtao_settings.quality = quality;
            renderer.render(input, {frame.id(), width, height});
            CHECK(frame.read_rgba() == ao);
        }
        CHECK(gl::glGetError() == gl::GL_NO_ERROR);
    }
    if (wall) { CHECK(modes_differ); }
    frame.release();
    scene.release();
    renderer.release();
}

void indirect_lighting(int samples, bool glow = false, bool cutout = false, bool shadows = false) {
    auto level = corner(true);
    level.light_sets.resize(256);
    level.light_sets[0] = 0.4f;  // red directional light
    level.light_sets[4] = level.light_sets[6] = -0.70710678f;
    if (cutout) {
        renderer::Rgba8Image transparent(1, 1);
        transparent.set_texel(0, 0, 0x00ffffffu);
        level.images.push_back(transparent);
        level.image_paths.push_back("gtao-test-cutout.png");
        level.translucent_images.push_back(true);
        scene::Material material;
        material.image = 0;
        material.cutout = true;
        level.materials.push_back(material);
        level.models[0].primitives = {{0, 6, 0}, {6, 6, 1}};
    }
    if (glow) {
        for (auto& vertex : level.vertices) {
            vertex.light_slot = -2;
        }
        scene::Instance instance;
        instance.moby_light = {0, 0, 0, 1};
        instance.moby_ambient = {0, 0, 0, 256};  // opaque red glow over the corner
        level.instances[static_cast<std::size_t>(scene::Layer::Mobys)].push_back(instance);
    }
    renderer::Renderer renderer;
    renderer.set_ambient_occlusion(std::make_unique<renderer::Gtao>());
    scene::LevelScene scene;
    lighting::use_scene_lighting(scene);
    std::string error;
    CHECK(scene.upload(level, renderer.textures(), error));
    if (shadows) {
        CHECK(test::shadow_probe_fixture(scene, error));
        CHECK(scene.enable_sun_shadow(true, error));
    }
    CHECK(renderer.add(std::make_unique<BeforeWorld>(), error));
    CHECK(
        renderer.add(std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Terrain), error)
    );
    if (glow) {
        CHECK(
            renderer.add(std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Mobys), error)
        );
    }
    CHECK(renderer.init(error));
    lighting::ProbeGrid grid;
    grid.origin = {-8, -8, -4};
    grid.spacing = {16, 16, 16};
    auto volume = lighting::ProbeVolume::allocate(grid);
    for (auto& p : volume.probes) {
        p.valid = true;
    }
    for (auto& e : volume.irradiance) {
        e = {0, std::numbers::pi_v<float> * .25f, 0};
    }
    CHECK(
        lighting::scene_lighting(scene)->set_probe_volume(openrac::test::gpu_probes(volume), error)
    );
    std::array<float, 64> points{};
    points[2] = .5f;
    points[4] = 2;
    points[5] = -2;
    points[6] = 2;
    points[7] = 10;  // blue point light
    scene.set_point_lights(points);
    renderer::FrameBuffer frame;
    for (const auto size : {std::pair{128, 96}, std::pair{131, 99}}) {
        const auto [width, height] = size;
        CHECK(frame.create(width, height, error, samples));
        renderer::FrameInput input;
        input.sun_pixel_shadow = shadows;
        input.camera.view = renderer::look_along({4, -6, 4}, {-4, 6, -3.5f}, {0, 0, 1});
        input.camera.projection =
            renderer::perspective_reversed(1, float(width) / height, .1f, 100);
        input.clear_colour = {.1f, .2f, .3f, 1};
        scene.set_probe_view(1);
        renderer.render(input, {frame.id(), width, height});
        const auto plain = frame.read_rgba();
        // Force a real prepass without visible AO. EQUAL colour shading must
        // retain identical coverage after sky/background and shadow-map FBOs.
        input.gtao = true;
        input.gtao_settings.strength = .000001f;
        renderer.render(input, {frame.id(), width, height});
        CHECK(frame.read_rgba() == plain);
        input.gtao_settings = renderer::GtaoSettings{};
        input.gtao = true;
        renderer.render(input, {frame.id(), width, height});
        const auto ao = frame.read_rgba();
        if (samples == 1) {
            // Both paths must preserve the same pixels, including subsequent
            // depth-tested colour passes and changes to the borrowed depth.
            renderer::FrameBuffer direct;
            CHECK(direct.create(width, height, error, 1, true));
            renderer::RenderTarget target{direct.id(), width, height};
            target.depth_texture = direct.depth_texture();
            renderer.render(input, target);
            CHECK(direct.read_rgba() == ao);
            const auto saved_view = input.camera.view;
            input.camera.view = renderer::look_along({3, -5, 3}, {-3, 5, -2.5f}, {0, 0, 1});
            renderer.render(input, target);
            input.camera.view = saved_view;
            renderer.render(input, target);
            CHECK(direct.read_rgba() == ao);
            renderer::Gtao evaluation;
            CHECK(evaluation.render(input, target, false, true));
            CHECK(evaluation.depth_texture() == direct.depth_texture());
            evaluation.release();
            CHECK(evaluation.depth_texture() == 0);
            // Releasing GTAO must not delete the borrowed framebuffer texture.
            renderer.render(input, target);
            CHECK(direct.read_rgba() == ao);
            direct.release();
        }
        int contacts = 0;
        for (std::size_t i = 0; i < plain.size(); i += 4) {
            CHECK(ao[i] == plain[i]);          // directional red cannot be occluded by GTAO
            CHECK(ao[i + 2] == plain[i + 2]);  // point-light blue cannot be occluded
            CHECK(ao[i + 3] == plain[i + 3]);
            CHECK(ao[i + 1] <= plain[i + 1] + 1);
            if (plain[i + 1] > ao[i + 1] + 4) {
                ++contacts;
            }
        }
        CHECK(glow || cutout ? ao == plain : contacts > 20);
        std::printf(
            "indirect GTAO samples=%d glow=%d: %d GI contact pixels\n", samples, glow, contacts
        );
        input.gtao = false;
        renderer.render(input, {frame.id(), width, height});
        CHECK(frame.read_rgba() == plain);  // no stale AO when disabled
        if (glow || cutout) {
            continue;
        }
        input.gtao = true;
        input.frame += 29;
        renderer.render(input, {frame.id(), width, height});
        CHECK(frame.read_rgba() == ao);
        const auto view = input.camera.view;
        input.camera.view = renderer::look_along({3, -5, 3}, {-3, 5, -2.5f}, {0, 0, 1});
        renderer.render(input, {frame.id(), width, height});
        input.camera.view = view;
        renderer.render(input, {frame.id(), width, height});
        CHECK(frame.read_rgba() == ao);
        input.gtao = true;
        input.gtao_settings.strength = 0;
        renderer.render(input, {frame.id(), width, height});
        CHECK(frame.read_rgba() == plain);
        input.gtao_settings = renderer::GtaoSettings{};
        for (int mode : {2, 3, 4}) {
            scene.set_probe_view(mode);
            input.gtao = false;
            renderer.render(input, {frame.id(), width, height});
            const auto before = frame.read_rgba();
            input.gtao = true;
            renderer.render(input, {frame.id(), width, height});
            const auto after = frame.read_rgba();
            if (mode == 2) {
                CHECK(before != after);
            } else {
                CHECK(before == after);
            }
        }
        scene.set_probe_view(1);
        scene.set_fog({.2f, .3f, .4f}, 0, 1, 0, 0);
        input.gtao = false;
        renderer.render(input, {frame.id(), width, height});
        const auto fog = frame.read_rgba();
        input.gtao = true;
        renderer.render(input, {frame.id(), width, height});
        CHECK(frame.read_rgba() == fog);  // fog added after AO remains untouched
        scene.set_fog({0, 0, 0}, 0, 1, 255, 255);
        CHECK(gl::glGetError() == gl::GL_NO_ERROR);
    }
    frame.release();
    scene.release();
    renderer.release();
}

void scripted_glow_depth() {
    using namespace gl;
    auto level = corner(true);
    level.instances[static_cast<std::size_t>(scene::Layer::Terrain)].clear();
    for (auto& vertex : level.vertices) vertex.light_slot = -2;
    scene::Instance instance;
    instance.moby_light[3] = -1;  // Explicit gameplay colour, still a lit moby.
    renderer::Renderer renderer;
    renderer.set_ambient_occlusion(std::make_unique<renderer::Gtao>());
    scene::LevelScene scene;
    lighting::use_scene_lighting(scene);
    std::string error;
    CHECK(scene.upload(level, renderer.textures(), error));
    renderer::FrameBuffer target;
    CHECK(target.create(64, 48, error));
    renderer::Shader sentinel;
    CHECK(sentinel.build("unoccluded depth sentinel", R"glsl(#version 410 core
void main() {
    vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    gl_Position = vec4(p * 2.0 - 1.0, -1.0, 1.0);
}
)glsl", R"glsl(#version 410 core
layout(location=0) out vec4 result;
void main() { result = vec4(0, 1, 0, 1); }
)glsl", error));
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    renderer::FrameInput input;
    input.camera.view = renderer::look_along({4, -6, 4}, {-4, 6, -3.5f}, {0, 0, 1});
    input.camera.projection = renderer::perspective_reversed(1, 4.0f / 3.0f, .1f, 100);
    renderer::RenderState state{renderer.textures(), {target.id(), 64, 48}, {}};
    state.depth_prepass = true;
    for (bool glow : {false, true}) {
        instance.moby_ambient[3] = glow ? 256.0f : 0.0f;
        scene.set_instances(scene::Layer::Mobys, {instance});
        glBindFramebuffer(GL_FRAMEBUFFER, target.id());
        glViewport(0, 0, 64, 48);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glClearDepth(0);
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        scene.draw_layer(scene::Layer::Mobys, input, state);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthFunc(GL_GEQUAL);
        sentinel.use();
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        const auto pixels = target.read_rgba();
        unsigned occluded = 0;
        for (std::size_t i = 0; i < pixels.size(); i += 4) occluded += pixels[i] != 0;
        CHECK(glow ? occluded == 0 : occluded > 0);
    }
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    sentinel.release();
    target.release();
    scene.release();
    renderer.release();
}

void optimized_paths(int samples) {
    auto level = corner(true);
    for (auto& v : level.vertices) {
        v.colour[3] = .25f;
        v.light_slot = 0;
        v.weights[0] = .25f;
        v.weights[1] = .75f;
        v.joints[1] = 1;
    }
    level.images.emplace_back(1, 1);
    level.images[0].set_texel(0, 0, 0xffffffffu);
    level.image_paths.push_back("camera-depth-cutout.png");
    level.translucent_images.push_back(false);
    scene::Material cutout;
    cutout.image = 0;
    cutout.cutout = true;
    cutout.colour[3] = .995f;
    level.materials.push_back(cutout);
    level.light_colours.push_back(0x80808080u);  // tie alpha overrides the vertex's alpha
    level.models[0].primitives = {{0, 6, 0}, {6, 6, 1}};
    level.models[0].joints = 2;
    scene::Instance prop;
    prop.lights = 0;
    prop.matrix = renderer::scale(-.8f);
    prop.matrix[5] = .7f;
    prop.matrix[10] = 1.2f;
    prop.matrix[4] = .1f;
    prop.matrix[14] = .5f;
    std::vector<scene::Instance> props{prop};
    for (unsigned i = 0; i < 30; ++i) {
        prop.matrix = renderer::translation({1000.f + i * 40, 2000, 0});
        props.push_back(prop);
    }
    level.instances[static_cast<std::size_t>(scene::Layer::Ties)] = props;
    scene::Instance skinned;
    skinned.palette = 0;
    skinned.moby_light[3] = 1;
    skinned.matrix = renderer::translation({1, -1, .3f});
    level.instances[static_cast<std::size_t>(scene::Layer::Mobys)].push_back(skinned);
    // Pose-aware world bounds: one visible animated instance and distant
    // opaque, translucent and additive copies. Their bind boxes are unusable.
    auto bounded = skinned;
    bounded.world_sphere = {1, -1, 1, 8};
    level.instances[static_cast<std::size_t>(scene::Layer::Mobys)].push_back(bounded);
    for (int blend = 0; blend < 3; ++blend) {
        bounded.matrix = renderer::translation({3000, 4000, 0});
        bounded.world_sphere = {3000, 4000, 0, 8};
        bounded.blend = blend;
        level.instances[static_cast<std::size_t>(scene::Layer::Mobys)].push_back(bounded);
    }
    // Invalid bounds retain animated geometry conservatively.
    bounded = skinned;
    bounded.world_sphere = {NAN, 0, 0, 1};
    level.instances[static_cast<std::size_t>(scene::Layer::Mobys)].push_back(bounded);
    bounded.world_sphere = {0, 0, 0, -1};
    level.instances[static_cast<std::size_t>(scene::Layer::Mobys)].push_back(bounded);
    auto ui = skinned;
    ui.palette = -1;
    ui.moby_light[3] = 2;
    ui.matrix = renderer::translation({-2, 0, 1});
    level.instances[static_cast<std::size_t>(scene::Layer::Mobys)].push_back(ui);
    renderer::Renderer renderer;
    renderer.set_ambient_occlusion(std::make_unique<renderer::Gtao>());
    scene::LevelScene scene;
    lighting::use_scene_lighting(scene);
    std::string error;
    CHECK(scene.upload(level, renderer.textures(), error));
    scene.set_palette({renderer::identity(), renderer::translation({.3f, .1f, .5f})});
    for (auto layer : {scene::Layer::Terrain, scene::Layer::Ties, scene::Layer::Mobys}) {
        CHECK(renderer.add(std::make_unique<scene::LayerRenderer>(scene, layer), error));
    }
    CHECK(renderer.init(error));
    auto volume = lighting::ProbeVolume::allocate(lighting::ProbeGrid{});
    for (auto& p : volume.probes) {
        p.valid = true;
    }
    for (auto& e : volume.irradiance) {
        e = {1, 1, 1};
    }
    CHECK(
        lighting::scene_lighting(scene)->set_probe_volume(openrac::test::gpu_probes(volume), error)
    );
    auto switch_reference = [](const char* variable, bool off) {
        for (auto* probe : stats::probes()) {
            if (std::strcmp(probe->variable, variable) == 0) {
                probe->on = off;
                return;
            }
        }
        CHECK(false);
    };
    renderer::FrameBuffer frame;
    for (auto size : {std::pair{128, 96}, std::pair{131, 99}}) {
        test::gpu_draw_reference(true);  // immediate CPU culling counters below
        CHECK(frame.create(size.first, size.second, error, samples));
        renderer::RenderTarget rt{frame.id(), size.first, size.second, .75f};
        renderer::FrameInput input;
        input.gtao = true;
        input.camera.view = renderer::look_along({4, -6, 4}, {-4, 6, -3.5f}, {0, 0, 1});
        input.camera.projection =
            renderer::perspective_reversed(1, float(size.first) / size.second, .1f, 100);
        renderer.render(input, rt);  // register probes
        switch_reference("OPENRAC_CAMERA_INSTANCE_CULL_OFF", true);
        renderer.render(input, rt);
        const auto reference = frame.read_rgba();
        const auto reference_triangles = renderer.last_stats().triangles;
        switch_reference("OPENRAC_CAMERA_INSTANCE_CULL_OFF", false);
        renderer.render(input, rt);
        CHECK(frame.read_rgba() == reference);
        CHECK(renderer.last_stats().triangles < reference_triangles);
        const auto culled_triangles = renderer.last_stats().triangles;
        CHECK(scene.camera_culling()[static_cast<std::size_t>(scene::Layer::Ties)].rejected >= 30);
        CHECK(scene.camera_culling()[static_cast<std::size_t>(scene::Layer::Mobys)].unbounded >= 1);
        CHECK(scene.camera_culling()[static_cast<std::size_t>(scene::Layer::Mobys)].rejected >= 3);
        switch_reference("OPENRAC_MOBY_SPHERE_CULL_OFF", true);
        scene.set_instances(
            scene::Layer::Mobys, level.instances[static_cast<std::size_t>(scene::Layer::Mobys)]
        );
        renderer.render(input, rt);
        CHECK(frame.read_rgba() == reference);
        CHECK(renderer.last_stats().triangles > culled_triangles);
        switch_reference("OPENRAC_MOBY_SPHERE_CULL_OFF", false);
        scene.set_instances(
            scene::Layer::Mobys, level.instances[static_cast<std::size_t>(scene::Layer::Mobys)]
        );
        renderer.render(input, rt);
        CHECK(frame.read_rgba() == reference);
        CHECK(renderer.last_stats().triangles == culled_triangles);
        input.camera.view = renderer::look_along({1004, 1994, 4}, {-4, 6, -3.5f}, {0, 0, 1});
        renderer.render(input, rt);
        const auto moved = frame.read_rgba();
        switch_reference("OPENRAC_CAMERA_INSTANCE_CULL_OFF", true);
        renderer.render(input, rt);
        CHECK(frame.read_rgba() == moved);
        switch_reference("OPENRAC_CAMERA_INSTANCE_CULL_OFF", false);
        if (gl::is_vulkan()) {
            test::gpu_draw_reference(false);
            CHECK(test::gpu_draw_active());
            renderer.render(input, rt);
            CHECK(frame.read_rgba() == moved);
        }
    }
    frame.release();
    scene.release();
    renderer.release();
}
}  // namespace

int main() {
    openrac::test::GlContext context;
    std::string error;
    if (!context.open(error)) {
        std::printf("SKIP: %s\n", error.c_str());
        return openrac::test::kSkip;
    }
    for (auto quality : {renderer::GtaoQuality::Performance, renderer::GtaoQuality::Quality}) {
        check_scene(false, 1, quality);
        check_scene(true, 1, quality);
        check_scene(true, 4, quality);
    }
    indirect_lighting(1);
    indirect_lighting(4);
    indirect_lighting(1, true);
    indirect_lighting(1, false, true);
    indirect_lighting(1, false, false, true);
    scripted_glow_depth();
    optimized_paths(1);
    optimized_paths(4);
    if (openrac::stats::gpu_timers()) {
        using namespace openrac;
        stats::set_enabled(stats::Client::Overlay, true);
        gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
        renderer::FrameBuffer timing_frame;
        CHECK(timing_frame.create(1024, 1024, error));
        gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, timing_frame.id());
        gl::glViewport(0, 0, 1024, 1024);
        for (unsigned i = 0; i < 8; ++i) {
            stats::begin_frame(stats::FrameKind::Tick, i);
            {
                const stats::Scope timed("timestamp flush", stats::kGpu);
                gl::glClear(gl::GL_COLOR_BUFFER_BIT);
                // A synchronous readback/finish starts another recording in
                // the same frame slot. Its active timestamp pair must survive.
                gl::glFinish();
            }
            {
                const stats::Scope work("timestamp work", stats::kGpu);
                for (unsigned clear = 0; clear < 64; ++clear) {
                    gl::glClearColor(float(clear & 1), 0, 0, 1);
                    gl::glClear(gl::GL_COLOR_BUFFER_BIT);
                }
            }
            // Empty scopes must not repeatedly charge earlier in-flight work.
            for (unsigned marker = 0; marker < 16; ++marker) {
                const stats::Scope empty("timestamp empty marker", stats::kGpu);
            }
            gl::glFinish();
            context.window().swap();
        }
        stats::begin_frame(stats::FrameKind::Tick, 8);
        const auto* measured = stats::history(3);
        CHECK(measured && measured->pending_gpu == 0 && measured->gpu_ms() >= 0);
        if (measured) {
            // Finish drains work before the next CPU frame begins, so the
            // non-overlapping GPU intervals fit inside that CPU interval.
            CHECK(measured->gpu_ms() <= measured->interval_ms + 1.0);
        }
        timing_frame.release();
        stats::set_enabled(stats::Client::Overlay, false);
    }
#if defined(OPENRAC_VULKAN)
    if (gl::is_vulkan()) CHECK(vkgl::submission_counters().validation_errors == 0);
#endif
    return openrac::test::result();
}
