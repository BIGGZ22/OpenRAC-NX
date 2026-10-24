// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "frontend.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>

#include "common/env.h"
#include "common/log.h"
#include "common/stats.h"
#include "debug_menu.h"
#include "debug_overlay.h"
#include "enhanced/game/scripted_moby_colour.h"
#include "enhanced/viewer/hero_shadow.h"
#include "fps_counter.h"
#include "frame_pacing.h"
#include "game/common/frontend/geometry.h"
#include "game/common/frontend/pacing.h"
#include "game/common/frontend/state.h"
#include "game/common/particle_texture_cache.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "options_menu.h"
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
#include "renderer/shadows.h"
#include "settings.h"
#include "touch_overlay.h"
#include "viewer/game_state.h"
#include "viewer/level.h"
#include "viewer/level_renderer.h"
#include "viewer/moby_pose.h"
#include "viewer/moby_shadow.h"
#include "viewer/screenshot.h"

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

// The frontend's state (frontend/state.h).
std::unique_ptr<State> g;

// When frames reach the display (frontend/state.h).
Pacing pacing;

// Returns the value `var` was set to as a string of its own, or nothing when it is unset. For the
// reads below that hand the value to the C library (a file to open, a script for sscanf), which
// wants a string that ends in a NUL: env::value's view need not.
std::optional<std::string> env_string(env::Var var) {
    const std::optional<std::string_view> found = env::value(var);

    // An unset variable stays nothing, which differs from the empty string.
    if (!found) {
        return std::nullopt;
    }

    return std::string(*found);
}

// Returns the frame number `var` names, read as std::strtoull reads it, or nothing when it is
// unset. The frame numbers (OPENRAC_DUMP_RAM, OPENRAC_FX_DUMP, OPENRAC_SHOT_FROM) have always been
// read so: env::integer reads as std::strtoll, which differs for a negative value past -2^63
// (strtoull wraps it to a frame number, strtoll holds it to -2^63).
std::optional<std::uint64_t> env_frame(env::Var var) {
    const std::optional<std::string> text = env_string(var);
    if (!text) {
        return std::nullopt;
    }
    return std::strtoull(text->c_str(), nullptr, 10);
}

namespace {

// A range of frames, both ends included.
struct FrameRange {
    std::uint64_t first = 0;
    std::uint64_t last = 0;

    // Returns whether `frame` is in the range.
    [[nodiscard]] bool holds(std::uint64_t frame) const { return frame >= first && frame <= last; }
};

// Returns the range `var` names, N or N-M, read as std::strtoull reads each end; nothing when it is
// unset. For OPENRAC_DUMP_DRAWS and OPENRAC_FRAME_LOG.
std::optional<FrameRange> env_frames(env::Var var) {
    const std::optional<std::string> text = env_string(var);
    if (!text) {
        return std::nullopt;
    }

    // N alone is the range of one frame.
    char* end = nullptr;
    FrameRange range;
    range.first = std::strtoull(text->c_str(), &end, 10);
    range.last = *end == '-' ? std::strtoull(end + 1, nullptr, 10) : range.first;
    return range;
}

}  // namespace

// Returns OPENRAC_SHOT read, once. EVERY is read as std::atoll read it (std::strtoll), and a
// negative one wraps to a large period, as it always did.
const Shots& shots() {
    static const Shots read = [] {
        Shots result;
        const std::optional<std::string_view> value = env::value(env::Var::Shot);

        // Without a ':' there is no path to write to, and no shot.
        const std::size_t colon = value ? value->find(':') : std::string_view::npos;
        if (colon != std::string_view::npos) {
            result.every = static_cast<std::uint64_t>(env::read_integer(value, 10, 0));
            result.prefix = std::string(value->substr(colon + 1));
        }
        return result;
    }();
    return read;
}

// Where a scripted run is (frontend/state.h).
Route g_route;

// Whether Start would skip what is playing now: a level's in-game scene (mode 2), or the flight
// between planets (the boot program's mode 6).
bool skippable() {
    return (g_route.level >= 0 && g_route.mode == 2) || (g_route.level < 0 && g_route.mode == 6);
}

namespace {

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

