// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "extract/extractor.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>
#include <functional>
#include <system_error>
#include <utility>
#include <vector>

#include "assets/disc/boot.h"
#include "assets/disc/checksum.h"
#include "assets/disc/disc.h"
#include "assets/disc/toc.h"
#include "assets/disc/wad.h"

namespace openrac::extract {

namespace fs = std::filesystem;
using assets::Game;
using assets::GameVersion;
using assets::u32;
using assets::u8;
using namespace assets::disc;

namespace {

constexpr std::size_t kChunk = 4u << 20;

bool lower_equal(std::string_view a, std::string_view b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char x, char y) {
        return std::tolower(static_cast<unsigned char>(x))
               == std::tolower(static_cast<unsigned char>(y));
    });
}

// A file name OpenRAC writes: what ISO 9660 allows on these discs.
bool safe_name(std::string_view path) {
    if (path.empty()) {
        return false;
    }

    std::size_t start = 0;

    while (start <= path.size()) {
        const std::size_t end = std::min(path.find('/', start), path.size());
        const std::string_view part = path.substr(start, end - start);

        if (part.empty() || part == "." || part == "..") {
            return false;
        }

        for (const char c : part) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '.' && c != '-') {
                return false;
            }
        }

        start = end + 1;
    }

    return true;
}

void make_dirs(const fs::path& dir) {
    std::error_code ec;
    fs::create_directories(dir, ec);

    if (ec) {
        throw ExtractError(
            ErrorCode::CannotWrite, std::format("{}: {}", dir.string(), ec.message())
        );
    }
}

void remove_dir(const fs::path& dir) {
    std::error_code ec;
    fs::remove_all(dir, ec);

    if (ec) {
        throw ExtractError(
            ErrorCode::CannotWrite, std::format("{}: {}", dir.string(), ec.message())
        );
    }
}

void rename_to(const fs::path& from, const fs::path& to) {
    std::error_code ec;
    fs::rename(from, to, ec);

    if (ec) {
        throw ExtractError(
            ErrorCode::CannotWrite, std::format("{}: {}", to.string(), ec.message())
        );
    }
}

// Writes `bytes` to `path` through `path.partial`.
void write_file(const fs::path& path, std::span<const u8> bytes) {
    make_dirs(path.parent_path());
    fs::path partial = path;
    partial += ".partial";
    {
        std::ofstream out(partial, std::ios::binary | std::ios::trunc);
        out.write(
            reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())
        );

        if (!out) {
            throw ExtractError(ErrorCode::CannotWrite, partial.string());
        }
    }
    rename_to(partial, path);
}

void write_text(const fs::path& path, std::string_view text) {
    write_file(path, {reinterpret_cast<const u8*>(text.data()), text.size()});
}

// Throws 4061 unless `bytes` (and the margin) fit where `dir` is.
void check_space(const fs::path& dir, std::uint64_t bytes) {
    std::error_code ec;
    const fs::space_info space = fs::space(dir, ec);

    if (ec) {
        return;  // a file system that cannot say: let the writes tell
    }

    if (space.available < bytes + kSpaceMargin) {
        throw ExtractError(
            ErrorCode::NoSpace,
            std::format(
                "{} MiB needed at {}, {} MiB free",
                (bytes + kSpaceMargin) >> 20,
                dir.string(),
                space.available >> 20
            )
        );
    }
}

// Copies `bytes` bytes of the image at `offset` to `path` (through .partial),
// returning their SHA-1.
std::string copy_range(
    const IsoImage& iso,
    std::uint64_t offset,
    std::uint64_t bytes,
    const fs::path& path,
    const std::function<void(std::uint64_t)>& advanced
) {
    make_dirs(path.parent_path());
    fs::path partial = path;
    partial += ".partial";
    Sha1 hash;
    std::vector<u8> buffer(static_cast<std::size_t>(std::min<std::uint64_t>(bytes, kChunk)));
    {
        std::ofstream out(partial, std::ios::binary | std::ios::trunc);

        if (!out) {
            throw ExtractError(ErrorCode::CannotWrite, partial.string());
        }

        std::uint64_t left = bytes;

        while (left != 0) {
            const std::size_t n = static_cast<std::size_t>(std::min<std::uint64_t>(left, kChunk));
            const std::span<u8> chunk(buffer.data(), n);
            iso.read_into(offset + (bytes - left), chunk);
            hash.update(chunk);
            out.write(reinterpret_cast<const char*>(chunk.data()), static_cast<std::streamsize>(n));

            if (!out) {
                throw ExtractError(ErrorCode::CannotWrite, partial.string());
            }

            left -= n;
            advanced(n);
        }
    }
    rename_to(partial, path);
    return to_hex(hash.finish());
}

