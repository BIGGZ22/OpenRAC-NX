// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <stb_truetype.h>

#include "platform/switch/launcher_ui.h"

namespace openrac::platform::launcher_ui {
namespace {
using Color = std::uint32_t;

constexpr Color rgb(unsigned r, unsigned g, unsigned b) {
    return r | g << 8 | b << 16 | 0xff000000u;
}

constexpr Color ink = rgb(9, 17, 30), panel = rgb(19, 32, 49), lineColor = rgb(39, 59, 77),
                white = rgb(236, 244, 249), muted = rgb(144, 166, 184), teal = rgb(68, 232, 193),
                orange = rgb(255, 177, 87);
const char* tabs[] =
    {"Play", "Display", "Audio", "Gameplay", "Game fixes", "Saves", "Setup", "Controller"};

unsigned codepoint(const std::string& s, std::size_t& i) {
    unsigned c = static_cast<unsigned char>(s[i++]);
    if (c < 128) {
        return c;
    }
    int more = (c & 0xe0) == 0xc0 ? 1 : (c & 0xf0) == 0xe0 ? 2 : (c & 0xf8) == 0xf0 ? 3 : 0;
    c &= more == 1 ? 31 : more == 2 ? 15 : 7;
    while (more-- && i < s.size()) {
        unsigned n = static_cast<unsigned char>(s[i++]);
        if ((n & 0xc0) != 0x80) {
            return '?';
        }
        c = (c << 6) | (n & 63);
    }
    return c;
}
}  // namespace

struct Canvas::Impl {
    std::vector<Color> pixels = std::vector<Color>(kWidth * kHeight);
    std::vector<std::uint8_t> font_data;
    stbtt_fontinfo face{};
    bool font_ok = false;

    struct Glyph {
        int w, h, x, y, advance;
        std::vector<unsigned char> alpha;
    };

    std::map<std::pair<unsigned, int>, Glyph> glyphs;

    void pixel(int x, int y, Color color, unsigned a = 255) {
        if (unsigned(x) >= kWidth || unsigned(y) >= kHeight) {
            return;
        }
        auto& old = pixels[std::size_t(y) * kWidth + x];
        if (a == 255) {
            old = color;
            return;
        }
        unsigned r = ((color & 255) * a + (old & 255) * (255 - a)) / 255;
        unsigned g = (((color >> 8) & 255) * a + ((old >> 8) & 255) * (255 - a)) / 255;
        unsigned b = (((color >> 16) & 255) * a + ((old >> 16) & 255) * (255 - a)) / 255;
        old = rgb(r, g, b);
    }

    void rect(int x, int y, int w, int h, Color color, int radius = 0) {
        if (w <= 0 || h <= 0) {
            return;
        }
        radius = std::min({radius, w / 2, h / 2});
        for (int row = 0; row < h; ++row) {
            int inset = 0;
            if (radius && (row < radius || row >= h - radius)) {
                const double dy = row < radius ? radius - row - .5 : row - (h - radius) + .5;
                inset = std::max(
                    0, int(std::ceil(radius - std::sqrt(std::max(0.0, radius * radius - dy * dy))))
                );
            }
            int yy = y + row, left = std::max(0, x + inset),
                right = std::min(kWidth, x + w - inset);
            if (yy >= 0 && yy < kHeight && right > left) {
                std::fill(
                    pixels.begin() + yy * kWidth + left, pixels.begin() + yy * kWidth + right, color
                );
            }
        }
    }

    void card(int x, int y, int w, int h, Color fill = panel, int r = 18) {
        rect(x, y, w, h, lineColor, r);
        rect(x + 1, y + 1, w - 2, h - 2, fill, r - 1);
    }

    void triangle(float ax, float ay, float bx, float by, float cx, float cy, Color color) {
        auto edge = [](float x, float y, float a, float b, float c, float d) {
            return (x - a) * (d - b) - (y - b) * (c - a);
        };
        float sign = edge(cx, cy, ax, ay, bx, by) >= 0 ? 1 : -1;
        for (int y = std::max(0, int(std::min({ay, by, cy})));
             y < std::min(kHeight, int(std::max({ay, by, cy})) + 1);
             ++y) {
            for (int x = std::max(0, int(std::min({ax, bx, cx})));
                 x < std::min(kWidth, int(std::max({ax, bx, cx})) + 1);
                 ++x) {
                if (sign * edge(x + .5f, y + .5f, ax, ay, bx, by) >= 0
                    && sign * edge(x + .5f, y + .5f, bx, by, cx, cy) >= 0
                    && sign * edge(x + .5f, y + .5f, cx, cy, ax, ay) >= 0) {
                    pixel(x, y, color);
                }
            }
        }
    }

