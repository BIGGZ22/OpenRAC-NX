// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <cstring>
#include <numbers>

#include "common/stats.h"
#include "enhanced/tests/gpu_probe_fixture.h"
#include "enhanced/tests/probe_fixture.h"
#include "enhanced/viewer/sun_shadow.h"
#include "renderer/gl.h"
#include "renderer/shadows.h"
#include "scene/level_renderer.h"
#include "tests/check.h"
#include "tests/gpu_draw_reference.h"
#include "tests/renderer_gl_context.h"
#include "viewer_shaders.h"
using namespace openrac;
using namespace openrac::gl;

renderer::Camera camera(float height) {
    renderer::Camera result;
    result.position = {0, 0, height};
    result.view = renderer::look_along(result.position, {0, 0, -1}, {0, 1, 0});
    result.projection = renderer::perspective_reversed(std::numbers::pi_v<float> / 2, 1, .1f, 512);
    return result;
}

void cpu() {
    const auto c = camera(12);
    const auto cascades = scene::sun_cascades(c, {1, 0, -1}, {-100, -100, -10}, {100, 100, 200});
    CHECK(std::abs(cascades.near_distance - .1f) < 1e-4f);
    CHECK(std::abs(cascades.ends.back() - 256) < 1e-3f);
    CHECK(cascades.ends[0] < 12);  // reserve the first cascade for nearby receivers
    float start = cascades.near_distance;
    for (unsigned i = 0; i < 4; ++i) {
        CHECK(cascades.ends[i] > start);
        const auto& volume = cascades.casters[i];
        CHECK(volume.plane_count >= 3 && volume.plane_count <= 6);
        // All eight receiver frustum corners fit their cascade.
        for (int corner = 0; corner < 8; ++corner) {
            float d = (corner & 4) ? cascades.ends[i] : start;
            renderer::Vec3 p{(corner & 1) ? d : -d, (corner & 2) ? d : -d, 12 - d};
            auto q = renderer::transform_point(cascades.matrices[i], p);
            CHECK(std::abs(q[0]) <= 1.001f && std::abs(q[1]) <= 1.001f && std::abs(q[2]) <= 1.001f);
            // The caster volume is infinite along the sun: retain both
            // off-camera upstream casters and the complete receiver footprint.
            for (unsigned j = 0; j < volume.plane_count; ++j) {
                const auto& plane = volume.planes[j];
                for (float travel : {-10000.f, 0.f, 10000.f}) {
                    auto caster = p;
                    caster[0] += travel;
                    caster[2] -= travel;
                    const float value = plane[0] * caster[0] + plane[1] * caster[1]
                                        + plane[2] * caster[2] + plane[3];
                    CHECK(value >= -0.002f);
                }
            }
        }
        start = cascades.ends[i];
    }
    auto moved = c;
    moved.position[1] += .00001f;
    moved.view = renderer::look_along(moved.position, {0, 0, -1}, {0, 1, 0});
    const auto snapped = scene::sun_cascades(moved, {1, 0, -1}, {-100, -100, -10}, {100, 100, 200});
    for (unsigned i = 0; i < 4; ++i) {
        CHECK(std::abs(cascades.matrices[i][12] - snapped.matrices[i][12]) < 1e-6f);
        CHECK(std::abs(cascades.matrices[i][13] - snapped.matrices[i][13]) < 1e-6f);
    }
    const auto near = renderer::transform_point(cascades.matrices[0], {0, 0, 0});
    const auto upstream = renderer::transform_point(cascades.matrices[0], {-8, 0, 8});
    CHECK(std::abs(near[0] - upstream[0]) < 1e-5f && std::abs(near[1] - upstream[1]) < 1e-5f);
    CHECK(upstream[2] > near[2]);  // reverse Z, including off-camera casters
    bool rejected = false;
    try {
        scene::sun_cascades(c, {0, 0, 0}, {}, {}, 256);
    } catch (const std::exception&) {
        rejected = true;
    }
    CHECK(rejected);
}

float linear(std::uint8_t c) {
    float v = c / 255.f;
    return v <= .04045f ? v / 12.92f : std::pow((v + .055f) / 1.055f, 2.4f);
}

