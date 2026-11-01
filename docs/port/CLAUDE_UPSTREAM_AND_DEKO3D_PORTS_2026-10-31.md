# Upstream fixes ported, and what other deko3d Switch ports do (2026-10-31)

**CLAUDE DID NOT BUILD.** No game build, no shader-pack regeneration, no commit, no push.

What was not touched:
- the session-48 changes, which stay in the tree as they were;
- the tested NROs;
- the configs.

Backups of edited files: `build/claude-scratch/backup-pre-claude-upstream-2026-10-31/`.

## 1. Upstream check

The Switch branch is based on upstream OpenRAC at #77 (Oct 27). There are 78 upstream commits
since then; I compared up to #161 (Nov 1) in a separate clone, not the PC's git.

**Already in the Switch tree:**
- #95: a skipped frame drops its effect-texture notes, the crash fix;
- #96: the camera follows the ship off a planet;
- #98: movie language;
- #108: level background colour;
- #114: unusable camera.

**Not needed:** #158. The tree keeps retail's 224 class slots, and #158 only reverts a #148
regression the tree never took.

### Ported now

| Upstream | What it fixes | Files | Notes |
|---|---|---|---|
| **#151** (93200741) | **PAL freeze before the flight to Quartu.** hostgen sent `((s32 (*)(s32))func_001F98C0)(12)` through `OPENRAC_CODE`. It found no level copy, so it ran level 16's own function at 0x1F98C0, and the fade waited 1.4M frames. | `port/tools/hostgen/lower.py` | Our PAL source has exactly this call: `games/rac1/src/game/space.c:1878` (`#define scale_game_frames ((s32 (*)(s32))func_001F98C0)`). It is applied as upstream wrote it. The local `call()` has `wide_ret = None` (no `_wide_cast_callee`), so the guard on it never stops the fix. |
| #151 (part 2) | After 20 s with no picture, the log names the level, the game mode and the host call stack. | `port/game/common/program/vsync.cpp` | Diagnostic only. Upstream's `sent`/`sent_once` locals did not exist here, so they are added from `g_vsync_since_kick` exactly as upstream computes them. On the Switch `openrac_host_caller` may give only "?". The warning line still comes. |
| **#143** (a4f47014) | **Grey Pyrocitor on the vendor screen** (open issue). Mobys drawn into a 2D target sampled with the 2D path's clamping samplers still bound. | `port/renderer/direct.cpp` | `glBindSampler(0/1, 0)` before `target_mobys`. dkgl's `bind_sampler` takes 0, and no shader changed. |
| #155 (95fc8c86) | Plain fills and fades reach the right and bottom edge (a line of the old picture under fades). | `port/renderer/direct.cpp`, `port/tests/renderer_gif_test.cpp` | Only untextured on-screen sprites whose edge covers the last column or line move. `ceil16` is now a file-level helper. |
| #150 (993fc641) | Suck Cannon gauge missing with the HUD at the sides. | `port/renderer/direct.cpp` | Only the edge-band test for untextured rectangles changes. |

### Recommended later (not ported)

**Should come later, separately:**
- **#156 and part of #128** (flight between planets). Planned changes:
  - keep the picture while the game sends none (planet name as a block, stripes);
  - the name shown in a flight that starts on a planet;
  - preload the destination's files during the flight.

  This is medium-sized frontend work, and the preload thread needs its own review.
- **#157** (`collision.c` against the retail kernels; all levels bit-exact). Port only the
  `collision.c` fixes; leave out the asmc "shadows" and the shader part.
- #147: a landing with a full moby pool, mostly with mods.

**Do not port:**
- **#145 and #157's shader half:** they change `direct.frag` (`gl_SampleMask`), which means
  regenerating the shader pack, and the Switch has no MSAA.
- **#142, #146, #122:** the desktop pacing and low-latency rework. It conflicts with our render
  worker and frame-cap fix.
- launcher, SDK, editor, Android, enhanced and co-op work.

### Tests (Linux container)

- **`lower.py`:** `py_compile` OK. Nothing exercises it without a full hostgen run, which is the
  build.
- **`vsync.cpp`:** reviewed. It cannot be compiled here on its own; it is the game program's
  translation unit.
- **`direct.cpp`:** compiles (g++, ASan/UBSan). `renderer_gif_test` was linked against the
  session-45 harness's inert GL and TexturePool stand-ins:
  - the new `fill_to_the_edges` passes;
  - the old and new test binaries fail identically, at an existing check and a crash in
    `textures_and_vif`;
  - both come from the stand-in pool (no images), not from these changes.

