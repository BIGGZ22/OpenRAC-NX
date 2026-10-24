// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>

#include "enhanced/lighting/probes.h"
#include "enhanced/renderer/gtao.h"
#include "enhanced/tests/gpu_probe_fixture.h"
#include "enhanced/viewer/probe_lighting.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "scene/level_renderer.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"
#include "viewer_shaders.h"
using namespace openrac;
using namespace openrac::gl;

namespace {
using lighting::Float3;
renderer::Shader* zero_weight_reference = nullptr;
constexpr const char* vertex = R"glsl(#version 410 core
void main() {
    vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)glsl";
const std::array<std::array<float, 4>, 8> positions{
    {{8, 8, 8, 1},
     {0, 0, 0, 1},
     {16, 16, 16, 1},
     {-1, 8, 8, 1},
     {4, 9, 3, 1},
     {5, 8, 8, 1},
     {8, 8, 8, 1},
     {16, 16, 15, 1}}
};
const std::array<std::array<float, 4>, 8> normals{
    {{0, 0, 1, 0},
     {0, 0, -1, 0},
     {1, 0, 0, 0},
     {0, 0, 1, 0},
     {-1, -1, -1, 0},
     {1, 2, -1, 0},
     {0, 0, 0, 0},
     {0, 0, 1, 0}}
};

// Model atlas precision separately from the CPU interpolation reference. The
// fifth-root encoding magnifies packed-float rounding when decoded back to E.
lighting::ProbeVolume packed_reference(lighting::ProbeVolume volume) {
    for (auto& texel : volume.irradiance) {
        for (unsigned c = 0; c < 3; ++c) {
            const float encoded = lighting::encode_probe_irradiance(texel[c]);
            int exponent = 0;
            std::frexp(encoded, &exponent);
            const int mantissa_bits = c == 2 ? 5 : 6;
            const float step = std::ldexp(1.0f, std::max(exponent - 1, -14) - mantissa_bits);
            const float rounded = std::nearbyint(encoded / step) * step;
            texel[c] = std::pow(rounded, lighting::kProbeIrradianceGamma);
        }
    }
    return volume;
}

void check(
    lighting::ProbeVolume& volume,
    renderer::Shader& shader,
    renderer::FrameBuffer& frame,
    unsigned vao,
    lighting::ProbeVolume* dense = nullptr,
    bool camera_bias = false,
    bool bias_occluder = false
) {
    scene::ProbeLighting probes;
    std::string error;
    const auto packed_dense =
        dense ? std::optional{openrac::test::gpu_probes(*dense)} : std::nullopt;
    CHECK(probes.upload(
        openrac::test::gpu_probes(volume), error, packed_dense ? &*packed_dense : nullptr
    ));
    if (!probes.active()) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return;
    }
    const auto reference_volume = packed_reference(volume);
    const auto reference_dense = dense ? std::optional{packed_reference(*dense)} : std::nullopt;
    // Reuse the uploaded probes while varying each slider independently.
    const std::array<std::array<float, 2>, 4> biases{
        {{0.5f, 0.5f}, {0.5f, 0.0f}, {0.0f, 0.5f}, {0.5f, 0.5f}}
    };
    for (unsigned step = 0; step < (camera_bias ? biases.size() : 1); ++step) {
        probes.normal_bias = biases[step][0];
        probes.view_bias = biases[step][1];
        glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
        glViewport(0, 0, 8, 1);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        shader.use();
        probes.bind(shader, scene::Layer::Terrain);
        glUniform1i(shader.uniform("camera_biased"), camera_bias ? 1 : 0);
        glUniform3f(shader.uniform("test_camera"), 16, 16, 20);
        glUniform4fv(shader.uniform("positions"), 8, positions[0].data());
        glUniform4fv(shader.uniform("normals"), 8, normals[0].data());
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        const auto pixels = frame.read_rgba();
        if (zero_weight_reference) {
            auto& reference = *zero_weight_reference;
            reference.use();
            probes.bind(reference, scene::Layer::Terrain);
            glUniform1i(reference.uniform("camera_biased"), camera_bias ? 1 : 0);
            glUniform3f(reference.uniform("test_camera"), 16, 16, 20);
            glUniform4fv(reference.uniform("positions"), 8, positions[0].data());
            glUniform4fv(reference.uniform("normals"), 8, normals[0].data());
            glDrawArrays(GL_TRIANGLES, 0, 3);
            CHECK(frame.read_rgba() == pixels);
        }
        for (unsigned i = 0; i < 8; ++i) {
            Float3 p{positions[i][0], positions[i][1], positions[i][2]};
            Float3 n{normals[i][0], normals[i][1], normals[i][2]};
            Float3 view_offset{};
            if (camera_bias) {
                const Float3 delta{16 - p[0], 16 - p[1], 20 - p[2]};
                const float length =
                    std::sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
                for (unsigned c = 0; c < 3; ++c) {
                    view_offset[c] = probes.view_bias * delta[c] / length;
                }
            }
            const auto expected = lighting::sample_probe_volumes(
                reference_volume,
                reference_dense ? &*reference_dense : nullptr,
                p,
                n,
                camera_bias ? probes.normal_bias : 0.02f,
                view_offset
            );
            CHECK(pixels[i * 4 + 3] == (expected ? 255 : 0));
            if (!expected) {
                continue;
            }
            for (unsigned c = 0; c < 3; ++c) {
                const float gpu = static_cast<float>(pixels[i * 4 + c]) / 255 * 4;
                CHECK(std::abs(gpu - expected->irradiance[c]) < 0.02f);
            }
        }
        if (bias_occluder) {
            // The final receiver needs both offsets to clear its nearby occluder.
            if (step == 1 || step == 2) {
                CHECK(pixels[7 * 4] < 10);
            } else {
                CHECK(pixels[7 * 4] > 50);
            }
        }
        CHECK(glGetError() == GL_NO_ERROR);
    }
    probes.release();
}