void caster_hulls() {
    bool trims_square = false;
    for (float aspect : {.6f, 1.f, 2.4f}) {
        for (const renderer::Vec3 direction :
             {renderer::Vec3{1, 0, -1}, renderer::Vec3{0, 0, -1}, renderer::Vec3{1, .2f, .1f}}) {
            auto c = camera(12);
            c.position = {100, -200, 30};
            c.view = renderer::look_along(c.position, {.3f, .7f, -.6f}, {0, 0, 1});
            c.projection = renderer::perspective_reversed(1.1f, aspect, .1f, 512);
            const auto cascades =
                scene::sun_cascades(c, direction, {-500, -500, -100}, {500, 500, 300});
            for (unsigned i = 0; i < scene::kSunCascades; ++i) {
                const auto& volume = cascades.casters[i];
                CHECK(volume.plane_count >= 3 && volume.plane_count <= 6);
                const auto& matrix = cascades.matrices[i];
                const float radius =
                    1
                    / std::sqrt(
                        matrix[0] * matrix[0] + matrix[4] * matrix[4] + matrix[8] * matrix[8]
                    );
                const float texel = 2 * radius / scene::kSunResolutions[i];
                const float previous = i ? cascades.ends[i - 1] : cascades.near_distance;
                const float start =
                    i ? previous
                            - .1f
                                  * (previous
                                     - (i > 1 ? cascades.ends[i - 2] : cascades.near_distance))
                      : previous;
                for (unsigned corner = 0; corner < 8; ++corner) {
                    const float d = corner & 4 ? cascades.ends[i] : start;
                    const renderer::Vec3 eye{
                        (corner & 1 ? d : -d) / c.projection[0],
                        (corner & 2 ? d : -d) / c.projection[5],
                        -d
                    };
                    renderer::Vec3 world = c.position;
                    for (unsigned k = 0; k < 3; ++k) {
                        for (unsigned j = 0; j < 3; ++j) {
                            world[k] += c.view[k * 4 + j] * eye[j];
                        }
                    }
                    for (unsigned shift = 0; shift < volume.plane_count; ++shift) {
                        auto p = world;
                        for (unsigned k = 0; k < 3; ++k) {
                            p[k] +=
                                direction[k] * 5000 - volume.planes[shift][k] * (2 * texel + .025f);
                        }
                        for (unsigned j = 0; j < volume.plane_count; ++j) {
                            const auto& plane = volume.planes[j];
                            CHECK(
                                plane[0] * p[0] + plane[1] * p[1] + plane[2] * p[2] + plane[3]
                                >= -.005f
                            );
                        }
                    }
                }
                // Some map space must be rejected, rather than silently
                // falling back to the original enclosing square everywhere.
                for (float x : {-.9f, 0.f, .9f}) {
                    for (float y : {-.9f, 0.f, .9f}) {
                        renderer::Vec3 p;
                        for (unsigned k = 0; k < 3; ++k) {
                            p[k] = radius * radius
                                   * ((x - matrix[12]) * matrix[k * 4]
                                      + (y - matrix[13]) * matrix[k * 4 + 1]);
                        }
                        for (unsigned j = 0; j < volume.plane_count; ++j) {
                            const auto& plane = volume.planes[j];
                            trims_square |=
                                plane[0] * p[0] + plane[1] * p[1] + plane[2] * p[2] + plane[3]
                                < -.01f;
                        }
                    }
                }
            }
        }
    }
    CHECK(trims_square);
}

void filter_gpu() {
    renderer::FrameBuffer frame;
    renderer::Shader shader;
    std::string error;
    CHECK(frame.create(64, 1, error));
    const char* vertex = R"glsl(#version 410 core
void main() {
    vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)glsl";
    const std::string fragment = std::string("#version 410 core\n") + scene::shaders::probes_glsl
                                 + R"glsl(layout(location = 0) out vec4 out_colour;
uniform int filter_mode;
void main() {
    float u = 0.25 + gl_FragCoord.x / 128.0;
    float v = shadow_bilinear(sun_shadow_map0, vec3(u, 0.5, 0.5), 0.0, vec2(0.0));
    if (filter_mode != 0) {
        vec3 p = vec3(u * 2.0 - 1.0, 0.0, (0.3 + 0.4 * u) * 2.0 - 1.0);
        v = sun_shadow_pcf(
            sun_shadow_map0, mat4(1.0), p, vec3(0.0), normalize(vec3(-0.4, 0.0, 1.0))
        );
    }
    out_colour = vec4(v, v, v, 1.0);
}
)glsl";
    CHECK(shader.build("subtexel shadow comparisons", vertex, fragment, error));
    unsigned map = 0, vao = 0;
    glGenTextures(1, &map);
    glActiveTexture(GL_TEXTURE0 + 7);
    glBindTexture(GL_TEXTURE_2D, map);
    const float depths[] = {.75f, 0, 0, 1, .25f, 0, 0, 1};
    glTexImage2D(GL_TEXTURE_2D, 0, 0x8814, 2, 1, 0, GL_RGBA, GL_FLOAT, depths);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
    glViewport(0, 0, 64, 1);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    shader.use();
    glUniform1i(shader.uniform("sun_shadow_map0"), 7);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    const auto rgba = frame.read_rgba();
    for (unsigned i = 0; i < 64; ++i) {
        const float expected = (i + .5f) / 64;
        CHECK(std::abs(rgba[i * 4] / 255.f - expected) < .006f);
        if (i) {
            CHECK(rgba[i * 4] > rgba[(i - 1) * 4]);
        }
    }
    // A sloped receiver must compare at its plane depth at each tap, not
    // shadow itself with the neighbouring sample's higher depth.
    const float slope_depths[] = {.4f, 0, 0, 1, .6f, 0, 0, 1};
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 1, GL_RGBA, GL_FLOAT, slope_depths);
    glUniform1i(shader.uniform("filter_mode"), 1);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    const auto plane = frame.read_rgba();
    for (unsigned i = 0; i < 64; ++i) {
        CHECK(plane[i * 4] >= 254);
    }
    CHECK(glGetError() == GL_NO_ERROR);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &map);
    glActiveTexture(GL_TEXTURE0);
    shader.release();
    frame.release();
}

// Compare both filters in one shader so every case sees identical arithmetic,
// texture storage and coordinates. No game data or captures are involved.
void unique_filter_gpu() {
    renderer::FrameBuffer frame;
    renderer::Shader shader;
    std::string error;
    constexpr int width = 256, height = 32;
    CHECK(frame.create(width, height, error));
    const char* vertex = R"(#version 410 core
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)";
    auto fragment =
        std::string("#version 410 core\n") + scene::shaders::probes_glsl + R"(uniform int test_kind;
