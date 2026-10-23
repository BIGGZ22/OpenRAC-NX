// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/moby_anim.rs
// (spec: docs/plan/moby_animation.md sections 1, 2, 3 and 6): ISC License, Copyright (c)
// 2026 ReRAC contributors.
//
// RAC1 moby animation: the sequences and keyframes of a class, and the
// evaluator that turns a playback state into the joint palette the skinning
// uses (F_j = P_j S_j, the posed joint times its inverse bind matrix).
//
// In the port the game's own decompiled code animates its mobys and hands the
// renderer their matrices; this is for everything that shows a class without
// the game: the level viewer, the editor, tests. It plays a sequence as the
// game does (NTSC-U):
//
//   advance    MobyAnimAdvance (boot 0x20d580): t += speed * rate per 60 Hz
//              tick, the [0.99609, 1.00391] snap to 1, the carry
//              (t - 1) / r_old * r_new, wrap at the sequence's end, play
//              backwards
//   hard_cut   0x212ed8: jump to a sequence and frame
//   evaluate   MobyAnimEval (0x20e0e0) without the runtime pose layers and
//              joint modifiers (moby +0x60 and +0x64, which only mobys with
//              special update code have): the channel decode, the sparse
//              scale and translation lists (with the game's post-scale list
//              quirk), plain lerp or nlerp with a hemisphere flip, the
//              parent chain, post-scale after the chain, then P S
//
// The arithmetic is the PS2's (ps2_float.h), in the game's order, so the
// palette is the game's to the last bit. Left out, being game behaviour the
// decompilation supplies: blends between sequences and their pose snapshot,
// sound triggers, pose layers and joint modifiers.

#pragma once

#include <array>
#include <optional>
#include <span>
#include <vector>

#include "assets/bytes.h"
#include "assets/geometry/moby.h"
#include "common/ps2_float.h"

namespace openrac::assets::rac1 {

// A joint matrix as four rows: row i is the image of axis i, row 3 the
// translation (M v = r0 x + r1 y + r2 z + r3).
using JointMatrix = std::array<std::array<f32, 4>, 4>;

inline constexpr JointMatrix kIdentityJoint = {{
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
}};

// A sequence header (0x1c bytes), at a class-relative offset of the class's
// pointer list at 0x48.
struct MobySequenceHeader {
    std::array<f32, 4> sphere{};  // bounding sphere, packed model units
    u8 frame_count = 0;
    u8 loop_sound = 0;  // 0xff = none
    u8 trigger_count = 0;
    u8 pad = 0;
    u32 trigger_data = 0;   // the gait record some walkers read (gait_records)
    f32 rate_override = 0;  // when non-zero, replaces every frame's rate
};

static_assert(sizeof(MobySequenceHeader) == 0x1c);

// A keyframe's header (16 bytes); its payload of `qwc` quadwords follows.
struct MobyFrameHeader {
    f32 rate = 0;        // t per tick from this key to the next (8 / delta time)
    s16 time = 0;        // in 1/8 ticks
    u16 qwc = 0;         // (trans_offset + 8 * trans_count + 15) >> 4
    u16 quat_bytes = 0;  // 8 per joint: the scale records' offset
    u16 scale_count = 0;
    u16 trans_offset = 0;  // quat_bytes + 8 * scale_count
    u16 trans_count = 0;
};

static_assert(sizeof(MobyFrameHeader) == 0x10);

// u16 sx, sy, sz in 4.12, joint, flags. Flag 0x80: the scale enters the
// local matrix (children see it); otherwise it scales only this joint, after
// the chain.
struct MobyScaleRecord {
    std::array<u16, 3> scale{};
    u8 joint = 0;
    u8 flags = 0;

    bool inherited() const { return (flags & 0x80) != 0; }

    bool operator==(const MobyScaleRecord&) const = default;
};

static_assert(sizeof(MobyScaleRecord) == 8);

// s16 tx, ty, tz in packed model units, joint, pad.
struct MobyTransRecord {
    std::array<s16, 3> trans{};
    u8 joint = 0;
    u8 pad = 0;

