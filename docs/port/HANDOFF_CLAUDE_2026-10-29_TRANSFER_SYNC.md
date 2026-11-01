# Handoff: dkgl copy-engine ordering fix (2026-10-29, 03:58 build)

Supplements `HANDOFF_CLAUDE_2026-10-29_GRAPHICS.md` and `HANDOFF_2026-10-29.md`; read those first.
Nothing is committed. The working tree holds all changes. The pre-change dkgl sources are backed up in
`build/claude-scratch/backup-pre-transfer-sync-*`.

## Status in one paragraph

The garbled surfaces have a well-supported cause that the earlier geometry work missed. It is an
ordering bug between deko3d's **copy engine** and its 3D/2D engines; the vertex data is not at fault.
The fix is built: NRO `2812f59c…`, exit code 0, and the copied NRO and ELF match the build outputs.
**It has not been tested on hardware yet.** Nothing here is confirmed visually.

## Evidence chain

1. **Frames.** In `clip-review-*` and `upstream-graphics-result/clip-b-15s.png`, the broken things
   mostly have the *right shape in the right place* but the *wrong picture*:
   - ground quads showing a white-to-blue sky gradient;
   - solid black slabs and panels;
   - the ship body in space, which is dark against a dark background.
   World geometry and Ratchet up close are fine. That points at texture contents, not vertices.
2. **deko3d 0.5 source** (`source/dk_image.cpp`, `gpu_transfer.cpp`, `gpu_base.cpp`):
   - `dkCmdBufCopyImage` always uses `BlitCopyEngine`.
   - `dkCmdBufCopyBufferToImage` uses it for plain copies.
   - `dkCmdBufBlitImage` uses the 2D engine.
   - `DkBarrier_Primitives` is only `3D WaitForIdle`. The header says it "completes ... primitives
     and compute jobs".
   - Only `DkBarrier_Full` (Gpfifo `SetReference` plus an entry split) "completes the processing of
     all previous commands".
   - A queue flush adds cache invalidation but no wait.
3. **Reference practice.** night100 (`build/deko3d-debug-reference/night-backend-current.cpp`
   lines ~655–690, 839–858) uses `DkBarrier_Full` after every `copyBufferToImage` and every buffer
   copy.
4. **What dkgl did:**
   - **`generate_mipmap`** copied level 0 on the copy engine, then used a Primitives barrier and 2D
     blits for levels 1+. The 2D engine could read level 0 before the copy wrote it.
     `TexturePool::create_gl` (`renderer/texture_pool.cpp:523`) mipmaps **every 3D texture**. So the
     smaller levels held whatever the image-heap memory held before (another texture, or zeros).
     That means correct up close and wrong when minified, persistently.
   - **`upload`** (the palette every frame, light textures, every `TexSubImage`) waited before the
     copy only with Primitives, and only if sampled since the last barrier of any kind. After the
     copy it used Primitives. Neither orders the copy engine. That leaves read/write races on the
     joint palette, in both directions, including frame N-1's draws still on the GPU.
   - **`blit_framebuffer` and `copy_tex_sub_image_2d`** (same size and format, so the copy engine)
     waited only with Fragments before reading a render target. The affected paths are
     `FrameBuffer::blit_to`, the direct renderer's `frame_source` copy (`renderer/direct.cpp:1592`)
     and the vendor/menu preview targets.
5. **Why 17/17 selftests passed.** Every check does `glReadPixels` (a full GPU wait), and the
   textures are 64 B to 32 KB. Those copies finish before the next GPU work starts, so the race
   cannot show.

## The change

- **`dkgl_internal.h`**
  - `Context::full_epoch` counts Full barriers and idle waits.
  - `Context::pending_transfer`.
  - `Image::touched_full_epoch`, set when 3D/2D work samples, renders into or blits the image.
  - `touch()`, `before_transfer()`, `after_transfer()`, `transfer_fence()` and
    `legacy_transfer_sync()`.
- **`dkgl_core.cpp`**
  - `barrier()` with `Full` advances `full_epoch` and clears `pending_transfer`.
  - `flush_and_wait` does the same.
  - The helpers are defined here.
  - `present` fences before its 2D blit.
  - The dummy upload is marked as a transfer.
  - A start-up log line reports which mode is active.
  - The counters line gains `full` barriers and `engine copies`.