uniform int tent_kernel;
uniform vec2 test_slope;
layout(location = 0) out vec4 result;
void main() {
    vec2 extent = vec2(textureSize(sun_shadow_map0, 0));
    float id = gl_FragCoord.x + 256.0 * floor(gl_FragCoord.y);
    vec2 uv = fract(vec2(id * 0.6180339, id * 0.4142135));
    if (test_kind == 0) {
        uv = uv * (1.0 + 2.0 / extent) - 1.0 / extent;
    }
    if (test_kind == 1) {
        uv = (floor(uv * extent) + 0.5) / extent + vec2(mod(id, 3.0) - 1.0) * 0.0000001;
    }
    float bias = 0.00025;
    float depth =
        test_kind == 2 ? 0.2 + dot(test_slope, uv) + (mod(id, 3.0) - 1.0) * 0.0000001 : 0.5;
    vec3 p = vec3(uv, depth);
    float a = shadow_pcf_unique(sun_shadow_map0, p, bias, test_slope, tent_kernel != 0);
    float b = shadow_pcf_reference(sun_shadow_map0, p, bias, test_slope, tent_kernel != 0);
    // Coplanar receivers retain the real bias rather than cancelling it to
    // force rounding ties. Bias must keep the plane lit with either kernel.
    bool failed = abs(a - b) > 0.00002 || (test_kind == 2 && a < 0.99998);
    result = vec4(a, b, failed ? 1.0 : 0.0, 1.0);
}
)";
    if (gl::is_vulkan()) {
        fragment.insert(fragment.find('\n') + 1, "#define OPENRAC_SHADOW_GATHER 1\n");
    }
    CHECK(shader.build("unique shadow filter comparison", vertex, fragment, error));
    unsigned map = 0, vao = 0;
    glGenTextures(1, &map);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0 + 7);
    glBindTexture(GL_TEXTURE_2D, map);
    shader.use();
    glUniform1i(shader.uniform("sun_shadow_map0"), 7);
    for (const auto extent :
         {std::array{1, 1},
          std::array{2, 1},
          std::array{7, 5},
          std::array{32, 16},
          std::array{1024, 1024}}) {
        for (const auto slope :
             {std::array{0.f, 0.f},
              std::array{.4f, 0.f},
              std::array{-.3f, .2f},
              std::array{2.f, -2.f}}) {
            for (int kind = 0; kind < 3; ++kind) {
                std::vector<float> depths(std::size_t(extent[0] * extent[1]) * 4);
                for (int y = 0; y < extent[1]; ++y) {
                    for (int x = 0; x < extent[0]; ++x) {
                        float value = kind == 2
                                          ? .2f + slope[0] * (float(x) + .5f) / float(extent[0])
                                                + slope[1] * (float(y) + .5f) / float(extent[1])
                                          : float((x * 317 + y * 149 + 71) % 1024) / 1024;
                        depths[std::size_t(y * extent[0] + x) * 4] = value;
                    }
                }
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    0x8814,
                    extent[0],
                    extent[1],
                    0,
                    GL_RGBA,
                    GL_FLOAT,
                    depths.data()
                );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glUniform1i(shader.uniform("test_kind"), kind);
                glUniform2f(shader.uniform("test_slope"), slope[0], slope[1]);
                for (int tent = 0; tent < 2; ++tent) {
                    glUniform1i(shader.uniform("tent_kernel"), tent);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                    const auto pixels = frame.read_rgba();
                    unsigned mismatches = 0;
                    for (std::size_t i = 0; i < pixels.size(); i += 4) {
                        mismatches +=
                            pixels[i + 2] != 0 || std::abs(int(pixels[i]) - int(pixels[i + 1])) > 1;
                    }
                    if (mismatches) {
                        std::printf(
                            "filter mismatch: %dx%d slope %.2f,%.2f kind %d tent %d: %u\n",
                            extent[0],
                            extent[1],
                            slope[0],
                            slope[1],
                            kind,
                            tent,
                            mismatches
                        );
                    }
                    CHECK(mismatches == 0);
                }
            }
        }
    }
    CHECK(glGetError() == GL_NO_ERROR);
    glDeleteTextures(1, &map);
    glDeleteVertexArrays(1, &vao);
    glActiveTexture(GL_TEXTURE0);
}

