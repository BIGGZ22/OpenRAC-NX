// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The Vulkan pipeline cache's files (renderer/vulkan/pipeline_cache_file.h, in cache_file.h's
// envelope): a save reads back whole, a damaged file is dropped, and writers saving the same file
// at once, as threads and as processes, leave a whole save of one of them and nothing else; the
// pipeline manifest reads back its entries, and one of another key size, cut short or holding a
// key of the wrong size is dropped or refused. The payloads are made up: each
// writer's is a length and a byte pattern derived from its number, so a reader can tell whose
// save it got and whether it is whole.
//
// Run as `pipeline_cache_file_test --write <file> <writer>`, the program is one writer process of
// the last test; it exits 1 if it read a damaged file.

#include <atomic>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <ios>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "check.h"
#include "common/joining_thread.h"
#include "renderer/vulkan/pipeline_cache_file.h"

namespace {

namespace fs = std::filesystem;
using openrac::vkgl::ManifestEntry;
using openrac::vkgl::read_pipeline_cache;
using openrac::vkgl::read_pipeline_manifest;
using openrac::vkgl::SavedCacheState;
using openrac::vkgl::write_pipeline_cache;
using openrac::vkgl::write_pipeline_manifest;

// The writers of each parallel test, and the saves each makes.
constexpr int kWriters = 8;
constexpr int kSaves = 25;

// A payload's length: a base, plus a step per writer so each writer's save has its own length.
constexpr std::size_t kPayloadBase = 4096;
constexpr std::size_t kPayloadStep = 977;

// Returns writer `writer`'s payload: its own length, each byte its writer and its index mixed.
std::vector<char> payload(int writer) {
    std::vector<char> bytes(kPayloadBase + kPayloadStep * static_cast<std::size_t>(writer));
    for (std::size_t n = 0; n < bytes.size(); ++n) {
        const std::size_t mixed = n * 31 + static_cast<std::size_t>(writer) * 7;
        bytes[n] = static_cast<char>(static_cast<unsigned char>(mixed));
    }
    return bytes;
}

// Returns whether `data` is the whole payload of one of the writers.
bool is_a_payload(const std::vector<char>& data) {
    // The length names the writer; anything shorter than the first one's is no payload.
    if (data.size() < kPayloadBase || (data.size() - kPayloadBase) % kPayloadStep != 0) {
        return false;
    }
    return data == payload(static_cast<int>((data.size() - kPayloadBase) / kPayloadStep));
}

// Returns a new empty folder of this run's own under the system's temporary folder.
fs::path fresh_folder() {
    std::random_device device;
    const fs::path folder =
        fs::temp_directory_path()
        / std::format("openrac-pipeline-cache-{:08x}{:08x}", device(), device());
    fs::create_directories(folder);
    return folder;
}

// Returns how many entries `folder` holds: after the saves, only the cache file is left.
int entries(const fs::path& folder) {
    int count = 0;
    for ([[maybe_unused]] const auto& entry : fs::directory_iterator(folder)) {
        ++count;
    }
    return count;
}

// Saves `writer`'s payload kSaves times, reading the file after each; false if a read was damaged.
bool save_repeatedly(const fs::path& file, int writer) {
    const std::vector<char> data = payload(writer);
    bool whole = true;
    for (int n = 0; n < kSaves; ++n) {
        std::string error;
        // On Windows a rename can lose to a reader holding the file; that save is skipped.
        [[maybe_unused]] const bool saved = write_pipeline_cache(file, data, error);
        const auto read = read_pipeline_cache(file);
        whole = whole && read.state != SavedCacheState::Damaged;
        whole = whole && (read.state != SavedCacheState::Intact || is_a_payload(read.data));
    }
    return whole;
}

// Tests that a save reads back as the same data, and a file never saved as missing.
void test_a_save_reads_back_whole() {
    const fs::path folder = fresh_folder();
    const fs::path file = folder / "deeper" / "cache.bin";
    CHECK(read_pipeline_cache(file).state == SavedCacheState::Missing);

    std::string error;
    CHECK(write_pipeline_cache(file, payload(3), error));
    const auto read = read_pipeline_cache(file);
    CHECK(read.state == SavedCacheState::Intact);
    CHECK(read.data == payload(3));
    fs::remove_all(folder);
}

// Tests that a file cut short, changed, of another format or empty is dropped, and a save
// replaces it.
void test_a_damaged_file_is_dropped_and_replaced() {
    const fs::path folder = fresh_folder();
    const fs::path file = folder / "cache.bin";
    std::string error;
    CHECK(write_pipeline_cache(file, payload(1), error));
    const auto whole_size = fs::file_size(file);

    // Cut short, as a save interrupted by another writer's truncation was.
    fs::resize_file(file, whole_size - 100);
    CHECK(read_pipeline_cache(file).state == SavedCacheState::Damaged);

    // One byte of the data changed: the hash tells.
    CHECK(write_pipeline_cache(file, payload(1), error));
    {
        std::fstream edit(file, std::ios::binary | std::ios::in | std::ios::out);
        edit.seekp(static_cast<std::streamoff>(whole_size / 2));
        edit.put('\x5a');
    }
    CHECK(read_pipeline_cache(file).state == SavedCacheState::Damaged);

    // The driver's data alone, as the cache was saved before the envelope.
    {
        const std::vector<char> raw = payload(2);
        std::ofstream out(file, std::ios::binary | std::ios::trunc);
        out.write(raw.data(), static_cast<std::streamsize>(raw.size()));
    }
    CHECK(read_pipeline_cache(file).state == SavedCacheState::Damaged);

    // An empty file, as a crash between creating and writing one leaves.
    {
        std::ofstream out(file, std::ios::binary | std::ios::trunc);
    }
    CHECK(read_pipeline_cache(file).state == SavedCacheState::Damaged);

    CHECK(write_pipeline_cache(file, payload(4), error));
    CHECK(read_pipeline_cache(file).data == payload(4));
    fs::remove_all(folder);
}

// Runs `writer(n)`, counting it in `failures` when it fails or throws: one writer's thread body.
void run_counted(const std::function<bool(int)>& writer, int n, std::atomic<int>& failures) {
    try {
        failures.fetch_add(writer(n) ? 0 : 1);
    } catch (...) {
        failures.fetch_add(1);
    }
}

// Runs `writer(0)` to `writer(kWriters - 1)` at once on threads of their own; true when every one
// returned true and none threw.
bool all_writers_whole(const std::function<bool(int)>& writer) {
    std::atomic<int> failures{0};
    {
        std::vector<openrac::JoiningThread> threads;
        threads.reserve(kWriters);
        for (int n = 0; n < kWriters; ++n) {
            // A thread body: writer n; the threads are joined when the block ends.
            threads.emplace_back([&failures, &writer, n] { run_counted(writer, n, failures); });
        }
    }
    return failures.load() == 0;
}

// Returns the command that runs this program as writer `writer` of `file`.
std::string writer_command(const char* program, const fs::path& file, int writer) {
    const std::string command =
        std::format("\"{}\" --write \"{}\" {}", program, file.string(), writer);
#if defined(_WIN32)
    // cmd.exe drops the outer pair of quotes from a command that starts with one.
    return "\"" + command + "\"";
#else
    return command;
#endif
}

// Tests that threads saving one file at once never let a reader see a damaged file, and leave a
// whole save of one of them and no temporary file.
void test_writer_threads_leave_one_whole_save() {
    const fs::path folder = fresh_folder();
    const fs::path file = folder / "cache.bin";
    CHECK(all_writers_whole([&file](int writer) { return save_repeatedly(file, writer); }));
    const auto read = read_pipeline_cache(file);
    CHECK(read.state == SavedCacheState::Intact);
    CHECK(is_a_payload(read.data));
    CHECK(entries(folder) == 1);
    fs::remove_all(folder);
}

// Tests that processes saving one file at once never see a damaged file, and leave a whole save
// of one of them and no temporary file: the case of tests run in parallel.
void test_writer_processes_leave_one_whole_save(const char* program) {
    const fs::path folder = fresh_folder();
    const fs::path file = folder / "cache.bin";
    // Each thread waits for one writer process: the program itself, run with --write.
    CHECK(all_writers_whole([program, &file](int writer) {
        // The test runs its own program with its own arguments; no outside text reaches the shell.
        // NOLINTNEXTLINE(bugprone-command-processor)
        return std::system(writer_command(program, file, writer).c_str()) == 0;
    }));
    const auto read = read_pipeline_cache(file);
    CHECK(read.state == SavedCacheState::Intact);
    CHECK(is_a_payload(read.data));
    CHECK(entries(folder) == 1);
    fs::remove_all(folder);
}

// Runs one writer process of test_writer_processes_leave_one_whole_save: `file` and `writer`'s
// number as given on the command line. EXIT_FAILURE when it read a damaged file, or the number is
// not one.
int run_writer(const char* file, std::string_view writer) {
    int number = 0;
    const auto [end, error] = std::from_chars(writer.data(), writer.data() + writer.size(), number);

    // Only the test starts writers, always with a number in range.
    if (error != std::errc{} || end != writer.data() + writer.size() || number < 0
        || number >= kWriters) {
        return EXIT_FAILURE;
    }

    try {
        return save_repeatedly(file, number) ? EXIT_SUCCESS : EXIT_FAILURE;
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "pipeline_cache_file writer: %s\n", failure.what());
        return EXIT_FAILURE;
    }
}

