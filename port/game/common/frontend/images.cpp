// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The images the game moves between its memory and the GS outside the display list, a slice of
// frontend/state.h: the images it sends (upload_image), kept for every renderer made, and the
// frame it reads back (store_image), answered from the last frame drawn with its world in, scaled
// to the chip's pixels.

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "common/env.h"
#include "common/log.h"
#include "frontend.h"
#include "game/common/frontend/state.h"
#include "renderer/direct.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/texture_pool.h"
#include "scene/screenshot.h"

namespace openrac::frontend {

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
        "library image to block {:#x} width {} psm {:#x} at {},{} size {}x{}",
        base,
        width_units,
        psm,
        x,
        y,
        width,
        height
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
    if (render_pending()) {
        g->pending_images.push_back(image);
    } else if (g->renderer) {
        g->renderer->textures().upload(image);
    }
    g->images.push_back(std::move(image));
}

bool store_image(
    std::uint32_t base,
    std::uint32_t width_units,
    std::uint8_t psm,
    int x,
    int y,
    int width,
    int height,
    std::span<std::uint8_t> out
) {
    // Readbacks must see the completed snapshot, including its retained world image.
    if (!finish_render()) {
        return false;
    }
    int fx = 0;
    int fy = 0;
    // The draw buffer as the game left it: the last frame with its world in.
    const renderer::FrameBuffer* source = g && g->last_world.id() != 0 ? &g->last_world
                                          : g                          ? &g->frame
                                                                       : nullptr;
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
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA8, snap_w, snap_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
            );
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
                std::uint8_t* q =
                    chip.data() + (static_cast<std::size_t>(cy) * kChipWidth + cx) * 4;
                q[0] = p[0];
                q[1] = p[1];
                q[2] = p[2];
                q[3] = 0x80;
            }
        }
        // OPENRAC_DUMP_SNAPSHOT=FILE.png: the answer, as the game gets it.
        if (const std::optional<std::string> dump = env_string(env::Var::DumpSnapshot)) {
            scene::write_png(*dump, kChipWidth, lines, chip);
            log::info("frame read back at window frame {}, written to {}", g->index, *dump);
        }
    }
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            std::uint8_t* q = out.data() + (static_cast<std::size_t>(r) * width + c) * 4;
            if (renderer::frame_pixel(base, width_units, x + c, y + r, fx, fy)) {
                std::memcpy(
                    q, chip.data() + (static_cast<std::size_t>(fy) * kChipWidth + fx) * 4, 4
                );
            } else {
                std::memset(q, 0, 4);
            }
        }
    }
    renderer::g_frame_snapshot.answered.push_back(renderer::hash_bytes(out.data(), out.size()));
    return true;
}

}  // namespace openrac::frontend
