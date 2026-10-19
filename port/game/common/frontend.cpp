// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "frontend.h"
#include "debug_menu.h"
#include "debug_overlay.h"
#include "fps_counter.h"
#include "touch_overlay.h"
#include "options_menu.h"
#include "settings.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <memory>
#include <optional>
#include <vector>

#include "common/log.h"
#include "common/stats.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "platform/gpu_driver.h"
#include "platform/input.h"
#include "platform/performance_hint.h"
#include "platform/pump.h"
#include "platform/window.h"
#include "renderer/direct.h"
#include "renderer/effects.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/gl_stats.h"
#include "renderer/renderer.h"
#include "viewer/game_state.h"
#include "viewer/level.h"
#include "viewer/level_renderer.h"
#include "viewer/moby_pose.h"
#include "viewer/screenshot.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" int openrac_game_loaded_overlay(void);

#include <stb_image.h>

extern "C" {
extern const unsigned char openrac_icon_png[];
extern const unsigned int openrac_icon_png_size;
}

// The game's name and version, "Ratchet & Clank (PAL)" (cmake/Games.cmake).
#ifndef OPENRAC_GAME_TITLE
#define OPENRAC_GAME_TITLE "OpenRAC"
#endif

namespace openrac::frontend {

namespace {

// One tick's scene, as frame() read it from the game, for the frames drawn until the next tick
// (between), and the last tick's camera and moby matrices to move them from.
struct Kept {
    int level = -3;  // the level the mobys belong to: another one starts over
    viewer::GameState state, prev_state;
    bool has_state = false, has_prev_state = false;
    std::vector<viewer::Instance> live;
    std::vector<std::uint32_t> addresses;  // each live moby's address, for its last matrix
    std::unordered_map<std::uint32_t, renderer::Mat4> prev_matrices;
    std::vector<viewer::LevelScene::JointColumns> palette;
    // The last tick's poses, by moby: where its joints start in prev_palette and how many.
    std::vector<viewer::LevelScene::JointColumns> prev_palette;
    std::unordered_map<std::uint32_t, std::pair<int, int>> prev_poses;
    std::vector<int> joints;  // each live moby's joints in `palette` (0 without a pose)
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

struct State {
    std::string game;
    std::filesystem::path levels;
    std::unique_ptr<platform::Platform> sdl;
    std::unique_ptr<platform::Window> window;
    platform::Input input;
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

std::unique_ptr<State> g;

std::uint32_t word_at(std::span<const std::uint8_t> ram, std::uint32_t at) {
    std::uint32_t w = 0;
    at &= 0x01FFFFFF;
    if (at + 4 <= ram.size()) {
        std::memcpy(&w, ram.data() + at, 4);
    }
    return w;
}

/*
 * The GIF data a frame's display list sends straight to the GS (VIF DIRECT): the 2D path (HUD,
 * menus, text, fades). The chain is followed as the DMA controller follows a source chain (the
 * tag's upper half goes to VIF, as the game's lists are sent with TTE), and the VIF codes that feed
 * VU1 (UNPACK, MPG and the rest) are stepped over by their sizes: what VU1 draws, the world, has
 * renderers of its own.
 */
std::vector<std::uint8_t> direct_packets(std::span<const std::uint8_t> ram, std::uint32_t chain,
                                         std::vector<renderer::EffectVertex>* strip_vertices = nullptr,
                                         std::vector<renderer::EffectStrip>* strips = nullptr,
                                         std::vector<renderer::ImageUpload>* uploads = nullptr,
                                         std::size_t* before_world = nullptr,
                                         std::size_t* first_offscreen = nullptr,
                                         std::size_t* world_marker = nullptr) {
    std::vector<std::uint8_t> vif;
    std::uint32_t tag_at = chain & 0x01FFFFF0;
    std::uint32_t stack[2] = {0, 0};
    int depth = 0;
    auto append = [&](std::uint32_t at, std::uint32_t bytes) {
        at &= 0x01FFFFFF;
        if (at + bytes <= ram.size()) {
            vif.insert(vif.end(), ram.data() + at, ram.data() + at + bytes);
        }
    };
    // A tag whose address has bit 31 set sends its data from the scratchpad (16 KB at 0x70000000
    // in game memory), where the game builds some packets (the page menus' 2D setup among them).
    auto append_scratchpad = [&](std::uint32_t at, std::uint32_t bytes) {
        at &= 0x3FF0;
        const std::uint8_t* spr = static_cast<const std::uint8_t*>(G(0x70000000u));
        if (at + bytes <= 0x4000u) {
            vif.insert(vif.end(), spr + at, spr + at + bytes);
        }
    };
    for (int steps = 0; tag_at != 0 && steps < 20000; ++steps) {
        const std::uint32_t lo = word_at(ram, tag_at);
        const std::uint32_t raw_addr = word_at(ram, tag_at + 4);
        const bool from_spr = (raw_addr & 0x80000000u) != 0;
        const std::uint32_t addr = raw_addr & 0x7FFFFFF0;
        const std::uint32_t qwc = lo & 0xFFFF;
        const std::uint32_t id = (lo >> 28) & 7;
        append(tag_at + 8, 8);  // the two VIF codes in the tag
        const std::uint32_t after = tag_at + 16;
        if (from_spr && id != 0 && id != 3 && id != 4) {
            break;  // a chain continuing in the scratchpad: not followed
        }
        switch (id) {
            case 0:  // refe
                from_spr ? append_scratchpad(addr, qwc * 16) : append(addr, qwc * 16);
                tag_at = 0;
                break;
            case 1:  // cnt
                append(after, qwc * 16);
                tag_at = after + qwc * 16;
                break;
            case 2:  // next
                append(after, qwc * 16);
                tag_at = addr;
                break;
            case 3:  // ref
            case 4:  // refs
                from_spr ? append_scratchpad(addr, qwc * 16) : append(addr, qwc * 16);
                tag_at = after;
                break;
            case 5:  // call
                append(after, qwc * 16);
                if (depth < 2) {
                    stack[depth++] = after + qwc * 16;
                }
                tag_at = addr;
                break;
            case 6:  // ret
                append(after, qwc * 16);
                tag_at = depth > 0 ? stack[--depth] : 0;
                break;
            default:  // end
                append(after, qwc * 16);
                tag_at = 0;
                break;
        }
    }

    if (const char* dump = std::getenv("OPENRAC_DUMP_VIF")) {
        if (std::FILE* f = std::fopen(dump, "wb")) {
            std::fwrite(vif.data(), 1, vif.size(), f);
            std::fclose(f);
        }
    }
    std::vector<std::uint8_t> gif;
    // The GS registers the strips are drawn with, as the DIRECT data before them leaves them (A+D
    // writes), and the strip being unpacked: its GIF tag at VU address 0, then ST (V2-32) at 1,
    // RGBA (V4-8) at 2 and positions (V3-32) at 3, drawn by an MSCAL (func_L00_001FDE48's packet).
    std::uint64_t reg_tex0 = 0, reg_tex1 = 0, reg_clamp = 0, reg_alpha = 0x44;
    const std::uint8_t* strip_st = nullptr;
    const std::uint8_t* strip_rgba = nullptr;
    const std::uint8_t* strip_xyz = nullptr;
    std::uint32_t strip_count = 0;
    bool strip_tag = false;
    // The images sent through the DIRECT data (BITBLTBUF, TRXREG, then IMAGE data), latest per
    // destination block: a strip takes the ones at its TEX0's blocks.
    std::uint64_t reg_bitbltbuf = 0, reg_trxreg = 0;
    std::unordered_map<std::uint32_t, renderer::ImageUpload> images;
    renderer::ImageUpload* filling = nullptr;
    std::size_t filling_bytes = 0;
    const auto start_image = [&]() {
        renderer::ImageUpload image;
        image.dbp = static_cast<std::uint32_t>((reg_bitbltbuf >> 32) & 0x3FFF);
        image.dbw = static_cast<std::uint32_t>((reg_bitbltbuf >> 48) & 0x3F);
        image.dpsm = static_cast<std::uint8_t>((reg_bitbltbuf >> 56) & 0x3F);
        image.width = static_cast<std::uint32_t>(reg_trxreg & 0xFFF);
        image.height = static_cast<std::uint32_t>((reg_trxreg >> 32) & 0xFFF);
        const std::uint32_t bits = image.dpsm == 0x00 ? 32 : image.dpsm == 0x01 ? 24 : image.dpsm == 0x02 || image.dpsm == 0x0A ? 16
                                   : image.dpsm == 0x13 ? 8 : image.dpsm == 0x14 ? 4 : 32;
        filling_bytes = static_cast<std::size_t>(image.width) * image.height * bits / 8;
        renderer::ImageUpload& slot = images[image.dbp];
        slot = std::move(image);
        filling = &slot;
    };
    std::size_t image_left = 0;  // IMAGE data a tag announced that runs on into the next DIRECT
    std::size_t direct_at = 0;   // where the DIRECT being read starts in the 2D data
    const auto track_direct = [&](const std::uint8_t* d, std::size_t size) {
        std::size_t p = 0;
        if (image_left > 0) {
            const std::size_t bytes = std::min(image_left, size);
            if (filling != nullptr && filling->data.size() < filling_bytes) {
                const std::size_t take = std::min(bytes, filling_bytes - filling->data.size());
                filling->data.insert(filling->data.end(), d, d + take);
            }
            image_left -= bytes;
            p = bytes;
        }
        while (p + 16 <= size) {
            std::uint64_t lo, hi;
            std::memcpy(&lo, d + p, 8);
            std::memcpy(&hi, d + p + 8, 8);
            p += 16;
            const std::uint32_t nloop = static_cast<std::uint32_t>(lo & 0x7FFF);
            const std::uint32_t flg = static_cast<std::uint32_t>((lo >> 58) & 3);
            const std::uint32_t nreg = static_cast<std::uint32_t>((lo >> 60) & 0xF) == 0 ? 16 : static_cast<std::uint32_t>((lo >> 60) & 0xF);
            if (flg == 0) {
                for (std::uint32_t i = 0; i < nloop * nreg && p + 16 <= size; ++i, p += 16) {
                    if (((hi >> (4 * (i % nreg))) & 0xF) != 0xE) {
                        continue;
                    }
                    std::uint64_t value, address;
                    std::memcpy(&value, d + p, 8);
                    std::memcpy(&address, d + p + 8, 8);
                    switch (address & 0xFF) {
                        case 0x50: reg_bitbltbuf = value; break;
                        case 0x52: reg_trxreg = value; break;
                        case 0x53: if ((value & 3) == 0) { start_image(); } break;
                        case 0x06: reg_tex0 = value; break;
                        case 0x08: reg_clamp = value; break;
                        case 0x14: reg_tex1 = value; break;
                        case 0x42: reg_alpha = value; break;
                        case 0x4C:
                        case 0x4D:
                            // A FRAME away from the draw buffer (page 0) and the display buffer (page
                            // 0x70): the first render to a texture.
                            if (first_offscreen != nullptr && *first_offscreen == SIZE_MAX
                                && (value & 0x1FF) != 0 && (value & 0x1FF) != 0x70) {
                                *first_offscreen = direct_at;
                            }
                            break;
                        default: break;
                    }
                }
            } else if (flg == 1) {
                p += ((static_cast<std::size_t>(nloop) * nreg + 1) / 2) * 16;
            } else {
                const std::size_t bytes = std::min(static_cast<std::size_t>(nloop) * 16, size - p);
                // Image data runs on into the next DIRECT only for a transfer the game opened (as
                // the direct renderer reads it): without one this is not GIF data, and the next
                // DIRECT is read from its first tag.
                const bool open = filling != nullptr && filling->data.size() < filling_bytes;
                image_left = open ? static_cast<std::size_t>(nloop) * 16 - bytes : 0;
                if (open) {
                    const std::size_t take = std::min(bytes, filling_bytes - filling->data.size());
                    filling->data.insert(filling->data.end(), d + p, d + p + take);
                }
                p += bytes;
            }
        }
    };
    std::size_t at = 0;
    while (at + 4 <= vif.size()) {
        std::uint32_t code;
        std::memcpy(&code, vif.data() + at, 4);
        at += 4;
        const std::uint32_t command = (code >> 24) & 0x7F;
        const std::uint32_t imm = code & 0xFFFF;
        const std::uint32_t num = (code >> 16) & 0xFF;
        if (command == 0x20) {
            at += 4;
        } else if (command == 0x30 || command == 0x31) {
            at += 16;
        } else if (command == 0x4A) {
            at += std::size_t{num == 0 ? 256u : num} * 8;
        } else if (command == 0x50 || command == 0x51) {
            const std::size_t size = std::size_t{imm == 0 ? 65536u : imm} * 16;
            if (at + size > vif.size()) {
                break;
            }
            // Each DIRECT's data stays a DIRECT of its own (NOP NOP NOP DIRECT, then the data), so
            // the direct renderer reads every one from its first tag.
            const std::uint32_t head[4] = {0, 0, 0, 0x50000000u | (imm & 0xFFFF)};
            direct_at = gif.size();
            // The world's marker (renderer::GifInterpreter::kWorldMarker): one PACKED A+D write to
            // the NOP register, put where the game's terrain pass runs.
            if (world_marker != nullptr && *world_marker == SIZE_MAX && imm == 2) {
                std::uint64_t value, address;
                std::memcpy(&value, vif.data() + at + 16, 8);
                std::memcpy(&address, vif.data() + at + 24, 8);
                if ((address & 0xFF) == 0x7F && (value >> 48) == renderer::GifInterpreter::kWorldMarker) {
                    *world_marker = direct_at;
                }
            }
            gif.insert(gif.end(), reinterpret_cast<const std::uint8_t*>(head),
                       reinterpret_cast<const std::uint8_t*>(head) + sizeof(head));
            gif.insert(gif.end(), vif.data() + at, vif.data() + at + size);
            if (strips != nullptr) {
                track_direct(vif.data() + at, size);
            }
            at += size;
        } else if (command >= 0x60) {
            const std::uint32_t vn = (command >> 2) & 3;
            const std::uint32_t vl = command & 3;
            const std::uint32_t bits = vl == 0 ? 32 : vl == 1 ? 16 : vl == 2 ? 8 : 16;
            const std::uint32_t count = num == 0 ? 256 : num;
            const std::uint32_t components = vl == 3 ? 1 : vn + 1;
            const std::size_t size = ((count * components * bits + 31) / 32) * 4;
            if (strips != nullptr && at + size <= vif.size()) {
                const std::uint32_t address = imm & 0x3FF;
                const std::uint8_t* data = vif.data() + at;
                if (vn == 3 && vl == 0 && count == 1 && address == 0) {
                    // A strip's GIF tag: a triangle strip (PRIM 4) of NLOOP vertices.
                    std::uint64_t tag;
                    std::memcpy(&tag, data, 8);
                    strip_tag = ((tag >> 47) & 7) == 4 && (tag & 0x7FFF) >= 3;
                    strip_count = static_cast<std::uint32_t>(tag & 0x7FFF);
                    strip_st = strip_rgba = strip_xyz = nullptr;
                } else if (strip_tag && address == 1 && vn == 1 && vl == 0 && count == strip_count) {
                    strip_st = data;
                } else if (strip_tag && address == 2 && vn == 3 && vl == 2 && count == strip_count) {
                    strip_rgba = data;
                } else if (strip_tag && address == 3 && vn == 2 && vl == 0 && count == strip_count) {
                    strip_xyz = data;
                }
            }
            at += size;
        } else if ((command == 0x14 || command == 0x15 || command == 0x17) && before_world != nullptr
                   && *before_world == SIZE_MAX) {
            // The first VU1 program the frame starts: the 2D data so far comes before the world.
            *before_world = gif.size();
        }
        if ((command == 0x14 || command == 0x15 || command == 0x17) && strips != nullptr) {
            if (strip_tag && strip_st != nullptr && strip_rgba != nullptr && strip_xyz != nullptr) {
                renderer::EffectStrip strip;
                strip.first = static_cast<std::uint32_t>(strip_vertices->size());
                strip.count = strip_count;
                strip.tex0 = reg_tex0;
                strip.tex1 = reg_tex1;
                strip.clamp = reg_clamp;
                strip.alpha = reg_alpha;
                if (uploads != nullptr) {
                    const std::uint32_t tbp = static_cast<std::uint32_t>(reg_tex0 & 0x3FFF);
                    const std::uint32_t cbp = static_cast<std::uint32_t>((reg_tex0 >> 37) & 0x3FFF);
                    for (const std::uint32_t block : {cbp, tbp}) {
                        auto found = images.find(block);
                        if (found != images.end() && found->second.data.size() > 0) {
                            if (strip.uploads < 0) {
                                strip.uploads = static_cast<int>(uploads->size());
                            }
                            uploads->push_back(found->second);
                            strip.upload_count++;
                        }
                    }
                }
                for (std::uint32_t k = 0; k < strip_count; ++k) {
                    renderer::EffectVertex v;
                    std::memcpy(&v.x, strip_xyz + k * 12, 12);
                    std::memcpy(&v.s, strip_st + k * 8, 8);
                    std::memcpy(&v.rgba, strip_rgba + k * 4, 4);
                    strip_vertices->push_back(v);
                }
                strips->push_back(strip);
            }
            strip_tag = false;
        }
    }
    return gif;
}

// The game's frame as the TV showed it: 4:3 (the PS2's 512x448 PAL / 512x416 NTSC buffer), the
// largest such box in the window, centred; the rest of the window is black. As ReRAC's game frame
// (crates/rc-engine/src/display.rs; ISC License, Copyright (c) 2026 ReRAC contributors): the frame
// keeps the game's framing whatever the window's shape. `aspect`: a wider picture (the player's
// setting, settings.h), which shows more of the world to the sides.
struct Box {
    int x, y, width, height;
};

constexpr float kTvAspect = 4.0f / 3.0f;

Box frame_box(int window_width, int window_height, float aspect = kTvAspect) {
    int width = window_width;
    int height = aspect == kTvAspect ? width * 3 / 4 : static_cast<int>(std::lround(static_cast<float>(width) / aspect));
    if (height > window_height) {
        height = window_height;
        width = aspect == kTvAspect
                    ? height * 4 / 3
                    : std::min(window_width, static_cast<int>(std::lround(static_cast<float>(height) * aspect)));
    }
    return {(window_width - width) / 2, (window_height - height) / 2, width, height};
}

// The picture's shape now: the player's setting, for the window as it is.
float picture_aspect(int window_width, int window_height) {
    return settings::aspect_ratio(settings::get(), window_width, window_height);
}

// The development switches that keep one frame a tick whatever the settings say.
bool one_frame_a_tick() {
    static const bool on = std::getenv("OPENRAC_UNCAPPED") != nullptr || std::getenv("OPENRAC_NO_BLEND") != nullptr;
    return on;
}

// Whether a swap waits for a refresh: the player's vsync setting, when frames are drawn between
// ticks. With one frame a tick (the "Original" frame rate; OPENRAC_UNCAPPED, OPENRAC_NO_BLEND)
// nothing waits for the display: the tick paces the frames, and 50 of them a second each held for
// a refresh of a 60 or 120 Hz display would hold the tick up with them.
platform::SwapInterval wanted_swap() {
    const settings::Settings& s = settings::get();
    if (one_frame_a_tick() || s.frame_rate == settings::FrameRate::Original || s.vsync == settings::VSync::Off) {
        return platform::SwapInterval::Immediate;
    }
    return s.vsync == settings::VSync::Adaptive ? platform::SwapInterval::Adaptive : platform::SwapInterval::VSync;
}

platform::DisplayMode window_mode(settings::DisplayMode mode) {
    switch (mode) {
        case settings::DisplayMode::Borderless:
            return platform::DisplayMode::Borderless;
        case settings::DisplayMode::Fullscreen:
            return platform::DisplayMode::Fullscreen;
        default:
            return platform::DisplayMode::Windowed;
    }
}

// The size the frame is drawn at for a picture of box_width x box_height window pixels: the
// player's render scale of it, held to what a texture can be on any card the port runs on.
void frame_size(int box_width, int box_height, int& width, int& height) {
    constexpr int kLargest = 8192;
    const float scale = static_cast<float>(std::clamp(settings::get().render_scale, 25, 400)) / 100.0f;
    const float fit = std::min(1.0f, static_cast<float>(kLargest)
                                         / (scale * static_cast<float>(std::max({box_width, box_height, 1}))));
    width = std::max(1, static_cast<int>(std::lround(static_cast<float>(box_width) * scale * fit)));
    height = std::max(1, static_cast<int>(std::lround(static_cast<float>(box_height) * scale * fit)));
}

// Draws `framebuffer` (width x height) into the window's box, black around it; `flip`: its first
// row is the top (an uploaded picture) rather than the bottom (a rendered frame).
void present(unsigned framebuffer, int width, int height, int window_width, int window_height, bool flip,
             float aspect = kTvAspect) {
    using namespace gl;
    const Box b = frame_box(window_width, window_height, aspect);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    const int y0 = flip ? b.y + b.height : b.y;
    const int y1 = flip ? b.y : b.y + b.height;
    glBlitFramebuffer(0, 0, width, height, b.x, y0, b.x + b.width, y1, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

// The inverse of a view matrix (a rotation and a translation, column-major).
renderer::Mat4 rigid_inverse(const renderer::Mat4& m) {
    renderer::Mat4 r{};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            r[4 * j + i] = m[4 * i + j];
        }
    }
    for (int i = 0; i < 3; ++i) {
        r[12 + i] = -(r[i] * m[12] + r[4 + i] * m[13] + r[8 + i] * m[14]);
    }
    r[15] = 1.0f;
    return r;
}

// The extracted level the title world is written as (editor/level.py load_title).
constexpr int kTitleWorld = 99;
// The flight between planets' sky, written as a level of its own (editor/level.py load_flight).
constexpr int kFlightWorld = 98;
// Where the sky's textures are placed in the texture pool, past the GS's 14-bit blocks.
constexpr std::uint32_t kSkyTextureKey = 0x20000;
// The game mode word, as the game's front end names it (rac1: port/game/rac1/host/front_end.c); 6
// in the boot program is the flight between planets. 0 for a game without one.
std::uint32_t game_mode_address() {
    return openrac_game_front_end != nullptr ? openrac_game_front_end->game_mode : 0;
}
// The directional light bank, in the executable's terms (relocated to the loaded program's copy).
gaddr light_bank_address() {
    return openrac_game_front_end != nullptr ? openrac_game_front_end->light_bank : 0;
}

// Uploads a level's geometry; while the boot program runs (the title and the main menu), the title
// world's, or level 0's when the title world was not extracted.
void use_level(int number) {
    int wanted = number;
    if (number == -2 && std::filesystem::exists(g->levels / std::format("level_{:02d}", kFlightWorld))) {
        wanted = kFlightWorld;
    } else if (number < 0) {
        wanted = std::filesystem::exists(g->levels / std::format("level_{:02d}", kTitleWorld)) ? kTitleWorld : 0;
    }
    if (wanted == g->loaded) {
        return;
    }
    g->loaded = wanted;
    // One renderer for the whole run: a new level replaces the subsystem renderers, but its texture
    // pool is the GS memory, which keeps what the game uploaded (a card's text uploaded through the
    // display list, the fonts) across the change.
    const bool fresh = !g->renderer;
    if (fresh) {
        g->renderer = std::make_unique<renderer::Renderer>();
        for (const renderer::ImageUpload& image : g->images) {
            g->renderer->textures().upload(image);
        }
    } else {
        g->renderer->clear_renderers();
    }
    g->scene.release();
    g->scene = viewer::LevelScene();
    g->level = viewer::LevelData();
    std::string error;
    const auto dir = g->levels / std::format("level_{:02d}", wanted);
    // An edited level's scenery (--mods): the editor's pack writes it beside its gameplay file.
    std::filesystem::path edited;
    if (openrac_game_mods_dir != nullptr) {
        edited = std::filesystem::path(openrac_game_mods_dir) / std::format("level_{:02d}", wanted)
                 / "placements.json";
    }
    debug_overlay::notice({});
    if (!viewer::load_level(dir, g->level, error, edited)) {
        log::warn("level {}: {} (extract it with editor/extract.py port)", wanted, error);
        // Said in the picture: the game goes on, its HUD over a black world, and a player takes
        // that for the game being broken.
        debug_overlay::notice("This level's scenery could not be read, so the world is black.\n"
                              "Close the game and run \"Check the levels\" in the launcher (the log says which file).");
        g->renderer->add(std::make_unique<renderer::DirectRenderer>("hud", renderer::Bucket::Hud, renderer::DirectConfig{}, renderer::DirectRenderer::Input::Vif), error);
        if (fresh) {
            g->renderer->init(error);
        }
        return;
    }
    if (!g->scene.upload(g->level, g->renderer->textures(), error)) {
        log::error("level {}: {}", wanted, error);
        return;
    }
    // The 2D path: its part before the world first, the rest after the world renderers.
    auto hud = std::make_unique<renderer::DirectRenderer>("hud", renderer::Bucket::Hud, renderer::DirectConfig{},
                                                          renderer::DirectRenderer::Input::Vif);
    g->renderer->add(std::make_unique<renderer::DirectBackground>(*hud), error);
    for (std::size_t layer = 0; layer < viewer::kLayerCount; ++layer) {
        g->renderer->add(
            std::make_unique<viewer::LayerRenderer>(g->scene, static_cast<viewer::Layer>(layer)),
            error
        );
    }
    if (g->renderer->add(std::make_unique<renderer::EffectRenderer>(), error) == nullptr) {
        log::error("effect renderer: {}", error);
    }
    g->renderer->add(std::move(hud), error);
    if (fresh) {
        g->renderer->init(error);
    }
    // The sky's textures at pool keys of their own (kSkyTextureKey + number), for its sprites.
    const auto sky = viewer::load_sky_textures(dir);
    for (std::size_t i = 0; i < sky.size(); ++i) {
        if (sky[i].width > 0) {
            const renderer::TextureHandle h = g->renderer->textures().add(sky[i], renderer::AlphaScale::Full,
                                                                          std::format("sky {} of level {}", i, wanted));
            g->renderer->textures().place(kSkyTextureKey + static_cast<std::uint32_t>(i), h);
        }
    }
    log::info("drawing level {} natively ({} models)", wanted, g->level.models.size());
}

}  // namespace

void effect_quad(std::span<const std::uint8_t> ram, const openrac_game_quad& quad) {
    renderer::EffectQuad q;
    std::memcpy(q.corner, quad.corner, sizeof(q.corner));
    std::memcpy(q.rgba, quad.rgba, sizeof(q.rgba));
    std::memcpy(q.st, quad.st, sizeof(q.st));
    q.clamp = quad.clamp;
    q.tex0 = quad.tex0;
    q.tex1 = quad.tex1;
    q.alpha = quad.alpha;
    const std::uint32_t pixels = quad.pixels & 0x01FFFFFF;
    const std::uint32_t clut = quad.clut & 0x01FFFFFF;
    const std::uint32_t tbp = static_cast<std::uint32_t>(quad.tex0 & 0x3FFF);
    const std::uint32_t cbp = static_cast<std::uint32_t>((quad.tex0 >> 37) & 0x3FFF);
    q.tbp = tbp;
    q.cbp = cbp;
    const std::uint32_t width = 1u << ((quad.tex0 >> 26) & 0xF);
    const std::uint32_t height = 1u << ((quad.tex0 >> 30) & 0xF);
    if (pixels != 0 && clut != 0 && pixels + width * height <= ram.size() && clut + 1024 <= ram.size()) {
        renderer::ImageUpload palette;
        palette.dbp = cbp;
        palette.dbw = 1;
        palette.dpsm = 0x00;  // PSMCT32
        palette.width = 16;
        palette.height = 16;
        palette.data.assign(ram.data() + clut, ram.data() + clut + 1024);
        renderer::ImageUpload image;
        image.dbp = tbp;
        image.dbw = std::max<std::uint32_t>(1, width / 64);
        image.dpsm = 0x13;  // PSMT8
        image.width = width;
        image.height = height;
        image.data.assign(ram.data() + pixels, ram.data() + pixels + width * height);
        q.uploads = static_cast<int>(g->effect_uploads.size());
        g->effect_uploads.push_back(std::move(palette));
        g->effect_uploads.push_back(std::move(image));
        g->effects.push_back(q);
    } else {
        // No paging record: a texture the game put in the GS itself (the flight's planet picture,
        // loaded with the transition's data), resolved from the pool at TEX0's blocks; the
        // renderer leaves the quad out when nothing is there.
        g->effects.push_back(q);
    }
}

void sky_shell(int shell, const float* matrix) {
    if (!g || shell < 0 || shell >= static_cast<int>(g->sky_shells.size())) {
        return;
    }
    std::memcpy(g->sky_shells[static_cast<std::size_t>(shell)].data(), matrix, sizeof(float) * 16);
    g->sky_shell_mask |= 1u << shell;
}

int hud_side() {
    if (!g || !g->window || settings::get().hud != settings::Hud::Edges) {
        return 0;
    }
    int window_width = 0;
    int window_height = 0;
    g->window->drawable_size(window_width, window_height);
    const Box box = frame_box(window_width, window_height, picture_aspect(window_width, window_height));
    if (box.height <= 0) {
        return 0;
    }
    // The picture is box.width / box.height wide where the game's is 4:3: each side gained half
    // the difference, in pixels of which the game's picture has 512.
    const float wider = static_cast<float>(box.width) / (kTvAspect * static_cast<float>(box.height));
    return wider > 1.0f ? static_cast<int>(256.0f * (wider - 1.0f)) : 0;
}

void sky_sprite(const std::uint8_t* record, std::uint64_t tex0) {
    std::array<std::uint8_t, 0x20> r;
    std::memcpy(r.data(), record, r.size());
    g->sky_sprites.emplace_back(r, tex0);
}

// The sky sprites as effect quads, as SkySpriteProc draws them with the particle program (ReRAC's
// sky_stars.rs; ISC License, Copyright (c) 2026 ReRAC contributors): around the eye, facing it,
// the half-diagonal size * 832 / depth frame-buffer pixels, turned by the rotation, additive, behind
// everything. Drawn far out along their direction (kSkyDistance, inside the far plane) and scaled to
// match, so the world's depth hides them where it is in front of the sky.
void sky_sprite_quads(const viewer::GameState& state) {
    constexpr float kSkyDistance = 1900.0f;
    const std::array<float, 3> cam = state.camera_position;
    const std::array<float, 3> fwd = state.forward;
    const std::array<float, 3> right{-state.left[0], -state.left[1], -state.left[2]};
    const std::array<float, 3> down{-state.up[0], -state.up[1], -state.up[2]};
    const float tan_y = state.tan_half_fov_y > 0.0f ? state.tan_half_fov_y : 0.476f;
    for (const auto& [r, tex0] : g->sky_sprites) {
        float pos[3];
        float theta;
        float size;
        std::uint32_t rgba;
        std::memcpy(pos, r.data() + 0x10, 12);
        std::memcpy(&theta, r.data() + 8, 4);
        std::memcpy(&size, r.data() + 0x1C, 4);
        std::memcpy(&rgba, r.data() + 4, 4);
        const float len = std::sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
        const float z = pos[0] * fwd[0] + pos[1] * fwd[1] + pos[2] * fwd[2];
        if (!(len > 0.0f) || !(z > 0.0f) || !std::isfinite(size)) {
            continue;
        }
        const float k = kSkyDistance / len;
        const float h = 4.0f * size * tan_y * k;
        const float c = std::cos(theta);
        const float s = std::sin(theta);
        renderer::EffectQuad q{};
        float centre[3];
        float a[3];
        float b[3];
        for (int i = 0; i < 3; ++i) {
            centre[i] = cam[i] + pos[i] * k;
            a[i] = (c * right[i] + 1.0625f * s * down[i]) * h;
            b[i] = (-s * right[i] + 1.0625f * c * down[i]) * h;
        }
        for (int i = 0; i < 3; ++i) {
            q.corner[0][i] = centre[i] + a[i];
            q.corner[1][i] = centre[i] + b[i];
            q.corner[2][i] = centre[i] - b[i];
            q.corner[3][i] = centre[i] - a[i];
        }
        for (std::uint32_t& col : q.rgba) {
            col = rgba;
        }
        const float st[4][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};
        std::memcpy(q.st, st, sizeof(st));
        (void)tex0;
        q.tex0 = 1ull << 34;  // TCC, MODULATE; the texture is the sky's own (kSkyTextureKey)
        q.tbp = kSkyTextureKey + r[2];
        q.cbp = 0;
        q.clamp = 5;
        q.tex1 = 0xFF9000000120ull;
        q.alpha = r[3];
        g->effects.insert(g->effects.begin(), q);
    }
    g->sky_sprites.clear();
}

void particle(std::span<const std::uint8_t> ram, const std::uint8_t* record, std::uint32_t pixels,
              std::uint32_t clut, int log2_side) {
    (void)ram;
    State::Particle p;
    std::memcpy(p.record.data(), record, p.record.size());
    p.pixels = pixels & 0x01FFFFFF;
    p.clut = clut & 0x01FFFFFF;
    p.log2_side = log2_side;
    g->particles.push_back(p);
}

// A particle texture's pool key: its 32 x 32 PSMT8 pixels and its 16 x 16 CSM1 palette (both in game
// memory) decoded to RGBA once, in the chip's alpha units (0x80 = opaque), and placed at a key of its
// own, kept while the renderer lives. Earlier the pixels and palette went through the pool as
// uploads at keys handed out per frame in pool order: a key named one texture this frame and
// another the next, and the conversions the pool kept mixed them up (the nanotech crates' orbs as
// white squares). 0 when the addresses are not in memory.
// Refactor note: other effect textures (effect_quad) still go through uploads at their GS blocks.
std::uint32_t particle_texture(std::span<const std::uint8_t> ram, std::uint32_t pixels, std::uint32_t clut) {
    constexpr std::uint32_t kParticleTextureKey = 0x30000;  // past the sky's (kSkyTextureKey)
    constexpr std::size_t kSide = 32;
    if (!g->renderer || pixels == 0 || clut == 0 || pixels + kSide * kSide > ram.size() || clut + 1024 > ram.size()) {
        return 0;
    }
    if (g->particle_textures_for != g->renderer.get()) {  // a new renderer (a level loaded): a new pool
        g->particle_textures.clear();
        g->particle_textures_for = g->renderer.get();
    }
    const std::span<const std::uint8_t> indices = ram.subspan(pixels, kSide * kSide);
    const std::span<const std::uint8_t> colours = ram.subspan(clut, 1024);
    const std::uint64_t id = renderer::hash_bytes(indices.data(), indices.size())
                             ^ (renderer::hash_bytes(colours.data(), colours.size()) * 0x9E3779B97F4A7C15ull);
    if (const auto found = g->particle_textures.find(id); found != g->particle_textures.end()) {
        return found->second;
    }
    const renderer::gs::Texa texa{0, false, 0x80};
    const std::vector<std::uint32_t> palette = renderer::clut_csm1(colours, 0x00, 16, 256, texa);
    renderer::Rgba8Image image = renderer::convert_t8(indices, kSide, kSide, palette, renderer::AlphaScale::Gs);
    const renderer::TextureHandle h = g->renderer->textures().add(
        std::move(image), renderer::AlphaScale::Gs, std::format("particle {:#x}/{:#x}", pixels, clut));
    const std::uint32_t key = kParticleTextureKey + static_cast<std::uint32_t>(g->particle_textures.size());
    g->renderer->textures().place(key, h);
    g->particle_textures.emplace(id, key);
    return key;
}

// The frame's particles as effect quads, as the game's VU1 sprite program draws them (ReRAC's
// particle notes, docs/plan/particles.md section 7; ISC License, Copyright (c) 2026 ReRAC
// contributors): culled and faded by depth (the record's near and far, at most 500 units), back to
// front; a sprite faces the camera, its half-diagonal size / 210000 units (y scaled by 1.0625 as the
// game does on screen), turned by its rotation byte; a flat quad lies in the world's XY plane; a
// ribbon joins its two ends across the view. Lines are not drawn yet.
void particle_quads(std::span<const std::uint8_t> ram, const viewer::GameState& state) {
    const auto f32 = [](const std::uint8_t* b) {
        float v;
        std::memcpy(&v, b, 4);
        return v;
    };
    const auto u32 = [](const std::uint8_t* b) {
        std::uint32_t v;
        std::memcpy(&v, b, 4);
        return v;
    };
    const std::array<float, 3> cam = state.camera_position;
    const std::array<float, 3> fwd = state.forward;
    const std::array<float, 3> right{-state.left[0], -state.left[1], -state.left[2]};
    const std::array<float, 3> down{-state.up[0], -state.up[1], -state.up[2]};
    struct Item {
        float depth;
        renderer::EffectQuad quad;
    };
    std::vector<Item> items;
    for (const State::Particle& p : g->particles) {
        const std::uint8_t* r = p.record.data();
        const int kind = r[1] & 3;
        if (kind == 2 || p.pixels == 0 || p.clut == 0) {
            continue;
        }
        std::array<float, 3> p1{f32(r + 0x10), f32(r + 0x14), f32(r + 0x18)};
        std::array<float, 3> p2{f32(r + 0x20), f32(r + 0x24), f32(r + 0x28)};
        std::array<float, 3> centre = p1;
        if (kind == 3) {
            for (int i = 0; i < 3; ++i) {
                centre[i] = (p1[i] + p2[i]) * 0.5f;
            }
        }
        const float z = (centre[0] - cam[0]) * fwd[0] + (centre[1] - cam[1]) * fwd[1] + (centre[2] - cam[2]) * fwd[2];
        const int z12 = static_cast<int>(z * 4096.0f);
        const int near12 = (r[9] & 0xF) << 10;
        const int far12 = std::min((r[9] >> 4) << 17, 0x1F4000);
        if (z12 <= near12 || z12 >= far12) {
            continue;
        }
        const int fade = std::min({(far12 - z12) >> 4, z12 - near12, 0x1000});
        const auto faded = [fade](std::uint32_t c) {
            return (c & 0x00FFFFFFu) | (((c >> 24) * static_cast<std::uint32_t>(fade) >> 12) << 24);
        };
        renderer::EffectQuad q{};
        const std::uint32_t c1 = faded(u32(r + 4));
        for (std::uint32_t& c : q.rgba) {
            c = c1;
        }
        const float st[4][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};
        std::memcpy(q.st, st, sizeof(st));
        const float theta = static_cast<float>(r[8]) * 6.2831853f / 256.0f;
        const float c = std::cos(theta);
        const float s = std::sin(theta);
        if (kind == 0) {
            const float h = f32(r + 0xC) / 210000.0f * (z / (z + 0.5f));
            float a[3];
            float b[3];
            for (int i = 0; i < 3; ++i) {
                a[i] = (c * right[i] + 1.0625f * s * down[i]) * h;
                b[i] = (-s * right[i] + 1.0625f * c * down[i]) * h;
            }
            for (int i = 0; i < 3; ++i) {
                q.corner[0][i] = p1[i] + a[i];
                q.corner[1][i] = p1[i] + b[i];
                q.corner[2][i] = p1[i] - b[i];
                q.corner[3][i] = p1[i] - a[i];
            }
        } else if (kind == 1) {
            const float h = f32(r + 0xC) / 420000.0f;
            const float d[4][2] = {{c, s}, {-s, c}, {s, -c}, {-c, -s}};
            for (int k = 0; k < 4; ++k) {
                q.corner[k][0] = p1[0] + d[k][0] * h;
                q.corner[k][1] = p1[1] + d[k][1] * h;
                q.corner[k][2] = p1[2];
            }
        } else {
            const float w1 = f32(r + 0x1C);
            const float k2 = f32(r + 0x2C);
            const float e[3] = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
            const float m[3] = {centre[0] - cam[0], centre[1] - cam[1], centre[2] - cam[2]};
            float n[3] = {e[1] * m[2] - e[2] * m[1], e[2] * m[0] - e[0] * m[2], e[0] * m[1] - e[1] * m[0]};
            const float len = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
            if (len <= 0.0f) {
                continue;
            }
            for (float& v : n) {
                v /= len;
            }
            for (int i = 0; i < 3; ++i) {
                q.corner[0][i] = p1[i] + n[i] * w1;
                q.corner[1][i] = p1[i] - n[i] * w1;
                q.corner[2][i] = p2[i] + n[i] * w1 * k2;
                q.corner[3][i] = p2[i] - n[i] * w1 * k2;
            }
            const std::uint32_t c2 = faded(u32(r + 0xC));
            q.rgba[2] = c2;
            q.rgba[3] = c2;
        }
        // Its texture: 32 x 32 PSMT8 with a 256-colour CSM1 palette, decoded here and kept in the pool
        // at a key of its own (particle_texture).
        q.tbp = particle_texture(ram, p.pixels, p.clut);
        q.cbp = 0;
        if (q.tbp == 0) {
            continue;
        }
        q.tex0 = 1ull << 34;  // TCC, MODULATE; the texture is the one placed at q.tbp
        q.clamp = 5;
        q.tex1 = 0xFF9000000120ull;
        q.alpha = r[3];
        items.push_back({z, q});
    }
    std::stable_sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return a.depth > b.depth; });
    for (const Item& it : items) {
        g->effects.push_back(it.quad);
    }
    g->particles.clear();
    g->sky_sprites.clear();
}

// OpenRAC's icon on the window (the taskbar's, and the title bar's where there is one): the
// launcher's picture, embedded by cmake/Games.cmake.
void set_window_icon(platform::Window& window) {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* rgba = stbi_load_from_memory(openrac_icon_png, static_cast<int>(openrac_icon_png_size), &width,
                                                &height, &channels, 4);
    if (rgba == nullptr) {
        return;
    }
    window.set_icon(rgba, width, height);
    stbi_image_free(rgba);
}

#if defined(__linux__) || defined(__FreeBSD__)
static bool write_file_if_changed(const std::filesystem::path& path, const void* data, std::size_t size) {
    std::error_code ec;
    if (std::filesystem::exists(path, ec) && std::filesystem::file_size(path, ec) == size) {
        std::ifstream current(path, std::ios::binary);
        if (current) {
            std::vector<char> buffer(size);
            if (current.read(buffer.data(), static_cast<std::streamsize>(size))) {
                if (std::memcmp(buffer.data(), data, size) == 0) {
                    return false;
                }
            }
        }
    }
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        return false;
    }
    out.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    return out.good();
}

