// SPDX-License-Identifier: GPL-3.0-or-later
// The Switch launcher's picture, drawn on the CPU into one 1280x720 RGBA buffer: a top bar of
// sections (L / R), the page, and a bottom bar of button hints. Colours come from the launcher's
// appearance settings (launcher_appearance.h); the layout constants are shared with the input code
// in launcher_ui.h, so what a touch hits is what is drawn.
#include <algorithm>
#include <cctype>
#include <cmath>
#include <initializer_list>
#include <map>
#include <sstream>
#include <tuple>
#include <stb_truetype.h>

#include "platform/switch/launcher_ui.h"
#include "renderer/ui/logo.h"
#include "platform/switch/version.h"

namespace openrac::platform::launcher_ui {
namespace {
using Color = std::uint32_t;

constexpr Color rgb(unsigned r, unsigned g, unsigned b) {
    return r | g << 8 | b << 16 | 0xff000000u;
}
unsigned red(Color c) { return c & 255; }
unsigned green(Color c) { return (c >> 8) & 255; }
unsigned blue(Color c) { return (c >> 16) & 255; }
// `amount` (0..255) of b over a.
Color mix(Color a, Color b, unsigned amount) {
    const auto m = [amount](unsigned x, unsigned y) { return (x * (255 - amount) + y * amount) / 255; };
    return rgb(m(red(a), red(b)), m(green(a), green(b)), m(blue(a), blue(b)));
}
bool light_colour(Color c) { return red(c) * 299 + green(c) * 587 + blue(c) * 114 >= 145000; }

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

// A button hint in the bottom bar.
enum class Key { A, B, X, Y, LR, LeftRight, UpDown, Plus };
struct Hint {
    Key key;
    const char* label;
};
}  // namespace

struct Canvas::Impl {
    // ---- Theme (appearance settings → the roles the pages use) ----
    appearance::Settings appearance_settings;
    Color ink = rgb(10, 15, 30), bar = rgb(13, 19, 38), panel = rgb(18, 25, 51), line = rgb(31, 40, 71),
          text_c = rgb(238, 242, 250), muted = rgb(143, 155, 184), accent = rgb(255, 181, 71);
    Color raised = 0, soft_accent = 0, on_accent = 0, nav = 0, ok = 0, ok_bg = 0, warn = 0, warn_bg = 0, sunk = 0;

    void theme() {
        const auto p = appearance_settings.palette();
        ink = p.background; bar = p.sidebar; panel = p.panel; line = p.border;
        text_c = p.text; muted = p.muted; accent = p.accent;
        const bool light = light_colour(panel);
        raised = mix(panel, text_c, light ? 14 : 22);
        sunk = mix(panel, ink, 150);
        soft_accent = mix(panel, accent, 40);
        on_accent = appearance::contrasting(accent);
        nav = mix(muted, text_c, 90);
        ok = light ? rgb(0, 122, 88) : rgb(94, 224, 181);
        ok_bg = mix(panel, ok, light ? 40 : 50);
        warn = light ? rgb(178, 72, 10) : rgb(255, 154, 92);
        warn_bg = mix(panel, warn, light ? 40 : 55);
    }

    // ---- Pixels ----
    std::vector<Color> pixels = std::vector<Color>(kWidth * kHeight);

    void pixel(int x, int y, Color color, unsigned a = 255) {
        if (unsigned(x) >= unsigned(kWidth) || unsigned(y) >= unsigned(kHeight) || a == 0) {
            return;
        }
        auto& old = pixels[std::size_t(y) * kWidth + x];
        old = a >= 255 ? (color | 0xff000000u) : mix(old, color, a);
    }

    // A rectangle with anti-aliased round corners; `a` < 255 blends it.
    void rect(int x, int y, int w, int h, Color color, int radius = 0, unsigned a = 255) {
        if (w <= 0 || h <= 0) {
            return;
        }
        radius = std::max(0, std::min({radius, w / 2, h / 2}));
        for (int row = 0; row < h; ++row) {
            const int yy = y + row;
            if (yy < 0 || yy >= kHeight) {
                continue;
            }
            const bool corner_row = radius && (row < radius || row >= h - radius);
            if (!corner_row) {
                const int left = std::max(0, x), right = std::min(kWidth, x + w);
                if (a >= 255) {
                    std::fill(pixels.begin() + yy * kWidth + left, pixels.begin() + yy * kWidth + right, color | 0xff000000u);
                } else {
                    for (int xx = left; xx < right; ++xx) pixel(xx, yy, color, a);
                }
                continue;
            }
            const double cy = row < radius ? radius - 0.5 : h - radius - 0.5;
            for (int col = 0; col < w; ++col) {
                double cx = col;
                if (col < radius) cx = radius - 0.5;
                else if (col >= w - radius) cx = w - radius - 0.5;
                else { pixel(x + col, yy, color, a); continue; }
                const double d = std::hypot(col + 0.5 - (cx + 0.5), row + 0.5 - (cy + 0.5));
                const double cover = std::clamp(radius - d + 0.5, 0.0, 1.0);
                if (cover > 0) pixel(x + col, yy, color, unsigned(cover * a));
            }
        }
    }

    void card(int x, int y, int w, int h, Color fill, int r = 20, Color border = 0) {
        rect(x, y, w, h, border ? border : line, r);
        rect(x + 1, y + 1, w - 2, h - 2, fill, r - 1);
    }

    // A filled disc, anti-aliased.
    void disc(float cx, float cy, float r, Color color, unsigned a = 255) {
        for (int y = int(cy - r - 1); y <= int(cy + r + 1); ++y) {
            for (int x = int(cx - r - 1); x <= int(cx + r + 1); ++x) {
                const float d = std::hypot(x + 0.5f - cx, y + 0.5f - cy);
                const float cover = std::clamp(r - d + 0.5f, 0.0f, 1.0f);
                if (cover > 0) pixel(x, y, color, unsigned(cover * a));
            }
        }
    }