void coverage_seam(
    lighting::ProbeVolume& coarse,
    lighting::ProbeVolume& dense,
    renderer::Shader& shader,
    renderer::FrameBuffer& frame,
    unsigned vao
) {
    const float xs[] = {6, 6.5f, 7, 7.5f, 7.9f, 7.99f, 8, 8.01f};
    std::array<std::array<float, 4>, 8> points{}, directions{};
    for (unsigned i = 0; i < 8; ++i) {
        points[i] = {xs[i], 7, 7, 1};
        directions[i] = {0, 0, 1, 0};
    }
    for (bool occluded : {false, true}) {
        const auto texels = dense.grid.texels_per_probe();
        for (std::size_t i = 0; i < dense.probes.size(); ++i) {
            const bool usable = i % 5 == 1;
            dense.probes[i].valid = usable || occluded;
            for (std::size_t t = 0; t < texels; ++t) {
                dense.distance_moments[i * texels + t] =
                    usable ? lighting::Float2{1024, 1048576} : lighting::Float2{0.01f, 0.0001f};
            }
        }
        scene::ProbeLighting probes;
        std::string error;
        const auto packed_dense = openrac::test::gpu_probes(dense);
        CHECK(probes.upload(openrac::test::gpu_probes(coarse), error, &packed_dense));
        glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
        glViewport(0, 0, 8, 1);
        shader.use();
        probes.bind(shader, scene::Layer::Terrain);
        glUniform1i(shader.uniform("camera_biased"), 0);
        glUniform4fv(shader.uniform("positions"), 8, points[0].data());
        glUniform4fv(shader.uniform("normals"), 8, directions[0].data());
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        const auto pixels = frame.read_rgba();
        for (unsigned i = 0; i < 8; ++i) {
            CHECK(pixels[i * 4 + 3] == 255);
            if (occluded) {
                CHECK(pixels[i * 4] == 0);
                if (i == 2) {
                    CHECK(pixels[i * 4 + 2] >= 63);
                }
            } else {
                CHECK(std::abs(int(pixels[i * 4]) + int(pixels[i * 4 + 2]) - 64) <= 1);
            }
            if (i > 0) {
                CHECK(pixels[i * 4 + 2] <= pixels[(i - 1) * 4 + 2]);
            }
            const auto cpu =
                lighting::sample_probe_volumes(coarse, &dense, {xs[i], 7, 7}, {0, 0, 1});
            CHECK(cpu && std::abs(pixels[i * 4 + 2] / 255.0f * 4 - cpu->irradiance[2]) < 0.02f);
        }
        // Missing probes reach coarse GI; blocked valid probes reach darkness.
        // CPU/GPU parity alone would also accept the old bright fallback.
        CHECK(pixels[2] >= 63);
        if (!occluded) {
            CHECK(pixels[4 * 4 + 2] <= 1);
        }
        CHECK(pixels[5 * 4 + 2] <= 1);
        CHECK(pixels[6 * 4 + 2] == 0);
        CHECK(glGetError() == GL_NO_ERROR);
        probes.release();
    }
}