**Codex must:**
- run `renderer_gif_test` on Windows;
- check the build's generated host C: the diff before and after must show `scale_game_frames`
  call sites turning into direct calls of `func_001F98C0`, and nothing else.

### Hardware checks (new build)

- A vendor's Pyrocitor preview: textured, not grey.
- A level fade before the story captions: no line along the right or bottom edge.
- The Suck Cannon gauge with the HUD at the screen edges.
- **PAL: pick Quartu in the ship.** The fade must end, and the flight must happen.
- The "has sent no picture for 1200 ticks" warning should appear only in a real hang.

## 2. Other Switch ports on deko3d (as of today)

New since the 2026-10-29 notes (`claude/switch-deko3d-ports-notes.md` in the project):

**warthog-switch** (Halo CE), PR #5, `port/switch/DEKO3D.md`.
- **newlib malloc lock, measured.** Their uam shader compile ran slower on two threads than one
  because Mesa hit newlib's malloc lock about 57 000 times a shader. A per-thread allocator fixed
  it:
  - 24.5 s on one thread with newlib;
  - 16.3 s with their allocator;
  - 8.8 s on two threads and 6.2 s on three.

  This is the same contention we suspect in the pose helper, measured on the same OS.
- **Thread placement:** `place_thread` logs "a thread starts on core N". A plain thread starts on
  the game thread's core, which is our `OPENRAC_POOL_CORES` finding too.
- **Profile fixes:**
  - a rename ring (up to 4 storages) took vertex-buffer lock waits from 13% to 0;
  - byte `memcmp` in draw comparisons (7.3%) became word compares;
  - logging moved to its own thread.
- **Memory:**
  - game memory as CPU-cached blocks, cleaned (not invalidated) per range read, instead of an
    uncached window;
  - a 4 MB per-frame upload slice and an 8 MB command slice;
  - 3 swapchain images (we already use 3).
- **Their conclusion:** "the limit was the CPU and its waits."

**SwitchWakerHD** (Wind Waker HD, GX2 → deko3d):
- Shaders come from Cemu's decompiler as GLSL and are compiled by a runtime uam on a background
  thread, about 70 ms each. The object being drawn shows up a few frames late.
- Results are cached on the SD, with an offline pack built on a PC from a harvested shader list.
- It has CPU and GPU clock profiles in its menu and dynamic resolution (1x handheld, 1.5x docked).
- About 30 fps at stock CPU and 460.8 MHz GPU.

**naruto-rise-of-a-ninja-nx and mgrr-nx** (Xbox 360 recompilation):
- They draw through Renut, a Direct3D-level renderer on plume's deko3d backend (from sa2-nx).
- Their offline shader chain is XenosRecomp → DXC → SPIRV-Cross → uam.
- Fights run at 32–47 fps at 720p, GPU-bound. Their codegen speed-ups include registers as locals,
  direct calls and non-volatile guest memory.

**sdl3-nx:** SDL_GPU on deko3d. Shaders are DKSH compiled from GLSL with UAM. Early: a clear test
plus halo3-nx's renderer starting on it.

**halo-ce-universal and SwitchWaker (GameCube):** Switch builds through Mesa OpenGL (ES), not
deko3d.

### What we could use

1. **The malloc-lock finding** supports the session-48 pose change. If the pool is ever wanted, a
   per-thread arena for hot paths is the proven route there.
2. **A logging thread.** Our diagnostic runs log a lot from the game thread to the SD card, and
   warthog measured gains from moving logging to its own thread.
3. **Word-wise compares** wherever we `memcmp` large buffers per tick. The image-send comparison
   uses hashes already, so this only matters if a profile shows it.
4. **Already in place:**
   - a render worker on another core;
   - 3 swapchain images;
   - offline DKSH pack (no runtime uam, so none of their compile stutter);
   - thread placement logging.

Sources:
- https://github.com/topics/deko3d
- https://github.com/ChupathingyCE/warthog-switch/pull/5
- https://github.com/ChupathingyCE/warthog-switch/blob/main/port/switch/DEKO3D.md
- https://github.com/centollOS/SwitchWakerHD
- https://github.com/centollOS/SwitchWaker
- https://github.com/malinkin-s/naruto-rise-of-a-ninja-nx
- https://github.com/nx-mod/sdl3-nx
- https://github.com/thelinkin3000/halo-ce-universal
