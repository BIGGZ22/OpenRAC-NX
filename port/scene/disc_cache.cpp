// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene/disc_cache.h"
#include "assets/disc/checksum.h"
#include <fstream>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>
#ifdef __SWITCH__
#include <unistd.h>
#endif
namespace openrac::scene {
namespace {
using assets::disc::Sha256;
constexpr std::uint64_t kMaximum = 768ull << 20;
struct Header {
    char magic[8] = {'O','R','N','X','C','A','C','H'};
    unsigned version = kDiscCacheVersion;
    unsigned vertex = sizeof(Vertex), material = sizeof(Material), primitive = sizeof(Primitive), instance = sizeof(Instance);
};
struct Writer {
    std::ofstream out;
    Sha256 hash;
    explicit Writer(const std::filesystem::path& p) : out(p, std::ios::binary | std::ios::trunc) {
        if (!out) throw std::runtime_error("Cannot create cache: " + p.string());
    }
    void bytes(const void* p, std::size_t n) {
        out.write(static_cast<const char*>(p), static_cast<std::streamsize>(n));
        if (!out || out.tellp() > static_cast<std::streamoff>(kMaximum)) throw std::runtime_error("Cache write failed or exceeds size limit");
        hash.update(std::span(static_cast<const std::uint8_t*>(p), n));
    }
    template<class T> void pod(const T& t) { static_assert(std::is_trivially_copyable_v<T>); bytes(&t, sizeof t); }
    template<class T> void vec(const std::vector<T>& v) { pod(static_cast<std::uint32_t>(v.size())); bytes(v.data(), v.size() * sizeof(T)); }
    void str(const std::string& s) { pod(static_cast<std::uint32_t>(s.size())); bytes(s.data(), s.size()); }
};
struct Reader {
    std::ifstream in;
    std::uint64_t remaining;
    Reader(const std::filesystem::path& p, std::uint64_t size) : in(p, std::ios::binary), remaining(size) {}
    void bytes(void* p, std::size_t n) {
        if (n > remaining || !in.read(static_cast<char*>(p), static_cast<std::streamsize>(n))) throw std::runtime_error("Truncated renderer cache");
        remaining -= n;
    }
    template<class T> void pod(T& t) { static_assert(std::is_trivially_copyable_v<T>); bytes(&t, sizeof t); }
    std::uint32_t count(std::size_t width, std::size_t limit) {
        std::uint32_t n; pod(n);
        if (n > limit || n > remaining / width) throw std::runtime_error("Invalid cache array size");
        return n;
    }
    template<class T> void vec(std::vector<T>& v) { v.resize(count(sizeof(T), 12'000'000)); bytes(v.data(), v.size() * sizeof(T)); }
    void str(std::string& s) { s.resize(count(1, 4096)); bytes(s.data(), s.size()); }
};
}
void write_disc_cache(const std::filesystem::path& file, const LevelData& l) {
    auto partial = file; partial += ".partial";
    std::filesystem::create_directories(file.parent_path());
    Writer w(partial);
    w.pod(Header{}); w.pod(l.level); w.pod(l.background); w.pod(l.bounds_min); w.pod(l.bounds_max);
    w.vec(l.vertices); w.vec(l.indices); w.vec(l.materials);
    w.pod(static_cast<std::uint32_t>(l.models.size()));
    for (const auto& m : l.models) { w.str(m.name); w.vec(m.primitives); w.pod(m.min); w.pod(m.max); w.pod(m.joints); }
    w.pod(static_cast<std::uint32_t>(l.images.size()));
    for (std::size_t i = 0; i < l.images.size(); ++i) {
        w.pod(l.images[i].width); w.pod(l.images[i].height); w.vec(l.images[i].pixels);
        w.str(l.image_paths[i]); w.pod(static_cast<std::uint8_t>(l.translucent_images[i]));
    }
    for (const auto& v : l.instances) w.vec(v);
    for (const auto& v : l.draw_distances) w.vec(v);
    w.vec(l.light_colours); w.vec(l.light_sets); w.vec(l.pvs.grid);
    w.pod(static_cast<std::uint32_t>(l.moby_classes.size()));
    for (const auto& [id, m] : l.moby_classes) { w.pod(id); w.pod(m.first); w.pod(static_cast<std::uint8_t>(m.second)); }
    const auto digest = w.hash.finish();
    w.out.write(reinterpret_cast<const char*>(digest.data()), digest.size());
    w.out.close();
    if (w.out.fail()) throw std::runtime_error("Cannot flush renderer cache");
#ifdef __SWITCH__
    FILE* sync = std::fopen(partial.string().c_str(), "r+b");
    bool synced = sync && fsync(fileno(sync)) == 0;
    if (sync && std::fclose(sync) != 0) synced = false;
    if (!synced) throw std::runtime_error("Cannot flush renderer cache to SD");
#endif
    std::filesystem::rename(partial, file);
}
bool read_disc_cache(const std::filesystem::path& file, LevelData& level, std::string& error) {
    try {
        const auto size = std::filesystem::file_size(file);
        if (size < sizeof(Header) + 32 || size > kMaximum) throw std::runtime_error("Invalid renderer cache length");
        // Verify before interpreting records or allocating their arrays.
        std::ifstream verify(file, std::ios::binary);
        Sha256 hash;
        std::array<std::uint8_t, 65536> block;
        std::uint64_t left = size - 32;
        while (left) {
            auto n = static_cast<std::size_t>(std::min<std::uint64_t>(left, block.size()));
            if (!verify.read(reinterpret_cast<char*>(block.data()), n)) throw std::runtime_error("Cannot read renderer cache");
            hash.update(std::span(block.data(), n)); left -= n;
        }
        assets::disc::Sha256Digest expected;
        if (!verify.read(reinterpret_cast<char*>(expected.data()), expected.size()) || hash.finish() != expected) throw std::runtime_error("Renderer cache checksum failed; prepare this level again");
        Reader r(file, size - 32);
        Header h, want; r.pod(h);
        if (std::memcmp(&h, &want, sizeof h) != 0) throw std::runtime_error("Renderer cache needs regeneration for this build");
        LevelData l;
        l.dir = file.parent_path();
        r.pod(l.level); r.pod(l.background); r.pod(l.bounds_min); r.pod(l.bounds_max);
        r.vec(l.vertices); r.vec(l.indices); r.vec(l.materials);
        l.models.resize(r.count(1, 10000));
        for (auto& m : l.models) { r.str(m.name); r.vec(m.primitives); r.pod(m.min); r.pod(m.max); r.pod(m.joints); }
        const auto images = r.count(1, 10000);
        for (unsigned i = 0; i < images; ++i) {
            renderer::Rgba8Image image;
            r.pod(image.width); r.pod(image.height);
            if (image.width < 1 || image.width > 4096 || image.height < 1 || image.height > 4096) throw std::runtime_error("Invalid cached image dimensions");
            r.vec(image.pixels);
            if (image.pixels.size() != std::size_t(image.width) * image.height * 4) throw std::runtime_error("Invalid cached image size");
            l.images.push_back(std::move(image));
            std::string path; r.str(path); l.image_paths.push_back(std::move(path));
            std::uint8_t alpha; r.pod(alpha); l.translucent_images.push_back(alpha != 0);
        }
        for (auto& v : l.instances) r.vec(v);
        for (auto& v : l.draw_distances) r.vec(v);
        r.vec(l.light_colours); r.vec(l.light_sets); r.vec(l.pvs.grid);
        const auto classes = r.count(1, 10000);
        for (unsigned i = 0; i < classes; ++i) { int id; std::uint32_t model; std::uint8_t box; r.pod(id); r.pod(model); r.pod(box); if (model >= l.models.size()) throw std::runtime_error("Invalid cached class model"); l.moby_classes[id] = {model, box != 0}; }
        if (r.remaining != 0) throw std::runtime_error("Trailing renderer cache data");
        for (auto i : l.indices) if (i >= l.vertices.size()) throw std::runtime_error("Invalid cached vertex index");
        for (const auto& m : l.models) for (const auto& p : m.primitives) {
            if (p.first_index > l.indices.size() || p.index_count > l.indices.size() - p.first_index || p.material < 0 || std::size_t(p.material) >= l.materials.size()) throw std::runtime_error("Invalid cached primitive");
        }
        for (const auto& m : l.materials) if (m.image < -1 || m.image >= int(l.images.size())) throw std::runtime_error("Invalid cached material");
        for (const auto& layer : l.instances) for (const auto& i : layer) if (i.model >= l.models.size()) throw std::runtime_error("Invalid cached instance");
        level = std::move(l); return true;
    } catch (const std::exception& e) { error = e.what(); return false; }
}
}