    void bolt(int x, int y, int size, Color c) {
        triangle(
            x + size * .54f,
            y,
            x + size * .08f,
            y + size * .58f,
            x + size * .56f,
            y + size * .58f,
            c
        );
        triangle(
            x + size * .44f,
            y + size * .42f,
            x + size * .92f,
            y + size * .42f,
            x + size * .35f,
            y + size,
            c
        );
    }

    Glyph& glyph(unsigned cp, int size) {
        auto key = std::pair(cp, size);
        auto it = glyphs.find(key);
        if (it != glyphs.end()) {
            return it->second;
        }
        Glyph g{};
        float scale = stbtt_ScaleForPixelHeight(&face, float(size));
        int advance, bearing;
        stbtt_GetCodepointHMetrics(&face, int(cp), &advance, &bearing);
        g.advance = int(std::round(advance * scale));
        unsigned char* bitmap =
            stbtt_GetCodepointBitmap(&face, scale, scale, int(cp), &g.w, &g.h, &g.x, &g.y);
        if (bitmap) {
            g.alpha.assign(bitmap, bitmap + g.w * g.h);
            stbtt_FreeBitmap(bitmap, nullptr);
        }
        return glyphs.emplace(key, std::move(g)).first->second;
    }

    int width(const std::string& s, int size) {
        if (!font_ok) {
            return 0;
        }
        int w = 0;
        for (std::size_t i = 0; i < s.size();) {
            w += glyph(codepoint(s, i), size).advance;
        }
        return w;
    }

    void text(
        int x,
        int y,
        const std::string& s,
        int size = 24,
        Color color = white,
        int max_width = 10000
    ) {
        if (!font_ok) {
            return;
        }
        int start = x, ascent, descent, gap;
        stbtt_GetFontVMetrics(&face, &ascent, &descent, &gap);
        int baseline = y + int(std::round(ascent * stbtt_ScaleForPixelHeight(&face, float(size))));
        for (std::size_t i = 0; i < s.size();) {
            auto& g = glyph(codepoint(s, i), size);
            if (x + g.advance - start > max_width) {
                break;
            }
            for (int yy = 0; yy < g.h; ++yy) {
                for (int xx = 0; xx < g.w; ++xx) {
                    auto alpha = g.alpha[yy * g.w + xx];
                    if (alpha) {
                        pixel(x + g.x + xx, baseline + g.y + yy, color, alpha);
                    }
                }
            }
            x += g.advance;
        }
    }

    int wrap(
        int x,
        int y,
        const std::string& message,
        int size,
        Color color,
        int max_width,
        int max_lines = 4
    ) {
        std::istringstream input(message);
        std::string word, current;
        int n = 0;
        while (input >> word) {
            if (!current.empty() && width(current + " " + word, size) > max_width) {
                text(x, y + n * (size + 6), current, size, color, max_width);
                current.clear();
                if (++n >= max_lines) {
                    return n;
                }
            }
            if (!current.empty()) {
                current += ' ';
            }
            current += word;
        }
        if (!current.empty() && n < max_lines) {
            text(x, y + n * (size + 6), current, size, color, max_width);
            ++n;
        }
        return n;
    }

    void button(int x, int y, int w, const std::string& title, bool primary = false) {
        rect(x, y, w, 50, primary ? teal : rgb(34, 54, 73), 12);
        text(x + 20, y + 10, title, 25, primary ? ink : white, w - 40);
    }