void link_or_copy(const fs::path& source, const fs::path& dest, Reporter& report) {
    std::error_code ec;
    fs::create_hard_link(source, dest, ec);

    if (!ec) {
        report.info(std::format("  disc.iso: linked to {}", source.string()));
        return;
    }

    fs::copy_file(source, dest, fs::copy_options::overwrite_existing, ec);

    if (ec) {
        throw ExtractError(
            ErrorCode::CannotWrite, std::format("{}: {}", dest.string(), ec.message())
        );
    }

    report.info(std::format("  disc.iso: copied from {}", source.string()));
}

std::string build_info_json(const BuildInfo& b) {
    return std::format(
        "[\n  {{\n    \"serial\": {},\n    \"version\": {},\n    \"elf_sha1\": {},\n"
        "    \"files\": {},\n    \"contents\": {},\n    \"image\": {}\n  }}\n]\n",
        json_string(b.serial),
        json_string(b.version),
        json_string(b.boot_sha1),
        b.files,
        json_string(b.contents),
        json_string(b.image)
    );
}

// The value of "key": "..." in a buildinfo.json this program wrote.
std::string json_field(std::string_view json, std::string_view key) {
    const std::string needle = std::format("\"{}\": \"", key);
    const std::size_t at = json.find(needle);

    if (at == std::string_view::npos) {
        return {};
    }

    const std::size_t start = at + needle.size();
    const std::size_t end = json.find('"', start);
    return end == std::string_view::npos ? std::string{}
                                         : std::string(json.substr(start, end - start));
}

std::string read_text(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

}  // namespace

void check_image_file(const fs::path& image, std::uint64_t minimum_size) {
    std::error_code ec;
    std::string ext = image.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (!fs::is_regular_file(image, ec) || ext != ".iso") {
        throw ExtractError(ErrorCode::NotImageFile, image.string());
    }

    const std::uint64_t size = fs::file_size(image, ec);

    if (ec || size < minimum_size) {
        throw ExtractError(ErrorCode::ImageTooSmall, std::format("{} bytes", size));
    }
}

IsoImage open_image(const fs::path& image) {
    try {
        IsoImage iso = IsoImage::open(image);

        if (iso.sector_count() < iso.volume_sectors()) {
            throw ExtractError(
                ErrorCode::CannotRead,
                std::format(
                    "truncated: {} of the {} sectors its volume descriptor declares",
                    iso.sector_count(),
                    iso.volume_sectors()
                )
            );
        }

        return iso;
    } catch (const IoError& e) {
        throw ExtractError(ErrorCode::CannotRead, e.what());
    } catch (const assets::AssetError& e) {
        throw ExtractError(ErrorCode::NotIso, e.what());
    }
}