// ---- The pipeline manifest ----

// The key size of the manifest tests: any size will do, the backend's is its PipelineKey's.
constexpr std::size_t kManifestKeyBytes = 24;

// Returns entry `n` of the manifest tests: its program, age and key bytes derived from `n`.
ManifestEntry manifest_entry(int n) {
    ManifestEntry entry;
    entry.program = 0x1000 + static_cast<std::uint64_t>(n);
    entry.age = static_cast<std::uint32_t>(n);
    entry.key.resize(kManifestKeyBytes);

    // Each byte its entry and its index mixed, so two entries' keys differ.
    for (std::size_t at = 0; at < entry.key.size(); ++at) {
        entry.key[at] = static_cast<char>(static_cast<unsigned char>(at * 13 + n * 5));
    }

    return entry;
}

// Tests that a manifest reads back the entries saved, in their order, and a missing one as such.
void test_a_manifest_reads_back_its_entries() {
    const fs::path folder = fresh_folder();
    const fs::path file = folder / "cache.bin.manifest";
    CHECK(read_pipeline_manifest(file, kManifestKeyBytes).state == SavedCacheState::Missing);

    const std::vector<ManifestEntry> saved =
        {manifest_entry(0), manifest_entry(1), manifest_entry(7)};
    std::string error;
    CHECK(write_pipeline_manifest(file, saved, kManifestKeyBytes, error));
    const auto read = read_pipeline_manifest(file, kManifestKeyBytes);
    CHECK(read.state == SavedCacheState::Intact);
    CHECK(read.entries.size() == saved.size());

    // Each entry as it was saved.
    for (std::size_t n = 0; n < saved.size() && n < read.entries.size(); ++n) {
        CHECK(read.entries[n].program == saved[n].program);
        CHECK(read.entries[n].age == saved[n].age);
        CHECK(read.entries[n].key == saved[n].key);
    }

    fs::remove_all(folder);
}

