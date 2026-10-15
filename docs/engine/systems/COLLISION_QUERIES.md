# Collision queries

How RAC1 queries its collision: a segment (nearest hit), a sphere and a
vertical capsule (closest face, push-out) against the level mesh, the mobys
near the query and the hero groups. All the kernels are hand-written EE and
VU0 macro assembly. This is ReRAC's instruction-level reading of the NTSC-U
level 01 program (`SCUS_971.99` for the boot copies), ported and tested on
hand-built cells and on Novalis. The data is in
[COLLISION.md](../formats/COLLISION.md).

**Games.** RAC1. The sequels' kernels are not measured.

## 1. Functions (NTSC-U level 01 and PAL)

| Role | NTSC-U | PAL |
|---|---|---|
| Segment, nearest hit (`CollLine_Fix`) | level 01 0x211870; boot 0x1EFA68 | `func_L00_001EFFF0`; boot `func_001EFE10` |
| Sphere, closest face | level 01 0x212960 | `coll_sphere` `func_L00_001F10E0` |
| Vertical capsule (the hero's body) | level 01 0x2135A0 | `coll_capsule` `func_L00_001F1D20` |
| Sphere against mobys only | level 01 0x214468 | `coll_sphere_mobys` `func_L00_001F2BE8` |
| Sphere against hero groups | level 01 0x214D70 | `coll_sphere_hero_groups` `func_L00_001F34F0` |
| Cell lookup | level 01 0x2117D0; boot 0x1EF9C8 | `CollCellLookup` `func_L00_001EFF50`; boot `func_001EFD70` |
| Surface id of the last hit | level 01 0x2151D8; boot 0x1F0B58 | `CollType` `func_L00_001F3958`; boot `func_001F0F00` |
| Footstep class of the last hit | level 01 0x215208 | `CollSoundClass` `func_L00_001F3988` |
| Init: mesh pointer, hero group pointers | level 01 0x255740 | `CollInit` `func_L00_002420C0` |
| Moby grid update | level 01 0x265900 | `func_L00_00251B58` |
| Moby matrix and grid rectangle | level 01 0x265BD8 | `MobyBuildMatrix` `func_L00_00251E30` |
| Hero movement passes | level 01 0x233940 | `HeroCapsulePasses` `func_L01_00233F58` |
| Hero ground probe | level 01 0x232DC0 | `HeroGroundProbe` `func_L01_002333D8` |

The boot executable has only the segment kernel, the lookup and `CollType`;
the sphere and capsule kernels exist only in the level programs.

## 2. The output record (`CollOutput`)

Written on a hit; the function returns 1.

| Offset | Content |
|---|---|
| 0x00 | world mesh |
| 0x04–0x0C | moby pose cache: buffer (8 × 0x800), keys, round-robin index |
| 0x10 | query stamp; each moby is tested once per query |
| 0x14 | hit log index (64 entries of 0x40) |
| 0x18 | hit moby; 0 for the world mesh and hero groups |
| 0x1C | `0x1000 | type` for a triangle; negative for a moby primitive |
| 0x20 | hit point (segment) or closest point (sphere, capsule), world units |
| 0x30 | sphere and capsule: the centre pushed out to touch |
| 0x40 | face normal, not normalised; for a primitive, point − centre |
| 0x50–0x70 | the triangle's vertices |

**Flags**, shared by the three main queries: 0x1 skip the world mesh; 0x2
skip moby primitives; 0x4 selects which moby sub-mask; 0x10 two-sided
faces; 0x20 exclude faces whose surface id equals `(flags >> 8) & 0x1F`; 0x80
exclude faces with type bit 7.

## 3. How the queries work

- **Bounds**: a query outside [0, 1024)³ misses.
- **Segment**: three-axis grid walk; crossings of the planes 4k per axis,
  merged in t order (ties x, then y, then z), cells visited in order, and
  once a hit exists the walk stops at the first cell entered beyond it. A
  triangle is tested only when its vertices' outcodes overlap the
  sub-segment inside that cell. Ray against triangle: one-sided unless 0x10
  (the start must be on the +N side), inclusive edge tests, strictly closer
  hits replace the best. The hit t is not stored.
- **Sphere**: every cell of the bounding box within r of the centre; per
  triangle the centre is projected onto the plane (front side only unless
  0x10); if an edge test fails, the closest point on the first failing edge;
  a hit is accepted when d² ≤ best, then best = 0.99951 · d². Push-out: the
  centre is moved to r / √best from the hit.
- **Capsule**: like the sphere, with the axis point moved along the capsule's
  height; only cells within r of the base centre are gathered, so faces in
  cells reached only by the upper part are never seen.
- **Arithmetic**: in 1/1024 units relative to the cell centre being tested;
  products and sums truncate (VU0); sign tests read the float sign bit, so
  −0.0 counts as negative; there is no epsilon anywhere. ReRAC's port runs
  each operation in the game's order on the console's float model
  ([HARDWARE.md](HARDWARE.md)).
- **Scratchpad**: each leaf is copied by DMA (its cell word's size byte),
  double-buffered; vertices at 0x70002000 (256 × 16 bytes), walk lists at
  0x70003000–0x70003800.

## 4. Surface types

There is no table: behaviour is spread over comparisons in the code. What is
established:

| Id | Behaviour | Confidence |
|---|---|---|
| 0x1F | none / default (`CollType` returns −1) | high |
| 0 | water: the ground probe records its height and casts again excluding id 0 to find the floor; the hero capsule always excludes it | medium |
| 0x0D | liquid-like, level 12 only: height recorded like water | low |
| 0x0B | height stored separately; the ground snap is skipped | low |
| 9, 0x0A, 0x0C | rejected by wall and ledge checks, which then test the slope (20° and 75°) | medium |

Projectile and moby code test sets such as {0, 1, 3, 8, 0xB, 0xC, 0xD} for
"ground-like". The footstep class (bits 5–6, 3 = none) picks the sound
together with the level and the foot.

## 5. Mobys and hero groups

- **Moby grid**: 64 × 64 cells of 16 units (x, y), each a list of moby
  indices; a moby is registered by `MobyBuildMatrix` when its bounding
  rectangle changes and removed on delete. No per-tick rebuild.
- **Moby collision blob** (class +0x10): two u16 joint counts (pose for
  flag-0x4 queries and others), section sizes, then 0x20-byte primitives,
  8-byte vertices and 4-byte faces (triangles only). Primitive kinds: 1
  sphere; 2 sphere on a joint; 3 vertical cylinder (not rotated); 4 capsule
  between two joints. Bit 15 of a primitive's mask ends the list. The mesh is
  transformed by the moby's rows × scale and position; posed primitives read
  joint translations through an 8-entry pose cache. On the disc: 1,064 blobs,
  kinds 1–4 = 464 / 31 / 350 / 302.
- **Hero groups**: groups culled by sphere at × 64; vertices unsigned;
  one-sided sphere test, no filters.
- **Ties** have no query path: their collision is baked into the level mesh.

**Who calls what**: the hero runs up to eight capsule passes plus the hero
groups per movement, with flags 0x24 (or 0xD24); the ground probe is a
segment with flag 2; bolts and crates use segments and spheres with 0x22.

## Open

- Primitive kinds 2–4's exact geometry beyond what the port needed.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/collision_queries.md`,
`docs/formats/collision_rac1.md` section 6b; PAL names from
`games/rac1/config/overlays/us_map.tsv`.
