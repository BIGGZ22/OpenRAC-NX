// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's shared state: the window, the renderer and the scene the game drew (State, behind
// `g`), the tick kept for the frames drawn between ticks (Kept), and when frames reach the display
// (Pacing, `pacing`), and the functions frontend.cpp and frontend/*.cpp call across files. The
// frontend is cut into files by concern around this one state, and each file reaches it here.
//
// Internal to the frontend: only frontend.cpp and frontend/*.cpp include this header. Anything
// else talks to the window through frontend.h. The state is file-global as it was in frontend.cpp;
// it becomes an object passed to what needs it later (CODING_CONVENTIONS.md, section 11.4).

#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "enhanced/game/scripted_moby_colour.h"
#include "openrac/guest.h"
#include "platform/input.h"
#include "platform/window.h"
#include "renderer/framebuffer.h"
#include "renderer/renderer.h"
#include "renderer/shadows.h"
#include "scene/level.h"
#include "scene/level_renderer.h"
#include "scene/moby_shadow.h"
#include "settings.h"
#include "viewer/game_state.h"

union SDL_Event;

// The environment's variables (common/env.h), named here only for env_string's declaration.
namespace openrac::env {
enum class Var : std::uint16_t;
}  // namespace openrac::env

namespace openrac::frontend {

// One tick's scene, as frame() read it from the game, for the frames drawn until the next tick
// (between), and the last tick's camera and moby matrices to move them from.
struct Kept {
    int level = -3;  // the level the mobys belong to: another one starts over
    viewer::GameState state, prev_state;
    bool has_state = false, has_prev_state = false;
    std::vector<viewer::Instance> live;
    std::vector<std::uint32_t> addresses;  // each live moby's address, for its last matrix
    std::unordered_map<std::uint32_t, renderer::Mat4> prev_matrices;
    std::unordered_map<std::uint32_t, std::array<float, 4>> prev_spheres;
    std::vector<viewer::LevelScene::JointColumns> palette;
    std::vector<renderer::ShadowCapsule> shadows, prev_shadows;
    std::uint32_t shadow_moby = 0, prev_shadow_moby = 0;
    viewer::MobyShadowState shadow_state, prev_shadow_state;
    std::optional<bool> hero_airborne;
    // The last tick's poses, by moby: where its joints start in prev_palette and how many.
    std::vector<viewer::LevelScene::JointColumns> prev_palette;
    std::unordered_map<std::uint32_t, std::pair<int, int>> prev_poses;
    std::vector<int> joints;            // each live moby's joints in `palette` (0 without a pose)
    std::vector<std::uint8_t> packets;  // the 2D path
    std::size_t before_world = 0;
    std::vector<renderer::EffectVertex> strip_vertices;
    std::vector<renderer::EffectStrip> strips;
    std::vector<renderer::EffectQuad> effects;
    std::vector<renderer::ImageUpload> effect_uploads;
    std::array<renderer::Mat4, 8> sky_shells{};
    unsigned sky_shell_mask = 0;
    std::vector<renderer::UiQuad> ui_quads;

    // Mobys the game drew into a texture with a camera of its own (the vendor's screens): each
    // drawn at its marker in the 2D path (renderer::FrameInput::target_mobys).
    struct TargetMoby {
        std::uint32_t address = 0;
        viewer::Instance instance;
        viewer::GameState camera;
    };

    std::vector<TargetMoby> target_mobys;
    std::uint32_t draws = 0;
    bool has_camera = false;
    std::uint64_t frame = 0;
};

// Everything the window holds while it is open: the window and the renderer, the level drawn, what
// the game drew since the last frame (gathered for frame()), and the window as the settings left it.
struct State {
    std::string game;
    ScriptedMobyColours scripted_moby_colours;
    std::filesystem::path levels;
    std::unique_ptr<platform::Platform> sdl;
    std::unique_ptr<platform::Window> window;
    platform::Input input;
    bool capture_wanted = false;  // a mod's asking (capture_mouse)
    bool keyboard_taken = false;  // a mod took the keyboard (take_keyboard)
    std::unique_ptr<renderer::Renderer> renderer;
    renderer::FrameBuffer frame;
    renderer::FrameBuffer picture;  // show_picture's: a movie frame, a boot still
    // The last frame the game drew its world in: what its draw buffer still holds on a frame it
    // draws nothing in (the tick a page menu opens), and so what reading the frame back answers.
    renderer::FrameBuffer last_world;
    viewer::LevelData level;
    viewer::LevelScene scene;
    int loaded = -2;  // the level whose geometry is uploaded; -2 none
    std::uint64_t index = 0;
    // Every image the game sent outside the display list, oldest first, the latest per
    // rectangle: given again to each renderer made (use_level makes a new one per level).
    std::vector<renderer::ImageUpload> images;
    // The camera the game's world renderers drew with this frame (world_drawn), if they ran.
    std::optional<viewer::GameState> world_camera;
    // Mobys drawn this frame with a camera of their own (mobys_drawn): address, that camera.
    std::vector<std::pair<std::uint32_t, viewer::GameState>> moby_cameras;
    // The world effect quads the game drew since the last frame (effect_quad).
    std::vector<renderer::EffectQuad> effects;
    std::vector<renderer::ImageUpload> effect_uploads;  // the quads' textures (EffectQuad::uploads)
    // The sky shells the game drew since the last frame, with their matrices (sky_shell).
    std::array<renderer::Mat4, 8> sky_shells{};
    unsigned sky_shell_mask = 0;

