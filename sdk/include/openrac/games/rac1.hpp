// Generated from sdk/bindings/rac1.json by sdk/tools/gen_binding.py. Do not edit.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// rac1's hook points as C++ types, for the SDK's C++ layer (openrac/mod.hpp, docs/port/MODS.md):
// each one's name and signature as openrac_api.hook takes them, and the type of a function of it.
#pragma once

#include <cstdint>

namespace openrac::rac1 {

// One pad report into the pad's state (PadState, at pad), each poll, before the tick reads it.
struct Pad_ProcessReport {
    static constexpr const char* name = "Pad_ProcessReport";
    static constexpr const char* signature = "void(u32,u32,i32)";
    using fn = void (*)(uint32_t, uint32_t, int32_t);
};

// The stick read into the hero, early in his update each tick.
struct Hero_UpdateMotionInput {
    static constexpr const char* name = "Hero_UpdateMotionInput";
    static constexpr const char* signature = "void()";
    using fn = void (*)();
};

// Eases the hero's facing toward his move yaw.
struct Hero_TurnToHeading {
    static constexpr const char* name = "Hero_TurnToHeading";
    static constexpr const char* signature = "void(i32,f32,f32,f32)";
    using fn = void (*)(int32_t, float, float, float);
};

// The hero's velocity at his ground speed along an angle; an angle above pi means his facing.
struct Hero_SetPlanarVelocity {
    static constexpr const char* name = "Hero_SetPlanarVelocity";
    static constexpr const char* signature = "void(f32)";
    using fn = void (*)(float);
};

// The follow camera's per-tick requests from the hero's state; the camera moby's update calls it before it places the eye.
struct FollowCamera_ApplyOverrides {
    static constexpr const char* name = "FollowCamera_ApplyOverrides";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// Ratchet's animation moved on by his animation speed, once a tick after his state's update.
struct Hero_AdvanceAnimation {
    static constexpr const char* name = "Hero_AdvanceAnimation";
    static constexpr const char* signature = "void()";
    using fn = void (*)();
};

}  // namespace openrac::rac1
