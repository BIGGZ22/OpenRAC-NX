// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the debug overlay's files share, and nothing outside the overlay includes: its state, the
// constants and types the state is made of, and the helpers its panels draw with. debug_overlay.cpp
// implements debug_overlay.h with them; debug/overlay.cpp defines the state and the helpers, and
// each panel is defined in the file of debug/ named for its subject.
//
// The state is file-global, as it was inside debug_overlay.cpp; it is to become an object passed
// to what needs it (CODING_CONVENTIONS.md, sections 11.4 and 24). The window's thread, the one
// that calls debug_overlay.h's functions, is the only one that reads or writes it.
//
// The viewer's types (viewer::GameState, viewer::LiveMoby) come through debug_overlay.h: its
// include of viewer/game_state.h is the layering's listed exception (rules.tsv, until P2-03), and
// the overlay's other files reach them through it rather than adding includes of their own.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <imgui.h>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "debug_overlay.h"

namespace openrac::frontend::debug_overlay {

constexpr std::uint32_t kRamSize = 32u * 1024 * 1024;
constexpr std::uint32_t kMobySize = 0x100;

// The sections of the panel, as OPENRAC_OVERLAY names them.
enum Section : std::size_t { kFrame, kRenderer, kMemory, kMobys, kGame, kShaders, kSectionCount };
constexpr std::array<const char*, kSectionCount> kSectionNames = {"frame", "renderer", "memory",
                                                                 "mobys", "game", "shaders"};

constexpr std::array<const char*, 19> kLevelNames = {
    "Veldin", "Novalis", "Aridia", "Kerwan", "Eudora", "Rilgar", "Blarg Station", "Umbris", "Batalia", "Gaspar",
    "Orxon", "Pokitaru", "Hoven", "Gemlik Base", "Oltanis", "Quartu", "Kalebo III", "Drek's Fleet", "Veldin (final)"};

// The pad's buttons by bit (active low in the report).
constexpr std::array<const char*, 16> kButtonNames = {"select", "L3", "R3", "start", "up", "right", "down", "left",
                                                      "L2", "R2", "L1", "R1", "triangle", "circle", "cross", "square"};

// The main memory map: one cell per 128 KB.
constexpr std::uint32_t kCellBytes = 128 * 1024;
constexpr std::size_t kCells = kRamSize / kCellBytes;

struct RamCell {
    std::uint16_t used_pages = 0;  // of 32 (4 KB each) holding anything but zero
    std::uint64_t sum = 0;
    bool changed = false;
};

// A scripted key or click (OPENRAC_KEYS).
struct ScriptedInput {
    std::uint64_t frame = 0;  // the window frame it happens at
    int held = 0;             // and that many held pictures later, while paused there
    SDL_Keycode key = 0;      // 0: a click
    float x = 0.0f, y = 0.0f;
    bool done = false;
};

// The card a section of the drawer's rows is drawn on (card_begin).
struct Card {
    ImVec2 top;
    float width = 0.0f;
    int rows = 0;
};

struct State {
    std::vector<ScriptedInput> script;
    SDL_Window* window = nullptr;  // as last drawn into
    int held_pictures = 0;         // pictures of the tick held now
    bool wanted = false;      // shown from the start
    bool configured = false;  // the environment was read
    bool visible = false;
    bool initialised = false;  // the ImGui context exists
    bool gl_subset = false;    // drawn with the renderer's GL (imgui_gl.h): the window's is Vulkan's
    std::array<bool, kSectionCount> open_at_start = {true, false, false, false, false, false};

    // The last tick, as the window gave it.
    std::string game;
    std::span<const std::uint8_t> ram;
    int level = -1;
    viewer::GameState state;
    std::uint32_t draws = 0;
    std::uint64_t tick = 0;
    Tick counts;
    std::uint16_t buttons = 0xFFFF;
    std::uint8_t analog[4] = {128, 128, 128, 128};

    // Pause, step, slow motion.
    bool paused = false;
    bool step = false;
    double stretch = 1.0;
    long long pause_at = -1;
    long long reload_at = -1;  // OPENRAC_SHADER_RELOAD: the tick to read the shaders again at
    int held_frames = 0;  // frames to hold before going on by itself (a scripted pause); 0: for good

    // The renderer's passes hidden here, by name; wireframe.
    std::set<std::string> hidden;
    bool wireframe = false;
    bool wireframe_set = false;

    // Mobys.
    std::uint32_t selected = 0;  // address; 0 none
    bool follow_hero = false;    // "inspect": the hero is selected, whichever moby he is now
    bool inspector = false;
    bool labels = false;
    bool near_hero = true;
    float radius = 30.0f;
    bool pick = false;  // a click in the game's picture selects the nearest moby

    // Memory.
    bool memory_viewer = false;
    std::uint32_t view_address = 0x00100000;
    bool view_words = false;
    std::array<RamCell, kCells> cells{};
    std::uint64_t scanned_at = ~0ull;
    std::uint32_t used_pages = 0;

    // Screenshots of the window, with the overlay.
    std::string shot_path;        // write the next frame there
    std::uint64_t shot_every = 0;  // OPENRAC_OVERLAY_SHOT=N:PREFIX
    std::string shot_prefix;
    std::uint64_t shot_from = 0;
    int shots = 0;