    // The live particles the game drew since the last frame (particle): record, texture source.
    struct Particle {
        std::array<std::uint8_t, 0x40> record;
        std::uint32_t pixels, clut;
        int log2_side;
    };

    std::vector<Particle> particles;
    // The particle textures decoded so far (particle_texture): pixels and palette hash -> pool key,
    // for the renderer that holds them.
    std::unordered_map<std::uint64_t, std::uint32_t> particle_textures;
    const void* particle_textures_for = nullptr;
    // The sky sprites the game drew since the last frame (sky_sprite): record, TEX0.
    std::vector<std::pair<std::array<std::uint8_t, 0x20>, std::uint64_t>> sky_sprites;
    // The 2D layer's quads drawn natively since the last frame (ui_quad), by marker index.
    std::vector<renderer::UiQuad> ui_quads;
    // The quads of the last ticks that drew any, by generation: a marker names the generation of
    // the tick that recorded it (ui_quad), and a list is drawn with that tick's quads (frame).
    std::uint32_t ui_generation = 0;
    std::array<std::pair<std::uint32_t, std::vector<renderer::UiQuad>>, 4> ui_kept{};
    // The last world drawn (last_world): the tick it was drawn on; whether the last tick was a page
    // menu's with its snapshot; whether this tick's frames go over the last world (draw_kept).
    std::uint64_t last_world_tick = 0;
    bool last_tick_menu = false;
    bool hold_last_world = false;
    // Ticks in a row on which the game drew no world while its 2D list was a page menu's (the
    // picture read back, at least kSnapshotBytes): the ticks after the pause menu closes. The
    // picture they would make is the menu without its framed models, then the dimmed picture
    // alone; the console keeps showing its last frame meanwhile, and so does the port (frame).
    int menu_gap_ticks = 0;
    bool repeat_frame = false;
    // What the window is in now of the player's settings (apply_settings): the display mode, the
    // display and the fullscreen size; the picture's shape; the swap's wait as asked and as the
    // driver took it; whether the sound is held silent (the window lost the keyboard).
    settings::DisplayMode display_mode = settings::DisplayMode::Windowed;
    int monitor = 0;
    int fullscreen_width = 0;
    int fullscreen_height = 0;
    settings::Aspect aspect = settings::Aspect::Standard;
    int aspect_custom_x = 0;
    int aspect_custom_y = 0;
    platform::SwapInterval swap_asked = platform::SwapInterval::VSync;
    platform::SwapInterval swap = platform::SwapInterval::VSync;
    bool muted = false;
    // The tick's own frame was left undrawn this many ticks in a row (frame(): the game is late).
    int skipped = 0;
    Kept kept;
};

// The frontend's state: made by open, dropped by close (frontend.h); null before and after. Only
// the thread that opened the window touches it.
extern std::unique_ptr<State> g;

// When frames reach the display, for drawing between ticks (set_tick, between): the tick the kept
// scene belongs to and the game's tick length; the last swap that waited (a refresh) and the
// display's refresh interval; what drawing a frame costs before its swap.
struct Pacing {
    std::chrono::steady_clock::time_point tick{};
    std::chrono::duration<double> period{0.02};
    std::chrono::steady_clock::time_point last_swap{};
    double interval = 0.0;  // seconds; 0 when the display does not say
    // The display's refresh rate is to be read again: at start, and whenever the window says its
    // display or that display's mode changed (window_event), not by asking every so many frames.
    bool interval_stale = true;
    double cost = 0.004;                      // seconds
    std::chrono::duration<double> late{0.0};  // how long after its time the tick came (set_tick)
    float last_alpha = -1.0f;                 // how far between ticks the last frame was drawn
    std::chrono::steady_clock::time_point due{};  // when the frame cap lets the next frame start
    // The kept tick has not been drawn yet (the cap held its own frame back): the next frame drawn
    // is its first.
    bool first_pending = false;
};

// When frames reach the display, for the window's whole life. Only the thread that opened the
// window touches it.
extern Pacing pacing;

// Where a scripted run is (port/tools/regress): the game mode as frame() last read it, and the
// window frame at which the player got control (-1 before). Control is a level's game mode 0 (play)
// held for kControlHold ticks in a row: a level's program can show mode 0 for a few ticks between
// loading and its arrival scene, and those are no control.
constexpr int kControlHold = 30;

struct Route {
    int level = -1;
    int mode = -1;
    int streak = 0;
    std::int64_t control = -1;
    // The window frame at which what plays now became skippable (-1 while nothing can be skipped):
    // OPENRAC_PRESS's s entries count from it, so a skip falls on the same tick of a scene in every
    // version, wherever the scene starts in window frames.
    std::int64_t skippable_from = -1;
};

// Where a scripted run is, as frame() last read it (frontend.cpp).
extern Route g_route;

// OPENRAC_SHOT=EVERY:PATH: a PNG of every EVERY-th frame, written to PATH<frame>.png.
struct Shots {
    std::uint64_t every = 0;  // 0: no shots (unset, no ':', or EVERY not above 0)
    std::string prefix;       // PATH, what the frame's number and .png follow