static void ensure_linux_desktop_integration(const std::string& game_id) {
    const char* home = std::getenv("HOME");
    if (home == nullptr || home[0] == '\0') {
        return;
    }
    const std::filesystem::path home_path = home;
    const std::filesystem::path icons_base = home_path / ".local/share/icons/hicolor";
    const std::filesystem::path app_dir = home_path / ".local/share/applications";

    bool icons_changed = false;
    bool entries_changed = false;

    // The game's icon for the desktop and taskbar (KDE Plasma, GNOME, etc.).
    // Wayland compositors identify windows by their app_id (dev.openrac.<game_id>)
    // and match them against .desktop files in ~/.local/share/applications/ and
    // icons in ~/.local/share/icons/hicolor/.
    const std::filesystem::path icon_dir = icons_base / "512x512/apps";
    const std::string app_id = "dev.openrac." + game_id;
    const std::string prog_id = "openrac-" + game_id;

    if (openrac_icon_png_size > 0) {
        icons_changed |= write_file_if_changed(icon_dir / (app_id + ".png"), openrac_icon_png, openrac_icon_png_size);
        icons_changed |= write_file_if_changed(icon_dir / (prog_id + ".png"), openrac_icon_png, openrac_icon_png_size);
    }

    // Resolve current executable path for desktop entry.
    std::error_code ec;
    std::filesystem::path exe_path = std::filesystem::canonical("/proc/self/exe", ec);
    if (ec || exe_path.empty()) {
        exe_path = prog_id;
    }

    const auto make_entry = [&](const std::string& wm_class, bool hidden) {
        std::string entry = "[Desktop Entry]\n";
        entry += "Type=Application\n";
        entry += "Name=";
        entry += OPENRAC_GAME_TITLE;
        entry += "\nComment=Play ";
        entry += OPENRAC_GAME_TITLE;
        entry += " with OpenRAC\n";
        entry += "Exec=\"";
        entry += exe_path.string();
        entry += "\"\n";
        entry += "Icon=" + app_id + "\n";
        entry += "Terminal=false\n";
        entry += "Categories=Game;\n";
        entry += "StartupNotify=true\n";
        entry += "StartupWMClass=" + wm_class + "\n";
        if (hidden) {
            entry += "NoDisplay=true\n";
        }
        return entry;
    };

    const std::string dev_entry = make_entry(app_id, false);
    const std::string prog_entry = make_entry(prog_id, true);

    entries_changed |= write_file_if_changed(app_dir / (app_id + ".desktop"), dev_entry.data(), dev_entry.size());
    entries_changed |= write_file_if_changed(app_dir / (prog_id + ".desktop"), prog_entry.data(), prog_entry.size());

    // Refresh system caches if files were added or modified
    if (icons_changed) {
        if (std::system(nullptr)) {
            std::string cmd = "gtk-update-icon-cache -f -t \"" + icons_base.string() + "\" >/dev/null 2>&1 &";
            (void)std::system(cmd.c_str());
        }
    }
    if (entries_changed || icons_changed) {
        if (std::system(nullptr)) {
            std::string cmd1 = "update-desktop-database \"" + app_dir.string() + "\" >/dev/null 2>&1 &";
            (void)std::system(cmd1.c_str());
            std::string cmd2 = "kbuildsycoca6 >/dev/null 2>&1 &";
            (void)std::system(cmd2.c_str());
        }
    }
}
#endif