    void background(int active) {
        for (int y = 0; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                pixels[y * kWidth + x] =
                    rgb(9 + unsigned(x * 5 / kWidth),
                        17 + unsigned((x + y) * 5 / (kWidth + kHeight)),
                        30 + unsigned(x * 8 / kWidth));
            }
        }
        rect(0, 0, 250, 720, rgb(12, 23, 37));
        rect(249, 0, 1, 720, lineColor);
        bolt(28, 34, 28, teal);
        text(69, 31, "OPENRAC", 30);
        text(70, 63, "NX  /  SWITCH", 15, muted);
        for (int i = 0; i < 8; ++i) {
            int y = 148 + i * 60;
            if (i == active) {
                rect(18, y - 4, 212, 48, rgb(30, 61, 72), 12);
                rect(18, y + 7, 4, 25, teal, 2);
            }
            text(42, y + 3, tabs[i], 26, i == active ? teal : muted);
        }
        text(32, 648, "RATCHET 1  /  PAL", 16, muted);
        text(296, 34, "RATCHET & CLANK", 20, muted);
        text(296, 66, active >= 0 ? tabs[active] : "Preparing your game", 40);
        text(984, 41, "PAL v2.00", 17, white);
        text(984, 66, "SCES_509.16", 16, muted);
        rect(296, 643, 936, 1, lineColor);
        text(296, 665, "L / R   Pages     D-pad   Select     A   Confirm     B   Back", 20, muted);
        text(1110, 665, "+  Exit", 20, muted);
    }

    void toast(const std::string& message) {
        if (message.empty()) {
            return;
        }
        card(296, 558, 936, 68, rgb(26, 42, 57), 12);
        rect(296, 570, 4, 43, orange, 2);
        wrap(316, 570, message, 20, white, 890, 2);
    }
};

Canvas::Canvas() : p(std::make_unique<Impl>()) {}

Canvas::~Canvas() = default;

bool Canvas::font(std::span<const std::uint8_t> bytes) {
    p->font_data.assign(bytes.begin(), bytes.end());
    p->glyphs.clear();
    p->font_ok =
        !p->font_data.empty()
        && stbtt_InitFont(
            &p->face, p->font_data.data(), stbtt_GetFontOffsetForIndex(p->font_data.data(), 0)
        );
    return p->font_ok;
}

const std::vector<std::uint32_t>& Canvas::pixels() const {
    return p->pixels;
}

