// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the extractor tells whoever runs it (report.h): the text of each numbered error, the
// Reporter that prints information, progress and failures either as lines for a person or as one
// JSON object per line for the launcher, and the JSON string escaping the lines need.

#include "extract/report.h"

#include <format>

namespace openrac::extract {

namespace {

// The stage's name as the launcher's progress lines spell it.
std::string_view stage_name(Stage stage) {
    // One name per stage; the compiler warns here when a stage is added without one.
    switch (stage) {
        case Stage::Identify:
            return "identify";
        case Stage::Copy:
            return "copy";
        case Stage::Decompile:
            return "decompile";
    }

    return "unknown";
}

// The text of an error as printed: the number, what it means, and the detail of this failure.
std::string message_of(ErrorCode code, const std::string& detail) {
    std::string text = std::format("error {}: {}", static_cast<int>(code), error_text(code));

    // Most failures have a detail (a path, a count); some have only the meaning.
    if (!detail.empty()) {
        text += std::format(" ({})", detail);
    }

    return text;
}

}  // namespace

std::string_view error_text(ErrorCode code) {
    // Each code has one sentence a player can act on, written for the person and not for a
    // programmer.
    switch (code) {
        case ErrorCode::NoBootExecutable:
            return "the image has no boot executable (no SYSTEM.CNF, or the file it names is "
                   "missing)";
        case ErrorCode::UnknownVersion:
            return "the image is not a version of this game that OpenRAC knows";
        case ErrorCode::UnknownBuild:
            return "the boot executable is not the build OpenRAC knows for this serial (a "
                   "different revision, or a damaged image)";
        case ErrorCode::WrongFileCount:
            return "the image does not hold the files this build should have";
        case ErrorCode::WrongContents:
            return "the disc's files are not those of this build (a damaged or modified image)";
        case ErrorCode::NotIso:
            return "the image is not an ISO 9660 disc image";
        case ErrorCode::DecompileFailed:
            return "the disc's data is not what this build's layout says (a damaged image)";
        case ErrorCode::NotImageFile:
            return "the file is not a disc image (.iso)";
        case ErrorCode::ImageTooSmall:
            return "the image is too small to be a PlayStation 2 disc";
        case ErrorCode::NotYet:
            return "this step does not exist yet: the native port is being built "
                   "(docs/port/ROADMAP.md)";
        case ErrorCode::NoSpace:
            return "there is not enough free space where the files go";
        case ErrorCode::CannotWrite:
            return "a file or folder could not be written";
        case ErrorCode::CannotRead:
            return "the image could not be read (unreadable, or truncated)";
    }

    return "unknown error";
}

ExtractError::ExtractError(ErrorCode code, std::string detail)
    : std::runtime_error(message_of(code, detail)),
      m_code(code),
      m_detail(std::move(detail)) {}

Reporter::Reporter(bool json, std::FILE* out, std::FILE* err)
    : m_json(json),
      m_out(out),
      m_err(err),
      m_start(std::chrono::steady_clock::now()) {}

// Writes one line to `to` and flushes it at once, so a launcher reading the pipe sees it as it
// happens.
void Reporter::line(std::FILE* to, const std::string& text) {
    std::fputs(text.c_str(), to);
    std::fputc('\n', to);
    std::fflush(to);
}

void Reporter::info(std::string_view message) {
    // The launcher gets a JSON line, a person the bare message.
    if (m_json) {
        line(m_out, std::format("{{\"type\":\"info\",\"message\":{}}}", json_string(message)));
    } else {
        line(m_out, std::string(message));
    }
}

void Reporter::progress(
    Stage stage, std::uint64_t done, std::uint64_t total, std::string_view file
) {
    // A person reads the info lines instead of progress.
    if (!m_json) {
        return;  // a person reads the info lines
    }

    const auto now = std::chrono::steady_clock::now();

    // Progress lines come at most every 100 ms, except the last one (done equals total), so a fast
    // copy does not flood the pipe.
    if (done != total && now - m_last_progress < std::chrono::milliseconds(100)) {
        return;
    }

    m_last_progress = now;
    line(
        m_out,
        std::format(
            "{{\"type\":\"progress\",\"stage\":\"{}\",\"done\":{},\"total\":{},\"file\":{}}}",
            stage_name(stage),
            done,
            total,
            json_string(file)
        )
    );
}

void Reporter::disc(const DiscReport& d) {
    // The launcher gets a disc line; a person gets a one-line summary of what was identified.
    if (m_json) {
        line(
            m_out,
            std::format(
                "{{\"type\":\"disc\",\"serial\":{},\"version\":{},\"region\":{},\"game\":{},"
                "\"title\":{},\"supported\":{},\"boot_sha1\":{}}}",
                json_string(d.serial),
                json_string(d.version),
                json_string(d.region),
                json_string(d.game),
                json_string(d.title),
                d.supported ? "true" : "false",
                json_string(d.boot_sha1)
            )
        );
    } else if (!d.version.empty()) {
        line(m_out, std::format("{}: {} ({}, {})", d.serial, d.title, d.region, d.version));
    } else {
        line(m_out, std::format("{}: {}, a build OpenRAC does not know", d.serial, d.title));
    }
}

void Reporter::error(const ExtractError& e) {
    // The launcher gets the code and the message as separate fields; a person gets the whole line
    // on the error stream.
    if (m_json) {
        line(
            m_out,
            std::format(
                "{{\"type\":\"error\",\"code\":{},\"message\":{}}}",
                static_cast<int>(e.code()),
                json_string(
                    std::string_view(e.what()).substr(std::string_view(e.what()).find(": ") + 2)
                )
            )
        );
    } else {
        line(m_err, e.what());
    }
}

void Reporter::done() {
    // Only the launcher's stream ends with a done line, with the time the run took.
    if (m_json) {
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_start
        );
        line(m_out, std::format("{{\"type\":\"done\",\"elapsed_ms\":{}}}", ms.count()));
    }
}

std::string json_string(std::string_view text) {
    std::string out = "\"";

    // Every character of the text is escaped as JSON needs.
    for (const char c : text) {
        // The characters JSON must escape by name, or as \u for the other control characters.
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                // The other control characters (below 0x20) can only be written as \u escapes.
                if (static_cast<unsigned char>(c) < 0x20) {
                    out += std::format("\\u{:04x}", static_cast<unsigned>(c));
                } else {
                    out += c;
                }
        }
    }

    out += '"';
    return out;
}

}  // namespace openrac::extract