Identity identify(const IsoImage& iso, std::span<const GameVersion> known) {
    Identity id;
    id.volume = iso.volume_id();
    const IsoEntry* cnf_entry = iso.find("SYSTEM.CNF");

    if (cnf_entry == nullptr) {
        throw ExtractError(ErrorCode::NoBootExecutable, "no SYSTEM.CNF");
    }

    std::optional<SystemCnf> cnf;
    std::vector<u8> boot;

    try {
        const std::vector<u8> text = iso.read_file(*cnf_entry);
        cnf = SystemCnf::parse({reinterpret_cast<const char*>(text.data()), text.size()});

        if (!cnf) {
            throw ExtractError(ErrorCode::NoBootExecutable, "SYSTEM.CNF has no BOOT2 line");
        }

        const IsoEntry* boot_entry = iso.find(cnf->boot);

        if (boot_entry == nullptr) {
            throw ExtractError(
                ErrorCode::NoBootExecutable,
                std::format("SYSTEM.CNF boots {}, which is not on the disc", cnf->boot)
            );
        }

        boot = iso.read_file(*boot_entry);
    } catch (const IoError& e) {
        throw ExtractError(ErrorCode::CannotRead, e.what());
    } catch (const assets::AssetError& e) {
        throw ExtractError(ErrorCode::CannotRead, e.what());
    }

    id.serial = cnf->serial;
    id.disc_version = cnf->version;
    id.vmode = cnf->vmode;
    id.boot_size = boot.size();
    id.boot_sha1 = to_hex(sha1(boot));
    Game game{};

    if (assets::game_of_serial(id.serial, game)) {
        id.game = game;
    }

    for (const GameVersion& v : known) {
        if (lower_equal(v.serial, id.serial) && v.boot_sha1 == id.boot_sha1) {
            id.version = &v;
            id.game = v.game;
        }
    }

    // A known serial of a table passed in (tests) names its game too.
    if (!id.game) {
        for (const GameVersion& v : known) {
            if (lower_equal(v.serial, id.serial)) {
                id.game = v.game;
            }
        }
    }

    return id;
}

DiscReport disc_report(const Identity& id) {
    DiscReport d;
    d.serial = id.serial;
    d.boot_sha1 = id.boot_sha1;
    d.game = id.game ? std::string(assets::game_name(*id.game)) : "unknown";
    d.title = id.game ? std::string(assets::game_title(*id.game)) : id.volume;

    if (id.version != nullptr) {
        d.version = std::string(id.version->key);
        d.region = std::string(assets::region_name(id.version->region));
        d.title = std::string(id.version->title);
        d.supported = true;
    } else {
        d.region = id.vmode.empty() ? "unknown" : id.vmode;
    }

    return d;
}

const GameVersion& validate(const Identity& id, std::string_view game) {
    if (id.version != nullptr && assets::game_name(id.version->game) == game) {
        return *id.version;
    }

    if (!id.game) {
        throw ExtractError(ErrorCode::UnknownVersion, std::format("{} ({})", id.serial, id.volume));
    }

    if (assets::game_name(*id.game) != game) {
        throw ExtractError(
            ErrorCode::UnknownVersion,
            std::format("{} is {}, not {}", id.serial, assets::game_title(*id.game), game)
        );
    }

    if (id.version == nullptr) {
        throw ExtractError(
            ErrorCode::UnknownBuild,
            std::format("{} with a boot executable of SHA-1 {}", id.serial, id.boot_sha1)
        );
    }

    return *id.version;
}

std::string contents_hash(std::span<const std::pair<std::string, std::string>> file_sha1s) {
    std::vector<std::pair<std::string, std::string>> sorted(file_sha1s.begin(), file_sha1s.end());
    std::sort(sorted.begin(), sorted.end());
    Sha256 hash;

    for (const auto& [path, sha] : sorted) {
        hash.update(path);
        hash.update(std::string_view("\0", 1));
        hash.update(sha);
        hash.update("\n");
    }

    return to_hex(hash.finish());
}

