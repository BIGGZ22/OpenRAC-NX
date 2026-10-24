// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The port's environment variables: one table of every OPENRAC_* variable the port reads, with
// what kind of value it takes, what holds when it is unset and a line on what it does, and the
// typed reads of them. docs/port/DEBUGGING.md documents each one, and tests/env_test.cpp checks
// that the page and the table name the same variables.
//
// The variables are read once, all together, the first time any of them is asked for, and kept
// for the life of the program: they are for development, debugging and tests, and are set before
// the program starts (a player's choice is a setting, section 16 of docs/CODING_CONVENTIONS.md).
// Each kind of value is read as the code that called getenv for it read it, which env_test pins.
// Any thread may call these functions.
//
// A variable the port does not own (DISPLAY, VULKAN_SDK) is read through system_variable, at each
// call, and is not kept.

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace openrac::env {

// What a variable's value is.
enum class Kind : std::uint8_t {
    Flag,     // on or off: its Truth says which values turn it on
    Integer,  // a whole number in decimal
    Hex,      // a whole number in hexadecimal, with or without 0x
    Number,   // a decimal number that may have a fraction
    Text,     // a word, a list or a small script of its own syntax, which its reader parses
    Path,     // a file or a folder
};

// Which values turn a flag on. The flags were added one at a time and do not all agree: each
// keeps what its reader has always taken.
enum class Truth : std::uint8_t {
    NotAFlag,        // a variable of another kind
    Set,             // set to anything, the empty string included
    LeadingOne,      // a value that starts with 1
    One,             // the value 1 and nothing else
    NotEmpty,        // any value but the empty string
    NotEmptyOrZero,  // any value but the empty string and 0
    UnlessZero,      // anything but the value 0, unset included: on by default
};

// The variables, one per name: each enumerator is the name without OPENRAC_, in PascalCase
// (OPENRAC_GL_API is GlApi), in the table's order, which is alphabetical by name.
enum class Var : std::uint16_t {
    Anim,
    AudioTicked,
    CameraInstanceCullOff,
    CamLog,
    CollTrace,
    ControlFrames,
    CrashTest,
    Data,
    Debug,
    DebugDump,
    DebugMenu,
    Denormals,
    DepthReuseOff,
    Developer,
    Direct,
    Drawer,
    DumpDraws,
    DumpRam,
    DumpSnapshot,
    DumpTexture,
    DumpVif,
    EarlyLightingOff,
    Enhanced,
    Fps,
    FrameCost,
    FrameLog,
    FxDump,
    GlApi,
    GlHideExtensions,
    Gpu,
    GpuDrivenOff,
    GpuDriverDir,
    GpuDriverLibrary,
    GpuMarkers,
    GpuMaterialsOff,
    Guard,
    HeroShadow,
    Hidden,
    HidePasses,
    Hitch,
    HitLog,
    Keys,
    Log,
    MenuBar,
    MobyLog,
    MobySphereCullOff,
    NativeLibraryDir,
    NoBlend,
    NoPumpFiber,
    OpaqueColourBatchOff,
    OpaqueSpecialiseOff,
    Overlay,
    OverlayShot,
    Pacing,
    PauseAt,
    Press,
    ProbeBacklight,
    ProbeComputeOff,
    ProbeDense,
    ProbeGi,
    ProbeRays,
    ProbeRegions,
    ProbeSky,
    ProbeSkyIntensity,
    ProbeSpheres,
    ProbeTiledReference,
    ProbeView,
    ProbeZeroWeightReference,
    PvsOff,
    Reach,
    RenderOptOff,
    RoundNearest,
    ScriptBudgetMs,
    ShaderDir,
    ShaderReload,
    ShadowAtlasOff,
    ShadowBatchOff,
    ShadowFilterReference,
    ShadowGatherOff,
    ShadowLayerReference,
    ShadowSolidOff,
    Shot,
    ShotBetween,
    ShotFrom,
    ShowMissing,
    SlowFrame,
    SoftwareGl,
    SoundLog,
    Strict,
    SunInstanceCullOff,
    SunReceiverCullOff,
    SunShadows,
    SunTimingDetail,
    TimeScale,
    Trace,
    TraceCd,
    TraceInput,
    TraceMissing,
    TraceRelocation,
    TraceSif,
    TraceWad,
    UiLog,
    UiReplay,
    UiResidual,
    UiSkip,
    Uncapped,
    VkEffectUploadReuseOff,
    VkParticleTextureReuseOff,
    VkUniformReuseOff,
    VulkanLibrary,
    VulkanPipelineCache,
    VulkanValidation,
    Watch,
    Write,
};

// One row of the table: a variable and what a person needs to know to set it.
struct Variable {
    Var var;                       // Its enumerator, which is also its place in the table.
    std::string_view name;         // Its whole name, OPENRAC_ and the rest.
    Kind kind;                     // What its value is.
    Truth truth;                   // For a flag, which values turn it on; NotAFlag otherwise.
    std::string_view when_unset;   // What holds when it is unset, in words ("off", "1024").
    std::string_view description;  // What it does, in one line.
};

// Returns every variable, in the table's order, for listing them and checking the table.
[[nodiscard]] std::span<const Variable> all();

// Returns the table's row of `var`.
[[nodiscard]] const Variable& variable(Var var);

// Returns the variable whose whole name (OPENRAC_ and the rest) is `name`, or nothing when the
// table has no such variable. For the C entry point (openrac/host_env.h), which names variables by
// their strings; C++ names them by their Var.
[[nodiscard]] std::optional<Var> find(std::string_view name);

// Returns the value `var` was set to, or nothing when it is unset. The view stays valid for the
// life of the program, and its data() ends in a NUL, so it can be handed to C as a string.
[[nodiscard]] std::optional<std::string_view> value(Var var);

// Returns whether the flag `var` is on, by its row's Truth. `var` is a Kind::Flag.
[[nodiscard]] bool flag(Var var);

// Returns `var` read as a whole number, in hexadecimal for a Kind::Hex, as read_integer reads it;
// `fallback` when it is unset. `var` is a Kind::Integer or a Kind::Hex.
[[nodiscard]] std::int64_t integer(Var var, std::int64_t fallback);

// Returns `var` read as a number, as read_number reads it; `fallback` when it is unset. `var` is a
// Kind::Number.
[[nodiscard]] double number(Var var, double fallback);

// Returns the value of a variable the port does not own (DISPLAY, VULKAN_SDK), or nothing when it
// is unset. Read at each call and not kept, since the port sets some of them itself (DRI_PRIME).
[[nodiscard]] std::optional<std::string> system_variable(const char* name);

// Returns whether `value` (nothing for an unset variable) turns on a flag read by `truth`; a
// NotAFlag is never on. The reading behind flag, declared for the tests that pin it.
[[nodiscard]] bool reads_as_on(Truth truth, std::optional<std::string_view> value);

// Returns `value` read as std::strtoll reads it in `base` (10 or 16): leading spaces and a sign
// allowed, the digits up to the first other character, 0 when there are none, held to the range of
// std::int64_t. `fallback` when `value` is nothing. The reading behind integer.
[[nodiscard]] std::int64_t read_integer(
    std::optional<std::string_view> value, int base, std::int64_t fallback
);

// Returns `value` read as std::strtod reads it: leading spaces allowed, the number up to the first
// character that cannot continue it, 0 when there is none. `fallback` when `value` is nothing. The
// reading behind number.
[[nodiscard]] double read_number(std::optional<std::string_view> value, double fallback);

}  // namespace openrac::env
