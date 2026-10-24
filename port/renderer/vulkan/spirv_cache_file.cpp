// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The SPIR-V cache (spirv_cache_file.h): its file, the encoding of a compiled program, and the
// cache in memory.
//
// The file's data, little-endian: its format and the compiler's version text, the entry count,
// then each entry's key, check, length and value. A value or a text is its byte count (32 bits)
// and its bytes; a program's fields follow CompiledProgram's order.

#include "renderer/vulkan/spirv_cache_file.h"

#include <climits>
#include <cstring>
#include <format>
#include <utility>

namespace openrac::vkgl {

namespace {

// The SPIR-V cache file's magic: OpenRAC Vulkan SPIR-V cache. Its data says its format.
constexpr CacheMagic kMagic{'O', 'R', 'V', 'K', 'S', 'P', 'V', 'C'};

// Appends bytes to a buffer, little-endian.
class Writer {
public:
    // Appends the `bytes` low bytes of `value`.
    void number(std::uint64_t value, std::size_t bytes) {
        // Byte n holds bits 8n to 8n+7.
        for (std::size_t n = 0; n < bytes; ++n) {
            m_out.push_back(static_cast<char>(static_cast<unsigned char>(value >> (CHAR_BIT * n))));
        }
    }

    // Appends a 32-bit and a 64-bit number.
    void u32(std::uint64_t value) { number(value, sizeof(std::uint32_t)); }

    void u64(std::uint64_t value) { number(value, sizeof(std::uint64_t)); }

    // Appends a signed number as its 32-bit two's complement.
    void i32(int value) { u32(static_cast<std::uint32_t>(value)); }

    // Appends `bytes` after their count.
    void bytes(std::span<const char> bytes) {
        u32(bytes.size());
        m_out.insert(m_out.end(), bytes.begin(), bytes.end());
    }

    // Appends SPIR-V `words` after their count.
    void words(const std::vector<std::uint32_t>& words) {
        u32(words.size());

        // Each word.
        for (const std::uint32_t word : words) {
            u32(word);
        }
    }

    // The bytes written, given away.
    std::vector<char> take() { return std::move(m_out); }

private:
    std::vector<char> m_out;  // The bytes so far.
};

// Reads what a Writer wrote, failing (and staying failed) at the first read past the end.
class Reader {
public:
    explicit Reader(std::span<const char> in) : m_in(in) {}

    // Reads a little-endian number of `bytes` bytes; 0 once failed.
    std::uint64_t number(std::size_t bytes) {
        // Past the end: the encoding is cut short.
        if (m_failed || m_in.size() - m_at < bytes) {
            m_failed = true;
            return 0;
        }

        std::uint64_t value = 0;

        // Byte n holds bits 8n to 8n+7.
        for (std::size_t n = 0; n < bytes; ++n) {
            value |= std::uint64_t{static_cast<unsigned char>(m_in[m_at + n])} << (CHAR_BIT * n);
        }

        m_at += bytes;
        return value;
    }

    // Reads a 32-bit, a 64-bit and a signed 32-bit number.
    std::uint32_t u32() { return static_cast<std::uint32_t>(number(sizeof(std::uint32_t))); }

    std::uint64_t u64() { return number(sizeof(std::uint64_t)); }

    int i32() { return static_cast<int>(u32()); }

    // Reads bytes after their count.
    std::vector<char> bytes() {
        const std::size_t count = u32();

        // More than is left: the encoding is cut short or not one.
        if (m_failed || m_in.size() - m_at < count) {
            m_failed = true;
            return {};
        }

        std::vector<char> out(
            m_in.begin() + static_cast<std::ptrdiff_t>(m_at),
            m_in.begin() + static_cast<std::ptrdiff_t>(m_at + count)
        );
        m_at += count;
        return out;
    }

    // Reads a text after its count.
    std::string text() {
        const std::vector<char> raw = bytes();
        return std::string(raw.begin(), raw.end());
    }

