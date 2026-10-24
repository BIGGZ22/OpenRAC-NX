// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's memory section and its memory viewer: the host's memory, a map of the game's
// main memory and the bytes at an address (debug/overlay.h).

#include "debug/overlay.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <imgui.h>
#include <string>

#include "common/log.h"
#include "common/stats.h"
#include "openrac/guest.h"
#include "renderer/gl_stats.h"

namespace openrac::frontend::debug_overlay {

void scan_ram() {
    if (s.ram.size() < kRamSize || s.scanned_at == s.tick / 25) {
        return;
    }
    const bool first = s.scanned_at == ~0ull;
    s.scanned_at = s.tick / 25;
    s.used_pages = 0;
    for (std::size_t cell = 0; cell < kCells; ++cell) {
        std::uint16_t used = 0;
        std::uint64_t sum = 0;
        const std::uint8_t* base = s.ram.data() + cell * kCellBytes;
        for (std::uint32_t page = 0; page < kCellBytes / 4096; ++page) {
            std::uint64_t any = 0;
            for (std::uint32_t i = 0; i < 4096; i += 8) {
                std::uint64_t word;
                std::memcpy(&word, base + page * 4096 + i, 8);
                any |= word;
                sum = sum * 31 + word;
            }
            used = static_cast<std::uint16_t>(used + (any != 0 ? 1 : 0));
        }
        s.cells[cell].changed = !first && sum != s.cells[cell].sum;
        s.cells[cell].sum = sum;
        s.cells[cell].used_pages = used;
        s.used_pages += used;
    }
}

void memory_section() {
    const stats::ProcessMemory host = stats::process_memory();
    text("host process: {} resident{}{}", megabytes(host.resident),
         host.footprint != 0 ? std::format(", {} footprint", megabytes(host.footprint)) : std::string(),
         host.peak != 0 ? std::format(", peak {}", megabytes(host.peak)) : std::string());
    const renderer::gl_stats::Textures textures = renderer::gl_stats::textures();
    text("GL textures: {} in {} textures", megabytes(textures.bytes), textures.count);
    scan_ram();
    text("game memory: 32 MB, {} holding data ({} of 8192 pages)",
         megabytes(static_cast<std::uint64_t>(s.used_pages) * 4096), s.used_pages);
    text_coloured(kDim, "the game's own heaps are not read yet: their headers are not identified");
    text("game stack: {:.1f} KB in use of 1024", static_cast<double>(GUEST_STACK_TOP - openrac_guest_sp) / 1024.0);
    // The map: 128 KB a cell, 64 a row (8 MB); brighter with more of it in use, orange where it
    // changed since the last look (twice a second).
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float cw = dp(5.0f), ch = dp(9.0f);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    for (std::size_t cell = 0; cell < kCells; ++cell) {
        const float x = origin.x + static_cast<float>(cell % 64) * cw;
        const float y = origin.y + static_cast<float>(cell / 64) * ch;
        const RamCell& c = s.cells[cell];
        const int level = c.used_pages == 0 ? 0 : 70 + c.used_pages * 185 / 32;
        const ImU32 colour = c.used_pages == 0 ? IM_COL32(40, 40, 48, 255)
                             : c.changed ? IM_COL32(level, level * 6 / 10, 30, 255)
                                         : IM_COL32(40, level, 70, 255);
        draw->AddRectFilled(ImVec2(x, y), ImVec2(x + cw - 1.0f, y + ch - 1.0f), colour);
    }
    ImGui::InvisibleButton("ram map", ImVec2(64.0f * cw, static_cast<float>(kCells / 64) * ch));
    if (ImGui::IsItemHovered()) {
        const ImVec2 mouse = ImGui::GetMousePos();
        const std::size_t column = static_cast<std::size_t>(std::clamp((mouse.x - origin.x) / cw, 0.0f, 63.0f));
        const std::size_t line = static_cast<std::size_t>(
            std::clamp((mouse.y - origin.y) / ch, 0.0f, static_cast<float>(kCells / 64 - 1)));
        const std::size_t cell = line * 64 + column;
        ImGui::SetTooltip("%#010x - %#010x: %d of 32 pages in use%s\nclick: open in the memory viewer",
                          static_cast<unsigned>(cell * kCellBytes), static_cast<unsigned>((cell + 1) * kCellBytes - 1),
                          static_cast<int>(s.cells[cell].used_pages), s.cells[cell].changed ? ", changing" : "");
        if (ImGui::IsItemClicked()) {
            s.view_address = static_cast<std::uint32_t>(cell * kCellBytes);
            s.memory_viewer = true;
        }
    }
    text_coloured(kDim, "0 MB at the top left, 8 MB a row; orange: changing");
    ImGui::Checkbox("memory viewer", &s.memory_viewer);
    ImGui::SameLine();
    if (ImGui::Button("write memory to a file")) {
        const std::string path = std::format("ram_{}.bin", s.tick);
        if (std::FILE* f = std::fopen(path.c_str(), "wb")) {
            std::fwrite(s.ram.data(), 1, s.ram.size(), f);
            std::fclose(f);
            s.message = "main memory written to " + path;
            log::info("{}", s.message);
        } else {
            s.message = "cannot write " + path;
        }
    }
}

void memory_viewer_window() {
    if (!s.memory_viewer) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Kept in the bottom right corner, whatever its size becomes.
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 10.0f,
                                   viewport->WorkPos.y + viewport->WorkSize.y - 10.0f),
                            ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.88f);
    if (ImGui::Begin("Game memory", &s.memory_viewer,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        static char input[16] = "100000";
        ImGui::SetNextItemWidth(dp(90.0f));
        if (ImGui::InputText("address", input, sizeof(input),
                             ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            s.view_address = static_cast<std::uint32_t>(std::strtoul(input, nullptr, 16));
        }
        const auto go = [&](std::uint32_t address) {
            s.view_address = address & 0x01FFFFF0;
            std::snprintf(input, sizeof(input), "%X", static_cast<unsigned>(s.view_address));
        };
        ImGui::SameLine();
        if (ImGui::Button("-0x100")) {
            go(s.view_address - 0x100);
        }
        ImGui::SameLine();
        if (ImGui::Button("+0x100")) {
            go(s.view_address + 0x100);
        }
        ImGui::SameLine();
        if (ImGui::Button("follow")) {
            go(read<std::uint32_t>(s.view_address));
        }
        ImGui::SameLine();
        ImGui::Checkbox("words", &s.view_words);
        if (s.selected != 0) {
            ImGui::SameLine();
            if (ImGui::Button("selected moby")) {
                go(s.selected);
            }
        }
        const std::uint32_t base = s.view_address & 0x01FFFFF0;
        for (std::uint32_t line = 0; line < 16; ++line) {
            const std::uint32_t at = base + line * 16;
            if (!in_ram(at, 16)) {
                break;
            }
            std::string out = std::format("{:08X} ", at);
            if (s.view_words) {
                for (std::uint32_t i = 0; i < 16; i += 4) {
                    out += std::format(" {:08X}", read<std::uint32_t>(at + i));
                }
                out += "  ";
                for (std::uint32_t i = 0; i < 16; i += 4) {
                    const float f = read<float>(at + i);
                    out += std::isfinite(f) && std::fabs(f) < 1.0e7f && (std::fabs(f) > 1.0e-4f || f == 0.0f)
                               ? std::format(" {:10.3f}", f) : std::string("          .");
                }
            } else {
                for (std::uint32_t i = 0; i < 16; ++i) {
                    out += std::format(" {:02X}", read<std::uint8_t>(at + i));
                }
                out += "  ";
                for (std::uint32_t i = 0; i < 16; ++i) {
                    const char c = static_cast<char>(read<std::uint8_t>(at + i));
                    out += c >= 0x20 && c < 0x7F ? c : '.';
                }
            }
            ImGui::TextUnformatted(out.c_str());
        }
    }
    ImGui::End();
}

}  // namespace openrac::frontend::debug_overlay
