// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's geometry: where the game's picture sits in the window, at what size it is drawn,
// and the camera and the mobys drawn part way between two ticks. Arithmetic on what it is given
// only: nothing here reads the player's settings or the frontend's state (frontend.cpp passes them
// in), so that tests/frontend_geometry_test.cpp can pin each result.

#pragma once

#include <array>

#include "renderer/math.h"

namespace openrac::frontend {

// The shape of the picture the game was made for: a 4:3 TV.
constexpr float kTvAspect = 4.0f / 3.0f;

// A rectangle of the window, in window pixels.
struct Box {
    int x;       // The margin to its left.
    int y;       // The margin above or below it (the window's height left over, halved).
    int width;   // Its width.
    int height;  // Its height.
};

// A camera's place and axes: the part of a tick's view that is drawn part way between ticks.
struct CameraPose {
    renderer::Vec3 position{};  // Where the eye is, in game units.
    renderer::Vec3 forward{};   // The unit axis it looks along.
    renderer::Vec3 left{};      // The unit axis to its left.
    renderer::Vec3 up{};        // The unit axis above it.
};

// The game's frame as the TV showed it: 4:3 (the PS2's 512x448 PAL / 512x416 NTSC buffer), the
// largest such box in the window, centred; the rest of the window is black. As ReRAC's game frame
// (crates/rc-engine/src/display.rs; ISC License, Copyright (c) 2026 ReRAC contributors): the frame
// keeps the game's framing whatever the window's shape. `aspect`: a wider picture (the player's
// setting, settings.h), which shows more of the world to the sides.
Box frame_box(int window_width, int window_height, float aspect = kTvAspect);

// The size the frame is drawn at for a picture of box_width x box_height window pixels: the
// player's render scale of it, held to what a texture can be on any card the port runs on.
// `render_scale`: the player's render scale in percent (settings.h), held to 25-400. The size goes
// to `width` and `height`, each at least 1.
void frame_size(int box_width, int box_height, int render_scale, int& width, int& height);

// Whether `to` can be the moby `from` was a tick before: it moved at most 4 units (16 is that
// squared) and its X axis turned by less than about 45 degrees (0.7 is that angle's cosine).
bool same_moby(const renderer::Mat4& from, const renderer::Mat4& to);

// A moby's matrix part way from the last tick's to this one's, or this one's when it moved too far
// or turned too much in a tick to be the same thing moving (a moby slot used again, a teleport).
// `t`: how far, from 0 (`from`) to 1 (`to`).
renderer::Mat4 blend_matrix(const renderer::Mat4& from, const renderer::Mat4& to, float t);

// The camera part way from the last tick's to this one's (a cut is not blended): a camera that
// moved more than 8 units (64 is that squared), or turned so far that its forward axes' cosine is
// below 0.7, is `to`. `t`: how far, from 0 (`from`) to 1 (`to`).
CameraPose blend_pose(const CameraPose& from, const CameraPose& to, float t);

// Returns a world sphere (centre, radius) that holds a moby drawn with `drawn`, a matrix part way
// between `from` and `to`, whose joint poses blend between the ones `from_sphere` and `to_sphere`
// bound at those ticks: a conservative bound for linearly blended joint poses and model matrices.
// A missing endpoint bound, or a singular transform, returns an unknown sphere (radius 0).
std::array<float, 4> moby_sphere_envelope(
    const std::array<float, 4>& from_sphere,
    const renderer::Mat4& from,
    const std::array<float, 4>& to_sphere,
    const renderer::Mat4& to,
    const renderer::Mat4& drawn
);

}  // namespace openrac::frontend