- **`dkgl_gl.cpp`**
  - **Draws and clears** call `transfer_fence()` first, then touch their targets.
  - **Sampling** touches the image.
  - **`upload`** calls `before_transfer` (a Full barrier if 3D/2D touched the image since the last
    Full barrier) and `after_transfer`.
  - **`generate_mipmap`** puts a Full barrier between the copy-engine level 0 and the first 2D blit.
    Later levels keep the 3D wait, which is GR to GR.
  - **`copy_tex_sub_image_2d` and `blit_framebuffer`** use Full ordering on their copy-engine paths;
    the 2D paths are unchanged apart from the fence.
- **A/B switch.** `OPENRAC_DK_TRANSFER_SYNC=legacy` in env.txt restores 3D-only ordering.
- **`OPENRAC_DK_MIP_AUDIT=N`** (default 6, 0 turns it off). For the first N mip chains the game
  makes, dkgl reads level 1 back from the GPU and compares 16 texels with level 0's 2×2 averages,
  computed on the CPU from the uploaded pixels. It logs `dkgl mip audit PASS` or `FAIL`. This checks
  the game's real textures. It assumes the 2D engine's linear 2:1 blit equals the 2×2 average to
  within ±3. If default mode FAILs but the selftests PASS, check that assumption first.
- **`scene/dkgl_selftest.cpp`**: three new 4 MiB checks:
  - a draw right after an upload must see the new texels;
  - a queued draw keeps the old texels while a later draw sees the replacement;
  - the smallest level of a generated 1024×1024 mip chain must be red.
  The clear colour is blue, so undrawn, stale or zero texels and the correct result all differ.
- **The vertex-binding reservation workaround is unchanged.**

## Local verification done (not hardware)

- `g++ -std=c++23 -Wall -Wextra -fsyntax-only` against the deko3d 0.5 headers: clean.
- `build/deko3d-debug-reference/transfer-sync-mocktest.cpp` links the real dkgl sources against a
  recording mock deko3d (build line in its header comment; it needs a stub `switch.h`):
  - default mode passes all 6 ordering checks;
  - `OPENRAC_DK_TRANSFER_SYNC=legacy` fails 5 of 6;
  - it is ASan/UBSan clean.
- The devkitA64 build recompiled `dkgl_core.o`, `dkgl_gl.o`, `dkgl_selftest.o` and
  `platform/switch/graphics.o`, then relinked. The ELF contains the new log strings.

## Build identity

- `build/deko3d-test/openrac-rac1-pal-deko3d.nro`: 29092305 bytes, SHA-256 `2812f59cb018b7a6039f4efae1be3d4ab27e8a39ecbbaf31dafddbdafb7369a7`.
- `build/deko3d-test/last-build.elf`: SHA-256 `f3f89520341e67d9e745c0bb8738365bede0ab36437219808ff854c01208a7dd`.
- Both are identical to `port/build-switch-deko3d-game/`. `last-build.json` was updated.
- The previous tested build (`f82786…`) and its ELF are in `diagnostics/2026-10-29-pre-transfer-sync/`.

## Hardware test plan

1. **Normal run.** Use the current env.txt (`OPENRAC_DK_COUNTERS=120`, `OPENRAC_FRAME_COST=120`).
   Play the title, the landing cutscene, Novalis ground and the vendor. Send the session log,
   `dkgl-driver.log` (only new lines matter) and clips.
2. **What to read in the log:**
   - `copy-engine transfers ordered with DkBarrier_Full`;
   - 20 selftest results, where the 3 new ones are the key evidence;
   - `dkgl mip audit PASS/FAIL` lines;
   - `full`/`engine copies` per frame, plus the FPS change.
3. **If the picture is still wrong**, do an A/B run with `OPENRAC_DK_TRANSFER_SYNC=legacy`. If the new
   selftests or audits fail only in legacy mode, the race is confirmed on hardware. If the defects
   persist identically in both modes, look elsewhere: palette contents, depth prepass EQUAL, stencil
   shadows. The `no-shadows` diagnostic has never been run.

