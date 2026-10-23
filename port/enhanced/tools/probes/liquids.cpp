// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/tools/probes/liquids.h"

#include <cmath>
#include <map>
#include <optional>

#include "assets/disc/overlay.h"

namespace openrac::lighting {
namespace {
using namespace assets;

float linear(float c) {
    return c <= .04045f ? c / 12.92f : std::pow((c + .055f) / 1.055f, 2.4f);
}

float display(float c) {
    return c <= .0031308f ? c * 12.92f : 1.055f * std::pow(c, 1 / 2.4f) - .055f;
}
}  // namespace

std::vector<BakeTriangle> gaspar_liquid_triangles(
    assets::ByteView overlay, assets::ByteView index, assets::ByteView core
) {
    using namespace assets;
    const auto sections = disc::parse_overlay_sections(overlay);
    const auto read = [&](u32 address, std::size_t count) {
        const auto bytes = disc::read_overlay(sections, address, count);
        if (!bytes) {
            fail("Gaspar liquid data is outside the loaded overlay");
        }
        return *bytes;
    };
    // NTSC FUN_L09_002ef750 draws this authored record. No scan of arbitrary
    // bright materials: only this reviewed liquid renderer contributes here.
    const auto record = read(0x1fc740, 0x40);
    Float3 origin{};
    for (unsigned c = 0; c < 3; ++c) {
        origin[c] = record.read<float>(c * 4);
    }
    const float sx = record.read<float>(16), sy = record.read<float>(20);
    const auto cols = record.read<u16>(24), rows = record.read<u16>(26);
    for (float v : {origin[0], origin[1], origin[2], sx, sy}) {
        if (!std::isfinite(v)) {
            fail("Gaspar liquid grid is not finite");
        }
    }
    if (sx <= 0 || sy <= 0 || !cols || !rows || cols > 512 || rows > 512 || cols % 7 || rows % 7) {
        fail("Unsupported Gaspar liquid grid");
    }
    const auto blocks = std::size_t(cols / 7) * (rows / 7);
    const auto colours = read(record.read<u32>(0x34), blocks * 512);

    // Find the authored 124-index strip table, whose first row alternates
    // neighbouring grid rows. Derive the search pattern from that layout.
    std::optional<std::array<u16, 124>> order;
    for (const auto& section : sections) {
        if (section.kind == 8) {
            continue;
        }
        const ByteView bytes(section.data);
        for (std::size_t at = 0; at + 16 <= bytes.size(); at += 2) {
            bool match = true;
            for (unsigned i = 0; i < 8; ++i) {
                if (bytes.read<u16>(at + i * 2) != (i / 2) * 8 + (i % 2) * 64) {
                    match = false;
                    break;
                }
            }
            if (!match) {
                continue;
            }
            if (order) {
                fail("Ambiguous Gaspar liquid strip table");
            }
            std::array<u16, 124> candidate{};
            for (unsigned i = 0; i < candidate.size(); ++i) {
                candidate[i] = bytes.read<u16>(at + i * 2);
                if (candidate[i] > 504 || candidate[i] % 8) {
                    fail("Invalid Gaspar liquid strip index");
                }
            }
            order = candidate;
        }
    }
    if (!order) {
        fail("Missing Gaspar liquid strip table");
    }

    const auto first = record.read<u16>(0x3c);
    const auto frames = record.read<u8>(0x3e);
    if (first != 44 || frames != 16) {
        fail("Unsupported Gaspar liquid animation");
    }
    const auto count = index.read<u32>(0x58), table = index.read<u32>(0x5c);
    const auto bank = index.read<u32>(0x68);
    std::map<u32, std::size_t> histogram;
    std::size_t total = 0;
    for (unsigned frame = first; frame < unsigned(first) + frames; ++frame) {
        if (frame >= count) {
            fail("Missing Gaspar liquid animation frame");
        }
        const auto entry = index.sub(std::size_t(table) + std::size_t(frame) * 16, 16);
        if (entry.read<s32>(0) < 0 || entry.read<s32>(4) < 0 || entry.read<s32>(8) != 64
            || entry.read<s32>(12) != 64) {
            fail("Invalid Gaspar liquid animation texture");
        }
        const auto palette = core.sub(std::size_t(bank) + entry.read<u32>(0), 1024);
        const auto pixels = core.sub(std::size_t(bank) + entry.read<u32>(4), 4096);
        // The live FX renderer uses the stored palette indices directly.
        for (u8 p : pixels.span()) {
            const u32 rgb = palette.read<u32>(std::size_t(p) * 4) & 0x00ffffff;
            ++histogram[rgb];
            ++total;
        }
    }
    std::map<u32, Float3> averaged;
    const auto colour_at = [&](std::size_t at) {
        const u32 rgb = colours.read<u32>(at) & 0x00ffffff;
        if (const auto found = averaged.find(rgb); found != averaged.end()) {
            return found->second;
        }
        Float3 result{};
        for (unsigned c = 0; c < 3; ++c) {
            double sum = 0;
            const float tint = float((rgb >> (c * 8)) & 255) / 128;
            for (const auto& [pixel, n] : histogram) {
                const float value =
                    std::clamp(float((pixel >> (c * 8)) & 255) / 255 * tint, 0.0f, 1.0f);
                sum += double(linear(value)) * double(n);
            }
            // Existing shared emission sampling interpolates authored sRGB
            // vertex tint, then decodes it with the texture. A white texture
            // is implicit here, keeping the same material convention.
            result[c] = display(float(sum / double(total)));
        }
        averaged.emplace(rgb, result);
        return result;
    };
    std::vector<BakeTriangle> triangles;
    triangles.reserve(blocks * 98);
    std::size_t block = 0;
    for (unsigned by = 0; by < rows / 7; ++by) {
        for (unsigned bx = 0; bx < cols / 7; ++bx, ++block) {
            std::array<Float3, 124> positions, emission;
            for (unsigned k = 0; k < order->size(); ++k) {
                const unsigned point = (*order)[k] / 8;
                positions[k] =
                    {origin[0] + float(bx * 7 + point % 8) * sx,
                     origin[1] + float(by * 7 + point / 8) * sy,
                     origin[2]};
                emission[k] = colour_at(block * 512 + 16 + k * 4);
            }
            for (unsigned k = 0; k < 122; ++k) {
                const std::array<unsigned, 3> indices =
                    k % 2 ? std::array<unsigned, 3>{k + 1, k, k + 2}
                          : std::array<unsigned, 3>{k, k + 1, k + 2};
                BakeTriangle triangle;
                triangle.diffuse = {};
                triangle.emission = {1, 1, 1};
                triangle.emission_vertex_colour = true;
                for (unsigned i = 0; i < 3; ++i) {
                    triangle.vertices[i] = positions[indices[i]];
                    triangle.emission_colours[i] = emission[indices[i]];
                }
                const auto& a = triangle.vertices[0];
                const auto& b = triangle.vertices[1];
                const auto& c = triangle.vertices[2];
                if (std::abs((b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]))
                    > 1e-8f) {
                    triangles.push_back(triangle);
                }
            }
        }
    }
    return triangles;
}
}  // namespace openrac::lighting