void Canvas::menu(const View& v) {
    p->background(v.tab);
    if ((v.tab >= 1 && v.tab <= 4) || v.tab == 7) {
        p->card(296, 128, 936, 412);
        int first = std::max(0, v.selected - 8), end = std::min(int(v.rows.size()), first + 9);
        for (int i = first; i < end; ++i) {
            int y = 143 + (i - first) * 43;
            if (i == v.selected) {
                p->rect(310, y, 907, 39, rgb(30, 61, 72), 9);
            }
            p->text(332, y + 6, v.rows[i].label, 23, i == v.selected ? white : muted, 475);
            const auto& value = v.rows[i].value;
            if (value == "On" || value == "Off") {
                bool on = value == "On";
                p->rect(1125, y + 8, 53, 25, on ? teal : lineColor, 12);
                p->rect(on ? 1155 : 1129, y + 12, 17, 17, on ? ink : muted, 8);
            } else {
                p->text(850, y + 6, value, 22, i == v.selected ? teal : white, 330);
            }
        }
        if (int(v.rows.size()) > 9) {
            p->rect(1220, 150, 3, 355, lineColor, 1);
            p->rect(
                1220,
                150 + int(314.0 * v.selected / std::max(1, int(v.rows.size()) - 1)),
                3,
                41,
                teal,
                1
            );
        }
        if (v.tab == 7 && v.notice.empty()) {
            p->wrap(
                316,
                568,
                "A: Rebind  |  X: Clear  |  Left / Right: Adjust  |  Swipe to scroll",
                21,
                muted,
                880,
                2
            );
        } else if (v.notice.empty()) {
            p->text(
                316,
                578,
                "Left / Right to adjust. Your settings are saved automatically.",
                21,
                muted
            );
        }
    } else if (v.tab == 0) {
        p->card(296, 128, 936, 273);
        p->text(328, 158, "YOUR NEXT ADVENTURE", 16, teal);
        p->text(328, 192, "Ratchet & Clank", 48);
        p->text(330, 250, "Native Switch port  |  Europe / Australia v2.00", 21, muted);
        p->button(
            328,
            322,
            262,
            v.disc && v.executable && v.levels && v.writable ? "A   Launch game"
                                                             : "A   Check setup",
            true
        );
        p->rect(1043, 163, 142, 142, rgb(28, 58, 69), 71);
        p->rect(1056, 176, 116, 116, rgb(18, 40, 54), 58);
        p->bolt(1071, 181, 89, teal);
        const bool good[] = {v.disc && v.executable, v.levels, v.writable};
        const char* title[] = {"DISC IMAGE", "GAME ASSETS", "SAVE STORAGE"};
        for (int i = 0; i < 3; ++i) {
            int x = 296 + i * 318;
            p->card(x, 419, 300, 115);
            p->rect(x + 21, 442, 9, 9, good[i] ? teal : orange, 4);
            p->text(x + 40, 437, title[i], 17, muted);
            p->text(
                x + 20, 471, good[i] ? "Ready" : "Needs attention", 25, good[i] ? white : orange
            );
        }
        if (v.notice.empty()) {
            p->text(
                312, 580, "X   Recheck installation          Y   Diagnostic boot check", 21, muted
            );
        }
    } else if (v.tab == 5) {
        p->card(296, 128, 936, 411);
        p->text(328, 156, "MEMORY CARD 1", 17, teal);
        p->text(
            328, 193, v.writable ? "Your progress has a home." : "Save storage needs attention.", 34
        );
        p->text(330, 256, "/switch/openrac/memcard/SCES_509.16/", 21, muted);
        p->text(330, 306, "Storage check", 22, muted);
        p->text(
            750,
            306,
            v.writable ? "Write and read-back passed" : "Unable to write",
            23,
            v.writable ? teal : orange
        );
        p->text(330, 350, "Stored files", 22, muted);
        p->text(750, 350, std::to_string(v.save_files), 23);
        p->wrap(
            330,
            411,
            "Save through the game's Save menu, wait for completion, then exit normally. Updating "
            "the NRO keeps your saves.",
            22,
            muted,
            830,
            3
        );
    } else {
        p->card(296, 128, 936, 411);
        p->text(328, 153, "NRO + YOUR ISO. WE HANDLE THE REST.", 19, teal);
        const char* titles[] =
            {"Copy your supported PAL ISO", "Prepare directly on Switch", "Start your adventure"};
        const char* body[] =
            {"Put one ISO beside the NRO in /switch/openrac/.",
             "Use full application mode and keep 2 GiB of extra free space.",
             "Leave the ISO in place. Completed scenes resume after interruption."};
        for (int i = 0; i < 3; ++i) {
            int y = 205 + i * 84;
            p->rect(329, y, 34, 34, rgb(30, 61, 72), 17);
            p->text(341, y + 4, std::to_string(i + 1), 22, teal);
            p->text(382, y, titles[i], 26);
            p->text(382, y + 34, body[i], 20, muted, 810);
        }
        p->button(328, 470, 315, "A   Prepare / resume", true);
    }
    p->toast(v.notice);
}

void Canvas::progress(const std::string& stage, unsigned done, unsigned total) {
    p->background(-1);
    p->card(296, 147, 936, 389);
    p->text(330, 178, "FIRST-LAUNCH PREPARATION", 18, teal);
    p->wrap(330, 222, stage, 33, white, 845, 2);
    float fraction = total ? std::clamp(float(done) / total, 0.0f, 1.0f) : 0;
    p->rect(330, 337, 866, 13, lineColor, 6);
    if (fraction > 0) {
        p->rect(330, 337, std::max(13, int(866 * fraction)), 13, teal, 6);
    }
    p->text(
        330,
        372,
        total ? std::to_string(done) + " / " + std::to_string(total) + "   -   "
                    + std::to_string(int(fraction * 100)) + "%"
              : "Working...",
        24,
        teal
    );
    p->wrap(
        330,
        424,
        "Keep the console powered. Your ISO stays in place, and completed scenes are kept if you "
        "need to stop.",
        23,
        muted,
        835,
        2
    );
    p->text(330, 492, "+   Cancel and return to the menu", 21, white);
}

void Canvas::binding(const std::string& action, const std::string& prompt) {
    p->background(7);
    p->card(296, 147, 936, 389);
    p->text(330, 178, "REBIND CONTROLLER BUTTON", 18, teal);
    p->text(330, 224, action, 36, white, 850);
    p->wrap(330, 290, prompt, 27, white, 850, 2);
    p->wrap(
        330,
        379,
        "Press + and - together to cancel. This screen times out after 20 seconds. Occupied "
        "buttons swap assignments.",
        22,
        muted,
        850,
        3
    );
    p->button(328, 470, 315, "Cancel", false);
}
}  // namespace openrac::platform::launcher_ui
