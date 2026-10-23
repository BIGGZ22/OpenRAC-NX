# Baked probe lighting

Probe GI needs an enhanced Vulkan build and a valid level bake. Without probes,
original lighting applies and Enhanced controls are disabled; settings are kept.
OpenGL and normal builds use original lighting. Core Vulkan optimisations work
in either build.

## Build

```sh
cd port
cmake --preset enhanced
cmake --build --preset enhanced
```

For the offline baker, also set `-DOPENRAC_PROBE_BAKER=ON` and install Embree 4
where CMake can find it. Windows needs its runtime DLLs on PATH. Python tools
need no extra packages; the game does not need Embree.

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

Outputs include `sparse.orpg`, `region-NNN.orpg`, `sparse.orpg.regions.json`,
planning reports, logs and `bake-result.json` with format/bake versions.
`--rays` and `--bounces` set quality for the whole batch.

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

| Setting | Default |
| --- | --- |
| Sparse / dense spacing | 256 / 4 units |
| Probe map | 6x6 texels, 8x8 with border |
| Rays / bounces | 1,024 / 3 |
| Sky intensity / sky colour weight | 1 / 0.7 |
| Sunlight RGB scale | `[1, 1, 1]` |
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
  "max_regions": 128,
  "no_moby_seeds": false,
  "no_geometry_coverage": false,
  "geometry_max_rise": 32,
  "geometry_padding": 3
}
```

Only `version` is required. `enabled: false` makes a sparse-only level; space
levels can use wider sparse spacing. Dense seeds include authored lighting
volumes, gameplay markers, crates, bolts, pistons and enemies. Planning covers
piston travel, jump clearance and nearby walls/ceilings, then splits large
regions with overlap. `seed-plan.json` records skipped seeds and limits;
enemy patrol routes are excluded. `seed_regions.py` can inspect or replace
regions for an existing sparse bake, saving the previous list.

## Formats and compatibility

Only **file format 3, bake revision 1** is supported. Rebake older data.
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

The baker derives normals from triangles. Runtime uses camera-facing normals,
receiver biases and distance moments to limit leaks; thin walls can still leak.

## Dense regions

A sparse grid covers the level. Vulkan keeps up to 128 dense regions within
128 MiB, choosing at most two per fragment through depth and 16x16 screen tiles.
Visibility is checked per volume before boundary blending. If selection or
atlas capacity is insufficient, the CPU selects two camera-visible regions and
fades changes. Secondary cameras use resident regions, not another camera's tiles.

`<sparse.orpg>.regions.json` loads automatically. `OPENRAC_PROBE_REGIONS` chooses
another list (`0` disables it); paths are relative to the list.
`OPENRAC_PROBE_DENSE` adds one explicit region.

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

Design: [GI](../docs/RAC1_PROBE_GI.md),
[sun shadows](../docs/RAC1_SUN_SHADOW.md),
[Ratchet's shadow](../docs/RAC1_HERO_SHADOW.md).
[NVIDIA RTXGI-DDGI](https://github.com/NVIDIAGameWorks/RTXGI-DDGI) informed
irradiance encoding and blending.
