// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests the Vulkan backend's SPIR-V cache (renderer/vulkan/spirv_cache_file.h, R-12): a program
// taken from the cache is the program glslang makes, word for word and field for field; the
// cache's file reads back its entries; and a file cut short, with a byte changed, made by another
// compiler, or an entry whose key matches but whose text does not, gives nothing, so the program
// is compiled afresh. It needs no GPU: the programs are compiled with no context open, which
// keeps no cache, and the cache is tested on its own.
//
// The shaders are small ones in the forms the port's renderers write: loose uniforms, a sampler,
// a uniform block, defines spliced in as the level renderer splices its variants, a compute stage.

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "renderer/vulkan/shader_compiler.h"
#include "renderer/vulkan/spirv_cache_file.h"
#include "tests/check.h"

namespace {

namespace fs = std::filesystem;
using openrac::vkgl::CacheFileState;
using openrac::vkgl::CompiledProgram;
using openrac::vkgl::SpirvCache;
using openrac::vkgl::SpirvCacheEntry;

// A vertex stage that feeds a colour and a position.
constexpr std::string_view kVertex = R"(#version 410 core
layout(location = 0) in vec3 position;
layout(location = 1) in mat4 instance;
out vec2 v_uv;
uniform mat4 view;
void main() {
    v_uv = position.xy;
    gl_Position = view * instance * vec4(position, 1.0);
}
)";

// A fragment stage with loose uniforms, a sampler and a uniform block, whose two variants differ
// by a define spliced in after the version line.
constexpr std::string_view kFragmentHead = "#version 410 core\n";
constexpr std::string_view kFragmentBody = R"(uniform sampler2D picture;
uniform vec4 tint;
uniform float weights[4];
layout(std140) uniform Light { vec4 sun; vec4 sky; };
in vec2 v_uv;
layout(location = 0) out vec4 result;
void main() {
    vec4 c = texture(picture, v_uv) * tint * weights[2];
#ifdef OPENRAC_TEST_LIT
    c.rgb *= sun.rgb + sky.rgb;
#endif
    result = c;
}
)";

// A compute stage with a storage block.
constexpr std::string_view kCompute = R"(#version 430 core
layout(local_size_x = 64) in;
layout(std430, binding = 0) buffer Counts { uint counts[]; };
uniform uint step_size;
void main() { counts[gl_GlobalInvocationID.x] += step_size; }
)";

// The compiler text the cache tests are made for, and another one.
constexpr std::string_view kCompiler = "glslang test options 1";
constexpr std::string_view kOtherCompiler = "glslang test options 2";

// Returns a new empty folder of this run's own under the system's temporary folder.
fs::path fresh_folder() {
    std::random_device device;
    const fs::path folder = fs::temp_directory_path()
                            / std::format("openrac-spirv-cache-{:08x}{:08x}", device(), device());
    fs::create_directories(folder);
    return folder;
}

// Returns whether every field of `a` is `b`'s.
bool same_program(const CompiledProgram& a, const CompiledProgram& b) {
    bool same = a.vertex == b.vertex && a.fragment == b.fragment && a.compute == b.compute
                && a.default_binding == b.default_binding && a.default_size == b.default_size
                && a.uniforms.size() == b.uniforms.size() && a.blocks.size() == b.blocks.size()
                && a.storage.size() == b.storage.size()
                && a.attributes.size() == b.attributes.size();

    // Each uniform, field by field.
    for (std::size_t n = 0; same && n < a.uniforms.size(); ++n) {
        const auto& x = a.uniforms[n];
        const auto& y = b.uniforms[n];
        same = x.name == y.name && x.gl_type == y.gl_type && x.offset == y.offset
               && x.array_size == y.array_size && x.array_stride == y.array_stride
               && x.sampler == y.sampler && x.binding == y.binding && x.kind == y.kind;
    }

    // Each uniform block and storage block.
    for (std::size_t n = 0; same && n < a.blocks.size(); ++n) {
        same = a.blocks[n].name == b.blocks[n].name && a.blocks[n].binding == b.blocks[n].binding
               && a.blocks[n].size == b.blocks[n].size;
    }

    // Each storage block.
    for (std::size_t n = 0; same && n < a.storage.size(); ++n) {
        same = a.storage[n].name == b.storage[n].name
               && a.storage[n].binding == b.storage[n].binding
               && a.storage[n].size == b.storage[n].size;
    }

    // Each vertex input.
    for (std::size_t n = 0; same && n < a.attributes.size(); ++n) {
        same = a.attributes[n].name == b.attributes[n].name
               && a.attributes[n].location == b.attributes[n].location
               && a.attributes[n].gl_type == b.attributes[n].gl_type;
    }

    return same;
}

// Tests that each test program, encoded as the cache keeps it and decoded, is the program
// glslang made, and that compiling it again makes the same words: what a hit hands the backend is
// what a fresh compile would.
void test_a_program_from_the_cache_is_the_program_compiled() {
    const std::string plain = std::string(kFragmentHead) + std::string(kFragmentBody);
    const std::string lit =
        std::string(kFragmentHead) + "#define OPENRAC_TEST_LIT 1\n" + std::string(kFragmentBody);
    const std::array<std::string_view, 2> fragments = {plain, lit};

    // Each variant of the fragment stage with the vertex stage.
    for (const std::string_view fragment : fragments) {
        CompiledProgram fresh;
        CompiledProgram again;
        CompiledProgram decoded;
        std::string log;
        CHECK(openrac::vkgl::compile_program(kVertex, fragment, fresh, log));
        CHECK(openrac::vkgl::compile_program(kVertex, fragment, again, log));
        CHECK(!fresh.vertex.empty() && !fresh.fragment.empty() && !fresh.uniforms.empty());
        CHECK(openrac::vkgl::decode_program(openrac::vkgl::encode_program(fresh), decoded));
        CHECK(same_program(fresh, again));
        CHECK(same_program(fresh, decoded));
    }

    CompiledProgram compute;
    CompiledProgram decoded;
    std::string log;
    CHECK(openrac::vkgl::compile_program({}, {}, compute, log, kCompute));
    CHECK(!compute.compute.empty() && !compute.storage.empty());
    CHECK(openrac::vkgl::decode_program(openrac::vkgl::encode_program(compute), decoded));
    CHECK(same_program(compute, decoded));
}

