# How other Switch homebrew ports moved to deko3d (after OpenGL or Vulkan)

Checked 2026-10-29 for OpenRAC-NX. This covers only ports that ran on Switch with OpenGL (Mesa) or
Vulkan (Mesa NVK) first and then gained a deko3d renderer. Direct3D-based paths are left out: no
Switch port reaches deko3d through D3D, and a D3D→deko3d layer would add a translation step we do
not have.

Local clones were in the Claude container: `/home/claude/sw/sms-switch`, `/home/claude/refs/lo`
(LostOdysseyRecomp, Ad0ky's Switch fork, with SwitchWakerHD's uam vendored in `thirdparty/uam`).

## The common pattern

Every port that got deko3d right after OpenGL or Vulkan hit the same few problems. Mesa's driver
never had them, so the "OpenGL looks right, deko3d does not" symptom is typical.

| Problem | Symptom on the console | Fix the ports used | OpenRAC-NX status |
|---|---|---|---|
| uam dual issue (devkitPro uam 1.1.0 pairs an ALU op with a load or SFU op; Mesa never does) | Wrong lighting, a black colour channel, 4x8-pixel specks; emulators look right because they ignore scheduling words | Never dual-issue (sms-switch `uam-no-dual-issue.patch`; SwitchWakerHD patch 7) | **Applied 2026-10-29**: pack rebuilt; 301 dual-issued instructions removed, 78 in each big moby vertex shader |
| uam uninitialised scheduling word (`Instruction::sched` read before it is set) | The same shader compiles differently from run to run; a block can read a register too early | `sched` initialised (sms-switch `uam-scheduling.patch`: 0; SwitchWakerHD patch 6: 0x7e0) | **Applied**: our shipped `862166b696cc81f8.frag.dksh` already differed by one stall from a clean rebuild of the same uam, which is this bug |
| uam stall before a new block (devkitPro/uam#7) | The next block passes a barrier that is not active yet (SwitchWaker torch halos) | Stall 2 when the next instruction is in another block (SwitchWaker / SwitchWakerHD patch 8) | **Applied** |
| deko3d 0.5 `BindVtxBuffers` reserves 5 words a buffer and writes 6 | Command memory overrun: GPU method errors and hangs after minutes | Reserve 6 (sms-switch `deko3d-cmd-reserve.patch`) or reserve extra yourself | Already worked around in dkgl (vertex-command reservation) |
| deko3d 0.5 separate alpha blend factor | Alpha blend uses the colour's destination factor (sms-switch shadows drew solid) | `deko3d-alpha-dst-factor.patch` | Already worked around in dkgl (blend commands written directly) |
| deko3d 0.5 waiting texture-cache invalidation at method 0x3DD, which the GPU does not have (NVIDIA's is 0x4CE) | A target drawn and then sampled reads stale texels (sms-switch: ground, sky, glyphs, screen copies) | `deko3d-texture-cache-invalidate.patch` | **Worked around 2026-10-29** in dkgl `cmd_barrier`: wait for idle, then the no-wait invalidation (0x4A2), as Mesa does |
| Sampler descriptor first read partway through a command list comes out stale; invalidating the sampler cache does not help | Surfaces in the wrong colour until the next submission (sms-switch: plaza ground in shadow colour) | Bind the sampler set again before a sampler not read since the last bind (sms-switch f73ec5b) | **Added 2026-10-29**, on by default |
| Zcull with depth direction and target changes | Near geometry or depth-biased decals dropped | Off by default (LostOdysseyRecomp `LO_DEKO3D_ZCULL` off; sms-switch has a switch) | Off by default since the 05:00 changes |
| Clip distances | Geometry clipped by enabled-but-unwritten planes | Enable only the planes a program writes (sms-switch 8d9279c) | Done (method 0x544 inline) |
| deko3d `CmdBuf::clear` leaks control chunks | Memory growth over a long session | sms-switch patch / vendored deko3d | Not fixed; minor for us so far |

## The ports

### sms-switch (Super Mario Sunshine), chasem-dev — Mesa OpenGL first, then dkgl
- **Path:** ran on Switch with Mesa's OpenGL (nouveau, with a Mesa shader disk cache). Then it gained
  `dkgl`, a GL-on-deko3d layer much like ours (commits de5c232 "Start a deko3d renderer", then
  #3 "deko3d" and #5 "eclipse-deko3d").
- **Shaders:** uam is built into the game as a library (it compiles GX shaders while the game runs),
  with three patches: `uam-library.patch`, `uam-scheduling.patch` and `uam-no-dual-issue.patch`.
- **deko3d:** built from source with patches: alpha destination factor, command reservation, texture-cache
  invalidate register, and the error report.
- **Lessons we took:** everything in the table marked as from sms-switch.
- **Performance ideas not yet taken:**
  - hardware compression on render targets (63164f3): GPU 11.3 → 9.5 ms a frame in Delfino Plaza;
  - drawing the default framebuffer straight into the swapchain images (3d6ae0a): no full-screen copy each frame;
  - a per-frame GPU-busy-time log (0ebc9e6).
- **Debugging approach worth copying:**
  - env switches for every suspect (`SMS_DKGL_SERIAL` waits after every draw, `_INVAL_EVERY`,
    `_SAMPLER_EVERY`, `_IMAGE_FRESH`);
  - a draw trace;
  - running on the console's own clocks for tests.

### SwitchWaker / SwitchWakerHD (Wind Waker GC / HD) — OpenGL first, then deko3d
- **Path:** started on OpenGL (Mesa 20.1, via `gfx/gl`), then moved to deko3d.
- **uam:** vendored with documented patches (`thirdparty/uam/PATCHES.md` in LostOdysseyRecomp's copy):
  1. real mutexes;
  2. DKSH in memory;
  3. a log callback;
  4. no `exit()`/`abort()`;
  5. a resident frontend;
  6. deterministic `sched`;
  7. **no dual issue** (bisected on the console, 2026-10-07: as compiled = specks, no dual issue =
     clean, same fps);
  8. **stall 2 before a new block** (torch-halo specks).
- **deko3d:** patches 1–2, including the `CmdBuf::clear` leak.
- **Lesson:** patch the shader compiler's scheduling before chasing GL-state bugs. Small test
  shaders pass with or without it; long ones (skinning, lighting) do not.

### LostOdysseyRecomp (Xbox 360 recompilation), Switch fork by Ad0ky — Vulkan (Mesa NVK) first, then deko3d
- **Path:** ran on Switch with plume's Vulkan backend on a statically linked Mesa NVK, then gained
  `LO_SWITCH_BACKEND=deko3d` (now what its releases use). See `docs/SWITCH.md`, "deko3d back end".
- **Backend:** `thirdparty/plume-deko3d`, plume's deko3d backend taken from sa2-nx / banjo-nx, with:
  - SPIR-V input (translated to GLSL for uam);
  - two device-wide descriptor heaps;
  - texture handles from the bound sets.
- **Shaders:** a PC-built DKSH pack from the Vulkan SPIR-V pack, with a console fallback compile
  (about 0.1 s each) cached on the SD card. This matches what we do with `dk_pack.py` / `dk_rebuild.py`.
- **GPU switches, all off by default** (one `LO_DEKO3D_GPU_FAST=1` turns them on):
  - compressed depth;
  - Zcull ("another port saw depth-biased decals dropped with it");
  - merged barriers;
  - lean barriers (no texture-cache invalidation for a target that is only written next);
  - trimmed vertex outputs.
- **Colour compression:** on by default, with its own memory blocks, because reused compressed memory
  showed stripes after resizing.
- **Lesson:** keep every speed-up behind a switch, defaulting to the safe setting, and log what is on
  at start (`video: deko3d GPU options`).

### sa2-nx / banjo-nx — the origin of plume-deko3d
- Named by LostOdysseyRecomp as the source of its deko3d backend.
- banjo-nx also wrote the extra uam optimisation passes in the LostOdysseyRecomp tree (PRMT,
  DomCSE…; `banjoFlags`).
- **We did not take those passes.** They change instruction selection, not just scheduling, so
  they are a separate experiment.

### Also seen earlier tonight (details not re-checked in this pass)
- warthog-switch (Halo), PR #5: the game on deko3d.
- WiiCompiled-Switch, PR #386.
- PlayGuard, PR #12.
- sdl3-nx: SDL_GPU on deko3d.
- Chronicle, PR #205.
- night100.
- NetherSX2_nx.

These went from GL or Vulkan to deko3d too, but the notes above come from the ports whose sources
were checked line by line.

## What this means for OpenRAC-NX
1. **Our pack was built with the unpatched uam.**
   - Rebuilding it with stock uam 1.1.0 (5a5afc2) reproduced 17 of 18 stages bit for bit.
   - The pack is now rebuilt with patches 6, 7 and 8 (`port/tools/deko3d/uam-sched-fix.patch`).
   - Only scheduling control words changed; every instruction word is identical.
   - The old pack ships too, at `romfs:/dkgl-stock` (`OPENRAC_DK_PACK=stock`), for an A/B on the console.
2. **Texture-cache invalidation after drawing to a target** now actually happens
   (`OPENRAC_DK_TEXINVAL=deko3d` for the old sequence).
3. **The sampler set is bound again before a fresh sampler** (`OPENRAC_DK_SAMPLER_REBIND=0` to turn off).
4. **Later, for FPS:** render-target compression, direct-to-swapchain drawing, and LostOdysseyRecomp's
   lean or merged barriers. Each would go behind a switch.
