// SPDX-License-Identifier: GPL-3.0-or-later
// Independent native test: no game, ISO, settings, saves, SDL or Mesa.
#include "renderer/deko3d/presenter.h"
#include "renderer/texture.h"
#include "renderer/deko3d/level_scene.h"
#include <cstdio>
#include <exception>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <switch.h>

namespace {
std::vector<std::uint8_t> shader(const char* name) {
    std::ifstream file(name,std::ios::binary | std::ios::ate);
    if (!file) return {};
    const auto size = file.tellg();
    if (size < 16 || size > 64*1024) return {};
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()))) return {};
    return bytes;
}
}
int main() {
    std::error_code ec;
    std::filesystem::create_directories("sdmc:/switch/openrac",ec);
    FILE* report = std::fopen("sdmc:/switch/openrac/deko3d-probe.log","w");
    if (!report) return 1;
    const auto note = [report](const std::string& message) {
        std::fprintf(report,"%s\n",message.c_str()); std::fflush(report);
    };
    const bool mounted = R_SUCCEEDED(romfsInit());
    int result = 1;
    try {
        result = [&] {
            if (!mounted) { note("FAIL: probe shader RomFS could not be mounted."); return 1; }
            openrac::deko::Presenter renderer;
            std::string error;
            constexpr unsigned width = 1280, height = 720;
            if (!renderer.open(width,height,error)) { note(error); return 1; }
            note("Native Deko3D device, queue and two-image swapchain opened.");
            if (!renderer.clear(0xff401008u,error)) { note(error); return 1; }
            std::vector<std::uint32_t> pixels(width*height);
            // Distinct corners reveal channel swaps, flips and row-stride errors.
            for (unsigned y = 0; y < height; ++y) for (unsigned x = 0; x < width; ++x) {
                pixels[std::size_t(y)*width+x] = y < height/2
                    ? (x < width/2 ? 0xff0000ffu : 0xff00ff00u)
                    : (x < width/2 ? 0xffff0000u : 0xffffffffu);
            }
            pixels[0] = 0xff123456u; pixels[1] = 0xffabcdefu;
            pixels[width] = 0xff102030u; pixels[width+1] = 0xff405060u;
            if (!renderer.present(pixels,error,true)) { note(error); return 1; }
            note("PASS: CPU upload -> native image -> GPU readback matched 2x2 pixels.");
            const auto vertex = shader("romfs:/shaders/probe.vert.dksh");
            const auto fragment = shader("romfs:/shaders/probe.frag.dksh");
            if (!renderer.load_triangle(vertex,fragment,error) || !renderer.triangle(error,true)) { note(error); return 1; }
            note("PASS: UAM shader programs, native triangle drawing and center-pixel readback.");
            const auto mesh_vertex = shader("romfs:/shaders/mesh.vert.dksh");
            const auto mesh_fragment = shader("romfs:/shaders/mesh.frag.dksh");
            if (!renderer.load_mesh_shaders(mesh_vertex,mesh_fragment,error)) { note(error); return 1; }
            using Vertex = openrac::deko::MeshVertex;
            const Vertex overlap[] = {
                {{-.7f,-.6f,.2f},{1,0,0,1}},{{.7f,-.6f,.2f},{1,0,0,1}},{{0,.7f,.2f},{1,0,0,1}},
                {{-.7f,-.6f,.8f},{0,0,1,1}},{{.7f,-.6f,.8f},{0,0,1,1}},{{0,.7f,.8f},{0,0,1,1}}};
            const std::uint32_t overlap_indices[] = {0,1,2,3,4,5};
            const std::array<float,16> identity{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
            if (!renderer.upload_mesh(overlap,overlap_indices,error) || !renderer.mesh(identity,error,true)) { note(error); return 1; }
            note("PASS: retained vertex/index buffers, transform uniform and near/far depth rejection.");
            // Exercise the actual game converter, GS alpha 0x80 -> full opacity,
            // a non-square texture and a source row requiring 32-byte padding.
            const std::array<std::uint8_t,24> ps2_pixels{
                0,255,0,128, 255,0,0,128, 255,255,255,128,
                255,255,0,128, 255,0,0,128, 0,255,255,128};
            const auto game_texture = openrac::renderer::convert_ct32(ps2_pixels,3,2,openrac::renderer::AlphaScale::Full);
            const Vertex textured_quad[] = {
                {{-.8f,-.8f,.3f},{1,1,1,1},{0,0}},{{.8f,-.8f,.3f},{1,1,1,1},{1,0}},
                {{.8f,.8f,.3f},{1,1,1,1},{1,1}},{{-.8f,.8f,.3f},{1,1,1,1},{0,1}}};
            const std::uint32_t quad_indices[] = {0,1,2,0,2,3};
            if (!renderer.upload_texture(game_texture,error)
                || !renderer.upload_mesh(textured_quad,quad_indices,error)
                || !renderer.mesh(identity,error,true,true)) { note(error); return 1; }
            // Revisit both frame slots and reuse their recorded textured lists.
            for (unsigned i = 0; i < 4; ++i)
                if (!renderer.mesh(identity,error,true,true)) { note(error); return 1; }
            note("PASS: game PS2 RGBA conversion, padded native texture upload, descriptors and cached textured draws.");
            const Vertex cube[] = {
                {{-.5f,-.5f,-.5f},{1,0,0,1}},{{.5f,-.5f,-.5f},{0,1,0,1}},
                {{.5f,.5f,-.5f},{0,0,1,1}},{{-.5f,.5f,-.5f},{1,1,0,1}},
                {{-.5f,-.5f,.5f},{1,0,1,1}},{{.5f,-.5f,.5f},{0,1,1,1}},
                {{.5f,.5f,.5f},{1,1,1,1}},{{-.5f,.5f,.5f},{.4f,.7f,1,1}}};
            const std::uint32_t cube_indices[] = {0,1,2,0,2,3,4,6,5,4,7,6,0,4,5,0,5,1,
                3,2,6,3,6,7,0,3,7,0,7,4,1,5,6,1,6,2};
            if (!renderer.upload_mesh(cube,cube_indices,error)) { note(error); return 1; }
            // The same native scene API the game adapter will use: retained
            // multi-material LevelData, captured instances and a GPU joint pose.
            openrac::deko::SceneRenderer native_scene(renderer);
            const auto scene_vertex = shader("romfs:/shaders/scene.vert.dksh");
            const auto scene_fragment = shader("romfs:/shaders/scene.frag.dksh");
            if (!native_scene.load_shaders(scene_vertex,scene_fragment,error)) { note(error); return 1; }
            openrac::scene::LevelData level;
            for (unsigned i=0;i<6;++i) {
                openrac::scene::Vertex v{};
                for (unsigned c=0;c<3;++c) v.position[c]=overlap[i].position[c];
                v.position[2]=i<3 ? .8f : .2f;
                if (i<3) v.position[0]+=3.0f; // only the joint pose brings it into view
                v.normal[2]=1;
                for (auto& channel:v.colour) channel=1;
                v.uv[0]=v.uv[1]=.5f;
                v.weights[0]=i<3 ? 1.0f : 0.0f;
                v.light_slot=-1;
                level.vertices.push_back(v);
            }
            level.indices.assign(std::begin(overlap_indices),std::end(overlap_indices));
            level.images.push_back(game_texture);
            openrac::scene::Material front,back;
            front.image=0; back.colour={0,0,1,1};
            level.materials={front,back};
            openrac::scene::Model near_model,far_model;
            openrac::scene::Primitive near_primitive,far_primitive;
            near_primitive.index_count=3; far_primitive.first_index=3; far_primitive.index_count=3; far_primitive.material=1;
            near_model.primitives.push_back(near_primitive); near_model.joints=1;
            far_model.primitives.push_back(far_primitive);
            level.models={near_model,far_model};
            openrac::scene::Instance near_instance,far_instance;
            near_instance.palette=0; far_instance.model=1;
            const std::array instances{near_instance,far_instance};
            auto posed=identity; posed[12]=-3.0f;
            const std::array<openrac::renderer::Mat4,1> pose{posed};
            openrac::deko::LevelScene level_scene(native_scene);
            if (!level_scene.upload(level,error)) { note(error); return 1; }
            std::vector<openrac::deko::SceneDraw> native_draws;
            if (!level_scene.append(openrac::scene::Layer::Mobys,instances,pose,native_draws,error)) { note(error); return 1; }
            const openrac::deko::SceneView native_view;
            for (unsigned i=0;i<4;++i)
                if (!native_scene.render(native_view,native_draws,error,true)) { note(error); return 1; }
            note("PASS: LevelData adapter, retained materials, multi-object frame, GPU joints and reversed depth.");
            std::array<openrac::scene::Vertex,3> dynamic_vertices{};
            for (unsigned i=0;i<3;++i) {
                auto& v=dynamic_vertices[i];
                for(unsigned c=0;c<3;++c) v.position[c]=overlap[i].position[c];
                v.colour[0]=v.colour[3]=1; v.light_slot=-1;
            }
            const std::array<std::uint32_t,3> dynamic_indices{0,1,2};
            openrac::deko::SceneDraw dynamic_draw;
            dynamic_draw.dynamic_vertices=dynamic_vertices; dynamic_draw.dynamic_indices=dynamic_indices;
            dynamic_draw.index_count=3; dynamic_draw.screen_space=true;
            dynamic_draw.depth_test=dynamic_draw.depth_write=false;
            for(unsigned i=0;i<4;++i)
                if (!native_scene.render(native_view,std::span(&dynamic_draw,1),error,true)) { note(error); return 1; }
            note("PASS: frame-owned dynamic geometry ring and screen-space draw path.");
            PadState pad;
            padConfigureInput(1,HidNpadStyleSet_NpadStandard); padInitializeDefault(&pad);
            for (unsigned frame = 0; frame < 900 && appletMainLoop(); ++frame) {
                padUpdate(&pad); if (padGetButtonsDown(&pad) & HidNpadButton_Plus) break;
                // Three seconds quadrants, three seconds triangle, then a cube.
                const float angle = float(frame) * .02f, c = std::cos(angle), s = std::sin(angle);
                constexpr float sx = .8f * height / width;
                const std::array<float,16> transform{sx*c,0,-.35f*s,0, 0,.8f,0,0,
                    sx*s,0,.35f*c,0, 0,0,.5f,1};
                const bool shown = frame < 180 ? renderer.present(pixels,error)
                    : frame < 360 ? renderer.triangle(error)
                    : frame < 600 ? renderer.mesh(transform,error) : native_scene.render(native_view,native_draws,error);
                if (!shown) { note(error); return 1; }
            }
            note("Presentation calls: " + std::to_string(renderer.frames()));
            note("READBACK CHECKS COMPLETED. Visual display still needs human confirmation.");
            return 0;
        }();
    } catch (const std::exception& error) { note(std::string("FAIL: ")+error.what()); }
    if (mounted) romfsExit();
    std::fclose(report); return result;
}
