# Profiling, transition flashes and sampler batching

User supplied session26, driver13, startup15 and clips
2026102910322700 / 2026102910325400 (Temp paths, suffix
899BF99A6A4D5CD93E86FC3EDF92F308). Profiling was enabled at 720p.
No error/warning lines were found; GPU checks remain21/21 passing.

## Measurements

51 cost windows total. Approximate gameplay selection: tick>=2000 and last-frame
draw count>=400, giving33 windows. This is not an isolated route benchmark.

| Component | Mean ms | Range ms |
|---|---:|---:|
| Scene preparation | 0.30 | 0.170–0.421 |
| Render command submission/recording | 9.75 | 6.194–14.455 |
| GPU completion wait | 12.69 | 8.724–19.335 |
| Present/show | 0.29 | 0.282–0.296 |
| Blocking diagnostic total | 23.02 | 15.825–34.326 |

These are FRAME_COST's existing synchronous intervals. Scene preparation excludes
the game update. GPU wait is not a timestamp-delimited pure GPU render duration.
Do not infer normal FPS by taking1/23.02ms: the diagnostic forces completion and
changes CPU/GPU overlap. Raw parsed windows and selection are archived in
diagnostics/2026-10-29-before-flash-sampler-fix/profile-summary.json.

## Candidate transition fix

Extracted frame sequences in build/deko3d-test/flash-review/ show solid purple
at title/flight transitions and cyan at the landing transition. They persist for
multiple recorded frames and then the correctly rendered world returns.
Frontend set_camera selected the level's background when it had a valid camera,
even when scene.draws submitted no world buckets. A camera can outlive the world
during these transitions, exposing the level's vivid clear colour.

world_clear_colour now uses the level background only with a camera AND world
submission (bucket mask0x1F). Other frames clear black. Retained menu backgrounds
still use keep_colour and are not overwritten. First eight suppressions log
`transition tick ... no world submitted; clear black ...` for hardware correlation.
This addresses a supported candidate, not proof that every flash is resolved.
No camera cut, palette or geometry/shader behavior is altered.

## Sampler optimization

Old code refreshed the sampler set inside the uniform loop and cleared the read
set each time. A draw using samplersA/B/C could finish remembering onlyC, so the
next identical draw refreshed again forA, thenB, thenC. Busier previous samples
averaged ~1922 rebinds for~881 draws/frame.

prepare_draw now gathers all active texture handles first, invalidates newly
created descriptors, refreshes the sampler set ONCE if any sampler is fresh,
then remembers ALL samplers used by the draw. Texture binding caching remains.
The first-use sampler workaround stays enabled, and per-list resets still force
refresh. No shader binaries changed. Startup message says `on, batched per draw`.

## Verification and artifact

- Native sampler regression: different samplers,1000 identical draws, image-only
  changes, newly introduced samplers, empty draws and new-list reset pass.
- Native frontend camera/geometry/transition regression passes, including vivid
  backgrounds with/without camera/world submission.
- Real dkgl sources linked against recording mock: previous barrier/feedback and
  cache checks pass. Added multi-sampler API-level regression verifies at most
  one refresh initially and none across ten repeated draws. Zero mock failures.
  The mock is not a GPU test; no hardware speedup is yet measured.
- Deko3D game target built successfully, inner exit0. NRO/ELF copies match outputs,
  and all patched/stock shader stages match the staged RomFS. Matching ELF includes
  both new diagnostic strings.
- NRO SHA2568595c8b64b39a7c76e9ec67c6acdc308911878d4686eb19504cb7d8e1a765f2c.
- Build ID f1f92f0758fb299346ba8f083060731ef63064c7.
- Artifact build/deko3d-test/openrac-rac1-pal-deko3d.nro; last-build.json updated.

Previous working-geometry NRO/ELF and supplied logs are preserved in
diagnostics/2026-10-29-before-flash-sampler-fix/. No Git commit or push.

## Next run and open issues

Use the same COUNTERS120 / FRAME_COST120 settings briefly to compare with this
baseline. Check title-to-game/landing transitions, then a similar Novalis route.
Look for the batched startup marker, transition suppression lines and lower
sampler-rebind counts. Return logs and flash clips if they persist. Remove
FRAME_COST again afterward for normal FPS. Vendor Pyrocitor material and landing
audio delay remain unresolved; this batch makes no claim to fix them.

## Hardware result: session27, user leaving for work

User supplied four clips10470900/10474000/10482400/10485300 and session27,
driver14/startup16. No further test is currently available. Do not request an
immediate FRAME_COST-off run. Review artifacts are in profile27-review/ and
diagnostics/2026-10-29-flash-sampler-result/ under build/deko3d-test.

- All21 GPU checks pass; no warning/error entries. Driver14 contains driver13
  unchanged plus one queue creation, with no new GPU error.
- Batched sampler startup marker is present. Eight bounded transition entries
  confirm black substituted for the old purple background on no-world ticks.
- Reviewed contact sheets across all four clips show intact geometry. A30fps
  downscaled full-clip scan found no nearly uniform coloured frames (spatial
  channel std<3, mean colour spread>20 and max channel>30), unlike previous solid
  purple/cyan flashes. This does not establish that every possible flicker is gone.
- Busy counter windows: prior~902 draws/1920rebinds, new~896draws/356rebinds.
  Normalized2.128 ->0.397rebinds/draw, about81% fewer. This directly confirms the
  targeted optimization, without claiming81% less CPU time.
- Same approximate cost-window filter (tick>=2000,last-frame draws>=400): prior
  33windows, new26windows. Total mean23.017 ->19.027ms; scene0.298 ->0.321ms;
  command record9.745 ->9.496ms; GPU wait12.686 ->8.912ms; show0.288 ->0.298ms.
  Routes/workloads differ and GPU wait is synchronous, so this is a promising
  comparison, not a controlled17% frame-rate gain or pure GPU timing result.
- Current artifact identity remains8595c8b6... / buildID f1f92f0758fb299346ba8f083060731ef63064c7.
  Marked manifest hardware_tested with these limited observations. No new build
  or rendering-code changes were made during this result review.

Preserve this build as the next known-good reference. Pyrocitor preview material,
landing audio delay and nonblocking CPU/GPU timestamp reporting remain open.
