# Occlusion culling, per frame

How RAC1 builds its visibility mask each frame from the camera position and
tests every terrain fragment, tie and moby against it before any frustum or
distance test. ReRAC's reading of the NTSC-U code (`SCUS_971.99`), verified
in the disassembly of the four functions below and against the data of all
19 levels. The data: [OCCLUSION.md (formats)](../formats/OCCLUSION.md).
RENDERER.md's summary (a 1,024-bit mask per 4-unit cell, read by the renderer
cores) is this system.

**Games.** RAC1. Not measured in the sequels.

## Functions (NTSC-U and PAL)

| Role | NTSC-U (boot) | PAL |
|---|---|---|
| Per frame, by mode | 0x1F2C10 (level 01: 0x2193F8) | `UpdateOcclusion` `func_001F2FB8` |
| Build the mask | 0x1F2820 | `BuildOcclVisibility` `func_001F2BC8` |
| Cell lookup | 0x1F2690 | `ParseOcclGrid` `func_001F2A38` |
| Neighbour lookup | 0x1F2768 | `GetOcclGridFromPair` `func_001F2B10` |

## 1. When

The frame render (`DrawWorld`, [RENDER_PIPELINE.md](RENDER_PIPELINE.md))
calls `UpdateOcclusion` near its top, before the sky and all world passes,
whatever the pass switches. The mode is set at level load: 2 (active) when
the level has a grid and mappings, else 0 (off). The debug menu calls it
"occlusion" with off, freeze and active; retail never changes it.

## 2. Building the mask

    mode 0: mask = all visible
    mode 1 (freeze): mask unchanged
    mode 2:
      c = (int)(camera × 0.25) per axis
      if the cell exists: mask = its mask; remember it as "previous"
      else:
        if fallback == 0: for each axis try the nearer neighbour, then the
            other; OR the hits; if any, mask = that union (remembered)
        if still nothing:
          fallback 1: all visible
          fallback 2 with an octant override: the octant's mask
          otherwise: the previous mask, or all visible if there is none
            or the debug camera is on
      set bit 1023

- Outside the grid the game keeps the last mask it used, so a camera that
  leaves the playable space can hide things in direct view; the game's
  camera never goes there.
- The fallback flag is 0 in play; the freeze game modes set 2 for the next
  frame, a camera placement path sets 1, the end of every frame clears it.
- The previous mask is never reset at level load in the code found (a port
  should reset it).

## 3. The test

Each renderer core copies the 0x80-byte mask to the scratchpad and, for each
object with occlusion word w, skips it when `mask[w >> 8] & (w & 0xFF)` is
0:

| Core | Where in its loop |
|---|---|
| `TfragProc` | first test; occluded fragments never reach the lighting list |
| `TieProc` | first real test (after a DMA wait), before draw distance; both tie passes |
| MobyProc | after the moby's state check, before the mode skip and the sphere tests; an occluded moby also gets +0x31 (drawn) cleared and no animation job |
| Shrubs | none |

Occluded mobys keep updating: the update loop does not read +0x31.

## Open

- The initial previous mask and the mask before the first frame.
- Why the camera placement path sets fallback 1; the gameplay meaning of
  the octant override.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/occlusion_culling.md`,
`docs/formats/occlusion_rac1.md`; PAL names from
`games/rac1/config/overlays/us_map.tsv` and
[RENDERER.md](../../port/RENDERER.md).
