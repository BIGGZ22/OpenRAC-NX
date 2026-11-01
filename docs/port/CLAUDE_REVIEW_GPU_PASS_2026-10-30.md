# Claude review of the session30 GPU pass (2026-10-30): source only, NOT BUILT

This reviews Codex's `HANDOFF_CLAUDE_GPU_PASS_2026-10-30.md`.

**Nothing here was built or measured on hardware.**
- Claude did not build the game, did not regenerate shader packs, and made no commit and no push.
- The 01:05 performance NRO (`54184009…`, build ID `3a507cc6…`) and the diagnostic NRO do not
  contain Codex's GPU pass or this review.
- Every result below is a host test: the real dkgl sources compiled on Linux g++ 13.3 against a
  mock deko3d, plus unit tests.

Preserved untouched:
- geometry and shader fixes, and the patched UAM pack;
- the SDK 0.5 vertex capture/replay workaround;
- the texture-cache invalidate mapping;
- the Full copy-engine barriers;
- the sampler-set refresh workaround: still ON by default, with the same rule;
- lazy target barriers: still OFF;
- the canopy world capture, saves, language persistence and the launcher.

## Evidence used, and a gap

- `STATUS_MONITOR_HARDWARE_SESSION30_2026-10-30.md`:
  - 640x480 internal, 1280x720 output;
  - 16.5–20.2 FPS;
  - GPU 77.8–89.5 % at 307.2 MHz;
  - the busiest CPU core near 40 %.
  - These are snapshots, not run averages.
- `DKGL_FLASH_AND_SAMPLER_2026-10-29.md`: ~896 draws/frame with ~356 sampler-set rebinds/frame,
  about 0.4 a draw.
- Codex's handoff: up to ~2450 draws/frame, and hundreds of sampler refreshes in session30.
- **The session30 game log itself is not in the repository.** A request for read access to
  Downloads was refused, so the session30 counters were not re-read. The numbers above come from
  the docs.

## 1. Empty glow-group skip (`scene/moby_passes.h`, `level_renderer.h/.cpp`): correct, unchanged

**Shader semantics (`viewer/shaders/mesh.vert`, `mesh.frag`):**
- `v_glow` starts at 0 (`mesh.vert:112`).
- Its only assignment to 1 is at line 149, inside `abs(moby_light.w) > 0.5`, under
  `light_slot < -1.5 && moby_ambient.w > 0.5`.
- The metal pass resets it to 0.
- In the glow pass (`glow_pass = 1`, `effect_pass = 0`), `mesh.frag:253`
  (`if (effect_pass == 0 && v_glow != glow_pass) discard;`) discards every fragment whose provoking
  vertex has `v_glow == 0`. The discard happens before any output write.
- So a group with no glow-capable instance writes no colour, depth or stencil in either glow step.

**Shader variant:**
- The discard sits inside `#ifndef OPENRAC_SOLID`. A solid variant would *not* discard.
- The selection code in `draw_layer` uses the full `mesh.frag` program (`m_mesh`) for moby pass 1:
  - `solid` is false for mobys;
  - early lighting is only for pass 0;
  - the batch and specialised shaders are only for opaque phases;
  - the depth prepass breaks before moby pass 1.
- So the glow steps always run the discarding shader.

**The gate:** `moby_instance_may_glow` matches the shader gates exactly. NaN or infinity returns
true; the shader would treat NaN as non-glow, so the helper is conservative.

**Mixed, pooled and live groups:**
- `fill_group` recomputes `may_glow` from every record uploaded:
  - initial `build_groups`;
  - pooled groups taken from `idle`;
  - live updates (`fill_group(g, g.cpu_instances, …)`);
  - the shadow compact group.
- Groups are true until their first fill.
- The PVS and camera compact groups are filled from subsets. The skip reads the parent group's flag,
  a superset, so it can only keep extra draws, never drop one.
- No other code writes the instance buffers (`glBufferData` at line 2784 is the PVS index cache).

**UI instances:** `moby_light.w = 2` passes `abs > 0.5`, so it is covered.

**What it saves:** the vertex and rasterisation work of glow-packet primitives on models whose
instances have no glow mode. Their fragments were already discarded early.
`OPENRAC_SCENE_PASS_COUNTS` reports how many groups were eligible, not measured time.

**Verdict: correct; no change.** An integration fixture of a real scene was not practical on the
host; the reasoning above plus `moby_passes_test` stand in for it.

## 2. Texture-binding runs (`dkgl_gl.cpp prepare_draw`): correct, unchanged

**The deko3d 0.5 call (`cmd_bind_common.cpp:239`):** one graphics-stage `dkCmdBufBindTextures`
writes `SelectDriverConstbuf(textures[firstId])`, a `PipeNop` and one `LoadConstbufData` of
`numHandles` words. A run of n slots therefore costs 3+n words instead of 4n per stage. This is
command-stream and CPU savings. It does not reduce GPU sampling work or sampler refreshes.