## Still uncertain

- Whether this is the *only* cause. Skinned-pose distortion could also come from palette data. The
  palette races are fixed too, but that part is not proven.
- Whether a 3D WaitForIdle orders 2D-to-2D blits between mip levels. The original code assumed so
  (nouveau does the same). If the mip audit fails in default mode, make those barriers Full as well.
- Performance: roughly 2–6 extra Full barriers per frame (palette before/after, frame copies). Check
  the counters and FPS.

## Addendum 04:30: hardware results and the Zcull change

**Run 04:01 (transfer-sync build `2812f59c`)**
- Selftests: 21/21 pass.
- Mip audits on real game textures: 6/6 pass.
- Visible defects: unchanged.
- Conclusion: copy-engine ordering is correct now, but it was not the visible fault. The fix stays in.

**Run 04:21 (CPU-skinning build)**
- The draw trace showed the GPU palette byte-identical to the CPU's in every trace, with no joint
  lookups outside the palette.
- CPU skinning on: no visible change.
- Conclusion: skinning is **not** the fault. The bind-pose result was not discriminating.
- CPU skinning is now off by default (`OPENRAC_DK_CPU_SKINNING=1` turns it on); the trace budget is 4.

**New evidence: clip 2026102904224400 at 27.5 s**
- The ground directly under and in front of the camera is missing. Whole triangles are gone, and the
  rock walls *behind and below* them still draw, along with a white void.
- OpenGL draws this correctly, according to the user.
- So near-camera geometry is rejected while farther geometry behind it passes. That is the signature
  of a coarse depth rejection with the wrong sense: zcull, under reversed depth.

**Change**
- `dkgl_core.cpp` creates the queue with `DkQueueFlags_DisableZcull` unless `OPENRAC_DK_ZCULL=1`.
- Why this is plausible:
  - deko3d 0.5 enables zcull on every queue by default and configures it only from depth clears and
    target binds (`gpu_3d_base.cpp`: `ZcullRegion`, `ConditionalZcullInvalidate`, `ZcullClearDepth`).
  - The renderers use reversed depth (clear 0, GEQUAL) and switch depth targets within a frame
    (world, offscreen panels, shadows).
  - Mesa/nouveau, which draws correctly, has no zcull.
- Risk: none to correctness. Zcull only saves work.

**If the holes remain with zcull off**, look at these in order:
1. Near-plane clipping (`ViewVolumeClipControl` 0x181D, deko3d 0.5).
2. Depth-prepass EQUAL invariance between `mesh_depth` and `mesh` in their DKSH forms.
3. The 2D-layer clip distances (always enabled on deko3d; GL enables them only when `cut`).

## Overnight work, 2026-10-29 04:35–05:30 (nothing built or tested on hardware since)

All changes compile with `g++ -std=c++23 -Wall -Wextra` against the deko3d 0.5 headers. dkgl
changes also pass the mock harness (`build/deko3d-debug-reference/transfer-sync-mocktest.cpp`)
under ASan/UBSan. **Not yet built with devkitA64.** The container cannot reach pkg.devkitpro.org,
and the PC build needs the user.

### Defaults in the next build and their switches (env.txt)

| Change | Default | Switch | Why |
|---|---|---|---|
| Zcull | **off** | `OPENRAC_DK_ZCULL=1` | Near-camera geometry rejected while farther geometry behind it draws (clip 04224400 @27.5 s). Mesa GL has no zcull. Cannot change a correct picture. |
| Clip-distance enable | follows `glEnable(GL_CLIP_DISTANCE0/1)` | `OPENRAC_DK_CLIP_ALL=1` (old: all 8 on) | GL semantics. Method 0x544 written inline; encoding verified against deko3d's own `MakeInlineCmd` (`80030544`/`80ff0544`). |
| Draw-state cache | **on** | `OPENRAC_DK_STATE_CACHE=0` | Skips repeated shaders, UBOs, textures, vertex inputs, fixed state, viewport/scissor and index buffer. Reset at every list, GPU wait, target bind and program change. |
| CPU skinning | off | `OPENRAC_DK_CPU_SKINNING=1` | 04:21 run: no visible change and the palette matched, so skinning is not the fault. |
| PVS culling on dkgl | off | `OPENRAC_DK_PVS=1` | Performance experiment only; the code is shared with Vulkan. |
| Transfer ordering | Full barriers | `OPENRAC_DK_TRANSFER_SYNC=legacy` | Correct; keep. |