bool open(const std::string& game_id, const std::filesystem::path& levels, std::string& error) {
    g = std::make_unique<State>();
    g->game = game_id;
    g->levels = levels;
    // Before SDL starts: on Wayland the app id names the program to the desktop, and a machine
    // with two graphics cards is asked for the fast one.
    const std::string app_id = "dev.openrac." + game_id;
#if defined(__linux__) || defined(__FreeBSD__)
    ensure_linux_desktop_integration(game_id);
#endif
    platform::set_app_metadata(OPENRAC_GAME_TITLE, app_id.c_str());
    platform::prefer_discrete_gpu();
    platform::prepare_gpu_driver();
#if defined(__ANDROID__)
    // Back opens and shuts the options drawer instead of leaving the game (debug_overlay.h), from
    // the first moment the window is there.
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
#endif
    g->sdl = std::make_unique<platform::Platform>(platform::kVideo | platform::kGamepad | platform::kAudio);
    if (!g->sdl->ok()) {
        error = g->sdl->error();
        return false;
    }
    platform::WindowConfig config;
    config.title = OPENRAC_GAME_TITLE;
    config.width = 1280;
    config.height = 960;
    // The game ticks 60 times a second (main.cpp, every version) and the window draws at the
    // display's, the frames between ticks moving the scene on (between), each shown at a refresh
    // when the player's vsync is on (wanted_swap).
    config.vsync = false;  // set below, as asked
    const settings::Settings& chosen = settings::get();
    if (chosen.window_width >= 320 && chosen.window_height >= 240) {
        // The window as the player left it.
        config.width = chosen.window_width;
        config.height = chosen.window_height;
    } else if (chosen.aspect != settings::Aspect::Standard && chosen.aspect != settings::Aspect::Window) {
        // A window of the picture's shape.
        config.width = static_cast<int>(std::lround(static_cast<float>(config.height) * picture_aspect(0, 0)));
    }
    config.mode = window_mode(chosen.display_mode);
    config.display = chosen.monitor;
    config.fullscreen_width = chosen.fullscreen_width;
    config.fullscreen_height = chosen.fullscreen_height;
    // The game is drawn by a graphics card or not at all: a software rasteriser shows a level at a
    // few frames a second, which looks like a broken port (OPENRAC_SOFTWARE_GL=1: run it anyway).
    config.software = std::getenv("OPENRAC_SOFTWARE_GL") != nullptr ? platform::SoftwareRenderer::Allow
                                                                    : platform::SoftwareRenderer::Refuse;
    // OPENRAC_HIDDEN=1: the window is made and drawn as always but never shown, for scripted runs
    // that should not open windows on the player's screen (several can run at once).
    config.hidden = std::getenv("OPENRAC_HIDDEN") != nullptr;
    g->window = platform::Window::open(config, error);
    if (!g->window) {
        return false;
    }
    {
        const char* developer = std::getenv("OPENRAC_DEVELOPER");
        platform::install_app_menus(developer != nullptr && developer[0] != '\0' && developer[0] != '0');
    }
    set_window_icon(*g->window);
    g->swap_asked = wanted_swap();
    g->swap = g->window->set_swap_interval(g->swap_asked);
    g->display_mode = chosen.display_mode;
    g->monitor = chosen.monitor;
    g->fullscreen_width = chosen.fullscreen_width;
    g->fullscreen_height = chosen.fullscreen_height;
    g->aspect = chosen.aspect;
    g->aspect_custom_x = chosen.aspect_custom_x;
    g->aspect_custom_y = chosen.aspect_custom_y;
    std::string missing;
    if (!gl::load(platform::Window::gl_loader(), missing)) {
        error = "OpenGL functions missing: " + missing;
        return false;
    }
    // [debug tools] The renderer's GL calls counted from here on (renderer/gl_stats.h).
    renderer::gl_stats::install(platform::Window::gl_loader());
    log::info("graphics: up to {} samples a pixel, anisotropy up to {:.0f}; the swap {}", 
              renderer::FrameBuffer::max_samples(), renderer::TexturePool::max_anisotropy(),
              g->swap == platform::SwapInterval::Immediate ? "does not wait"
              : g->swap == platform::SwapInterval::Adaptive ? "waits for the display (adaptive)"
                                                            : "waits for the display");
    return true;
}

