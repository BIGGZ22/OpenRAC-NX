// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), the unit tests of
// crates/rc-formats/src/moby_anim.rs: ISC License, Copyright (c) 2026 ReRAC
// contributors. ReRAC's tests use values of real classes; these use values
// of their own, with the expected results worked out by hand.
//
// Moby animation: frame parsing, playback (advance, hard_cut) and the
// evaluator (quaternion rows, lerp and nlerp, post-scale and the chain).

#include "assets/geometry/moby_animation.h"

#include <cmath>

#include "common/ps2_float.h"
#include "tests/check.h"

using namespace openrac::assets;
using namespace openrac::assets::rac1;
namespace ps2 = openrac::ps2;

namespace {

constexpr std::array<s16, 4> kQuatIdentity = {0, 0, 0, 0x7fff};

bool close(f32 a, f32 b, f32 tolerance) {
    return std::abs(a - b) <= tolerance;
}

// A rotation of `degrees` about y, in 1.15.
std::array<s16, 4> quat_about_y(double degrees) {
    const double half = degrees * 3.14159265358979323846 / 360.0;
    return {
        0,
        static_cast<s16>(std::lround(std::sin(half) * 32768.0)),
        0,
        static_cast<s16>(std::lround(std::cos(half) * 32767.0)),
    };
}

ps2::V4 quat_v4(const std::array<s16, 4>& q) {
    return {
        ps2::bits(q[0] / 32768.0f),
        ps2::bits(q[1] / 32768.0f),
        ps2::bits(q[2] / 32768.0f),
        ps2::bits(q[3] / 32768.0f),
    };
}

MobyFrame make_frame(
    const std::vector<std::array<s16, 4>>& quats,
    const std::vector<MobyScaleRecord>& scales,
    const std::vector<MobyTransRecord>& trans,
    f32 rate
) {
    ByteWriter w;
    for (const auto& q : quats) {
        w.put(q);
    }
    for (const auto& s : scales) {
        w.put(s);
    }
    const std::size_t trans_offset = w.size();
    for (const auto& t : trans) {
        w.put(t);
    }
    w.pad_to(16);
    MobyFrame f;
    f.header.rate = rate;
    f.header.qwc = static_cast<u16>(w.size() / 16);
    f.header.quat_bytes = static_cast<u16>(quats.size() * 8);
    f.header.scale_count = static_cast<u16>(scales.size());
    f.header.trans_offset = static_cast<u16>(trans_offset);
    f.header.trans_count = static_cast<u16>(trans.size());
    f.quats = quats;
    f.scales = scales;
    f.trans = trans;
    f.payload = w.bytes();
    return f;
}

MobySequence make_sequence(std::vector<MobyFrame> frames, f32 rate_override = 0) {
    MobySequence s;
    s.header.frame_count = static_cast<u8>(frames.size());
    s.header.loop_sound = 0xff;
    s.header.rate_override = rate_override;
    s.frames = std::move(frames);
    return s;
}

MobyAnimClass one_joint(std::vector<std::optional<MobySequence>> sequences) {
    MobyAnimClass c;
    c.joint_count = 1;
    c.skeleton = {kIdentityJoint};
    c.rest = {{0, 0, 0}};
    c.parent_word = {0};
    c.sequences = std::move(sequences);
    return c;
}

// Two joints, joint 1 the child of joint 0. Frame 1 post-scales joint 0 by
// 0.75 on z and moves joint 1 to z = 21000.
MobyAnimClass two_joints() {
    MobyAnimClass c;
    c.joint_count = 2;
    JointMatrix s0 = kIdentityJoint;
    s0[3] = {0, 0, -1000, 1};
    JointMatrix s1 = kIdentityJoint;
    s1[3] = {0, 0, -20000, 1};
    c.skeleton = {s0, s1};
    c.rest = {{0, 0, 1000}, {0, 0, 20000}};
    c.parent_word = {0, 0x7000'0000};
    c.sequences.push_back(make_sequence({
        make_frame({kQuatIdentity, kQuatIdentity}, {}, {}, 0.25f),
        make_frame(
            {kQuatIdentity, kQuatIdentity},
            {MobyScaleRecord{{0x1000, 0x1000, 0x0c00}, 0, 0}},
            {MobyTransRecord{{0, 0, 21000}, 1, 0}},
            0.25f
        ),
    }));
    return c;
}

void frames_parse() {
    const MobyFrame f =
        make_frame({kQuatIdentity}, {MobyScaleRecord{{1, 2, 3}, 0, 0x80}}, {}, 0.5f);
    ByteWriter w;
    w.put(f.header);
    w.put_bytes(f.payload);
    // A sequence header at 0x40 whose one frame is at 0.
    std::vector<u8> blob = w.bytes();
    blob.resize(0x40);
    MobySequenceHeader h;
    h.frame_count = 1;
    h.trigger_count = 1;
    ByteWriter seq;
    seq.put(h);
    seq.put<u32>(0);            // the frame pointer
    seq.put<u32>(0x0010'0005);  // a trigger: sound 5 at 1/16 frame 16
    blob.insert(blob.end(), seq.bytes().begin(), seq.bytes().end());
    const MobySequence s = parse_sequence(blob, 0x40);
    CHECK(s.frames.size() == 1 && s.triggers.size() == 1 && s.triggers[0] == 0x0010'0005);
    CHECK(s.frames[0].header.rate == 0.5f);
    CHECK(s.frames[0].scales.size() == 1 && s.frames[0].scales[0].inherited());
    CHECK((s.frames[0].quat_at(0) == kQuatIdentity));
    CHECK((s.frames[0].quat_at(9) == std::array<s16, 4>{}));
    blob[0x40 + 0x1c + 3] = 0x10;  // a frame pointer with its top nibble set
    bool threw = false;
    try {
        parse_sequence(blob, 0x40);
    } catch (const AssetError&) {
        threw = true;
    }
    CHECK(threw);
}

void quaternion_rows() {
    const auto q = quat_about_y(24.0);
    const auto r = quat_rows(quat_v4(q));
    const f32 c = static_cast<f32>(std::cos(24.0 * 3.14159265358979323846 / 180.0));
    const f32 s = static_cast<f32>(std::sin(24.0 * 3.14159265358979323846 / 180.0));
    // The rows are the matrix as the game stores it, R transposed: row 0 is
    // (cos, 0, sin) for a positive rotation about y.
    CHECK(close(ps2::to_float(r[0][0]), c, 1e-4f) && close(ps2::to_float(r[0][2]), s, 1e-4f));
    CHECK(close(ps2::to_float(r[2][0]), -s, 1e-4f) && close(ps2::to_float(r[2][2]), c, 1e-4f));
    // 0x7fff (0.99997) gives the exact identity.
    const auto id = quat_rows(quat_v4(kQuatIdentity));
    CHECK((id[0] == ps2::V4{ps2::kOne, 0, 0, 0}));
    CHECK((id[1] == ps2::V4{0, ps2::kOne, 0, 0}));
    CHECK((id[2] == ps2::V4{0, 0, ps2::kOne, 0}));
}

void lerp_and_flip() {
    const ps2::V4 a = quat_v4(kQuatIdentity);
    const auto q = quat_about_y(-24.0);
    const ps2::V4 b = quat_v4(q);
    const u32 half = ps2::bits(0.5f);
    // nlerp of a and b is normalised.
    const auto n = ps2::to_floats(nlerp_flip(a, b, half, half));
    CHECK(close(n[0] * n[0] + n[1] * n[1] + n[2] * n[2] + n[3] * n[3], 1.0f, 1e-4f));
    // The same rotation with w negated lies in the other hemisphere: flipped.
    const ps2::V4 b_neg = quat_v4({q[0], static_cast<s16>(-q[1]), q[2], static_cast<s16>(-q[3])});
    const auto flipped = ps2::to_floats(nlerp_flip(b_neg, a, half, half));
    CHECK(flipped[3] < 0 && close(std::abs(flipped[1]), std::abs(n[1]), 1e-4f));
}

void post_scale_and_chain() {
    const MobyAnimClass c = two_joints();
    AnimState s = AnimState::spawn(c);
    s.frame_a = 1;
    s.frame_b = 1;
    const auto f = evaluate(c, s);
    CHECK(f.size() == 2);
    // Joint 0: P0 = identity at z 1000, post-scaled 0.75 on z; F0 = P0 S0.
    CHECK(close(f[0][2][2], 0.75f, 1e-6f));
    CHECK(close(f[0][3][2], 250.0f, 1e-2f));
    // Joint 1: at 1000 + 21000 before the post-scale, which it does not see.
    CHECK(close(f[1][3][2], 2000.0f, 1e-2f));
    CHECK(f[1][0][0] == 1.0f && f[1][0][1] == 0.0f && f[1][0][2] == 0.0f);
    CHECK(f[1][2][0] == 0.0f && f[1][2][1] == 0.0f && f[1][2][2] == 1.0f);
    // No sequence 0 frame: identity for every joint.
    MobyAnimClass empty = c;
    empty.sequences.clear();
    CHECK((evaluate(empty, s) == std::vector<JointMatrix>(2, kIdentityJoint)));
}

void advance_and_wrap() {
    // 15 keys, rate override 0.125 (8 ticks a key): a cycle is 120 ticks.
    std::vector<MobyFrame> frames;
    for (int i = 0; i < 15; ++i) {
        frames.push_back(make_frame({kQuatIdentity}, {}, {}, 0.125f));
    }
    const MobyAnimClass c = one_joint({make_sequence(frames, 0.125f)});
    AnimState s = AnimState::spawn(c);
    CHECK(s.speed == 1.0f);
    advance(s, c);
    CHECK(s.frame_a == 0 && s.frame_b == 1 && s.t == 0.0f && s.rate == 0.125f && s.flags == 1);
    int wraps = 0;
    for (int tick = 2; tick <= 121; ++tick) {
        advance(s, c);
        if (s.flags & 2) {
            ++wraps;
            CHECK(tick == 1 + 14 * 8);  // when B steps past the last key
        }
    }
    CHECK(s.frame_a == 0 && s.frame_b == 1 && s.t == 0.0f);
    CHECK(wraps == 1);
    // Backwards from (0 -> 1, t = 0) steps to (14 -> 0).
    s.speed = -1;
    advance(s, c);
    CHECK(s.frame_a == 14 && s.frame_b == 0 && s.flags == 3);
    CHECK(close(s.t, 0.875f, 1e-6f));
    // A class with one one-frame sequence and no loop sound is frozen.
    const MobyAnimClass still =
        one_joint({make_sequence({make_frame({kQuatIdentity}, {}, {}, 1.0f)})});
    const AnimState st = AnimState::spawn(still);
    CHECK(st.speed == 0.0f && st.skip_advance);
}

void snap_window() {
    std::vector<MobyFrame> frames;
    for (int i = 0; i < 3; ++i) {
        frames.push_back(make_frame({kQuatIdentity}, {}, {}, 0.3f));
    }
    const MobyAnimClass c = one_joint({make_sequence(frames)});
    // t' = 0.9975 is in the snap window: one key step, t = 0.
    AnimState s;
    s.frame_b = 1;
    s.t = 0.6975f;
    s.rate = 0.3f;
    advance(s, c);
    CHECK(s.frame_a == 1 && s.frame_b == 2 && s.t == 0.0f);
    AnimState below;
    below.frame_b = 1;
    below.t = 0.6f;
    below.rate = 0.3f;
    advance(below, c);
    CHECK(below.frame_a == 0 && below.flags == 0);
}

void post_scale_lists() {
    using V = std::vector<u8>;
    CHECK((post_scale_list(V{0, 1, 2}, V{0, 1, 2}) == V{0, 1, 2}));
    CHECK((post_scale_list(V{0, 1}, V{2, 3}) == V{0, 1, 2, 3}));
    // B lists 0 again: what followed it in A is cut off.
    CHECK((post_scale_list(V{0, 1, 3}, V{0, 2}) == V{0, 2}));
    CHECK((post_scale_list(V{1, 0}, V{0, 1}) == V{1}));
    CHECK((post_scale_list(V{4, 5}, V{}) == V{4, 5}));
    CHECK((post_scale_list(V{}, V{7}) == V{7}));
}

void hard_cuts() {
    MobyAnimClass c = two_joints();
    c.sequences.push_back(make_sequence({
        make_frame({kQuatIdentity, kQuatIdentity}, {}, {}, 0.5f),
        make_frame({kQuatIdentity, kQuatIdentity}, {}, {}, 0.5f),
        make_frame(
            {quat_about_y(24.0), kQuatIdentity}, {}, {MobyTransRecord{{0, 0, 21000}, 1, 0}}, 0.75f
        ),
    }));
    AnimState s = AnimState::spawn(c);
    s.t = 0.3f;
    s.flags = 3;
    CHECK(hard_cut(s, c, 1, 0));
    CHECK(s.seq_a == 1 && s.frame_a == 0 && s.seq_b == 1 && s.frame_b == 1);
    CHECK(s.rate == 0.5f && s.t == 0.3f && s.flags == 1);
    // Past the end: A the last key, B clamped to it (no wrap).
    CHECK(hard_cut(s, c, 1, 7));
    CHECK(s.frame_a == 2 && s.frame_b == 2 && s.rate == 0.75f);
    CHECK(!hard_cut(s, c, 5, 0));
}

// The keys as a live moby points at them (+0x68 / +0x6c): read with parse_frame from where they
// lie, then evaluated as MobyAnimEval takes them; the same palette as from the state.
void keys_from_memory() {
    const MobyAnimClass c = two_joints();
    ByteWriter w;
    w.put_bytes(std::vector<u8>(0x30, 0xEE));  // whatever lies before
    for (const MobyFrame& f : c.sequences[0]->frames) {
        w.put(f.header);
        w.put_bytes(f.payload);
    }
    const std::vector<u8> memory = w.bytes();
    const MobyFrame a = parse_frame(memory, 0x30);
    const MobyFrame b = parse_frame(memory, 0x30 + 0x10 + c.sequences[0]->frames[0].payload.size());
    CHECK(a.header.rate == 0.25f && b.trans.size() == 1 && b.scales.size() == 1);
    AnimState s = AnimState::spawn(c);
    s.frame_b = 1;
    s.t = 0.5f;
    CHECK((evaluate_keys(c, &a, &b, 0.5f, true) == evaluate(c, s)));
    // t = 0: key A alone, B not read.
    s.t = 0.0f;
    CHECK((evaluate_keys(c, &a, nullptr, 0.0f, true) == evaluate(c, s)));
    // A key it needs missing: identity.
    CHECK((evaluate_keys(c, &a, nullptr, 0.5f, true) == std::vector<JointMatrix>(2, kIdentityJoint)));
}

}  // namespace

int main() {
    frames_parse();
    quaternion_rows();
    lerp_and_flip();
    post_scale_and_chain();
    advance_and_wrap();
    snap_window();
    post_scale_lists();
    hard_cuts();
    keys_from_memory();
    return openrac::test::result();
}
