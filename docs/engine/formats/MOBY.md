# Moby classes

The class blob of a moby, the engine's animated or scripted object: header,
mesh packets for three passes (high detail, low detail, metal), vertex
tables that drive VU0 skinning, skeleton, animation sequences, collision and
sound definitions. The layouts are Wrench's as ReRAC describes them, with
ReRAC's corrections from the game's own code (the EE skinning driver, VU0
program 104691, the animation evaluator), checked on every class of the
NTSC-U disc (2,968 class blobs; 22,227 high-detail, 3,435 low-detail and
1,123 metal packets). OpenRAC's editor reads the high-detail mesh, skeleton
and animations on PAL and passes every check on all 19 levels
([ASSETS.md](../../../games/rac1/docs/ASSETS.md#moby-classes)). How the
game draws and animates them: [MOBY_RENDERING.md](../systems/MOBY_RENDERING.md),
[MOBY_ANIMATION.md](../systems/MOBY_ANIMATION.md).

**Games.** RAC1, measured. Per Wrench (**reference**): the 0x48-byte header
is the same in all four games but some fields change meaning; RAC2 and later
use a 16-byte vertex table header with u16 fields (RAC1: 0x20 bytes of u32),
RAC2 adds a "corncob" block, RAC3 and Deadlocked add team palettes,
Deadlocked stores 0x30-byte skeleton matrices and an undecoded animation
format; RAC2 classes with header byte 0x0B set use the RAC1 layout. The
moby VU1 program 13859 was edited for RAC2. None of this is measured in
OpenRAC.

## 1. Where classes are

- **Level classes**: the moby class table in the core index
  ([LEVEL.md](LEVEL.md#4-class-tables)); offset 0 means no blob (spawn points,
  triggers, gadgets). Blobs are 0x40-aligned and have no size field.
- **Gadgets** (RAC1 only): 21 WAD-compressed blobs per level, always the same
  classes (71, 157, 163, 168, 175, 176, 177, 180, 185, 188, 190, 192, 208, 229,
  454, 483, 562, 585, 619, 849, 1251), byte-identical across levels, each with
  one texture slot. The level loader only registers them; the game
  decompresses one into one of two 0x18000-byte buffers when the hand item
  changes (`select_world_object_resource_tables`, NTSC-U 0x204A40, PAL
  `func_00205270`). Class 71 is the wrench.
- **Ratchet** (class 0) has empty sequence slots; his sequences are the 256
  offsets at core index +0x78, each blob in the same sequence format,
  offsets relative to itself.

## 2. Header (0x48 bytes)

All pointers are offsets from the blob start; 0 means absent.

| Offset | Type | Field |
|---|---|---|
| 0x00 | s32 | packet table; 0 = no mesh |
| 0x04 | 4 × u8 | high-detail, low-detail and metal packet counts; first metal packet |
| 0x08 | u8 | joint count (at most 111) |
| 0x09 | u8 | **low-detail joint count**: every low-detail vertex uses joints below max(this, 1) |
| 0x0A, 0x0B | u8 | first **glow** packet of the high and low list (0xFF or past the end: none) |
| 0x0C | u8 | sequence count |
| 0x0D | u8 | sound definition count |
| 0x0E | u8 | **LOD switch**: low detail when depth > this × 1024 raw units (0xFF: never) |
| 0x0F | u8 | count of 16-byte shadow entries, just before the skeleton ([SHADOWS.md](../systems/SHADOWS.md)) |
| 0x10 | s32 | collision (section 8) |
| 0x14 | s32 | skeleton: one 4 × 4 float matrix per joint |
| 0x18 | s32 | joint hierarchy (`common_trans`), 16 bytes per joint |
| 0x1C | s32 | joint lists (section 7) |
| 0x20 | s32 | GIF usage table |
| 0x24 | f32 | scale: a vertex is drawn at packed × scale / 1024 |
| 0x28 | s32 | sound definitions, 0x20 bytes each |
| 0x2C | u8 | bangle block, in quadwords |
| 0x2D | u8 | mip distance |
| 0x2E | s16 | unknown in RAC1 (the corncob pointer in RAC2) |
| 0x30 | 4 × f32 | bounding sphere |
| 0x40 | u32 | glow colour, R in the low byte; non-zero sets the moby's mode bit 0x10 |
| 0x44 | u16 | mode bits, ORed with the instance's |
| 0x46, 0x47 | u8 | type and more mode bits, unknown |

Then, at 0x48, the sequence pointers (0: empty slot).

**Sound definition** (0x20): min and max range (f32), min and max volume,
min and max pitch, loop byte, flags byte, s16 sound index, bank index.

## 3. Packets

Packet table entries (16 bytes): VIF list offset, its size in quadwords, the
offset of its texture unpack, vertex table offset, vertex data size in
quadwords, ⌈6n / 16⌉, ⌈4n / 16⌉ and n, the number of vertices sent to VU1.
The table holds the high-detail packets, then low detail, then metal, then
bangles.

**VIF list**: two or three unpacks, FLG set:

1. texture coordinates, V2-16 at VU 0xC2, s and t / 4096 (not in metal
   packets);
2. the index stream, V4-8 at 0x12D, after a 4-byte header (an unknown byte,
   the quadword offset of the GS setups, the first extra index, 0);
3. GS setups, V4-32 just after the indices, 64 bytes each: TEX1, CLAMP, TEX0,
   MIPTBP1 as A+D quadwords whose last four bytes carry more extra indices.

**Index stream.** One-based indices into the packet's vertices; bit 7 means
"do not draw the triangle this index completes". A kicked index draws the
triangle of the last three indices: strips with no consistent winding, drawn
double-sided. An index of 0 switches to the next GS setup and takes the next
extra index as the vertex: the first extra index is in the header, extra
index k (k ≥ 1) is byte 0xC of quadword k − 1 of the setups (quadword stride,
not block stride; with block stride 668 retail packets end early). An extra
index of 0 ends the packet; its last three indices (1, 1, 1) only flush the
VU1 pipeline and are not drawn. Every list (high, low, metal) starts with a
texture switch; a later packet without setups keeps the previous packet's
texture.

**Texture slot in TEX0's low word**: the class's texture slot, −1 untextured,
−2 chrome, −3 glass (metal packets only). The class loader rewrites every
setup at load (`MobyClassRelocate`, NTSC-U 0x203338, PAL `func_00203B70`,
and `BuildMobyAdGif`, NTSC-U 0x202D78, PAL `func_002035B0`):

- a slot ≥ 0: from the moby texture entry, as for terrain;
- −1: an 8 × 8 texture at GS block 0x3FFB whose every texel is 0x80, filled by
  `InitOnce` and never overwritten: under MODULATE the face shows the lit
  vertex colour and alpha (334 of the disc's 11,087 setups);
- −2, −3: the level's chrome map (128 × 128) and glass map (64 × 64), from the
  core index +0x90 to +0x9C (offsets into `gs_ram`), clamped, linear.

## 4. Vertex table (RAC1 form)

A 0x20-byte header of u32: matrix transfer count, two-joint, three-joint and
single-joint vertex counts, duplicate count, transfer count (the sum),
offset of the vertices, offset of the **colour multipliers**. Then 2-byte
matrix transfers (palette joint, VU0 address), 8-byte-aligned duplicates
(u16, cache slot × 128), and 16-byte vertices, followed by 1 to 6 trailing
records. The multipliers are four bytes (R, G, B, A) per transfer vertex,
0x80 = 1.0; every one on the disc is 0x80.

**Vertex record** (16 bytes):

| Bytes | Content |
|---|---|
| 0–1 | bits 0–8: the 9-bit cache id **of the vertex seven places earlier** (the VU1 pipeline depth); bits 9–15: palette joint to transfer, or (three-joint) the third VU0 address / 2 |
| 2–7 | two-joint: two VU0 addresses, two weights, transfer store address, blend store address. Three-joint: two addresses, three weights, blend store address. Single: load address, transfer store address |
| 8, 9 | normal azimuth a and elevation e, 256 steps per turn |
| 10–15 | s16 x, y, z |

- **Normal**: (cos a cos e, sin a cos e, sin e). Wrench swaps x and y; the
  game's table and the triangle winding both say otherwise (OpenRAC
  measured the same on PAL).
- **IDs**: the last ids sit in the trailing records, the remainder packed into
  the last record's bytes 4–15.
- **Duplicates** copy a vertex (position, normal, skin) from the 512-entry
  cache, possibly from an earlier packet of the same list, and take their own
  texture coordinates from the entries past the real vertices.
- **VU0 matrix slots.** VU0 memory holds 64 four-quadword slots (addresses
  are multiples of 4; 0xF4 means "do not store"). Per packet: the pre-loop
  transfers copy palette matrices into slots; each two-joint vertex first
  stores its transfer, then blends (w1 · S[a] + w2 · S[b]) / 256 and may cache
  the blend; three-joint vertices blend three; single-joint vertices store
  their transfer and then load one slot. Weights are /256 (every blended
  vertex on the disc sums to 256). Slot contents persist across packets of
  one list, so a blend made in one packet may be used in the next; nothing on
  the disc depends on state from another list. Replaying the slots gives each
  vertex up to three joints and weights (1,940,989 / 308,135 / 51,665 vertices
  with one, two, three joints on NTSC-U).

**Metal vertex table**: a 16-byte header (vertex count, then the output
offsets of positions and colours and the total output size) and 16-byte
vertices: s16 x, y, z; azimuth, elevation; three palette joints; a count (≤ 1:
one joint); three weights (/256); pad. Metal vertices read the palette
directly, with no slot cache.

## 5. Skeleton and hierarchy

- **Skeleton** (+0x14): one matrix per joint, rows 0–2 the axes and row 3 the
  translation, in packed units; the w lanes are never read. It is the inverse
  bind matrix: the game skins with F = P · S for the joint pose P.
- **Hierarchy** (+0x18): per joint the rest translation relative to its
  parent (three f32) and a word: 0 for the root, else 0x70000000 + 0x40 ×
  parent, a ready-made scratchpad address of the parent's pose matrix.
  Parents come first.

## 6. Sequences and frames

**Sequence header** (0x1C): bounding sphere; frame count; loop sound (0xFF
none); trigger count; 0; a trigger data pointer (four sequences on the disc);
**rate override** (f32, replaces every frame's rate when non-zero). Then the
frame pointers and the triggers (low half sound, high half time in 1/16 ticks).
The "special" frame format Wrench knows does not occur in RAC1.

**Frame** (16-byte header): f32 rate (t increment per tick to the next key;
rate × Δtime = 8 on 98,406 of 98,411 pairs), s16 time in 1/8 ticks, payload
size in quadwords, quaternion bytes (8 × joints), scale record count,
translation record offset and count. Payload:

- one quaternion per joint: s16 x, y, z, w / 32768;
- scale records: u16 sx, sy, sz / 4096, joint, flag (0x80: inherited, so it
  reaches the children; 0: applies only to this joint after the whole chain);
- translation records: s16 x, y, z in packed units, joint (signed byte), 0.
  Joints without one use their rest translation.

Classes 1 and 2 have joints and sequences but no skeleton; their translation
records name joints 52–86 of another skeleton, so a reader must bounds-check
joint indices. How the game evaluates frames: [MOBY_ANIMATION.md](../systems/MOBY_ANIMATION.md).

## 7. Joint lists

+0x1C: a count, then offsets of lists, each `{s16 n1; s16 n2; n1 bytes; n2
bytes; 0xFF}`. The game uses them as joint chains: the first byte list is a
root-to-joint chain (attachments evaluate only these joints), and the second
list's first entry is the target joint of an animation modifier
([MOBY_ANIMATION.md](../systems/MOBY_ANIMATION.md#attachments-and-modifiers)).

## 8. Collision, bangles, GIF usage

**Collision** (+0x10; 0: none). What the game's moby query kernels read
(the segment, sphere and capsule queries and `coll_sphere_mobys`,
[COLLISION_QUERIES.md](../systems/COLLISION_QUERIES.md#5-mobys-and-hero-groups)),
as the port's `port/game/rac1/host/collision.c` reads it from the game's
memory (adapted from ReRAC's port of the kernels), measured by OpenRAC's
editor on every class blob of both RAC1 discs: 1,064 on PAL and 1,064 on
NTSC-U, all 19 levels each, every one read and written back byte for byte
([moby_collision.py](../../../editor/moby_collision.py)).

| Offset | Type | Field |
|---|---|---|
| 0x00 | u16 | joints posed for the queries with flag 0x4 (the hero's movement passes among them) |
| 0x02 | u16 | joints posed for the other queries |
| 0x04 | s32 | primitive bytes, 0x20 each |
| 0x08 | s32 | face bytes, 4 each |
| 0x0C | s32 | vertex bytes, 8 each |
| 0x10 | | the primitives, then the vertices, then the faces |

So "part 1" is the primitives and "part 3" the faces. The blob starts
16-byte aligned and is zero-padded to 16; what follows it is the class's next
block (the shadow entries before the skeleton, else the skeleton) on every
blob of both discs, so its place is exactly its padded size. Counted per
level (a class in several levels once in each), 1,064 of the PAL disc's 2,972
class blobs have collision.

**Primitive** (0x20): s8 kind, u8 always 4 (no kernel reads it), u16 mask,
then by kind:

| Kind | Shape | Fields | PAL / NTSC-U |
|---|---|---|---|
| 1 | sphere | centre +0x10 (3 × f32), radius +0x1C | 464 / 464 |
| 2 | sphere on a joint | s32 joint +4, radius +0xC, offset from the joint +0x10 (zero on the disc) | 31 / 31 |
| 3 | upright cylinder | height +4, centre of the base +0x10, radius +0x1C; the base turns with the moby, the axis stays the world's Z | 350 / 350 |
| 4 | capsule between two joints | s16 joints +4 and +6, radius +0xC | 303 / 302 |

The bytes a kind does not name are zero on both discs. Mask bit 0: tested
by the queries that pass flag 0x4 (the hero's capsule passes, 0x24 / 0xD24);
bit 1: by the others; bit 15: set on the last primitive and only there (on
every blob with primitives). At most 15 primitives on the disc; the moby's
own `collision_disable` (+0x98) turns primitive *i* off with bit *i*.
Joints are the posed joint matrices' translations, so joint primitives move
with the animation; the header's counts are how many joints a query poses:
at least one past the highest joint the primitives of that query set name
(exactly that on 1,027 of the 1,064 blobs, more on the others, never less).

**Vertex** (8): s16 x, y, z and a zero. **Face** (4): three u8 vertex
indices and a type byte, as the level collision's
([COLLISION.md](COLLISION.md#3-leaf): surface id, footstep class, query
exclusion), one-sided with the game's normal (v2 − v0) × (v1 − v0). The
faces' byte indices reach 256 vertices (one blob, class 825 of level 3, has
257, the last unused); the kernel transforms at most 2,048. At most 446 faces
on the disc. Classes have the primitives only (486 blobs), the triangles only
(460), or both (118). Faces are tested by every query that tests mobys
(each face's type byte filtered as in the level mesh); primitives only by the
queries whose mask bit they have, and never by one with flag 0x2.

**Units**: a blob point p is at the moby's position + its rotation of
p × the moby's scale / 1024, where the moby's scale (+0x2C) is the class
scale times the instance's: the class's packed units, like its mesh. A hit on
a primitive reports no surface (`CollType` returns −1 for it).

**Where it lives**: in the level's core data, in the class blob; the loader
(`MobyClassRelocate`) adds the class's address to +0x10 and relocates nothing
inside it, so the blob may sit anywhere in the core. Each moby copies the
pointer at spawn (`Moby.collision`, +0x94), and code that sets it to 0 makes
that one moby intangible (the game does so in dozens of places); the moby's
record in the gameplay file has no field for it.

- **Bangles**: a 4-byte header, 15 slots of (high first packet, count, low
  first packet, count), then two 8-byte vectors per bangle, unknown.
- **GIF usage**: per regular packet with setups, 12 texture slots and the
  offset of its setup unpack, the last entry ORed with 0x80000000: the list the
  loader walks to patch every TEX0.

## Open

- RAC1: the shadow block's records; primitive byte 1 (always 4); the
  class mode bits one by one; the bangle vectors; the joint lists' second
  byte list beyond its first entry.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/moby_rac1.md`,
`docs/plan/moby_skinning_lighting.md`, `docs/plan/moby_animation.md`,
`docs/plan/moby_untextured.md`, and its port of the moby query kernels
(`crates/rc-game/src/collision_query/mobys.rs`, as port/game/rac1/host/collision.c
adapts it); PAL names from
`games/rac1/config/overlays/us_map.tsv`.
