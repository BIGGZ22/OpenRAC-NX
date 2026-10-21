// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-extract/tests/extract/
// synthetic.rs: ISC License, Copyright (c) 2026 ReRAC contributors.
//
// openrac-extractor's steps on synthetic discs (tests/disc/synthetic_disc.h)
// written to a temporary folder, against a table of builds made for them: no
// byte of a real disc, and no real build's checksum is needed.

#include "extract/extractor.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include "assets/disc/checksum.h"
#include "tests/check.h"
#include "tests/disc/synthetic_disc.h"

namespace fs = std::filesystem;
using namespace openrac;
using namespace openrac::extract;
using openrac::assets::Game;
using openrac::assets::GameVersion;
using openrac::assets::Region;

namespace {

struct Run {
    int status;
    std::string output;  // stdout and stderr, in order
};

std::string slurp(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

Run run_with(const Options& options, bool json) {
    std::FILE* out = std::tmpfile();
    Reporter report(json, out, out);
    const int status = run(options, report);
    std::fflush(out);
    std::rewind(out);
    std::string text;
    char buffer[4096];
    std::size_t n;
    while ((n = std::fread(buffer, 1, sizeof buffer, out)) != 0) {
        text.append(buffer, n);
    }
    std::fclose(out);
    return {status, text};
}

class Scratch {
public:
    Scratch() {
        m_dir = fs::temp_directory_path()
                / ("openrac-extract-test-" + std::to_string(std::random_device{}()));
        fs::create_directories(m_dir);
    }

    ~Scratch() {
        std::error_code ec;
        fs::remove_all(m_dir, ec);
    }

    const fs::path& dir() const { return m_dir; }

    fs::path write(const std::string& name, const std::vector<assets::u8>& bytes) const {
        const fs::path p = m_dir / name;
        std::ofstream out(p, std::ios::binary);
        out.write(
            reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())
        );
        return p;
    }

private:
    fs::path m_dir;
};

GameVersion fake_version(
    std::string_view id,
    std::string_view key,
    Game game,
    std::string_view serial,
    std::string_view sha1
) {
    return GameVersion{
        id, key, game, Region::NtscU, "Test", serial, "RATCHETANDCLANK", "1.00", 0, "", 0, sha1, 60
    };
}

int last_error_code(const std::string& json) {
    const std::size_t at = json.rfind("\"type\":\"error\",\"code\":");
    return at == std::string::npos ? 0 : std::stoi(json.substr(at + 22));
}

void json_strings() {
    CHECK(json_string("a\"b\\c\n") == "\"a\\\"b\\\\c\\n\"");
    CHECK(json_string(std::string_view("\x01", 1)) == "\"\\u0001\"");
}

void image_checks() {
    Scratch s;
    const fs::path notiso = s.write("disc.bin", std::vector<assets::u8>(10));
    try {
        check_image_file(notiso, 0);
        CHECK(false);
    } catch (const ExtractError& e) {
        CHECK(e.code() == ErrorCode::NotImageFile);
    }
    const fs::path small = s.write("disc.iso", std::vector<assets::u8>(10));
    try {
        check_image_file(small, 1000);
        CHECK(false);
    } catch (const ExtractError& e) {
        CHECK(e.code() == ErrorCode::ImageTooSmall);
    }
}

void extract_then_decompile() {
    Scratch s;
    const std::vector<assets::u8> elf = test::fake_elf("Ratchet & Clank");
    const std::string sha = assets::disc::to_hex(assets::disc::sha1(elf));
    const GameVersion known[] = {
        fake_version("rac1-ntsc", "rac1/ntsc", Game::Rac1, "SCUS_971.99", sha)
    };
    Options o;
    o.image = s.write("game.iso", test::rac1_disc("SCUS_971.99", elf));
    o.game = "rac1";
    o.proj = s.dir() / "proj";
    o.minimum_size = 0;
    o.known = known;

    o.extract = true;
    const Run extracted = run_with(o, true);
    CHECK(extracted.status == 0);
    CHECK(
        extracted.output.find(
            "\"type\":\"disc\",\"serial\":\"SCUS_971.99\",\"version\":\"rac1/ntsc\""
        )
        != std::string::npos
    );
    CHECK(
        extracted.output.ends_with("\n")
        && extracted.output.rfind("\"type\":\"done\"") != std::string::npos
    );
    const fs::path iso_data = o.proj / "iso_data" / "rac1";
    CHECK(fs::exists(iso_data / "SYSTEM.CNF"));
    CHECK(fs::exists(iso_data / "SCUS_971.99"));
    CHECK(fs::exists(iso_data / "disc.iso"));
    CHECK(!fs::exists(o.proj / "iso_data" / "_temp"));
    CHECK(slurp(iso_data / "SCUS_971.99").size() == elf.size());
    const std::string info = slurp(iso_data / "buildinfo.json");
    CHECK(info.find("\"version\": \"rac1/ntsc\"") != std::string::npos);
    CHECK(info.find("\"elf_sha1\": \"" + sha + "\"") != std::string::npos);
    CHECK(info.find("\"files\": 2") != std::string::npos);

    o.extract = false;
    o.decompile = true;
    const Run decompiled = run_with(o, false);
    CHECK(decompiled.status == 0);
    const fs::path out = o.proj / "decompiler_out" / "rac1";
    CHECK(fs::file_size(out / "toc.bin") == 0x2960);
    CHECK(fs::exists(out / "levels/01/level_header.bin"));
    CHECK(fs::exists(out / "levels/01/gameplay_ntsc.bin"));
    CHECK(!fs::exists(out / "levels/01/scene"));  // indexed, not copied
    const std::string lumps = slurp(out / "lumps.tsv");
    CHECK(lumps.find("global/mpegs/021.bin\t") != std::string::npos);
    CHECK(lumps.find("levels/01/scene/00_ntsc.bin\t") != std::string::npos);
    CHECK(fs::exists(out / "decompileinfo.json"));
    CHECK(!fs::exists(o.proj / "decompiler_out" / "_temp"));

    // Every step: the compile step does not exist yet.
    o.decompile = false;
    const Run all = run_with(o, true);
    CHECK(all.status == 1);
    CHECK(last_error_code(all.output) == 4060);
}

void refusals() {
    Scratch s;
    const std::vector<assets::u8> elf = test::fake_elf("Ratchet & Clank");
    const std::string sha = assets::disc::to_hex(assets::disc::sha1(elf));
    Options o;
    o.image = s.write("game.iso", test::rac1_disc("SCUS_971.99", elf));
    o.game = "rac1";
    o.proj = s.dir() / "proj";
    o.minimum_size = 0;
    o.extract = true;

    // The serial is known, its boot executable is another build. The hash outlives the
    // version, whose field only views it.
    const std::string other_sha1(40, '0');
    const GameVersion other_build[] = {
        fake_version("rac1-ntsc", "rac1/ntsc", Game::Rac1, "SCUS_971.99", other_sha1)
    };
    o.known = other_build;
    Run r = run_with(o, true);
    CHECK(r.status == 1 && last_error_code(r.output) == 4002);
    CHECK(r.output.find("game.json") != std::string::npos);  // the line a maintainer adds
    CHECK(!fs::exists(o.proj / "iso_data" / "rac1"));

    // A disc of another game of the series.
    const GameVersion right[] = {
        fake_version("rac1-ntsc", "rac1/ntsc", Game::Rac1, "SCUS_971.99", sha)
    };
    o.known = right;
    o.game = "rac2";
    r = run_with(o, true);
    CHECK(r.status == 1 && last_error_code(r.output) == 4001);

    // Not a PlayStation 2 game disc.
    o.game = "rac1";
    o.image = s.write("other.iso", test::make_iso(40, {{"README.TXT", test::bytes_of("hello")}}));
    r = run_with(o, true);
    CHECK(r.status == 1 && last_error_code(r.output) == 4000);

    // Not an ISO 9660 image at all.
    o.image = s.write("junk.iso", std::vector<assets::u8>(64 * 2048, 0x5a));
    r = run_with(o, false);
    CHECK(r.status == 1 && r.output.starts_with("error 4020:"));
}

void unknown_layout_does_not_decompile() {
    Scratch s;
    const std::vector<assets::u8> elf = test::fake_elf("Going Commando");
    const std::string sha = assets::disc::to_hex(assets::disc::sha1(elf));
    const GameVersion known[] = {
        fake_version("rac2-ntsc", "rac2/ntsc", Game::Rac2, "SCUS_972.68", sha)
    };
    Options o;
    o.image = s.write(
        "rac2.iso",
        test::make_iso(40, {{"SYSTEM.CNF", test::system_cnf("SCUS_972.68")}, {"SCUS_972.68", elf}})
    );
    o.game = "rac2";
    o.proj = s.dir() / "proj";
    o.minimum_size = 0;
    o.known = known;
    o.extract = true;
    CHECK(run_with(o, true).status == 0);
    o.extract = false;
    o.decompile = true;
    const Run r = run_with(o, true);
    CHECK(r.status == 1 && last_error_code(r.output) == 4060);
}

}  // namespace

int main() {
    json_strings();
    image_checks();
    extract_then_decompile();
    refusals();
    unknown_layout_does_not_decompile();
    return openrac::test::result();
}
