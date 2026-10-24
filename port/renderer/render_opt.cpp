// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The optimisation registry (render_opt.h): the table of entries, the reading of
// OPENRAC_RENDER_OPT_OFF, and the switches the draw code reads.

#include "renderer/render_opt.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <string>
#include <utility>

#include "common/env.h"
#include "common/log.h"

namespace openrac::renderer::render_opt {
namespace {

// The word of OPENRAC_RENDER_OPT_OFF that switches every entry off.
constexpr std::string_view kAll = "all";

// Every optimisation, in the order of Opt. The descriptions are the short form of
// docs/port/RENDER_PERF.md's sections, which say more.
// clang-format off: one entry per row: its names, then its default and tolerance.
constexpr std::array<Entry, kOptCount> kEntries = {{
    {Opt::RegistrySelftest, "registry-selftest",
     "draws nothing: the entry the registry's and the equivalence harness's tests switch",
     true, Tolerance{}},
    {Opt::VkDrawClock, "vk-draw-clock",
     "the Vulkan draw takes its two clock reads only while the debug tools measure",
     true, Tolerance{}},
    {Opt::VkStateSkip, "vk-state-skip",
     "Vulkan dynamic state and binds recorded only when they differ from the last recorded",
     true, Tolerance{}},
    {Opt::StaticBuffersDeviceLocal, "static-buffers-device-local",
     "a static GL buffer of 64 KiB or more goes into device memory at once, and its CPU copy goes",
     true, Tolerance{}},
    {Opt::StaticBuffersForceStaging, "static-buffers-force-staging",
     "with static-buffers-device-local: the staging copy into memory the CPU cannot see, always",
     false, Tolerance{}},
    {Opt::TexturePrepare, "texture-prepare",
     "a level's textures created, uploaded and mipped at its load instead of at their first draw",
     true, Tolerance{}},
    {Opt::VkDrawCache, "vk-draw-cache",
     "the Vulkan draw finds its pipeline and descriptor set by fixed keys, with no heap allocation",
     true, Tolerance{}},
}};
// clang-format on

// Returns the place of `opt` in the table.
constexpr std::size_t index(Opt opt) {
    return static_cast<std::size_t>(opt);
}

// Returns whether every row of kEntries is at the place of its Opt.
constexpr bool rows_in_order() {
    // Each row against its place: the table is read by place, so a row out of order is another's.
    for (std::size_t n = 0; n < kEntries.size(); ++n) {
        // A row at another entry's place.
        if (index(kEntries[n].opt) != n) {
            return false;
        }
    }

    return true;
}

static_assert(rows_in_order(), "each row of kEntries is at the place of its Opt");

// Returns `text` without the spaces and tabs at either end.
std::string_view trimmed(std::string_view text) {
    constexpr std::string_view kBlank = " \t";
    const std::size_t first = text.find_first_not_of(kBlank);

    // Nothing but blanks: an empty item.
    if (first == std::string_view::npos) {
        return {};
    }

    const std::size_t last = text.find_last_not_of(kBlank);
    return text.substr(first, last - first + 1);
}

// Returns what OPENRAC_RENDER_OPT_OFF names, warning once of each name that is no entry's.
OffList read_environment() {
    const auto text = env::value(env::Var::RenderOptOff);
    OffList off = parse_off_list(text.value_or(std::string_view{}));

    // A misspelt name switches nothing; the log says which it was.
    for (const std::string& name : off.unknown) {
        log::warn("render_opt: OPENRAC_RENDER_OPT_OFF names {}, which is no optimisation", name);
    }

    return off;
}

// The switches of every entry, and what the environment said, read once.
//
// The registry is process-wide by design, like the environment it reads: the draw code of every
// renderer asks it without being handed a context, so its state is the one function-local static
// of registry() (section 11.4's rule bent for a hot read; render_opt.h's enabled says what it
// costs). The switches are atomics so that a worker thread may read them while the drawing thread
// sets them. Neither copied nor moved: there is one.
class Registry {
public:
    // Makes the switches with `environment` applied over the defaults.
    explicit Registry(OffList environment) : m_environment(std::move(environment)) { reset(); }

    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;

    // Returns whether `opt` is on.
    [[nodiscard]] bool enabled(Opt opt) const {
        return m_on[index(opt)].load(std::memory_order_relaxed);
    }

    // Switches `opt` on or off.
    void set(Opt opt, bool on) { m_on[index(opt)].store(on, std::memory_order_relaxed); }

    // Sets every switch to its entry's default with the environment's list applied over it.
    void reset() {
        // Each entry as the table has it, unless the list said all.
        for (const Entry& row : kEntries) {
            set(row.opt, row.default_on && !m_environment.all);
        }

        // Then the entries the list named.
        for (const Opt opt : m_environment.off) {
            set(opt, false);
        }
    }

private:
    std::array<std::atomic<bool>, kOptCount> m_on{};  // Each entry's switch, by its place.
    OffList m_environment;                            // OPENRAC_RENDER_OPT_OFF, parsed.
};

// Returns the registry, made at the first call with the environment read.
Registry& registry() {
    static Registry state(read_environment());
    return state;
}

}  // namespace

std::span<const Entry> entries() {
    return kEntries;
}

const Entry& entry(Opt opt) {
    return kEntries[index(opt)];
}

const Entry* find(std::string_view name) {
    const auto found =
        std::ranges::find_if(kEntries, [name](const Entry& row) { return row.name == name; });
    return found != kEntries.end() ? &*found : nullptr;
}

OffList parse_off_list(std::string_view text) {
    OffList list;

    // One item at a time, up to the next comma, until the text is used up.
    while (!text.empty()) {
        const std::size_t comma = text.find(',');
        const std::string_view item = trimmed(text.substr(0, comma));
        text = comma == std::string_view::npos ? std::string_view{} : text.substr(comma + 1);

        // Two commas in a row, or one at an end: nothing named.
        if (item.empty()) {
            continue;
        }

        // The word that names every entry.
        if (item == kAll) {
            list.all = true;
            continue;
        }

        const Entry* row = find(item);

        // A name no entry has: kept to be warned of, and switches nothing.
        if (row == nullptr) {
            list.unknown.emplace_back(item);
            continue;
        }

        // An entry named twice is listed once.
        if (std::ranges::find(list.off, row->opt) == list.off.end()) {
            list.off.push_back(row->opt);
        }
    }

    return list;
}

bool enabled(Opt opt) {
    return registry().enabled(opt);
}

void set(Opt opt, bool on) {
    registry().set(opt, on);
}

void reset() {
    registry().reset();
}

}  // namespace openrac::renderer::render_opt
