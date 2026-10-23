// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac-viewer: one extracted level, drawn natively on the GPU with a free
// camera (docs/port/ROADMAP.md, P3). No game code runs; the level comes from
// the editor's port export:
//
//   python3 editor/extract.py port baserom/SCES_509.16.iso build/port --level 0
//   openrac-viewer build/port/level_00
//
// Controls: W A S D move, Q and E down and up, Shift faster, the mouse wheel
// changes speed; hold the right mouse button to look around. 1 to 5 show or
// hide the sky, terrain, ties, shrubs and mobys; L toggles the lighting;
// F11 toggles borderless fullscreen; Escape quits.
//
// Headless check: --screenshot out.png --frames N draws N frames in a hidden
// window (SDL's offscreen driver when there is no display) and writes the
// last one.

#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <SDL3/SDL.h>

#include "common/log.h"
#include "platform/clock.h"
#include "platform/window.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/renderer.h"
#include "scene/level.h"
#include "scene/level_renderer.h"
#include "scene/screenshot.h"
#include "viewer/camera.h"
#include "viewer/game_state.h"

namespace {

using namespace openrac;

struct Options {
    std::filesystem::path level;
    std::string screenshot;
    int frames = 1;
    int width = 1280;
    int height = 720;
    platform::DisplayMode mode = platform::DisplayMode::Windowed;
    bool vsync = true;
    bool lighting = true;
    bool camera_given = false;
    float camera[5] = {};  // x, y, z, yaw and pitch in degrees
    std::filesystem::path state;  // a 32 MB memory picture of the running game
};

constexpr const char* kUsage =
    "usage: openrac-viewer LEVEL_DIR [options]\n"
    "  LEVEL_DIR            a level written by `editor/extract.py port` (OUT/level_NN)\n"
    "  --screenshot FILE    draw --frames frames without showing a window, write the last as PNG\n"
    "  --frames N           frames to draw before the screenshot (default 1)\n"
    "  --size WxH           window or screenshot size (default 1280x720)\n"
    "  --camera X,Y,Z,YAW,PITCH   start here (game units; degrees) instead of framing the level\n"
    "  --fullscreen | --borderless\n"
    "  --no-vsync  --no-lighting\n"
    "  --state FILE         the game's main memory at one frame (32 MB): its camera and live mobys\n";

bool parse_floats(std::string_view text, float* out, int count) {
    for (int i = 0; i < count; ++i) {
        const std::size_t comma = text.find(',');
        const std::string part(text.substr(0, comma));
        char* end = nullptr;
        out[i] = std::strtof(part.c_str(), &end);
        if (part.empty() || end != part.c_str() + part.size()) {
            return false;
        }
        if (i + 1 < count) {
            if (comma == std::string_view::npos) {
                return false;
            }
            text.remove_prefix(comma + 1);
        } else if (comma != std::string_view::npos) {
            return false;
        }
    }
    return true;
}

bool parse(int argc, char** argv, Options& o) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        auto next = [&]() -> std::string_view {
            return i + 1 < argc ? std::string_view(argv[++i]) : std::string_view();
        };
        if (arg == "--screenshot") {
            o.screenshot = std::string(next());
        } else if (arg == "--frames") {
            const std::string_view v = next();
            if (std::from_chars(v.data(), v.data() + v.size(), o.frames).ec != std::errc()
                || o.frames < 1) {
                return false;
            }
        } else if (arg == "--size") {
            const std::string_view v = next();
            const std::size_t x = v.find('x');
            if (x == std::string_view::npos
                || std::from_chars(v.data(), v.data() + x, o.width).ec != std::errc()
                || std::from_chars(v.data() + x + 1, v.data() + v.size(), o.height).ec
                       != std::errc()
                || o.width < 16 || o.height < 16) {
                return false;
            }
        } else if (arg == "--camera") {
            if (!parse_floats(next(), o.camera, 5)) {
                return false;
            }
            o.camera_given = true;
        } else if (arg == "--fullscreen") {
            o.mode = platform::DisplayMode::Fullscreen;
        } else if (arg == "--borderless") {
            o.mode = platform::DisplayMode::Borderless;
        } else if (arg == "--no-vsync") {
            o.vsync = false;
        } else if (arg == "--state") {
            o.state = std::filesystem::path(next());
        } else if (arg == "--no-lighting") {
            o.lighting = false;
        } else if (!arg.empty() && arg[0] != '-' && o.level.empty()) {
            o.level = std::filesystem::path(arg);
        } else {
            return false;
        }
    }
    return !o.level.empty();
}