    void ring(float cx, float cy, float r, float width, Color color) {
        for (int y = int(cy - r - width); y <= int(cy + r + width); ++y) {
            for (int x = int(cx - r - width); x <= int(cx + r + width); ++x) {
                const float d = std::fabs(std::hypot(x + 0.5f - cx, y + 0.5f - cy) - r);
                const float cover = std::clamp(width / 2 - d + 0.5f, 0.0f, 1.0f);
                if (cover > 0) pixel(x, y, color, unsigned(cover * 255));
            }
        }
    }

    // A thick line with round ends, anti-aliased.
    void segment(float ax, float ay, float bx, float by, float width, Color color) {
        const float half = width / 2;
        const int x0 = int(std::min(ax, bx) - half - 1), x1 = int(std::max(ax, bx) + half + 1);
        const int y0 = int(std::min(ay, by) - half - 1), y1 = int(std::max(ay, by) + half + 1);
        const float dx = bx - ax, dy = by - ay, length = dx * dx + dy * dy;
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                const float px = x + 0.5f - ax, py = y + 0.5f - ay;
                const float t = length > 0 ? std::clamp((px * dx + py * dy) / length, 0.0f, 1.0f) : 0;
                const float d = std::hypot(px - t * dx, py - t * dy);
                const float cover = std::clamp(half - d + 0.5f, 0.0f, 1.0f);
                if (cover > 0) pixel(x, y, color, unsigned(cover * 255));
            }
        }
    }

    void triangle(float ax, float ay, float bx, float by, float cx, float cy, Color color) {
        auto edge = [](float x, float y, float a, float b, float c, float d) {
            return (x - a) * (d - b) - (y - b) * (c - a);
        };
        const float sign = edge(cx, cy, ax, ay, bx, by) >= 0 ? 1 : -1;
        for (int y = std::max(0, int(std::min({ay, by, cy}))); y < std::min(kHeight, int(std::max({ay, by, cy})) + 1); ++y) {
            for (int x = std::max(0, int(std::min({ax, bx, cx}))); x < std::min(kWidth, int(std::max({ax, bx, cx})) + 1); ++x) {
                // 2x2 samples for soft edges.
                int inside = 0;
                for (float sy : {0.25f, 0.75f}) for (float sx : {0.25f, 0.75f}) {
                    inside += sign * edge(x + sx, y + sy, ax, ay, bx, by) >= 0
                              && sign * edge(x + sx, y + sy, bx, by, cx, cy) >= 0
                              && sign * edge(x + sx, y + sy, cx, cy, ax, ay) >= 0;
                }
                if (inside) pixel(x, y, color, unsigned(inside * 255 / 4));
            }
        }
    }

    void bolt(int x, int y, int size, Color c) {
        triangle(x + size * .56f, y, x + size * .10f, y + size * .58f, x + size * .58f, y + size * .58f, c);
        triangle(x + size * .42f, y + size * .42f, x + size * .90f, y + size * .42f, x + size * .36f, y + size, c);
    }

    void check(int cx, int cy, float s, Color c) {
        segment(cx - s * .55f, cy + s * .02f, cx - s * .15f, cy + s * .42f, s * .24f, c);
        segment(cx - s * .15f, cy + s * .42f, cx + s * .6f, cy - s * .45f, s * .24f, c);
    }

    void chevron(int cx, int cy, int s, bool right, Color c) {
        const float d = right ? 1.f : -1.f;
        segment(cx - d * s * .3f, cy - s * .6f, cx + d * s * .3f, cy, 2.6f, c);
        segment(cx + d * s * .3f, cy, cx - d * s * .3f, cy + s * .6f, 2.6f, c);
    }

    void image(const artwork::Image& art, int x, int y, int w, int h, bool crop = true, int radius = 0) {
        if (art.width < 1 || art.height < 1 || art.pixels.size() != std::size_t(art.width) * art.height) return;
        const double scale = crop ? std::max(double(w) / art.width, double(h) / art.height)
                                  : std::min(double(w) / art.width, double(h) / art.height);
        const double ox = (art.width - w / scale) / 2, oy = (art.height - h / scale) / 2;
        for (int dy = 0; dy < h; ++dy) for (int dx = 0; dx < w; ++dx) {
            unsigned cover = 255;
            if (radius) {
                const int cx = std::clamp(dx, radius, w - radius - 1), cy = std::clamp(dy, radius, h - radius - 1);
                const double d = std::hypot(dx - cx, dy - cy);
                if (d > radius + 0.5) continue;
                cover = unsigned(std::clamp(radius - d + 0.5, 0.0, 1.0) * 255);
            }
            const int sx = int(ox + dx / scale), sy = int(oy + dy / scale);
            if (sx < 0 || sy < 0 || sx >= art.width || sy >= art.height) continue;
            const auto color = art.pixels[std::size_t(sy) * art.width + sx];
            pixel(x + dx, y + dy, color, (color >> 24) * cover / 255);
        }
    }

    // ---- Text: the system interface font, and an optional display face for titles ----
    struct Face {
        std::vector<std::uint8_t> data;
        stbtt_fontinfo info{};
        bool ok = false;
    };
    Face body, display;
    struct Glyph {
        int w, h, x, y, advance;
        std::vector<unsigned char> alpha;
    };
    std::map<std::tuple<bool, unsigned, int>, Glyph> glyphs;

    Face& face(bool title) { return title && display.ok ? display : body; }

    Glyph& glyph(bool title, unsigned cp, int size) {
        Face& f = face(title);
        // A codepoint the display face lacks comes from the interface font.
        const bool use_display = &f == &display && stbtt_FindGlyphIndex(&display.info, int(cp)) != 0;
        Face& from = use_display ? display : body;
        const auto key = std::tuple(use_display, cp, size);
        if (auto it = glyphs.find(key); it != glyphs.end()) {
            return it->second;
        }
        Glyph g{};
        const float scale = stbtt_ScaleForPixelHeight(&from.info, float(size));
        int advance, bearing;
        stbtt_GetCodepointHMetrics(&from.info, int(cp), &advance, &bearing);
        g.advance = int(std::round(advance * scale));
        if (unsigned char* bitmap = stbtt_GetCodepointBitmap(&from.info, scale, scale, int(cp), &g.w, &g.h, &g.x, &g.y)) {
            g.alpha.assign(bitmap, bitmap + g.w * g.h);
            stbtt_FreeBitmap(bitmap, nullptr);
        }
        return glyphs.emplace(key, std::move(g)).first->second;
    }

    int width(const std::string& s, int size, bool title = false, int tracking = 0) {
        if (!body.ok) return 0;
        int w = 0;
        for (std::size_t i = 0; i < s.size();) w += glyph(title, codepoint(s, i), size).advance + tracking;
        return w;
    }

    // Text with its top at y; cut with an ellipsis past max_width.
    void text(int x, int y, const std::string& s, int size = 20, Color color = 0, int max_width = 10000,
              bool title = false, int tracking = 0) {
        if (!body.ok) return;
        if (!color) color = text_c;
        std::string shown = s;
        if (width(shown, size, title, tracking) > max_width) {
            const std::string dots = "...";
            while (!shown.empty() && width(shown + dots, size, title, tracking) > max_width) {
                shown.pop_back();
                while (!shown.empty() && (static_cast<unsigned char>(shown.back()) & 0xc0) == 0x80) shown.pop_back();
                if (!shown.empty() && (static_cast<unsigned char>(shown.back()) & 0xc0) == 0xc0) shown.pop_back();
            }
            shown += dots;
        }
        Face& f = face(title);
        int ascent, descent, gap;
        stbtt_GetFontVMetrics(&f.info, &ascent, &descent, &gap);
        const int baseline = y + int(std::round(ascent * stbtt_ScaleForPixelHeight(&f.info, float(size))));
        for (std::size_t i = 0; i < shown.size();) {
            auto& g = glyph(title, codepoint(shown, i), size);
            for (int yy = 0; yy < g.h; ++yy) {
                for (int xx = 0; xx < g.w; ++xx) {
                    if (const auto alpha = g.alpha[yy * g.w + xx]) pixel(x + g.x + xx, baseline + g.y + yy, color, alpha);
                }
            }
            x += g.advance + tracking;
        }
    }
    // The y that puts the text's capitals (H) centred on cy.
    int mid_top(int cy, int size, bool title = false) {
        if (!body.ok) return cy;
        Face& f = face(title);
        const float scale = stbtt_ScaleForPixelHeight(&f.info, float(size));
        int ascent, descent, gap, x0, y0, x1, y1;
        stbtt_GetFontVMetrics(&f.info, &ascent, &descent, &gap);
        if (!stbtt_GetCodepointBox(&f.info, 'H', &x0, &y0, &x1, &y1)) y1 = int(ascent * 0.7f);
        return int(std::round(cy + y1 * scale / 2 - ascent * scale));
    }
    // Text centred on (cx, cy).
    void text_mid(int cx, int cy, const std::string& s, int size, Color color, bool title = false) {
        text(cx - width(s, size, title) / 2, mid_top(cy, size, title), s, size, color, 10000, title);
    }
    void text_right(int right, int y, const std::string& s, int size, Color color, int max_width = 10000) {
        text(right - std::min(max_width, width(s, size)), y, s, size, color, max_width);
    }
    void text_centre(int cx, int y, const std::string& s, int size, Color color, bool title = false) {
        text(cx - width(s, size, title) / 2, y, s, size, color, 10000, title);
    }
    void label(int x, int y, const std::string& s, Color color, int size = 13) {  // small capitals line
        text(x, y, s, size, color, 10000, false, 2);
    }

    int wrap(int x, int y, const std::string& message, int size, Color color, int max_width, int max_lines = 4,
             int leading = 0) {
        if (!leading) leading = size * 3 / 2;
        std::istringstream input(message);
        std::string word, current;
        int n = 0;
        while (input >> word) {
            if (!current.empty() && width(current + " " + word, size) > max_width) {
                if (n + 1 == max_lines) {
                    text(x, y + n * leading, current + " " + word, size, color, max_width);
                    return n + 1;
                }
                text(x, y + n * leading, current, size, color, max_width);
                current.clear();
                ++n;
            }
            if (!current.empty()) current += ' ';
            current += word;
        }
        if (!current.empty() && n < max_lines) {
            text(x, y + n * leading, current, size, color, max_width);
            ++n;
        }
        return n;
    }

    // ---- Shared pieces ----
    const renderer::ui::logo::Mask upstream_logo = renderer::ui::logo::draw(38, true);

    void upstream(int x, int y) {
        for (int row = 0; row < upstream_logo.height; ++row) {
            for (int col = 0; col < upstream_logo.width; ++col) {
                pixel(x + col, y + row, col < upstream_logo.name_x ? rgb(232, 163, 12) : text_c,
                      upstream_logo.alpha[row * upstream_logo.width + col]);
            }
        }
    }

    // A round face button: A, B, X or Y.
    void face_button(int cx, int cy, int r, const char* letter, Color fill, Color ink_c) {
        disc(cx, cy, r, fill);
        text_mid(cx, cy, letter, r * 5 / 4, ink_c);
    }

    void key(Key k, int x, int cy) {
        switch (k) {
            case Key::A: face_button(x + 13, cy, 13, "A", text_c, ink); return;
            case Key::B: face_button(x + 13, cy, 13, "B", text_c, ink); return;
            case Key::X: face_button(x + 13, cy, 13, "X", text_c, ink); return;
            case Key::Y: face_button(x + 13, cy, 13, "Y", text_c, ink); return;
            case Key::Plus:
                ring(x + 13, cy, 12, 1.6f, muted);
                rect(x + 8, cy - 1, 10, 3, text_c, 1);
                rect(x + 12, cy - 5, 3, 10, text_c, 1);
                return;
            case Key::LR:
                rect(x, cy - 11, 50, 22, mix(bar, muted, 120), 6);
                rect(x + 2, cy - 9, 46, 18, bar, 5);
                text_mid(x + 25, cy, "L   R", 13, nav);
                return;
            case Key::LeftRight:
                rect(x, cy - 11, 46, 22, mix(bar, muted, 120), 6);
                rect(x + 2, cy - 9, 42, 18, bar, 5);
                triangle(x + 10, cy, x + 18, cy - 5, x + 18, cy + 5, nav);
                triangle(x + 36, cy, x + 28, cy - 5, x + 28, cy + 5, nav);
                return;
            case Key::UpDown:
                rect(x, cy - 11, 46, 22, mix(bar, muted, 120), 6);
                rect(x + 2, cy - 9, 42, 18, bar, 5);
                triangle(x + 15, cy - 5, x + 10, cy + 4, x + 20, cy + 4, nav);
                triangle(x + 31, cy + 5, x + 26, cy - 4, x + 36, cy - 4, nav);
                return;
        }
    }
    int key_width(Key k) { return k == Key::LR ? 50 : (k == Key::LeftRight || k == Key::UpDown) ? 46 : 26; }

    void top_bar(int active_section, const char* centre_title = nullptr) {
        rect(0, 0, kWidth, kTopBar, bar);
        rect(0, kTopBar - 1, kWidth, 1, line);
        bolt(40, 22, 30, accent);
        text(80, 16, "OpenRAC-NX", 22, text_c, 200, true);
        text(81, 46, std::string(switch_build::kLabel) + "  ·  " + switch_build::kBuildLabel, 12, muted, 190);
        if (centre_title) {
            text_mid(kWidth / 2, 38, centre_title, 17, muted);
        } else {
            // L, the sections, R.
            const int left = kSectionX - 46;
            rect(left, 25, 38, 26, mix(bar, muted, 120), 8);
            rect(left + 2, 27, 34, 22, bar, 7);
            text_mid(left + 19, 38, "L", 14, nav);
            for (int i = 0; i < kSectionCount; ++i) {
                const int x = kSectionX + i * kSectionStep;
                const bool on = i == active_section;
                if (on) rect(x, kSectionY, kSectionW, kSectionH, soft_accent, 22);
                text_mid(x + kSectionW / 2, kSectionY + kSectionH / 2, kSectionNames[i], 19, on ? accent : nav);
            }
            const int right = kSectionX + kSectionCount * kSectionStep + 4;
            rect(right, 25, 38, 26, mix(bar, muted, 120), 8);
            rect(right + 2, 27, 34, 22, bar, 7);
            text_mid(right + 19, 38, "R", 14, nav);
        }
        text_right(kWidth - 40, 30, "PAL v2.00  ·  SCES_509.16", 13, muted);
    }

    // The bottom bar: button hints, or a notice in their place while there is one.
    void footer(std::initializer_list<Hint> hints, const char* exit_label = "Exit", const std::string& notice = {}) {
        rect(0, kFooterY, kWidth, kHeight - kFooterY, bar);
        rect(0, kFooterY, kWidth, 1, line);
        const int cy = kFooterY + 29;
        int x = 40;
        if (!notice.empty()) {
            disc(x + 13, cy, 13, soft_accent);
            rect(x + 12, cy - 7, 3, 9, accent, 1);
            rect(x + 12, cy + 4, 3, 3, accent, 1);
            text(x + 36, mid_top(cy, 17), notice, 17, text_c, 940);
        }
        for (const Hint& h : hints) {
            if (!notice.empty()) break;
            key(h.key, x, cy);
            x += key_width(h.key) + 9;
            text(x, mid_top(cy, 16), h.label, 16, nav);
            x += width(h.label, 16) + 28;
        }
        if (exit_label) {
            const int w = width(exit_label, 16);
            key(Key::Plus, kWidth - 40 - w - 35, cy);
            text(kWidth - 40 - w, mid_top(cy, 16), exit_label, 16, nav);
        }
    }

    void status_icon(int cx, int cy, bool good) {
        disc(cx, cy, 22, good ? ok_bg : warn_bg);
        if (good) {
            check(cx, cy, 13, ok);
        } else {
            rect(cx - 1, cy - 9, 3, 11, warn, 1);
            rect(cx - 1, cy + 5, 3, 3, warn, 1);
        }
    }

    // ---- Page-list icons ----
    void page_icon(int tab, int cx, int cy, Color c) {
        switch (tab) {
            case 1:  // display
                rect(cx - 10, cy - 8, 20, 14, c, 3); rect(cx - 8, cy - 6, 16, 10, raised, 2);
                rect(cx - 4, cy + 9, 8, 2, c, 1);
                break;
            case 2:  // speaker
                rect(cx - 9, cy - 4, 5, 8, c, 1);
                triangle(cx - 5, cy - 4, cx + 2, cy - 9, cx + 2, cy + 9, c);
                triangle(cx - 5, cy - 4, cx + 2, cy + 9, cx - 5, cy + 4, c);
                segment(cx + 6, cy - 4, cx + 8, cy, 2, c); segment(cx + 8, cy, cx + 6, cy + 4, 2, c);
                break;
            case 3:  // gameplay: a clock face
                ring(cx, cy, 9, 2, c); segment(cx, cy - 5, cx, cy, 2, c); segment(cx, cy, cx + 4, cy + 3, 2, c);
                break;
            case 7:  // controller
                rect(cx - 11, cy - 6, 22, 13, c, 6); rect(cx - 9, cy - 4, 18, 9, raised, 4);
                rect(cx - 7, cy - 1, 6, 2, c, 1); rect(cx - 5, cy - 3, 2, 6, c, 1); disc(cx + 5, cy, 1.6f, c);
                break;
            case 4:  // wrench
                ring(cx + 3, cy - 3, 5, 2.4f, c); segment(cx - 8, cy + 8, cx, cy, 3, c);
                break;
            case kArtworkTab:  // picture
                rect(cx - 10, cy - 8, 20, 16, c, 3); rect(cx - 8, cy - 6, 16, 12, raised, 2);
                triangle(cx - 7, cy + 5, cx - 2, cy - 2, cx + 3, cy + 5, c); disc(cx + 4, cy - 2, 2, c);
                break;
            default:  // appearance: palette
                ring(cx, cy, 9, 2, c); disc(cx - 3, cy - 3, 1.8f, c); disc(cx + 3, cy - 3, 1.8f, c); disc(cx + 4, cy + 3, 1.8f, c);
                break;
        }
    }

    void background(int active_section, const char* centre_title = nullptr) {
        theme();
        std::fill(pixels.begin(), pixels.end(), ink | 0xff000000u);
        top_bar(active_section, centre_title);
    }

    // ---- Pages ----
    void play(const View& v) {
        const auto& style = v.appearance;
        const int x = kHeroX, y = kHeroY, w = kHeroW, h = kHeroH;
        card(x, y, w, h, style.banner_color, 24);
        if (v.banner_background) {
            image(*v.banner_background, x + 1, y + 1, w - 2, h - 2, true, 23);
            // The player's art, calmed, then a panel under the text so it stays readable.
            rect(x + 1, y + 1, w - 2, h - 2, style.banner_color, 23, unsigned(style.tint * 255 / 300));
        } else if (!v.window_art) {
            // No art chosen: a planet, so the banner is not empty.
            const float pcx = x + 920, pcy = y + 200, pr = 150;
            for (int yy = int(pcy - pr - 1); yy <= int(pcy + pr + 1); ++yy) {
                for (int xx = int(pcx - pr - 1); xx <= int(pcx + pr + 1); ++xx) {
                    const float d = std::hypot(xx + .5f - pcx, yy + .5f - pcy);
                    const float cover = std::clamp(pr - d + .5f, 0.f, 1.f);
                    if (cover <= 0) continue;
                    const float t = std::clamp(std::hypot(xx - (pcx - pr * .35f), yy - (pcy - pr * .4f)) / (pr * 1.5f), 0.f, 1.f);
                    const Color shade = t < .45f ? mix(rgb(255, 197, 110), rgb(224, 122, 52), unsigned(t / .45f * 255))
                                                 : mix(rgb(224, 122, 52), rgb(107, 42, 30), unsigned((t - .45f) / .55f * 255));
                    pixel(xx, yy, shade, unsigned(cover * 255));
                }
            }
            // Its ring, in front of the planet's lower half only.
            const float rc = std::cos(-0.24f), rs = std::sin(-0.24f), ra = 232, rb = 30;
            for (int yy = int(pcy - 90); yy <= int(pcy + 90); ++yy) {
                for (int xx = int(pcx - 260); xx <= int(pcx + 260); ++xx) {
                    const float px = xx + .5f - pcx, py = yy + .5f - pcy;
                    const float u = px * rc + py * rs, vv = -px * rs + py * rc;
                    const float e = std::sqrt((u * u) / (ra * ra) + (vv * vv) / (rb * rb));
                    const float cover = std::clamp(1.6f - std::fabs(e - 1) * rb + .5f, 0.f, 1.f);
                    if (cover <= 0 || xx >= x + w - 2 || (vv < 0 && std::hypot(px, py) < pr)) continue;
                    pixel(xx, yy, rgb(255, 197, 110), unsigned(cover * 150));
                }
            }
            disc(x + 1062, y + 318, 30, rgb(60, 127, 166));
            disc(x + 1054, y + 310, 16, rgb(159, 232, 255), 90);
            const int stars[][2] = {{690, 70}, {700, 330}, {1150, 70}, {940, 24}, {1170, 250}, {760, 220}, {790, 40}, {1130, 370}, {1180, 160}};
            for (const auto& s : stars) disc(x + s[0], y + s[1], 1.4f, rgb(221, 230, 255));
        }
        if (v.window_art) image(*v.window_art, x + 790, y + 30, 380, h - 60, false);
        // Readability panel on the left, fading out over the art.
        const unsigned scrim = unsigned(std::max(style.tint, 60) * 255 / 100);
        for (int col = 1; col < 761; ++col) {
            const unsigned a = col <= 600 ? scrim : unsigned(scrim * (760 - col) / 160);
            for (int row = 1; row < h - 1; ++row) {
                // Inside the card's round left corners only.
                float cover = 1;
                const float r = 23, cx = x + 1 + r, cy = row < h / 2 ? y + 1 + r : y + h - 1 - r;
                const float px = x + col + .5f, py = y + row + .5f;
                if (px < cx && (row < h / 2 ? py < cy : py > cy)) cover = std::clamp(r - std::hypot(px - cx, py - cy) + .5f, 0.f, 1.f);
                if (cover > 0) pixel(x + col, y + row, style.banner_color, unsigned(a * cover));
            }
        }

        const Color title_c = style.banner_text;
        const Color sub_c = mix(style.banner_color, title_c, 210);
        int ty = y + 40;
        if (!style.heading.empty()) { label(x + 48, ty, style.heading, accent, 14); ty += 24; }
        if (!style.title.empty()) { text(x + 46, ty, style.title, 60, title_c, 560, true); ty += 76; }
        // The subtitle's parts (split at '|') as chips.
        int cx = x + 48;
        std::string part;
        std::istringstream parts(style.subtitle);
        while (std::getline(parts, part, '|')) {
            part.erase(0, part.find_first_not_of(' '));
            part.erase(part.find_last_not_of(' ') + 1);
            if (part.empty()) continue;
            const int pw = width(part, 15) + 24;
            if (cx + pw > x + 620) break;
            rect(cx, ty, pw, 32, mix(style.banner_color, title_c, 34), 10);
            text(cx + 12, ty + 7, part, 15, sub_c);
            cx += pw + 8;
        }
        const bool ready = v.disc && v.executable && v.levels && v.writable;
        rect(kLaunchX, kLaunchY, kLaunchW, kLaunchH, accent, 18);
        disc(kLaunchX + 33, kLaunchY + 32, 19, on_accent);
        text_mid(kLaunchX + 33, kLaunchY + 32, "A", 22, accent);
        text(kLaunchX + 64, mid_top(kLaunchY + 32, 23), ready ? "Launch game" : "Check setup", 23, on_accent, kLaunchW - 74);
        if (v.disc && v.executable) {
            rect(kBootX, kLaunchY, kBootW, kLaunchH, mix(style.banner_color, title_c, 60), 18);
            rect(kBootX + 2, kLaunchY + 2, kBootW - 4, kLaunchH - 4, mix(style.banner_color, ink, 60), 16);
            ring(kBootX + 30, kLaunchY + 32, 15, 1.6f, mix(title_c, style.banner_color, 90));
            text_mid(kBootX + 30, kLaunchY + 32, "Y", 17, title_c);
            text(kBootX + 56, mid_top(kLaunchY + 32, 19), "Diagnostic boot", 19, title_c, kBootW - 66);
        }
        if (!v.banner_background && !v.window_art) {
            const std::string hint = "Banner art: Customize > Artwork";
            const int hw = width(hint, 13) + 24;
            rect(x + w - hw - 18, y + 18, hw, 30, ink, 10, 190);
            text(x + w - hw - 6, mid_top(y + 33, 13), hint, 13, muted);
        }

        // Readiness.
        const bool good[] = {v.disc && v.executable, v.levels, v.writable};
        const char* titles[] = {"DISC IMAGE", "GAME ASSETS", "SAVE STORAGE"};
        const char* fine[] = {"ISO found in /switch/openrac/", "All scenes prepared", "Write and read-back passed"};
        const char* bad[] = {"Copy your PAL ISO to /switch/openrac/", "Open Setup to prepare them", "Check the SD card"};
        const int cw = (kHeroW - 32) / 3;
        for (int i = 0; i < 3; ++i) {
            const int sx = kHeroX + i * (cw + 16);
            card(sx, kStatusY, cw, kStatusH, panel, 18, good[i] ? line : mix(line, warn, 90));
            status_icon(sx + 44, kStatusY + kStatusH / 2, good[i]);
            label(sx + 80, kStatusY + 25, titles[i], muted, 12);
            text(sx + 80, kStatusY + 46, good[i] ? "Ready" : "Needs attention", 22, good[i] ? text_c : warn);
            text(sx + 80, kStatusY + 78, good[i] ? fine[i] : bad[i], 14, muted, cw - 96);
        }
        footer({{Key::A, ready ? "Launch" : "Check setup"}, {Key::X, "Recheck installation"},
                {Key::Y, "Diagnostic boot"}, {Key::LR, "Sections"}}, "Exit", v.notice);
    }

    // Settings and Customize: the page list, the rows, and a panel about the selected row.
    void paged(const View& v) {
        const int section = section_of(v.tab);
        label(kPageX + 16, 102, section == 1 ? "SETTINGS" : "CUSTOMIZE", muted, 12);
        const auto pages = section_pages(section);
        for (std::size_t i = 0; i < pages.size(); ++i) {
            const int py = kPageY + int(i) * kPageStep;
            const bool on = pages[i] == v.tab;
            if (on) {
                if (v.pages_focus) rect(kPageX, py, kPageW, kPageH, accent, 14);
                rect(kPageX + (v.pages_focus ? 2 : 0), py + (v.pages_focus ? 2 : 0), kPageW - (v.pages_focus ? 4 : 0),
                     kPageH - (v.pages_focus ? 4 : 0), raised, v.pages_focus ? 12 : 14);
            }
            page_icon(pages[i], kPageX + 30, py + kPageH / 2, on ? accent : muted);
            text(kPageX + 56, mid_top(py + kPageH / 2, 19), page_name(pages[i]), 19, on ? text_c : nav, kPageW - 70);
        }

        // Rows.
        card(kRowsX, kRowsY, kRowsW, kRowsH, panel, 20);
        const int n = int(v.rows.size());
        const int first = first_visible_row(v.selected), end = std::min(n, first + kVisibleRows);
        const bool artwork = v.tab == kArtworkTab;
        for (int i = first; i < end; ++i) {
            const int ry = kRowY + (i - first) * kRowStep;
            const bool sel = i == v.selected && !v.pages_focus;
            if (sel) {
                rect(kRowX, ry, kRowW, kRowH, accent, 12);
                rect(kRowX + 2, ry + 2, kRowW - 4, kRowH - 4, raised, 10);
            }
            const auto& row = v.rows[i];
            const int mid = ry + kRowH / 2;
            text(kRowX + 18, mid_top(mid, 18), row.label, 18, sel ? text_c : mix(muted, text_c, 120), artwork ? 230 : 300);
            const auto& value = row.value;
            const int right = kRowX + kRowW - 18;
            if (value == "On" || value == "Off") {
                const bool on = value == "On";
                rect(right - 52, mid - 14, 52, 28, on ? accent : line, 14);
                disc(on ? right - 16 : right - 36, mid, 11, on ? on_accent : muted);
            } else if (sel) {
                const int vw = std::min(width(value, 17), artwork ? 260 : 200);
                chevron(right - 6, mid, 12, true, accent);
                text(right - 22 - vw, mid_top(mid, 17), value, 17, accent, vw);
                chevron(right - 34 - vw, mid, 12, false, accent);
            } else {
                text_right(right, mid_top(mid, 17), value, 17, text_c, artwork ? 270 : 230);
            }
            // A colour value also shows its colour.
            appearance::Color swatch = 0;
            if (value.size() == 7 && value[0] == '#' && appearance::parse_color(value, swatch)) {
                const int vw = std::min(width(value, 17), 230) + (sel ? 44 : 0);
                disc(right - vw - 18, mid, 9, line);
                disc(right - vw - 18, mid, 8, swatch);
            }
        }
        if (n > kVisibleRows) {
            const int track = kRowsH - 40;
            rect(kRowsX + kRowsW - 9, kRowsY + 20, 4, track, line, 2);
            const int thumb = std::max(40, track * kVisibleRows / n);
            const int top = (track - thumb) * first / std::max(1, n - kVisibleRows);
            rect(kRowsX + kRowsW - 9, kRowsY + 20 + top, 4, thumb, mix(line, text_c, 90), 2);
        }

        // About the selected row.
        card(kHelpX, kRowsY, kHelpW, kRowsH, panel, 20);
        const int hx = kHelpX + 26, hw = kHelpW - 52;
        int hy = kRowsY + 26;
        if (v.pages_focus || v.rows.empty()) {
            label(hx, hy, "PAGE", accent, 12);
            text(hx, hy + 22, page_name(v.tab), 28, text_c, hw, true);
            wrap(hx, hy + 70, "Press A or Right to change these settings. Up / Down picks another page.", 16, nav, hw, 4);
        } else {
            const auto& row = v.rows[std::min(v.selected, n - 1)];
            label(hx, hy, "SELECTED", accent, 12);
            const int lines = wrap(hx, hy + 22, row.label, 26, text_c, hw, 2, 32);
            hy += 22 + lines * 32 + 8;
            if (artwork && v.art) {
                image(*v.art, hx, hy, hw, 150, v.custom_art, 12);
                hy += 164;
                wrap(hx, hy, "Your art stays cached offline. Your API key stays on your SD card.", 15, nav, hw, 3);
            } else if (!row.help.empty()) {
                wrap(hx, hy, row.help, 16, nav, hw, 6);
            } else {
                wrap(hx, hy, "Left / Right changes it. Settings are saved as you go.", 16, nav, hw, 3);
            }
        }
        if (!v.notice.empty()) {
            std::string lower = v.notice;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return char(std::tolower(c)); });
            const bool good = lower.find("not") == std::string::npos && lower.find("fail") == std::string::npos
                              && lower.find("could") == std::string::npos && lower.find("error") == std::string::npos;
            const int by = kRowsY + kRowsH - 92;
            rect(hx - 8, by, hw + 16, 72, good ? ok_bg : warn_bg, 12);
            if (good) check(hx + 12, by + 22, 11, ok);
            else { rect(hx + 11, by + 13, 3, 11, warn, 1); rect(hx + 11, by + 27, 3, 3, warn, 1); }
            wrap(hx + 30, by + 12, v.notice, 14, text_c, hw - 34, 3, 19);
        }

        if (v.pages_focus) {
            footer({{Key::UpDown, "Choose page"}, {Key::A, "Open"}, {Key::B, "Play"}, {Key::LR, "Sections"}});
        } else if (v.tab == 7) {
            footer({{Key::LeftRight, "Change"}, {Key::A, "Rebind"}, {Key::X, "Clear"}, {Key::B, "Pages"}, {Key::LR, "Sections"}});
        } else if (artwork) {
            footer({{Key::A, "Select"}, {Key::LeftRight, "Browse"}, {Key::B, "Cancel / Pages"}, {Key::LR, "Sections"}});
        } else if (v.tab == kAppearanceTab) {
            footer({{Key::A, "Edit"}, {Key::LeftRight, "Change"}, {Key::Y, "Preview Play"}, {Key::B, "Pages"}, {Key::LR, "Sections"}});
        } else {
            footer({{Key::LeftRight, "Change"}, {Key::UpDown, "Move"}, {Key::B, "Pages"}, {Key::LR, "Sections"}});
        }
    }

    void numbered(int x, int y, int n, const char* title, const char* body_text, int w) {
        disc(x + 18, y + 18, 18, soft_accent);
        text_mid(x + 18, y + 18, std::to_string(n), 17, accent);
        text(x + 52, y, title, 20, text_c, w - 52);
        wrap(x + 52, y + 30, body_text, 15, nav, w - 52, 2, 21);
    }

    void saves(const View& v) {
        card(40, kRowsY, 700, kRowsH, panel, 22);
        // A memory card.
        rect(76, 132, 92, 112, mix(panel, text_c, 40), 14);
        rect(78, 134, 88, 108, raised, 12);
        rect(90, 146, 64, 40, accent, 6);
        text_mid(122, 166, "1", 22, on_accent, true);
        rect(90, 222, 64, 8, line, 4);
        label(192, 140, "MEMORY CARD 1", accent, 13);
        text(190, 162, v.writable ? "Your progress has a home." : "Save storage needs attention.", 32, text_c, 520, true);
        text(192, 210, "/switch/openrac/memcard/SCES_509.16/", 15, muted, 520);
        const int cw = (700 - 72 - 14) / 2;
        card(76, 276, cw, 92, sunk, 16);
        text(96, 294, "Storage check", 14, muted);
        if (v.writable) check(106, 334, 11, ok);
        else { rect(104, 324, 3, 11, warn, 1); rect(104, 338, 3, 3, warn, 1); }
        text(124, 322, v.writable ? "Write and read-back passed" : "Unable to write", 18, v.writable ? ok : warn, cw - 60);
        card(76 + cw + 14, 276, cw, 92, sunk, 16);
        text(96 + cw + 14, 294, "Stored files", 14, muted);
        text(96 + cw + 14, 322, std::to_string(v.save_files), 22, text_c);
        const int iy = kRowsY + kRowsH - 86;
        rect(76, iy, 628, 50, raised, 14);
        ring(100, iy + 25, 9, 1.8f, nav);
        rect(99, iy + 23, 2, 7, nav, 1);
        rect(99, iy + 19, 2, 2, nav, 1);
        text(122, mid_top(iy + 25, 15), "Updating the NRO keeps your saves.", 15, text_c);

        card(760, kRowsY, 480, kRowsH, panel, 22);
        label(794, 130, "SAVING SAFELY", muted, 13);
        numbered(794, 170, 1, "Save from the game's menu", "Use the in-game Save option, as on the PS2.", 412);
        numbered(794, 270, 2, "Wait for it to finish", "Don't close the game while the save message is showing.", 412);
        numbered(794, 370, 3, "Exit normally", "Quit from the game or with +, not from the HOME menu.", 412);
        footer({{Key::X, "Recheck storage"}, {Key::B, "Play"}, {Key::LR, "Sections"}}, "Exit", v.notice);
    }

    void setup(const View& v) {
        card(40, kRowsY, 1200, kRowsH, panel, 22);
        label(88, 130, "NRO + YOUR ISO. WE HANDLE THE REST.", accent, 13);
        text(86, 152, v.disc && v.executable && v.levels ? "Your game is prepared." : "Set up your game", 34, text_c, 1000, true);
        numbered(88, 230, 1, "Copy your supported PAL ISO", "Put one ISO beside the NRO in /switch/openrac/.", 1050);
        numbered(88, 316, 2, "Prepare directly on Switch", "Use full application mode and keep 2 GiB of extra free space.", 1050);
        numbered(88, 402, 3, "Start your adventure", "Leave the ISO in place. Completed scenes resume after interruption.", 1050);
        rect(kSetupButtonX, kSetupButtonY, kSetupButtonW, kSetupButtonH, accent, 18);
        disc(kSetupButtonX + 32, kSetupButtonY + 30, 18, on_accent);
        text_mid(kSetupButtonX + 32, kSetupButtonY + 30, "A", 21, accent);
        text(kSetupButtonX + 62, mid_top(kSetupButtonY + 30, 22), "Prepare / resume", 22, on_accent);
        footer({{Key::A, "Prepare / resume"}, {Key::X, "Recheck"}, {Key::B, "Play"}, {Key::LR, "Sections"}}, "Exit", v.notice);
    }

    void about(const View& v) {
        card(40, kRowsY, 1200, kRowsH, panel, 22);
        bolt(88, 132, 44, accent);
        text(146, 128, std::string("OpenRAC-NX ") + switch_build::kLabel, 36, text_c, 900, true);
        text(148, 178, "Independent Nintendo Switch port", 19, accent);
        wrap(88, 236, "Built on OpenRAC's native game port and the Ratchet & Clank decompilation projects. "
             "Thank you to their developers and contributors.", 19, text_c, 1000, 3, 28);
        text(88, 320, "Upstream: github.com/OpenRAC/OpenRAC", 18, accent);
        text(88, 352, "Switch fork: github.com/BIGGZ22/OpenRAC-NX", 18, nav);
        wrap(88, 404, "An unofficial fan project, independently maintained. Not affiliated with or endorsed "
             "by Sony Interactive Entertainment or Insomniac Games.", 16, muted, 1000, 2, 24);
        label(88, 488, "BASED ON", muted, 12);
        upstream(88, 510);
        footer({{Key::B, "Play"}, {Key::LR, "Sections"}}, "Exit", v.notice);
    }
};