void upload_image(
    std::uint32_t base,
    std::uint32_t width_units,
    std::uint8_t psm,
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t width,
    std::uint32_t height,
    std::span<const std::uint8_t> pixels
) {
    if (!g) {
        return;
    }
    log::debug(
        "library image to block {:#x} width {} psm {:#x} at {},{} size {}x{}", base, width_units, psm, x,
        y, width, height
    );
    renderer::ImageUpload image;
    image.dbp = base;
    image.dbw = width_units;
    image.dpsm = psm;
    image.x = x;
    image.y = y;
    image.width = width;
    image.height = height;
    image.data.assign(pixels.begin(), pixels.end());
    // A later image to the same rectangle replaces the earlier one.
    std::erase_if(g->images, [&](const renderer::ImageUpload& old) {
        return old.dbp == base && old.dpsm == psm && old.x == x && old.y == y && old.width == width
               && old.height == height;
    });
    if (g->renderer) {
        g->renderer->textures().upload(image);
    }
    g->images.push_back(std::move(image));
}

bool store_image(std::uint32_t base, std::uint32_t width_units, std::uint8_t psm, int x, int y,
                 int width, int height, std::span<std::uint8_t> out) {
    int fx = 0;
    int fy = 0;
    // The draw buffer as the game left it: the last frame with its world in.
    const renderer::FrameBuffer* source = g && g->last_world.id() != 0 ? &g->last_world : g ? &g->frame : nullptr;
    if (source == nullptr || psm != 0 || source->id() == 0 || source->width() <= 0
        || !renderer::frame_pixel(base, width_units, x, y, fx, fy)) {
        return false;
    }
    // The last frame drawn, read once per frame however many strips the game reads: kept at full
    // resolution for drawing it back, and scaled to the chip's 512 x (the draw buffer's lines) for the answer.
    static std::uint64_t read_at = ~0ull;
    static std::vector<std::uint8_t> chip;  // 512 x the draw buffer's lines, RGBA, top row first
    constexpr int kChipWidth = 512;
    const int lines = renderer::g_draw_buffer_lines;
    if (read_at != g->index) {
        using namespace openrac::gl;
        read_at = g->index;
        renderer::FrameSnapshot& snap = renderer::g_frame_snapshot;
        snap.answered.clear();
        if (snap.texture == 0) {
            glGenTextures(1, &snap.texture);
        }
        glBindTexture(GL_TEXTURE_2D, snap.texture);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, source->resolved());
        static int snap_w = 0;
        static int snap_h = 0;
        if (snap_w != source->width() || snap_h != source->height()) {
            snap_w = source->width();
            snap_h = source->height();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, snap_w, snap_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, snap_w, snap_h);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        const std::vector<std::uint8_t> full = source->read_rgba();
        const int fw = source->width();
        const int fh = source->height();
        chip.assign(static_cast<std::size_t>(kChipWidth) * lines * 4, 0);
        for (int cy = 0; cy < lines; ++cy) {
            const int sy = std::min(fh - 1, (2 * cy + 1) * fh / (2 * lines));
            for (int cx = 0; cx < kChipWidth; ++cx) {
                const int sx = std::min(fw - 1, (2 * cx + 1) * fw / (2 * kChipWidth));
                const std::uint8_t* p = full.data() + (static_cast<std::size_t>(sy) * fw + sx) * 4;
                std::uint8_t* q = chip.data() + (static_cast<std::size_t>(cy) * kChipWidth + cx) * 4;
                q[0] = p[0];
                q[1] = p[1];
                q[2] = p[2];
                q[3] = 0x80;
            }
        }
        // OPENRAC_DUMP_SNAPSHOT=FILE.png: the answer, as the game gets it.
        if (const char* dump = std::getenv("OPENRAC_DUMP_SNAPSHOT")) {
            viewer::write_png(dump, kChipWidth, lines, chip);
            log::info("frame read back at window frame {}, written to {}", g->index, dump);
        }
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            std::uint8_t* q = out.data() + (static_cast<std::size_t>(r) * width + c) * 4;
            if (renderer::frame_pixel(base, width_units, x + c, y + r, fx, fy)) {
                std::memcpy(q, chip.data() + (static_cast<std::size_t>(fy) * kChipWidth + fx) * 4, 4);
            } else {
                std::memset(q, 0, 4);
            }
        }
    }
    renderer::g_frame_snapshot.answered.push_back(renderer::hash_bytes(out.data(), out.size()));
    return true;
}

void mobys_drawn(std::span<const std::uint8_t> ram, std::uint32_t first, int count) {
    if (!g) {
        return;
    }
    const int level = openrac_game_loaded_overlay();
    const viewer::GameAddresses& a =
        viewer::rac1_addresses(g->game, level);
    const viewer::GameState camera = viewer::read_game_state(ram, a);
    for (int i = 0; i < count; ++i) {
        g->moby_cameras.emplace_back(first + static_cast<std::uint32_t>(i) * 0x100u, camera);
    }
}

void world_drawn(std::span<const std::uint8_t> ram) {
    if (!g || g->world_camera) {
        return;
    }
    const int level = openrac_game_loaded_overlay();
    const viewer::GameAddresses& a =
        viewer::rac1_addresses(g->game, level);
    g->world_camera = viewer::read_game_state(ram, a);
}

// The on-screen pad, kept inside the screen's safe area (a camera's notch at a phone's side).
static void draw_touch_pad(int window_width, int window_height) {
    platform::TouchPad& touch = g->input.touch_pad();
    SDL_Rect safe{};
    int w = 0;
    int h = 0;
    if (SDL_GetWindowSafeArea(g->window->sdl(), &safe) && SDL_GetWindowSize(g->window->sdl(), &w, &h) && h > 0) {
        const float height = static_cast<float>(h);
        touch.set_safe_area(static_cast<float>(safe.x) / height,
                            static_cast<float>(w - safe.x - safe.w) / height);
    }
    // Android's options are in a drawer (debug_overlay.h), opened with the pad's own button.
    touch.set_menu_button(debug_overlay::drawer_available());
    if (touch.take_menu_press()) {
        debug_overlay::toggle_drawer();
    }
    touch_overlay::draw(touch, window_width, window_height);
}

bool show_picture(const std::uint8_t* rgba, int width, int height, float black) {
    using namespace gl;
    if (!g || width <= 0 || height <= 0) {
        return true;
    }
    SDL_Event event;
    while (platform::poll_event(event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
        if (debug_overlay::event(event)) {
            continue;  // [debug tools]
        }
        g->input.handle_event(event);
    }
    g->input.update();
    stats::begin_frame(stats::FrameKind::Picture, g->index);  // [debug tools]

    // The fade, on the bytes as the chip's black quad blends them: C * (1 - coverage).
    std::vector<std::uint8_t> pixels(rgba, rgba + static_cast<std::size_t>(width) * height * 4);
    if (black > 0.0f) {
        const float keep = black >= 1.0f ? 0.0f : 1.0f - black;
        for (std::size_t i = 0; i < pixels.size(); i += 4) {
            for (int c = 0; c < 3; ++c) {
                pixels[i + c] = static_cast<std::uint8_t>(static_cast<float>(pixels[i + c]) * keep);
            }
        }
    }
    std::string error;
    if (g->picture.width() != width || g->picture.height() != height) {
        g->picture.create(width, height, error);
    }
    glBindTexture(GL_TEXTURE_2D, g->picture.colour_texture());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    int window_width = 0;
    int window_height = 0;
    g->window->drawable_size(window_width, window_height);
    const std::uint64_t index = g->index++;
    if (const char* shot = std::getenv("OPENRAC_SHOT")) {
        const char* colon = std::strchr(shot, ':');
        const std::uint64_t every = static_cast<std::uint64_t>(std::atoll(shot));
        if (colon && every > 0 && index % every == 0) {
            const std::string path = std::string(colon + 1) + std::to_string(index) + ".png";
            viewer::write_png(path, width, height, pixels);
            log::info("frame {} written to {}", index, path);
        }
    }
    // The picture fills the frame the game's own frames fill; its first row is the top.
    present(g->picture.id(), width, height, window_width, window_height, true);
    draw_touch_pad(window_width, window_height);
    {
        // [debug tools] The overlay over a picture too (no renderer or camera then).
        const Box box = frame_box(window_width, window_height);
        debug_overlay::Shown shown{};
        shown.window = g->window->sdl();
        shown.window_width = window_width;
        shown.window_height = window_height;
        shown.box_x = box.x;
        shown.box_y = box.y;
        shown.box_width = box.width;
        shown.box_height = box.height;
        shown.frame_width = width;
        shown.frame_height = height;
        shown.vsync = g->swap != platform::SwapInterval::Immediate;
        debug_overlay::draw(shown);
    }
    g->window->swap();
    return true;
}

namespace {

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
    double cost = 0.004;  // seconds
    std::chrono::duration<double> late{0.0};  // how long after its time the tick came (set_tick)
    float last_alpha = -1.0f;  // how far between ticks the last frame was drawn
    std::chrono::steady_clock::time_point due{};  // when the frame cap lets the next frame start
    // The kept tick has not been drawn yet (the cap held its own frame back): the next frame drawn
    // is its first.
    bool first_pending = false;
} pacing;

// Frames a second the port's own clock holds the frames between ticks to; 0: not held (one frame a
// tick, which the tick paces; a display's rate kept by the swap's wait; unlimited).
double frame_cap() {
    if (!interpolating()) {
        return 0.0;
    }
    const settings::Settings& s = settings::get();
    switch (s.frame_rate) {
        case settings::FrameRate::Capped:
            return static_cast<double>(std::clamp(s.frame_cap, settings::kFrameCapMin, settings::kFrameCapMax));
        case settings::FrameRate::Display:
            // Without the swap's wait the display's rate is kept here.
            return g->swap == platform::SwapInterval::Immediate && pacing.interval > 0.0 ? 1.0 / pacing.interval : 0.0;
        default:
            return 0.0;
    }
}

// The frames between ticks show the game half a tick late, so that the next tick is ready before
// the refresh that needs it.
constexpr double kLagTicks = 0.5;

// The refresh a frame started now is shown at: the first after the frame is drawn.
std::chrono::steady_clock::time_point next_display(std::chrono::steady_clock::time_point now) {
    using namespace std::chrono;
    const auto ready = now + duration_cast<steady_clock::duration>(duration<double>(pacing.cost));
    if (pacing.interval <= 0.0 || pacing.last_swap.time_since_epoch().count() == 0) {
        return ready;
    }
    const double since = duration<double>(ready - pacing.last_swap).count();
    const double n = std::max(1.0, std::ceil(since / pacing.interval));
    return pacing.last_swap + duration_cast<steady_clock::duration>(duration<double>(n * pacing.interval));
}

// How far the scene is from the last tick's to the kept one's at that refresh.
float alpha_at(std::chrono::steady_clock::time_point display) {
    using namespace std::chrono;
    const double t = duration<double>(display - pacing.tick).count() / pacing.period.count() - kLagTicks;
    return static_cast<float>(std::clamp(t, 0.0, 1.0));
}

void swapped(std::chrono::steady_clock::time_point started) {
    using namespace std::chrono;
    const auto before = steady_clock::now();
    platform::frame_work_done();
    g->window->swap();
    const auto after = steady_clock::now();
    const double cost = duration<double>(before - started).count();
    pacing.cost = 0.8 * pacing.cost + 0.2 * cost;
    if (settings::get().show_fps) {
        fps_counter::frame_drawn(after, cost);
    }
    if (pacing.interval_stale) {
        const float rate = g->window->refresh_rate();
        pacing.interval = rate > 20.0f ? 1.0 / rate : 0.0;
        pacing.interval_stale = false;
    }
    // What a frame may take, for the system's CPU clocks (performance_hint.h): a refresh, or the
    // player's frame cap when it is lower.
    const double cap = frame_cap();
    platform::set_frame_target(cap > 0.0 ? std::max(1.0 / cap, pacing.interval) : pacing.interval);
    if (g->swap == platform::SwapInterval::Immediate) {
        // No refresh is waited for: there is no phase to draw the next frame for.
        pacing.last_swap = {};
    } else if (duration<double>(after - before).count() < 0.0005) {
        // A swap that did not wait for a refresh (a driver or a compositor that ignores the swap
        // interval; a window that is not shown) gives no phase; the frames are then paced at the
        // display's rate here.
        pacing.last_swap = {};
        if (pacing.interval > 0.0) {
            std::this_thread::sleep_until(started + duration_cast<steady_clock::duration>(duration<double>(pacing.interval)));
        }
    } else {
        pacing.last_swap = after;
    }
    // A frame rate of the player's own (and the display's, when no swap waits): the next frame is
    // due that long after this one was, so a frame held up by the game's tick is made up for by the
    // next ones; one more than a frame late starts the count again. It holds beside the swap's
    // wait: 60 frames a second on a 120 Hz display are every other refresh. Nothing sleeps here:
    // the game's tick may be due before the next frame is (frame_due).
    if (cap > 0.0) {
        const auto step = duration_cast<steady_clock::duration>(duration<double>(1.0 / cap));
        const auto now = steady_clock::now();
        pacing.due = pacing.due.time_since_epoch().count() == 0 || now > pacing.due + 2 * step ? now
                                                                                               : pacing.due + step;
    } else {
        pacing.due = {};
    }
    // The next frame's work counts from here: the swap, and any wait in it, is the display's.
    platform::work_resumed();
}

// Whether the frame cap lets a frame start now (within a millisecond: a sleep wakes that coarsely).
bool frame_due(std::chrono::steady_clock::time_point now) {
    return frame_cap() <= 0.0 || pacing.due.time_since_epoch().count() == 0
           || now + std::chrono::milliseconds(1) >= pacing.due;
}

// OPENRAC_FRAME_COST=N: every N frames drawn (default 200), in the log, what a frame cost on
// average and where: `scene` (the tick's mobys and poses moved between ticks and sent to their
// buffers, on the CPU), `submit` (the renderers' OpenGL calls, on the CPU), `gpu` (the wait for the
// graphics card to finish them: this switch calls glFinish after each frame, so that the card's
// time is not counted into a later call; it lowers the frame rate a little) and `show` (the
// frame scaled to the window, and the swap). With the draw calls and triangles of the last frame.
struct FrameCost {
    bool on = false;
    int every = 200;
    int frames = 0;
    double scene = 0.0, submit = 0.0, gpu = 0.0, show = 0.0;
    std::chrono::steady_clock::time_point mark{};

    FrameCost() {
        if (const char* v = std::getenv("OPENRAC_FRAME_COST")) {
            on = true;
            every = std::max(1, std::atoi(v) > 1 ? std::atoi(v) : 200);
        }
    }

    // The seconds since the last call, added to `part`.
    void lap(double& part) {
        const auto now = std::chrono::steady_clock::now();
        part += std::chrono::duration<double>(now - mark).count();
        mark = now;
    }
} frame_cost;

// OPENRAC_SLOW_FRAME=MS: every frame drawn takes that much longer, as on a graphics card too slow
// for the settings, for checking that the game's time does not slow with it.
void slow_frame() {
    static const double ms = [] {
        const char* v = std::getenv("OPENRAC_SLOW_FRAME");
        return v != nullptr ? std::atof(v) : 0.0;
    }();
    if (ms > 0.0) {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(ms));
    }
}

