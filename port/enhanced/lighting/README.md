# Baked probe lighting

Probe GI needs Vulkan, enhanced rendering switched on at start and a valid
level bake. Without probes, original lighting applies and Enhanced controls are
disabled; settings are kept. OpenGL, and enhanced rendering off (the default),
use original lighting. Core Vulkan optimisations work either way. Interiors can still be too dark.

## Build

Every build compiles it in (`OPENRAC_ENHANCED_RENDERING` is ON by default; the
`enhanced` preset is the release preset under another folder). Switch it on with
`enhanced_rendering = 1` in the settings file, which the launcher's Mods tab
(**Enhanced graphics**) writes, or `OPENRAC_ENHANCED=1`; setting
`OPENRAC_PROBE_GI` also switches it on.

For the offline baker, `python3 tools/build_port.py --probe-baker` builds a
pinned Embree 4 from source as a static library into `openrac-probe-bake`, with
no Embree or TBB library beside it; this is what the launcher's installers
carry. Alternatively set `-DOPENRAC_PROBE_BAKER=ON` and install Embree 4 where
CMake can find it (Windows then needs its runtime DLLs on PATH). Python tools
need no extra packages; the game does not need Embree.

## In the launcher

Mods, **Enhanced graphics** turns it on for the disc being played and plays
with Vulkan. Switched on the first time, and again whenever the levels are read
from the disc again, it runs the `probe-bake` action:

```sh
python port/enhanced/tools/probes/bake_all.py --levels-dir <levels> \
  --output-dir <data>/probes/<version> --baker <openrac-probe-bake> --disc <disc> --jobs 0 --update
```

`--jobs 0` bakes one level per CPU thread, as many at once as half the memory
holds; `--update` keeps each level whose export, levels format, baker and
options are those of its bake (`openrac-bake.json`) and bakes the others again
beside it, replacing a bake only once the new one is verified. The game is
started with `--probes <data>/probes/<version>` and takes `level_NN/sparse.orpg`
from there; a level without a current bake keeps the original lighting.

## Bake a level

After extracting your own disc, run from the repository root:

```sh
python port/enhanced/tools/probes/bake_level.py \
  --level-dir build/native-ntsc-data/port/ntsc/level_01 \
  --output-dir build/probe-gi/novalis-new-batch
```

Use a fresh output directory outside the source level. The tool loads settings,
finds emission, plans dense regions, bakes sparse/dense probes and checks the
batch before registering it. Failures keep logs without registering partial
results. Existing batches are not overwritten; running games need a restart.

One native process loads and converts the level once, then shares its Embree
scene across every grid. Each file is read back, checked under runtime rounding
and compared with the sparse grid before the batch is registered. Source files
are checked before loading, after planning and after baking; changes abort the
batch. `batch.log` records diagnostics and each probe file has a `.txt` report.

Outputs include `sparse.orpg`, `region-NNN.orpg`, `sparse.orpg.regions.json`,
planning reports, logs and `bake-result.json` with format/bake versions.
`--rays` and `--bounces` set quality for the whole batch.

Dense-region clustering caches eligible pair scores in a priority queue. After
a merge, only pairs involving the changed region are recalculated. Stable IDs
preserve the original best-first tie order; generation counters discard stale
pairs. Region bounds, source assignments and coverage checks remain unchanged.

To bake every exported level in one version, use a fresh output root:

```sh
python port/enhanced/tools/probes/bake_all.py \
  --levels-dir build/native-ntsc-data/port/ntsc \
  --output-dir build/probe-gi/all-levels
```

Each `level_XX` directory gets a complete sparse/dense bake using its own
settings. Levels run sequentially; failures retain logs and the remaining levels
continue. `bake-all-result.json` records results, and any failure returns a
nonzero exit code. `--dry-run` lists levels without writing files. The same
`--baker`, `--disc`, `--rays`, `--bounces` and `--threads` options are supported.

Baking uses available CPU threads. `--threads N` limits `bake_level.py`,
`seed_regions.py` and `openrac-probe-bake`; 1 is serial and 0 is automatic.
Threads share scene acceleration and preserve output bytes and bake identity.
`probe_bake_parallel_test --benchmark` compares one, two, four and automatic
threads using synthetic data; it checks equality and prints time without
launching the game.

Embree uses separate opaque and cutout geometry. Opaque textures with unit
vertex alpha skip the alpha callback at cutoffs up to 0.5. Other cutouts use
bilinear alpha filtering inside traversal. Guaranteed zero-alpha textured
triangles are omitted only when the cutoff is positive. Untextured faces keep
their existing acceptance rules. Source materials and emitter sampling order
are preserved.

Sunlight and emitter visibility use occlusion queries that stop at the first
accepted blocker. Sun rays are unbounded; emitter rays end just before the
sampled light. Transport and probe placement still use closest hits. Each
primary ray shares its first hit between GI and distance moments: transport
stays unbounded, while moments clamp distance to the grid's visibility range
(normally 1.5 times the cell diagonal).