Canvas::Canvas() : p(std::make_unique<Impl>()) {}

Canvas::~Canvas() = default;

bool Canvas::font(std::span<const std::uint8_t> bytes) {
    auto& f = p->body;
    f.data.assign(bytes.begin(), bytes.end());
    p->glyphs.clear();
    f.ok = !f.data.empty() && stbtt_InitFont(&f.info, f.data.data(), stbtt_GetFontOffsetForIndex(f.data.data(), 0));
    return f.ok;
}

bool Canvas::heading_font(std::span<const std::uint8_t> bytes) {
    auto& f = p->display;
    f.data.assign(bytes.begin(), bytes.end());
    p->glyphs.clear();
    f.ok = !f.data.empty() && stbtt_InitFont(&f.info, f.data.data(), stbtt_GetFontOffsetForIndex(f.data.data(), 0));
    return f.ok;
}

const std::vector<std::uint32_t>& Canvas::pixels() const {
    return p->pixels;
}

void Canvas::menu(const View& v) {
    p->appearance_settings = v.appearance;
    const int section = section_of(v.tab);
    p->background(section);
    if (v.tab == 0) {
        p->play(v);
    } else if (section == 1 || section == 3) {
        p->paged(v);
    } else if (v.tab == 5) {
        p->saves(v);
    } else if (v.tab == 6) {
        p->setup(v);
    } else {
        p->about(v);
    }
}