    // Reads SPIR-V words after their count.
    std::vector<std::uint32_t> words() {
        const std::size_t count = u32();

        // More words than bytes are left: not an encoding of these.
        if (m_failed || (m_in.size() - m_at) / sizeof(std::uint32_t) < count) {
            m_failed = true;
            return {};
        }

        std::vector<std::uint32_t> out(count);

        // Each word.
        for (std::uint32_t& word : out) {
            word = u32();
        }

        return out;
    }

    // Reads a count of items, refusing one larger than the bytes left could hold.
    std::size_t count() {
        const std::size_t n = u32();

        // Each item takes a byte at least.
        if (n > m_in.size() - m_at) {
            m_failed = true;
            return 0;
        }

        return n;
    }

    // True when every read so far was inside the bytes.
    [[nodiscard]] bool ok() const { return !m_failed; }

    // True when every byte was read.
    [[nodiscard]] bool done() const { return m_at == m_in.size(); }

private:
    std::span<const char> m_in;  // The bytes read from.
    std::size_t m_at = 0;        // The next byte.
    bool m_failed = false;       // A read went past the end.
};

// Writes a block's name, binding and size.
void write_block(Writer& out, const ProgramBlock& block) {
    out.bytes(block.name);
    out.i32(block.binding);
    out.i32(block.size);
}

// Reads what write_block wrote.
ProgramBlock read_block(Reader& in) {
    ProgramBlock block;
    block.name = in.text();
    block.binding = in.i32();
    block.size = in.i32();
    return block;
}

}  // namespace

std::vector<char> encode_program(const CompiledProgram& program) {
    Writer out;
    out.words(program.vertex);
    out.words(program.fragment);
    out.words(program.compute);
    out.i32(program.default_binding);
    out.i32(program.default_size);
    out.u32(program.uniforms.size());

    // Each loose uniform and sampler.
    for (const ProgramUniform& u : program.uniforms) {
        out.bytes(u.name);
        out.u32(u.gl_type);
        out.i32(u.offset);
        out.i32(u.array_size);
        out.i32(u.array_stride);
        out.u32(u.sampler ? 1 : 0);
        out.i32(u.binding);
        out.u32(static_cast<std::uint32_t>(u.kind));
    }

    out.u32(program.blocks.size());

    // Each uniform block.
    for (const ProgramBlock& block : program.blocks) {
        write_block(out, block);
    }

    out.u32(program.storage.size());

    // Each storage block.
    for (const ProgramBlock& block : program.storage) {
        write_block(out, block);
    }

    out.u32(program.attributes.size());

    // Each vertex input.
    for (const CompiledProgram::Attribute& a : program.attributes) {
        out.bytes(a.name);
        out.i32(a.location);
        out.u32(a.gl_type);
    }

    return out.take();
}

bool decode_program(std::span<const char> bytes, CompiledProgram& program) {
    Reader in(bytes);
    program = CompiledProgram{};
    program.vertex = in.words();
    program.fragment = in.words();
    program.compute = in.words();
    program.default_binding = in.i32();
    program.default_size = in.i32();
    program.uniforms.resize(in.count());

    // Each loose uniform and sampler.
    for (ProgramUniform& u : program.uniforms) {
        u.name = in.text();
        u.gl_type = in.u32();
        u.offset = in.i32();
        u.array_size = in.i32();
        u.array_stride = in.i32();
        u.sampler = in.u32() != 0;
        u.binding = in.i32();
        const std::uint32_t kind = in.u32();

        // A kind past the last: not a program this code encoded.
        if (kind > static_cast<std::uint32_t>(SamplerKind::Buffer)) {
            return false;
        }

        u.kind = static_cast<SamplerKind>(kind);
    }

    program.blocks.resize(in.count());

    // Each uniform block.
    for (ProgramBlock& block : program.blocks) {
        block = read_block(in);
    }

    program.storage.resize(in.count());

    // Each storage block.
    for (ProgramBlock& block : program.storage) {
        block = read_block(in);
    }

    program.attributes.resize(in.count());

    // Each vertex input.
    for (CompiledProgram::Attribute& a : program.attributes) {
        a.name = in.text();
        a.location = in.i32();
        a.gl_type = in.u32();
    }

    return in.ok() && in.done();
}

SavedSpirvCache read_spirv_cache(const std::filesystem::path& path, std::string_view compiler) {
    const CacheFile file = read_cache_file(path, kMagic);

    // No file, or not a whole one.
    if (file.state != CacheFileState::Intact) {
        return {file.state, {}};
    }

    Reader in(file.data);
    const std::uint32_t format = in.u32();
    const std::string made_by = in.text();

    // Another format, or another compiler: none of its results is this build's.
    if (!in.ok() || format != kSpirvCacheFormat || made_by != compiler) {
        return {CacheFileState::Damaged, {}};
    }

    SavedSpirvCache saved{CacheFileState::Intact, {}};
    saved.entries.resize(in.count());

    // Each entry.
    for (SpirvCacheEntry& entry : saved.entries) {
        entry.key = in.u64();
        entry.check = in.u64();
        entry.length = in.u64();
        entry.value = in.bytes();
    }

    // Short, or with bytes left over: not one whole save.
    if (!in.ok() || !in.done()) {
        return {CacheFileState::Damaged, {}};
    }

    return saved;
}

bool write_spirv_cache(
    const std::filesystem::path& path,
    std::string_view compiler,
    std::span<const SpirvCacheEntry> entries,
    std::string& error
) {
    Writer out;
    out.u32(kSpirvCacheFormat);
    out.bytes(compiler);
    out.u32(entries.size());

    // Each entry.
    for (const SpirvCacheEntry& entry : entries) {
        out.u64(entry.key);
        out.u64(entry.check);
        out.u64(entry.length);
        out.bytes(entry.value);
    }

    const std::vector<char> data = out.take();
    return write_cache_file(path, kMagic, data, error);
}

CacheFileState SpirvCache::open(const std::filesystem::path& path, std::string compiler) {
    m_path = path;
    m_compiler = std::move(compiler);
    m_entries.clear();
    m_index.clear();
    m_changed = false;

    // No cache kept.
    if (m_path.empty()) {
        return CacheFileState::Missing;
    }

    SavedSpirvCache saved = read_spirv_cache(m_path, m_compiler);

    // Each entry read, up to the cache's size.
    for (SpirvCacheEntry& entry : saved.entries) {
        // Full, or a key twice (a file this code would not write).
        if (m_entries.size() >= kSpirvCacheEntries || m_index.contains(entry.key)) {
            continue;
        }

        m_index.emplace(entry.key, m_entries.size());
        m_entries.push_back(std::move(entry));
    }

    // A damaged or stale file is replaced at close even if nothing is added.
    m_changed = saved.state == CacheFileState::Damaged;
    return saved.state;
}

const std::vector<char>* SpirvCache::find(
    std::uint64_t key, std::uint64_t check, std::uint64_t length
) const {
    const auto found = m_index.find(key);

    // Never compiled, or kept under this key for another text.
    if (found == m_index.end() || m_entries[found->second].check != check
        || m_entries[found->second].length != length) {
        return nullptr;
    }

    return &m_entries[found->second].value;
}

void SpirvCache::insert(SpirvCacheEntry entry) {
    // No cache kept.
    if (!active()) {
        return;
    }

    // The key's other text: replaced by this one.
    if (const auto found = m_index.find(entry.key); found != m_index.end()) {
        m_entries[found->second] = std::move(entry);
        m_changed = true;
        return;
    }

    // Full: results past the limit are compiled again each run.
    if (m_entries.size() >= kSpirvCacheEntries) {
        return;
    }

    m_index.emplace(entry.key, m_entries.size());
    m_entries.push_back(std::move(entry));
    m_changed = true;
}

bool SpirvCache::save(std::string& error) {
    // Nothing new, or no cache kept.
    if (!m_changed || !active()) {
        return true;
    }

    m_changed = false;
    return write_spirv_cache(m_path, m_compiler, m_entries, error);
}

}  // namespace openrac::vkgl