    if (const std::optional<std::string> dump = env_string(env::Var::DumpVif)) {
        if (std::FILE* f = std::fopen(dump->c_str(), "wb")) {
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

}  // namespace

// The picture's shape now: the player's setting, for the window as it is.
float picture_aspect(int window_width, int window_height) {
    return settings::aspect_ratio(settings::get(), window_width, window_height);
}

// The development switches that keep one frame a tick whatever the settings say.
bool one_frame_a_tick() {
    static const bool on = env::flag(env::Var::Uncapped) || env::flag(env::Var::NoBlend);
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

namespace {

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

// The game mode word, as the game's front end names it (rac1: port/game/rac1/host/front_end.c); 6
// in the boot program is the flight between planets. 0 for a game without one.
std::uint32_t game_mode_address() {
    return openrac_game_front_end != nullptr ? openrac_game_front_end->game_mode : 0;
}

}  // namespace

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
    const std::optional<std::string> home = env::system_variable("HOME");
    if (!home || home->empty()) {
        return;
    }
    const std::filesystem::path home_path = *home;
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
    config.software = env::flag(env::Var::SoftwareGl) ? platform::SoftwareRenderer::Allow
                                                      : platform::SoftwareRenderer::Refuse;
    // OPENRAC_HIDDEN=1: the window is made and drawn as always but never shown, for scripted runs
    // that should not open windows on the player's screen (several can run at once).
    config.hidden = env::flag(env::Var::Hidden);
    g->window = platform::Window::open(config, error);
    if (!g->window) {
        return false;
    }
    platform::install_app_menus(env::flag(env::Var::Developer));
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
    apply_mouse_capture();
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
    if (shots().due(index)) {
        const std::string path = shots().prefix + std::to_string(index) + ".png";
        viewer::write_png(path, width, height, pixels);
        log::info("frame {} written to {}", index, path);
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

namespace {

// Reads the window's events: the game's input from them, the window's own keys, the overlay's
// and the debug menu's, then the settings their menus changed. False once the window was closed.
bool read_window_events() {
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
    apply_mouse_capture();
    g->input.update();
    // The port's rows on the game's Video options page (options_menu.h), and the overlay's drawer
    // and menus: a changed one is applied. Before the debug menu, which adds its entry to the
    // Options list options_menu gives the game.
    const bool changed = options_menu::update(openrac_game_loaded_overlay());
    if (debug_overlay::take_settings_changed() || changed) {
        apply_settings();
    }
    debug_menu::update(openrac_game_loaded_overlay(), g->index);
    return true;
}

// Uses the world the game draws now: the level loaded (`level`, -1 the boot program), or one of
// the boot program's worlds.
void use_world(std::span<const std::uint8_t> ram, int level) {
    // The boot program's worlds: the flight between planets (game mode 6), else the title world.
    // A flight between planets also plays inside a planet's program. The game draws it by the ship's
    // state (dispatch_game_state_update): the planet in the take-off (0), the landing (8) and while
    // the ship leaves along its path (3, 7); space only in state 4 (DrawSpaceFlightFrame), whose
    // world is the flight world's. The mode there is the level's own copy of the word
    // (openrac_game_mode), not the boot program's.
    int world = level >= 0 && level < 19 ? level : word_at(ram, game_mode_address()) == 6 ? -2 : -1;
    std::uint32_t level_mode = 0xFFFFFFFFu;
    if (world >= 0 && openrac_game_front_end != nullptr && openrac_game_front_end->ship_state != 0
        && openrac_game_mode != nullptr && openrac_game_mode(&level_mode) != 0 && level_mode == 6) {
        const std::uint32_t ship = word_at(ram, openrac_game_front_end->ship_state);
        if (ship == 4) {
            world = -2;
        }
    }
    use_level(world);
}

// Puts the camera the game's world renderers drew with this tick into `state`, if they ran.
void take_world_camera(viewer::GameState& state) {
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
}

// Logs the camera this tick is drawn from (OPENRAC_CAM_LOG). `world_ran`: the game's world
// renderers ran this tick.
void log_camera(const viewer::GameState& state, bool world_ran) {
    // OPENRAC_CAM_LOG: the camera each frame is drawn from (position, forward, up), in the log.
    static const bool& cam_log = stats::probe("OPENRAC_CAM_LOG", "the camera each tick is drawn from, in the log");
    if (cam_log) {
        log::info("cam {} {:.3f} {:.3f} {:.3f} f {:.4f} {:.4f} {:.4f} u {:.4f} {:.4f} {:.4f} world {}", g->index,
                  state.camera_position[0], state.camera_position[1], state.camera_position[2], state.forward[0],
                  state.forward[1], state.forward[2], state.up[0], state.up[1], state.up[2], world_ran ? 1 : 0);
    }
}

// Reads where a scripted run is (g_route): the level, the game mode and when the player got
// control.
void follow_route(std::span<const std::uint8_t> ram, int level) {
    // The game mode (the loaded level's copy, or the boot program's word) and control (Route).
    std::uint32_t mode = 0;
    const bool in_level = level >= 0 && level < 19;
    if (!(in_level && openrac_game_mode != nullptr && openrac_game_mode(&mode) != 0)) {
        mode = game_mode_address() != 0 ? word_at(ram, game_mode_address()) : 0;
    }
    g_route.level = in_level ? level : -1;
    g_route.mode = static_cast<int>(mode);
    if (!skippable()) {
        g_route.skippable_from = -1;
    } else if (g_route.skippable_from < 0) {
        g_route.skippable_from = static_cast<std::int64_t>(g->index);
    }
    if (g_route.control < 0) {
        g_route.streak = in_level && mode == 0 ? g_route.streak + 1 : 0;
        if (g_route.streak >= kControlHold) {
            g_route.control = static_cast<std::int64_t>(g->index);
            log::info("control at frame {} (level {})", g->index, level);
        }
    }
}

// Writes this tick's line of OPENRAC_TRACE. `draws`: the layers the game drew (frame()).
void trace_tick(const viewer::GameState& state, std::uint32_t draws) {
    // OPENRAC_TRACE=FILE: one JSON line a tick, for port/tools/regress (docs/port/REGRESSION.md):
    // f the window frame, c frames since control (null before), lv the level (-1 the boot
    // program), m the game mode, cam/fw the camera's position and forward row, hp/hr/hs the hero's
    // position, rotation and state, mb the live mobys, dr the layers the game drew.
    static std::FILE* trace = [] {
        const std::optional<std::string> path = env_string(env::Var::Trace);
        std::FILE* f = path && !path->empty() ? std::fopen(path->c_str(), "w") : nullptr;
        if (path && f == nullptr) {
            log::warn("OPENRAC_TRACE: cannot write {}", *path);
        }
        return f;
    }();
    if (trace != nullptr) {
        const auto num = [](float v) { return std::isfinite(v) ? std::format("{:.4f}", v) : std::string("null"); };
        const auto vec = [&](const float* v) { return std::format("[{},{},{}]", num(v[0]), num(v[1]), num(v[2])); };
        std::string line = std::format("{{\"f\":{},\"c\":{},\"lv\":{},\"m\":{},\"cam\":{},\"fw\":{}", g->index,
                                       g_route.control < 0 ? std::string("null")
                                                           : std::to_string(static_cast<std::int64_t>(g->index) - g_route.control),
                                       g_route.level, g_route.mode, vec(state.camera_position.data()),
                                       vec(state.forward.data()));
        float hp[3]{}, hr[3]{};
        std::int32_t hs = 0;
        if (openrac_game_hero != nullptr && openrac_game_hero(hp, hr, &hs) != 0) {
            line += std::format(",\"hp\":{},\"hr\":{},\"hs\":{}", vec(hp), vec(hr), hs);
        }
        line += std::format(",\"mb\":{},\"dr\":{}}}\n", state.mobys.size(), draws);
        std::fputs(line.c_str(), trace);
        std::fflush(trace);  // a crash keeps every tick before it
    }
}

// Whether a scripted run is over: OPENRAC_CONTROL_FRAMES frames after control.
bool run_over() {
    // OPENRAC_CONTROL_FRAMES=N: the run ends N frames after control (a route's length).
    static const std::int64_t control_frames = env::integer(env::Var::ControlFrames, -1);
    if (control_frames >= 0 && g_route.control >= 0
        && static_cast<std::int64_t>(g->index) >= g_route.control + control_frames) {
        log::info("{} frames after control: the run ends", control_frames);
        return true;
    }
    return false;
}

// Writes the game's main memory at the frame OPENRAC_DUMP_RAM names.
void dump_ram(std::span<const std::uint8_t> ram) {
    // OPENRAC_DUMP_RAM=FRAME: the game's 32 MB of main memory at that frame, to ram_FRAME.bin.
    static const std::optional<std::uint64_t> dump_ram = env_frame(env::Var::DumpRam);
    if (dump_ram && g->index == *dump_ram) {
        const std::string path = std::format("ram_{}.bin", g->index);
        if (std::FILE* f = std::fopen(path.c_str(), "wb")) {
            std::fwrite(ram.data(), 1, ram.size(), f);
            std::fclose(f);
            log::info("main memory written to {}", path);
        }
    }
}

// Logs the game's state for bring-up (OPENRAC_DEBUG, each 100 ticks): the level, the camera, the
// front end's words and the pad, the fog, the effects, the sky and the particles drawn.
void log_tick_state(
    std::span<const std::uint8_t> ram,
    const viewer::GameState& state,
    int level,
    std::uint32_t draws
) {
    log::info(
        "frame {}: level {}, camera {:.1f} {:.1f} {:.1f}, forward {:.2f} {:.2f} {:.2f}, "
        "fov {:.3f}, {} mobys, layers drawn {:#x}",
        g->index,
        level,
        state.camera_position[0],
        state.camera_position[1],
        state.camera_position[2],
        state.forward[0],
        state.forward[1],
        state.forward[2],
        state.tan_half_fov_y,
        state.mobys.size(),
        draws
    );
    if (const openrac_front_end* fe = openrac_game_front_end) {
        // The first pad's block: +0x100 the sticks, +0x194 its socket, +0x198 its state,
        // +0x1A0 the buttons held, +0x1A4 the raw word.
        // The level's own game mode (openrac_game_mode: rac1's level copy of its mode word, 6 in
        // the ship's landing, 2 in a cutscene, 0 in play), or -1 outside a level.
        std::uint32_t level_mode = 0xFFFFFFFFu;
        if (openrac_game_mode == nullptr || openrac_game_mode(&level_mode) == 0) {
            level_mode = 0xFFFFFFFFu;
        }
        log::info(
            "  mode {} level mode {} dialog kind {} step {} level word {} title exit {} pad {:#x}",
            static_cast<int>(word_at(ram, fe->game_mode)),
            static_cast<int>(level_mode),
            static_cast<int>(word_at(ram, fe->dialog)),
            static_cast<int>(word_at(ram, fe->dialog + 0x1C)),
            static_cast<int>(word_at(ram, fe->level)),
            static_cast<int>(word_at(ram, fe->title_exit)),
            word_at(ram, fe->pad + 0x1A4)
        );
        float axes[4]{};
        std::memcpy(axes, ram.data() + fe->pad + 0x100, sizeof axes);
        log::info(
            "  pad socket {} state {} held {:#x}, right {:.3f} {:.3f}, left {:.3f} {:.3f}",
            word_at(ram, fe->pad + 0x194),
            word_at(ram, fe->pad + 0x198),
            word_at(ram, fe->pad + 0x1A0),
            axes[0],
            axes[1],
            axes[2],
            axes[3]
        );
    }
    log::info(
        "  fog colour {:.0f} {:.0f} {:.0f}, depth {:.0f} to {:.0f}, F {:.0f} to {:.0f}",
        state.fog_colour[0] * 255,
        state.fog_colour[1] * 255,
        state.fog_colour[2] * 255,
        state.fog_near,
        state.fog_far,
        state.fog_near_f,
        state.fog_far_f
    );
    static const std::optional<std::uint64_t> fx_dump = env_frame(env::Var::FxDump);
    if (fx_dump && g->index >= *fx_dump && g->index < *fx_dump + 100) {
        for (const renderer::EffectQuad& e : g->effects) {
            log::info(
                "    quad {:.1f} {:.1f} {:.1f} {:.2f} | {:.1f} {:.1f} {:.1f} | "
                "{:.1f} {:.1f} {:.1f} | {:.1f} {:.1f} {:.1f} rgba {:#x} "
                "st {:.2f},{:.2f} {:.2f},{:.2f} tex0 {:#x} alpha {:#x} clamp {:#x}",
                e.corner[0][0],
                e.corner[0][1],
                e.corner[0][2],
                e.corner[0][3],
                e.corner[1][0],
                e.corner[1][1],
                e.corner[1][2],
                e.corner[2][0],
                e.corner[2][1],
                e.corner[2][2],
                e.corner[3][0],
                e.corner[3][1],
                e.corner[3][2],
                e.rgba[0],
                e.st[0][0],
                e.st[0][1],
                e.st[3][0],
                e.st[3][1],
                e.tex0,
                e.alpha,
                e.clamp
            );
        }
    }
    if (!g->sky_sprites.empty()) {
        const auto& [r, tex0] = g->sky_sprites.front();
        float size;
        std::memcpy(&size, r.data() + 0x1C, 4);
        log::info(
            "  {} sky sprites; the first RGBA {:#x} size {:.3f} TEX0 {:#x} ALPHA {:#x}",
            g->sky_sprites.size(),
            r[4] | r[5] << 8 | r[6] << 16 | static_cast<std::uint32_t>(r[7]) << 24,
            size,
            tex0,
            r[3]
        );
    }
    if (!g->particles.empty()) {
        int kinds[4] = {0, 0, 0, 0};
        for (const State::Particle& p : g->particles) {
            kinds[p.record[1] & 3]++;
        }
        log::info(
            "  {} particles (sprites {}, flat {}, lines {}, ribbons {})",
            g->particles.size(),
            kinds[0],
            kinds[1],
            kinds[2],
            kinds[3]
        );
    }
    if (!g->effects.empty()) {
        const renderer::EffectQuad& e = g->effects.front();
        log::info(
            "  {} effect quads; the first at {:.1f} {:.1f} {:.1f}, "
            "TEX0 {:#x}, ALPHA {:#x}, RGBA {:#x}",
            g->effects.size(),
            e.corner[0][0],
            e.corner[0][1],
            e.corner[0][2],
            e.tex0,
            e.alpha,
            e.rgba[0]
        );
    }
    log::info(
        "  load stage {:#x} retries {} snd state {:#x}",
        word_at(ram, 0x0015EF48),
        word_at(ram, 0x0015EFBC),
        word_at(ram, word_at(ram, 0x001517D0) + 8) & 0xFFFF
    );
}

// Logs the mobys near the hero (OPENRAC_MOBY_LOG).
void log_mobys_near_hero(std::span<const std::uint8_t> ram, const viewer::GameState& state) {
    // OPENRAC_MOBY_LOG=N[:RADIUS]: every N ticks, the mobys within RADIUS game units of the hero
    // (default 20) in the log: class, address, state, mode bits, animation, position, the distance,
    // the collision radius and the update function, for seeing what an object is doing.
    static const std::optional<std::string_view> moby_log = env::value(env::Var::MobyLog);
    // N read as std::atoi read it, held to int's range, where atoi's answer was undefined.
    static const std::int64_t moby_every =
        std::min<std::int64_t>(env::read_integer(moby_log, 10, 0), std::numeric_limits<int>::max());
    if (moby_every > 0 && g->index % static_cast<std::uint64_t>(moby_every) == 0) {
        const std::size_t colon = moby_log->find(':');
        const float radius =
            colon != std::string_view::npos
                ? static_cast<float>(env::read_number(moby_log->substr(colon + 1), 0.0))
                : 20.0f;
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
}

// The live mobys of one tick, as pose_mobys reads them for keep_tick: each one drawn and its
// address and joints, the joint palette they index, the hero's shadow, and the mobys the game
// drew into a texture.
struct TickMobys {
    std::vector<viewer::Instance> live;
    std::vector<std::uint32_t> addresses;
    std::vector<int> joints;
    std::vector<viewer::LevelScene::JointColumns> palette;
    std::vector<renderer::ShadowCapsule> shadows;
    std::uint32_t shadow_moby = 0;
    viewer::MobyShadowState shadow_state;
    std::vector<Kept::TargetMoby> target_mobys;
};

// Reads the live mobys this tick: which are drawn, how (blend, light, colour, the camera they
// were drawn with) and in what pose, and the hero's shadow. Mobys the game drew but did not list
// are added to `state`. `world_ran`, `only_own_camera`: the world renderers ran this tick, and
// only mobys with a camera of their own were drawn without them; `debug`: OPENRAC_DEBUG.
TickMobys pose_mobys(
    std::span<const std::uint8_t> ram,
    viewer::GameState& state,
    bool world_ran,
    bool only_own_camera,
    bool debug
) {
    // The live mobys, by class, each in the pose its animation fields give (viewer/moby_pose.h):
    // its joint palette goes to the scene, which skins the class's mesh with it. OPENRAC_ANIM=0
    // draws every moby in its bind pose.
    static const bool animate = env::flag(env::Var::Anim);
    std::vector<viewer::Instance> live;
    std::vector<std::uint32_t> live_addresses;
    std::vector<int> live_joints;
    std::vector<viewer::LevelScene::JointColumns> palette;
    std::vector<renderer::ShadowCapsule> shadows;
    std::uint32_t shadow_moby = 0;
    viewer::MobyShadowState shadow_state;
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
        // Beyond its draw distance, or wholly behind the eye: MobyProc does not draw it
        // (viewer::moby_beyond_draw_distance). Pokitaru places two mobys of class 1161 with a turn of -5 pi,
        // past the reach of the game's sine; their rows come out thousands long and their spheres
        // thousands of units away, so the game never draws them, while drawn here they covered the
        // sky in cyan. A moby the game draws with a camera of its own (the menus', the vendor's) is
        // not the world camera's to cull.
        if (!extra && world_ran && m.address + 0x34 <= ram.size()
            && std::none_of(g->moby_cameras.begin(), g->moby_cameras.end(),
                            [&](const auto& c) { return c.first == m.address; })
            && viewer::moby_beyond_draw_distance(ram.data() + m.address, state.camera_position, state.forward)) {
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
        instance.occlusion = m.occlusion;
        if (extra || into_texture.contains(m.address)) {
            instance.occlusion = viewer::Pvs::always;
        }
        // Authored bounds do not cover debug stand-ins or replacement meshes.
        if (!cls->second.second && g->level.overridden_models == 0) {
            instance.world_sphere = m.world_sphere;
        }
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
        // Preserve explicit gameplay colour writes while that RGB is still
        // current. Countdown crates restore their saved colour between flashes;
        // once restored they automatically resume normal GI. No class whitelist.
        if (m.address + 0x3f <= ram.size()) {
            const auto* b = ram.data() + m.address;
            const unsigned colour = b[0x3c] | b[0x3d] << 8 | b[0x3e] << 16;
            if (g->scripted_moby_colours.matches(g->loaded, m.address, colour)) {
                instance.moby_light[3] = -1.0f;
            }
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
            if (address == m.address) {
                instance.world_sphere = {};  // this object belongs to another camera's space
                instance.occlusion = viewer::Pvs::always;
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
        instance.hero_shadow_caster = m.class_id == 0 && !extra && g->loaded >= 0 && g->loaded < 19
                                      && instance.moby_light[3] < 1.5f && instance.tint[3] > 0;
        live_joints.push_back(instance.palette >= 0 ? joints : 0);
        if (shadow_directions_address() != 0 && g->loaded >= 0 && g->loaded < 19 && animate
            && m.class_id == 0 && !extra && instance.moby_light[3] < 1.5f && instance.tint[3] > 0.0f) {
            const auto directions = OPENRAC_DATA(shadow_directions_address());
            shadow_state = viewer::moby_shadow_state(ram, m.address, directions, state.view());
            if (shadow_state.enabled) {
                shadows = viewer::moby_shadows(ram, m.address, m.matrix, shadow_state.scale);
                shadow_moby = m.address;
            }
            if (debug && g->index%100 == 0) {
                log::info("Ratchet shadow: {} proxies, slot {}, table {:#x}, extrusion {:.2f} {:.2f} {:.2f}, slab {:.3f}..{:.3f}, scale {:.3f}",
                          shadows.size(), shadow_state.direction_index, directions,
                          shadow_state.extrusion[0], shadow_state.extrusion[1], shadow_state.extrusion[2],
                          shadow_state.heights[0], shadow_state.heights[1], shadow_state.scale);
            }
        }
        live.push_back(instance);
        live_addresses.push_back(m.address);
    }
    g->moby_cameras.clear();
    return {
        std::move(live),
        std::move(live_addresses),
        std::move(live_joints),
        std::move(palette),
        std::move(shadows),
        shadow_moby,
        shadow_state,
        std::move(target_mobys)
    };
}

// Gives the scene this tick's fog and the lights as the game holds them.
void light_scene(std::span<const std::uint8_t> ram, const viewer::GameState& state) {
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
            if (bank + 16 * 0x40 + 8 * 0x20 <= ram.size()) {
                std::array<float, 64> points{};
                std::memcpy(points.data(), ram.data() + bank + 16 * 0x40, sizeof(points));
                g->scene.set_point_lights(points);
            }
        }
    }
}

// Keeps this tick's scene for the frames drawn until the next one (Kept), the last tick's as
// where they move from, and counts the tick.
void keep_tick(
    std::span<const std::uint8_t> ram,
    const viewer::GameState& state,
    std::uint32_t draws,
    TickMobys&& mobys
) {
    // What this tick drew, kept for the frames drawn until the next one (between): the scene
    // then moves from the last tick's camera and mobys to this one's.
    Kept& k = g->kept;
    if (k.level != g->loaded) {
        k.has_state = false;
        k.live.clear();
        k.addresses.clear();
        k.joints.clear();
        k.shadows.clear();
        k.shadow_moby = 0;
        k.level = g->loaded;
    }
    k.prev_state = k.state;
    k.has_prev_state = k.has_state;
    k.prev_matrices.clear();
    k.prev_spheres.clear();
    k.prev_poses.clear();
    for (std::size_t i = 0; i < k.live.size() && i < k.addresses.size(); ++i) {
        k.prev_matrices[k.addresses[i]] = k.live[i].matrix;
        k.prev_spheres[k.addresses[i]] = k.live[i].world_sphere;
        if (i < k.joints.size() && k.joints[i] > 0 && k.live[i].palette >= 0) {
            k.prev_poses[k.addresses[i]] = {k.live[i].palette, k.joints[i]};
        }
    }
    k.prev_palette = std::move(k.palette);
    k.prev_shadows = std::move(k.shadows);
    k.prev_shadow_moby = k.shadow_moby;
    k.shadows = std::move(mobys.shadows);
    k.shadow_moby = mobys.shadow_moby;
    k.prev_shadow_state = k.shadow_state;
    k.shadow_state = mobys.shadow_state;
    k.hero_airborne = hero_state_address() != 0
                          ? viewer::hero_airborne(ram, OPENRAC_DATA(hero_state_address()))
                          : std::nullopt;
    k.state = state;
    k.has_state = true;
    k.live = std::move(mobys.live);
    k.target_mobys = std::move(mobys.target_mobys);
    k.addresses = std::move(mobys.addresses);
    k.joints = std::move(mobys.joints);
    k.palette = std::move(mobys.palette);
    k.draws = draws;
    k.has_camera = state.forward[0] != 0.0f || state.forward[1] != 0.0f || state.forward[2] != 0.0f;
    g->last_tick_menu = k.before_world >= 0x40000;  // the last tick's, read before this one's is kept
    k.frame = g->index++;
}

// Keeps this tick's 2D path (the display list's direct GIF data, from `chain` where the game
// sends it so, else the list `a` names) and where its world goes in it. Returns the native quads
// its markers name, if they are still kept.
const std::vector<renderer::UiQuad>* keep_2d_path(
    std::span<const std::uint8_t> ram,
    std::uint32_t chain,
    const viewer::GameAddresses& a,
    int level,
    bool debug
) {
    Kept& k = g->kept;
    // OPENRAC_DUMP_DRAWS=N or N-M: the frames whose 2D draws are logged.
    static const std::optional<FrameRange> dump_at = env_frames(env::Var::DumpDraws);
    if (dump_at) {
        renderer::g_dump_draws = dump_at->holds(k.frame);
    }
    if (renderer::g_dump_draws) {
        log::info("frame {}: the 2D path's draws", k.frame);
    }
    // The 2D path: the frame's direct GIF data, drawn by the direct renderer.
    // RAC1 keeps two alternating list buffers and the sent list is one tick behind; Deadlocked
    // submits the title's list through VIF1 DMA without that RAC1 buffer-index convention, and
    // so does Going Commando.
    const bool sent_chain = g->game == "rac4-ntsc" || g->game == "rac2-ntsc";
    const std::uint32_t shown = sent_chain ? chain : viewer::shown_chain(ram, a);
    std::vector<renderer::EffectVertex> strip_vertices;
    std::vector<renderer::EffectStrip> strips;
    std::size_t before_world = SIZE_MAX;
    std::size_t first_offscreen = SIZE_MAX;
    std::size_t world_marker = SIZE_MAX;
    std::vector<std::uint8_t> packets = direct_packets(ram, shown, &strip_vertices, &strips,
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
    return named_quads;
}

// Decides whether the last frame stands for this tick's (a page menu's gap, a list not finished).
void choose_repeat_frame(bool debug) {
    Kept& k = g->kept;
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
}

// Keeps the world effects, the sky and the particles the game drew this tick, and the native
// quads its 2D list names (`named_quads`, keep_2d_path), and clears what was gathered.
void keep_effects(
    std::span<const std::uint8_t> ram,
    const viewer::GameState& state,
    const std::vector<renderer::UiQuad>* named_quads
) {
    Kept& k = g->kept;
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
}

// Reports what this tick drew: in the log (OPENRAC_FRAME_LOG) and to the debug overlay.
void report_tick(std::span<const std::uint8_t> ram, int level) {
    Kept& k = g->kept;
    // OPENRAC_FRAME_LOG=FROM-TO: what each tick in that range drew, in the log.
    static const std::optional<FrameRange> frame_log = env_frames(env::Var::FrameLog);
    if (frame_log && frame_log->holds(k.frame)) {
        log::info(
            "tick {}: draws {:#x} camera {} 2D bytes {} before world {} quads {} effects {} "
            "mobys {}",
            k.frame,
            k.draws,
            k.has_camera,
            k.packets.size(),
            k.before_world,
            k.ui_quads.size(),
            k.effects.size(),
            k.live.size()
        );
    }
    // [debug tools] What the game holds this tick, for the overlay and the dump.
    debug_overlay::tick({g->game.c_str(), ram, level, &k.state, k.draws, k.frame, k.packets.size(), k.ui_quads.size(),
                         k.effects.size(), k.strips.size(), k.live.size()});
}

// Whether the tick's own frame is left out because the tick came late. Counts and warns.
bool leave_late_frame() {
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
    return false;
}

// Draws the tick's own frame, or leaves it to between when the frame cap holds it back.
void draw_tick_frame() {
    // The frame cap may hold the tick's own frame back: the tick is never made to wait for it. The
    // frame is then drawn between ticks, when it is due (between), or not at all when the next
    // tick comes first (a cap under the game's tick rate).
    if (!frame_due(std::chrono::steady_clock::now())) {
        pacing.first_pending = true;
        return;
    }
    pacing.first_pending = false;
    draw_kept(true);
}

}  // namespace

bool frame(std::span<const std::uint8_t> ram, std::uint32_t chain, std::uint32_t draws) {
    if (!g) {
        return true;
    }
    // The window was closed.
    if (!read_window_events()) {
        return false;
    }

    // The level program loaded now (-1: the boot program, title and menus), as the port knows it.
    const int level = openrac_game_loaded_overlay();
    const viewer::GameAddresses& a = viewer::rac1_addresses(g->game, level);
    use_world(ram, level);
    viewer::GameState state = viewer::read_game_state(ram, a);
    // The world is drawn from the camera the game's world renderers used, not from whatever the
    // camera globals hold at the end of the frame (the page menu sets its own camera, at
    // (256, 256, 64) facing +x, to draw its frame objects, as ReRAC notes).
    // A tick on which the world renderers did not run but mobys were drawn with a camera of their
    // own (a page menu's frame objects): only those are drawn, as the game drew no others (the
    // pause page skips the Goodies frame while it has none).
    const bool world_ran = g->world_camera.has_value();
    const bool only_own_camera = !world_ran && !g->moby_cameras.empty();
    take_world_camera(state);
    log_camera(state, world_ran);
    follow_route(ram, level);
    trace_tick(state, draws);
    // The run's frames are done.
    if (run_over()) {
        return false;
    }
    dump_ram(ram);

    // OPENRAC_DEBUG: the game's state each 100 frames, for bring-up.
    static const bool& debug =
        stats::probe("OPENRAC_DEBUG", "the game's state each 100 ticks, in the log");
    if (debug && g->index % 100 == 0) {
        log_tick_state(ram, state, level, draws);
    }
    log_mobys_near_hero(ram, state);
    TickMobys mobys = pose_mobys(ram, state, world_ran, only_own_camera, debug);
    light_scene(ram, state);
    keep_tick(ram, state, draws, std::move(mobys));
    const std::vector<renderer::UiQuad>* named_quads = keep_2d_path(ram, chain, a, level, debug);
    choose_repeat_frame(debug);
    keep_effects(ram, state, named_quads);
    report_tick(ram, level);
    // The tick came late: its frame is left out.
    if (leave_late_frame()) {
        return true;
    }
    draw_tick_frame();
    return true;
}

std::uint32_t text(int id) {
    return options_menu::text(id);
}

std::int64_t since_control() {
    return g && g_route.control >= 0 ? static_cast<std::int64_t>(g->index) - g_route.control : -1;
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