void Canvas::progress(const std::string& stage, unsigned done, unsigned total) {
    p->background(-1, "Preparing your game");
    const int x = 210, y = 150, w = 860, h = 400;
    p->card(x, y, w, h, p->panel, 26);
    p->disc(x + 84, y + 84, 28, p->soft_accent);
    p->bolt(x + 70, y + 68, 30, p->accent);
    p->label(x + 130, y + 56, "FIRST-LAUNCH PREPARATION", p->accent, 13);
    p->wrap(x + 128, y + 78, stage, 30, p->text_c, w - 180, 2, 36);
    const float fraction = total ? std::clamp(float(done) / float(total), 0.0f, 1.0f) : 0;
    p->rect(x + 56, y + 184, w - 112, 16, p->line, 8);
    if (fraction > 0) p->rect(x + 56, y + 184, std::max(16, int((w - 112) * fraction)), 16, p->accent, 8);
    if (total) {
        p->text(x + 56, y + 214, std::to_string(done) + " of " + std::to_string(total), 18, p->text_c);
        p->text_right(x + w - 56, y + 214, std::to_string(int(fraction * 100)) + "%", 18, p->accent);
    } else {
        p->text(x + 56, y + 214, "Working...", 18, p->text_c);
    }
    const int cw = (w - 112 - 14) / 2;
    p->card(x + 56, y + 268, cw, 92, p->sunk, 16);
    p->wrap(x + 76, y + 290, "Keep the console powered. Docked or plugged in is best.", 15, p->nav, cw - 40, 2, 21);
    p->card(x + 70 + cw, y + 268, cw, 92, p->sunk, 16);
    p->check(x + 92 + cw, y + 300, 11, p->ok);
    p->wrap(x + 112 + cw, y + 290, "Your ISO stays in place. Finished scenes are kept if you stop.", 15, p->nav, cw - 76, 2, 21);
    p->footer({}, "Cancel and return to the menu");
}

void Canvas::binding(const std::string& action, const std::string& prompt) {
    p->background(1, nullptr);
    const int x = 210, y = 150, w = 860, h = 400;
    p->card(x, y, w, h, p->panel, 26);
    p->label(x + 48, y + 40, "REBIND CONTROLLER BUTTON", p->accent, 13);
    p->text(x + 46, y + 64, action, 34, p->text_c, w - 96, true);
    p->wrap(x + 48, y + 124, prompt, 24, p->text_c, w - 96, 2, 32);
    p->wrap(x + 48, y + 210, "Press + and - together to cancel. This screen times out after 20 seconds. "
            "Occupied buttons swap assignments.", 17, p->nav, w - 96, 3, 25);
    p->rect(kCancelX, kCancelY, kCancelW, kCancelH, p->line, 16);
    p->rect(kCancelX + 2, kCancelY + 2, kCancelW - 4, kCancelH - 4, p->raised, 14);
    p->text_mid(kCancelX + kCancelW / 2, kCancelY + kCancelH / 2, "Cancel", 20, p->text_c);
    p->footer({}, nullptr);
}
}  // namespace openrac::platform::launcher_ui