// Tests that an encoding cut short, or with bytes after it, is not decoded.
void test_a_damaged_encoding_is_not_a_program() {
    CompiledProgram program;
    std::string log;
    CHECK(openrac::vkgl::compile_program({}, {}, program, log, kCompute));
    std::vector<char> bytes = openrac::vkgl::encode_program(program);
    CompiledProgram decoded;
    std::vector<char> shorter(bytes.begin(), bytes.end() - 1);
    CHECK(!openrac::vkgl::decode_program(shorter, decoded));
    bytes.push_back('\0');
    CHECK(!openrac::vkgl::decode_program(bytes, decoded));
}

// Returns entry `n` of the file tests: its key, check, length and value derived from `n`.
SpirvCacheEntry entry(int n) {
    const auto u = static_cast<std::uint64_t>(n);
    SpirvCacheEntry out{0x1000 + u, 0x2000 + u, 100 + u, std::vector<char>(8 + u, 'a')};
    out.value.front() = static_cast<char>('0' + n);
    return out;
}

// Saves a cache of entries 0 to 2 at `file` for kCompiler, over whatever was there.
void save_three(const fs::path& file) {
    std::error_code ec;
    fs::remove(file, ec);
    SpirvCache cache;
    CHECK(cache.open(file, std::string(kCompiler)) == CacheFileState::Missing);

    // Entries 0, 1 and 2.
    for (int n = 0; n < 3; ++n) {
        cache.insert(entry(n));
    }

    std::string error;
    CHECK(cache.save(error));
}

// Tests that a saved cache reads back its entries, found by their key, check and length.
void test_a_saved_cache_reads_back() {
    const fs::path folder = fresh_folder();
    const fs::path file = folder / "pipelines.bin.spirv";
    save_three(file);
    SpirvCache cache;
    CHECK(cache.open(file, std::string(kCompiler)) == CacheFileState::Intact);

    // Each entry, by what found it.
    for (int n = 0; n < 3; ++n) {
        const SpirvCacheEntry e = entry(n);
        const std::vector<char>* found = cache.find(e.key, e.check, e.length);
        CHECK(found != nullptr && *found == e.value);
    }

    std::error_code ec;
    fs::remove_all(folder, ec);
}

// Tests that a hit needs the check and the length as well as the key: an entry forged with
// another text's key is not handed out, so that text is compiled afresh.
void test_a_key_collision_is_a_miss() {
    SpirvCache cache;
    const fs::path folder = fresh_folder();
    CHECK(
        cache.open(folder / "collision.spirv", std::string(kCompiler)) == CacheFileState::Missing
    );
    const SpirvCacheEntry kept = entry(5);
    cache.insert(kept);
    CHECK(cache.find(kept.key, kept.check, kept.length) != nullptr);
    CHECK(cache.find(kept.key, kept.check + 1, kept.length) == nullptr);
    CHECK(cache.find(kept.key, kept.check, kept.length + 1) == nullptr);
    std::error_code ec;
    fs::remove_all(folder, ec);
}

// Tests that a file cut short, one with a byte changed, and one made by another compiler are
// dropped whole: nothing is found in them.
void test_a_damaged_or_stale_file_is_dropped() {
    const fs::path folder = fresh_folder();
    const fs::path file = folder / "pipelines.bin.spirv";
    const SpirvCacheEntry first = entry(0);

    // Another compiler made it: stale.
    save_three(file);
    SpirvCache stale;
    CHECK(stale.open(file, std::string(kOtherCompiler)) == CacheFileState::Damaged);
    CHECK(stale.find(first.key, first.check, first.length) == nullptr);

    // Cut short.
    save_three(file);
    fs::resize_file(file, fs::file_size(file) - 3);
    SpirvCache truncated;
    CHECK(truncated.open(file, std::string(kCompiler)) == CacheFileState::Damaged);
    CHECK(truncated.find(first.key, first.check, first.length) == nullptr);

    // One byte of the data changed: the envelope's hash no longer matches.
    save_three(file);
    {
        std::fstream edit(file, std::ios::binary | std::ios::in | std::ios::out);
        edit.seekp(static_cast<std::streamoff>(fs::file_size(file) - 2));
        edit.put('#');
    }
    SpirvCache changed;
    CHECK(changed.open(file, std::string(kCompiler)) == CacheFileState::Damaged);
    CHECK(changed.find(first.key, first.check, first.length) == nullptr);

    std::error_code ec;
    fs::remove_all(folder, ec);
}

}  // namespace

int main() {
    test_a_program_from_the_cache_is_the_program_compiled();
    test_a_damaged_encoding_is_not_a_program();
    test_a_saved_cache_reads_back();
    test_a_key_collision_is_a_miss();
    test_a_damaged_or_stale_file_is_dropped();
    return openrac::test::result();
}
