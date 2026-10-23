// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The table of the port's environment variables and the reads of them (env.h). A new variable is
// an enumerator of Var, a row here at the same place, and a row in docs/port/DEBUGGING.md.

#include "common/env.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>

namespace openrac::env {
namespace {

// The number of variables: Write is the last enumerator of Var.
constexpr std::size_t kVariableCount = static_cast<std::size_t>(Var::Write) + 1;

// The bases read_integer reads a Kind::Integer and a Kind::Hex in.
constexpr int kDecimalBase = 10;
constexpr int kHexBase = 16;

// Every variable the port reads, in the order of Var. The descriptions are the short form of
// DEBUGGING.md's rows, which say more.
// clang-format off: one variable per row: its names, then how it is read, then what it does.
constexpr std::array<Variable, kVariableCount> kVariables = {{
    {Var::Anim, "OPENRAC_ANIM",
     Kind::Flag, Truth::UnlessZero, "on",
     "moby animation; 0 draws every moby in its bind pose"},
    {Var::AudioTicked, "OPENRAC_AUDIO_TICKED",
     Kind::Flag, Truth::Set, "off",
     "no sound device: sounds advance by one tick's samples each tick, so runs repeat"},
    {Var::CameraInstanceCullOff, "OPENRAC_CAMERA_INSTANCE_CULL_OFF",
     Kind::Flag, Truth::Set, "off",
     "profiling reference: aggregate-group camera culling"},
    {Var::CamLog, "OPENRAC_CAM_LOG",
     Kind::Flag, Truth::Set, "off",
     "the camera each drawn frame is seen from, in the log"},
    {Var::CollTrace, "OPENRAC_COLL_TRACE",
     Kind::Flag, Truth::Set, "off",
     "collision queries and hits by kind, totals on stderr every 2000 queries"},
    {Var::ControlFrames, "OPENRAC_CONTROL_FRAMES",
     Kind::Integer, Truth::NotAFlag, "never",
     "the run ends this many frames after the player gets control"},
    {Var::CrashTest, "OPENRAC_CRASH_TEST",
     Kind::Text, Truth::NotAFlag, "none",
     "KIND:FRAME: stops the program on purpose at that frame, to test the crash report"},
    {Var::Data, "OPENRAC_DATA",
     Kind::Path, Truth::NotAFlag, "--data",
     "the extracted data folder, when --data is not given"},
    {Var::Debug, "OPENRAC_DEBUG",
     Kind::Flag, Truth::Set, "off",
     "the game's state every 100 ticks, in the log"},
    {Var::DebugDump, "OPENRAC_DEBUG_DUMP",
     Kind::Path, Truth::NotAFlag, "none",
     "the frame dump's file, as --debug-dump"},
    {Var::DebugMenu, "OPENRAC_DEBUG_MENU",
     Kind::Integer, Truth::NotAFlag, "never",
     "opens the developer menu by script at this window frame"},
    {Var::Denormals, "OPENRAC_DENORMALS",
     Kind::Flag, Truth::Set, "off",
     "the game's thread keeps the host's denormal floats, to compare"},
    {Var::DepthReuseOff, "OPENRAC_DEPTH_REUSE_OFF",
     Kind::Flag, Truth::Set, "off",
     "profiling reference: the GTAO prepass depth discarded before the world's colour"},
    {Var::Developer, "OPENRAC_DEVELOPER",
     Kind::Flag, Truth::NotEmptyOrZero, "off",
     "the launcher's developer mode: the developer menu and the in-window menus"},
    {Var::Direct, "OPENRAC_DIRECT",
     Kind::Flag, Truth::Set, "off",
     "with --level N, goes straight into level N from the title world"},
    {Var::Drawer, "OPENRAC_DRAWER",
     Kind::Flag, Truth::Set, "off (on on Android)",
     "the overlay's menus in the drawer, as on Android, in place of the menu bar"},
    {Var::DumpDraws, "OPENRAC_DUMP_DRAWS",
     Kind::Text, Truth::NotAFlag, "none",
     "N or N-M: the 2D path's draws of those ticks, in the log"},
    {Var::DumpRam, "OPENRAC_DUMP_RAM",
     Kind::Integer, Truth::NotAFlag, "never",
     "writes the game's main memory to ram_FRAME.bin at that frame"},
    {Var::DumpSnapshot, "OPENRAC_DUMP_SNAPSHOT",
     Kind::Path, Truth::NotAFlag, "none",
     "writes the frame read back for the game's screen capture to that PNG"},
    {Var::DumpTexture, "OPENRAC_DUMP_TEXTURE",
     Kind::Hex, Truth::NotAFlag, "none",
     "writes each conversion of the texture at that block (TBP0) as a PPM and a PGM"},
    {Var::DumpVif, "OPENRAC_DUMP_VIF",
     Kind::Path, Truth::NotAFlag, "none",
     "writes the VIF data of the frame's display list to that file"},
    {Var::EarlyLightingOff, "OPENRAC_EARLY_LIGHTING_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: full-resolution GI and cascades (reload the level)"},
    {Var::Enhanced, "OPENRAC_ENHANCED",
     Kind::Flag, Truth::One, "on when OPENRAC_PROBE_GI names a bake",
     "enhanced rendering: 1 turns it on, any other value off"},
    {Var::Fps, "OPENRAC_FPS",
     Kind::Flag, Truth::Set, "off",
     "the ticks per second over each 300 ticks, in the log"},
    {Var::FrameCost, "OPENRAC_FRAME_COST",
     Kind::Integer, Truth::NotAFlag, "off",
     "every N frames drawn (200 for 1 or less), what a frame cost and where, in the log"},
    {Var::FrameLog, "OPENRAC_FRAME_LOG",
     Kind::Text, Truth::NotAFlag, "none",
     "FROM-TO or N: what each tick in that range drew, in the log"},
    {Var::FxDump, "OPENRAC_FX_DUMP",
     Kind::Integer, Truth::NotAFlag, "none",
     "with OPENRAC_DEBUG, each effect quad of the 100 frames from this one, in the log"},
    {Var::GlApi, "OPENRAC_GL_API",
     Kind::Text, Truth::NotAFlag, "desktop (es on Android)",
     "the graphics API: es, desktop or vulkan"},
    {Var::GlHideExtensions, "OPENRAC_GL_HIDE_EXTENSIONS",
     Kind::Text, Truth::NotAFlag, "none",
     "NAME,...: the renderer acts as if the driver lacked these GL extensions"},
    {Var::Gpu, "OPENRAC_GPU",
     Kind::Text, Truth::NotAFlag, "the discrete card (Linux)",
     "integrated: the system chooses the graphics card (Linux)"},
    {Var::GpuDrivenOff, "OPENRAC_GPU_DRIVEN_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: CPU instance visibility and direct mesh draws (Vulkan)"},
    {Var::GpuDriverDir, "OPENRAC_GPU_DRIVER_DIR",
     Kind::Path, Truth::NotAFlag, "the system's driver",
     "Android: the folder of the chosen GPU driver package"},
    {Var::GpuDriverLibrary, "OPENRAC_GPU_DRIVER_LIBRARY",
     Kind::Path, Truth::NotAFlag, "the system's driver",
     "Android: the driver package's Vulkan library, in OPENRAC_GPU_DRIVER_DIR"},
    {Var::GpuMarkers, "OPENRAC_GPU_MARKERS",
     Kind::Flag, Truth::Set, "off",
     "names timed GPU passes for Vulkan profilers (a build with OPENRAC_GPU_MARKERS=ON)"},
    {Var::GpuMaterialsOff, "OPENRAC_GPU_MATERIALS_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: individual mesh material uniforms and textures (reload shaders)"},
    {Var::Guard, "OPENRAC_GUARD",
     Kind::Text, Truth::NotAFlag, "none",
     "FRAME:ADDRESS:BYTES[:SKIP]: the first write into those bytes ends the program"},
    {Var::HeroShadow, "OPENRAC_HERO_SHADOW",
     Kind::Flag, Truth::Set, "off",
     "the hero's own shadow map, without the sun's cascades"},
    {Var::Hidden, "OPENRAC_HIDDEN",
     Kind::Flag, Truth::Set, "off",
     "with --window, the window is drawn but never shown"},
    {Var::HidePasses, "OPENRAC_HIDE_PASSES",
     Kind::Text, Truth::NotAFlag, "none",
     "NAME,...: the renderer's passes of those names are not drawn"},
    {Var::Hitch, "OPENRAC_HITCH",
     Kind::Flag, Truth::Set, "off",
     "each tick that took over 1.25 tick times, in the log"},
    {Var::HitLog, "OPENRAC_HIT_LOG",
     Kind::Flag, Truth::Set, "off",
     "each hit sphere with a template, and the mobys it listed, on stderr"},
    {Var::Keys, "OPENRAC_KEYS",
     Kind::Text, Truth::NotAFlag, "none",
     "FRAME[+HELD]:KEY,...: keys and clicks by script"},
    {Var::Log, "OPENRAC_LOG",
     Kind::Text, Truth::NotAFlag, "info",
     "the log's level: verbose or debug"},
    {Var::MenuBar, "OPENRAC_MENU_BAR",
     Kind::Flag, Truth::Set, "off",
     "the overlay's in-window menu bar on macOS too"},
    {Var::MobyLog, "OPENRAC_MOBY_LOG",
     Kind::Text, Truth::NotAFlag, "none",
     "N[:RADIUS]: every N ticks, the mobys near the hero, in the log"},
    {Var::MobySphereCullOff, "OPENRAC_MOBY_SPHERE_CULL_OFF",
     Kind::Flag, Truth::Set, "off",
     "profiling reference: animated mobys left unbounded in native culling"},
    {Var::NativeLibraryDir, "OPENRAC_NATIVE_LIBRARY_DIR",
     Kind::Path, Truth::NotAFlag, "none",
     "Android: the app's native library folder, where libadrenotools finds its hooks"},
    {Var::NoBlend, "OPENRAC_NO_BLEND",
     Kind::Flag, Truth::Set, "off",
     "one frame per tick: no interpolation between ticks and no vertical sync"},
    {Var::NoPumpFiber, "OPENRAC_NO_PUMP_FIBER",
     Kind::Flag, Truth::Set, "off",
     "Windows: the game and the window pump in one loop, not in fibers"},
    {Var::OpaqueColourBatchOff, "OPENRAC_OPAQUE_COLOUR_BATCH_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: opaque colour in per-layer order"},
    {Var::OpaqueSpecialiseOff, "OPENRAC_OPAQUE_SPECIALISE_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: the unspecialised mesh colour shaders"},
    {Var::Overlay, "OPENRAC_OVERLAY",
     Kind::Text, Truth::NotAFlag, "off",
     "1 or a list of sections: the debug overlay from the start (0: not)"},
    {Var::OverlayShot, "OPENRAC_OVERLAY_SHOT",
     Kind::Text, Truth::NotAFlag, "none",
     "N:PREFIX: every N ticks, the window with the overlay to PREFIX<tick>.png"},
    {Var::Pacing, "OPENRAC_PACING",
     Kind::Flag, Truth::Set, "off",
     "each frame shown and when, in the log"},
    {Var::PauseAt, "OPENRAC_PAUSE_AT",
     Kind::Text, Truth::NotAFlag, "none",
     "TICK[:FRAMES]: pauses the game at that window frame"},
    {Var::Press, "OPENRAC_PRESS",
     Kind::Text, Truth::NotAFlag, "none",
     "FRAME:MASK:FRAMES[:LX:LY[:RX:RY]],...: pad buttons on port 0 by script"},
    {Var::ProbeBacklight, "OPENRAC_PROBE_BACKLIGHT",
     Kind::Flag, Truth::One, "off",
     "the original back-light factors added to the probe GI's direct lighting"},
    {Var::ProbeComputeOff, "OPENRAC_PROBE_COMPUTE_OFF",
     Kind::Flag, Truth::Set, "off",
     "the CPU/frustum probe selector in place of compute coverage"},
    {Var::ProbeDense, "OPENRAC_PROBE_DENSE",
     Kind::Path, Truth::NotAFlag, "none",
     "a finer local probe bake (.orpg) beside the level's (0: none)"},
    {Var::ProbeGi, "OPENRAC_PROBE_GI",
     Kind::Path, Truth::NotAFlag, "the level's bake in the --probes folder",
     "the probe bake (.orpg) for terrain and moby GI (0: off)"},
    {Var::ProbeRays, "OPENRAC_PROBE_RAYS",
     Kind::Integer, Truth::NotAFlag, "1024",
     "the ray budget the probe bake must have been made with (16 or more)"},
    {Var::ProbeRegions, "OPENRAC_PROBE_REGIONS",
     Kind::Path, Truth::NotAFlag, "<OPENRAC_PROBE_GI>.regions.json",
     "a JSON manifest of finer probe regions"},
    {Var::ProbeSky, "OPENRAC_PROBE_SKY",
     Kind::Text, Truth::NotAFlag, "the bake profile's",
     "level or uniform: the sky the probe bake must have been made with"},
    {Var::ProbeSkyIntensity, "OPENRAC_PROBE_SKY_INTENSITY",
     Kind::Number, Truth::NotAFlag, "the bake profile's",
     "the sky intensity the probe bake must have been made with"},
    {Var::ProbeSpheres, "OPENRAC_PROBE_SPHERES",
     Kind::Text, Truth::NotAFlag, "off",
     "1 or xray: the probes' positions, coloured by status"},
    {Var::ProbeTiledReference, "OPENRAC_PROBE_TILED_REFERENCE",
     Kind::Flag, Truth::Set, "off",
     "reference: every resident probe volume evaluated per fragment, not tile masks"},
    {Var::ProbeView, "OPENRAC_PROBE_VIEW",
     Kind::Text, Truth::NotAFlag, "combined",
     "the probe GI's view: original, combined, indirect, direct or coverage"},
    {Var::ProbeZeroWeightReference, "OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE",
     Kind::Flag, Truth::Set, "off",
     "profiling reference: probe atlas reads for zero-weight valid corners"},
    {Var::PvsOff, "OPENRAC_PVS_OFF",
     Kind::Flag, Truth::Set, "off",
     "profiling reference: no baked camera visibility (Vulkan)"},
    {Var::Reach, "OPENRAC_REACH",
     Kind::Path, Truth::NotAFlag, "none",
     "the functions without C this run reached, written to that file"},
    {Var::RoundNearest, "OPENRAC_ROUND_NEAREST",
     Kind::Flag, Truth::Set, "off",
     "the game's thread keeps the host's rounding to nearest, to compare"},
    {Var::ScriptBudgetMs, "OPENRAC_SCRIPT_BUDGET_MS",
     Kind::Number, Truth::NotAFlag, "2",
     "the milliseconds a script mod may run each time the game calls into it"},
    {Var::ShaderDir, "OPENRAC_SHADER_DIR",
     Kind::Path, Truth::NotAFlag, "the build's source tree",
     "where a shader reload reads the files"},
    {Var::ShaderReload, "OPENRAC_SHADER_RELOAD",
     Kind::Integer, Truth::NotAFlag, "never",
     "reloads every shader program at that window frame"},
    {Var::ShadowAtlasOff, "OPENRAC_SHADOW_ATLAS_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: separate shadow targets and per-map submission"},
    {Var::ShadowBatchOff, "OPENRAC_SHADOW_BATCH_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: shadow casters submitted in model order"},
    {Var::ShadowFilterReference, "OPENRAC_SHADOW_FILTER_REFERENCE",
     Kind::Flag, Truth::Set, "off",
     "profiling reference: nine bilinear shadow taps instead of unique texels"},
    {Var::ShadowGatherOff, "OPENRAC_SHADOW_GATHER_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: 16 individual shadow texel fetches (reload the level)"},
    {Var::ShadowLayerReference, "OPENRAC_SHADOW_LAYER_REFERENCE",
     Kind::Flag, Truth::Set, "off",
     "reference: shadow buckets flushed after each layer"},
    {Var::ShadowSolidOff, "OPENRAC_SHADOW_SOLID_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: the alpha-capable shader for opaque static sun casters"},
    {Var::Shot, "OPENRAC_SHOT",
     Kind::Text, Truth::NotAFlag, "none",
     "EVERY:PATH: a PNG of every EVERY-th window frame, to PATH<frame>.png"},
    {Var::ShotBetween, "OPENRAC_SHOT_BETWEEN",
     Kind::Flag, Truth::Set, "off",
     "OPENRAC_SHOT also writes the frames between ticks"},
    {Var::ShotFrom, "OPENRAC_SHOT_FROM",
     Kind::Integer, Truth::NotAFlag, "0",
     "no shots before this frame"},
    {Var::ShowMissing, "OPENRAC_SHOW_MISSING",
     Kind::Flag, Truth::Set, "off",
     "primitives whose texture is missing are drawn with the magenta placeholder"},
    {Var::SlowFrame, "OPENRAC_SLOW_FRAME",
     Kind::Number, Truth::NotAFlag, "0",
     "every frame drawn takes that many milliseconds longer"},
    {Var::SoftwareGl, "OPENRAC_SOFTWARE_GL",
     Kind::Flag, Truth::Set, "off",
     "runs on a software rasteriser instead of refusing it"},
    {Var::SoundLog, "OPENRAC_SOUND_LOG",
     Kind::Flag, Truth::Set, "off",
     "each command the game sends its sound driver, on stderr"},
    {Var::Strict, "OPENRAC_STRICT",
     Kind::Flag, Truth::LeadingOne, "off",
     "a call to a code address no function has stops the program"},
    {Var::SunInstanceCullOff, "OPENRAC_SUN_INSTANCE_CULL_OFF",
     Kind::Flag, Truth::Set, "off",
     "profiling reference: every instance kept in a surviving cascade group"},
    {Var::SunReceiverCullOff, "OPENRAC_SUN_RECEIVER_CULL_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: square shadow-map caster bounds"},
    {Var::SunShadows, "OPENRAC_SUN_SHADOWS",
     Kind::Flag, Truth::UnlessZero, "on",
     "the sun's shadow cascades in gameplay; 0 turns them off"},
    {Var::SunTimingDetail, "OPENRAC_SUN_TIMING_DETAIL",
     Kind::Flag, Truth::Set, "off",
     "detailed cascade and layer GPU timers"},
    {Var::TimeScale, "OPENRAC_TIME_SCALE",
     Kind::Number, Truth::NotAFlag, "1",
     "the game's speed from the start, from 1/64 to 1"},
    {Var::Trace, "OPENRAC_TRACE",
     Kind::Path, Truth::NotAFlag, "none",
     "one line of the run's state per tick, written to that file"},
    {Var::TraceCd, "OPENRAC_TRACE_CD",
     Kind::Flag, Truth::Set, "off",
     "each disc read, on stderr"},
    {Var::TraceInput, "OPENRAC_TRACE_INPUT",
     Kind::Flag, Truth::Set, "off",
     "a port's buttons and sticks, at most every 25 frames, in the log"},
    {Var::TraceMissing, "OPENRAC_TRACE_MISSING",
     Kind::Flag, Truth::Set, "off",
     "a host stack trace on the first call to each function without C"},
    {Var::TraceRelocation, "OPENRAC_TRACE_RELOCATION",
     Kind::Flag, Truth::Set, "off",
     "the level code's relocation, in the log and on stderr"},
    {Var::TraceSif, "OPENRAC_TRACE_SIF",
     Kind::Flag, Truth::NotEmpty, "off",
     "the first 32 SIF binds and calls, on stderr"},
    {Var::TraceWad, "OPENRAC_TRACE_WAD",
     Kind::Flag, Truth::Set, "off",
     "each WAD decompression, on stderr"},
    {Var::UiLog, "OPENRAC_UI_LOG",
     Kind::Flag, Truth::Set, "off",
     "the caller of each 2D quad routine call and each quad it made, in the log"},
    {Var::UiReplay, "OPENRAC_UI_REPLAY",
     Kind::Flag, Truth::Set, "off",
     "the game's GS packets replayed for the 2D layer instead of native quads"},
    {Var::UiResidual, "OPENRAC_UI_RESIDUAL",
     Kind::Flag, Truth::Set, "off",
     "each kind of 2D primitive still drawn from the GS packets, once, in the log"},
    {Var::UiSkip, "OPENRAC_UI_SKIP",
     Kind::Text, Truth::NotAFlag, "none",
     "NAME,...: these 2D routines send the game's packets instead of native quads"},
    {Var::Uncapped, "OPENRAC_UNCAPPED",
     Kind::Flag, Truth::Set, "off",
     "the frame loop never waits: the game runs as fast as the machine allows"},
    {Var::VkEffectUploadReuseOff, "OPENRAC_VK_EFFECT_UPLOAD_REUSE_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: every effect texture upload copied and hashed (Vulkan)"},
    {Var::VkParticleTextureReuseOff, "OPENRAC_VK_PARTICLE_TEXTURE_REUSE_OFF",
     Kind::Flag, Truth::Set, "off",
     "reference: the texture of every visible particle hashed (Vulkan)"},
    {Var::VkUniformReuseOff, "OPENRAC_VK_UNIFORM_REUSE_OFF",
     Kind::Flag, Truth::Set, "off",
     "profiling reference: repeated uniforms uploaded, descriptors made again (Vulkan)"},
    {Var::VulkanLibrary, "OPENRAC_VULKAN_LIBRARY",
     Kind::Path, Truth::NotAFlag, "the bundle's MoltenVK",
     "macOS: the Vulkan library to open in place of the bundle's"},
    {Var::VulkanPipelineCache, "OPENRAC_VULKAN_PIPELINE_CACHE",
     Kind::Path, Truth::NotAFlag, "the user's cache folder",
     "where Vulkan pipelines are cached between runs (0, off or none: not at all)"},
    {Var::VulkanValidation, "OPENRAC_VULKAN_VALIDATION",
     Kind::Flag, Truth::LeadingOne, "off",
     "Khronos' validation layer, with OPENRAC_GL_API=vulkan"},
    {Var::Watch, "OPENRAC_WATCH",
     Kind::Text, Truth::NotAFlag, "none",
     "ADDRESS:COUNT[:i],...: values from game memory each frame, in the log"},
    {Var::Write, "OPENRAC_WRITE",
     Kind::Text, Truth::NotAFlag, "none",
     "FRAME:ADDRESS:VALUE[:FRAMES],...: words written into game memory by script"},
}};
// clang-format on

// Returns the place of `var` in the table.
constexpr std::size_t index(Var var) {
    return static_cast<std::size_t>(var);
}

// Returns whether every row is at the place of its enumerator, which variable() relies on.
constexpr bool rows_in_order() {
    for (std::size_t n = 0; n < kVariables.size(); ++n) {
        // A row added out of place would answer for another variable.
        if (index(kVariables[n].var) != n) {
            return false;
        }
    }
    return true;
}

static_assert(rows_in_order(), "each row of kVariables is at the place of its Var");

// The value of each variable as the program found it, in the table's order; nothing when unset.
using Snapshot = std::array<std::optional<std::string>, kVariableCount>;

// Reads every variable of the table from the environment.
Snapshot read_environment() {
    Snapshot values;
    for (const Variable& row : kVariables) {
        // std::getenv wants the name ending in a NUL, which a view need not have.
        const std::string name(row.name);
        const char* found = std::getenv(name.c_str());

        // An unset variable stays nothing; one set to the empty string is kept as set.
        if (found != nullptr) {
            values[index(row.var)] = std::string(found);
        }
    }
    return values;
}

// Returns the variables' values, read the first time any of them is asked for. They never change
// afterwards: a constant of the process rather than state (section 11.4), so a static of this
// function holds them, which C++ makes safe to read first from any thread.
const Snapshot& snapshot() {
    static const Snapshot values = read_environment();
    return values;
}

}  // namespace

std::span<const Variable> all() {
    return kVariables;
}

const Variable& variable(Var var) {
    return kVariables[index(var)];
}

std::optional<Var> find(std::string_view name) {
    for (const Variable& row : kVariables) {
        if (row.name == name) {
            return row.var;
        }
    }
    return std::nullopt;
}

std::optional<std::string_view> value(Var var) {
    const std::optional<std::string>& found = snapshot()[index(var)];

    // An unset variable has no value, which differs from the empty string.
    if (!found) {
        return std::nullopt;
    }

    // A view of the kept std::string, so its data() ends in a NUL (env.h promises it to C).
    return std::string_view(*found);
}

bool flag(Var var) {
    const Variable& row = variable(var);
    assert(row.kind == Kind::Flag);
    return reads_as_on(row.truth, value(var));
}

std::int64_t integer(Var var, std::int64_t fallback) {
    const Variable& row = variable(var);
    assert(row.kind == Kind::Integer || row.kind == Kind::Hex);

    // A Kind::Hex is the one integer written in hexadecimal.
    const int base = row.kind == Kind::Hex ? kHexBase : kDecimalBase;

    return read_integer(value(var), base, fallback);
}

double number(Var var, double fallback) {
    assert(variable(var).kind == Kind::Number);
    return read_number(value(var), fallback);
}

std::optional<std::string> system_variable(const char* name) {
    const char* found = std::getenv(name);

    // An unset variable has no value, which differs from the empty string.
    if (found == nullptr) {
        return std::nullopt;
    }

    return std::string(found);
}

bool reads_as_on(Truth truth, std::optional<std::string_view> value) {
    switch (truth) {
        case Truth::NotAFlag:  // asked of a variable that is not a flag
            return false;
        case Truth::Set:  // OPENRAC_TRACE_MISSING and most flags
            return value.has_value();
        case Truth::LeadingOne:  // OPENRAC_STRICT, OPENRAC_VULKAN_VALIDATION
            return value.has_value() && value->starts_with('1');
        case Truth::One:  // OPENRAC_ENHANCED, OPENRAC_PROBE_BACKLIGHT
            return value.has_value() && *value == "1";
        case Truth::NotEmpty:  // OPENRAC_TRACE_SIF
            return value.has_value() && !value->empty();
        case Truth::NotEmptyOrZero:  // OPENRAC_DEVELOPER
            return value.has_value() && !value->empty() && *value != "0";
        case Truth::UnlessZero:  // OPENRAC_ANIM, OPENRAC_SUN_SHADOWS
            return !value.has_value() || *value != "0";
    }
    return false;
}

std::int64_t read_integer(std::optional<std::string_view> value, int base, std::int64_t fallback) {
    // The caller's fallback stands for an unset variable; an empty one reads as 0, as in strtoll.
    if (!value) {
        return fallback;
    }

    // std::strtoll reads a string that ends in a NUL, which a view need not have.
    const std::string text(*value);

    return static_cast<std::int64_t>(std::strtoll(text.c_str(), nullptr, base));
}

double read_number(std::optional<std::string_view> value, double fallback) {
    // The caller's fallback stands for an unset variable; an empty one reads as 0, as in strtod.
    if (!value) {
        return fallback;
    }

    // std::strtod reads a string that ends in a NUL, which a view need not have.
    const std::string text(*value);

    return std::strtod(text.c_str(), nullptr);
}

}  // namespace openrac::env