int run(const Options& options) {
    const bool headless = !options.screenshot.empty();
    if (headless && !platform::Platform::display_available()) {
        platform::Platform::use_offscreen_video();
    }

    viewer::LevelData level;
    std::string error;
    if (!viewer::load_level(options.level, level, error)) {
        log::error("{}", error);
        return 1;
    }
    std::size_t placed = 0;
    for (std::size_t layer = 0; layer < viewer::kLayerCount; ++layer) {
        log::info(
            "{}: {} placed",
            viewer::layer_name(static_cast<viewer::Layer>(layer)),
            level.instances[layer].size()
        );
        placed += level.instances[layer].size();
    }
    log::info(
        "level {}: {} models, {} vertices, {} triangles, {} textures ({} missing), {} placed",
        level.level,
        level.models.size(),
        level.vertices.size(),
        level.indices.size() / 3,
        level.images.size(),
        level.missing_images,
        placed
    );

    platform::Platform sdl(platform::kVideo);
    if (!sdl.ok()) {
        log::error("{}", sdl.error());
        return 1;
    }
    platform::WindowConfig config;
    config.title = std::format("OpenRAC level viewer: level {}", level.level);
    config.width = options.width;
    config.height = options.height;
    config.mode = headless ? platform::DisplayMode::Windowed : options.mode;
    config.vsync = options.vsync && !headless;
    config.hidden = headless;
    auto window = platform::Window::open(config, error);
    if (!window) {
        log::error("{}", error);
        return 1;
    }
    std::string missing;
    if (!gl::load(platform::Window::gl_loader(), missing)) {
        log::error("OpenGL functions missing: {}", missing);
        return 1;
    }

    renderer::Renderer renderer;
    viewer::LevelScene scene;
    scene.lighting = options.lighting;
    if (!scene.upload(level, renderer.textures(), error)) {
        log::error("{}", error);
        return 1;
    }
    for (std::size_t layer = 0; layer < viewer::kLayerCount; ++layer) {
        renderer.add(
            std::make_unique<viewer::LayerRenderer>(scene, static_cast<viewer::Layer>(layer)), error
        );
    }
    if (!renderer.init(error)) {
        log::error("{}", error);
        return 1;
    }

    // The running game's camera and mobys, from its memory.
    std::optional<viewer::GameState> game;
    if (!options.state.empty()) {
        std::vector<std::uint8_t> ram;
        if (!viewer::load_memory(options.state, ram, error)) {
            log::error("{}", error);
            return 1;
        }
        game = viewer::read_game_state(ram, viewer::rac1_pal_addresses(ram));
        std::vector<viewer::Instance> live;
        int unknown = 0;
        for (const viewer::LiveMoby& m : game->mobys) {
            auto cls = level.moby_classes.find(m.class_id);
            if (cls == level.moby_classes.end()) {
                ++unknown;
                continue;
            }
            viewer::Instance instance;
            instance.model = cls->second.first;
            instance.matrix = m.matrix;
            live.push_back(instance);
        }
        scene.set_instances(viewer::Layer::Mobys, live);
        log::info(
            "game state: camera at {:.1f} {:.1f} {:.1f}, {} live mobys ({} of classes the level does not have)",
            game->camera_position[0],
            game->camera_position[1],
            game->camera_position[2],
            game->mobys.size(),
            unknown
        );
    }

    viewer::FlyCamera camera;
    camera.frame(level.bounds_min, level.bounds_max);
    if (options.camera_given) {
        camera.position = {options.camera[0], options.camera[1], options.camera[2]};
        camera.yaw = options.camera[3] * 3.14159265f / 180.0f;
        camera.pitch = options.camera[4] * 3.14159265f / 180.0f;
    }

    renderer::FrameBuffer frame;
    renderer::FrameInput input;
    input.clear_colour = {level.background[0], level.background[1], level.background[2], 1.0f};
    viewer::FlyControls controls;
    bool looking = false;
    bool running = true;
    int status = 0;
    platform::Nanoseconds last = platform::now_ns();
    const platform::Nanoseconds start = last;

    for (std::uint64_t index = 0; running; ++index) {
        controls.look_x = 0.0f;
        controls.look_y = 0.0f;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        looking = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
                        SDL_SetWindowRelativeMouseMode(window->sdl(), looking);
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    if (looking) {
                        controls.look_x += event.motion.xrel;
                        controls.look_y += event.motion.yrel;
                    }
                    break;
                case SDL_EVENT_MOUSE_WHEEL:
                    camera.speed = std::max(1.0f, camera.speed * std::pow(1.25f, event.wheel.y));
                    break;
                case SDL_EVENT_KEY_DOWN: {
                    const SDL_Keycode key = event.key.key;
                    if (key == SDLK_ESCAPE) {
                        running = false;
                    } else if (key >= SDLK_1 && key <= SDLK_5) {
                        auto& r = renderer.renderers()[static_cast<std::size_t>(key - SDLK_1)];
                        r->enabled = !r->enabled;
                        log::info("{}: {}", r->name(), r->enabled ? "shown" : "hidden");
                    } else if (key == SDLK_L) {
                        scene.lighting = !scene.lighting;
                    } else if (key == SDLK_F11) {
                        const bool windowed =
                            window->display_mode() == platform::DisplayMode::Windowed;
                        window->set_display_mode(
                            windowed ? platform::DisplayMode::Borderless
                                     : platform::DisplayMode::Windowed
                        );
                    }
                    break;
                }
                default:
                    break;
            }
        }
        if (!headless) {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            controls.forward = keys[SDL_SCANCODE_W];
            controls.back = keys[SDL_SCANCODE_S];
            controls.left = keys[SDL_SCANCODE_A];
            controls.right = keys[SDL_SCANCODE_D];
            controls.down = keys[SDL_SCANCODE_Q];
            controls.up = keys[SDL_SCANCODE_E];
            controls.fast = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
        }
        const platform::Nanoseconds now = platform::now_ns();
        const float seconds = static_cast<float>(now - last) / 1e9f;
        last = now;
        camera.update(controls, std::min(seconds, 0.1f));

        int width = 0;
        int height = 0;
        window->drawable_size(width, height);
        if (width <= 0 || height <= 0) {
            width = options.width;
            height = options.height;
        }
        if (frame.width() != width || frame.height() != height) {
            if (!frame.create(width, height, error)) {
                log::error("{}", error);
                status = 1;
                break;
            }
        }
        input.camera.view = camera.view();
        input.camera.projection =
            camera.projection(static_cast<float>(width) / static_cast<float>(height));
        input.camera.position = camera.position;
        if (game) {
            input.camera.view = game->view();
            input.camera.projection = game->projection(
                static_cast<float>(width) / static_cast<float>(height), 0.05f, 2000.0f
            );
            input.camera.position = game->camera_position;
        }
        input.frame = index;
        scene.update_probe_regions(input.camera, seconds);
        input.after_world_depth =
            scene.compute_probe_selection()
                ? std::function<void(unsigned, int, int)>([&](unsigned depth, int width, int height
                                                          ) {
                      scene.select_visible_probe_regions(depth, input.camera, width, height);
                  })
                : std::function<void(unsigned, int, int)>{};
        input.seconds = static_cast<double>(now - start) / 1e9;
        renderer.render(input, {frame.id(), width, height});
        frame.blit_to(0, width, height);
        window->swap();

        if (headless && index + 1 >= static_cast<std::uint64_t>(options.frames)) {
            const auto& stats = renderer.last_stats();
            if (!viewer::write_png(options.screenshot, width, height, frame.read_rgba())) {
                log::error("could not write {}", options.screenshot);
                status = 1;
            } else {
                log::info(
                    "{}: {} draw calls, {} triangles",
                    options.screenshot,
                    stats.draw_calls,
                    stats.triangles
                );
            }
            running = false;
        }
    }
    frame.release();
    scene.release();
    renderer.release();
    return status;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    if (!parse(argc, argv, options)) {
        std::fputs(kUsage, stderr);
        return 2;
    }
    return run(options);
}
