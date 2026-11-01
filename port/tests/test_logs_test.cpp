// SPDX-License-Identifier: GPL-3.0-or-later
// game/common/test_logs.h on the host: run in an empty folder (the SD card's "sdmc:/..." paths are
// relative there). A run's folder gets the game log and only what a growing log added; a run that
// did not end cleanly is copied at the next start with a NOTE; latest/ holds the newest ended run.
#define OPENRAC_TEST_LOG_ROOT "test-sdmc/switch/openrac/test-logs"
#include "game/common/test_logs.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>
namespace fs = std::filesystem;
using namespace openrac;
static std::string read(const fs::path& p) { std::ifstream in(p, std::ios::binary); std::stringstream s; s << in.rdbuf(); auto text=s.str(); std::erase(text,'\r'); return text; }
static void write(const fs::path& p, const std::string& t, bool append = false) {
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary | (append ? std::ios::app : std::ios::trunc)) << t;
}
int main() {
    const fs::path home = test_logs::root().parent_path(), log = home / "openrac-rac1-pal.log";
    assert(!test_logs::managed_run(home / "outside"));
    assert(!test_logs::managed_run(test_logs::root() / "notes"));
    assert(!test_logs::managed_run(test_logs::root() / ".." / "2026-01-01_00-00-00"));
    write(home / "overlay-status.log", "old run overlay\n");
    write(log, "previous run log\n");
    test_logs::prepare();
    write(home / "overlay-status.log", "run 1 startup\n", true);
    // Run 1: begins, writes, ends cleanly.
    test_logs::begin(log, "2026-10-30 02:41:59 -0400");
    const fs::path run1 = test_logs::root() / "2026-10-30_02-41-59";
    assert(fs::exists(run1));
    write(log, "run 1 log\n");
    write(home / "env-applied.log", "OPENRAC_DK_TIMING=120\n");
    write(home / "overlay-status.log", "run 1 overlay\n", true);
    test_logs::archive();
    assert(read(run1 / "openrac-rac1-pal.log") == "run 1 log\n");
    assert(read(run1 / "overlay-status.log") == "run 1 startup\nrun 1 overlay\n");
    assert(read(run1 / "env-applied.log") == "OPENRAC_DK_TIMING=120\n");
    assert(fs::exists(run1 / "COMPLETE"));
    assert(read(test_logs::root() / "latest" / "openrac-rac1-pal.log") == "run 1 log\n");
    assert(read(test_logs::root() / "LATEST.txt") == "2026-10-30_02-41-59\n");
    // Run 2 begins, writes, and "crashes" (no archive).
    test_logs::run().active = false;
    test_logs::begin(log, "2026-10-30 03:00:00 -0400");
    write(log, "run 2 log, crashed\n");
    write(home / "overlay-status.log", "run 2 overlay\n", true);
    // Run 3 begins: run 2 is copied first, with a NOTE.
    test_logs::run().active = false;
    test_logs::prepare();
    write(home / "overlay-status.log", "run 3 startup\n", true);
    test_logs::begin(log, "2026-10-30 03:10:00 -0400");
    const fs::path run2 = test_logs::root() / "2026-10-30_03-00-00";
    assert(read(run2 / "openrac-rac1-pal.log") == "run 2 log, crashed\n");
    assert(read(run2 / "overlay-status.log") == "run 2 overlay\n");
    assert(fs::exists(run2 / "NOTE.txt") && !fs::exists(run2 / "COMPLETE"));
    // Same start second: a second folder, not an overwrite.
    test_logs::run().active = false;
    test_logs::begin(log, "2026-10-30 03:10:00 -0400");
    assert(fs::exists(test_logs::root() / "2026-10-30_03-10-00_2"));
    // Pruned to the newest 20.
    for (int i = 0; i < 30; ++i) fs::create_directories(test_logs::root() / ("2026-01-01_00-00-" + std::to_string(10 + i)));
    test_logs::prune();
    int folders = 0;
    for (const auto& e : fs::directory_iterator(test_logs::root())) folders += e.is_directory() && e.path().filename() != "latest";
    assert(folders == 20 && fs::exists(run1));
    fs::create_directories(test_logs::root()/"user-notes");
    test_logs::prune();
    assert(fs::exists(test_logs::root()/"user-notes"));
    test_logs::run().active = false;
    std::puts("PASS: per-run log folders, growing logs' own part, crash recovery at the next start, latest/, pruning");
}