void gpu() {
    scene::LevelData level;
    level.light_sets.resize(256);
    level.light_sets[0] = .6f;  // red sun
    level.light_sets[9] = .4f;  // green directional fill
    level.light_sets[4] = level.light_sets[12] = 1;
    level.light_sets[6] = level.light_sets[14] = -1;
    const renderer::Vec3 points[] =
        {{-300, -300, 0},
         {300, -300, 0},
         {300, 300, 0},
         {-300, 300, 0},
         {-3, -3, 8},
         {3, -3, 8},
         {3, 3, 8},
         {-3, 3, 8}};
    for (auto p : points) {
        scene::Vertex v{};
        std::copy(p.begin(), p.end(), v.position);
        v.normal[2] = 1;
        v.light_slot = -1;
        v.weights[0] = 1;
        for (auto& n : v.colour) {
            n = 1;
        }
        level.vertices.push_back(v);
    }
    level.indices = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
    level.materials.push_back({});
    scene::Model plane, block;
    plane.primitives.push_back({0, 6, 0});
    plane.min = {-300, -300, 0};
    plane.max = {300, 300, 0};
    block.primitives.push_back({6, 6, 0});
    block.min = {-3, -3, 8};
    block.max = {3, 3, 8};
    level.models = {plane, block};
    level.instances[static_cast<std::size_t>(scene::Layer::Terrain)].push_back({});
    scene::Instance caster;
    caster.model = 1;
    caster.occlusion = 1;
    // Mark this caster hidden in every camera cell used by the fixture.
    // All the shadow image checks below must still pass: camera PVS is never
    // a shadow-caster list, even when it hides an upstream object.
    level.pvs.grid.resize(192 + 128);
    auto put_pvs = [&](unsigned at, unsigned value) {
        level.pvs.grid[at] = static_cast<std::uint8_t>(value);
        level.pvs.grid[at + 1] = static_cast<std::uint8_t>(value >> 8);
    };
    put_pvs(0, 192);
    put_pvs(4, 0);
    put_pvs(6, 70);
    for (unsigned z = 0; z < 70; ++z) {
        put_pvs(8 + z * 2, 38);
    }
    put_pvs(152, 0);
    put_pvs(154, 1);
    put_pvs(156, 40);
    put_pvs(160, 0);
    put_pvs(162, 1);
    put_pvs(164, 0);
    CHECK(!scene::Pvs::visible(level.pvs.select({0, 0, 12}), caster.occlusion));
    level.instances[static_cast<std::size_t>(scene::Layer::Ties)].push_back(caster);
    renderer::Renderer renderer;
    scene::LevelScene scene;
    std::string error;
    if (!scene.upload(level, renderer.textures(), error)) {
        std::fprintf(stderr, "scene upload: %s\n", error.c_str());
        CHECK(false);
        return;
    }
    lighting::ProbeGrid grid;
    grid.origin = {-512, -512, -512};
    grid.spacing = {1024, 1024, 1024};
    auto probes = lighting::ProbeVolume::allocate(grid);
    for (auto& p : probes.probes) {
        p.valid = true;
    }
    for (auto& e : probes.irradiance) {
        e.fill(.2f * std::numbers::pi_v<float>);
    }
    CHECK(scene.set_probe_volume(openrac::test::gpu_probes(probes), error));
    scene.set_probe_view(3);
    CHECK(test::shadow_probe_fixture(scene, error));
    CHECK(scene.enable_sun_shadow(true, error));
    CHECK(
        renderer.add(std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Terrain), error)
    );
    CHECK(renderer.init(error));
    // Keep the tested shadow interior several half-resolution samples wide,
    // including the far cascade; this tests the 0.1 floor rather than its edge.
    constexpr int size = 512, middle = size / 2;
    renderer::FrameBuffer target;
    CHECK(target.create(size, size, error));
    renderer::FrameInput input;
    const renderer::RenderTarget rt{target.id(), size, size};
    // Modern shadows must skip stencil generation/compositing, even if the
    // game still submits valid retail proxy capsules.
    renderer::ShadowCapsule capsule{{0, 0, 2}, {0, 0, 3}, .5f, .5f, 4, 4};
    CHECK(!renderer::shadow_volumes(std::span(&capsule, 1), {1, 0, -4}).empty());
    renderer::FrameInput stencil_input;
    stencil_input.shadow_capsules = std::span(&capsule, 1);
    stencil_input.shadow_extrusion = {1, 0, -4};
    renderer::RenderState state{renderer.textures(), rt, {}};
    renderer::ShadowRenderer stencil;
    stencil_input.sun_pixel_shadow = true;
    stencil.render(stencil_input, state);
    stencil_input.sun_pixel_shadow = false;
    stencil_input.hero_pixel_shadow = true;
    stencil.render(stencil_input, state);
    CHECK(state.stats.draw_calls == 0);
    auto solid_reference = [](bool off) {
        stats::probe("OPENRAC_SHADOW_SOLID_OFF", "Opaque shadow shader reference");
        for (auto* probe : stats::probes()) {
            if (std::strcmp(probe->variable, "OPENRAC_SHADOW_SOLID_OFF") == 0) {
                probe->on = off;
                return;
            }
        }
        CHECK(false);
    };
    solid_reference(false);
    auto receiver_reference = [](bool off) {
        for (auto* probe : stats::probes()) {
            if (std::strcmp(probe->variable, "OPENRAC_SUN_RECEIVER_CULL_OFF") == 0) {
                probe->on = off;
                return;
            }
        }
        CHECK(false);
    };
    // A receiver in each cascade: verify actual depth sampling and 0.1 floor
    // independently for red primary and green fill, in linear light.
    const auto cascade_ends =
        scene::sun_cascades(camera(12), {1, 0, -1}, {-300, -300, 0}, {300, 300, 8}).ends;
    // The first near interval is too small for the 8-unit-high fixture caster,
    // so test it near its far end, and sample the remaining interval midpoints.
    for (float height :
         {cascade_ends[0] - .5f,
          (cascade_ends[0] + cascade_ends[1]) * .5f,
          (cascade_ends[1] + cascade_ends[2]) * .5f,
          (cascade_ends[2] + cascade_ends[3]) * .5f}) {
        input.camera = camera(height);
        input.sun_pixel_shadow = false;
        renderer.render(input, rt);
        const auto lit = target.read_rgba();
        input.sun_pixel_shadow = true;
        renderer.render(input, rt);
        const auto shadow = target.read_rgba();
        // Receiver-footprint culling must preserve the square-map result.
        receiver_reference(true);
        renderer.render(input, rt);
        CHECK(target.read_rgba() == shadow);
        receiver_reference(false);
        renderer.render(input, rt);
        std::uint64_t solid_draws = 0;
        for (const auto& counts : scene.sun_culling()) {
            solid_draws += counts.solid_draws;
        }
        CHECK(solid_draws > 0);
        solid_reference(true);
        renderer.render(input, rt);
        CHECK(target.read_rgba() == shadow);
        for (const auto& counts : scene.sun_culling()) {
            CHECK(counts.solid_draws == 0);
        }
        solid_reference(false);
        const int x = static_cast<int>(middle + 8 / height * middle);
        const auto at = static_cast<std::size_t>((middle * size + x) * 4);
        for (unsigned channel = 0; channel < 2; ++channel) {
            const float ratio = linear(shadow[at + channel]) / linear(lit[at + channel]);
            CHECK(ratio > .08f && ratio < .13f);
        }
    }
    // Blending at each split cannot introduce a dark/light seam on a fully
    // shadowed receiver. The final cascade fades smoothly toward unshadowed.
    const auto splits =
        scene::sun_cascades(camera(12), {1, 0, -1}, {-300, -300, 0}, {300, 300, 8}).ends;
    for (unsigned cascade = 0; cascade < 3; ++cascade) {
        for (float delta : {-.01f, .01f}) {
            const float height = splits[cascade] + delta;
            input.camera = camera(height);
            renderer.render(input, rt);
            const auto rgba = target.read_rgba();
            const int x = static_cast<int>(middle + 8 / height * middle);
            const auto at = static_cast<std::size_t>((middle * size + x) * 4);
            CHECK(linear(rgba[at]) > .035f && linear(rgba[at]) < .055f);
        }
    }
    float previous = 0;
    for (float height : {240.f, 248.f, 255.f, 260.f}) {
        input.camera = camera(height);
        renderer.render(input, rt);
        const int x = static_cast<int>(middle + 8 / height * middle);
        const float value = linear(target.read_rgba()[(middle * size + x) * 4]);
        CHECK(value >= previous - .01f);
        previous = value;
    }
    CHECK(previous > .4f);  // fully lit beyond the shadow distance
    // Combined mode preserves the independent baked indirect contribution.
    scene.set_probe_view(1);
    scene.set_probe_lighting(true, 1.0f);
    input.camera = camera(12);
    renderer.render(input, rt);
    CHECK(
        std::abs(
            linear(target.read_rgba()[(middle * size + size * 106 / 128) * 4])
            - (.2f + .6f / std::sqrt(2.f) * .1f)
        )
        < .015f
    );
    // Indirect-only diagnostics remain unchanged when maps are enabled.
    input.camera = camera(12);
    scene.set_probe_view(2);
    input.sun_pixel_shadow = false;
    renderer.render(input, rt);
    const auto gi = target.read_rgba();
    input.sun_pixel_shadow = true;
    renderer.render(input, rt);
    CHECK(gi == target.read_rgba());
    scene.set_probe_view(3);
    // An off-camera caster above/behind the camera still shadows the origin.
    caster.matrix = renderer::translation({-30, 0, 22});
    scene.set_instances(scene::Layer::Ties, {caster});
    renderer.render(input, rt);
    const auto offscreen = target.read_rgba();
    CHECK(linear(offscreen[(middle * size + middle) * 4]) < .06f);
    if (gl::is_vulkan()) {
        // These copies include space inside the square maps but outside the
        // receiver footprints. Prove actual work is removed without changing
        // either CPU-reference or GPU-driven shadow pixels.
        std::vector<scene::Instance> field{caster};
        for (int x = -120; x <= 120; x += 30) {
            for (int y = -120; y <= 120; y += 30) {
                auto copy = caster;
                copy.matrix = renderer::translation({float(x), float(y), 22});
                field.push_back(copy);
            }
        }
        scene.set_instances(scene::Layer::Ties, field);
        receiver_reference(true);
        renderer.render(input, rt);
        const auto square = target.read_rgba();
        const auto square_triangles = renderer.last_stats().triangles;
        receiver_reference(false);
        renderer.render(input, rt);
        CHECK(target.read_rgba() == square);
        CHECK(renderer.last_stats().triangles < square_triangles);
        test::gpu_draw_reference(false);
        renderer.render(input, rt);
        CHECK(target.read_rgba() == square);
        test::gpu_draw_reference(true);
    }
    // One class spread across the level: retain the upstream off-camera
    // caster, reject distant copies, compact survivors per cascade, and keep
    // the resulting shadow image identical to the aggregate-group reference.
    auto culling_reference = [](bool off) {
        for (auto* probe : stats::probes()) {
            if (std::strcmp(probe->variable, "OPENRAC_SUN_INSTANCE_CULL_OFF") == 0) {
                probe->on = off;
                return;
            }
        }
        CHECK(false);
    };
    std::vector<scene::Instance> spread{caster};
    for (int i = 0; i < 20; ++i) {
        auto distant = caster;
        distant.matrix = renderer::multiply(
            renderer::translation({float(2000 + i * 20), float(i * 50), 0}),
            renderer::scale(i % 2 ? -2.f : .5f)
        );
        spread.push_back(distant);
    }
    scene.set_instances(scene::Layer::Ties, spread);
    culling_reference(true);
    renderer.render(input, rt);
    const auto group_reference = target.read_rgba();
    const auto reference_triangles = renderer.last_stats().triangles;
    culling_reference(false);
    renderer.render(input, rt);
    CHECK(target.read_rgba() == group_reference);
    CHECK(renderer.last_stats().triangles < reference_triangles);
    for (const auto& counts : scene.sun_culling()) {
        CHECK(counts.rejected >= 20);
        CHECK(counts.compacted_groups >= 1);
        CHECK(counts.submitted >= 1);
        CHECK(counts.candidates == counts.rejected + counts.submitted);
    }
    // The colour instance buffer is independent of shadow compaction. Draw
    // the same class normally and compare both culling paths again.
    auto* ties_colour =
        renderer.add(std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Ties), error);
    CHECK(ties_colour);
    culling_reference(true);
    renderer.render(input, rt);
    const auto colour_reference = target.read_rgba();
    culling_reference(false);
    renderer.render(input, rt);
    CHECK(target.read_rgba() == colour_reference);
    ties_colour->enabled = false;
    // Rigid live transforms are refreshed rather than reusing stale bounds.
    for (auto& instance : spread) {
        instance.matrix = renderer::translation({10000, 10000, 0});
    }
    scene.set_instances(scene::Layer::Ties, spread);
    renderer.render(input, rt);
    for (const auto& counts : scene.sun_culling()) {
        CHECK(counts.compacted_groups == 0);
    }
    // A skinned caster can be outside by its instance transform while its
    // current joint pose puts it over the receiver. Never use a bind-pose box.
    auto animated = caster;
    animated.matrix = renderer::translation({10000, 10000, 0});
    animated.palette = 0;
    scene.set_palette({renderer::translation({-10030, -10000, 22})});
    scene.set_instances(scene::Layer::Mobys, {animated});
    culling_reference(true);
    renderer.render(input, rt);
    const auto posed_reference = target.read_rgba();
    culling_reference(false);
    renderer.render(input, rt);
    CHECK(target.read_rgba() == posed_reference);
    CHECK(linear(posed_reference[(middle * size + middle) * 4]) < .06f);
    for (const auto& counts : scene.sun_culling()) {
        CHECK(counts.unbounded >= 1);
    }
    // A world sphere contains the animated pose even though the model origin
    // is far away. Independent light-frustum culling retains its off-camera
    // shadow, rejects distant animated copies and refreshes on the next upload.
    animated.world_sphere = {-30, 0, 22, 8};
    auto remote = animated;
    remote.matrix = renderer::translation({20000, 20000, 0});
    remote.world_sphere = {9970, 10000, 22, 8};
    scene.set_instances(scene::Layer::Mobys, {animated, remote});
    culling_reference(true);
    renderer.render(input, rt);
    const auto sphere_reference = target.read_rgba();
    const auto sphere_triangles = renderer.last_stats().triangles;
    culling_reference(false);
    renderer.render(input, rt);
    CHECK(target.read_rgba() == sphere_reference);
    CHECK(renderer.last_stats().triangles < sphere_triangles);
    CHECK(linear(sphere_reference[(middle * size + middle) * 4]) < .06f);
    for (const auto& counts : scene.sun_culling()) {
        CHECK(counts.rejected >= 1);
        CHECK(counts.unbounded == 0);
    }
    scene.set_instances(scene::Layer::Mobys, {remote});
    renderer.render(input, rt);
    CHECK(linear(target.read_rgba()[(middle * size + middle) * 4]) > .3f);
    scene.set_instances(scene::Layer::Mobys, {});
    scene.set_instances(scene::Layer::Ties, {caster});
    // Disabling the modern route restores the unshadowed result.
    input.sun_pixel_shadow = false;
    renderer.render(input, rt);
    CHECK(linear(target.read_rgba()[(middle * size + middle) * 4]) > .3f);
    // Alpha-tested depth casting and the route without a probe asset.
    scene.release();
    scene::Material transparent;
    transparent.colour[3] = 0;
    transparent.cutout = true;
    level.materials.push_back(transparent);
    level.models[1].primitives[0].material = 1;
    CHECK(scene.upload(level, renderer.textures(), error));
    CHECK(test::shadow_probe_fixture(scene, error));
    CHECK(scene.enable_sun_shadow(true, error));
    input.sun_pixel_shadow = true;
    renderer.render(input, rt);
    const auto clear = target.read_rgba();
    CHECK(clear[(middle * size + size * 106 / 128) * 4] > 240);
    scene.release();
    level.models[1].primitives[0].material = 0;
    CHECK(scene.upload(level, renderer.textures(), error));
    CHECK(test::shadow_probe_fixture(scene, error));
    CHECK(scene.enable_sun_shadow(true, error));
    renderer.render(input, rt);
    CHECK(
        std::abs(linear(target.read_rgba()[(middle * size + size * 106 / 128) * 4]) - .1f) < .01f
    );
    // A nominally opaque material can have vertex or instance alpha holes.
    // Both must stay on the reference shader and retain the same coverage.
    for (unsigned mode = 0; mode < 7; ++mode) {
        scene.release();
        level.materials[0].cutout = false;
        level.materials[0].image = -1;
        level.models[1].primitives[0].material = 0;
        for (unsigned i = 4; i < 8; ++i) {
            level.vertices[i].colour[3] = mode == 0 ? 0.25f : mode == 3 ? 0.5f : 1.0f;
        }
        if (mode == 4) {
            level.vertices[4].colour[3] = 0.25f;
        }
        if (mode == 6) {
            level.vertices[5].colour[3] = 0.25f;
            level.models[1].primitives = {{6, 3, 0}, {9, 3, 0}};
        }
        auto& placement = level.instances[static_cast<std::size_t>(scene::Layer::Ties)][0];
        placement.tint[3] = mode == 1 ? 0.25f : mode == 5 ? 0.5f : 1.0f;
        if (mode == 2) {
            level.materials[1] = scene::Material{};
            level.materials[1].cutout = true;
            level.materials[1].image = 0;
            level.models[1].primitives[0].material = 1;
            renderer::Rgba8Image image(2, 1);
            image.set_texel(0, 0, 0x00ffffffu);
            image.set_texel(1, 0, 0xffffffffu);
            level.images = {image};
            level.image_paths = {"/textures/shadow-cutout.png"};
            level.vertices[4].uv[0] = level.vertices[7].uv[0] = 0;
            level.vertices[5].uv[0] = level.vertices[6].uv[0] = 1;
        }
        CHECK(scene.upload(level, renderer.textures(), error));
        CHECK(test::shadow_probe_fixture(scene, error));
        CHECK(scene.enable_sun_shadow(true, error));
        solid_reference(true);
        renderer.render(input, rt);
        const auto alpha_reference = target.read_rgba();
        solid_reference(false);
        renderer.render(input, rt);
        CHECK(target.read_rgba() == alpha_reference);
        if (mode == 2) {
            // Keep the receiver opaque: the cutout must actually produce both
            // shadowed and unshadowed floor, rather than an empty comparison.
            unsigned minimum = 255, maximum = 0;
            for (std::size_t i = 0; i < alpha_reference.size(); i += 4) {
                minimum = std::min(minimum, unsigned(alpha_reference[i]));
                maximum = std::max(maximum, unsigned(alpha_reference[i]));
            }
            CHECK(minimum < 100 && maximum > 240);
        }
        std::uint64_t alpha_draws = 0;
        for (const auto& counts : scene.sun_culling()) {
            alpha_draws += counts.alpha_draws;
        }
        if (mode != 3) {
            CHECK(alpha_draws > 0);
        }
        if (mode == 6) {
            std::uint64_t solid_draws = 0;
            for (const auto& counts : scene.sun_culling()) {
                solid_draws += counts.solid_draws;
            }
            CHECK(solid_draws > 0);
        }
        if (gl::is_vulkan()) {
            test::gpu_draw_reference(false);
            CHECK(test::gpu_draw_active());
            renderer.render(input, rt);
            CHECK(target.read_rgba() == alpha_reference);
            test::gpu_draw_reference(true);
        }
    }
    if (gl::is_vulkan()) {
#if defined(OPENRAC_VULKAN)
        scene.release();
        level.models.resize(2);
        level.materials[0] = scene::Material{};
        level.materials[1] = scene::Material{};
        level.materials[1].cutout = true;
        level.materials[1].image = 0;
        for (unsigned i = 4; i < 8; ++i) {
            level.vertices[i].colour[3] = 1;
        }
        auto& casters = level.instances[static_cast<std::size_t>(scene::Layer::Ties)];
        casters.clear();
        // Alternate opaque/cutout ranges in many distinct model groups. All
        // use the shared geometry and GPU instance page, but model-order
        // submission would switch shader at each range.
        for (unsigned i = 0; i < 24; ++i) {
            auto model = block;
            model.primitives = {{6, 3, 0}, {9, 3, 1}};
            auto instance = caster;
            instance.model = static_cast<unsigned>(level.models.size());
            instance.matrix = renderer::translation(
                {static_cast<float>(i % 6) * 4 - 10, static_cast<float>(i / 6) * 4 - 6, 0}
            );
            level.models.push_back(model);
            casters.push_back(instance);
        }
        CHECK(scene.upload(level, renderer.textures(), error));
        CHECK(test::shadow_probe_fixture(scene, error));
        CHECK(scene.enable_sun_shadow(true, error));
        test::gpu_draw_reference(false);
        const auto reference = [](bool off) {
            stats::probe("OPENRAC_SHADOW_BATCH_OFF", "Shadow model-order reference");
            for (auto* probe : stats::probes()) {
                if (std::strcmp(probe->variable, "OPENRAC_SHADOW_BATCH_OFF") == 0) {
                    probe->on = off;
                }
            }
        };
        reference(true);
        const auto set_probe = [](const char* name, bool on) {
            stats::probe(name, "Shadow submission test reference");
            for (auto* probe : stats::probes()) {
                if (std::strcmp(probe->variable, name) == 0) {
                    probe->on = on;
                }
            }
        };
        set_probe("OPENRAC_SHADOW_ATLAS_OFF", true);
        set_probe("OPENRAC_SHADOW_LAYER_REFERENCE", true);
        const auto before_reference = vkgl::gpu_material_counters().native_draws;
        renderer.render(input, rt);
        const auto pixels = target.read_rgba();
        const auto reference_calls = vkgl::gpu_material_counters().native_draws - before_reference;
        const auto triangles = renderer.last_stats().triangles;
        reference(false);
        set_probe("OPENRAC_SHADOW_LAYER_REFERENCE", false);
        const auto before_batched = vkgl::gpu_material_counters().native_draws;
        renderer.render(input, rt);
        CHECK(target.read_rgba() == pixels);
        const auto batched_calls = vkgl::gpu_material_counters().native_draws - before_batched;
        CHECK(renderer.last_stats().triangles == triangles);
        CHECK(batched_calls < reference_calls / 2);
        set_probe("OPENRAC_SHADOW_ATLAS_OFF", false);
        const auto before_atlas = vkgl::gpu_material_counters().native_draws;
        renderer.render(input, rt);
        CHECK(target.read_rgba() == pixels);
        const auto atlas_calls = vkgl::gpu_material_counters().native_draws - before_atlas;
        CHECK(renderer.last_stats().triangles == triangles);
        if (vkgl::indexed_viewports_supported(5)) {
            CHECK(scene.shadow_atlas_active());
            CHECK(atlas_calls < batched_calls);
        }
        std::printf(
            "shadow batching: %llu -> %llu native calls, identical pixels and triangles\n",
            static_cast<unsigned long long>(reference_calls),
            static_cast<unsigned long long>(batched_calls)
        );
        std::printf(
            "shadow atlas: %llu native calls, identical pixels and triangles\n",
            static_cast<unsigned long long>(atlas_calls)
        );
        // Share the opaque colour rounds across enabled static layers. Keep
        // both alpha-tested and blended texels, and compare against the old
        // layer order using exactly the same depth prepass and GPU culling.
        renderer.add(std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Shrubs), error);
        auto& shrubs = level.instances[static_cast<std::size_t>(scene::Layer::Shrubs)];
        shrubs.assign(casters.begin() + 12, casters.end());
        casters.resize(12);
        // The first layer is entirely solid, so its opaque work can join
        // the next layer before that layer's translucent boundary.
        for (const auto& instance : casters) {
            level.models[instance.model].primitives[1].material = 0;
        }
        CHECK(scene.upload(level, renderer.textures(), error));
        CHECK(test::shadow_probe_fixture(scene, error));
        CHECK(scene.enable_sun_shadow(true, error));
        input.after_world_depth = [](unsigned depth, int, int) { CHECK(depth != 0); };
        CHECK(scene.batches_opaque_colour());
        stats::set_enabled(stats::Client::Overlay, true);
        stats::begin_frame(stats::FrameKind::Tick, 0);
        set_probe("OPENRAC_OPAQUE_COLOUR_BATCH_OFF", true);
        const auto colour_reference_before = vkgl::gpu_material_counters().native_draws;
        renderer.render(input, rt);
        const auto colour_pixels = target.read_rgba();
        const auto colour_reference_calls =
            vkgl::gpu_material_counters().native_draws - colour_reference_before;
        const auto colour_triangles = renderer.last_stats().triangles;
        vkgl::present();
        stats::begin_frame(stats::FrameKind::Tick, 1);
        set_probe("OPENRAC_OPAQUE_COLOUR_BATCH_OFF", false);
        const auto colour_batch_before = vkgl::gpu_material_counters().native_draws;
        renderer.render(input, rt);
        CHECK(target.read_rgba() == colour_pixels);
        CHECK(renderer.last_stats().triangles == colour_triangles);
        const auto colour_batch_calls =
            vkgl::gpu_material_counters().native_draws - colour_batch_before;
        vkgl::present();
        stats::begin_frame(stats::FrameKind::Tick, 2);
        stats::set_enabled(stats::Client::Overlay, false);
        CHECK(colour_batch_calls <= colour_reference_calls);
        // Specialisation may add program boundaries; compare against the
        // combined shader independently of the per-layer reference above.
        set_probe("OPENRAC_OPAQUE_SPECIALISE_OFF", true);
        const auto combined_before = vkgl::gpu_material_counters().native_draws;
        renderer.render(input, rt);
        CHECK(target.read_rgba() == colour_pixels);
        CHECK(renderer.last_stats().triangles == colour_triangles);
        const auto combined_calls = vkgl::gpu_material_counters().native_draws - combined_before;
        vkgl::present();
        set_probe("OPENRAC_OPAQUE_SPECIALISE_OFF", false);
        std::printf("opaque shaders: %llu combined, %llu specialised native calls; identical pixels\n",
                    static_cast<unsigned long long>(combined_calls),
                    static_cast<unsigned long long>(colour_batch_calls));
        std::printf(
            "opaque colour: %llu -> %llu native calls, identical pixels and triangles\n",
            static_cast<unsigned long long>(colour_reference_calls),
            static_cast<unsigned long long>(colour_batch_calls)
        );
        test::gpu_draw_reference(true);