Exports need `bake-inputs/`. Re-extract older exports or pass
`--disc <owned-local-disc.iso>` for planning. Gaspar lava also needs exported
overlay/core inputs or a prepared legacy surface. Keep inputs and bakes local
and untracked; PAL and NTSC need their own exports.

## Level settings

Optional `probe-bake.json` overrides built-in defaults. Include `"version": 1`
and only changed values; the [example](../tools/probes/probe-bake.example.json)
lists every default. Delete the file and any mod profile to reset a level.
CLI/runtime overrides still win; `--print-profile` shows resolved bake settings.
Baking never edits the profile. Keep completed-bake profiles as input records,
and rebake sparse and dense together after scene or setting changes.

`sunlight_direction` optionally overrides primary sunlight with a world-space light travel
vector, for example `[0, 1, -1]` for downward light travelling toward +Y. Z is height.
The vector is normalised; zero and nonfinite vectors are rejected. Omit it or use `null` to
retain authored directions. Runtime direct lighting, cascade shadows, Ratchet's grounded
shadow and baked sun bounces share the override. Secondary fill and airborne downward shadows
retain their directions. Changing it requires rebaking the level.

The saved [Novalis NTSC profile](profiles/rac1-ntsc/level_01/probe-bake.json)
sets sky intensity to 0.5, sky weight to 0.3 and the primary sunlight direction.
Copy it to the exported `level_01/probe-bake.json` before baking, and keep it
there when playing with that bake. Saved profiles are not applied automatically.
They contain lighting settings only; extracted meshes and baked assets stay local.

| Setting | Default |
| --- | --- |
| Sparse / dense spacing | 256 / 4 units |
| Probe map | 6x6 texels, 8x8 with border |
| Rays / bounces | 1,024 / 3 |
| Sky intensity / sky colour weight | 1 / 0.7 |
| Sunlight RGB scale | `[1, 1, 1]` |
| Sunlight direction | Authored light sets |
| Terrain vertex albedo | `true` |
| Distance exponent / inside fraction | 32 / 0.75 |
| Relocation | 4 attempts within 45% of minimum spacing |

`sunlight_scale` multiplies primary sun RGB in runtime lighting and baked
bounces; each channel accepts 0 to 4. Equal channels change strength without
changing colour. `terrain_vertex_albedo` keeps authored terrain lighting as a
material tint. Both settings require a rebake.

Optional `probe-regions.json` controls dense placement:

```json
{
  "version": 1,
  "enabled": true,
  "padding": 8,
  "vertical_padding": 4,
  "max_probes": 4096,
  "max_regions": 512,
  "no_moby_seeds": false,
  "no_geometry_coverage": false,
  "geometry_max_rise": 32,
  "geometry_padding": 3
}
```

Only `version` is required. `enabled: false` makes a sparse-only level; space
levels can use wider sparse spacing. Dense seeds include authored lighting
volumes, gameplay markers, crates, bolts, pistons and enemies. Planning covers
piston travel and jump clearance. High airborne markers use separate ground and
actor bands instead of filling the empty air between them; known piston travel
remains continuous. Height-preserving merges run before and after classification:
a taller top cannot spread over a lower neighbouring footprint.

Collision queries classify indoor cells using
walkable floor support, overhead coverage across most of the cell and walls in
at least five of eight horizontal directions within 32 units. Enclosed cells
extend to their first ceiling, with at least one probe cell of surface support;
connected cells can fill gaps beyond the initial markers. Growth checks the
connection for walls and stays within the seed's floor-height band. Outdoor or
unresolved seeds keep their original height. Roof-only overhangs do not qualify.

Regions crossing indoor/outdoor boundaries are partitioned before bounded
merging and overlapping budget splits. Existing spacing and height/probe limits, plus the
512-region and 128 MiB runtime limits, remain in force; the planner does not
silently increase them. A 4,096-sample cap per seed bounds enclosure work.
`geometry_max_rise` still limits added height; ceilings beyond that cap are
reported as incomplete coverage. Missing collision ceilings remain unresolved.
The resulting bake uses the same lighting inputs and transport as before;
classification does not create lights from authored light-bank IDs.

`seed-plan.json` includes indoor and unresolved/outdoor sample counts,
connected cells added, height caps and sampling-cap diagnostics. Dense regions
are split with overlap. `seed-plan.json` records skipped seeds and limits;
enemy patrol routes are excluded. The all-level bake cache includes planner
scripts and per-level region/bake settings, so planner changes invalidate cached
layouts even when the native baker executable has not changed.
`seed_regions.py` can inspect or replace regions for an existing sparse bake, saving the previous list.

## Formats and compatibility

