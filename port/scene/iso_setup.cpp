// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene/disc_import.h"
#include "scene/disc_cache.h"
#include "extract/extractor.h"
#include "assets/disc/checksum.h"
#include "common/json.h"
#include <fstream>
#include <format>
#include <cstdio>
#ifdef __SWITCH__
#include <unistd.h>
#endif
namespace openrac::scene {
namespace {
namespace fs = std::filesystem;
namespace d = assets::disc;
void progress(const SetupProgress& cb, const std::string& text, unsigned now = 0, unsigned total = 0) {
    if (cb && !cb(text,now,total)) throw std::runtime_error("Setup cancelled. Completed levels will be reused next time.");
}
void text_file(const fs::path& dest, const std::string& text) {
    auto temp = dest; temp += ".partial";
    std::ofstream out(temp,std::ios::binary|std::ios::trunc);
    out.write(text.data(),static_cast<std::streamsize>(text.size())); out.close();
    if (out.fail()) throw std::runtime_error("Cannot write " + dest.string());
#ifdef __SWITCH__
    FILE* f = std::fopen(temp.string().c_str(),"r+b");
    bool ok = f && fsync(fileno(f)) == 0;
    if (f && std::fclose(f) != 0) ok = false;
    if (!ok) throw std::runtime_error("Cannot flush " + dest.string());
#endif
    std::error_code ec;
    fs::rename(temp,dest,ec);
    if (ec) {
        // Only installer-owned small marker files are replaced here.
        fs::remove(dest,ec);
        fs::rename(temp,dest);
    }
}
}
void prepare_switch_disc(const fs::path& image, const fs::path& data, const SetupProgress& cb) {
    progress(cb,"Checking disc identity");
    auto iso = d::IsoImage::open(image);
    const auto identity = extract::identify(iso,assets::versions());
    const auto& version = extract::validate(identity,"rac1");
    if (version.id != "rac1-pal") throw std::runtime_error("This build needs Ratchet & Clank PAL SCES_509.16 v2.00");
    if (iso.raw_sector_size() != 2048 || fs::file_size(image) != version.disc_size)
        throw std::runtime_error("ISO length does not match the supported PAL disc. Use a complete, unsplit ISO.");
    const auto boot = iso.find(version.serial);
    const auto system = iso.find("SYSTEM.CNF");
    if (!boot || !system) throw std::runtime_error("Disc boot files are missing");
    const auto boot_bytes = iso.read_file(*boot), cnf_bytes = iso.read_file(*system);
    const auto card = data / "iso_data/rac1";
    fs::create_directories(card);
    // Hash in small chunks. Never read a multi-gigabyte ISO into memory.
    {
        std::ifstream file(image,std::ios::binary);
        d::Sha1 hash; std::vector<std::uint8_t> block(1 << 20);
        std::uint64_t done = 0;
        while (done < version.disc_size) {
            const auto n = static_cast<std::size_t>(std::min<std::uint64_t>(block.size(),version.disc_size-done));
            if (!file.read(reinterpret_cast<char*>(block.data()),n)) throw std::runtime_error("Cannot read complete ISO");
            hash.update(std::span(block.data(),n)); done += n;
            progress(cb,"Validating full ISO",static_cast<unsigned>(done >> 20),static_cast<unsigned>(version.disc_size >> 20));
        }
        if (d::to_hex(hash.finish()) != version.disc_sha1) throw std::runtime_error("ISO checksum mismatch. This disc is damaged, modified or an unsupported revision.");
    }
    // Keep the user's original ISO in place: data_path.txt is an internal pointer,
    // read by the program before opening disc sectors. This avoids a second copy.
    const auto absolute_image = (image.string().find(':') != std::string::npos
        ? image : fs::absolute(image)).lexically_normal();
    text_file(card / "disc-path.txt",absolute_image.string() + "\n");
    text_file(card / std::string(version.serial),std::string(reinterpret_cast<const char*>(boot_bytes.data()),boot_bytes.size()));
    text_file(card / "SYSTEM.CNF",std::string(reinterpret_cast<const char*>(cnf_bytes.data()),cnf_bytes.size()));
    text_file(card / "buildinfo.json",std::format("{{\"serial\":\"{}\",\"version\":\"rac1/pal\",\"boot_sha1\":\"{}\",\"disc_sha1\":\"{}\",\"on_device\":true}}\n",version.serial,identity.boot_sha1,version.disc_sha1));
    const auto root = data / "port/pal";
    fs::create_directories(root);
    text_file(root / "setup-in-progress",std::to_string(kDiscCacheVersion));
    auto disc = d::Disc::open(image,d::disc_layout(version));
    const auto normals = assets::rac1::NormalTable::from_elf(boot_bytes,0x165600);
    std::vector<int> ids = {99,0,98};
    for (auto id : disc.level_ids()) if (id != 0) ids.push_back(static_cast<int>(id));
    unsigned completed = 0;
    for (int id : ids) {
        const auto folder = root / std::format("level_{:02}",id);
        const auto cache = folder / "scene.orcache";
        LevelData test; std::string reason;
        progress(cb,std::format("Checking level {:02}",id),completed,static_cast<unsigned>(ids.size()));
        json::Value existing;
        const bool has_manifest = json::parse_file((folder / "manifest.json").string(),existing,reason);
        const auto publish_manifest = [&] {
            text_file(folder / "manifest.json",std::format("{{\"format\":4,\"game\":\"rac1\",\"version\":\"pal\",\"level\":{},\"native_cache\":{}}}\n",id,kDiscCacheVersion));
        };
        if (!fs::exists(cache) && has_manifest
            && existing["native_cache"].is_null() && existing["format"].number() == 4
            && existing["level"].number(-1) == id && existing["game"].string() == "rac1"
            && existing["version"].string() == "pal" && fs::exists(folder / "placements.json")) {
            ++completed; continue; // retain a previously prepared PC export
        }
        if (fs::exists(cache) && read_disc_cache(cache,test,reason) && test.level == id) {
            // A power loss between the cache rename and its manifest must not
            // leave a valid cache permanently invisible to the launcher.
            if (!has_manifest || existing["native_cache"].number() != kDiscCacheVersion
                || existing["level"].number(-1) != id) publish_manifest();
            ++completed; continue;
        }
        test = {};
        const auto update = [&](const std::string& text,unsigned,unsigned) {
            return !cb || cb(std::format("Level {:02}: {}",id,text),completed,static_cast<unsigned>(ids.size()));
        };
        auto level = import_disc_level(disc,id,normals,update);
        // Check against a conservative uncompressed estimate before publishing this level.
        std::uint64_t needed = level.vertices.size()*sizeof(Vertex) + level.indices.size()*4 + (64ull << 20);
        for (const auto& img : level.images) needed += img.pixels.size();
        std::error_code ec; const auto space = fs::space(root,ec);
        if (!ec && space.available < needed) throw std::runtime_error("Not enough free SD space. Completed levels are kept; free space and restart to continue.");
        fs::create_directories(folder);
        // Replace only our own corrupt/incompatible cache, never an exported PC level or saves.
        if (fs::exists(cache)) fs::remove(cache);
        progress(cb,std::format("Writing level {:02} cache",id),completed,static_cast<unsigned>(ids.size()));
        write_disc_cache(cache,level);
        level = {};
        progress(cb,std::format("Verifying level {:02} cache",id),completed,static_cast<unsigned>(ids.size()));
        if (!read_disc_cache(cache,test,reason) || test.level != id)
            throw std::runtime_error("Cache read-back failed: " + reason);
        publish_manifest();
        ++completed;
    }
    text_file(root / "setup-complete",std::to_string(kDiscCacheVersion));
    fs::remove(root / "setup-in-progress");
    // Completion has already been published. A late cancel must not turn a
    // successfully prepared installation into a reported failure.
    if (cb) cb("Setup complete. Your ISO must stay on the SD card.",completed,completed);
}
}