**Checks:**
- **Binding range:** `sampler_uniforms` keeps only bindings in 0..31, so the `dirty_bindings[b]`
  index cannot overflow.
- **Runs:** a run consists only of dirty slots, and each has its handle in `draw_handles`. Gaps are
  never written.
- **Duplicate bindings:** the last handle wins, as before.
- **Stages and order:** both stages are bound. Vertex runs and fragment runs are separate commands
  with the same handles; per-slot ordering between stages is irrelevant because they are different
  constbufs.
- **Program switches:** texture bindings live in the driver constbuf, per stage, independent of the
  bound shader. The `sent.textures` cache is keyed by binding, not by program, as before.
- **Resets:**
  - `reset_bound_state` (list break, `flush_and_wait`) clears `sent`, so every slot is re-sent.
  - With the state cache off, every draw binding is re-sent, now in runs.
- **Descriptor visibility and the sampler refresh:**
  - the refresh decision and the `cmd()` descriptor invalidation are unchanged;
  - the bind happens after the sampler-set rebind, as before;
  - the `c = cmd()` after the binds is unchanged.

**Verdict: correct; no change.** The mock checks the exact handles for slots 5 and 6 plus slot 8,
that gap 7 is not written, both stages, 4 calls instead of 6, and a warm repeat with no calls.

## 3. Change made: opt-in sampler-set refresh experiments (default unchanged)

### Finding

The default rule (`refresh_draw_samplers`, sms-switch f73ec5b) refreshes when any sampler in the
draw has not been read since the last bind. It then **forgets every other sampler**. Two draws that
alternate between sampler sets therefore refresh on *every* switch. The new mock check shows it:
10 rebinds over 10 alternating draws. That pattern explains the remaining ~0.4 rebinds a draw.

### Cost of a refresh

- Each refresh is `SetTexSamplerPool` on the 3D and Compute engines (`gpu_base.cpp:131`). That
  rewrites the pool register and refetches sampler descriptors. Mesa sets this once at context
  creation.
- Its GPU cost on Tegra X1 has **not been measured**. Treat it as a candidate, not a proven
  bottleneck.

### Change

`OPENRAC_DK_SAMPLER_REBIND` now also accepts two experimental values:

- **unset / `1` (default):** unchanged, the sms-switch rule. Its startup line still says
  "on, batched per draw".
- **`0`:** unchanged, off.
- **`first` (experiment):** refresh before a sampler's *first read in each command list*. The read
  set clears at `begin_list` and is not cleared on a refresh. This is the symptom exactly as the
  port notes record it: "first read partway through a command list comes out stale".
- **`new` (experiment):** refresh only when a draw reads a sampler descriptor written *after* the
  set was last bound. Sampler descriptors are append-only and never rewritten; `samplers_at_bind`
  is recorded at `begin_list` and at each refresh. This is the narrowest rule.
- Any unrecognised value keeps the default.

A newly written sampler still refreshes once in every mode except `0` (checked in the mock).

### Files

- `port/renderer/dkgl/sampler_refresh.h`: `SamplerRebind`, `parse_sampler_rebind`,
  `refresh_first_samplers`, `refresh_new_samplers`.
- `port/renderer/dkgl/dkgl_internal.h`: `Context::samplers_at_bind`.
- `port/renderer/dkgl/dkgl_core.cpp`: `begin_list` records `samplers_at_bind`.
- `port/renderer/dkgl/dkgl_gl.cpp`: mode parsing with its startup line, and the mode dispatch.
  The refresh updates `samplers_at_bind`.
- `port/tests/dkgl_sampler_refresh_test.cpp`: covers default ping-pong (10 of 10), `first` (2
  refreshes, then warm, list reset), `new` (index threshold) and parsing.
- `build/deko3d-debug-reference/transfer-sync-mocktest.cpp`: real-dkgl alternating-sampler check
  per mode, and a newly written sampler refreshing once.

### Risk

`first` and `new` deliberately apply *less* of a correctness workaround. If either is wrong,
expect the sms-switch symptom: a surface drawn with the wrong sampler state (for example the wrong
wrap or filter, or a wrong-colour ground) until the next submission. **Do not make either the
default without a console run showing no such surfaces.**

## 4. Other GPU findings (not changed)

- **Command span is not GPU busy time.** Attributing GPU time to passes (sky, ties, terrain,
  mobys, effects, 2D, upscale) needs GPU timestamps around passes. dkgl has
  `dkCmdBufReportCounter` timing for the whole slot only. `stats::Scope(..., stats::kGpu)` scopes
  exist in the renderer but are not wired to GPU timestamps in dkgl.
  **Recommended next implementation:** a bounded per-scope timestamp pair, read back after the
  slot fence. This is the most useful next measurement.
- **Wait-for-idle count per frame.** Each target switch (Fragments + Image becomes Primitives +
  Image), image barrier, feedback draw and Full copy-engine barrier is a full drain. Counters show
  ~6–20 barriers/frame, including ~2–7 Full ones. `OPENRAC_DK_LAZY_TARGET_BARRIERS=1` is the
  existing opt-in reduction.