Only **file format 3, bake revision 1** is supported. Rebake older data.
Each texel integrates a regular 2x2 angular footprint, including emission.
Irradiance and distance moments combine the four directions before normalising.
The footprint softens visibility boundaries; thin walls can still leak.

Sampling patterns are cached by resolution and rounding mode across all grids.
Probe rotations and bounce/emitter directions remain specific to each probe.
Integration processes four texels together using portable C++, preserving each
texel's accumulation order. Distance exponent 32 uses repeated squaring; other
exponents use `pow`. No SIMD library or fast-math flags are required.

Headers contain game/level identity, scene/settings hashes and grid/atlas
layout. Values are little-endian; texture planes upload without repacking:

| Plane | Format | Contents |
| --- | --- | --- |
| Irradiance | R11G11B10F | E^(1/5), including map borders |
| Distance | RG32F | Mean distance and mean squared distance |
| Metadata | RGBA16F | Relocation XYZ and validity |

Volumes use rows of 64 probe tiles, with unused tiles zeroed. Runtime keeps
file bytes for upload/streaming; Vulkan still needs a staging copy. CPU tools
can expand maps. Sampling blends sqrt(E), squares back to E and divides by pi
once for diffuse shading. Distances and biases use game units; Z is height.

## Bake lighting

Default sky light is `0.3 * primary directional colour + 0.7 * sky_intensity * sky`.
`sky_weight` changes that mix. Terrain, ties, shrubs and selected static mobys
block and bounce light. Moving objects receive GI without updating the bake.

Runtime and bake share emission textures, colours and masks for lamps, energy
spheres, Orxon ooze/windows and Gaspar lava. `probe-emission.json` overrides
emission; ordinary prelighting is not emission. Animated liquids are averaged,
and prepared legacy lava takes priority to avoid double counting.

The offline baker separates scene validation/Embree ownership (`bake_scene.*`),
materials and sky (`bake_material.*`), transport (`bake_transport.*`), and probe
workers (`bake.cpp`). `bake_integration.*` contains the portable texel kernel.

The baker derives normals from triangles. Runtime uses camera-facing normals,
receiver biases and distance moments to limit leaks; thin walls can still leak.

## Dense regions

A sparse grid covers the level. Vulkan retains up to 512 dense regions within
128 MiB of atlases. A GPU pass frustum-culls and ranks regions by squared camera
distance to their bounds, breaking ties by region ID. The nearest 128 visible
regions share a lookup; each 16x16 screen tile stores a 128-bit candidate mask.
Fragments sample at most two dense volumes, checking visibility before blending
at boundaries. Regions outside the lookup use sparse lighting.

Secondary cameras build their own lookup. Transparent receivers use that lookup
without opaque-depth tile rejection. When GPU selection or atlas capacity is
insufficient, the CPU chooses two camera-visible regions and fades changes.

`<sparse.orpg>.regions.json` loads automatically. `OPENRAC_PROBE_REGIONS` selects
another list (`0` disables it); paths are relative to the list.
`OPENRAC_PROBE_DENSE` adds one explicit region.

The geometry-aware planner preserves padding and only merges boxes whose
rectangular union introduces no extra spatial volume. This retains smaller local
coverage instead of filling empty corners between overlapping seed boxes. More
regions can duplicate support probes and incur atlas row padding; the planner's
resident-byte report includes that cost. Capacity changes require a new bake.

Level loading hashes the source scene once for the sparse/dense asset set. Each
asset still validates its version, level, bake settings and scene fingerprint.

## Controls

Set `OPENRAC_PROBE_GI` to the sparse bake before launching. With valid probes,
F4 toggles enhanced lighting. The Enhanced menu also provides:

- Separate probe-lighting and modern-shadow toggles.
- GI/GTAO strength (default 1) and normal/view biases (default 0.5 units).
- Probe spheres: cyan sparse, green dense, gold relocated, magenta invalid.

GTAO mainly darkens indirect light. Modern shadows retain 10% directional/fill
light. Ratchet casts along sunlight when grounded and downward when airborne,
with an extra footprint cue in shade. See the
[debugging guide](../../../docs/port/DEBUGGING.md) for environment options.

## Checks and references

```sh
python -m unittest discover -s port/enhanced/tools/probes
```

Native fixtures cover formats, baking, CPU/GPU sampling, regions, shadows,
GTAO and feature gating. CI checks normal/enhanced builds with synthetic data,
software Vulkan and Embree.

Details: [GI](../docs/RAC1_PROBE_GI.md),
[sun shadows](../docs/RAC1_SUN_SHADOW.md),
[Ratchet's shadow](../docs/RAC1_HERO_SHADOW.md).
[NVIDIA RTXGI-DDGI](https://github.com/NVIDIAGameWorks/RTXGI-DDGI) informed
irradiance encoding and blending.
