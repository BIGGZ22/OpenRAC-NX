// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The optimisation registry: one table of every optimisation of what drawing costs, each with a
// name, what it does, whether it is on by default and how far its frames may differ from its
// reference path's, and one place to switch any of them off. docs/port/RENDER_PERF.md is the
// glossary of the counters each one lowers.
//
// An optimisation keeps its reference path (the code as it was before it) for as long as it is
// registered: the draw code asks enabled() and takes the reference path when it is off.
// OPENRAC_RENDER_OPT_OFF=name,name (or all) switches entries off for a run, so that the
// equivalence test (tests/render_equivalence_test.cpp) and the benchmark can render a scene with
// an entry on and off and compare the frames. The twelve older *_OFF switches of
// docs/port/RAC1_GPU_DRIVEN.md stay what they are; no optimisation adds a variable of its own.
//
// A new entry is an enumerator of Opt and a row of the table in render_opt.cpp at the same place.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace openrac::renderer::render_opt {

// The optimisations, one per entry of the table, in its order.
enum class Opt : std::uint8_t {
    RegistrySelftest,  // registry-selftest: no drawing; the tests' own entry
    VkDrawClock,       // vk-draw-clock: the Vulkan draw reads the clock only while measured
    VkStateSkip,       // vk-state-skip: Vulkan state and binds recorded only when they change
    StaticBuffersDeviceLocal,   // static-buffers-device-local
    StaticBuffersForceStaging,  // static-buffers-force-staging
    TexturePrepare,             // texture-prepare: a level's textures made at its load
    VkDrawCache,  // vk-draw-cache: Vulkan pipelines and descriptor sets found without allocating
    AttachmentDiscard,     // attachment-discard
    GpuResidentInstances,  // gpu-resident-instances: static instances kept on the GPU
    ShadowStaticCache,     // shadow-static-cache: static shadow casters kept between frames
    PresentSingleCopy,     // present-single-copy
    VkTightSync,           // vk-tight-sync
    PipelinePrewarm,       // pipeline-prewarm: a run's pipelines made again ahead, on a worker
    PresentTiming,         // present-timing
    // vk-extended-dynamic-state: cull, front face, depth and stencil not in the pipeline's key
    VkExtendedDynamicState,
};

// The number of entries: VkExtendedDynamicState is the last enumerator of Opt.
inline constexpr std::size_t kOptCount = static_cast<std::size_t>(Opt::VkExtendedDynamicState) + 1;

// How far the frames an optimisation draws may differ from its reference path's. The default,
// zero, is byte-identical frames; anything else is a tolerance the maintainers accepted, named in
// the entry's package report.
struct Tolerance {
    double max_differing_fraction = 0.0;  // Of the frame's pixels, the share that may differ.
    int max_channel_delta = 0;            // How far one channel of a differing pixel may be off.

    // True for byte-identical frames: no pixel may differ.
    [[nodiscard]] bool exact() const {
        return max_differing_fraction == 0.0 && max_channel_delta == 0;
    }

    bool operator==(const Tolerance&) const = default;
};

// One optimisation and what a person needs to know to compare it against its reference path.
struct Entry {
    Opt opt;                       // Its enumerator, which is also its place in the table.
    std::string_view name;         // Its name in OPENRAC_RENDER_OPT_OFF, lower case with hyphens.
    std::string_view description;  // What it makes cheaper, in one line.
    bool default_on;               // Whether it is on when nothing switches it off.
    Tolerance tolerance;           // How far its frames may differ from the reference path's.
};

// What OPENRAC_RENDER_OPT_OFF's text names: the entries to switch off, whether it said all, and
// the names the table does not hold.
struct OffList {
    std::vector<Opt> off;              // The entries it names, in its order, each once.
    bool all = false;                  // It held the word all: every entry is off.
    std::vector<std::string> unknown;  // The names that are no entry's, in its order.
};

// Returns every entry, in the table's order, for listing them and checking the table.
[[nodiscard]] std::span<const Entry> entries();

// Returns the table's row of `opt`.
[[nodiscard]] const Entry& entry(Opt opt);

// Returns the entry named `name` (exactly, as entries() spells it), or null when there is none.
[[nodiscard]] const Entry* find(std::string_view name);

// Returns what `text` names: entry names and the word all, separated by commas, with spaces
// around each ignored and empty items skipped. A name the table does not hold goes into
// `unknown`. Reads nothing else; the reading behind OPENRAC_RENDER_OPT_OFF.
[[nodiscard]] OffList parse_off_list(std::string_view text);

// Returns whether `opt` is on: its default, unless OPENRAC_RENDER_OPT_OFF or set() switched it
// off. The variable is read at the first call of this or set(), and an unknown name in it is
// logged once as a warning and ignored.
//
// Hot path: after the first call, one test of an initialised static and one relaxed atomic load
// of an array element, so the draw path may ask it per draw. Any thread may call it.
[[nodiscard]] bool enabled(Opt opt);

// Switches `opt` on or off for the rest of the run, over its default and the variable. For the
// tests and the benchmark, which call it between frames on the thread that draws.
void set(Opt opt, bool on);

// Puts every entry back to its default with OPENRAC_RENDER_OPT_OFF applied, as the run began.
// For the tests and the benchmark, between frames.
void reset();

}  // namespace openrac::renderer::render_opt