#endif
    }
    CHECK(glGetError() == GL_NO_ERROR);
    renderer.clear_renderers();
    scene.release();
    target.release();
}

// Returns the static shadow cache's key of cascade `i` of `cascades` at static generation 1.
scene::StaticShadowKey static_key(const scene::SunCascades& cascades, unsigned i) {
    scene::StaticShadowKey key;
    key.valid = true;
    key.matrix = cascades.matrices[i];
    key.planes = cascades.casters[i].planes;
    key.plane_count = cascades.casters[i].plane_count;
    key.generation = 1;
    return key;
}

// Tests the static shadow cache's keys (R-18): the same camera and sun give the same key, and a
// change of the sun, of the static layers, of the hero's casters or of a matrix's last bit misses.
void test_static_shadow_keys() {
    const auto c = camera(12);
    const auto a = scene::sun_cascades(c, {1, 0, -1}, {-100, -100, -10}, {100, 100, 200});
    const auto b = scene::sun_cascades(c, {1, 0, -1}, {-100, -100, -10}, {100, 100, 200});
    const auto sun = scene::sun_cascades(c, {1, 0.01f, -1}, {-100, -100, -10}, {100, 100, 200});

    // Each cascade against itself and against the changes.
    for (unsigned i = 0; i < scene::kSunCascades; ++i) {
        const auto key = static_key(a, i);
        CHECK(scene::same_static_key(key, static_key(b, i)));
        CHECK(!scene::same_static_key(key, static_key(sun, i)));
        CHECK(!scene::same_static_key(key, scene::StaticShadowKey{}));
        auto level = key;
        ++level.generation;
        CHECK(!scene::same_static_key(key, level));
        auto hero = key;
        hero.separate_hero = true;
        CHECK(!scene::same_static_key(key, hero));
        auto nudged = key;
        nudged.matrix[12] = std::nextafter(nudged.matrix[12], 1e30f);
        CHECK(!scene::same_static_key(key, nudged));
    }
}