BuildInfo extract_disc(const Options& options, Reporter& report) {
    check_image_file(options.image, options.minimum_size);
    const IsoImage iso = open_image(options.image);

    report.progress(Stage::Identify, 0, 1, "SYSTEM.CNF");
    const Identity id = identify(iso, options.known);
    report.progress(Stage::Identify, 1, 1, id.serial);
    report.disc(disc_report(id));

    if (id.version == nullptr || assets::game_name(id.version->game) != options.game) {
        report.info(
            "If this is a genuine disc OpenRAC does not know yet, a maintainer can add it to "
            "games/<game>/game.json:"
        );
        report.info(
            std::format(
                "{{\"serial\": {}, \"boot\": {{\"sha1\": {}, \"size\": {}}}}}",
                json_string(id.serial),
                json_string(id.boot_sha1),
                id.boot_size
            )
        );
    }

    const GameVersion& version = validate(id, options.game);

    std::vector<const IsoEntry*> files;
    std::uint64_t total = 0;

    for (const IsoEntry& e : iso.entries()) {
        if (e.is_directory) {
            continue;
        }

        const std::string rel = e.path.starts_with('/') ? e.path.substr(1) : e.path;

        if (!safe_name(rel)) {
            throw ExtractError(
                ErrorCode::NotIso, std::format("a file name OpenRAC will not write: {}", rel)
            );
        }

        files.push_back(&e);
        total += e.size;
    }

    report.info(
        std::format(
            "{}: {} files on the disc, boot executable {}",
            options.image.filename().string(),
            files.size(),
            id.serial
        )
    );

    const fs::path iso_data = options.proj / "iso_data";
    const fs::path temp = iso_data / "_temp";
    const fs::path final_dir = iso_data / options.game;
    make_dirs(iso_data);
    check_space(iso_data, total);
    remove_dir(temp);
    make_dirs(temp);

    std::vector<std::pair<std::string, std::string>> hashes;
    std::uint64_t done = 0;

    for (const IsoEntry* e : files) {
        const std::string rel = e->path.substr(1);
        report.progress(Stage::Copy, done, total, rel);

        try {
            hashes.emplace_back(
                rel,
                copy_range(
                    iso,
                    std::uint64_t{e->lba} * kSectorSize,
                    e->size,
                    temp / rel,
                    [&](std::uint64_t n) {
                        done += n;
                        report.progress(Stage::Copy, done, total, rel);
                    }
                )
            );
        } catch (const IoError& err) {
            throw ExtractError(ErrorCode::CannotRead, std::format("{}: {}", rel, err.what()));
        } catch (const assets::AssetError& err) {
            throw ExtractError(ErrorCode::CannotRead, std::format("{}: {}", rel, err.what()));
        }

        report.info(std::format("  {}: {} bytes", rel, e->size));
    }

    report.progress(Stage::Copy, total, total, "");

    // The boot executable was hashed while copied: it must still be the build.
    for (const auto& [path, sha] : hashes) {
        if (lower_equal(fs::path(path).filename().string(), id.serial) && sha != id.boot_sha1) {
            throw ExtractError(ErrorCode::CannotRead, "the boot executable read back differently");
        }
    }

    report.info(
        std::format(
            "validated: {} ({}, {}), boot executable SHA-1 matches games/{}/game.json",
            version.title,
            assets::region_name(version.region),
            id.serial,
            options.game
        )
    );

    link_or_copy(fs::absolute(options.image), temp / "disc.iso", report);
    BuildInfo info;
    info.serial = id.serial;
    info.version = std::string(version.key);
    info.boot_sha1 = id.boot_sha1;
    info.files = hashes.size();
    info.contents = contents_hash(hashes);
    info.image = options.image.filename().string();
    write_text(temp / "buildinfo.json", build_info_json(info));
    remove_dir(final_dir);
    rename_to(temp, final_dir);
    report.info(std::format("extracted to {}", final_dir.string()));
    return info;
}