// Tests that a manifest of another key size (another build's keys) or cut short is dropped, and
// that an entry whose key is of the wrong size is not saved.
void test_a_manifest_of_another_build_or_damaged_is_dropped() {
    const fs::path folder = fresh_folder();
    const fs::path file = folder / "cache.bin.manifest";
    const std::vector<ManifestEntry> saved = {manifest_entry(2), manifest_entry(3)};
    std::string error;
    CHECK(write_pipeline_manifest(file, saved, kManifestKeyBytes, error));
    CHECK(read_pipeline_manifest(file, kManifestKeyBytes + 8).state == SavedCacheState::Damaged);

    // Cut short by one byte: the envelope tells.
    fs::resize_file(file, fs::file_size(file) - 1);
    CHECK(read_pipeline_manifest(file, kManifestKeyBytes).state == SavedCacheState::Damaged);

    // A pipeline cache is not a manifest: another magic.
    CHECK(write_pipeline_cache(file, payload(0), error));
    CHECK(read_pipeline_manifest(file, kManifestKeyBytes).state == SavedCacheState::Damaged);

    ManifestEntry short_key = manifest_entry(4);
    short_key.key.pop_back();
    const std::vector<ManifestEntry> wrong = {short_key};
    CHECK(!write_pipeline_manifest(file, wrong, kManifestKeyBytes, error));
    fs::remove_all(folder);
}

// Runs the tests in the order they are written; false when one threw.
bool run_tests(const char* program) {
    try {
        test_a_save_reads_back_whole();
        test_a_damaged_file_is_dropped_and_replaced();
        test_writer_threads_leave_one_whole_save();
        test_writer_processes_leave_one_whole_save(program);
        test_a_manifest_reads_back_its_entries();
        test_a_manifest_of_another_build_or_damaged_is_dropped();
        return true;
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "pipeline_cache_file: %s\n", failure.what());
        return false;
    }
}

}  // namespace

int main(int argc, char** argv) {
    constexpr int kWriterArguments = 4;

    // One writer process of test_writer_processes_leave_one_whole_save.
    if (argc == kWriterArguments && std::string_view(argv[1]) == "--write") {
        return run_writer(argv[2], argv[3]);
    }

    return run_tests(argv[0]) ? openrac::test::result() : EXIT_FAILURE;
}
