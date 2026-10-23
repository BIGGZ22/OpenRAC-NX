// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The player's settings for a native game: what the port adds to the game's own options (the
// window and the display it is on, the picture's shape and resolution, anti-aliasing and texture
// filtering, how many frames are drawn and whether they wait for the display, how loud). They are
// kept in a small text file, `key = value` a line, beside the memory cards (`--settings FILE`
// names another), read once at start and written whenever one changes.
// docs/port/SETTINGS.md lists every key, and what OpenGOAL and ReRAC offer beside them.
#pragma once

#include <filesystem>
#include <string>

#include "enhanced/renderer/gtao_settings.h"
#include "renderer/texture_filter.h"

namespace openrac::settings {

// The window. Borderless covers the desktop at the desktop's mode; Fullscreen takes the display
// for itself, at `Settings::fullscreen_width` x `fullscreen_height` (0 x 0: the desktop's mode).
enum class DisplayMode {
    Windowed,
    Borderless,
    Fullscreen,
    Count,
};

// The picture's shape. The game draws for a 4:3 television; a wider picture shows more of the
// world to the sides (the vertical field of view is the game's), with the 2D layer where it was.
enum class Aspect {
    Standard,   // 4:3, the game's own
    Wide16x10,  // 16:10
    Wide16x9,   // 16:9
    Wide21x9,   // 21:9
    Wide32x9,   // 32:9
    Window,     // whatever the window's shape is (never narrower than 4:3)
    Custom,     // Settings::aspect_custom_x : aspect_custom_y (the file's `aspect_custom`)
    Count,
};

// Where the game's HUD goes in a picture wider than the game's.
enum class Hud {
    Centre,  // where the game put it: inside the 4:3 picture in the middle
    Edges,   // what the game anchors to the screen's left or right goes out to the picture's sides
    Count,
};

// Whether a frame waits for the display's refresh.
enum class VSync {
    Off,
    On,
    Adaptive,  // waits, but a late frame is shown at once (it tears) rather than a refresh later
    Count,
};

// How many frames are drawn. The game's tick stays its own (50 a second for PAL): the frames
// between two ticks move the camera, the mobys and their poses from one tick to the next.
enum class FrameRate {
    Original,   // one frame a tick
    Display,    // the display's refresh rate
    Capped,     // Settings::frame_cap frames a second
    Unlimited,  // as many as the machine draws
    Count,
};

struct Settings {
    DisplayMode display_mode = DisplayMode::Windowed;
    int monitor = 0;  // 0: the display the window is on; n: the nth display, counted from 1
    int fullscreen_width = 0;  // DisplayMode::Fullscreen's mode; 0 x 0 is the desktop's
    int fullscreen_height = 0;
    int window_width = 0;  // the window's size when last windowed; 0 x 0: the port's default
    int window_height = 0;
    Aspect aspect = Aspect::Standard;
    int aspect_custom_x = 16;
    int aspect_custom_y = 9;
    Hud hud = Hud::Centre;
    int render_scale = 100;  // the frame's resolution, as a percentage of the window's picture
    int msaa = 1;            // samples a pixel: 1 (off), 2, 4, 8, 16
    renderer::GtaoSettings gtao_settings;
    // Enhanced rendering at all (port/enhanced; the launcher's Mods entry): read at start, Vulkan
    // only. Off, nothing of it is made and the game draws as a build without it.
    bool enhanced_rendering = false;
    bool enhanced_graphics = true;  // F4 within it; retains individual feature preferences
    bool enhanced_shadows = true;   // cascades and Ratchet map; off restores original shadows
    bool probe_lighting = true;     // uses a verified bake when one is loaded
    bool probe_spheres = false;     // diagnostic placement/fade overlay
    float probe_gi_multiplier = 1.0f;
    float probe_normal_bias = 0.5f;  // receiver offsets in world units
    float probe_view_bias = 0.5f;
    bool gtao = true;        // ground-truth ambient occlusion when enhanced rendering is enabled
    renderer::TextureFilter texture_filter = renderer::TextureFilter::EdgeAware;
    int anisotropy = 0;      // 0: mode default; otherwise 1 (off), 2, 4, 8, 16
    int anisotropy_samples(bool enhanced) const {
        return anisotropy ? anisotropy : enhanced ? 4 : 1;
    }
    VSync vsync = VSync::On;
    FrameRate frame_rate = FrameRate::Display;
    int frame_cap = 60;  // FrameRate::Capped's frames a second
    bool show_fps = false;  // the frame-rate counter in the picture's corner
    bool menu_bar = true;   // the menus along the window's top (not on macOS, whose menu bar has them); Alt hides and shows it
    bool mute_unfocused = false;  // silence while another window has the keyboard
    bool skip_cutscenes = true;   // Start skips an in-game cutscene (without the game's L1 L2 R1 R2 hold)
    int volume = 100;  // of everything the port plays, 0 (nothing at all) to 100 (as mixed)
    bool verbose_log = false;  // the detailed log (as OPENRAC_LOG=verbose): level loads, pad changes, callers
};

// The settings in use.
Settings& get();

// Reads `file` (missing or unreadable: the defaults) and remembers it for save().
void load(const std::filesystem::path& file);

// The settings in one line, for the log and its header.
std::string summary();

// Writes the settings to the file load() was given; nothing without one.
void save();

// Sets the volume (held to 0..100). output_gain() is it as a factor, for the sound's own thread;
// set_muted(true) holds the factor at 0 whatever the volume is (the window lost the keyboard).
void set_volume(int volume);
void set_muted(bool muted);
float output_gain();

// Width over height of the picture for a window of that size.
float aspect_ratio(const Settings& settings, int window_width, int window_height);

// The frame caps the options page offers, in frames a second, and the ones a file may hold.
inline constexpr int kFrameCaps[] = {30, 60, 75, 90, 120, 144, 165, 240, 360};
inline constexpr int kFrameCapMin = 20;
inline constexpr int kFrameCapMax = 1000;

// The render scales, sample counts and anisotropies the options page offers.
inline constexpr int kRenderScales[] = {50, 75, 100, 125, 150, 200};
inline constexpr int kSampleCounts[] = {1, 2, 4, 8, 16};
inline constexpr int kAnisotropies[] = {1, 2, 4, 8, 16};

// What the options page shows for a value.
const char* name(DisplayMode mode);
const char* name(Aspect aspect);
const char* name(Hud hud);
const char* name(VSync vsync);
const char* name(renderer::TextureFilter filter);
std::string frame_rate_name(const Settings& settings);

}  // namespace openrac::settings
