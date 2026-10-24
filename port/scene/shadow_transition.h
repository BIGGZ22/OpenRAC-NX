// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Scene-owned progress shared by a shadow's projection and receiver strength.

#pragma once

namespace openrac::scene {
// Render-thread state for a reversible, timed shadow transition.
struct ShadowTransition {
    float value = 0;               // Current eased progress, shared by direction and darkening.
    float origin = 0;              // Progress at the latest target change.
    float target = 0;              // Requested endpoint, zero for grounded and one for airborne.
    double started_seconds = 0;    // Monotonic time at the latest target change.
    double previous_seconds = -1;  // Negative until the first valid frame.
};
}  // namespace openrac::scene
