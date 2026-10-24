// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/strings.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Text blocks of the gameplay file.

#include "assets/disc/messages.h"

#include <cstring>

namespace openrac::assets::disc {

std::vector<Message> parse_text_block(ByteView block) {
    const u32 count = block.u32_at(0);
    const u32 size = block.u32_at(4);

    // A count beyond 0x10000 cannot be real, so the block is damaged; stop before allocating.
    if (count > 0x10000) {
        fail("implausible text block count {}", count);
    }

    const ByteView body = block.sub(0, std::max<std::size_t>(size, 8), "text block");
    std::vector<Message> out;
    out.reserve(count);

    // Read one 16-byte entry per message: text offset, id and audio id.
    for (u32 k = 0; k < count; ++k) {
        const std::size_t e = 8 + 16 * std::size_t{k};
        const s32 off = body.s32_at(e);
        const s32 id = body.s32_at(e + 4);
        const s32 audio = body.s32_at(e + 8);

        // A negative offset would wrap to a huge unsigned one and point outside the block.
        if (off < 0) {
            fail("negative text offset in message {}", k);
        }

        const ByteView tail = body.tail(static_cast<std::size_t>(off), "message text");
        const void* nul = std::memchr(tail.data(), 0, tail.size());

        // Each message's text ends at a NUL; one without it would run to the end of the block.
        if (!nul) {
            fail("message text without a NUL");
        }

        const std::size_t length = static_cast<const u8*>(nul) - tail.data();
        out.push_back({id, std::string(reinterpret_cast<const char*>(tail.data()), length), audio});
    }

    return out;
}

std::vector<Message> parse_messages(ByteView gameplay, u32 language) {
    // Only the languages the file has slots for are valid.
    if (language >= kLanguageSlots) {
        fail("language {} out of range", language);
    }

    const u32 off = gameplay.u32_at(0x10 + 4 * std::size_t{language});
    return parse_text_block(gameplay.tail(off, "text block"));
}

std::optional<std::size_t> find_message(const std::vector<Message>& messages, s32 id) {
    // Search linearly: a level has few messages and the ids are not sorted.
    for (std::size_t i = 0; i < messages.size(); ++i) {
        // Message ids are unique within a language, so the first match is the one wanted.
        if (messages[i].id == id) {
            return i;
        }
    }

    return std::nullopt;
}

std::string_view message_text(const std::vector<Message>& messages, s32 id) {
    const auto i = find_message(messages, id);
    return i ? std::string_view(messages[*i].text) : kMissingMessage;
}

std::string printable(std::string_view text) {
    std::string s;

    // Escape the text one character at a time so it prints on a single line.
    for (const char ch : text) {
        const auto c = static_cast<unsigned char>(ch);

        // Plain ASCII passes through; anything else, and the backslash itself, is escaped as hex.
        if (c >= 0x20 && c < 0x7f && c != '\\') {
            s.push_back(ch);
        } else {
            s += std::format("\\x{:02x}", c);
        }
    }

    return s;
}

}  // namespace openrac::assets::disc
