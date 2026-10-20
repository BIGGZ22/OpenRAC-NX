// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The log's sessions and memory of last lines (common/log.h, common/line_ring.h) and the crash
// report's pieces (common/crash.h).

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include "check.h"
#include "common/crash.h"
#include "common/line_ring.h"
#include "common/log.h"

namespace fs = std::filesystem;
using namespace openrac;

namespace {

std::string read(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::stringstream s;
    s << in.rdbuf();
    return s.str();
}

void ring_keeps_the_last_lines() {
    log::LineRing<3, 8> ring;
    CHECK(ring.size() == 0);
    ring.push("one");
    ring.push("two");
    std::vector<std::string> seen;
    ring.each([&](const char* t, std::size_t n) { seen.emplace_back(t, n); });
    CHECK(seen.size() == 2 && seen[0] == "one" && seen[1] == "two");
    ring.push("three");
    ring.push("a line longer than eight");  // cut to 7 bytes
    seen.clear();
    ring.each([&](const char* t, std::size_t n) { seen.emplace_back(t, n); });
    CHECK(seen.size() == 3);
    CHECK(seen[0] == "two" && seen[1] == "three" && seen[2] == "a line ");
    ring.clear();
    CHECK(ring.size() == 0);
}

void sessions_rotate(const fs::path& dir) {
    // Six runs with five kept: the oldest goes, each other moves along one.
    for (int run = 1; run <= 7; ++run) {
        CHECK(log::open_session(dir, "openrac-test", 5));
        log::header(std::string("session ") + std::to_string(run));
        log::info("run {}", run);
    }
    log::set_file(nullptr);
    CHECK(read(log::session_file(dir, "openrac-test", 0)).find("run 7") != std::string::npos);
    CHECK(read(log::session_file(dir, "openrac-test", 0)).find("run 6") == std::string::npos);
    CHECK(read(log::session_file(dir, "openrac-test", 1)).find("run 6") != std::string::npos);
    CHECK(read(log::session_file(dir, "openrac-test", 5)).find("run 2") != std::string::npos);
    CHECK(!fs::exists(log::session_file(dir, "openrac-test", 6)));
}

void home_reads_tilde() {
    const std::string& h = log::home();
    if (h.empty()) {
        return;
    }
    CHECK(log::tidy(h + "/Library/x.log") == "~/Library/x.log");
    CHECK(log::tidy("--data " + h) == "--data ~");
    CHECK(log::tidy(h + "extra/y") == h + "extra/y");  // someone else's folder
}

void times_and_names() {
    char name[64];
    // 2026-10-17 12:03:22 UTC, shown two hours ahead.
    crash::file_name(1792238602LL, 7200, name, sizeof(name));
    CHECK(std::string(name) == "crash-2026-10-17_14-03-22.txt");
    char when[64];
    crash::format_time(1792238602LL, -18000, when, sizeof(when));
    CHECK(std::string(when) == "2026-10-17 07:03:22 -0500");
    crash::Text t;
    t.str("at ").hex(0x15F6A8, 8).str(" frame ").dec(-42);
    CHECK(std::string(t.c_str()) == "at 0x0015F6A8 frame -42");
}

void report_has_every_section(const fs::path& dir) {
    CHECK(log::open_session(dir, "openrac-report", 5));
    log::header("==== OpenRAC session ====");
    log::header("game     rac1-pal");
    for (int i = 0; i < 250; ++i) {
        log::info("line {}", i);
    }
    crash::configure(dir / "crashes", "openrac-test", "2026-10-17 14:00:00 +0200");
    crash::GameState s;
    s.frame = 1234;
    s.level = 3;
    s.has_mode = true;
    s.mode = 2;
    s.has_hero = true;
    s.hero[0] = 101.25f;
    s.hero[1] = -7.5f;
    s.hero[2] = 33.0f;
    s.has_camera = true;
    s.camera[0] = 95.5f;
    s.camera[1] = -12.0f;
    s.camera[2] = 40.25f;
    crash::set_state(s);
    // The moby being updated, asked for while the report is written.
    crash::set_moby_source([](crash::Moby& m) {
        m.address = 0x00A1B200;
        m.oclass = 340;
        m.position[0] = 12.3f;
        m.position[1] = 45.6f;
        m.position[2] = 7.8f;
        m.has_uid = true;
        m.uid = 57;
        return true;
    });
    const auto written_report = [&dir](const char* name) {
        const fs::path out = dir / name;
#if defined(_WIN32)
        const int fd = _open(out.string().c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, 0644);
#else
        const int fd = ::open(out.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
#endif
        CHECK(fd >= 0);
        crash::write_report(fd, "access at game address 0x7FFF0000, which is not mapped", crash::Kind::Signal,
                            1792238602LL);
#if defined(_WIN32)
        _close(fd);
#else
        ::close(fd);
#endif
        return read(out);
    };
    const std::string text = written_report("report.txt");
    CHECK(text.find("Reason:   access at game address 0x7FFF0000") != std::string::npos);
    CHECK(text.find("Kind:     fault") != std::string::npos);
    CHECK(text.find("Game:     level 3, game mode 2, frame 1234, hero at 101.25 -7.50 33.00, updating moby class 340 "
                    "uid 57 at 12.30 45.60 7.80 (0x00A1B200), camera at 95.50 -12.00 40.25\n")
          != std::string::npos);
    CHECK(text.find("Backtrace") != std::string::npos);
    CHECK(text.find("[info] line 249") != std::string::npos);
    // No moby being updated (or no uid), no camera: only what is known.
    crash::set_moby_source([](crash::Moby& m) {
        m.address = 0x00A1B300;
        m.oclass = 7;
        return true;
    });
    CHECK(written_report("report_no_uid.txt").find("33.00, updating moby class 7 at 0.00 0.00 0.00 (0x00A1B300), camera")
          != std::string::npos);
    crash::set_moby_source([](crash::Moby&) { return false; });
    s.has_camera = false;
    crash::set_state(s);
    CHECK(written_report("report_plain.txt").find("Game:     level 3, game mode 2, frame 1234, hero at 101.25 -7.50 33.00\n")
          != std::string::npos);
    crash::set_moby_source(nullptr);
    CHECK(text.find("[info] line 49\n") == std::string::npos);  // 200 lines kept: 50..249
    CHECK(text.find("game     rac1-pal") != std::string::npos);
    CHECK(text.find("OpenRAC ") != std::string::npos);
    CHECK(text.find("==== end of the report ====") != std::string::npos);

    // The real thing: a file in crashes/, once.
    const char* written = crash::report("a test", crash::Kind::Trap);
    CHECK(written != nullptr);
    if (written != nullptr) {
        CHECK(fs::path(written).parent_path() == dir / "crashes");
        CHECK(read(written).find("Kind:     game trap") != std::string::npos);
    }
    CHECK(crash::report("again", crash::Kind::Trap) == nullptr);
    log::set_file(nullptr);
    CHECK(read(log::session_file(dir, "openrac-report", 0)).find("Crash report: ") != std::string::npos);
}

void capped_says_it_so_many_times() {
    log::Capped told(2);
    CHECK(told());
    CHECK(told());
    CHECK(!told());
    CHECK(!told());
    log::Capped never(0);
    CHECK(!never());
}

}  // namespace

int main() {
    const fs::path dir = fs::temp_directory_path() / ("openrac_log_test_" + std::to_string(std::rand()));
    fs::remove_all(dir);
    fs::create_directories(dir);
    ring_keeps_the_last_lines();
    capped_says_it_so_many_times();
    sessions_rotate(dir);
    home_reads_tilde();
    times_and_names();
    report_has_every_section(dir);
    fs::remove_all(dir);
    return test::result();
}