// A moby's matrix part way from the last tick's to this one's, or this one's when it moved too far
// or turned too much in a tick to be the same thing moving (a moby slot used again, a teleport).
bool same_moby(const renderer::Mat4& from, const renderer::Mat4& to) {
    const float dx = to[12] - from[12], dy = to[13] - from[13], dz = to[14] - from[14];
    if (dx * dx + dy * dy + dz * dz > 16.0f) {
        return false;
    }
    float dot = 0.0f, la = 0.0f, lb = 0.0f;
    for (int i = 0; i < 3; ++i) {
        dot += from[i] * to[i];
        la += from[i] * from[i];
        lb += to[i] * to[i];
    }
    return la > 0.0f && lb > 0.0f && dot >= 0.7f * std::sqrt(la * lb);
}

renderer::Mat4 blend_matrix(const renderer::Mat4& from, const renderer::Mat4& to, float t) {
    const float dx = to[12] - from[12], dy = to[13] - from[13], dz = to[14] - from[14];
    if (dx * dx + dy * dy + dz * dz > 16.0f) {
        return to;  // a moby slot used again, a teleport
    }
    // Where it is always moves with the blend: a thing carried by another (the wrench in the
    // hero's hand) turns fast, more in a tick than the same-moby test allows, and one left at this
    // tick's place while its carrier is drawn part way there runs ahead of the hand. Only its
    // turning is taken whole when it turned too far to blend its axes.
    renderer::Mat4 m = to;
    const int upto = same_moby(from, to) ? 12 : 0;
    for (int i = 0; i < upto; ++i) {
        m[i] = from[i] + (to[i] - from[i]) * t;
    }
    for (int i = 12; i < 15; ++i) {
        m[i] = from[i] + (to[i] - from[i]) * t;
    }
    return m;
}

renderer::Vec3 normalised(renderer::Vec3 v) {
    const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return l > 0.0f ? renderer::Vec3{v[0] / l, v[1] / l, v[2] / l} : v;
}

renderer::Vec3 cross(const renderer::Vec3& a, const renderer::Vec3& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

// The camera part way from the last tick's to this one's (a cut is not blended).
viewer::GameState blend_camera(const viewer::GameState& from, const viewer::GameState& to, float t) {
    viewer::GameState c = to;
    const renderer::Vec3& p0 = from.camera_position;
    const renderer::Vec3& p1 = to.camera_position;
    const float d2 = (p1[0] - p0[0]) * (p1[0] - p0[0]) + (p1[1] - p0[1]) * (p1[1] - p0[1])
                     + (p1[2] - p0[2]) * (p1[2] - p0[2]);
    const float turn = from.forward[0] * to.forward[0] + from.forward[1] * to.forward[1]
                       + from.forward[2] * to.forward[2];
    if (d2 > 64.0f || turn < 0.7f) {
        return c;
    }
    for (int i = 0; i < 3; ++i) {
        c.camera_position[i] = p0[i] + (p1[i] - p0[i]) * t;
    }
    renderer::Vec3 f, u;
    for (int i = 0; i < 3; ++i) {
        f[i] = from.forward[i] + (to.forward[i] - from.forward[i]) * t;
        u[i] = from.up[i] + (to.up[i] - from.up[i]) * t;
    }
    c.forward = normalised(f);
    c.left = normalised(cross(u, c.forward));
    c.up = cross(c.forward, c.left);
    return c;
}

}  // namespace

// [debug tools] The picture being drawn is a paused game's (hold).
static bool g_held = false;

// Draws the kept tick, the camera and mobys `alpha` of the way from the last tick's to its own,
// and shows it. `first`: the tick's own frame (shots, the frame read back by store_image).
void draw_kept(bool first) {
    const auto started = std::chrono::steady_clock::now();
    const float alpha = interpolating() ? alpha_at(next_display(started)) : 1.0f;
    debug_overlay::begin(first, g_held, alpha, g->kept.frame);  // [debug tools] the frame's measurements
    pacing.last_alpha = alpha;
    frame_cost.mark = started;
    // OPENRAC_PACING: each frame shown, when (ms), how far between ticks, the refresh interval.
    static const bool& pacing_log = stats::probe("OPENRAC_PACING", "each frame shown and when, in the log");
    if (pacing_log) {
        static const auto t0 = started;
        log::info("pace {} {:.2f} ms alpha {:.3f} {} refresh {:.2f} ms cost {:.2f} ms", g->kept.frame,
                  std::chrono::duration<double, std::milli>(started - t0).count(), alpha, first ? "tick" : "between",
                  pacing.interval * 1000.0, pacing.cost * 1000.0);
    }
    Kept& k = g->kept;
    const viewer::GameState state =
        k.has_prev_state && alpha < 1.0f ? blend_camera(k.prev_state, k.state, alpha) : k.state;
    if (!g->level.models.empty()) {
        std::vector<viewer::Instance> live = k.live;
        if (alpha < 1.0f) {
            for (std::size_t i = 0; i < live.size() && i < k.addresses.size(); ++i) {
                auto it = k.prev_matrices.find(k.addresses[i]);
                if (it != k.prev_matrices.end()) {
                    live[i].matrix = blend_matrix(it->second, live[i].matrix, alpha);
                }
            }
        }
        if (alpha < 1.0f && !k.prev_poses.empty()) {
            // The poses too: each joint part way from the last tick's to this one's, for a moby
            // that is the same thing moving (its matrix was blended) with the same skeleton.
            std::vector<viewer::LevelScene::JointColumns> palette = k.palette;
            for (std::size_t i = 0; i < live.size() && i < k.addresses.size() && i < k.joints.size(); ++i) {
                const auto pose = k.prev_poses.find(k.addresses[i]);
                const int first = live[i].palette;
                const int count = k.joints[i];
                const auto last = k.prev_matrices.find(k.addresses[i]);
                if (pose == k.prev_poses.end() || first < 0 || count <= 0 || pose->second.second != count
                    || last == k.prev_matrices.end() || !same_moby(last->second, k.live[i].matrix)
                    || static_cast<std::size_t>(first + count) > palette.size()) {
                    continue;
                }
                const viewer::LevelScene::JointColumns* from = k.prev_palette.data() + pose->second.first;
                for (int j = 0; j < count; ++j) {
                    viewer::LevelScene::JointColumns& to = palette[static_cast<std::size_t>(first + j)];
                    for (std::size_t c = 0; c < to.size(); ++c) {
                        to[c] = from[j][c] + (to[c] - from[j][c]) * alpha;
                    }
                }
            }
            g->scene.set_palette(palette);
        } else {
            g->scene.set_palette(k.palette);
        }
        g->scene.set_instances(viewer::Layer::Mobys, live);
    }

    if (frame_cost.on) {
        frame_cost.lap(frame_cost.scene);
    }

    int window_width = 0;
    int window_height = 0;
    g->window->drawable_size(window_width, window_height);
    const float aspect = picture_aspect(window_width, window_height);
    const Box box = frame_box(window_width, window_height, aspect);
    // The frame: the picture's box in the window, at the player's render scale, with the player's
    // samples a pixel. It is scaled to the box when it is shown (present).
    int width = 0;
    int height = 0;
    frame_size(box.width, box.height, width, height);
    // How much of the frame's width the game's 4:3 picture is: all of it, or its middle.
    const float picture_scale = aspect > kTvAspect && box.width > 0 && box.height > 0
        ? std::min(1.0f, kTvAspect * static_cast<float>(box.height) / static_cast<float>(box.width)) : 1.0f;
    std::string error;
    const int samples = std::clamp(settings::get().msaa, 1, renderer::FrameBuffer::max_samples());
    if (box.width > 0 && box.height > 0
        && (g->frame.width() != width || g->frame.height() != height || g->frame.samples() != samples)) {
        if (!g->frame.create(width, height, error, samples) && samples > 1) {
            // A card that refuses the samples it listed: the frame without them.
            log::warn("{}; drawing without anti-aliasing", error);
            g->frame.create(width, height, error, 1);
        }
        log::info("frame {}x{} for a picture of {}x{} pixels, {} sample(s) a pixel", g->frame.width(),
                  g->frame.height(), box.width, box.height, g->frame.samples());
    }
    if (g->renderer) {
        g->renderer->textures().set_anisotropy(static_cast<float>(settings::get().anisotropy));
    }
    renderer::FrameInput input;
    // No camera yet (loading, fades between programs): a black frame, not the level's background.
    const bool has_camera = k.has_camera;
    input.clear_colour = has_camera
        ? std::array<float, 4>{g->level.background[0], g->level.background[1], g->level.background[2], 1}
        : std::array<float, 4>{0, 0, 0, 1};
    for (auto& r : g->renderer ? g->renderer->renderers() : std::span<const std::unique_ptr<renderer::BucketRenderer>>{}) {
        if (r->bucket() == renderer::Bucket::Particles) {
            // The effect quads are there only when the game drew them.
            r->enabled = has_camera;
        } else if (r->bucket() != renderer::Bucket::Hud) {
            r->enabled = has_camera && (k.draws & (1u << static_cast<unsigned>(r->bucket()))) != 0;
        }
    }
    input.camera.view = state.view();
    // The game's frustum: its horizontal and vertical tangents as the game set them (the frame is
    // the TV's 4:3, which they were made for), not the window's shape. A wider picture keeps the
    // vertical one and opens the horizontal one by as much as the picture is wider: the game's
    // view is its middle.
    input.camera.projection = state.projection(
        (state.tan_half_fov_y > 0.0f ? state.tan_half_fov_x / state.tan_half_fov_y : 4.0f / 3.0f) / picture_scale,
        0.05f, 2000.0f
    );
    input.camera.position = state.camera_position;
    input.frame = k.frame;
    input.strip_vertices = k.strip_vertices;
    input.strips = k.strips;
    input.direct_before_world = std::span<const std::uint8_t>(k.packets).first(k.before_world);
    input.packets[static_cast<std::size_t>(renderer::Bucket::Hud)] =
        std::span<const std::uint8_t>(k.packets).subspan(k.before_world);
    input.effects = k.effects;
    input.sky_shells = k.sky_shells;
    input.sky_shell_mask = k.sky_shell_mask;
    input.effect_uploads = k.effect_uploads;
    input.ui_quads = k.ui_quads;
    // The mobys the game drew into a texture, at their markers (Kept::target_mobys): seen by the
    // camera they were drawn with, through the frame's focal lengths, about the centre of the
    // target the 2D layer was drawing into (its XYOFFSET's 2048), in its 512-pixel square.
    input.target_mobys = [&](std::uint64_t marker, unsigned framebuffer, int size, float centre_x, float centre_y,
                             bool clear_depth) {
        const std::uint32_t first = static_cast<std::uint32_t>(marker);
        const std::uint32_t count = static_cast<std::uint32_t>(marker >> 32) & 0xFFu;
        std::vector<viewer::Instance> instances;
        const viewer::GameState* camera = nullptr;
        for (const Kept::TargetMoby& t : k.target_mobys) {
            if (t.address >= first && t.address < first + count * 0x100u) {
                instances.push_back(t.instance);
                camera = &t.camera;
            }
        }
        if (camera == nullptr || !g->renderer || camera->focal_x <= 0.0f) {
            return;
        }
        using namespace openrac::gl;
        glViewport(0, 0, size, size);
        glDisable(GL_SCISSOR_TEST);
        if (clear_depth) {
            glDepthMask(GL_TRUE);
            glClearDepth(0.0);
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        const float half = 0.5f * static_cast<float>(renderer::kOffscreenPixels);
        const float tan_x = half / camera->focal_x;
        const float tan_y = half / camera->focal_y;
        renderer::Mat4 projection = renderer::perspective_reversed(2.0f * std::atan(tan_y), tan_x / tan_y, 0.05f, 2000.0f);
        // The axis through the centre: x on and y down from the square's middle, in device units.
        renderer::Mat4 offset = renderer::identity();
        offset[12] = (centre_x - half) / half;
        offset[13] = -(centre_y - half) / half;
        renderer::FrameInput panel;
        panel.camera.view = camera->view();
        panel.camera.projection = renderer::multiply(offset, projection);
        panel.camera.position = camera->camera_position;
        renderer::RenderState target{g->renderer->textures(), renderer::RenderTarget{framebuffer, size, size}, {}};
        g->scene.draw_mobys(instances, panel, target);
    };
    // A tick that draws no world and has no picture of its own to put under its 2D (the page
    // menu's first ticks, before its snapshot of the frame comes; the tick it closes on): the
    // console still shows the last frame there, so the 2D goes over the last world drawn, not black.
    // Only right after the world was drawn (the menu opening) or right after a menu tick with its
    // snapshot (the menu closing): other ticks without a world (a new game's captions between its
    // scenes) are drawn on black, as they come.
    constexpr std::size_t kSnapshotBytes = 0x40000;
    const bool after_world = g->last_world_tick != 0 && k.frame - g->last_world_tick <= 3;
    if (first && has_camera && (k.draws & 0x0Fu) == 0 && k.before_world < kSnapshotBytes
        && (after_world || g->last_tick_menu) && g->last_world.width() == width
        && g->last_world.height() == height) {
        g->hold_last_world = true;
    } else if (first) {
        g->hold_last_world = false;
    }
    if (g->hold_last_world && !g->repeat_frame && g->last_world.width() == width
        && g->last_world.height() == height) {
        g->last_world.blit_to(g->frame.id(), width, height);
        input.keep_colour = true;
    }
    // The last frame that drew its world, as the world alone (before the HUD's 2D goes over it):
    // what the page menus read back for their background, and what a tick without a world of its
    // own is drawn over.
    if (first && has_camera && (k.draws & 0x1Fu) != 0 && width > 0 && height > 0) {
        input.before_hud = [&]() {
            if (g->last_world.width() != width || g->last_world.height() != height) {
                std::string e;
                g->last_world.create(width, height, e);
            }
            g->frame.blit_to(g->last_world.id(), width, height);
            g->last_world_tick = k.frame;
        };
    }
    if (g->renderer && !g->repeat_frame) {
        // [debug tools] The passes hidden in the overlay; the render timed (common/stats.h).
        debug_overlay::before_render(g->renderer.get());
        const stats::Scope render_scope("render");
        renderer::RenderTarget target{g->frame.id(), width, height, picture_scale};
        target.resolve_framebuffer = g->frame.resolve_target();
        target.resolves_itself = g->frame.resolves_itself();
        target.wide_2d = picture_scale < 1.0f && settings::get().hud == settings::Hud::Edges;
        g->renderer->render(input, target);
    }
    if (frame_cost.on) {
        frame_cost.lap(frame_cost.submit);
        gl::glFinish();
        frame_cost.lap(frame_cost.gpu);
    }
    // What is shown: the frame with one sample a pixel (a multisampled one is resolved here, once),
    // with the port's own overlays over it.
    const unsigned shown = g->frame.resolved();
    debug_menu::draw(shown, width, height);
    if (settings::get().show_fps) {
        fps_counter::draw(shown, width, height);
    }
    // OPENRAC_SHOT=FRAME:FILE.png writes that frame (for checking without looking at the screen);
    // OPENRAC_SHOT_BETWEEN also writes the frames drawn between ticks (FILE<frame>_b<n>.png).
    static const bool& shot_between = stats::probe("OPENRAC_SHOT_BETWEEN", "OPENRAC_SHOT also writes the frames between ticks");
    static int between_index = 0;
    between_index = first ? 0 : between_index + 1;
    if (const char* shot = first || shot_between ? std::getenv("OPENRAC_SHOT") : nullptr) {
        const char* colon = std::strchr(shot, ':');
        const std::uint64_t every = static_cast<std::uint64_t>(std::atoll(shot));
        // OPENRAC_SHOT_FROM=FRAME: no shots before that frame.
        static const std::uint64_t shot_from = [] {
            const char* v = std::getenv("OPENRAC_SHOT_FROM");
            return v != nullptr ? std::strtoull(v, nullptr, 10) : 0ull;
        }();
        if (colon && every > 0 && input.frame % every == 0 && input.frame >= shot_from) {
            const std::string path = std::string(colon + 1) + std::to_string(input.frame)
                                     + (first ? std::string() : "_b" + std::to_string(between_index)) + ".png";
            viewer::write_png(path, width, height, renderer::read_rgba(shown, width, height));
            log::info("frame {} written to {}", input.frame, path);
        }
    }
    slow_frame();
    present(shown, width, height, window_width, window_height, false, aspect);
    draw_touch_pad(window_width, window_height);
    // [debug tools] The overlay, over the frame in the window.
    debug_overlay::draw({g->window->sdl(), window_width, window_height, box.x, box.y, box.width, box.height,
                         g->renderer.get(), &input.camera, g->frame.width(), g->frame.height(),
                         g->last_world.width(), g->last_world.height(), g->swap != platform::SwapInterval::Immediate,
                         interpolating(), pacing.interval, pacing.cost, pacing.period.count()});
    const stats::Scope swap_scope("swap");
    swapped(started);
    if (frame_cost.on) {
        frame_cost.lap(frame_cost.show);
        if (++frame_cost.frames >= frame_cost.every) {
            const double n = frame_cost.frames * 0.001;
            const renderer::FrameStats stats = g->renderer ? g->renderer->last_stats() : renderer::FrameStats{};
            log::info("cost tick {}: {:.3f} ms a frame over {} frames (scene {:.3f}, submit {:.3f}, gpu {:.3f}, "
                      "show {:.3f}); {} draw calls, {} triangles, frame {}x{} x{}",
                      g->kept.frame, (frame_cost.scene + frame_cost.submit + frame_cost.gpu + frame_cost.show) / n,
                      frame_cost.frames, frame_cost.scene / n, frame_cost.submit / n, frame_cost.gpu / n,
                      frame_cost.show / n, stats.draw_calls, stats.triangles, width, height, g->frame.samples());
            frame_cost.frames = 0;
            frame_cost.scene = frame_cost.submit = frame_cost.gpu = frame_cost.show = 0.0;
        }
    }
}

namespace {

// A window the player sizes: never on Android, where the game covers the screen whatever the
// settings say (Window::set_display_mode).
bool windowed() {
#if defined(__ANDROID__)
    return false;
#else
    return g->display_mode == settings::DisplayMode::Windowed;
#endif
}

// The window as the player's settings want it: its display mode, display and fullscreen size, its
// shape, and each swap waiting for a refresh or not. Called when they may have changed; all of
// them take effect at once (the frame's size, its samples and the textures' filter follow the
// settings by themselves, in draw_kept).
void apply_settings() {
    const settings::Settings& s = settings::get();
    if (s.display_mode != g->display_mode || s.monitor != g->monitor
        || (s.display_mode == settings::DisplayMode::Fullscreen
            && (s.fullscreen_width != g->fullscreen_width || s.fullscreen_height != g->fullscreen_height))) {
        g->window->set_display_mode(window_mode(s.display_mode), s.fullscreen_width, s.fullscreen_height, s.monitor);
        g->display_mode = s.display_mode;
        g->monitor = s.monitor;
        g->fullscreen_width = s.fullscreen_width;
        g->fullscreen_height = s.fullscreen_height;
        pacing.interval_stale = true;
        // A change of mode may leave the driver's swap interval at its default: asked again below.
        g->swap_asked = wanted_swap() == platform::SwapInterval::Immediate ? platform::SwapInterval::VSync
                                                                           : platform::SwapInterval::Immediate;
    }
    if (s.aspect != g->aspect || s.aspect_custom_x != g->aspect_custom_x || s.aspect_custom_y != g->aspect_custom_y) {
        g->aspect = s.aspect;
        g->aspect_custom_x = s.aspect_custom_x;
        g->aspect_custom_y = s.aspect_custom_y;
        // A window takes the picture's new shape, at the height it has.
        int w = 0;
        int h = 0;
        if (windowed() && s.aspect != settings::Aspect::Window
            && SDL_GetWindowSize(g->window->sdl(), &w, &h) && h > 0) {
            SDL_SetWindowSize(g->window->sdl(), static_cast<int>(std::lround(static_cast<float>(h) * picture_aspect(w, h))), h);
        }
    }
    if (wanted_swap() != g->swap_asked) {
        g->swap_asked = wanted_swap();
        g->swap = g->window->set_swap_interval(g->swap_asked);
        pacing.last_swap = {};
    }
}

// The window's own events, which the game never sees: the window moved to another display or its
// display changed mode (the refresh rate is read again), it was sized (the size is kept for the
// next start), it lost or got the keyboard (the player's "mute in the background").
void window_event(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
        case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            pacing.interval_stale = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            // Remembered in memory; written with the next setting saved and when the window closes.
            if (windowed() && event.window.data1 >= 320
                && event.window.data2 >= 240) {
                settings::get().window_width = event.window.data1;
                settings::get().window_height = event.window.data2;
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            g->muted = event.type == SDL_EVENT_WINDOW_FOCUS_LOST;
            break;
        default:
            break;
    }
    const bool muted = g->muted && settings::get().mute_unfocused;
    settings::set_muted(muted);
}

// The window's own keys: F11 and Alt+Return change between fullscreen and the window, F9 and F10
// turn the port's volume down and up a tenth, F2 shows or hides the frame counter. True when the event was one of them.
bool window_key(const SDL_Event& event) {
    window_event(event);
    if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat) {
        return false;
    }
    if (event.key.key == SDLK_F2) {  // the frame counter, as the Video options' row
        settings::get().show_fps = !settings::get().show_fps;
        settings::save();
        return true;
    }
    if (event.key.key == SDLK_F9 || event.key.key == SDLK_F10) {
        settings::set_volume(settings::get().volume + (event.key.key == SDLK_F10 ? 10 : -10));
        settings::save();
        log::info("volume {}", settings::get().volume);
        return true;
    }
    const bool alt_return = (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER)
                            && (event.key.mod & SDL_KMOD_ALT) != 0;
    if (event.key.key != SDLK_F11 && !alt_return) {
        return false;
    }
    // Between the window and the fullscreen kind last chosen (borderless until one was).
    static settings::DisplayMode full = settings::DisplayMode::Borderless;
    settings::Settings& s = settings::get();
    if (s.display_mode == settings::DisplayMode::Windowed) {
        s.display_mode = full;
    } else {
        full = s.display_mode;
        s.display_mode = settings::DisplayMode::Windowed;
    }
    settings::save();
    apply_settings();
    return true;
}

}  // namespace