    // Returns whether `frame` is one to take.
    [[nodiscard]] bool due(std::uint64_t frame) const { return every > 0 && frame % every == 0; }
};

// Whether Start would skip what is playing now (frontend.cpp).
bool skippable();

// Returns the frame number `var` names, read as std::strtoull reads it, or nothing when it is
// unset (frontend.cpp).
std::optional<std::uint64_t> env_frame(env::Var var);

// Returns OPENRAC_SHOT read, once (frontend.cpp).
const Shots& shots();

// Whether a development switch keeps one frame a tick whatever the settings say (frontend.cpp).
bool one_frame_a_tick();

// Draws `framebuffer` (width x height) into the window's box for a picture of shape `aspect`, black
// around it; `flip`: its first row is the top (frontend.cpp).
void present(unsigned framebuffer, int width, int height, int window_width, int window_height, bool flip,
             float aspect);

// Returns the value `var` was set to as a string of its own, or nothing when it is unset
// (frontend.cpp).
std::optional<std::string> env_string(env::Var var);

// The picture's shape now: the player's setting, for the window as it is (frontend.cpp).
float picture_aspect(int window_width, int window_height);

// Whether a swap waits for a refresh: the player's vsync setting, unless one frame is drawn a tick
// (frontend.cpp says why).
platform::SwapInterval wanted_swap();

// The platform's display mode for the player's setting (frontend.cpp).
platform::DisplayMode window_mode(settings::DisplayMode mode);

// Puts the player's display settings into effect on the window: the display mode, display and
// fullscreen size, the picture's shape, the swap's wait (frontend/window.cpp).
void apply_settings();

// Handles the window's own events and keys, which the game never sees: true when `event` was one
// of the keys (frontend/window.cpp).
bool window_key(const SDL_Event& event);

// Draws the on-screen pad over the frame, inside the screen's safe area (frontend/input.cpp).
void draw_touch_pad(int window_width, int window_height);

// Captures the cursor as the mods ask for it, while nothing else has the mouse
// (frontend/input.cpp).
void apply_mouse_capture();

// The directional light bank, in the executable's terms (relocated to the loaded program's copy);
// 0 for a game without one (frontend/world.cpp).
gaddr light_bank_address();

// The four shadow directions in each version's executable (relocated when read); 0 for a game
// without them (frontend/world.cpp).
gaddr shadow_directions_address();

// The hero's state in each version's executable; 0 for a game without one (frontend/world.cpp).
gaddr hero_state_address();

// Uploads level `number`'s geometry and makes the renderers that draw it. Below 0 the boot program
// runs: the title world (level 0 where it was not extracted), or at -2 the flight between planets
// where it was (frontend/world.cpp).
void use_level(int number);

// The sky sprites the game drew since the last frame, as effect quads seen from `state`'s camera,
// put first among the frame's effect quads: drawn behind everything (frontend/world.cpp).
void sky_sprite_quads(const viewer::GameState& state);

// The particles the game drew since the last frame, as effect quads back to front, seen from
// `state`'s camera; their textures read from `ram` (frontend/world.cpp).
void particle_quads(std::span<const std::uint8_t> ram, const viewer::GameState& state);

// Whether the frame cap lets a frame start now (frontend/interpolation.cpp).
bool frame_due(std::chrono::steady_clock::time_point now);

// Draws the kept tick, the camera and mobys part way from the last tick's to its own, and shows
// it. `first`: the tick's own frame (frontend/interpolation.cpp).
void draw_kept(bool first);

}  // namespace openrac::frontend
