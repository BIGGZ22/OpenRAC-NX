// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Every run's logs gathered in one folder on the SD card, for copying after a test:
//
//   sdmc:/switch/openrac/test-logs/<date>_<time>/   this run's files
//   sdmc:/switch/openrac/test-logs/latest/          the same files of the newest run that ended
//   sdmc:/switch/openrac/test-logs/LATEST.txt       the newest folder's name
//
// The game's live log (<game>.log beside the startup logs), env-applied.log, deko3d-startup.log and, from the logs that
// keep growing over runs (overlay-status.log, dkgl-driver.log, deko3d-runtime.log and the Vulkan
// ones), only what this run added. Copied at a clean end (the program's exit: the HOME menu's
// close, L+R+Minus, the game's own quit). A run that ended without one (a crash, the power) has
// its files copied into its folder when the next run starts, with a NOTE.txt saying so. The 20
// newest run folders are kept. OPENRAC_TEST_LOGS=0 turns it off. Header-only: game/common/main.cpp
// calls begin() on the Switch before the log of the run is opened.

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace openrac::test_logs {

namespace fs = std::filesystem;

inline constexpr std::size_t kKeepRuns = 20;

inline const fs::path& root() {
#ifdef OPENRAC_TEST_LOG_ROOT
    // Compile-time host fixture path; production always uses the SD directory.
    static const fs::path r = OPENRAC_TEST_LOG_ROOT;
#else
    static const fs::path r = "sdmc:/switch/openrac/test-logs";
#endif
    return r;
}

inline bool managed_run(const fs::path& path) {
    if(path.lexically_normal().parent_path()!=root().lexically_normal()) return false;
    const auto name=path.filename().string();
    if(name.size()<19) return false;
    for(std::size_t i=0;i<name.size();++i) {
        const char expected=i==4 || i==7 || i==13 || i==16 ? '-' : i==10 || i==19 ? '_' : 0;
        if(expected ? name[i]!=expected : name[i]<'0' || name[i]>'9') return false;
    }
    if(name.size()==20) return false;
    std::error_code ec;
    return !fs::is_symlink(fs::symlink_status(path,ec));
}

// The logs of the folder sdmc:/switch/openrac itself; `grows`: written in append mode across runs,
// so only the part a run added is copied.
struct Source {
    const char* name;
    bool grows;
};
inline constexpr Source kSources[] = {
    {"env-applied.log", false},  {"deko3d-startup.log", false}, {"vulkan-startup.log", false},
    {"overlay-status.log", true}, {"dkgl-driver.log", true},     {"deko3d-runtime.log", true},
    {"vulkan-runtime.log", true},
};

struct Run {
    bool active = false;
    fs::path dir;       // this run's folder
    fs::path game_log;  // the game's live log, beside the startup and overlay logs
    std::vector<std::uintmax_t> start;  // each growing source's size when the run began
};

inline Run& run() {
    static Run r;
    return r;
}

inline std::uintmax_t size_of(const fs::path& p) {
    std::error_code ec;
    const auto n = fs::file_size(p, ec);
    return ec ? 0 : n;
}

inline std::vector<std::uintmax_t>& early_offsets() {
    static std::vector<std::uintmax_t> values;
    return values;
}

// Capture append-log boundaries before the launcher/overlay starts writing.
inline void prepare() {
    auto& values=early_offsets();
    values.clear();
    for(const auto& source:kSources)
        values.push_back(source.grows ? size_of(root().parent_path()/source.name) : 0);
}

// Copies `from` past its first `skip` bytes into `to` (all of it when it is shorter than that:
// a file that was removed and written again).
inline void copy_tail(const fs::path& from, const fs::path& to, std::uintmax_t skip,
                      std::uintmax_t end=std::numeric_limits<std::uintmax_t>::max()) {
    std::ifstream in(from, std::ios::binary);
    if (!in) {
        return;
    }
    const auto size=size_of(from);
    if(skip>size) skip=0;
    if (skip > 0) {
        in.seekg(static_cast<std::streamoff>(skip));
    }
    std::ofstream out(to, std::ios::binary | std::ios::trunc);
    auto remaining=std::min(size,end)>skip ? std::min(size,end)-skip : 0;
    char buffer[16384];
    while(remaining && in) {
        in.read(buffer,static_cast<std::streamsize>(std::min<std::uintmax_t>(remaining,sizeof(buffer))));
        const auto got=in.gcount();
        if(got<=0) break;
        out.write(buffer,got);
        remaining-=std::uintmax_t(got);
    }
}

inline void copy_run(const fs::path& game_log, const fs::path& to, const std::vector<std::uintmax_t>& start,
                     const std::vector<std::uintmax_t>& end={}) {
    std::error_code ec;
    fs::create_directories(to, ec);
    if (ec) {
        return;
    }
    if (fs::exists(game_log, ec)) {
        fs::copy_file(game_log, to / game_log.filename(), fs::copy_options::overwrite_existing, ec);
    }
    const fs::path home = root().parent_path();
    for (std::size_t i = 0; i < std::size(kSources); ++i) {
        const fs::path from = home / kSources[i].name;
        if (!fs::exists(from, ec)) {
            continue;
        }
        copy_tail(from, to / kSources[i].name, kSources[i].grows && i < start.size() ? start[i] : 0,
                  kSources[i].grows && i<end.size() ? end[i] : std::numeric_limits<std::uintmax_t>::max());
    }
}

// Keeps the kKeepRuns newest run folders (their names sort by date and time).
inline void prune() {
    std::error_code ec;
    std::vector<fs::path> runs;
    for (const auto& entry : fs::directory_iterator(root(), ec)) {
        if (entry.is_directory(ec) && managed_run(entry.path())) {
            runs.push_back(entry.path());
        }
    }
    std::sort(runs.begin(), runs.end());
    for (std::size_t i = 0; i + kKeepRuns < runs.size(); ++i) {
        fs::remove_all(runs[i], ec);
    }
}

// This run's files into its folder, and into latest/. Safe to call more than once.
inline void archive() {
    Run& r = run();
    if (!r.active) {
        return;
    }
    copy_run(r.game_log, r.dir, r.start);
    std::error_code ec;
    std::ofstream(r.dir / "COMPLETE") << "copied at the end of the run\n";
    const fs::path latest = root() / "latest";
    // Only replace our known archive files, never recursively remove unrelated
    // user content or follow an unexpected latest symlink.
    if(fs::is_symlink(fs::symlink_status(latest,ec))) return;
    fs::remove(latest / r.game_log.filename(),ec);
    for(const auto& source:kSources) fs::remove(latest / source.name,ec);
    copy_run(r.game_log, latest, r.start);
    std::ofstream(root() / "LATEST.txt") << r.dir.filename().string() << "\n";
}

// "2026-10-30 02:41:59 -0400" -> "2026-10-30_02-41-59".
inline std::string folder_name(std::string_view started) {
    std::string name(started.substr(0, std::min<std::size_t>(started.size(), 19)));
    for (char& c : name) {
        if (c == ' ') {
            c = '_';
        } else if (c == ':') {
            c = '-';
        }
    }
    return name.empty() ? std::string("run") : name;
}

// At the start of a run, before its log is opened (the last run's log is still `game_log` then).
inline void begin(const fs::path& game_log, std::string_view started) {
    const char* off = std::getenv("OPENRAC_TEST_LOGS");
    if (off != nullptr && std::string_view(off) == "0") {
        return;
    }
    std::error_code ec;
    fs::create_directories(root(), ec);
    if (ec) {
        return;
    }
    const fs::path home = root().parent_path();
    // The last run, if it did not end cleanly: its files now, before this run's replace them.
    {
        std::ifstream last(root() / "last-run.txt");
        std::string dir;
        std::getline(last, dir);
        std::vector<std::uintmax_t> start;
        for (std::uintmax_t n = 0; last >> n;) {
            start.push_back(n);
        }
        if (!dir.empty() && managed_run(fs::path(dir)) && fs::exists(dir, ec) && !fs::exists(fs::path(dir) / "COMPLETE", ec)) {
            copy_run(game_log, dir, start,early_offsets());
            std::ofstream(fs::path(dir) / "NOTE.txt")
                << "Copied when the next run started: this run did not end cleanly (a crash, the power, or a forced close).\n";
        }
    }
    Run& r = run();
    r.dir = root() / folder_name(started);
    for (int n = 2; fs::exists(r.dir, ec); ++n) {
        r.dir = root() / (folder_name(started) + "_" + std::to_string(n));
    }
    fs::create_directories(r.dir, ec);
    if (ec) {
        return;
    }
    r.game_log = game_log;
    r.start.clear();
    for (const Source& s : kSources) {
        r.start.push_back(s.grows ? size_of(home / s.name) : 0);
    }
    if(early_offsets().size()==std::size(kSources)) r.start=early_offsets();
    early_offsets().clear();
    {
        std::ofstream last(root() / "last-run.txt", std::ios::trunc);
        last << r.dir.string() << "\n";
        for (const auto n : r.start) {
            last << n << "\n";
        }
    }
    prune();
    r.active = true;
    std::atexit([] { archive(); });
}

}  // namespace openrac::test_logs