// The generation the first native-quad marker in a 2D list names (ui_quad): an A+D write of
// kUiQuadMarker to the NOP register. None when the list has no marker.
std::optional<std::uint32_t> marker_generation(std::span<const std::uint8_t> packets) {
    for (std::size_t at = 0; at + 9 <= packets.size(); at += 8) {
        std::uint64_t value = 0;
        std::memcpy(&value, packets.data() + at, 8);
        if ((value >> 48) == renderer::GifInterpreter::kUiQuadMarker && packets[at + 8] == 0x7F) {
            return static_cast<std::uint32_t>(value >> 24) & 0x3F;
        }
    }
    return std::nullopt;
}

bool frame(std::span<const std::uint8_t> ram, std::uint32_t chain, std::uint32_t draws) {
    if (!g) {
        return true;
    }
    SDL_Event event;
    while (platform::poll_event(event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
        if (window_key(event) || debug_overlay::event(event)
            || debug_menu::event(event, openrac_game_loaded_overlay())) {
            continue;  // the window's own keys; the debug overlay's; the port's debug menu (debug_menu.h)
        }
        g->input.handle_event(event);
    }
    g->input.update();
    // The port's rows on the game's Video options page (options_menu.h), and the overlay's drawer
    // and menus: a changed one is applied. Before the debug menu, which adds its entry to the
    // Options list options_menu gives the game.
    const bool changed = options_menu::update(openrac_game_loaded_overlay());
    if (debug_overlay::take_settings_changed() || changed) {
        apply_settings();
    }
    debug_menu::update(openrac_game_loaded_overlay(), g->index);

    // The level program loaded now (-1: the boot program, title and menus), as the port knows it.
    const int level = openrac_game_loaded_overlay();
    const viewer::GameAddresses& a =
        viewer::rac1_addresses(g->game, level);
    // The boot program's worlds: the flight between planets (game mode 6), else the title world.
    use_level(level >= 0 && level < 19 ? level : word_at(ram, game_mode_address()) == 6 ? -2 : -1);
    viewer::GameState state = viewer::read_game_state(ram, a);
    // The world is drawn from the camera the game's world renderers used, not from whatever the
    // camera globals hold at the end of the frame (the page menu sets its own camera, at
    // (256, 256, 64) facing +x, to draw its frame objects, as ReRAC notes).
    // A tick on which the world renderers did not run but mobys were drawn with a camera of their
    // own (a page menu's frame objects): only those are drawn, as the game drew no others (the
    // pause page skips the Goodies frame while it has none).
    const bool world_ran = g->world_camera.has_value();
    const bool only_own_camera = !world_ran && !g->moby_cameras.empty();
    if (g->world_camera) {
        state.camera_position = g->world_camera->camera_position;
        state.forward = g->world_camera->forward;
        state.left = g->world_camera->left;
        state.up = g->world_camera->up;
        state.tan_half_fov_x = g->world_camera->tan_half_fov_x;
        state.tan_half_fov_y = g->world_camera->tan_half_fov_y;
        state.fog_colour = g->world_camera->fog_colour;
        state.fog_near = g->world_camera->fog_near;
        state.fog_far = g->world_camera->fog_far;
        state.fog_near_f = g->world_camera->fog_near_f;
        state.fog_far_f = g->world_camera->fog_far_f;
        g->world_camera.reset();
    }

    // OPENRAC_CAM_LOG: the camera each frame is drawn from (position, forward, up), in the log.
    static const bool& cam_log = stats::probe("OPENRAC_CAM_LOG", "the camera each tick is drawn from, in the log");
    if (cam_log) {
        log::info("cam {} {:.3f} {:.3f} {:.3f} f {:.4f} {:.4f} {:.4f} u {:.4f} {:.4f} {:.4f} world {}", g->index,
                  state.camera_position[0], state.camera_position[1], state.camera_position[2], state.forward[0],
                  state.forward[1], state.forward[2], state.up[0], state.up[1], state.up[2], world_ran ? 1 : 0);
    }
    // OPENRAC_DUMP_RAM=FRAME: the game's 32 MB of main memory at that frame, to ram_FRAME.bin.
    static const char* dump_ram = std::getenv("OPENRAC_DUMP_RAM");
    if (dump_ram != nullptr && g->index == std::strtoull(dump_ram, nullptr, 10)) {
        const std::string path = std::format("ram_{}.bin", g->index);
        if (std::FILE* f = std::fopen(path.c_str(), "wb")) {
            std::fwrite(ram.data(), 1, ram.size(), f);
            std::fclose(f);
            log::info("main memory written to {}", path);
        }
    }

    // OPENRAC_DEBUG: the game's state each 100 frames, for bring-up.
    static const bool& debug = stats::probe("OPENRAC_DEBUG", "the game's state each 100 ticks, in the log");
    if (debug && g->index % 100 == 0) {
        log::info(
            "frame {}: level {}, camera {:.1f} {:.1f} {:.1f}, forward {:.2f} {:.2f} {:.2f}, fov {:.3f}, {} mobys, layers drawn {:#x}",
            g->index, level, state.camera_position[0], state.camera_position[1],
            state.camera_position[2], state.forward[0], state.forward[1], state.forward[2],
            state.tan_half_fov_y, state.mobys.size(), draws
        );
        if (const openrac_front_end* fe = openrac_game_front_end) {
            // The first pad's block: +0x100 the sticks, +0x194 its socket, +0x198 its state,
            // +0x1A0 the buttons held, +0x1A4 the raw word.
            log::info("  mode {} dialog kind {} step {} level word {} title exit {} pad {:#x}",
                      static_cast<int>(word_at(ram, fe->game_mode)), static_cast<int>(word_at(ram, fe->dialog)),
                      static_cast<int>(word_at(ram, fe->dialog + 0x1C)), static_cast<int>(word_at(ram, fe->level)),
                      static_cast<int>(word_at(ram, fe->title_exit)), word_at(ram, fe->pad + 0x1A4));
            float axes[4]{};
            std::memcpy(axes, ram.data() + fe->pad + 0x100, sizeof axes);
            log::info("  pad socket {} state {} held {:#x}, right {:.3f} {:.3f}, left {:.3f} {:.3f}",
                      word_at(ram, fe->pad + 0x194), word_at(ram, fe->pad + 0x198), word_at(ram, fe->pad + 0x1A0),
                      axes[0], axes[1], axes[2], axes[3]);
        }
        log::info("  fog colour {:.0f} {:.0f} {:.0f}, depth {:.0f} to {:.0f}, F {:.0f} to {:.0f}",
                  state.fog_colour[0] * 255, state.fog_colour[1] * 255, state.fog_colour[2] * 255,
                  state.fog_near, state.fog_far, state.fog_near_f, state.fog_far_f);
        static const char* fx_dump = std::getenv("OPENRAC_FX_DUMP");
        if (fx_dump != nullptr && g->index >= std::strtoull(fx_dump, nullptr, 10) && g->index < std::strtoull(fx_dump, nullptr, 10) + 100) {
            for (const renderer::EffectQuad& e : g->effects) {
                log::info("    quad {:.1f} {:.1f} {:.1f} {:.2f} | {:.1f} {:.1f} {:.1f} | {:.1f} {:.1f} {:.1f} | {:.1f} {:.1f} {:.1f} rgba {:#x} st {:.2f},{:.2f} {:.2f},{:.2f} tex0 {:#x} alpha {:#x} clamp {:#x}",
                          e.corner[0][0], e.corner[0][1], e.corner[0][2], e.corner[0][3], e.corner[1][0], e.corner[1][1], e.corner[1][2],
                          e.corner[2][0], e.corner[2][1], e.corner[2][2], e.corner[3][0], e.corner[3][1], e.corner[3][2],
                          e.rgba[0], e.st[0][0], e.st[0][1], e.st[3][0], e.st[3][1], e.tex0, e.alpha, e.clamp);
            }
        }
        if (!g->sky_sprites.empty()) {
            const auto& [r, tex0] = g->sky_sprites.front();
            float size;
            std::memcpy(&size, r.data() + 0x1C, 4);
            log::info("  {} sky sprites; the first RGBA {:#x} size {:.3f} TEX0 {:#x} ALPHA {:#x}", g->sky_sprites.size(),
                      r[4] | r[5] << 8 | r[6] << 16 | static_cast<std::uint32_t>(r[7]) << 24, size, tex0, r[3]);
        }
        if (!g->particles.empty()) {
            int kinds[4] = {0, 0, 0, 0};
            for (const State::Particle& p : g->particles) {
                kinds[p.record[1] & 3]++;
            }
            log::info("  {} particles (sprites {}, flat {}, lines {}, ribbons {})", g->particles.size(), kinds[0],
                      kinds[1], kinds[2], kinds[3]);
        }
        if (!g->effects.empty()) {
            const renderer::EffectQuad& e = g->effects.front();
            log::info("  {} effect quads; the first at {:.1f} {:.1f} {:.1f}, TEX0 {:#x}, ALPHA {:#x}, RGBA {:#x}",
                      g->effects.size(), e.corner[0][0], e.corner[0][1], e.corner[0][2], e.tex0, e.alpha, e.rgba[0]);
        }
        log::info("  load stage {:#x} retries {} snd state {:#x}", word_at(ram, 0x0015EF48),
                  word_at(ram, 0x0015EFBC), word_at(ram, word_at(ram, 0x001517D0) + 8) & 0xFFFF);
    }

    // OPENRAC_MOBY_LOG=N[:RADIUS]: every N ticks, the mobys within RADIUS game units of the hero
    // (default 20) in the log: class, address, state, mode bits, animation, position, the distance,
    // the collision radius and the update function, for seeing what an object is doing.
    static const char* moby_log = std::getenv("OPENRAC_MOBY_LOG");
    if (moby_log != nullptr && std::atoi(moby_log) > 0 && g->index % static_cast<std::uint64_t>(std::atoi(moby_log)) == 0) {
        const char* colon = std::strchr(moby_log, ':');
        const float radius = colon != nullptr ? static_cast<float>(std::atof(colon + 1)) : 20.0f;
        const viewer::LiveMoby* hero = nullptr;
        for (const viewer::LiveMoby& m : state.mobys) {
            if (m.class_id == 0) {
                hero = &m;
                break;
            }
        }
        for (const viewer::LiveMoby& m : state.mobys) {
            if (hero == nullptr || m.address + 0x100 > ram.size()) {
                continue;
            }
            const float dx = m.matrix[12] - hero->matrix[12], dy = m.matrix[13] - hero->matrix[13],
                        dz = m.matrix[14] - hero->matrix[14];
            const float d = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (d > radius) {
                continue;
            }
            const std::uint8_t* b = ram.data() + m.address;
            float sphere[4];
            float t = 0.0f;
            std::memcpy(sphere, b, sizeof sphere);
            std::memcpy(&t, b + 0x54, 4);
            log::info("moby {} class {} at {:#x}: state {} mode {:#06x} seq {} frame {} t {:.2f} pos {:.2f} {:.2f} {:.2f} "
                      "dist {:.2f} radius {:.2f} update {:#x} pvars {:#x} draw {} {} {:#x}",
                      g->index, m.class_id, m.address, b[0x20], static_cast<unsigned>(b[0x34] | b[0x35] << 8), b[0x52],
                      b[0x50], t, m.matrix[12], m.matrix[13], m.matrix[14], d, sphere[3], word_at(ram, m.address + 0x74),
                      word_at(ram, m.address + 0x78), b[0x30], b[0x31], static_cast<unsigned>(b[0x32] | b[0x33] << 8));
        }
    }

    // The live mobys, by class, each in the pose its animation fields give (viewer/moby_pose.h):
    // its joint palette goes to the scene, which skins the class's mesh with it. OPENRAC_ANIM=0
    // draws every moby in its bind pose.
    static const bool animate = [] {
        const char* v = std::getenv("OPENRAC_ANIM");
        return v == nullptr || std::strcmp(v, "0") != 0;
    }();
    std::vector<viewer::Instance> live;
    std::vector<std::uint32_t> live_addresses;
    std::vector<int> live_joints;
    std::vector<viewer::LevelScene::JointColumns> palette;
    std::vector<Kept::TargetMoby> target_mobys;
    // Mobys the game drew into a texture, and the ones it drew that are not in its list. The game
    // renders some mobys into a target the 2D layer then copies onto the screen: the vendor's item
    // and salesman screens (ReRAC interaction.md 9.3), the page menus' widgets (the 3D Ratchet and
    // the item previews, ReRAC menu_models.rs). It sets a zoom of 1.0 for them instead of the
    // frame's 0.63: a moby drawn with a wider tangent than the frame's goes into the texture at its
    // marker. The zoom does not reach the moby program's scales: drawn with it, the models sat at
    // the targets' edges at a third of their size, as ReRAC found too; they are drawn with the
    // frame's focal lengths (the frame camera's GameState::focal_x). A moby not in the list (the
    // vendor's own) is read from its address; with the frame's camera (the item's hologram over the
    // vendor) it is drawn in the world.
    const std::size_t listed = state.mobys.size();
    std::unordered_map<std::uint32_t, const viewer::GameState*> into_texture;
    std::array<float, 2> frame_focal{};
    {
        const auto drawn = [&](std::uint32_t address) {
            return std::any_of(state.mobys.begin(), state.mobys.end(),
                               [&](const viewer::LiveMoby& m) { return m.address == address; });
        };
        float zoom = world_ran ? state.tan_half_fov_x : 0.0f;
        frame_focal = world_ran ? std::array<float, 2>{state.focal_x, state.focal_y} : std::array<float, 2>{};
        for (const auto& [address, camera] : g->moby_cameras) {
            if (zoom == 0.0f || camera.tan_half_fov_x < zoom) {
                zoom = camera.tan_half_fov_x;
                frame_focal = {camera.focal_x, camera.focal_y};
            }
        }
        for (const auto& [address, camera] : g->moby_cameras) {
            if (camera.tan_half_fov_x > zoom + 1.0e-3f && frame_focal[0] > 0.0f) {
                into_texture.emplace(address, &camera);
            }
            if (!drawn(address)) {
                if (const auto extra = viewer::read_live_moby(ram, address)) {
                    state.mobys.push_back(*extra);
                }
            }
        }
    }
    for (std::size_t index = 0; index < state.mobys.size(); ++index) {
        const viewer::LiveMoby& m = state.mobys[index];
        const bool extra = index >= listed;
        // Hidden (mode bit 0): MobyProc does not draw it (the parked ship while the landing one comes
        // down, Clank's spare parts).
        if (m.address + 0x36 <= ram.size() && (ram[m.address + 0x34] & 1) != 0) {
            continue;
        }
        // A draw distance (+0x32) of 0: MobyProc's distance test never passes, so the game never draws
        // it. Volumes and blockers the levels place as mobys (Veldin's grey box behind the start, the
        // walls of Kalebo III's landing) are such: untextured, they showed as solid grey walls.
        if (!extra && m.address + 0x34 <= ram.size() && (ram[m.address + 0x32] | ram[m.address + 0x33]) == 0) {
            continue;
        }
        if (only_own_camera
            && std::none_of(g->moby_cameras.begin(), g->moby_cameras.end(),
                            [&](const auto& c) { return c.first == m.address; })) {
            continue;
        }
        auto cls = g->level.moby_classes.find(m.class_id);
        if (cls == g->level.moby_classes.end()) {
            continue;
        }
        viewer::Instance instance{cls->second.first, m.matrix, {1, 1, 1, 1}};
        // How MobyProc blends it (ReRAC's MobyBlend, from the game's moby draw): mode bit 0x200
        // (+0x34) adds it; an alpha (+0x23) below 0x80 blends it at that alpha (the explosions'
        // flashes start at 0x20..0x40 and fade out; a fireball can be spawned below 0x80).
        if (m.address + 0x36 <= ram.size()) {
            const std::uint8_t* b = ram.data() + m.address;
            const unsigned mode = b[0x34] | b[0x35] << 8;
            const unsigned alpha = b[0x23];
            // Its shine distance (+0x73, 0x18 for a class with metal packets, 32 for the Gadgetron
            // logo) for the metal pass, in the tie-only `lights` slot as -1 - distance (mesh.vert).
            if (b[0x73] != 0) {
                instance.lights = -1 - static_cast<int>(b[0x73]);
            }
            if ((mode & 0x200) != 0) {
                instance.blend = 2;
                instance.tint[3] = static_cast<float>(alpha) / 128.0f;
            } else if (alpha < 0x80) {
                instance.blend = 1;
                instance.tint[3] = static_cast<float>(alpha) / 128.0f;
            }
        }
        // Its light word (+0x38: set 0, set 1, the cross-fade) and ambient colour (+0x3C), as
        // MobyProc reads them to light it.
        if (!g->level.light_sets.empty() && m.address + 0x100 <= ram.size()) {
            const std::uint8_t* b = ram.data() + m.address;
            instance.moby_light = {static_cast<float>(b[0x38]), static_cast<float>(b[0x39]),
                                   static_cast<float>(b[0x3A]) / 256.0f, 1.0f};
            // w: with mode bit 0x10, 1 + its glow word's RGB (+0x90), which MobyProc's glow list
            // draws its glow packets in; 0 without (a 24-bit integer is exact in a float).
            const std::uint32_t glow = static_cast<std::uint32_t>(b[0x90] | b[0x91] << 8 | b[0x92] << 16);
            instance.moby_ambient = {b[0x3C] / 128.0f, b[0x3D] / 128.0f, b[0x3E] / 128.0f,
                                     (b[0x34] & 0x10) != 0 ? static_cast<float>(glow) + 1.0f : 0.0f};
        }
        // Drawn by the game with a camera of its own: placed so the world's camera sees it where
        // that camera did (M' = V_world^-1 V_own M), as ReRAC draws the menu's frame objects.
        for (const auto& [address, camera] : g->moby_cameras) {
            if (into_texture.contains(m.address)) {
                break;
            }
            if (address == m.address && instance.moby_light[3] == 1.0f) {
                // One of the 2D layer's: in a picture wider than the game's it is cut at the game's
                // picture's sides (mesh.vert), where those parked beside the screen stay unseen.
                instance.moby_light[3] = 2.0f;
            }
            if (address == m.address
                && (camera.camera_position != state.camera_position || camera.forward != state.forward)) {
                instance.matrix = renderer::multiply(
                    rigid_inverse(state.view()), renderer::multiply(camera.view(), instance.matrix)
                );
                break;
            }
        }
        const int joints = g->level.models[instance.model].joints;
        if (debug && g->index % 100 == 0 && m.class_id == 0) {
            // The hero, for bring-up: where he is and what his animation fields and pose say.
            const auto pose = viewer::moby_palette(ram, m.address);
            const std::uint8_t* b = ram.data() + m.address;
            float t = 0.0f;
            std::memcpy(&t, b + 0x54, 4);
            bool finite = !pose.empty();
            for (const auto& f : pose) {
                for (const auto& row : f) {
                    for (float x : row) {
                        finite = finite && std::isfinite(x) && std::fabs(x) < 1.0e6f;
                    }
                }
            }
            log::info("  hero at {:#x}: position {:.2f} {:.2f} {:.2f}, scale {:.3f}, keys {}:{} -> {}:{} t {:.3f}, "
                      "{} joints posed ({} in the mesh), {}, mode {:#x}",
                      m.address, m.matrix[12], m.matrix[13], m.matrix[14], m.matrix[0], b[0x52], b[0x50],
                      b[0x53], b[0x51], t, pose.size(), joints, finite ? "finite" : "NOT finite",
                      static_cast<unsigned>(b[0x34] | b[0x35] << 8));
        }
        if (animate && joints > 0) {
            const auto pose = viewer::moby_palette(ram, m.address);
            if (!pose.empty()) {
                instance.palette = static_cast<int>(palette.size());
                for (int j = 0; j < joints; ++j) {
                    // Columns: the images of the axes, then the translation, which the palette
                    // has in packed units (the mesh is in packed units / 1024).
                    viewer::LevelScene::JointColumns c{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
                    if (static_cast<std::size_t>(j) < pose.size()) {
                        const auto& f = pose[static_cast<std::size_t>(j)];
                        for (int i = 0; i < 3; ++i) {
                            c[4 * i + 0] = f[i][0];
                            c[4 * i + 1] = f[i][1];
                            c[4 * i + 2] = f[i][2];
                            c[4 * i + 3] = 0.0f;
                        }
                        c[12] = f[3][0] / 1024.0f;
                        c[13] = f[3][1] / 1024.0f;
                        c[14] = f[3][2] / 1024.0f;
                        c[15] = 1.0f;
                    }
                    palette.push_back(c);
                }
            }
        }
        if (const auto t = into_texture.find(m.address); t != into_texture.end()) {
            target_mobys.push_back({m.address, instance, *t->second});
            target_mobys.back().camera.focal_x = frame_focal[0];
            target_mobys.back().camera.focal_y = frame_focal[1];
            continue;
        }
        live_joints.push_back(instance.palette >= 0 ? joints : 0);
        live.push_back(instance);
        live_addresses.push_back(m.address);
    }
    g->moby_cameras.clear();
    g->scene.set_fog(state.fog_colour, state.fog_near, state.fog_far, state.fog_near_f,
                     state.fog_far_f);
    // The light bank as the game holds it now (16 sets of 0x40 bytes; the executable's, each
    // level's copy elsewhere): the level loader fills it from the gameplay file, and the game changes
    // sets at run time (the page menus write set 14 for their frame mobys, fog zones the hero's).
    if (light_bank_address() != 0) {
        const gaddr bank = openrac_relocate_data(light_bank_address());
        if (bank + 16 * 0x40 <= ram.size()) {
            std::vector<float> sets(16 * 16);
            std::memcpy(sets.data(), ram.data() + bank, sets.size() * sizeof(float));
            g->scene.set_light_sets(sets);
        }
    }
    // What this tick drew, kept for the frames drawn until the next one (between): the scene
    // then moves from the last tick's camera and mobys to this one's.
    Kept& k = g->kept;
    if (k.level != g->loaded) {
        k.has_state = false;
        k.live.clear();
        k.addresses.clear();
        k.joints.clear();
        k.level = g->loaded;
    }
    k.prev_state = k.state;
    k.has_prev_state = k.has_state;
    k.prev_matrices.clear();
    k.prev_poses.clear();
    for (std::size_t i = 0; i < k.live.size() && i < k.addresses.size(); ++i) {
        k.prev_matrices[k.addresses[i]] = k.live[i].matrix;
        if (i < k.joints.size() && k.joints[i] > 0 && k.live[i].palette >= 0) {
            k.prev_poses[k.addresses[i]] = {k.live[i].palette, k.joints[i]};
        }
    }
    k.prev_palette = std::move(k.palette);
    k.state = state;
    k.has_state = true;
    k.live = std::move(live);
    k.target_mobys = std::move(target_mobys);
    k.addresses = std::move(live_addresses);
    k.joints = std::move(live_joints);
    k.palette = std::move(palette);
    k.draws = draws;
    k.has_camera = state.forward[0] != 0.0f || state.forward[1] != 0.0f || state.forward[2] != 0.0f;
    g->last_tick_menu = k.before_world >= 0x40000;  // the last tick's, read before this one's is kept
    k.frame = g->index++;
    // OPENRAC_DUMP_DRAWS=N or N-M: the frames whose 2D draws are logged.
    static const char* dump_at = std::getenv("OPENRAC_DUMP_DRAWS");
    if (dump_at) {
        char* end = nullptr;
        const std::uint64_t first = std::strtoull(dump_at, &end, 10);
        const std::uint64_t last = *end == '-' ? std::strtoull(end + 1, nullptr, 10) : first;
        renderer::g_dump_draws = k.frame >= first && k.frame <= last;
    }
    if (renderer::g_dump_draws) {
        log::info("frame {}: the 2D path's draws", k.frame);
    }
    // The 2D path: the frame's direct GIF data, drawn by the direct renderer.
    (void)chain;
    std::vector<renderer::EffectVertex> strip_vertices;
    std::vector<renderer::EffectStrip> strips;
    std::size_t before_world = SIZE_MAX;
    std::size_t first_offscreen = SIZE_MAX;
    std::size_t world_marker = SIZE_MAX;
    std::vector<std::uint8_t> packets = direct_packets(ram, viewer::shown_chain(ram, a), &strip_vertices, &strips,
                                                       &g->effect_uploads, &before_world, &first_offscreen,
                                                       &world_marker);
    // The markers in a list index the quads of the tick that built it, and the list shown is the
    // one the last tick built (the game builds the next while this one is sent), one tick behind
    // the quads just recorded; FadeToBlack sends an old list again. Drawn with this tick's quads, a
    // marker named another quad whenever they changed, and the HUD (the bolts, the health, a
    // HelpDesk frame) and the menus flashed for a tick: the white flicker, and the palettes landing
    // on pixels that 8587dcfa held the last layer for. Each marker names its tick's generation
    // (ui_quad); the quads of the last four ticks that drew any are kept, and the list is drawn
    // with the ones it names.
    if (!g->ui_quads.empty()) {
        const std::uint32_t generation = g->ui_generation & 0x3F;
        g->ui_kept[generation & 3] = {generation, std::move(g->ui_quads)};
        ++g->ui_generation;
    }
    const std::vector<renderer::UiQuad>* named_quads = nullptr;
    if (const auto named = marker_generation(packets)) {
        for (const auto& [generation, quads] : g->ui_kept) {
            if (generation == *named && !quads.empty()) {
                named_quads = &quads;
            }
        }
        if (debug && !named_quads) {
            log::info("frame {}: the 2D list names the quads of generation {}, no longer kept; the last ones stand in",
                      g->index, *named);
        }
    }
    k.strip_vertices = std::move(strip_vertices);
    k.strips = std::move(strips);
    k.packets = std::move(packets);
    // Only a level's frame is ordered so: the boot program (title, main menu) starts VU1 programs
    // of its own before its 2D path, and draws its world natively, all of its 2D path after it.
    // A frame with mobys and no VU1 program in its chain (the vendor's menu: its frozen world, then
    // the vendor, which the port draws natively, then its screens rendered to textures and put on
    // the monitors): what comes before the first render to a texture is the picture under the mobys.
    // A level's frame whose terrain was drawn has the world's marker where it was (put by the
    // terrain pass, rac1/host/render_native.c): the split is there, or at a VU1 program started
    // before it. The first VU1 program alone was too fragile: the port's world renderers send
    // none, and the first is the world text's sprite pass (func_001F7B70), which runs only while
    // there is any. On Kerwan in play there was none: all of the 2D path, the HUD with it, was
    // taken as coming before the world and drawn under it. On Rilgar the split moved from frame
    // to frame, and the HUD and the sea's layers flickered.
    if (world_marker != SIZE_MAX) {
        before_world = std::min(before_world, world_marker);
    }
    if (before_world == SIZE_MAX && (k.draws & 0x10u) != 0) {
        before_world = first_offscreen;
    }
    k.before_world = level >= 0 && level < 19 ? std::min(before_world, k.packets.size()) : 0;
    const bool menu_gap = (k.draws & 0x1Fu) == 0 && k.before_world >= 0x40000;
    g->menu_gap_ticks = menu_gap ? g->menu_gap_ticks + 1 : 0;
    // A tick that draws its world with no 2D list at all (the first tick of the vendor's fly-in,
    // the first after a scene) is one whose list the game has not finished: drawn, it showed the
    // world unfaded between the black of a fade's ticks (the flicker on opening the vendor). The
    // last frame stands for it, as the console's screen did.
    const bool unfinished = k.has_camera && (k.draws & 0x1Fu) != 0 && k.packets.empty();
    g->repeat_frame = ((menu_gap && g->menu_gap_ticks <= 3) || unfinished) && g->frame.id() != 0;
    if (g->repeat_frame && debug) {
        log::info("frame {}: {}; the last frame stands", g->index,
                  unfinished ? "a world without its 2D list" : "no world under a page menu's list");
    }
    if (debug && k.frame % 100 == 0 && !k.strips.empty()) {
        int bare = 0;
        for (const auto& st : k.strips) {
            bare += st.upload_count == 0 ? 1 : 0;
        }
        log::info("  {} strips ({} vertices, {} without their texture's upload), the first TEX0 {:#x} ALPHA {:#x}",
                  k.strips.size(), k.strip_vertices.size(), bare, k.strips.front().tex0, k.strips.front().alpha);
    }
    sky_sprite_quads(state);
    particle_quads(ram, state);
    k.effects = std::move(g->effects);
    k.effect_uploads = std::move(g->effect_uploads);
    k.sky_shells = g->sky_shells;
    k.sky_shell_mask = std::exchange(g->sky_shell_mask, 0u);
    // The list's own tick's quads (above). The game's FadeToBlack (after a story card, between
    // scenes) sends the last display list again for each of its frames, with its black over it,
    // and draws nothing itself: its markers still name the tick that built it, which is kept.
    // Without those quads the card's two lines were left out and only the band behind them was
    // drawn, a blue strip across the picture for the fade's two frames. A list whose tick is no
    // longer kept keeps the quads last drawn; a list without markers does not look at them.
    if (named_quads) {
        k.ui_quads = *named_quads;
    }
    g->ui_quads.clear();
    g->effects.clear();
    g->effect_uploads.clear();
    g->particles.clear();
    g->sky_sprites.clear();
    // OPENRAC_FRAME_LOG=FROM-TO: what each tick in that range drew, in the log.
    static const char* frame_log = std::getenv("OPENRAC_FRAME_LOG");
    if (frame_log != nullptr) {
        char* end = nullptr;
        const std::uint64_t from = std::strtoull(frame_log, &end, 10);
        const std::uint64_t to = *end == '-' ? std::strtoull(end + 1, nullptr, 10) : from;
        if (k.frame >= from && k.frame <= to) {
            log::info("tick {}: draws {:#x} camera {} 2D bytes {} before world {} quads {} effects {} mobys {}", k.frame,
                      k.draws, k.has_camera, k.packets.size(), k.before_world, k.ui_quads.size(), k.effects.size(),
                      k.live.size());
        }
    }
    // [debug tools] What the game holds this tick, for the overlay and the dump.
    debug_overlay::tick({g->game.c_str(), ram, level, &k.state, k.draws, k.frame, k.packets.size(), k.ui_quads.size(),
                         k.effects.size(), k.strips.size(), k.live.size()});
    // The tick's own frame (when frames are drawn between ticks, part way from the last tick's
    // scene, by the refresh it is shown at). A tick that comes more than a tick late (set_tick: the
    // machine draws frames slower than the game ticks) is not drawn, so that drawing never slows
    // the game's time: the next tick's frame shows where the game got to. At most three in a row,
    // so that a picture still comes; a machine slower than that runs the game slow, as it must.
    constexpr int kMostSkipped = 3;
    // Said when it goes on (a quarter of a hundred ticks' frames left out), not for the odd tick
    // that comes late after the system held the program up.
    static int ticks = 0;
    static int left_out = 0;
    static int said = 0;
    if (++ticks >= 100) {
        if (left_out >= 25 && said++ < 3) {
            log::warn("a frame takes longer to draw ({:.1f} ms) than a game tick lasts ({:.1f} ms): {} of the last "
                      "{} ticks' frames were left out to keep the game's speed. A lower render scale or fewer "
                      "samples a pixel would help.",
                      pacing.cost * 1000.0, pacing.period.count() * 1000.0, left_out, ticks);
        }
        ticks = 0;
        left_out = 0;
    }
    if (pacing.late > pacing.period && g->skipped < kMostSkipped && g->kept.has_camera && g->index > 1) {
        ++g->skipped;
        ++left_out;
        return true;
    }
    g->skipped = 0;
    // The frame cap may hold the tick's own frame back: the tick is never made to wait for it. The
    // frame is then drawn between ticks, when it is due (between), or not at all when the next
    // tick comes first (a cap under the game's tick rate).
    if (!frame_due(std::chrono::steady_clock::now())) {
        pacing.first_pending = true;
        return true;
    }
    pacing.first_pending = false;
    draw_kept(true);
    return true;
}

void set_tick(std::chrono::steady_clock::time_point tick, std::chrono::duration<double> period,
              std::chrono::duration<double> late) {
    pacing.tick = tick;
    pacing.period = period;
    pacing.late = late;
}

int ui_quad(const openrac_game_ui_rect& quad) {
    // The 2D layer's quads are drawn natively (on the way off the GS packets); OPENRAC_UI_REPLAY
    // replays the game's packets instead, for comparing.
    static const bool native = std::getenv("OPENRAC_UI_REPLAY") == nullptr;
    if (!g || !native || g->ui_quads.size() >= 0xFFFFFFu) {  // the marker's index is 24 bits
        return -1;
    }
    renderer::UiQuad q;
    for (int i = 0; i < 4; ++i) {
        q.x[i] = quad.x[i];
        q.y[i] = quad.y[i];
        q.u[i] = quad.u[i];
        q.v[i] = quad.v[i];
    }
    q.z = quad.z;
    q.rgba = quad.rgba;
    q.tex0 = quad.tex0;
    q.flags = quad.flags;
    g->ui_quads.push_back(q);
    return static_cast<int>((g->ui_quads.size() - 1) | (g->ui_generation & 0x3F) << 24);
}

bool interpolating() {
    return !one_frame_a_tick() && settings::get().frame_rate != settings::FrameRate::Original && g && g->window;
}

std::uint32_t text(int id) {
    return options_menu::text(id);
}

bool wants_between() {
    return interpolating() && g->kept.has_state && g->kept.has_camera;
}

int between(std::chrono::steady_clock::time_point next_tick) {
    if (!g) {
        return 0;
    }
    // A frame only for a refresh the kept tick is still shown at (the next tick's, with the lag).
    const auto lag = std::chrono::duration_cast<std::chrono::steady_clock::duration>(pacing.period * kLagTicks);
    auto now = std::chrono::steady_clock::now();
    // The frame cap: a frame not due before the next tick is left to after it; one due sooner is
    // waited for here.
    if (!frame_due(now)) {
        if (pacing.due >= next_tick) {
            return 0;
        }
        std::this_thread::sleep_until(pacing.due);
        platform::work_resumed();
        now = std::chrono::steady_clock::now();
    }
    const auto display = next_display(now);
    if (display >= next_tick + lag) {
        return 0;
    }
    // Nothing waits for the display: a frame that would not be done by the next tick's time is not
    // started, so that the tick comes on time whatever a frame costs (with the swap's wait the
    // frame belongs to a refresh, and the tick after it is a little late and made up for).
    if (g->swap == platform::SwapInterval::Immediate
        && now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(pacing.cost))
               >= next_tick) {
        return 0;
    }
    // The scene has arrived at the kept tick's: another frame would be the same picture again.
    if (!pacing.first_pending && pacing.last_alpha >= 1.0f && alpha_at(display) >= 1.0f) {
        return 0;
    }
    SDL_Event event;
    while (platform::poll_event(event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return -1;
        }
        if (window_key(event) || debug_overlay::event(event)
            || debug_menu::event(event, openrac_game_loaded_overlay())) {
            continue;
        }
        g->input.handle_event(event);
    }
    const bool dump = renderer::g_dump_draws;
    renderer::g_dump_draws = false;
    const bool first = pacing.first_pending;
    pacing.first_pending = false;
    draw_kept(first);
    renderer::g_dump_draws = dump;
    return 1;
}

// [debug tools] While the overlay holds the game (pause): the last tick drawn again, the window's
// events read, until it lets a tick through. False once the window was closed.
bool hold() {
    while (g && g->window && debug_overlay::holding()) {
        SDL_Event event;
        while (platform::poll_event(event)) {
            if (event.type == SDL_EVENT_QUIT) {
                return false;
            }
            if (window_key(event) || debug_overlay::event(event)) {
                continue;
            }
            g->input.handle_event(event);
        }
        if (debug_overlay::take_settings_changed()) {
            apply_settings();  // the drawer is open over the paused game
        }
        const auto started = std::chrono::steady_clock::now();
        const bool dump = renderer::g_dump_draws;
        renderer::g_dump_draws = false;
        g_held = true;
        draw_kept(false);
        g_held = false;
        renderer::g_dump_draws = dump;
        // No faster than the display shows them (a 120 Hz screen's drawer moves at 120), and no
        // faster than 60 a second where its rate is not known.
        const double interval = pacing.interval > 0.0 ? pacing.interval : 1.0 / 60.0;
        std::this_thread::sleep_until(
            started + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(interval * 0.9)));
    }
    return true;
}