    bool operator==(const MobyTransRecord&) const = default;
};

static_assert(sizeof(MobyTransRecord) == 8);

struct MobyFrame {
    MobyFrameHeader header;
    std::vector<std::array<s16, 4>> quats;  // x, y, z, w in 1.15, one per joint
    std::vector<MobyScaleRecord> scales;    // in disc order (the order matters)
    std::vector<MobyTransRecord> trans;
    // The payload as the game reads it: a class with more joints than the
    // frame reads its quaternions from the bytes that follow, as the game does.
    std::vector<u8> payload;

    // Joint j's quaternion read from the payload (0 past its end).
    std::array<s16, 4> quat_at(std::size_t j) const;
};

struct MobySequence {
    MobySequenceHeader header;
    std::vector<MobyFrame> frames;
    // After the frame pointers: lo16 the sound, hi16 the time in 1/16 of a
    // key interval.
    std::vector<u32> triggers;
};

// A keyframe (header and payload) at `offset` of `base`: a class blob, or the
// game's memory, where a moby's +0x68 / +0x6c point at its two keys.
MobyFrame parse_frame(ByteView base, std::size_t offset);

// A sequence at `offset` of `base`; frame pointers are relative to `base`
// (the class blob, or a ratchet_seq lump for Ratchet's own sequences).
MobySequence parse_sequence(ByteView base, std::size_t offset);

// A class's sequences from its pointer list; none for empty slots.
std::vector<std::optional<MobySequence>> parse_sequences(
    ByteView class_blob, const MobyClass& moby
);

// The 0x50-byte records sequence headers +0x14 point at (the leg walkers'
// gait records): (sequence, 20 words) for each sequence that has one.
std::vector<std::pair<u8, std::array<u32, 20>>> gait_records(
    ByteView class_blob, const MobyClass& moby
);

// What the evaluator reads of a class.
struct MobyAnimClass {
    std::size_t joint_count = 0;
    std::vector<JointMatrix> skeleton;     // inverse bind; identity when absent
    std::vector<std::array<f32, 3>> rest;  // common_trans xyz: rest local translation
    std::vector<u32> parent_word;          // 0 for the root, else 0x70000000 + 0x40 * parent
    std::vector<std::optional<MobySequence>> sequences;

    MobyAnimClass() = default;
    MobyAnimClass(const MobyClass& moby, std::vector<std::optional<MobySequence>> sequences);

    const MobySequence* sequence(u8 seq) const;
    const MobyFrame* frame(u8 seq, u8 index) const;
    std::optional<std::size_t> parent(std::size_t joint) const;
};

// The animation fields of a moby (+0x50..+0x70).
struct AnimState {
    u8 seq_a = 0;  // key A's sequence and frame
    u8 frame_a = 0;
    u8 seq_b = 0;  // key B's
    u8 frame_b = 0;
    f32 t = 0;      // A to B
    f32 speed = 1;  // 0 freezes, negative plays backwards
    f32 rate = 1;   // t per tick for the current interval
    u8 flags = 0;   // set each tick: bit 0 crossed a key, bit 1 wrapped
    u8 trigger_count = 0;
    bool skip_advance = false;  // mode 0x40: the game never advances it

    // As a new instance starts (0x20c5f0): sequence 0 at rest; a class with a
    // single one-frame sequence is frozen.
    static AnimState spawn(const MobyAnimClass& anim);