void material_path(
    scene::Layer layer,
    int blend,
    bool glow,
    bool indirect = false,
    bool authored_light = false,
    bool authored_material = false,
    bool masked = false,
    bool masked_frame = false,
    bool mask_transition = false,
    bool authored_tint = false
) {
    scene::LevelData level;
    level.light_sets.resize(256);
    const Float3 triangle_positions[] = {{-1, -1, 0}, {1, -1, 0}, {0, 1, 0}};
    for (const auto& p : triangle_positions) {
        scene::Vertex vtx{};
        std::copy(p.begin(), p.end(), vtx.position);
        vtx.normal[2] = 1;
        for (unsigned c = 0; c < 3; ++c) {
            vtx.colour[c] = layer == scene::Layer::Mobys ? 0.5f : 0.0f;
        }
        vtx.colour[3] = 1;
        if (authored_tint) {
            vtx.colour[0] = .25f;
            vtx.colour[1] = 1;
            vtx.colour[2] = .5f;
        }
        vtx.light_slot = glow ? -2.0f : -1.0f;
        vtx.uv[0] = masked_frame ? .75f : .25f;
        vtx.uv[1] = .5f;
        level.vertices.push_back(vtx);
    }
    level.indices = {0, 1, 2};
    level.materials.push_back({});
    if (authored_material) {
        level.materials.back().emission = {.25f, .125f, .0625f};
    }
    if (authored_tint) {
        level.images.emplace_back(1, 1);
        level.images.back()
            .pixels = {255, 128, 0, 255};  // yellow source, authored green modulation
        level.image_paths.push_back("authored-ooze");
        auto& material = level.materials.back();
        material.image = material.emission_image = 0;
        material.emission = {1, 1, 1};
        material.emission_vertex_colour = true;
        material.diffuse_scale = 0;
    }
    if (masked) {
        level.images.emplace_back(2, 1);
        level.images.back().pixels = {0, 255, 0, 255, 128, 128, 128, 255};
        if (mask_transition) {
            level.images.back().pixels = {128, 160, 128, 255, 128, 128, 128, 255};
        }
        level.image_paths.push_back("emission-test");
        level.materials.back().emission_image = 0;
        level.materials.back().emission_mask = scene::EmissionMask::Green;
    }
    scene::Model model;
    model.primitives.push_back({0, 3, 0});
    level.models.push_back(model);
    scene::Instance instance;
    if (layer == scene::Layer::Mobys) {
        instance.moby_light = {0, 0, 0, 1};
        instance.moby_ambient = {0, 0, 0, glow ? 256.0f : 0.0f};  // glow RGB = red 255
        if (authored_light || authored_material) {
            if (authored_light) {
                instance.moby_light[3] = -1;
            }
            instance.moby_ambient = {1, .5f, .25f, 0};
        }
        if (authored_tint) {
            instance.moby_ambient = {1, 1, 1, 0};
        }
        instance.blend = blend;
        if (blend == 1) {
            instance.tint[3] = 0.5f;
        }
    }
    level.instances[static_cast<std::size_t>(layer)].push_back(instance);
    renderer::Renderer renderer;
    renderer.set_ambient_occlusion(std::make_unique<renderer::Gtao>());
    scene::LevelScene scene;
    std::string error;
    CHECK(scene.upload(level, renderer.textures(), error));
    scene.set_probe_lighting(true, 1.2f);  // explicit intensity for this fixture
    CHECK(renderer.add(std::make_unique<scene::LayerRenderer>(scene, layer), error));
    CHECK(renderer.init(error));
    renderer::FrameBuffer frame;
    CHECK(frame.create(32, 32, error));
    renderer::FrameInput input;
    input.camera.view = renderer::identity();
    input.camera.projection = renderer::identity();
    input.clear_colour = {0, 0, 0, 1};
    renderer.render(input, {frame.id(), 32, 32});
    auto before = frame.read_rgba();
    const auto at = (16 * 32 + 16) * 4;
    if (!glow && !authored_light && !authored_material) {
        CHECK(before[at] == 0);
    }
    lighting::ProbeGrid grid;
    grid.origin = {-2, -2, -2};
    grid.spacing = {4, 4, 4};
    auto volume = lighting::ProbeVolume::allocate(grid);
    for (auto& p : volume.probes) {
        p.valid = true;
    }
    for (auto& e : volume.irradiance) {
        e = {3.14159265f, 3.14159265f, 3.14159265f};
    }
    CHECK(scene.set_probe_volume(openrac::test::gpu_probes(volume), error));
    if (indirect) {
        scene.set_probe_view(2);
    }
    renderer.render(input, {frame.id(), 32, 32});
    auto after = frame.read_rgba();
    if (authored_tint) {
        CHECK(std::abs(int(after[at]) - 64) <= 1 && std::abs(int(after[at + 1]) - 128) <= 1);
        CHECK(after[at + 2] == 0);
        CHECK(after == before);  // preserve the original GS green, not the yellow texture
        for (auto& e : volume.irradiance) {
            e = {};
        }
        CHECK(scene.set_probe_volume(openrac::test::gpu_probes(volume), error));
        for (float multiplier : {0.f, 5.f}) {
            scene.set_probe_lighting(true, multiplier);
            input.gtao = true;
            renderer.render(input, {frame.id(), 32, 32});
            CHECK(frame.read_rgba() == after);
        }
        return;
    }
    if (authored_light || authored_material) {
        CHECK(before[at] > 100);
        CHECK(after[at + 1] > before[at + 1]);  // diffuse still contributes alongside emission
        // GI exposure cannot dim emission; a black volume isolates that term.
        for (auto& e : volume.irradiance) {
            e = {};
        }
        CHECK(scene.set_probe_volume(openrac::test::gpu_probes(volume), error));
        std::vector<std::uint8_t> isolated;
        for (float multiplier : {0.0f, 5.0f}) {
            scene.set_probe_lighting(true, multiplier);
            renderer.render(input, {frame.id(), 32, 32});
            const auto pixels = frame.read_rgba();
            if (masked_frame) {
                CHECK(pixels[at + 1] == 0);
            } else {
                CHECK(pixels[at + 1] > 0);
            }
            if (authored_material && !masked_frame) {
                // Compare the GLSL mask to the bake's shared CPU definition,
                // including fractional coverage at the green/grey transition.
                auto linear = [](float c) {
                    return c <= .04045f ? c / 12.92f : std::pow((c + .055f) / 1.055f, 2.4f);
                };
                Float3 colour =
                    mask_transition
                        ? Float3{linear(128.f / 255), linear(160.f / 255), linear(128.f / 255)}
                        : (masked ? Float3{0, 1, 0} : Float3{1, 1, 1});
                float coverage = scene::emission_mask(
                    masked ? scene::EmissionMask::Green : scene::EmissionMask::None, colour
                );
                float e = .125f * colour[1] * coverage;
                float display =
                    e <= .0031308f ? e * 12.92f : 1.055f * std::pow(e, 1.f / 2.4f) - .055f;
                CHECK(std::abs(int(pixels[at + 1]) - int(std::round(display * 255))) <= 2);
            }
            if (isolated.empty()) {
                isolated = pixels;
            } else {
                CHECK(pixels == isolated);
            }
        }
        CHECK(after[at + 1] > isolated[at + 1]);
        input.gtao = true;
        renderer.render(input, {frame.id(), 32, 32});
        CHECK(frame.read_rgba() == isolated);  // AO cannot consume the isolated emission term
        input.gtao = false;
        scene.set_probe_view(2);  // GI-only view excludes directly visible emission
        renderer.render(input, {frame.id(), 32, 32});
        CHECK(frame.read_rgba()[at] == 0);
        CHECK(glGetError() == GL_NO_ERROR);
        return;
    }
    if (glow) {
        CHECK(after[at] > 250 && after[at + 1] < 2 && after[at + 2] < 2);
    } else if (blend == 2) {
        CHECK(after[at] == 0 && after[at + 1] == 0 && after[at + 2] == 0);
    } else if (blend == 1) {
        CHECK(after[at] > 99 && after[at] < 105);
    } else if (layer != scene::Layer::Mobys) {
        CHECK(after[at] > 250 && after[at + 1] > 250 && after[at + 2] > 250);
    } else {
        CHECK(after[at] > 200 && after[at] < 207);
    }
    if (blend == 0 && !glow) {
        // Runtime controls preserve the loaded volume and restore the exact
        // original result when disabled, including AO routing.
        scene.set_probe_lighting(false, 1.2f);
        CHECK(!scene.probes_active());
        CHECK(scene.ao_lighting() == renderer::AoLighting::Legacy);
        renderer.render(input, {frame.id(), 32, 32});
        CHECK(frame.read_rgba() == before);
        scene.set_probe_lighting(true, 1.2f);
        CHECK(scene.probes_active());
        CHECK(scene.ao_lighting() == renderer::AoLighting::Indirect);
        renderer.render(input, {frame.id(), 32, 32});
        CHECK(frame.read_rgba() == after);
        scene.set_probe_lighting(true, 0.0f);
        renderer.render(input, {frame.id(), 32, 32});
        CHECK(frame.read_rgba()[at] == 0);
        scene.set_probe_lighting(true, 0.6f);
        renderer.render(input, {frame.id(), 32, 32});
        const auto dimmed = frame.read_rgba();
        CHECK(dimmed[at] > 0 && dimmed[at] < after[at]);
        scene.set_probe_lighting(true, 1.2f);
    }
    if (layer == scene::Layer::Mobys && blend == 0 && !glow) {
        // Direct light must use each fragment's position, rather than the
        // original per-moby colour merge. With black GI, a white point light
        // halfway through its radius supplies about 0.5 * authored tint 0.5.
        for (auto& e : volume.irradiance) {
            e = {0, 0, 0};
        }
        CHECK(scene.set_probe_volume(openrac::test::gpu_probes(volume), error));
        std::array<float, 64> points{};
        points[0] = points[1] = points[2] = 1;
        points[6] = 1;
        points[7] = 2;
        scene.set_point_lights(points);
        renderer.render(input, {frame.id(), 32, 32});
        const auto direct = frame.read_rgba();
        CHECK(direct[at] > 130 && direct[at] < 145);
        scene.set_probe_lighting(true, 0.0f);
        renderer.render(input, {frame.id(), 32, 32});
        CHECK(frame.read_rgba() == direct);  // GI multiplier never changes direct light
    }
    CHECK(glGetError() == GL_NO_ERROR);
    if (layer == scene::Layer::Terrain && !glow && scene::ProbeSelection::supported()) {
        // Exercise the real prepass hook with AO disabled, then enabled. The
        // resolved depth must select green GI before the colour pass shades it.
        auto local_grid = volume.grid;
        local_grid.spacing = {1, 1, 0.5f};
        local_grid.dimensions = {5, 5, 9};
        auto local = lighting::ProbeVolume::allocate(local_grid);
        for (auto& p : local.probes) {
            p.valid = true;
        }
        for (auto& e : local.irradiance) {
            e = {0, 3.14159265f, 0};
        }
        for (auto& m : local.distance_moments) {
            m = {1024, 1024 * 1024};
        }
        CHECK(scene.set_probe_regions(
            openrac::test::gpu_probes(volume), {openrac::test::gpu_probes(local)}, error
        ));
        CHECK(scene.compute_probe_selection());
        scene.update_probe_regions(input.camera, 1);
        input.after_world_depth = [&](unsigned depth, int width, int height) {
            scene.select_visible_probe_regions(depth, input.camera, width, height);
        };
        for (bool ao : {false, true}) {
            input.gtao = ao;
            renderer.render(input, {frame.id(), 32, 32});
            const auto result = frame.read_rgba();
            CHECK(result[at] == 0 && result[at + 1] > 250 && result[at + 2] == 0);
        }
    }
    renderer.clear_renderers();
    scene.release();
    frame.release();
}