Draw-state cache, mock results:
- With the cache on, a repeated identical draw sends 0 shader, vertex, rasterizer and texture
  binds; without it, 1/1/1/6.
- A culling change is still sent (2 of 2), and so is a moved vertex pointer (1 of 1).
- dkgl's own work per draw falls from 0.47 µs to 0.33 µs on x86. The bigger saving is the deko3d
  command words that are no longer encoded.

### Ruled out (with evidence)
- **Texture uploads, mips, palette data and skinning:** hardware checks pass (selftests 21/21,
  mip audits 6/6, palette byte-identical).
- **Depth prepass EQUAL:** never used on Switch. Every sampled draw uses GEQUAL (0x206); the prepass
  only runs for GTAO or early lighting.
- **Near-plane clipping:** deko3d's `ViewVolumeClipControl` 0x181D and NVIDIA GL's 0x281C agree on
  the clip and clamp bits (bit 11, depth clamp disabled). They differ only in the 0..1 versus −1..1
  depth-range bits, which match dkgl's DepthZeroToOne. Reference: yuzu commit bb1ed66d "Fix depth
  clamping register".
- **Missing GL entry points:** the 21 functions dkgl lacks are not called on Switch paths (uniform
  vector variants, queries, instanced arrays in the uninstalled GS world renderer).
- **Formats and state calls:** no `glVertexAttrib*` constants, polygon offset, sRGB framebuffers or
  separate stencil functions are used. All texture and renderbuffer formats requested are mapped.

### Performance
- **`flushes 1.00` per frame** is `OPENRAC_FRAME_COST` itself: it calls glFinish each frame
  (`scene_frame.cpp:370`). Remove it from env.txt when judging FPS.
- **`claude-build-deko3d-release.sh/.bat`** (new, not run) is a separate build
  (`port/build-switch-deko3d-release` → `build/deko3d-release`) with `-DOPENRAC_DKGL_DEBUG=OFF`, so
  it links the release libdeko3d. A first build configures from scratch.

### Upstream
OpenRAC `develop` is unchanged since 9161f19. Open PR #119 is enhanced rendering (off on Switch).
Nothing to take.

### Open
- **Cutscene audio delay (level 98):** does the OpenGL build have it too? If not, suspect dkgl's
  level-load cost (uploads, mip chains and the one-time GPU waits from audits and selftests) delaying
  the first frames.
- **Vendor screen:** the top title bar is black and the right box is dark. These come from the GS
  emulator's offscreen panels (`renderer/direct.cpp` `offscreen()`, `target_mobys`). Re-check after
  the Zcull change.

### Added last: a draw that samples its own render target
- **Before:** dkgl substituted a black dummy texel.
- **Now:** dkgl samples the real image after a `DkBarrier_Fragments` + image-invalidate barrier,
  matching what Mesa GL returns in practice. The GS emulation's offscreen panels
  (`renderer/direct.cpp`) are the likely users; this may explain the black vendor panels.
- **Logging:** each case is logged ("a draw samples its own colour target"), up to 8 times.
- **Switch:** `OPENRAC_DK_FEEDBACK=dummy` restores the old behaviour.
- **Depth:** a depth target sampled while bound still gets the dummy.
- **Mock check:** the barrier is recorded before the draw.

### Build confidence
- `dkgl_gl.cpp`, `dkgl_core.cpp`, `level_renderer.cpp` and `dkgl_selftest.cpp` compile with zero
  warnings under both g++ 13 and clang++ 18 at `-std=gnu++20 -Wall -Wextra` (the project uses C++20).
- This is not the devkitA64 compiler; the user's build is the real check.

### Next hardware test
1. Build with `build/deko3d-test/build-deko3d.bat`, then verify `=== exit code 0` and the hashes.
2. env.txt (see `build/deko3d-test/env-next-test.txt`): `OPENRAC_DK_COUNTERS=120` only. No
   FRAME_COST, so FPS is real.
