// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A live moby's pose, evaluated from the running game's memory.
//
// The game animates its mobys itself (MobyAnimAdvance runs in the port as C:
// it steps each moby's keys and blend factor every frame), but the evaluator
// that turns those fields into joint matrices, MobyAnimEval, is hand-written
// VU0 code that fed the game's VU1 renderer and does not run in the port. So
// the renderer evaluates the pose itself, from exactly what MobyAnimEval
// reads (ReRAC docs/plan/moby_animation.md sections 2, 5 and 6):
//
//   moby +0x24   the class: +0x08 joint count, +0x14 the skeleton (inverse
//                bind matrices), +0x18 common_trans (rest translations and
//                parent words)
//   moby +0x50 / +0x51 / +0x52 / +0x53   frame and sequence of keys A and B
//   moby +0x54   t, the blend from A to B
//   moby +0x68 / +0x6c   the two keys' frame headers
//
// The keys are read where the moby points, so whatever set them (the class's
// own sequences, Ratchet's table of sequences, a blend's snapshot) is drawn
// as the game would draw it. The arithmetic is assets/geometry's
// moby_animation.cpp (the game's own, to the bit).

#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "assets/geometry/moby_animation.h"

namespace openrac::scene {

/**
 * The joint palette of the moby at `moby` (a game address): one matrix per joint of the class's
 * high LOD, F_j = P_j S_j in packed model units (rows: the images of the axes, then the
 * translation). Empty when the moby has no class, no joints or no readable key: draw it as stored.
 */
std::vector<assets::rac1::JointMatrix> moby_palette(
    std::span<const std::uint8_t> ram, std::uint32_t moby
);

/**
 * The pose matrices P_j of the same moby (each joint's frame in the model, before the inverse bind
 * matrix): what MobyAnimEvalChain leaves in the scratchpad for the game's own use (bone points,
 * attachments). Empty as for moby_palette.
 */
std::vector<assets::rac1::JointMatrix> moby_pose_matrices(
    std::span<const std::uint8_t> ram, std::uint32_t moby
);

/**
 * moby_palette into `out` (cleared first, its storage reused): the same matrices, bit for bit,
 * with the evaluation's temporaries kept per thread (the class's skeleton, rest and parent words,
 * the keys, the joint modifiers and pose layers, the scratchpad records) instead of allocated
 * anew. Nothing is cached between mobys or ticks: everything is read again from `ram` on every
 * call, so a changed key, a moved class or a reused moby slot is seen at once. A call made while
 * this thread's temporaries are in use (from within an evaluation) uses temporaries of its own.
 * Under OPENRAC_POSE_REFERENCE=1 it is moby_palette as before (new temporaries every call).
 * Returns whether `out` holds a palette.
 */
bool moby_palette_into(
    std::span<const std::uint8_t> ram, std::uint32_t moby, std::vector<assets::rac1::JointMatrix>& out
);

/** moby_palette as it was before the reused temporaries, whatever OPENRAC_POSE_REFERENCE says. */
std::vector<assets::rac1::JointMatrix> moby_palette_reference(
    std::span<const std::uint8_t> ram, std::uint32_t moby
);

/** OPENRAC_POSE_REFERENCE=1: moby_palette_into evaluates as moby_palette_reference. */
bool pose_reference();

/** OPENRAC_POSE_VERIFY=N (1..3600, off when unset): check the palettes of every Nth tick against
 * moby_palette_reference (the frontend counts and logs; diagnostic only). */
int pose_verify_every();

}  // namespace openrac::scene