void compute_regions(renderer::Shader&, renderer::FrameBuffer&, unsigned vao) {
    if (!scene::ProbeSelection::supported()) {
        std::printf("Tiled probe fixture unavailable on this context\n");
        return;
    }
    lighting::ProbeGrid grid;
    grid.origin = {-4, -4, -4};
    grid.spacing = {4, 4, 4};
    grid.dimensions = {3, 3, 3};
    auto base = lighting::ProbeVolume::allocate(grid);
    const auto fill = [](auto& volume, Float3 colour) {
        for (auto& p : volume.probes) {
            p.valid = true;
        }
        for (auto& e : volume.irradiance) {
            e = colour;
        }
        for (auto& m : volume.distance_moments) {
            m = {1024, 1024 * 1024};
        }
    };
    fill(base, {1, 0, 0});
    std::vector<lighting::ProbeVolume> regions;
    for (int i = 0; i < 35; ++i) {
        auto local = grid;
        local.origin = {i == 0 ? -1.0f : i == 32 ? -0.25f : i == 34 ? 0.5f : 2.0f, -1, -0.5f};
        if (i == 31) {
            local.origin = {-0.25f, -1, 0.5f};  // separate depth layer
        }
        local.spacing = {0.125f, 0.25f, 0.25f};
        local.dimensions = {7, 9, 5};
        if (i == 1) {
            local.origin[0] = -1;
            local.spacing[0] = 0.25f;
            local.dimensions[0] = 4;
        }
        if (i == 2) {
            local.origin[0] = -1;
            local.spacing[0] = 0.0625f;
            local.dimensions[0] = 13;
        }
        auto region = lighting::ProbeVolume::allocate(local);
        fill(
            region,
            i == 0              ? Float3{0, 1, 0}
            : i == 2 || i == 32 ? Float3{0, 0, 1}
            : i == 34           ? Float3{1, 1, 0}
                                : Float3{1, 0, 1}
        );
        regions.push_back(std::move(region));
    }
    scene::ProbeLighting probes;
    std::string error;
    CHECK(probes.upload_regions(
        openrac::test::gpu_probes(base), openrac::test::gpu_regions(regions), error
    ));
    CHECK(probes.compute_selection());
    if (!probes.compute_selection()) {
        probes.release();
        return;
    }
    std::string fragment = "#version 410 core\n";
    fragment += scene::shaders::probes_glsl;
    fragment += R"glsl(uniform sampler2D receiver_depth;
uniform mat4 test_clip;
uniform vec2 test_extent;
layout(location = 0) out vec4 result;
void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    float d = texelFetch(receiver_depth, pixel, 0).r;
    if (d <= 0.0) {
        result = vec4(0, 0, 0, 1);
        return;
    }
    vec4 h = inverse(test_clip) * vec4(gl_FragCoord.xy / test_extent * 2.0 - 1.0, d * 2.0 - 1.0, 1);
    vec3 e = vec3(0);
    sample_probe_irradiance(h.xyz / h.w, vec3(0, 0, 1), e);
    result = vec4(e / 4.0, 1);
}
)glsl";
    renderer::Shader shader;
    CHECK(shader.build("tiled/all-volume reference", vertex, fragment, error));
    unsigned depth = 0;
    glGenTextures(1, &depth);
    renderer::FrameBuffer target;
    renderer::Camera camera;
    // Resize and partial/empty tiles; selected IDs cross the 32-bit word boundary.
    unsigned iteration = 0;
    for (const auto [width, height] :
         {std::pair{35, 19}, std::pair{17, 17}, std::pair{1, 1}, std::pair{35, 19}}) {
        if (iteration++ == 3) {
            camera.projection =
                renderer::perspective_reversed(1.0f, float(width) / float(height), 0.1f, 10);
            camera.view = renderer::look_along({0, 0, 2}, {0, 0, -1}, {0, 1, 0});
        }
        CHECK(target.create(width, height, error));
        const auto clip = renderer::multiply(camera.projection, camera.view);
        const auto depth_at = [&](float z) {
            return ((clip[10] * z + clip[14]) / (clip[11] * z + clip[15]) + 1) * 0.5f;
        };
        std::vector<float> depths(std::size_t(width) * height, depth_at(0));
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (x >= 16 && x < 32 && (x & 1)) {
                    depths[std::size_t(y) * width + x] = depth_at(0.75f);
                }
                if (x >= 32 && y >= 16) {
                    depths[std::size_t(y) * width + x] = 0;
                }
            }
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT32F,
            width,
            height,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            depths.data()
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        probes.select_visible_regions(depth, camera, width, height);
        const auto draw = [&](int mode, bool depth_matched = true) {
            glBindFramebuffer(GL_FRAMEBUFFER, target.id());
            glViewport(0, 0, width, height);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            shader.use();
            probes.bind(shader, scene::Layer::Terrain, false, &camera, depth_matched);
            if (mode) {
                glUniform1i(shader.uniform("probe_gpu_selection"), mode);
            }
            glUniform1i(shader.uniform("receiver_depth"), 0);
            glUniform2f(shader.uniform("test_extent"), float(width), float(height));
            const auto clip = renderer::multiply(camera.projection, camera.view);
            glUniformMatrix4fv(shader.uniform("test_clip"), 1, GL_FALSE, clip.data());
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depth);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            return target.read_rgba();
        };
        const auto tiled = draw(1), reference = draw(2);
        CHECK(tiled.size() == reference.size());
        for (std::size_t i = 0; i < tiled.size(); ++i) {
            CHECK(std::abs(int(tiled[i]) - int(reference[i])) <= 1);
        }
        if (width == 35 && iteration == 1) {
            // Green, blue and yellow locals must coexist in one image.
            CHECK(tiled[(8 * width + 4) * 4 + 1] > 0);
            CHECK(
                tiled[(8 * width + 4) * 4] == 0 && tiled[(8 * width + 4) * 4 + 2] > 0
            );  // finer blue beats coarse red
            CHECK(tiled[(8 * width + 20) * 4 + 2] > 0);
            CHECK(tiled[(8 * width + 30) * 4] > 0 && tiled[(8 * width + 30) * 4 + 1] > 0);
        }
        if (width == 35 && iteration == 1) {
            // Opaque geometry lies at z=0, but a blended receiver lies in the
            // separate z=0.75 region. Its GI must not use opaque depth masks.
            const auto replace_depth = [&](float z) {
                depths.assign(depths.size(), depth_at(z));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, depth);
                glTexSubImage2D(
                    GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_DEPTH_COMPONENT, GL_FLOAT, depths.data()
                );
            };
            replace_depth(0);
            probes.select_visible_regions(depth, camera, width, height);
            replace_depth(0.75f);
            const auto blended = draw(0, false), blended_reference = draw(2);
            CHECK(blended == blended_reference);
            CHECK(blended != draw(1));  // The fixture actually excludes its region.
            CHECK(blended[(8 * width + 16) * 4 + 2] > 0);
            // A subsequent depth-matched bind must restore tiled selection.
            CHECK(draw(0) == draw(1));
        }
        const float saved_x = camera.projection[12];
        camera.projection[12] += 1.5f;
        const auto secondary = draw(0), secondary_reference = draw(2);
        for (std::size_t i = 0; i < secondary.size(); ++i) {
            CHECK(std::abs(int(secondary[i]) - int(secondary_reference[i])) <= 1);
        }
        camera.projection[12] = saved_x;
        depths.assign(depths.size(), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depths.data()
        );
        probes.select_visible_regions(depth, camera, width, height);
        const auto empty = draw(1);
        for (std::size_t i = 0; i < empty.size(); i += 4) {
            CHECK(empty[i] == 0 && empty[i + 1] == 0 && empty[i + 2] == 0);
        }
        target.release();
    }
    CHECK(glGetError() == GL_NO_ERROR);
    probes.release();
    shader.release();
    glDeleteTextures(1, &depth);
}

