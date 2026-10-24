// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's geometry (game/common/frontend/geometry.h): the frame box for windows of each
// shape, the size the frame is drawn at, and the blends of a moby and the camera between ticks.
// Characterisation tests: they pin what the functions returned when they were moved out of
// frontend.cpp, so that the frontend's restructure can show it changed none of it. Each expected
// value is written as the arithmetic that gives it.

#include <cmath>

#include "check.h"
#include "game/common/frontend/geometry.h"
#include "renderer/math.h"

namespace {

using openrac::frontend::blend_matrix;
using openrac::frontend::blend_pose;
using openrac::frontend::Box;
using openrac::frontend::CameraPose;
using openrac::frontend::frame_box;
using openrac::frontend::frame_size;
using openrac::frontend::kTvAspect;
using openrac::frontend::same_moby;
using openrac::renderer::Mat4;
using openrac::renderer::Vec3;

// A 16:9 picture, the wider shape the player's setting offers.
constexpr float kWideAspect = 16.0f / 9.0f;

// The render scale that draws the frame at the box's own size (percent).
constexpr int kFullScale = 100;

// How far apart two floats may be and still count as the same result of float arithmetic.
constexpr float kTolerance = 1e-5f;

// True when `box` is at (`x`, `y`) and `width` x `height`.
bool box_is(const Box& box, int x, int y, int width, int height) {
    return box.x == x && box.y == y && box.width == width && box.height == height;
}

// True when `a` and `b` differ by at most kTolerance.
bool near(float a, float b) {
    return std::fabs(a - b) <= kTolerance;
}

// True when frame_size draws a box of `box_width` x `box_height` at `scale` percent as a frame of
// `width` x `height`.
bool size_is(int box_width, int box_height, int scale, int width, int height) {
    int drawn_width = 0;
    int drawn_height = 0;
    frame_size(box_width, box_height, scale, drawn_width, drawn_height);
    return drawn_width == width && drawn_height == height;
}

// A moby's matrix: no turn, at `x`, `y`, `z`.
Mat4 moby_at(float x, float y, float z) {
    return openrac::renderer::translation({x, y, z});
}

// A camera at `position` looking along `forward` (unit), with Z up.
CameraPose camera(Vec3 position, Vec3 forward) {
    CameraPose pose;
    pose.position = position;
    pose.forward = forward;
    pose.up = {0.0f, 0.0f, 1.0f};
    pose.left = openrac::renderer::cross(pose.up, forward);
    return pose;
}

// A window the TV's shape is filled by its 4:3 box.
void test_a_four_by_three_window_is_filled() {
    CHECK(box_is(frame_box(640, 480), 0, 0, 640, 480));
    CHECK(box_is(frame_box(640, 480, kTvAspect), 0, 0, 640, 480));
}

// A window wider than 4:3 puts the box in the middle of its width, as tall as the window.
void test_a_wide_window_centres_a_full_height_box() {
    // 720 * 4 / 3 = 960 wide, (1280 - 960) / 2 to each side.
    CHECK(box_is(frame_box(1280, 720), 160, 0, 960, 720));
    // 1080 * 4 / 3 = 1440 wide, (1920 - 1440) / 2 to each side.
    CHECK(box_is(frame_box(1920, 1080), 240, 0, 1440, 1080));
    // An ultrawide window: (2560 - 1440) / 2 to each side.
    CHECK(box_is(frame_box(2560, 1080), 560, 0, 1440, 1080));
}

// A portrait window puts the box in the middle of its height, as wide as the window.
void test_a_portrait_window_centres_a_full_width_box() {
    // 1080 * 3 / 4 = 810 tall, (1920 - 810) / 2 above and below.
    CHECK(box_is(frame_box(1080, 1920), 0, 555, 1080, 810));
}

// Odd and tiny windows round the box down, in integers, as the 4:3 path divides.
void test_odd_windows_round_the_box_down() {
    // 641 * 3 / 4 = 480 (480.75 truncated), (481 - 480) / 2 = 0 above.
    CHECK(box_is(frame_box(641, 481), 0, 0, 641, 480));
    // 333 * 4 / 3 = 444 wide, (1001 - 444) / 2 = 278 (278.5 truncated) to the left.
    CHECK(box_is(frame_box(1001, 333), 278, 0, 444, 333));
    // 1 * 3 / 4 = 0: a one-pixel window has a box no rows tall.
    CHECK(box_is(frame_box(1, 1), 0, 0, 1, 0));
    CHECK(box_is(frame_box(0, 0), 0, 0, 0, 0));
}

// A 16:9 picture fills a 16:9 window and is centred, rounded to the nearest pixel, in others.
void test_a_sixteen_by_nine_picture_rounds_to_the_nearest_pixel() {
    CHECK(box_is(frame_box(1920, 1080, kWideAspect), 0, 0, 1920, 1080));
    CHECK(box_is(frame_box(1280, 720, kWideAspect), 0, 0, 1280, 720));
    // 640 / (16 / 9) = 360 tall, (480 - 360) / 2 above.
    CHECK(box_is(frame_box(640, 480, kWideAspect), 0, 60, 640, 360));
    // 1080 * 16 / 9 = 1920 wide, (2560 - 1920) / 2 to each side.
    CHECK(box_is(frame_box(2560, 1080, kWideAspect), 320, 0, 1920, 1080));
    // 1080 / (16 / 9) = 607.5, which std::lround takes away from zero to 608.
    CHECK(box_is(frame_box(1080, 1920, kWideAspect), 0, 656, 1080, 608));
    // 1 / (16 / 9) = 0.5625, rounded to 1 row.
    CHECK(box_is(frame_box(1, 1, kWideAspect), 0, 0, 1, 1));
}

// The frame is the box at the render scale, the scale held to 25-400 percent.
void test_the_frame_is_the_box_at_the_render_scale() {
    CHECK(size_is(960, 720, kFullScale, 960, 720));
    CHECK(size_is(960, 720, 50, 960 / 2, 720 / 2));
    CHECK(size_is(1440, 1080, 200, 1440 * 2, 1080 * 2));
    // 10 percent is held to 25: a quarter of each side.
    CHECK(size_is(960, 720, 10, 960 / 4, 720 / 4));
    // 1000 percent is held to 400: 1440 * 4 = 5760 is still within the 8192 a texture may be.
    CHECK(size_is(1440, 1080, 1000, 1440 * 4, 1080 * 4));
    // 641 * 0.75 = 480.75 and 481 * 0.75 = 360.75, rounded to the nearest pixel.
    CHECK(size_is(641, 481, 75, 481, 361));
}

// A frame that would be wider or taller than a texture may be is scaled down to fit, both sides
// alike; an empty box still gives a frame of one pixel.
void test_the_frame_fits_a_texture_and_is_never_empty() {
    // 2560 * 4 = 10240 > 8192: both sides by 8192 / 10240 = 0.8.
    CHECK(size_is(2560, 1440, 400, 8192, 1440 * 4 * 4 / 5));
    CHECK(size_is(0, 0, kFullScale, 1, 1));
    // 1 * 0.25 rounds to 0, held to 1.
    CHECK(size_is(1, 1, 25, 1, 1));
}

// A moby that moved at most 4 units and turned less than the cosine 0.7 is the same moby.
void test_same_moby_takes_small_moves_and_turns() {
    CHECK(same_moby(moby_at(0, 0, 0), moby_at(4, 0, 0)));
    CHECK(!same_moby(moby_at(0, 0, 0), moby_at(4.01f, 0, 0)));
    // Turned 45 degrees about Z: X axis (0.7071, 0.7071, 0), its cosine 0.7071 >= 0.7.
    Mat4 turned = moby_at(0, 0, 0);
    turned[0] = std::sqrt(0.5f);
    turned[1] = std::sqrt(0.5f);
    CHECK(same_moby(moby_at(0, 0, 0), turned));
    // Turned 90 degrees: cosine 0.
    Mat4 quarter = moby_at(0, 0, 0);
    quarter[0] = 0.0f;
    quarter[1] = 1.0f;
    CHECK(!same_moby(moby_at(0, 0, 0), quarter));
    // A matrix with no X axis (a moby not set up yet) is never the same.
    Mat4 empty{};
    CHECK(!same_moby(empty, empty));
}

// A moby's matrix is blended at `t`, its turn too when it is the same moby.
void test_blend_matrix_moves_part_way() {
    const Mat4 half = blend_matrix(moby_at(0, 0, 0), moby_at(2, 2, -2), 0.5f);
    CHECK(near(half[12], 1.0f) && near(half[13], 1.0f) && near(half[14], -1.0f));
    CHECK(blend_matrix(moby_at(0, 0, 0), moby_at(2, 2, -2), 0.0f)[12] == 0.0f);
    CHECK(blend_matrix(moby_at(0, 0, 0), moby_at(2, 2, -2), 1.0f) == moby_at(2, 2, -2));
    // A scale from 1 to 3 blends to 2 halfway: the axes blend for the same moby.
    Mat4 grown = moby_at(0, 0, 0);
    grown[0] = 3.0f;
    CHECK(near(blend_matrix(moby_at(0, 0, 0), grown, 0.5f)[0], 2.0f));
}

// A moby that moved more than 4 units is drawn where it is now; one that turned too far blends only
// its place.
void test_blend_matrix_takes_a_jump_or_a_turn_whole() {
    const Mat4 far = moby_at(10, 0, 0);
    CHECK(blend_matrix(moby_at(0, 0, 0), far, 0.5f) == far);
    Mat4 quarter = moby_at(2, 0, 0);
    quarter[0] = 0.0f;
    quarter[1] = 1.0f;
    const Mat4 half = blend_matrix(moby_at(0, 0, 0), quarter, 0.5f);
    CHECK(half[0] == 0.0f && half[1] == 1.0f);
    CHECK(near(half[12], 1.0f));
}

// The camera's place and axes are blended at `t`, the axes kept square to each other.
void test_blend_pose_moves_and_turns_part_way() {
    const CameraPose from = camera({0, 0, 0}, {1, 0, 0});
    const CameraPose to = camera({4, 2, 0}, {std::sqrt(0.5f), std::sqrt(0.5f), 0});
    const CameraPose half = blend_pose(from, to, 0.5f);
    CHECK(near(half.position[0], 2.0f) && near(half.position[1], 1.0f));
    // Halfway between 0 and 45 degrees is 22.5: forward (cos 22.5, sin 22.5, 0).
    const float angle = std::atan2(1.0f, 1.0f) / 2.0f;
    CHECK(near(half.forward[0], std::cos(angle)) && near(half.forward[1], std::sin(angle)));
    CHECK(near(half.forward[2], 0.0f));
    // Up stays Z; left is up x forward.
    CHECK(near(half.up[2], 1.0f));
    CHECK(near(half.left[0], -std::sin(angle)) && near(half.left[1], std::cos(angle)));
}

// A camera that moved more than 8 units, or turned so its forwards' cosine is below 0.7, is a cut.
void test_blend_pose_takes_a_cut_whole() {
    const CameraPose from = camera({0, 0, 0}, {1, 0, 0});
    const CameraPose moved = camera({8.01f, 0, 0}, {1, 0, 0});
    CHECK(blend_pose(from, moved, 0.5f).position == moved.position);
    const CameraPose turned = camera({0, 0, 0}, {0, 1, 0});
    CHECK(blend_pose(from, turned, 0.5f).forward == turned.forward);
    CHECK(blend_pose(from, turned, 0.5f).left == turned.left);
    // Exactly 8 units is still blended.
    const CameraPose edge = camera({8, 0, 0}, {1, 0, 0});
    CHECK(near(blend_pose(from, edge, 0.5f).position[0], 4.0f));
}

}  // namespace

int main() {
    test_a_four_by_three_window_is_filled();
    test_a_wide_window_centres_a_full_height_box();
    test_a_portrait_window_centres_a_full_width_box();
    test_odd_windows_round_the_box_down();
    test_a_sixteen_by_nine_picture_rounds_to_the_nearest_pixel();
    test_the_frame_is_the_box_at_the_render_scale();
    test_the_frame_fits_a_texture_and_is_never_empty();
    test_same_moby_takes_small_moves_and_turns();
    test_blend_matrix_moves_part_way();
    test_blend_matrix_takes_a_jump_or_a_turn_whole();
    test_blend_pose_moves_and_turns_part_way();
    test_blend_pose_takes_a_cut_whole();
    return openrac::test::result();
}