bool pad(int port, std::uint16_t* buttons, std::uint8_t analog[4]) {
    if (!g || port < 0 || port >= platform::kPadPorts) {
        return false;
    }
    const platform::Pad& p = g->input.pad(port);
    *buttons = p.buttons;
    analog[0] = p.right_x;
    analog[1] = p.right_y;
    analog[2] = p.left_x;
    analog[3] = p.left_y;
    // OPENRAC_PRESS=FRAME:MASK:FRAMES[:LX:LY[:RX:RY]],... holds buttons (mask bits as the pad reports
    // them, 0x4000 cross, 0x8 start) and optionally the left and right sticks (0-255, 128 centred; 0
    // is left or up) for scripted checks. FRAME counts the window's frames (world, 2D and movie frames).
    if (const char* script = port == 0 ? std::getenv("OPENRAC_PRESS") : nullptr) {
        for (const char* at = script; at && *at;) {
            unsigned long frame = 0, mask = 0, length = 5;
            unsigned int lx = 128, ly = 128, rx = 128, ry = 128;
            const int read = std::sscanf(at, "%lu:%lx:%lu:%u:%u:%u:%u", &frame, &mask, &length, &lx, &ly, &rx, &ry);
            if (read >= 2 && g->index >= frame && g->index < frame + length) {
                *buttons = static_cast<std::uint16_t>(*buttons & ~mask);
                if (read >= 5) {
                    analog[2] = static_cast<std::uint8_t>(lx);
                    analog[3] = static_cast<std::uint8_t>(ly);
                }
                if (read >= 7) {
                    analog[0] = static_cast<std::uint8_t>(rx);
                    analog[1] = static_cast<std::uint8_t>(ry);
                }
            }
            at = std::strchr(at, ',');
            at = at ? at + 1 : nullptr;
        }
    }
    // Start skips an in-game cutscene: the game skips one on Start with L1, L2, R1 and R2 held
    // (Start alone only once it is beaten; ReRAC cutscenes_transitions.md, "skip"), so while one
    // plays a pressed Start also presses the four shoulders. Bits are active low: 0x8 Start,
    // 0x0F00 L2 R2 L1 R1.
    if (port == 0 && settings::get().skip_cutscenes && (*buttons & 0x8) == 0) {
        if (openrac_game_in_cutscene != nullptr && openrac_game_in_cutscene() != 0) {
            *buttons = static_cast<std::uint16_t>(*buttons & ~0x0F00);
        }
        if (openrac_game_skip != nullptr) {
            openrac_game_skip();
        }
    }
    if (port == 0) {
        debug_menu::filter_pad(*buttons, openrac_game_loaded_overlay());
        debug_overlay::pad(*buttons, analog);  // [debug tools]
    }
    if (std::getenv("OPENRAC_TRACE_INPUT") != nullptr) {
        static std::uint64_t last_frame[platform::kPadPorts]{};
        if (g->index >= last_frame[port] + 25) {
            last_frame[port] = g->index;
            log::info("input frame {} port {}: buttons {:#x}, right {} {}, left {} {}",
                      g->index, port, *buttons, analog[0], analog[1], analog[2], analog[3]);
        }
    }
    return true;
}

void close() {
    if (g) {
        debug_overlay::release();
        settings::save();  // the window's size, if it was changed
        fps_counter::release();
        touch_overlay::release();
        debug_menu::release();
        g->frame.release();
        g->picture.release();
        g->last_world.release();
        g->renderer.reset();
        g->scene.release();
        g.reset();
    }
}

}  // namespace openrac::frontend