void camera_regions(renderer::Shader& shader, renderer::FrameBuffer& frame, unsigned vao) {
    lighting::ProbeGrid base_grid;
    base_grid.spacing = {64, 64, 64};
    base_grid.dimensions = {5, 2, 2};
    auto base = lighting::ProbeVolume::allocate(base_grid);
    for (auto& p : base.probes) {
        p.valid = true;
    }
    for (auto& e : base.irradiance) {
        e = {1, 0, 0};
    }
    for (auto& m : base.distance_moments) {
        m = {1024, 1024 * 1024};
    }
    std::vector<lighting::ProbeVolume> regions;
    for (unsigned i = 0; i < 3; ++i) {
        auto grid = base_grid;
        grid.origin = {float(i) * 8, 0, 0};
        grid.spacing = {4, 4, 2};
        grid.dimensions = {5, 5, 9};
        auto region = lighting::ProbeVolume::allocate(grid);
        for (auto& p : region.probes) {
            p.valid = true;
            p.offset = {0.05f, 0, 0};
        }
        for (auto& e : region.irradiance) {
            e = i == 0   ? Float3{0, .03125f, 0}
                : i == 1 ? Float3{0, 0, .03125f}
                         : Float3{1, .03125f, 0};
        }
        for (auto& m : region.distance_moments) {
            m = {1024, 1024 * 1024};
        }
        regions.push_back(std::move(region));
    }
    std::array<std::array<float, 4>, 8> points{}, directions{};
    for (unsigned i = 0; i < 8; ++i) {
        points[i] = {8 + float(i) * 4, 8, 1 + float(i) * 2, 1};
        directions[i] = {0, 0, 1, 0};
    }
    for (int variant = 0; variant < 4; ++variant) {
        if (variant == 1) {
            for (auto& p : regions[1].probes) {
                p.valid = false;
            }
        }
        if (variant == 2) {
            for (auto& m : regions[0].distance_moments) {
                m = {0.001f, 0.000001f};
            }
        }
        if (variant == 3) {
            regions.resize(129, regions.back());  // Force two-slot streaming fallback.
        }
        scene::ProbeLighting probes;
        std::string error;
        CHECK(probes.upload_regions(
            openrac::test::gpu_probes(base), openrac::test::gpu_regions(regions), error
        ));
        if (variant == 3) {
            CHECK(!probes.compute_selection());
        }

        struct Step {
            float camera, seconds;
        };

        for (const auto step :
             {Step{8, 0.1f},
              Step{8, 1},
              Step{120, 0.1f},
              Step{120, 1},
              Step{24, 1},
              Step{24, 0.1f},
              Step{200, 0.1f},
              Step{200, 1},
              Step{8, 1}}) {
            probes.update_regions({step.camera, 8, 8}, step.seconds);
            std::array<lighting::WeightedProbeRegion, 2> weighted{};
            for (unsigned slot = 0; slot < 2; ++slot) {
                const auto& selected = probes.region_slots()[slot];
                if (selected.region >= 0) {
                    weighted[slot] = {&regions[selected.region], selected.weight};
                }
            }
            glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
            glViewport(0, 0, 8, 1);
            glDisable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            shader.use();
            probes.bind(shader, scene::Layer::Terrain);
            glUniform4fv(shader.uniform("positions"), 8, points[0].data());
            glUniform4fv(shader.uniform("normals"), 8, directions[0].data());
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            const auto pixels = frame.read_rgba();
            for (unsigned i = 0; i < 8; ++i) {
                auto sample = lighting::sample_probe_regions(
                    base, weighted, {points[i][0], points[i][1], points[i][2]}, {0, 0, 1}
                );
                CHECK(sample && pixels[i * 4 + 3] == 255);
                for (unsigned c = 0; c < 3; ++c) {
                    CHECK(
                        std::abs(float(pixels[i * 4 + c]) / 255 * 4 - sample->irradiance[c]) < 0.02f
                    );
                }
            }
            CHECK(glGetError() == GL_NO_ERROR);
        }
        probes.release();
    }
}