    std::string message;  // what the last action said (a reload, a file written)
    std::string notice;   // kept in the picture until cleared (debug_overlay::notice)
    std::string toast;    // the same for an action taken with the panel hidden, shown a moment
    double toast_until = 0.0;
    bool alt_alone = false;  // Alt went down and no other key since: its release toggles the menus
    bool developer = false;  // the launcher's developer tools (OPENRAC_DEVELOPER): the level select
    std::string dump_path = "openrac_frames.jsonl";

    // The drawer (drawer_available()).
    bool drawer_open = false;
    float drawer_at = 0.0f;      // how far out: 0 shut, 1 open
    bool keep_paused = false;    // the pause the game is left in when it shuts
    bool drawer_focus = false;   // given the keyboard and gamepad when it opens
    bool drawer_scrolled = false;  // the finger down now has dragged the rows: no row is pressed
    bool shot_when_shut = false;   // a screenshot asked for in it, taken once it is out of the picture
    bool drawer_developer = false;  // its developer tools unfolded
    Card card;
    bool settings_changed = false;  // a setting changed in the drawer or the menus, not applied yet
    bool panel_scrolled = false;    // the panel's drag_scroll
    float ui_scale = 1.0f;         // the screen's density, for rows a thumb can press
    // Fingers on the screen as ImGui's pointer: the first one down, and those the drawer took from
    // the on-screen pad (their moves and lifts are the drawer's too).
    SDL_FingerID pointer = 0;
    bool pointer_down = false;
    std::vector<SDL_FingerID> taken;

    // The processor's load, measured each half second.
    double cpu_seconds = 0.0;
    double cpu_wall = 0.0;
    double cpu_load = 0.0;
};

// The overlay's state: what the last tick gave it and what its panels and windows were set to.
// Defined in debug/overlay.cpp; only the window's thread touches it.
extern State s;

struct Averages {
    int frames = 0, ticks = 0;
    double frame_ms = 0.0, worst_ms = 0.0;
    double fps = 0.0, tps = 0.0;
    double game_ms = 0.0, scene_ms = 0.0, wait_ms = 0.0;                         // per tick
    double prepare_ms = 0.0, render_ms = 0.0, present_ms = 0.0, overlay_ms = 0.0, swap_ms = 0.0;  // per frame
    double gpu_ms = -1.0;
    int gpu_frames = 0;
};

// The colours of the panels' text: a warning, a good result, a note, a selection.
extern const ImVec4 kRed;
extern const ImVec4 kGreen;
extern const ImVec4 kDim;
extern const ImVec4 kYellow;

// Returns `v`, a length in the overlay's units, in pixels: `v` on a desktop's screen, more on a
// dense one (s.ui_scale).
float dp(float v);

// Says whether the `size` bytes at `address` (a game address, its segment bits ignored) are all in
// the game's main memory as the last tick gave it.
bool in_ram(std::uint32_t address, std::uint32_t size);

// Returns the moby class names by class number, read once from the editor's table
// (openrac_moby_class_names, cmake/Games.cmake).
const std::unordered_map<int, std::string>& class_names();

// Returns the name of moby class `id`, or the empty string for a class the table does not name.
std::string class_name(int id);

// Returns the game function at `address` as the panels show it: its address and its name, "(no C)"
// for one without C, "none" for 0.
std::string function_name(std::uint32_t address);

// Returns the hero's moby (class 0) among the last tick's live mobys, or nullptr when there is none.
// The pointer lives until the next tick.
const viewer::LiveMoby* hero();

// Returns the selected moby (s.selected) among the last tick's live mobys, or nullptr when none is
// selected or it is gone. The pointer lives until the next tick.
const viewer::LiveMoby* selected_moby();

// Returns the distance between the positions of mobys `a` and `b`, in the game's units.
float distance(const viewer::LiveMoby& a, const viewer::LiveMoby& b);

// Returns `bytes` as megabytes with one decimal ("12.3 MB").
std::string megabytes(std::uint64_t bytes);

// Draws one row of a two-column table: `label`, then `value`. Inside an ImGui table only.
void row(const char* label, const std::string& value);

// Draws one line of text, formatted as std::format does.
template <typename... Args>
void text(std::format_string<Args...> fmt, Args&&... args) {
    ImGui::TextUnformatted(std::format(fmt, std::forward<Args>(args)...).c_str());
}

// Draws one line of text in `colour`, formatted as std::format does.
template <typename... Args>
void text_coloured(const ImVec4& colour, std::format_string<Args...> fmt, Args&&... args) {
    ImGui::PushStyleColor(ImGuiCol_Text, colour);
    ImGui::TextUnformatted(std::format(fmt, std::forward<Args>(args)...).c_str());
    ImGui::PopStyleColor();
}

// Returns the T at `address` in the game's main memory (its segment bits ignored), or a T of
// zeros when it is not all inside.
template <typename T>
T read(std::uint32_t address) {
    T value{};
    address &= 0x01FFFFFF;
    if (static_cast<std::size_t>(address) + sizeof(T) <= s.ram.size()) {
        std::memcpy(&value, s.ram.data() + address, sizeof(T));
    }
    return value;
}

}  // namespace openrac::frontend::debug_overlay