- **Per-frame engine copies (~1–5.5/frame) force Full barriers.**
  - If they are per-frame framebuffer copies, routing them through the 3D engine instead of the
    copy engine would avoid the Full barrier entirely, rather than weakening it.
  - First identify the call sites. A counter by source would do this.
- **Draw count (~1800–2450/frame).** Reducing draws (merging moby groups, instancing ties) would
  cut front-end and vertex cost more than any binding change. That is larger work.

## 5. Tests run (host only)

Real `dkgl_gl.cpp` + `dkgl_core.cpp` against the mock deko3d (`transfer-sync-mocktest.cpp`):

| Environment | PASS | FAIL |
|---|---|---|
| default | 27 | 0 |
| `OPENRAC_DK_TEXTURE_BIND_RUNS=0` | 27 | 0 |
| `OPENRAC_DK_SAMPLER_CACHE=0` | 22 | 0 |
| `OPENRAC_DK_LAZY_TARGET_BARRIERS=1` | 32 | 0 |
| `OPENRAC_DK_TIMING=120` | 28 | 0 |
| `OPENRAC_DK_VERTEX_CACHE=0` | 20 | 0 |
| `OPENRAC_DK_PANEL_TRACE=1` | 28 | 0 |
| `OPENRAC_DK_SAMPLER_REBIND=first` + `TEXTURE_BIND_RUNS=0` | 27 | 0 |
| `OPENRAC_DK_SAMPLER_REBIND=new` + `LAZY_TARGET_BARRIERS=1` | 32 | 0 |
| `OPENRAC_DK_SAMPLER_REBIND=0` | – | 1 expected: the test asserts the workaround |
| `OPENRAC_DK_TRANSFER_SYNC=legacy` | – | 3 expected: legacy omits Full barriers by design, as before |

Unit tests:
- `dkgl_sampler_refresh_test` (extended): PASS.
- `moby_passes_test`: PASS.
- `dkgl_draw_state`, `dkgl_geometry_validation`, `dkgl_transfer_validation`,
  `dkgl_vertex_binding`, `dkgl_diagnostics`: PASS. `dkgl_diagnostics` read shader sources from an
  older tree snapshot.

**Limitations:**
- The mock records commands; it does not run a GPU.
- There is no pixel or timing verification.
- `level_renderer.cpp` was not recompiled by Claude (unchanged; Codex syntax-checked it).
- No full Switch compile or link was done.

## 6. Files changed by Claude

Back-ups of the previous versions are in
`build/claude-scratch/backup-pre-claude-gpu-review-2026-10-30/`.

- `port/renderer/dkgl/sampler_refresh.h`
- `port/renderer/dkgl/dkgl_internal.h`
- `port/renderer/dkgl/dkgl_core.cpp` (one line)
- `port/renderer/dkgl/dkgl_gl.cpp` (rebind mode parse and dispatch only)
- `port/tests/dkgl_sampler_refresh_test.cpp`
- `build/deko3d-debug-reference/transfer-sync-mocktest.cpp`

Not changed: `scene/moby_passes.h`, `scene/level_renderer.*`, `dkgl.h`, and the shaders and packs.

## 7. Unresolved

- The pixel equivalence of the glow skip on hardware is untested. The reasoning in section 1 is
  strong, but it has not been seen on screen.
- The GPU cost of sampler-pool refreshes is unknown. The `first`/`new` experiments may show no FPS
  change.
- The session30 game log was not available to Claude.
- Carried over: audio delay, the Pyrocitor preview material, and canopy attachment (the log only
  confirmed activation).

## 8. Hardware plan after Codex builds (one switch at a time)

Conditions for every run: stock clocks, explicit 640x480, the same FPS cap and route (Metropolis
rotation, then ship flight and landing). Use `env-passive-timing.txt` plus `OPENRAC_GAME_TIMING=120`,
`OPENRAC_DK_COUNTERS=120` and `OPENRAC_SCENE_PASS_COUNTS=120`. Leave `FRAME_COST` out and lazy
barriers off.

1. **Baseline:** the new build with all defaults (glow skip ON, bind runs ON, sampler rebind
   default).
2. `OPENRAC_MOBY_EMPTY_GLOW_SKIP=0`: compare FPS, `dk gpu` span and the glow-step draw counts.
3. `OPENRAC_DK_TEXTURE_BIND_RUNS=0`: expect at most a CPU-side difference.
4. `OPENRAC_DK_SAMPLER_REBIND=first`: watch `sampler rebinds` drop. **Inspect for wrong-colour or
   wrong-wrap surfaces**, especially the first time an area is entered and after level loads.
5. Only if step 4 looks clean: `OPENRAC_DK_SAMPLER_REBIND=new`.

Report the Status Monitor FPS and GPU % photos, plus the game log, for each run.