    bool operator==(const AnimState&) const = default;
};

// One tick. Returns false (with flags cleared) when the rate or the speed is
// 0, or when a sequence the step needs is missing.
bool advance(AnimState& state, const MobyAnimClass& anim);

// Jumps to `seq` at `frame` (clamped): key A and B on the same sequence.
// Returns false, changing nothing, when the class has no such sequence.
bool hard_cut(AnimState& state, const MobyAnimClass& anim, u8 seq, s32 frame);

// Whether A and B are consecutive keys of one sequence (a plain lerp).
bool consecutive(const AnimState& state);

// The joints the post-scale list holds, linked as MobyAnimEval links it: a
// joint key B lists again is appended again, cutting off what followed it.
std::vector<u8> post_scale_list(std::span<const u8> a, std::span<const u8> b);

// The joint palette for a state: one matrix per joint (one identity for a
// class without joints; identity for every joint when a key is missing).
std::vector<JointMatrix> evaluate(const MobyAnimClass& anim, const AnimState& state);

// A node of a moby's joint-modifier list (moby +0x64, linked through +8; AttachManipulator
// func_0020D960 links one in front): what spins the vendors' holograms, turns an NPC's head, and so
// on. joint: the joint it acts on (+4, a scratchpad joint record 0x70000000 + 0x40 * joint); mode
// (+3): 0 composes (q = q * quat, scale * scale, translation + trans), else blends toward the node
// by weight (+0xC); quat (+0x10), scale (+0x20), trans (+0x30). After ReRAC's JointModifier and
// apply_modifiers (moby_anim.rs; ISC License, Copyright (c) 2026 ReRAC contributors).
struct JointModifier {
    u8 joint = 0;
    u8 mode = 0;
    f32 weight = 0.0f;
    std::array<f32, 4> quat{0.0f, 0.0f, 0.0f, 1.0f};
    std::array<f32, 3> scale{1.0f, 1.0f, 1.0f};
    std::array<f32, 3> trans{0.0f, 0.0f, 0.0f};
};

// A pose layer of a moby's +0x60 list (the 0x40-byte nodes PAL func_L00_00250060 links in, through
// +0x1C): Ratchet's weapon arm. Its own two keys (+0x38 / +0x3C, already in the moby's joint
// layout: a key of an animation-only class is retargeted into the node's buffer by the game), t
// (+0x24), whether they are consecutive keys of one sequence (+0x20..+0x23), the weight (+8), and
// the joints it covers (+0xC, +2 of them: the second byte list of its class joint list). It is
// blended over those joints after the moby's own keys, before the modifiers. After ReRAC's
// PoseLayer, layer_local and apply_layers (moby_anim.rs; ISC License, Copyright (c) 2026 ReRAC
// contributors).
struct PoseLayer {
    std::vector<u8> joints;
    const MobyFrame* a = nullptr;
    const MobyFrame* b = nullptr;
    f32 t = 0.0f;
    bool plain = false;
    f32 weight = 0.0f;
};

// The palette for two given keys, as MobyAnimEval takes them from the moby
// (+0x68 / +0x6c, +0x54): key A, key B (read only when t is not 0), t, and
// whether A and B are consecutive keys of one sequence (a plain lerp, no
// normalisation). Identity for every joint when a key it needs is missing.
std::vector<JointMatrix> evaluate_keys(
    const MobyAnimClass& anim,
    const MobyFrame* a,
    const MobyFrame* b,
    f32 t,
    bool plain,
    std::span<const JointModifier> mods = {},
    std::span<const PoseLayer> layers = {}
);

// The blend snapshot (0x20ede8 / PAL func_0020FC38 with flags 0x300, after ReRAC's moby_anim.rs
// snapshot): the current local pose from keys A and B (B read only when t is not 0) re-encoded as
// a keyframe, header and payload, as the game writes it into a blend slot for key A to point at
// while the moby blends to its next sequence. Empty when key A is missing.
std::vector<u8> encode_snapshot(
    const MobyAnimClass& anim, const MobyFrame* a, const MobyFrame* b, f32 t, bool plain
);

// A quaternion's rotation rows as the game builds them.
std::array<ps2::V4, 3> quat_rows(const ps2::V4& q);

// nlerp with the hemisphere flip.
ps2::V4 nlerp_flip(const ps2::V4& a, const ps2::V4& b, u32 u, u32 t);

}  // namespace openrac::assets::rac1