void camera_facing_normals() {
    // Red illumination from +Z, blue from -Z. Looking at either side must
    // select that side's direct light and directional GI, for either normal
    // sign and either triangle winding.
    for (bool reversed : {false, true}) {
        for (float sign : {1.0f, -1.0f, 0.0f}) {
            scene::LevelData level;
            level.light_sets.resize(256);
            level.light_sets[0] = 1;
            level.light_sets[6] = -1;
            level.light_sets[10] = 1;
            level.light_sets[14] = 1;
            for (const Float3 p :
                 {Float3{-1, -1, -0.5f}, Float3{1, -1, -0.5f}, Float3{0, 1, -0.5f}}) {
                scene::Vertex v{};
                std::copy(p.begin(), p.end(), v.position);
                v.normal[2] = sign;
                if (sign == 0) {
                    v.normal[0] = 1;  // bad tangent normal must not steer GI through the plane
                }
                v.colour[3] = 1;
                v.light_slot = -1;
                level.vertices.push_back(v);
            }
            level.indices = reversed ? std::vector<std::uint32_t>{0, 2, 1}
                                     : std::vector<std::uint32_t>{0, 1, 2};
            level.materials.push_back({});
            scene::Model model;
            model.primitives.push_back({0, 3, 0});
            level.models.push_back(model);
            level.instances[static_cast<std::size_t>(scene::Layer::Terrain)].push_back({});
            renderer::Renderer renderer;
            renderer.set_ambient_occlusion(std::make_unique<renderer::Gtao>());
            scene::LevelScene scene;
            std::string error;
            CHECK(scene.upload(level, renderer.textures(), error));
            CHECK(renderer.add(
                std::make_unique<scene::LayerRenderer>(scene, scene::Layer::Terrain), error
            ));
            CHECK(renderer.init(error));
            renderer::FrameBuffer frame;
            CHECK(frame.create(32, 32, error));
            lighting::ProbeGrid grid;
            grid.origin = {-2, -2, -2};
            grid.spacing = {4, 4, 4};
            auto volume = lighting::ProbeVolume::allocate(grid);
            for (auto& probe : volume.probes) {
                probe.valid = true;
            }
            for (std::size_t i = 0; i < volume.probes.size(); ++i) {
                for (unsigned y = 0; y < grid.map_resolution; ++y) {
                    for (unsigned x = 0; x < grid.map_resolution; ++x) {
                        const auto n = lighting::oct_decode(
                            {(x + 0.5f) / grid.map_resolution, (y + 0.5f) / grid.map_resolution}
                        );
                        volume.irradiance
                            [i * grid.texels_per_probe() + (y + 1) * (grid.map_resolution + 2) + x
                             + 1] =
                            {3.14159265f * std::max(n[2], 0.0f),
                             0,
                             3.14159265f * std::max(-n[2], 0.0f)};
                    }
                }
            }
            lighting::update_probe_gutters(volume);
            CHECK(scene.set_probe_volume(openrac::test::gpu_probes(volume), error));
            for (bool below : {false, true}) {
                renderer::FrameInput input;
                input.camera.view = renderer::identity();
                input.camera.projection = renderer::identity();
                if (below) {
                    input.camera.view[0] = input.camera.view[10] = -1;
                    input.camera.view[14] = -1;
                }
                for (int mode : {2, 3}) {
                    if (sign == 0 && mode == 3) {
                        continue;  // authored direct-light shading is unchanged
                    }
                    scene.set_probe_view(mode);
                    renderer.render(input, {frame.id(), 32, 32});
                    const auto pixels = frame.read_rgba();
                    const unsigned at = (16 * 32 + 16) * 4;
                    CHECK(pixels[at + (below ? 2 : 0)] > 220);
                    CHECK(pixels[at + (below ? 0 : 2)] < 10);
                    CHECK(pixels[at + 1] < 10);
                }
            }
            CHECK(glGetError() == GL_NO_ERROR);
            renderer.clear_renderers();
            scene.release();
            frame.release();
        }
    }
}

void dense_sphere_fade() {
    lighting::ProbeGrid grid;
    grid.origin = {-8, -8, -8};
    grid.spacing = {16, 16, 16};
    auto base = lighting::ProbeVolume::allocate(grid);
    grid.origin = {0, 0, 0.4f};
    grid.spacing = {4, 4, 4};
    auto dense = lighting::ProbeVolume::allocate(grid);
    for (auto& p : dense.probes) {
        p.valid = true;
    }
    scene::ProbeLighting probes;
    std::string error;
    CHECK(probes.upload_regions(
        openrac::test::gpu_probes(base), {openrac::test::gpu_probes(dense)}, error
    ));
    probes.show_spheres = true;
    renderer::FrameBuffer frame;
    CHECK(frame.create(32, 32, error));
    renderer::TexturePool textures;
    renderer::RenderState state{textures, {frame.id(), 32, 32}, {}};
    renderer::FrameInput input;
    const auto draw = [&]() {
        glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
        glViewport(0, 0, 32, 32);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0, 0, 0, 1);
        glDepthMask(GL_TRUE);
        glClearDepth(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        probes.draw_spheres(input, state);
        return frame.read_rgba()[(16 * 32 + 16) * 4 + 1];
    };
    probes.update_regions({0, 0, 0}, 0.125f);
    const auto partial = draw();
    probes.update_regions({0, 0, 0}, 1);
    const auto full = draw();
    CHECK(partial > 0 && full > partial);
    probes.update_regions({500, 0, 0}, 0.125f);
    const auto leaving = draw();
    CHECK(leaving > 0 && leaving < full);
    probes.update_regions({500, 0, 0}, 1);
    CHECK(draw() == 0);
    probes.update_regions({0, 0, 0}, 1);
    if (probes.compute_selection()) {
        unsigned depth = 0;
        glGenTextures(1, &depth);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth);
        std::array<float, 32 * 32> depths;
        depths.fill(0.9f);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT32F,
            32,
            32,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            depths.data()
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        probes.select_visible_regions(depth, input.camera, 32, 32);
        CHECK(draw() > 0);
        depths.fill(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, 32, 32, GL_DEPTH_COMPONENT, GL_FLOAT, depths.data()
        );
        probes.select_visible_regions(depth, input.camera, 32, 32);
        CHECK(draw() == 0);  // Tile masks, rather than CPU slots, control the overlay.
        glDeleteTextures(1, &depth);
    }
    probes.show_spheres = false;
    const auto calls = state.stats.draw_calls;
    CHECK(draw() == 0 && state.stats.draw_calls == calls);
    CHECK(glGetError() == GL_NO_ERROR);
    probes.release();
    frame.release();
}

void sphere_overlay() {
    lighting::ProbeGrid grid;
    grid.origin = {-0.5f, 0, 0.4f};
    grid.spacing = {8, 8, 8};
    auto volume = lighting::ProbeVolume::allocate(grid);
    volume.probes[0].valid = true;
    // The visible sphere must be drawn at this relocated centre (x=0),
    // rather than the nominal x=-0.5 position.
    volume.probes[0].offset = {0.5f, 0, 0};
    scene::ProbeLighting probes;
    std::string error;
    CHECK(probes.upload(openrac::test::gpu_probes(volume), error));
    probes.show_spheres = true;
    renderer::FrameBuffer frame;
    CHECK(frame.create(32, 32, error));
    renderer::TexturePool textures;
    renderer::RenderState state{textures, {frame.id(), 32, 32}, {}};
    renderer::FrameInput input;
    glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
    glViewport(0, 0, 32, 32);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0, 0, 0, 1);
    glDepthMask(GL_TRUE);
    glClearDepth(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    probes.draw_spheres(input, state);
    auto pixels = frame.read_rgba();
    const auto centre = (16 * 32 + 16) * 4;
    CHECK(pixels[centre] > pixels[centre + 1] && pixels[centre + 1] > pixels[centre + 2]);
    CHECK(pixels[(16 * 32 + 8) * 4] == 0);
    CHECK(state.stats.draw_calls == 1);
    // A nearer opaque surface must hide the placement markers (reverse Z).
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    probes.draw_spheres(input, state);
    pixels = frame.read_rgba();
    CHECK(pixels[centre] == 0 && pixels[centre + 1] == 0 && pixels[centre + 2] == 0);
    probes.sphere_xray = true;
    probes.draw_spheres(input, state);
    pixels = frame.read_rgba();
    CHECK(pixels[centre] > 0 && pixels[centre] < 100);
    CHECK(pixels[centre] > pixels[centre + 1] && pixels[centre + 1] > pixels[centre + 2]);
    CHECK(glGetError() == GL_NO_ERROR);
    glClearDepth(0);
    probes.release();
    frame.release();
}