3. Check the log for these lines:
   - `dkgl: Zcull off`
   - `dkgl: repeated draw state skipped`
   - any `a draw samples its own colour target`
   - the selftest summary
4. Look at the ground in front of Ratchet at the screen bottom, the landing ship in space, and the
   vendor screen.
5. If anything is newly wrong, bisect with the switches in the table above (one at a time):
   - `OPENRAC_DK_STATE_CACHE=0`
   - `OPENRAC_DK_FEEDBACK=dummy`
   - `OPENRAC_DK_CLIP_ALL=1`
   - `OPENRAC_DK_ZCULL=1`

### Added 05:00: same-size colour copies on the 2D engine
- `glBlitFramebuffer` (colour) and `glCopyTexSubImage2D` now use `dkCmdBufBlitImage` (2D engine,
  a 1:1 nearest blit for a same-size copy), ordered by the existing fragment barrier.
- This removes the copy engine and its two Full barriers from the frontend's per-frame world copy
  (`retain_background` → `last_world`) and from the GS emulation's frame reads.
- `OPENRAC_DK_CE_COPY=1` restores the copy-engine path. Depth copies stay on the copy engine.
- The mock checks pass in both modes.
- See also `docs/port/OTHER_RAC1_PORTS.md`: the other RaC1 projects and what is worth borrowing.

## Addendum 06:00: shader compiler scheduling, texture-cache invalidation, sampler rebinds (not built yet)

Found by comparing with other Switch ports that moved from OpenGL or Vulkan to deko3d. See
`docs/port/SWITCH_DEKO3D_PORTS_NOTES.md`. Not yet built with devkitA64 or tested on hardware.

### 1. The shader pack was compiled by a uam with known console miscompiles (strongest lead)
- **Old pack:** built with devkitPro's stock `uam.exe` (provenance sha `15f29d48…`).
- **Reproduction:** stock uam 1.1.0 (`5a5afc2`), rebuilt on Linux, reproduces 17 of 18 stages bit for
  bit. The 18th (`862166b696cc81f8.frag`) differs by one stall count. That is uam's uninitialised
  scheduling word (SwitchWakerHD patch 6), so the shipped pack really has the bug.
- **Rebuilt pack:** the same GLSL (sha checked against provenance), compiled with stock uam plus three
  fixes. The fixes are SwitchWakerHD patches 6, 7 and 8, also in sms-switch:
  - initialise `sched`;
  - never dual-issue;
  - stall 2 before a new block.
  The 37-line patch is `port/tools/deko3d/uam-sched-fix.patch`.
- **What changed:** 17 of 18 stages, 353 of 3288 instructions. **Only scheduling control words; every
  instruction word is identical.**
  - 301 of the changed instructions were dual-issued (stall 0).
  - Each 720-instruction moby vertex shader (`cfccf343…`, `fb03866c…`, skinning plus lighting) had
    78 dual-issued instructions.
  - The GS-emulation fragment shaders (`862166…`, `c91fbb…`) had 31 each.
  - Other ports saw exactly these symptoms: wrong lighting, black channels, wrong values in long
    shaders, while small tests and emulators look right.
- **Files:**
  - `port/renderer/dkgl/pack/`: new DKSH files; `.prog` unchanged; `provenance.json` rewritten with old
    and new hashes.
  - `port/renderer/dkgl/pack-stock-uam/`: the old pack, verbatim.
  - `port/cmake/Games.cmake`: copies `pack-stock-uam/` to `romfs:/dkgl-stock`.
  - `dkgl_core.cpp` `pack_dir()`: `OPENRAC_DK_PACK=stock` loads the old pack. The log says
    `dkgl: shader pack romfs:/dkgl (uam scheduling fixed…)`.
- **Backup:** `build/claude-scratch/backup-pre-uam-sched-2026-10-29/` (pack, `dkgl_core.cpp`,
  `Games.cmake`).
- **To rebuild the pack later:** build a uam with the patch (devkitPro/uam `5a5afc2` plus
  `uam-sched-fix.patch`; meson, bison, flex, mako). Then run
  `port/tools/deko3d/dk_rebuild.py <pack> --out <dir> --spirv-cross … --uam <patched uam>`.
  The stock devkitPro `uam.exe` must not be used for the pack again.

