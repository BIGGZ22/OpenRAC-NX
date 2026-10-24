// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "ui_colors.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <optional>

#include "common/env.h"

namespace openrac::ui_colors {
namespace {

// "#RRGGBB" or "#RRGGBBAA".
std::optional<Color> hex(std::string_view text) {
    if ((text.size() != 7 && text.size() != 9) || text.front() != '#') {
        return std::nullopt;
    }
    std::uint8_t bytes[4] = {0, 0, 0, 255};
    for (std::size_t i = 0; i + 1 < text.size(); i += 2) {
        const char* first = text.data() + 1 + i;
        const auto [end, error] = std::from_chars(first, first + 2, bytes[i / 2], 16);
        if (error != std::errc{} || end != first + 2) {
            return std::nullopt;
        }
    }
    return rgba(bytes[0], bytes[1], bytes[2], bytes[3]);
}

Color* field(Colors& c, std::string_view name) {
    struct Named {
        std::string_view name;
        Color Colors::* member;
    };

    static constexpr Named kNames[] = {
        {"ink-deep", &Colors::ink_deep},
        {"panel", &Colors::panel},
        {"panel-hi", &Colors::panel_hi},
        {"brand", &Colors::brand},
        {"bar-top", &Colors::bar_top},
        {"bar-bottom", &Colors::bar_bottom},
        {"bar-ink-deep", &Colors::bar_ink},
        {"amber", &Colors::amber},
        {"gold", &Colors::gold},
        {"text", &Colors::text},
        {"soft", &Colors::soft},
        {"dim", &Colors::dim},
        {"line", &Colors::line},
        {"line-strong", &Colors::line_strong},
        {"err", &Colors::err},
    };
    const auto* found = std::find_if(std::begin(kNames), std::end(kNames), [&](const Named& n) {
        return n.name == name;
    });
    return found != std::end(kNames) ? &(c.*found->member) : nullptr;
}

std::uint8_t channel(Color c, int shift) {
    return static_cast<std::uint8_t>(c >> shift & 0xFF);
}

}  // namespace

Colors parse(std::string_view text) {
    Colors c;
    while (!text.empty()) {
        const std::size_t comma = std::min(text.find(','), text.size());
        const std::string_view pair = text.substr(0, comma);
        text.remove_prefix(std::min(comma + 1, text.size()));
        const std::size_t equals = pair.find('=');
        if (equals == std::string_view::npos) {
            continue;
        }
        Color* slot = field(c, pair.substr(0, equals));
        const std::optional<Color> value = hex(pair.substr(equals + 1));
        if (slot != nullptr && value) {
            *slot = *value;
        }
    }
    return c;
}

const Colors& launcher() {
    static const Colors colors = parse(env::value(env::Var::UiColors).value_or(""));
    return colors;
}

Color mix(Color a, Color b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    const auto at = [&](int shift) {
        const float from = channel(a, shift);
        return static_cast<std::uint8_t>(std::lround(from + (channel(b, shift) - from) * t));
    };
    return rgba(at(0), at(8), at(16), at(24));
}

Color fade(Color c, float t) {
    const float alpha = channel(c, 24) * std::clamp(t, 0.0f, 1.0f);
    return (c & 0x00FFFFFFu) | static_cast<Color>(std::lround(alpha)) << 24;
}

}  // namespace openrac::ui_colors