// Manual RGB gains affect only the primary sun, including the shadow floor.
void sunlight_rgb_correction() {
    renderer::Shader shader;
    std::string error;
    const auto source = std::string("#version 410 core\n") + scene::shaders::probes_glsl + R"glsl(
uniform float test_visibility;
layout(location = 0) out vec4 result;
void main() {
    result = vec4(probe_direct(vec3(0), vec3(0,0,1), vec4(0), test_visibility), 1);
}
)glsl";
    CHECK(shader.build("sunlight RGB correction", vertex, source, error));
    renderer::FrameBuffer frame;
    CHECK(frame.create(4, 4, error));
    unsigned vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
    glViewport(0, 0, 4, 4);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glUseProgram(shader.id());
    std::array<float, 256> lights{};
    lights[0] = .2f;
    lights[1] = .4f;
    lights[2] = .6f;
    lights[6] = -1;
    lights[8] = .1f;
    lights[9] = .05f;
    lights[10] = .1f;
    lights[14] = -1;
    glUniform4fv(shader.uniform("light_sets"), 64, lights.data());
    glUniform3f(shader.uniform("probe_sunlight_scale"), .25f, .5f, .75f);
    glUniform4f(shader.uniform("probe_point_position[0]"), 0, 0, 1, 2);
    glUniform4f(shader.uniform("probe_point_colour[0]"), .2f, .1f, 0, 0);
    for (float visibility : {1.f, 0.f}) {
        glUniform1f(shader.uniform("test_visibility"), visibility);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        const auto pixels = frame.read_rgba();
        // The point light stays unchanged; both directionals retain their shadow floor.
        const float floor = visibility > 0 ? 1.f : .1f;
        const Float3 expected{.15f * floor + .1f, .25f * floor + .05f, .55f * floor};
        for (unsigned c = 0; c < 3; ++c) {
            CHECK(std::abs(pixels[c] / 255.f - expected[c]) < .008f);
        }
    }
    CHECK(glGetError() == GL_NO_ERROR);
    glDeleteVertexArrays(1, &vao);
    frame.release();
}

// Verify channel packing, ties-to-even, half subnormals, large distance moments,
// and partial row updates through the same RGBA/float upload path as probe atlases.
void compact_texture_uploads() {
    renderer::Shader shader;
    std::string error;
    CHECK(shader.build(
        "compact probe uploads",
        vertex,
        R"glsl(#version 410 core
uniform sampler2D source_map;
uniform vec4 expected, empty_value;
uniform int updated;
layout(location = 0) out vec4 result;
void main() {
    ivec2 at = ivec2(gl_FragCoord.xy);
    vec4 reference = updated != 0 && all(equal(at, ivec2(1, 1))) ? empty_value : expected;
    result = vec4(equal(texelFetch(source_map, at, 0), reference));
}
)glsl",
        error
    ));

    struct Case {
        unsigned format;
        std::array<float, 4> input, expected, empty;
    };

    const Case cases[] =
        {{0x8C3A, {1.0078125f, 1.0234375f, 0.5078125f, 13}, {1, 1.03125f, 0.5f, 1}, {0, 0, 0, 1}},
         {0x8230, {4788.5f, 22929732, 9, 13}, {4788.5f, 22929732, 0, 1}, {0, 0, 0, 1}},
         {0x881A,
          {-1.00048828125f, -1.00146484375f, 0x1p-24f, 1},
          {-1, -1.001953125f, 0x1p-24f, 1},
          {}}};
    renderer::FrameBuffer frame;
    CHECK(frame.create(3, 2, error));
    unsigned texture = 0, vao = 0;
    glGenTextures(1, &texture);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
    glViewport(0, 0, 3, 2);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    shader.use();
    glUniform1i(shader.uniform("source_map"), 0);
    for (const auto& test : cases) {
        std::array<std::array<float, 4>, 6> pixels;
        pixels.fill(test.input);
        glTexImage2D(GL_TEXTURE_2D, 0, test.format, 3, 2, 0, GL_RGBA, GL_FLOAT, pixels.data());
        glUniform4fv(shader.uniform("expected"), 1, test.expected.data());
        glUniform4fv(shader.uniform("empty_value"), 1, test.empty.data());
        for (int updated = 0; updated < 2; ++updated) {
            if (updated) {
                const std::array<float, 4> zero{};
                glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_RGBA, GL_FLOAT, zero.data());
            }
            glUniform1i(shader.uniform("updated"), updated);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            const auto read = frame.read_rgba();
            if (read[0] != 255 || read[1] != 255 || read[2] != 255 || read[3] != 255) {
                std::fprintf(
                    stderr,
                    "format %x update %d comparison %u %u %u %u\n",
                    test.format,
                    updated,
                    read[0],
                    read[1],
                    read[2],
                    read[3]
                );
            }
            for (auto channel : read) {
                CHECK(channel == 255);
            }
        }
    }
    CHECK(glGetError() == GL_NO_ERROR);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
}
}  // namespace