### 2. deko3d 0.5's waiting texture-cache invalidation does nothing
- **The bug:** `dkCmdBufBarrier(DkBarrier_Fragments or _Tiles, DkInvalidateFlags_Image)` writes 3D
  method 0x3DD. deko3d marks it "theoretical"; the GPU has no such method (NVIDIA's
  INVALIDATE_TEXTURE_DATA_CACHE is 0x4CE). Found by sms-switch (commit 1b0b1f0): on the console,
  targets drawn and then sampled came out stale.
- **Where dkgl used it:**
  - every render-target change;
  - every `glBlitFramebuffer` and `glCopyTexSubImage2D`;
  - feedback draws;
  - `present`.
- **Fix:** new `cmd_barrier()` (dkgl_core.cpp) records those as `DkBarrier_Primitives` (wait for idle) +
  Image. deko3d then writes the no-wait invalidation, 0x4A2, which exists. This is what Mesa's
  texture barrier does (wait for idle, then invalidate).
- **Switch:** `OPENRAC_DK_TEXINVAL=deko3d` restores the old sequence.
- **Counter:** "texture cache" in the counters line.

### 3. The sampler set is bound again before a fresh sampler
- **Finding:** sms-switch (f73ec5b) saw a sampler descriptor first read partway through a list come
  out stale on the console. Invalidating the sampler cache did not help; binding the set again did.
- **Fix:** dkgl now does the same.
  - It tracks the samplers read since the last bind (reset at each list start).
  - It binds the sampler set again before a draw uses one not yet read.
- **Switch:** `OPENRAC_DK_SAMPLER_REBIND=0` turns it off.
- **Counter:** "sampler set rebinds".

### Local checks
- `dkgl_core.cpp` and `dkgl_gl.cpp` compile with no warnings under g++ 13 and clang++ 18 at
  `-std=gnu++20 -Wall -Wextra`.
- The mock harness (`build/deko3d-debug-reference/transfer-sync-mocktest.cpp`, ASan/UBSan) passes 9/9:
  - Fragments+Image is never recorded;
  - the feedback draw gets wait-for-idle + invalidate;
  - a fresh sampler causes one rebind, and a repeat causes none.
- With the switches set, the expected checks fail, so the switches are wired.

### Next hardware test (replaces the one above)
1. Build with `build\deko3d-test\build-deko3d.bat`. The pack's bytes changed, so CMake reconfigures and
   copies it. Check `=== exit code 0` and the hashes.
2. Use the env.txt in `build/deko3d-test/env-next-test.txt`.
3. In the log, look for:
   - `dkgl: shader pack romfs:/dkgl (uam scheduling fixed`
   - `dkgl: texture cache invalidated as Mesa does`
   - `dkgl: sampler set bound again`
   - the counters line (texture cache and sampler rebinds per frame)
4. Look at the same places: the ground at the screen bottom, the landing ship, the vendor screen,
   Ratchet and enemies (moby shaders), and the HUD.
5. **A/B on the console, one at a time:**
   - `OPENRAC_DK_PACK=stock`: the old shaders. If the picture gets worse with this, the uam fix is real.
   - `OPENRAC_DK_TEXINVAL=deko3d`
   - `OPENRAC_DK_SAMPLER_REBIND=0`
   - then the earlier ones: `OPENRAC_DK_STATE_CACHE=0`, `OPENRAC_DK_FEEDBACK=dummy`,
     `OPENRAC_DK_CLIP_ALL=1`, `OPENRAC_DK_ZCULL=1`.
6. **FPS:** each target change now waits for idle. That is what Mesa does, but if FPS drops, compare
   with `OPENRAC_DK_TEXINVAL=deko3d`.

### Backups of everything this addendum changed
- `build/claude-scratch/backup-pre-uam-sched-2026-10-29/`
- `build/claude-scratch/backup-pre-texinval-2026-10-29/`: `dkgl_core.cpp`, `dkgl_gl.cpp`,
  `dkgl_internal.h`, `dkgl.h`, the mock.