void decompile_disc(const Options& options, Reporter& report) {
    const fs::path in = options.proj / "iso_data" / options.game;
    const fs::path image = in / "disc.iso";
    std::error_code ec;

    if (!fs::exists(in / "buildinfo.json", ec) || !fs::exists(image, ec)) {
        throw ExtractError(
            ErrorCode::DecompileFailed,
            std::format("{} has no extracted disc: run --extract first", in.string())
        );
    }

    const std::string key = json_field(read_text(in / "buildinfo.json"), "version");
    const GameVersion* version = nullptr;

    for (const GameVersion& v : options.known) {
        if (v.key == key) {
            version = &v;
        }
    }

    if (version == nullptr) {
        throw ExtractError(ErrorCode::DecompileFailed, std::format("buildinfo.json names {}", key));
    }

    const DiscLayout& layout = disc_layout(*version);

    if (!layout.known) {
        throw ExtractError(
            ErrorCode::NotYet,
            std::format("--decompile: the disc layout of {} is not known yet", version->key)
        );
    }

    const fs::path out_root = options.proj / "decompiler_out";
    const fs::path temp = out_root / "_temp";
    const fs::path final_dir = out_root / options.game;

    try {
        const Disc disc(open_image(image), layout);
        // What is written: the table of contents and every level's own files.
        // Everything else (movies, music, speech, global lumps) stays in
        // disc.iso and is indexed by its place there.
        std::vector<DiscFile> written;
        std::vector<DiscFile> indexed;

        for (const DiscFile& f : disc.archive_files()) {
            const bool level_file =
                f.path.starts_with("levels/") && f.path.find('/', 10) == std::string::npos;
            (f.path == "toc.bin" || level_file ? written : indexed).push_back(f);
        }

        std::uint64_t total = 0;

        for (const DiscFile& f : written) {
            total += f.bytes;
        }

        make_dirs(out_root);
        // Unpacked lumps are several times their packed size.
        check_space(out_root, total * 4);
        remove_dir(temp);
        make_dirs(temp);

        std::uint64_t done = 0;
        std::string index = "# path\toffset in disc.iso\tbytes\n";

        for (const DiscFile& f : written) {
            copy_range(disc.iso(), f.offset, f.bytes, temp / f.path, [&](std::uint64_t n) {
                done += n;
                report.progress(Stage::Decompile, done, total, f.path);
            });
            index += std::format("{}\t{}\t{}\n", f.path, f.offset, f.bytes);
        }

        for (const DiscFile& f : indexed) {
            index += std::format("{}\t{}\t{}\n", f.path, f.offset, f.bytes);
        }

        write_text(temp / "lumps.tsv", index);
        report.info(
            std::format(
                "  {} files written, {} lumps indexed in lumps.tsv", written.size(), indexed.size()
            )
        );

        // Each level's compressed lumps, unpacked.
        std::size_t unpacked = 0;

        for (const u32 id : disc.level_ids()) {
            const LevelFiles level = disc.level(id);
            const fs::path dir = temp / std::format("levels/{:02}/unpacked", id);
            auto unpack = [&](std::string_view name, const std::optional<std::vector<u8>>& lump) {
                if (lump && is_wad(*lump)) {
                    write_file(dir / std::format("{}.bin", name), wad_decompress(*lump));
                    ++unpacked;
                }
            };
            unpack("core_data", level.core_data);
            unpack("gameplay_ntsc", level.gameplay_ntsc);
            unpack("gameplay_pal", level.gameplay_pal);

            for (std::size_t i = 0; i < level.hud_banks.size(); ++i) {
                unpack(std::format("hud_bank_{}", i), level.hud_banks[i]);
            }

            report.progress(Stage::Decompile, total, total, std::format("levels/{:02}", id));
        }

        report.info(std::format("  {} lumps unpacked", unpacked));
        write_text(
            temp / "decompileinfo.json",
            std::format(
                "{{\n  \"version\": {},\n  \"format\": 1,\n  \"levels\": {}\n}}\n",
                json_string(version->key),
                disc.level_ids().size()
            )
        );
    } catch (const IoError& e) {
        throw ExtractError(ErrorCode::CannotRead, e.what());
    } catch (const assets::AssetError& e) {
        throw ExtractError(ErrorCode::DecompileFailed, e.what());
    }

    remove_dir(final_dir);
    rename_to(temp, final_dir);
    report.info(std::format("decompiled to {}", final_dir.string()));
}

int run(const Options& options, Reporter& report) {
    const bool every =
        !(options.extract || options.validate || options.decompile || options.compile);

    try {
        if (every || options.extract || options.validate) {
            extract_disc(options, report);
        }

        if (every || options.decompile) {
            decompile_disc(options, report);
        }

        if (every || options.compile) {
            throw ExtractError(ErrorCode::NotYet, "--compile");
        }
    } catch (const ExtractError& e) {
        report.error(e);
        return 1;
    } catch (const fs::filesystem_error& e) {
        report.error(ExtractError(ErrorCode::CannotWrite, e.what()));
        return 1;
    }

    report.done();
    return 0;
}

}  // namespace openrac::extract