int main() {
    test::GlContext context;
    std::string why;
    if (!context.open(why)) {
        std::printf("SKIP: %s\n", why.c_str());
        return test::kSkip;
    }
    compact_texture_uploads();
    std::string fragment = "#version 410 core\n";
    fragment += scene::shaders::probes_glsl;
    fragment += R"glsl(uniform vec4 positions[8];
uniform vec4 normals[8];
uniform int camera_biased;
uniform vec3 test_camera;
layout(location = 0) out vec4 result;
void main() {
    int index = int(gl_FragCoord.x);
    vec3 n = probe_unit(normals[index].xyz), e = vec3(0.0);
    vec3 p = positions[index].xyz;
    bool valid = dot(n, n) > 0.5
                 && (camera_biased != 0
                         ? sample_probe_irradiance(p, n, probe_surface_bias(n, test_camera - p), e)
                         : sample_probe_irradiance(p, n, e));
    result = valid ? vec4(e / 4.0, 1.0) : vec4(0.0);
}
)glsl";
    renderer::Shader shader;
    std::string error;
    CHECK(shader.build("probe CPU/GPU reference", vertex, fragment, error));
    if (!error.empty()) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    renderer::Shader reference;
    std::string reference_fragment(fragment);
    reference_fragment.insert(
        reference_fragment.find('\n') + 1, "#define OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE 1\n"
    );
    CHECK(reference.build("probe zero-weight reference", vertex, reference_fragment, error));
    zero_weight_reference = &reference;
    renderer::FrameBuffer frame;
    CHECK(frame.create(8, 1, error));
    unsigned vao = 0;
    glGenVertexArrays(1, &vao);
    lighting::ProbeGrid grid;
    auto volume = lighting::ProbeVolume::allocate(grid);
    for (std::size_t p = 0; p < volume.probes.size(); ++p) {
        volume.probes[p].valid = p != 4;
        volume.probes[p].offset = {0.25f, -0.5f, 0.125f};
        for (unsigned y = 0; y < grid.map_resolution; ++y) {
            for (unsigned x = 0; x < grid.map_resolution; ++x) {
                const auto n = lighting::oct_decode(
                    {(static_cast<float>(x) + 0.5f) / grid.map_resolution,
                     (static_cast<float>(y) + 0.5f) / grid.map_resolution}
                );
                const auto at =
                    p * grid.texels_per_probe() + (y + 1) * (grid.map_resolution + 2) + x + 1;
                volume.irradiance[at] =
                    {1 + n[0] * 0.5f, 1 + n[1] * 0.5f, 0.25f + static_cast<float>(p) * 0.25f};
                volume.distance_moments[at] = {10, 120};
            }
        }
    }
    lighting::update_probe_gutters(volume);
    check(volume, shader, frame, vao);
    check(volume, shader, frame, vao, nullptr, true);
    auto nonlinear = lighting::ProbeVolume::allocate(grid);
    for (std::size_t i = 0; i < nonlinear.probes.size(); ++i) {
        nonlinear.probes[i].valid = true;
        for (std::size_t t = 0; t < grid.texels_per_probe(); ++t) {
            nonlinear.irradiance[i * grid.texels_per_probe() + t] =
                i % 2 ? Float3{1, 1, 1} : Float3{};
        }
    }
    check(nonlinear, shader, frame, vao);
    for (std::size_t i = 0; i < nonlinear.irradiance.size(); ++i) {
        nonlinear.irradiance[i] = i % (grid.map_resolution + 2) < (grid.map_resolution + 2) / 2
                                      ? Float3{1, 1, 1}
                                      : Float3{};
    }
    check(nonlinear, shader, frame, vao);
    auto bias_volume = lighting::ProbeVolume::allocate(grid);
    bias_volume.probes[7].valid = true;
    for (auto& e : bias_volume.irradiance) {
        e = {1, 2, 3};
    }
    for (auto& m : bias_volume.distance_moments) {
        m = {0.25f, 0.0625f};
    }
    check(bias_volume, shader, frame, vao);
    check(bias_volume, shader, frame, vao, nullptr, true, true);
    auto boundary_grid = grid;
    boundary_grid.origin[2] = -1;
    boundary_grid.dimensions[2] = 3;
    auto boundary = lighting::ProbeVolume::allocate(boundary_grid);
    for (auto& probe : boundary.probes) {
        probe.valid = true;
    }
    for (auto& moment : boundary.distance_moments) {
        moment = {100, 10000};
    }
    for (std::size_t i = 8; i < boundary.probes.size(); ++i) {
        for (std::size_t t = 0; t < boundary_grid.texels_per_probe(); ++t) {
            boundary.irradiance[i * boundary_grid.texels_per_probe() + t] = {1, 2, 3};
        }
    }
    // Rebinding both bias sliders must change interpolation as well as moments.
    check(boundary, shader, frame, vao, nullptr, true);
    auto floor = lighting::ProbeVolume::allocate(grid);
    for (auto& probe : floor.probes) {
        probe.valid = true;
    }
    for (std::size_t i = 0; i < floor.probes.size(); ++i) {
        for (std::size_t t = 0; t < floor.grid.texels_per_probe(); ++t) {
            floor.irradiance[i * floor.grid.texels_per_probe() + t] =
                i < 4 ? lighting::Float3{1, 2, 3} : lighting::Float3{};
        }
    }
    const auto floor_sample = lighting::sample_probes(floor, {8, 8, 8}, {0, 0, 1});
    CHECK(floor_sample && (floor_sample->irradiance == lighting::Float3{}));
    check(floor, shader, frame, vao);
    // Sparse fallback must retain weak/zero visibility instead of restoring
    // full outdoor brightness or reporting blocked support as missing.
    for (auto& m : volume.distance_moments) {
        m = {1, 5};
    }
    check(volume, shader, frame, vao);
    for (auto& m : volume.distance_moments) {
        m = {0.1f, 0.01f};
    }
    check(volume, shader, frame, vao);
    for (auto& p : volume.probes) {
        p.valid = false;
    }
    check(volume, shader, frame, vao);
    // Distinct volumes exercise atlas offsets, interior replacement, smooth
    // boundary blending, invalid local fallback and local-only coverage.
    auto coarse = lighting::ProbeVolume::allocate(grid);
    for (auto& p : coarse.probes) {
        p.valid = true;
    }
    for (auto& e : coarse.irradiance) {
        e = {1, 0, 0};
    }
    auto local_grid = grid;
    local_grid.origin = {4, 4, 4};
    local_grid.spacing = {2, 2, 2};
    local_grid.dimensions = {5, 5, 5};
    auto dense = lighting::ProbeVolume::allocate(local_grid);
    for (auto& p : dense.probes) {
        p.valid = true;
    }
    for (auto& e : dense.irradiance) {
        e = {0, 0, 1};
    }
    check(coarse, shader, frame, vao, &dense);
    check(coarse, shader, frame, vao, &dense, true);
    coverage_seam(coarse, dense, shader, frame, vao);
    for (auto& p : dense.probes) {
        p.valid = false;
    }
    check(coarse, shader, frame, vao, &dense);
    for (auto& p : dense.probes) {
        p.valid = true;
    }
    for (auto& m : dense.distance_moments) {
        m = {0.1f, 0.01f};
    }
    check(coarse, shader, frame, vao, &dense);
    for (auto& m : dense.distance_moments) {
        m = {1024, 1024 * 1024};
    }
    for (auto& p : coarse.probes) {
        p.valid = false;
    }
    check(coarse, shader, frame, vao, &dense);
    check(coarse, shader, frame, vao);  // disables the preceding local uniforms
    camera_regions(shader, frame, vao);
    compute_regions(shader, frame, vao);
    glDeleteVertexArrays(1, &vao);
    shader.release();
    reference.release();
    zero_weight_reference = nullptr;
    frame.release();
    material_path(scene::Layer::Terrain, 0, false);
    material_path(scene::Layer::Mobys, 0, false);
    material_path(scene::Layer::Mobys, 0, false, false, true);
    material_path(scene::Layer::Mobys, 0, false, false, false, true);
    material_path(scene::Layer::Mobys, 0, false, false, false, true, true);
    material_path(scene::Layer::Mobys, 0, false, false, false, true, true, true);
    material_path(scene::Layer::Mobys, 0, false, false, false, true, true, false, true);
    material_path(scene::Layer::Mobys, 0, false, false, false, true, false, false, false, true);
    material_path(scene::Layer::Mobys, 1, false);
    material_path(scene::Layer::Mobys, 2, false);
    material_path(scene::Layer::Mobys, 0, true);
    material_path(scene::Layer::Ties, 0, false, true);
    material_path(scene::Layer::Shrubs, 0, false, true);
    sphere_overlay();
    dense_sphere_fade();
    camera_facing_normals();
    sunlight_rgb_correction();
    return test::result();
}