// Tests that a camera moved far less than a texel keeps some cascades' snapped matrices but no
// cascade's caster volume, which follows the camera: why a moving camera misses the static cache.
void test_static_shadow_keys_follow_the_camera() {
    const auto still = camera(12);
    auto moved = camera(12);
    constexpr float kStep = 1e-4f;  // far under a texel of the nearest cascade
    moved.position[0] += kStep;
    moved.view = renderer::look_along(moved.position, {0, 0, -1}, {0, 1, 0});
    const auto a = scene::sun_cascades(still, {1, 0, -1}, {-100, -100, -10}, {100, 100, 200});
    const auto b = scene::sun_cascades(moved, {1, 0, -1}, {-100, -100, -10}, {100, 100, 200});
    unsigned same_matrix = 0;
    unsigned same_key = 0;

    // Each cascade of the two frames.
    for (unsigned i = 0; i < scene::kSunCascades; ++i) {
        same_matrix += a.matrices[i] == b.matrices[i] ? 1u : 0u;
        same_key += scene::same_static_key(static_key(a, i), static_key(b, i)) ? 1u : 0u;
    }

    std::printf(
        "static shadow keys: %u of 4 matrices and %u of 4 keys kept after a %.4f step\n",
        same_matrix,
        same_key,
        static_cast<double>(kStep)
    );
    CHECK(same_matrix > 0);
    CHECK(same_key == 0);
}

int main(int argc, char**) {
    test::gpu_draw_reference(true);
    cpu();
    caster_hulls();
    test_static_shadow_keys();
    test_static_shadow_keys_follow_the_camera();
    if (argc > 1) {
        test::GlContext context;
        std::string why;
        if (!context.open(why)) {
            std::printf("SKIP: %s\n", why.c_str());
            return test::kSkip;
        }
        filter_gpu();
        unique_filter_gpu();
        gpu();
    }
    return test::result();
}
