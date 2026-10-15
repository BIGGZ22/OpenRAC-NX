# Moby animation

How RAC1 turns a moby's animation sequences into the joint palette VU0
skins with: per-tick advance, sequence changes and blends, keyframe
evaluation, the joint chain, and the attachment of one moby to another's
joint. ReRAC read it from the NTSC-U code (`SCUS_971.99`) and checked it on
every sequence of the 19 levels; OpenRAC's editor decodes the same data on
PAL and poses classes with it
([ASSETS.md](../../../games/rac1/docs/ASSETS.md#moby-skeletons-and-animations)).
The data layout is in [MOBY.md](../formats/MOBY.md#6-sequences-and-frames).

**Games.** RAC1. Wrench reads RAC2 and RAC3 frames with the same header and
leaves Deadlocked's undecoded (**reference**); the evaluator is not measured
in the sequels.

## 1. Functions (NTSC-U and PAL)

| Role | NTSC-U (boot) | PAL |
|---|---|---|
| Per-tick advance | 0x20D580 | `func_0020E3D0` |
| Keyframe evaluation and palette | 0x20E0E0 | `func_0020EF30` |
| Frame pointers from the indices | 0x20C880 | `update_moby_animation_state` `func_0020D6D0` |
| Cut to a sequence | 0x212ED8 | `set_moby_animation` `func_00213D28` |
| Blend to a sequence | 0x212F90 | `blend_moby_animation` `func_00213DE0` |
| Blend with flags | 0x2130D8 | `blend_moby_animation_ex` `func_00213F28` |
| Re-encode the current pose as a frame | 0x20EDE8 | `func_0020FC38` |
| Animated bounding sphere | 0x20E098 | `moby_anim_sphere_lerp` `func_0020EEE8` |
| Attach to a joint | 0x20CCA8 | `MobyAttachToJoint` `func_0020DAF8` |
| Mark a joint chain / evaluate it | 0x210850 / 0x2109B8 | `moby_mark_joint_chain` `func_002116A0` / `moby_anim_eval_chain` `func_00211808` |
| Attach / detach a joint modifier | 0x20CB10 (level 01: 0x264370 / 0x2643E8) | `AttachManipulator` `func_0020D960` / `DetachManipulator` `func_0020D9D8` |

No VU0 microprogram is involved: the code uses VU0 macro instructions inline.

## 2. Runtime fields (moby record)

| Offset | Field |
|---|---|
| 0x50, 0x51 | frame index of key A, key B |
| 0x52, 0x53 | sequence of A, B; A = 0xFF is a snapshot slot |
| 0x54 | f32 t, the blend from A to B |
| 0x58 | f32 speed (1; 0 freezes; negative plays backwards) |
| 0x5C | f32 rate: t increment per tick for this interval |
| 0x60 | pose-layer list |
| 0x64 | joint-modifier list |
| 0x68, 0x6C | frame A and B pointers |
| 0x70 | set each tick: bit 0 crossed a key, bit 1 wrapped (gameplay polls it) |
| 0x7C–0x7E | loop sound, voice, trigger count |
| 0xF0 | animated bounding sphere |

**Spawn**: zeroed (sequence 0, frames 0 and 0, t = 0), speed and rate 1. A
class with one sequence of at most one frame gets speed 0, and mode 0x40
(no advance) when its loop sound is none. Without an update function a moby
loops sequence 0.

## 3. Advance (once per moby per game tick)

With speed s, rate r and t: nothing when r or s is 0. Otherwise t′ = t + s·r
(t + r during a blend between two sequences). A t′ within [0.99609, 1.00391]
snaps to 1. Going forward past 1, repeatedly: the leftover is (t′ − 1) / r; A
takes B; B = A + 1, wrapping to 0 at the frame count and setting bit 1; r is
the sequence's rate override or the new A frame's rate; t′ = leftover × r.
Backwards is the mirror image. When A and B are in the same sequence, the
first trigger whose time falls in the interval just covered plays its sound.

Rates are 8 / Δtime, so keys authored at 30 Hz with rate 0.5 last 2 ticks.
The tick is taken to be 60 Hz (inferred, not traced in the main loop).

## 4. Changing sequence

- **Cut**: A = B = the sequence, A at the given frame, B the next; rate from
  A's frame; t kept.
- **Blend over N ticks**: if t > 0.025 or a pose layer or modifier is
  active, the current evaluated local pose is re-encoded as a frame in a
  snapshot slot (0x800 bytes each), A becomes that slot (sequence 0xFF); then
  B = the target key, t = 0, speed 1, rate = 1/N. The snapshot uses the frame
  format exactly (quaternions clamped to s16, scale records only where not 1,
  translation records only where not at rest).

## 5. Evaluation (`func_0020EF30`)

DMAs the hierarchy and the two frame payloads to the scratchpad, then per
joint record (0x40 bytes at 0x70000000 + 0x40 · j):

1. **Channels**: scale and translation records of A then B, onto the joint
   (inherited scale) or onto a separate post-scale list.
2. **Interpolation** with u = 1 − t: scale and translation lerped. For the
   quaternion, when A and B are consecutive keys of one sequence, a plain
   lerp **without normalising**; otherwise a lerp towards B or −B (by the
   sign of the four-component dot product) followed by normalisation. At
   t = 0 the raw key is used.
3. **Pose layers and modifiers** (only for mobys whose update code attaches
   them): layers blend whole joint records in by a weight; modifier mode 0
   composes a quaternion, multiplies the scale and adds a translation to its
   target joint, other modes blend.
4. **Local matrix**: rows `r0 = (1 − 2y² − 2z², 2xy − 2zw, 2xz + 2yw)` and so
   on (the rotation of the conjugate quaternion), each row × the inherited
   scale, translation in row 3.
5. **Chain**: `P_j = P_parent · L_j`, parents first.
6. **Post-scale**: rows of P_j scaled after the whole chain, so children do
   not see it. A quirk in the list building drops some joints' post-scale
   when both keys have records (167 key pairs on the disc); the exact rule is
   in ReRAC's `moby_animation.md`.
7. **Palette**: `F_j = P_j · S_j`, S the class skeleton; the w column of S is
   never read (the disc holds other values there). With joint count 0, the
   identity at slot 0.

ReRAC pinned the convention on five classes: frame 0 of sequence 0 gives
`P_j · S_j = I` within 2.4e-4; with the rows transposed the error is 1 to 2.
The game's arithmetic is VU0 macro operations in a fixed order, so a
bit-exact port runs them in that order on the console's float model
([HARDWARE.md](HARDWARE.md)).

## 6. Attachments and modifiers

**Items on Ratchet.** The hand, head, feet and back items are mobys placed
every frame at one of Ratchet's joint chains. The game evaluates only the
marked chain's joints (no post-scale, no skeleton step), giving the joint's
pose P_j in model space, then W = [R | position] · P_j with the translation
scaled by Ratchet's scale / 1024. The item takes W's translation as its
position and W's rows as its rotation, normalised for most items, and keeps
its own class scale. In gameplay nine chains are evaluated each frame; the
wrench is joint 56, the back items joint 5. Gloves, head items and boots
instead get a keyframe built from Ratchet's decoded local pose. Clank on
Ratchet's back is class 601, the pack 607.

**Modifiers.** `AttachManipulator` and `DetachManipulator` are the only
writers of +0x64. Their records hold a rotation quaternion, scale and
translation, a weight and a mode; the target joint is the first entry of
the class joint list's second byte list. Users include Ratchet's blink, the
vendor's hologram, the NPC head look-at (one shared function in every level,
called by about 40 classes), searchlights and missile fins.

## Open

- The main-loop tick rate (60 Hz assumed) and frame skipping.
- Which update functions replace sequence 0 at spawn; who fills the pose
  layers.
- What classes 1 and 2 animate.
- The trigger data block.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/moby_animation.md`,
`docs/formats/moby_rac1.md` section 0.4; PAL names from
`games/rac1/config/overlays/us_map.tsv`.
