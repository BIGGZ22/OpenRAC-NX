# Ratchet and Clank PAL native PC campaign

Complete the decompilation of Ratchet & Clank (2002), PAL SCES_509.16, and
make the full game playable natively on Windows. Implement, validate and
commit each logical fix locally, then continue with the next task during
the active session. The owner authorized these commits on 2026-10-10.
Pushing and publishing need a specific request.

## Repository and startup map

The configured parent workspace contains two independent Git repositories:

| Work | Repository and location |
|---|---|
| Retail matching C, declarations and match reports | `rac1-decomp/` |
| Native runtime, renderer, hostgen and platform fixes | `OpenRAC-native/port/` |
| Native implementations awaiting matching C | `OpenRAC-native/port/game/rac1-pal/hand/`, registered in `hand.tsv` |
| Shared campaign handoff | This file |
| Machine setup and launchers | Parent `SETUP.txt`, `Enter-Decomp.ps1`, `Build-Native.ps1`, `Run-Native.ps1` |

`OPENRAC_RAC1_PAL_SOURCE` in `port/build/release/CMakeCache.txt` points to the
sibling `rac1-decomp/`. Check that binding at startup. Editing the imported
`games/rac1/pal/` does not change this local build. On a different machine,
use the actual configured source path and its instructions.

Before editing, read both repositories' instructions, inspect their Git
status/staged diffs and recent commits, then read the latest hostgen report
and relevant run log. Check for concurrent work before rebuilding shared
outputs or choosing a function someone is already implementing. Preserve
other work. Use a topic branch if starting from `main`.

## Task selection and acceptance

1. Reproduce the earliest blocker on the normal boot, New Game and first
   level path. Diagnose the missing function, bad translation, invalid
   state or rendering fault from current evidence. Do not assume an old
   log still identifies a missing function: check source and registrations.
2. Prefer matching C in `rac1-decomp/` when recovering game logic. A native
   implementation can unblock play before a match, but must be based on
   retail behavior, tested and explicitly labeled nonmatching. Keep it in
   the native tree. Do not alter PS2 matching code just for host ABI needs.
3. Fix missing callees and state transitions in dependency order. A stub,
   forced success, skipped dialog or disabled stop-on-missing is not a fix.
   Test the behavior and failure/edge cases relevant to the change.
4. Once the current path runs, cover movement, camera, collision, combat,
   gadgets, enemy behavior, death/restart, saves and planet transitions.
   Extend coverage through every level and the ending. Investigate audio,
   cutscenes and rendering defects as game behavior, not just build issues.
5. Continue the remaining executable and overlay decompilation even after
   a playable path exists. Use rank/triage tools and reusable function
   families; preserve evidence when a function is blocked and move to an
   independent task instead of repeating unproductive attempts.

Keep the missing-function trap enabled. `--keep-going` and memory-write
shortcuts must never count as gameplay validation. A frame-count exit, a
title screenshot, and a free-camera level viewer do not prove playability.

## Validation and commits

Run commands from their documented directory and stop on a failed exit
code. PowerShell does not automatically throw on native command failure.

- Matching work: use `rac1-decomp/AGENTS.md` and its workflow. Require
  strict per-function/file checks, executable layout/image checks where
  applicable, and a regenerated `progress/report.json` committed with the
  matching source. Do not weaken build-fidelity or existing test gates.
- Native work on this machine: from the workspace run
  `.\Build-Native.ps1`. It configures the external PAL source, builds the
  release preset and runs CTest with `SDL_VIDEO_DRIVER=windows`. Read and
  address failures; record pre-existing failures separately with evidence.
- Translator changes: also dot-source `Enter-Native.ps1`, then from
  `OpenRAC-native/` run `python -m unittest discover -s port/tools/hostgen`.
  Report skipped tests as skipped, not as validation on Windows.
- Reproduce the affected native scenario using `Run-Native.ps1` or the
  executable with documented inputs. Record exact arguments, input timing,
  environment and level/save state, exit code, log and screenshot paths.
  Save local artifacts under the workspace `.tools/` directory. Never
  overwrite a player's saves; use an isolated test card directory.
- Review the diff and stage only explicit owned files. One coherent fix
  per local commit, with why it changed, validation and limitations in the
  body, plus the assistant co-author trailer required by `CONTRIBUTING.md`.
  Include related tests; keep unrelated fixes in separate commits. Commit
  corresponding changes separately in each repository and cross-reference
  the prerequisite commit when a native change needs decomp changes.
- Update this handoff with the completed task, evidence, remaining blocker
  and exact next action. Use commit subjects for the current commit and
  hashes of earlier commits; do not try to embed a commit's own hash in it.

## Evidence at setup on 2026-10-10

These are observed artifacts, not a fresh full-game validation. Both
repositories already contain uncommitted implementation work; the setup
commit deliberately does not absorb it.

- Decomp baseline: `4b86e4fc`. `progress/report.json` reports 4,214 of 5,109
  functions matched/finished (82.48%) and 78.50% matched code. The report
  includes classified original assembly; these numbers are not pure C
  coverage or proof that all remaining native paths work.
- Native baseline: `6321d39`, branch `setup/windows-native`. The observed
  generated report at `port/build/release/games/rac1-pal/gen/report.md`
  lists 3,798 translated functions, 187 from candidates, 3 translation
  stubs and 1,158 functions without C. Its call frontier is a prioritization
  aid, not exhaustive dynamic coverage.
- Parent `.tools/native-gameplay-tests.log` records 39/39 tests passing.
  Treat this as prior evidence for that working tree, not a test of later
  edits. Older `SETUP.txt` counts and blocker descriptions have been
  superseded by ongoing source changes.
- `.tools/native-run/zone.log` stopped at `func_L00_002E74B0`. A local
  `level_camera_hero.c` and its `hand.tsv` registration now exist.
  `game_freeze.c` also implements the former `func_001FBE80` blocker.
  Review and reproduce before implementing either again. The observed
  `camera.log` was still advancing and does not establish completion.

## Next session handoff

### Crate initialization, 2026-10-10

Local commit subject: `feat(rac1/pal): implement native crate initialization`.
Recovered `func_L00_002D1168` from the PAL retail instructions in a native
hand implementation. It handles independent crates, saved-counter removal,
group support links, moving-platform attachment, stack state propagation,
and list transitions. No matching source or matching report was changed;
this is not an exact PS2 match. Helpers receive overlay globals from the
level entry point so hostgen retains the correct level relocation context.

Validation on the existing working tree:

- `Build-Native.ps1`: 42/42 CTest tests pass; log
  `.tools/native-crate-build-final.log` in the parent workspace.
- `crate_init_test.c` also compiles and passes with the local
  `i686-w64-mingw32-clang.exe -std=c11 -O2 -fno-strict-aliasing
  -ffp-contract=off`, checking EE structure offsets as well as synthetic
  ground, miss, saved-counter, stack, platform and list scenarios.
- Hostgen: 378 units, 3,808 translated functions (includes new helpers),
  192 candidates, 3 stubs, 1,153 without C. Both `units_unreadable` and
  `index_problems` are empty.
- Before: `.tools/native-run/crate-before.log`, exit 2 at frame 2809,
  missing `func_L00_002D1168`. After: `crate-verified.log`, exit 2 at frame
  2809, missing `func_002116A0`. The crate call is passed; first-level
  gameplay is still blocked. Screenshots use the corresponding prefix,
  including `crate-verified-2800.png` (opening movie, not gameplay).

Exact reproduction, from the parent workspace in PowerShell:

```powershell
$p = .\.tools\Invoke-NativeProbe.ps1 -Name crate-verified -Frames 5200 -Press '100:4000:5,1200:8:5,1400:4000:5,1700:4000:5,2200:8:5,2500:8:5,2800:8:5,3700:4000:5,4000:0:150:128:0'
$p.WaitForExit()
$p.ExitCode
```

The local helper launches `port/build/release/openrac-rac1-pal.exe` with
`--data <native>/build/native-data --cards <native>/build/native-test-cards
--window --levels <native>/build/native-data/port --frames 5200 --no-card`.
It sets `OPENRAC_DEBUG=1`, `OPENRAC_UNCAPPED=1`, `OPENRAC_PRESS` to the
sequence above and `OPENRAC_SHOT=200:<workspace>/.tools/native-run/<name>-`.
This is fresh New Game, skips three opening movies with Start, uses no
player save, and keeps stop-on-missing enabled. Exit codes were captured
from the process object after `WaitForExit`.

Existing uncommitted camera, grid, dialog, vector, translator and renderer
changes were present at session start and remain separate. Only this
task's new implementation, test, registration, CMake target and handoff
are staged for the crate commit. The full runtime result depends on that
working-tree baseline; this commit alone does not supply those earlier
startup fixes. `rac1-decomp/tools/organize_asm.py` remains untouched.

### Joint matrix selection, 2026-10-10

Local commit subject: `feat(rac1/pal): implement native joint matrix selection`;
previous fix is `368205a`. Implemented `func_002116A0`, PAL's handwritten
chain selector, in native C. It builds dependency marks and terminal
indices, invokes the existing native `func_00211808`, and copies full
64-byte matrices in request order. Matching code/reports remain untouched.

- `Build-Native.ps1`: 43/43 CTest tests pass;
  `.tools/native-joint-build-final.log`. The selector fixture also passes
  with the 32-bit compiler, exercising duplicate requests, dependency
  unions, terminal bounds, untouched bytes and retail's zero-count loop.
- Hostgen: 3,809 functions, 193 candidates, 3 stubs, 1,152 without C;
  378 readable units and no index problems. An initial declaration conflict
  was caught by the unreadable-unit gate, corrected to the source's
  `void *, int, int *, void *` signature, and rebuilt before runtime testing.
- Same reproduction command/environment as above, with
  `-Name joints-verified`: exit 2, frame 2809, now at `func_00218928`.
  Artifacts: `.tools/native-run/joints-verified.log` and
  `joints-verified-2800.png`. This verifies passing the selector call, not
  complete pose fidelity or playable Veldin. Existing evaluator limitations
  (for example post-scale chain behavior) remain outside this selector fix.

### Particle allocation, 2026-10-10

Local commit subject: `feat(rac1/pal): implement native particle allocation`;
joint selection was committed as `ba91985`. Implemented the two public
handwritten entries `func_00218928` and `func_00218930` using the PAL body
through `00218A74`. Preserve allocation order, bitmap/high-water/count
updates, NULL on exhaustion and the original clearing of only 32 bytes of
each 64-byte record. The reverse reuse path and secondary bitmap stores
are unreachable behind unconditional retail jumps; no new fallback was
introduced. These functions remain classified assembly in the decomp.

- `Build-Native.ps1`: 44/44 CTest tests pass;
  `.tools/native-particle-build.log`. The fixture fills all 2,048 slots,
  checks sparse holes and byte-aligned rescan order, exhaustion, untouched
  secondary bitmap and preserved payload bytes.
- Hostgen: 3,811 functions (including helpers), 195 candidates, 3 stubs,
  1,150 without C; 378 readable units, no index problems.
- Same New Game probe with `-Name particles-verified`: exit 2, frame 2828,
  now at `func_L00_00217AE8`. Runtime logs entry into the native level-0
  draw path with 199 models, but this does not prove visible gameplay.
- A second probe with `-Name particles-level -ShotEvery 2820` (all other
  arguments unchanged) also exits 2 at 2828. Its frame-2820 PNG was viewed
  and is black, consistent with being early in the transition; no visible
  Veldin scene or player control has yet been demonstrated.

The next stop identified a bad native candidate for `func_L00_00267290`.
`rac1-decomp/nonmatching/game/func_L00_00267290.c` declares and twice calls
the nonexistent `func_L00_00217AE8`. Retail calls at `002673D4` and
`002674E8` name the existing `func_00217AE8` (its Veldin copy is at
`002677B8`, as `config/overlays/functions.tsv` records).

### Stream call binding and current next action, 2026-10-10

Local commit subject: `fix(rac1/pal): bind stream requests to the implemented callee`;
particle allocation was committed as `2436458`. Added a native override
of the existing `func_L00_00267290` candidate, correcting only the callee
declaration and its two call sites to `func_00217AE8`. A direct comparison
against the sibling candidate confirms the remaining logic is unchanged.
No fallback or fabricated implementation was added for the bad symbol.
This remains nonmatching native code; the candidate in `rac1-decomp/`
is preserved for a later matching review.

- `Build-Native.ps1`: 44/44 CTest tests pass;
  `.tools/native-stream-build.log`. All 378 source units are readable and
  `index_problems` is empty. Generated stream calls now name the real C
  callee at both sites.
- Hostgen remains at 3,811 translated functions and 195 candidates, with
  3 stubs. The no-C count is 1,149 and address count 48,344 because the
  nonexistent symbol was removed, not because another function was
  decompiled.
- Same New Game input sequence, `-Name stream-verified -ShotEvery 3000
  -Frames 5200`: exit 2 at frame 2828, stopping at
  `func_L00_00232EF0`. `.tools/native-run/stream-verified.log` is the latest
  runtime evidence. No frame-3000 screenshot was produced because execution
  stopped earlier. The last inspected level-transition capture remains
  the black `particles-level-2820.png`; visible gameplay is unverified.

**Exact next action:** review
`rac1-decomp/nonmatching/shared/func_L00_00232EF0.c` against
`rac1-decomp/asm/overlays/func_L00_00232EF0.s` (1,312 bytes), then implement
or register a verified native version of Ratchet's animation advancement.
The existing candidate reports a 1,288-byte PS2 attempt; it is not exact
and is not selected in `nonmatching/functional.tsv`. Check frame stepping,
sequence transitions, looping/end conditions and animation events with
synthetic tests before repeating `stream-verified`'s probe. Do not merely
enable an unreviewed candidate or suppress the missing-function stop.
The function was unclaimed at this handoff; check again before editing.

No matching source or report changed this session. The decomp still has
its original dirty `tools/organize_asm.py`; the earlier native working-tree
changes remain uncommitted and separate. All four fixes were committed
locally, no push was made, and this session's function claims are released.

### Hero animation advancement, 2026-10-10 continuation

Local commit subject: `feat(rac1/pal): advance native hero animation`.
Starting from native `949b5d9` and decomp `87162f57`, the working trees
still contained the earlier uncommitted startup work, with no staged work
or active claims. Reproduced `func_L00_00232EF0` at frame 2828, exit 2,
in `.tools/native-run/anim-before.log` before changing the implementation.

Implemented native-only hero animation advancement after reviewing the
existing candidate against the full PAL body. Explicit pointer fields
preserve EE layout through hostgen. Frame blending, curve transitions,
multi-key stepping, looping, restart paths, sound-event intervals and
sound ownership follow the retail branches. No PS2 match is claimed and
the sibling candidate/source/report remain unchanged.

- `Build-Native.ps1`: 45/45 CTest tests pass;
  `.tools/native-hero-animation-build.log`.
- The synthetic hero-animation fixture also passes as a 32-bit executable
  built with `i686-w64-mingw32-clang.exe -std=c11 -O2
  -fno-strict-aliasing -ffp-contract=off`. It asserts EE field offsets and
  covers step rates, strict snap boundaries, multi-key wrap, transition
  curves, forced loops, event endpoints, stale voices and muted modes.
- Hostgen: 3,813 translated functions (one entry plus one helper added),
  196 candidates, 3 stubs, 1,148 without C; 378 readable units, no index
  problems. Level globals retain level relocation in generated code.
- The same documented New Game sequence with `-Name anim-verified
  -ShotEvery 3000 -Frames 5200` passes the animation call and stops at
  `func_L00_00205FF0`, frame 2828, exit 2. Log:
  `.tools/native-run/anim-verified.log`. No frame-3000 image was produced;
  visible gameplay and player movement remain unverified.

Current next action: review and implement `func_L00_00205FF0` from its
retail overlay assembly, checking any existing candidate first, then
repeat the same New Game probe. Keep the missing-function trap enabled.
Only this task's implementation, test, registrations and handoff are
included in the local commit; earlier uncommitted work is preserved.

### Hero model effects, 2026-10-10 continuation

Local commit subject: `feat(rac1/pal): implement native hero model effects`;
hero animation was committed as `9c5ad64`. Implemented `func_L00_00205FF0`
from the complete 1,232-byte PAL body. It preserves model selection, color
pulse and flash timing, four joint manipulator templates, envelope updates
and final detach. A zero pulse period remains fatal, as retail's break is;
there is no missing-call bypass. This is native-only, not a PS2 match.

- `Build-Native.ps1`: 46/46 CTest tests pass;
  `.tools/native-hero-effects-build.log`. The effects fixture also passes
  with the 32-bit compiler and the same flags documented above. It checks
  selection/early exits, color values and phase, flash boundaries, timer
  scaling, attachment initialization/reuse/cleanup and the invalid-period
  trap. Generated code retains level relocation for all effect tables.
- Hostgen: 3,814 functions, 197 candidates, 3 stubs, 1,147 without C;
  378 readable units and no index problems. The external source remains
  the sibling `rac1-decomp/` checkout.
- Before: `anim-verified.log`, frame 2828 at `func_L00_00205FF0`.
  After: the same New Game probe with `-Name effects-verified
  -ShotEvery 3000 -Frames 5200`, exit 2 at frame 2827, now missing
  `func_L00_0020F118`. Log: `.tools/native-run/effects-verified.log`.
  No frame-3000 screenshot was produced. Visible gameplay remains unverified.

Exact next action: review the PAL assembly and any existing candidate for
`func_L00_0020F118`, implement the full behavior, then repeat this probe.
Earlier uncommitted work and all matching sources/reports remain untouched.
Per the owner's instruction during this continuation, new local commits
omit the assistant co-author trailer. No push is authorized or performed.

### Hero equipment creation, 2026-10-10 continuation

Local commit subject: `feat(rac1/pal): create native hero equipment models`;
model effects was committed as `28bc3ea`. Reviewed the existing near-match
candidate against PAL `0020F118..0020F750` and implemented the full equipment
creation routine with explicit EE pointer fields. Primary item selection,
saved/override precedence, scratch clearing, model initialization, optional
and paired items, special items and allocation failures are preserved.
This is native-only; matching source, candidate and report are unchanged.

- `Build-Native.ps1`: 47/47 CTest tests pass;
  `.tools/native-hero-items-build-final.log`. The first build was rejected
  by the unreadable-unit gate because a struct tag collided with an
  existing source definition; renaming the native tag resolved it. No
  runtime claim uses that failed build (`native-hero-items-build.log`).
- The fixture also passes on the 32-bit compiler with the documented flags,
  including static assertions for slot/model/hero EE offsets. Scenarios
  cover selection precedence, retry/failure, paired allocation ordering,
  hidden flags, model initialization and scratch bounds. Hostgen's emitted
  structures use 32-bit guest pointers and tables use level relocation.
- Hostgen: 3,816 functions (entry plus helper added), 198 candidates,
  3 stubs, 1,146 without C, 378 readable units, no index problems.
- Same New Game probe, `-Name items-verified -ShotEvery 3000 -Frames 5200`:
  exit 2, frame 2827, passes equipment creation and now stops at
  `func_L00_0020FC18`. Log: `.tools/native-run/items-verified.log`.
  No frame-3000 screenshot; visible gameplay remains unverified.

Exact next action: review
`rac1-decomp/nonmatching/shared/func_L00_0020FC18.c` against the complete
1,828-byte retail body. This walks seven equipment slots, advances and
attaches their models, and maintains the auxiliary model at hero+0x118C.
The candidate's 1,776-byte attempt is not verified for native use. In
particular, replace its separate `ta[0x30]` / `tb[0x10]` locals with one
64-byte matrix: retail passes sp+0x20 to `func_0020DAF8`, which writes a
whole matrix, and uses sp+0x50 as its translation row. Separate C arrays
do not guarantee that layout. Preserve the explicit zero-period trap at
002101FC. Review all remaining branches before enabling the routine.
Test slot filtering, both item pointers, attachment modes, the auxiliary
model allocation failure, matrix bounds and color animation, then repeat
the documented New Game probe. The function is unclaimed at this handoff.
Preserve the existing dirty baseline and keep missing-call traps enabled.

A separate local credit-cleanup change occurred during this continuation:
native `aacbd91` and decomp `a863e5ec` record the owner's trailer preference.
Earlier commits were rewritten without code-tree changes. Current hashes
for the animation and stream fixes are `72de54e` and `0f1d640`; the earlier
handoff's `9c5ad64` and `949b5d9` refer to their pre-cleanup identities.
The equipment commit is `f1a7a4e`, and effects remains `28bc3ea`.
The prior dirty files remain separate, the index is clear after each
commit, all task claims are released, and no push has been made.

### PR #4 build and test fixes, 2026-10-10

The owner authorized resolving the draft PR's build/test failures and
updating the published branch. Commits `47a46a2` and `591293a` adopt and
verify the required pending fixes, keeping unrelated startup work local.
The earlier excluded credit-preference edit is still uncommitted.

- Hostgen now normalizes Windows source paths, recognizes newer Clang's
  owned anonymous tags, and uses the desugared sizeof/pointer-difference
  types. CMake tracks the translator modules from the function's module
  directory. The portable synthetic regression failed before the fixes
  and passes afterward, covering struct/union/enum typedefs, sizeof,
  pointer subtraction and a nested source path containing spaces.
- The renderer tests now cover the existing raw-GIF continuation behavior
  while still rejecting truncated tags. The viewer expects both moby
  passes and verifies that disabling mobys removes both submissions;
  all prior pixel assertions remain. No renderer behavior or gameplay
  missing-call checks were relaxed.
- Verified the exact committed `591293a` tree in the clean detached
  `.tools/push-verify-9850bfe` checkout. Full Windows release build succeeds,
  using the sibling `rac1-decomp` and LLVM MinGW. All 42 CTest tests pass.
  Logs: `.tools/pr4-committed-build.log` and
  `.tools/pr4-committed-tests.log` in the parent workspace.
- Hostgen reads all 378 units, with no unreadable units or index problems:
  3,785 translated functions, 179 candidates, 3 translation stubs, 1,165
  without C. These differ from the integrated working checkout because
  its additional startup implementations remain uncommitted.
- Hostgen's Python suite: 25 tests, 8 passed and 17 POSIX-only tests skipped
  on Windows; `.tools/pr4-hostgen-tests.log`. Skipped tests are not claimed
  as passing validation. GitHub's upstream checks remain separate.

These fixes remove the two reported test failures and committed Windows
build failure. They do not establish playable gameplay. The last integrated
New Game stop remains `func_L00_0020FC18`; no new runtime probe is counted
here. The next decomp task remains the equipment attachment routine and
matrix-buffer correction described above. PR #4 targets
`OpenRAC/OpenRAC:master` from `PeterFarber:setup/windows-native`.

### PR #4 upstream merge, 2026-10-10

Merged upstream `69920f8` into the PR branch in the isolated
`.tools/push-verify-9850bfe` worktree. The only textual conflict was
`port/cmake/Games.cmake`: both branches fixed hostgen's dependency glob.
Kept upstream's equivalent `OPENRAC_HOSTGEN` directory resolution and
verified that Ninja tracks the translator modules. The Windows path and
newer Clang compatibility fixes remain combined with upstream's changes.

The merged build exposed upstream's unconditional `execinfo.h` include in
missing-call tracing. Windows now captures and prints stack addresses with
`CaptureStackBackTrace`; Unix retains its existing backtrace path. Checks:

- Full Windows release build succeeds; all 42 CTest tests pass using the
  sibling `rac1-decomp`. Logs in the parent workspace:
  `.tools/pr4-merge-build-fixed.log` and `.tools/pr4-merge-tests.log`.
- Hostgen: 8 passed, 17 POSIX-only tests skipped;
  `.tools/pr4-merge-hostgen-tests.log`.
- Synthetic missing-call probes cover tracing enabled/disabled and
  stopping enabled/disabled. Traces contain stack addresses, duplicate
  calls trace once, and stop-on-missing exits with code 2 in both modes.
  `.tools/pr4-merge-probes.log` also verifies CMake dependency inputs.
- The PR diff against upstream passes the whitespace check. Two existing
  trailing spaces in upstream imported candidates are unchanged.

Upstream now continues past missing functions by default. Future gameplay
validation MUST pass `--stop-on-missing` explicitly; continuing is not
evidence of implemented behavior. No gameplay probe is claimed here.
Next action: reconcile the existing equipment attachment investigation
with upstream's newly imported candidate before implementing it, and use
the explicit stop flag for the next New Game probe. The original dirty
workspace remains untouched at `c71be5c`; the merge is on local branch
`fix/pr4-upstream-conflict`, published to the existing PR branch. Bring
the working branch forward while preserving its edits before resuming
campaign work there.

### Equipment attachment, 2026-10-10 continuation

Fast-forwarded the working branch to the verified PR merge `359e61a` and
restored the existing local edits. The only obsolete local code was the
uncapped timing block, now supplied by upstream; removed its duplicate
declaration. The backup stash is retained (its ID is in the parent
`.tools/native-resume-stash.txt`). The excluded credit-preference edits
and unrelated startup implementations remain uncommitted.

Implemented native-only `func_L00_0020FC18` from PAL 0020FC18..0021033C and
the sibling's shared candidate. Explicit pointer fields preserve EE
layout. Covers seven slots, secondary items, detached/Euler/matrix modes,
glove/head/boot poses, auxiliary allocation and pulsing color. The joint
buffer is one 64-byte matrix, with translation at +0x30; the old candidate's
separate arrays did not guarantee that layout. Preserves the zero-period
trap. No matching sources or reports changed.

- Full build and 48/48 CTest tests pass: `.tools/native-attach-build.log`.
- Synthetic fixture also passes independent 32-bit and 64-bit builds;
  EE offset assertions, slot filtering, flag preservation, pose selection,
  matrix output, scratch restoration, failed/deleted/inactive auxiliary
  models, color limits and invalid-period handling are covered.
- Hostgen: 378 readable units, 3,818 translated functions, 199 candidates,
  3 stubs, 1,145 without C; no unreadable units or index problems.
- Fresh New Game probe before: exit 2 at frame 2827, missing attachment.
  After: exit 2 at frame 2828, missing `func_0020EEE8`. Logs:
  `.tools/native-run/attach-before.log` and `attach-verified.log`.
  Uses the earlier 5200-frame input sequence and isolated no-card setup,
  now with `--stop-on-missing` explicitly added to `Invoke-NativeProbe.ps1`.
  No frame-3000 screenshot or playable gameplay is claimed.

Exact next action: implement the handwritten `func_0020EEE8` stored-row
bounding-sphere update, including the shared `func_0020ED80` tail in
`asm/handwritten/text/func_0020ED48.s`. Validate sequence blend/cache,
reflected basis, world extent and packed grid update, then rerun this
strict New Game probe. Local commits only; no new push.

### Stored-basis bounding sphere, 2026-10-10 continuation

Attachment is committed as `1be5ddb`. Implemented native-only
`func_0020EEE8` and its shared tail from `func_0020ED48`: sequence blend
and cached sphere selection, stored/reflected basis, scaled world sphere,
revision count and packed grid update. Preserves the retail signed versus
unsigned grid comparison and uses the native collision code's saturating
float-to-integer convention. Handwritten PS2 code is replaced for native
execution only; matching source and progress reports are untouched.

- Full Windows build succeeds and 49/49 CTest tests pass:
  `.tools/native-bounds-build.log` in the parent workspace.
- Independent 32-bit and 64-bit fixtures pass, including EE layout,
  inactive models, cache reuse, sequence blending and snapshot selection,
  reflected rows, radius scaling, revision wrap, grid boundary rejection,
  unchanged cells and packed-word sign extension.
- Hostgen: 378 readable units, 3,820 translated functions, 200 candidates,
  3 stubs, 1,144 without C; no unreadable units or index problems.
- Same strict New Game probe as above, `-Name bounds-verified`: exit 2
  at frame 2828, now missing `func_L00_0020E3B8`. The former
  `func_0020EEE8` stop is passed. Log:
  `.tools/native-run/bounds-verified.log`; no frame-3000 screenshot.

Exact next action: claim and review the complete 0x63C-byte PAL body at
`rac1-decomp/asm/overlays/func_L00_0020E3B8.s`, whose source is still
`INCLUDE_ASM` in `src/overlays/shared/help_0020CDF0.c`. No sibling candidate
was found. It manages hero joint manipulators via `func_L00_00250060`,
`func_L00_00250120` and `func_L00_002501C8`; recover every branch and test
allocation/removal and state transitions before enabling it. Repeat the
documented probe with `--stop-on-missing`. Visible gameplay is still
unverified. Both task claims are released and commits remain local.

The pre-existing untracked files were checked against the retained stash
and all 18 are preserved. Only the two implementations, their fixtures,
registrations and these handoff sections were staged. Credit-preference
edits and unrelated startup work remain uncommitted; nothing was pushed.

### Hero pose manipulators, 2026-10-10 continuation

Bounds was committed as `ec0635a`. Reproduced the next missing call,
`func_L00_0020E3B8`, then reconstructed its entire PAL 0020E3B8..0020E9F4
body in native C. It maintains two pose nodes: selection/allocation,
fade/removal, interaction with the extra node, category transitions,
special-pose entry and release, frame/sequence propagation and buffer
selection. Explicit pointer fields preserve EE layout. Required allocation
failure remains fatal; no missing calls are bypassed. This is native-only,
not a matching PS2 decompilation; the sibling source/report is unchanged.

- Full Windows build succeeds; 50/50 CTest tests pass:
  `.tools/native-manip-build.log` in the parent workspace.
- Independent 32-bit and 64-bit fixtures pass. Covers layout assertions,
  both selectors, required allocation failure, forced and completed fades,
  extra-node suppression, category transition completion, special-pose
  thresholds/release, buffer bounds and preservation of inactive pointers.
- Hostgen: 378 readable units, 3,821 translated functions, 201 candidates,
  3 translation stubs, 1,143 without C; no unreadable units/index problems.
- Same strict 5200-frame New Game probe and input sequence, with no card:
  `manip-before` stops at the modifier call at frame 2828 (exit 2).
  `manip-verified` now stops at `func_L00_00248EF8` at frame 2827 (exit 2).
  Logs: `.tools/native-run/manip-before.log` and `manip-verified.log`.
  Frame timing varies by one frame; progress is the changed missing call,
  not the frame number. No frame-3000 screenshot or playable-gameplay claim.

Exact next action: claim and review map reveal `func_L00_00248EF8`, the
0x710-byte body in `rac1-decomp/asm/overlays/func_L00_00248EF8.s`, against
`nonmatching/shared/func_L00_00248EF8.c` (1,796-byte candidate versus 1,808
retail; not verified). Recover zone flags, altitude filters, all eight
predicates and the brush/tile-cache bitmap walk. Test map bounds, nibble
selection, callback rejection, alternate maps and reveal-mask changes;
preserve the retail loop limits rather than guessing inclusive edges.
Then repeat the strict probe. The modifier's special-pose helper
`func_L00_0020DC68` and removal helper `func_L00_00250120` are still missing
on other paths; keep their traps. The former's jump table includes tails
outside its nominal 0x44-byte body, so review 0020DCAC..0020DCEC too.

The task claim is released, unrelated dirty work remains uncommitted, and
the fix is committed locally without a push or assistant credit trailer.

### Map reveal, 2026-10-10 continuation

Hero pose manipulators was committed as `47b0229`. Reproduced map reveal
`func_L00_00248EF8` and reviewed its complete PAL 00248EF8..00249604 body
against the sibling's shared candidate. The native-only implementation
preserves projection/alternate maps, zone altitude and state filters,
eight ordered predicates, brush clipping, cache loads and packed zone IDs.
It retains retail's exclusive upper limit of 511. Matching C and its
progress report are unchanged; this is not a PS2 matching claim.

The actual callback table contains C definitions with differing signatures.
Native calls place integers first; four existing height predicates omit gy
and are called through their own signature. Explicit casts select these
signatures (Clang warns about the function-type casts in the raw fixture).
Generated code retains level relocation for both tables and code addresses.
Missing callback entries still trap; none are substituted or skipped.

- Full Windows build and 51/51 CTest tests pass:
  `.tools/native-map-build-final.log`. Earlier `native-map-build.log`
  predates the callback ABI correction and is not final validation.
- Independent 64-bit and 32-bit fixtures pass, covering EE field offsets,
  every filter and callback, callback rejection/order, both ABI signatures,
  clipping, alternate maps, all packed zone IDs, brush masks, cache hits,
  unchanged fog bits, invalid coordinates and guard bytes.
- Additional local fixture `.tools/map-reveal-generated-test.c` passes
  against a verbatim test-only extraction of hostgen's emitted function.
  It exercises guest pointer/stack layout, relocated globals and all four
  height callback addresses plus the full callback signature. The full
  unit could not link in isolation because of unrelated function references;
  production generated files were not modified.
- Hostgen: 378 readable units, 3,822 translated functions, 202 candidates,
  3 stubs, 1,142 without C; no unreadable units or index problems.
- Same strict no-card New Game probe, 5200-frame limit and prior input
  sequence: `map-before` stops at map reveal, frame 2828, exit 2;
  `map-verified` passes it and stops at `func_L00_0025805C`, frame 2828,
  exit 2. Logs are in `.tools/native-run/`. No frame-3000 screenshot or
  playable-gameplay claim. Keep `--stop-on-missing` enabled.

Exact next action: claim and review handwritten `func_L00_0025805C`
(0x1F0 bytes) in `asm/overlays/`, plus its spatial-query callees
`func_L00_00257E18` and `func_L00_00257F4C`, before implementing the moby
update. It is still INCLUDE_ASM in shared/mobyproc_00251A78.c. Validate all
branches and output fields and repeat the strict New Game probe.
Only this task's files, registrations and handoff are committed locally;
earlier edits and excluded credit preferences remain uncommitted. No push.

### Moby lighting and spatial queries, 2026-10-10 continuation

Map reveal is committed as `e5405f9`. The strict `map-verified` run then
reproduced missing `func_L00_0025805C`. Recovered this native lighting
update and both dependencies, `func_L00_00257E18` and
`func_L00_00257F4C`, from the complete PAL 00257E18..00258248 instructions.
The query implementations include the split 00257EBC and
00257FB4/0025804C branch tails. They do not dispatch to missing tail stubs.
This is a native replacement for handwritten PS2 code, not a matching
decompilation; sibling source and progress reports are unchanged.

Preserves the inclusive region scan, XY broad phase, first-hit cube
rejection, translation with w=1, relative grid lists, strict point-light
radius and first-light selection. Ambient blending follows the packed
byte saturation, which discards low product bytes before adding high
bytes; an ordinary lerp differs by one. Final light contribution saturates
each channel independently and leaves unrelated moby fields untouched.

- Windows release build succeeds; 52/52 CTest tests pass:
  `.tools/native-lighting-build.log` in the parent workspace.
- Independent 64-bit and 32-bit fixtures pass, covering layouts, region
  count/clip/boundary rules, first hit versus nearest, grid stride/offsets,
  XY distance, blend endpoints, packed light IDs, channel saturation,
  disabled/no regions, no point light and unaffected object bytes.
- Hostgen: 378 readable units, 3,826 translated functions (three entries
  and a conversion helper added), 205 candidates, 3 stubs, 1,139 without C;
  no unreadable units or index problems. Reviewed generated guest pointers,
  level data relocation and the existing native FPU division helper.
- Same strict 5200-frame New Game probe and no-card input sequence:
  `lighting-verified` passes moby lighting and stops at `func_00218A80`,
  frame 2828, exit 2. Log: `.tools/native-run/lighting-verified.log`.
  No frame-3000 screenshot. Visible gameplay is still unverified.

Exact next action: claim and review particle update dispatcher
`func_00218A80`, the 0x8C-byte handwritten body in
`rac1-decomp/asm/handwritten/text/func_00218A80.s` (source game/partproc.c).
It scans 0x40-byte particle entries, skips negative active bytes, dispatches
through D_001CE100 and reloads its saved cursor/end after callbacks. Recover
the gp-relative globals and inspect the actual callback signatures before
implementing it; test filtering, callback mutation and cursor restoration,
then repeat the strict New Game probe. Keep all missing-call traps enabled.

All six lighting entry/tail claims are released at handoff. The 18 earlier
untracked files match the retained stash byte-for-byte. Earlier tracked
edits and excluded credit preferences remain uncommitted. This fix is
committed locally without a push or assistant co-author trailer.

### Particle update dispatch, 2026-10-10 continuation

Lighting is committed as `01e10e9`. Reproduced `func_00218A80`, then
implemented its full handwritten PAL 00218A80..00218B08 loop in native C.
The 64-byte particle stride, signed active/type bytes, inclusive high
index, initial pool/end snapshot and callback-driven saved cursor/end
reloads follow retail. Inactive entries do not alter the saved cursor.
Callback entries retain normal missing-call traps. All 74 implemented
registered callbacks take one guest pointer; seven registrations still
resolve to missing/alias entries. No matching PS2 claim or source change.

- Windows build and 53/53 CTest tests pass:
  `.tools/native-particle-build.log` in the parent workspace.
- Independent 64-bit and 32-bit fixtures pass. Covers empty/single pools,
  signed inactive filtering, callback selection, payload changes, cursor
  redirection, end extension/truncation, pool/high snapshot behavior and
  mutation of pending entries. The 32-bit fixture needed a standard
  asInvoker manifest because Windows' installer detection requested
  elevation; it then ran without elevation and exited 0. Local artifact:
  `.tools/particle-dispatch-test32.exe`.
- Hostgen: 378 readable units, 3,827 translated functions, 206 candidates,
  3 stubs, 1,138 without C; no unreadable units or index problems. Reviewed
  generated 32-bit pointer globals, callback dispatch and data relocation.
- Same strict New Game probe and input sequence: `particle-before`
  stops at dispatch, frame 2827, exit 2; `particle-verified` passes it and
  stops at `func_001FA1C0`, frame 2827, exit 2. Logs in
  `.tools/native-run/`; no frame-3000 screenshot or playable-gameplay claim.

Exact next action: implement the complete 0x38-byte VU matrix constructor
`func_001FA1C0` from `asm/nonmatchings/text/func_001FA1C0.s`: clear all
16 elements, set the three spatial diagonal entries from f12 and set the
homogeneous diagonal to 1. Test complete writes and bounds, then repeat
the strict probe. Source is still INCLUDE_ASM in game/fastfunc.c.

The particle claim is released. All 18 earlier untracked files still
match the retained stash. Only this implementation, fixture, registration
and handoff are committed locally; no push or assistant credit trailer.

### Uniform-scale matrix constructor, 2026-10-10 continuation

Particle dispatch is committed as `cbe88df`. Its strict runtime probe
reproduced `func_001FA1C0`. Implemented the complete PAL VU constructor
001FA1C0..001FA1F4 in native C: clear the matrix, add the scalar to each
spatial diagonal and set the homogeneous diagonal to 1. All 16 elements
are written. This is a native-only replacement; no PS2 match is claimed
and matching source/progress are untouched.

- Windows release build succeeds and 54/54 CTest tests pass:
  `.tools/native-scale-build.log` in the parent workspace.
- The matrix fixture passes independently on the 32-bit compiler as
  `.tools/scale-matrix-test32.exe`. Tests cover positive, negative and
  fractional scales, both signed zeros, complete initialization and guard
  values before/after the 64-byte matrix. Generated C was reviewed.
- Hostgen: 378 readable units, 3,828 translated functions, 207 candidates,
  3 stubs, 1,137 without C; no unreadable units or index problems.
- Same strict New Game probe with the documented no-card input sequence
  and 5200-frame limit: `scale-verified` passes the matrix constructor and
  stops at `func_L00_002D2E60`, frame 2828, exit 2. Log:
  `.tools/native-run/scale-verified.log`. No playable-gameplay claim.

Exact next action: claim `func_L00_002D2E60`, inspect its full retail body and
any existing candidate/callees, reproduce from this strict probe, implement
the complete required behavior and repeat the same New Game validation.
Keep `--stop-on-missing` enabled; do not bypass missing rendering calls.

The matrix claim is released at handoff. Existing dirty work remains
preserved. Only this fix, its fixture/registration and handoff are committed
locally; nothing is pushed and no assistant co-author trailer is added.

### Upstream master integration, 2026-10-10 continuation

The owner requested the latest upstream master during campaign work.
First committed verified particle dispatch as `cbe88df` and the scale
matrix constructor as `ee8cda9`, then fetched and merged
`OpenRAC/OpenRAC:master` at `cece44d`. Upstream now includes PR #4.
The only merge conflict was hand.tsv: retained both all local native
implementations and upstream's `func_001FA648` quaternion registration.
No local campaign functions were replaced by imported near-match candidates.

All 22 pre-existing dirty files were saved and restored separately from
the merge index. The hand table additionally retains the upstream entry;
earlier credit preferences and unfinished startup work stay uncommitted.
The backup stash is retained, with its ID in the parent workspace's
`.tools/master-merge-stash.txt`. The sibling decomp checkout is untouched.

- Full Windows native build and 54/54 CTest tests pass:
  `.tools/master-merge-native-build.log` in the parent workspace.
- The new tools/test_build_port.py suite passes 12/12 tests via
  `python -m unittest discover -s tools -p test_build_port.py -v`.
- Confirmed CMake still binds to sibling rac1-decomp. Ninja now tracks
  the source overlay-function table and hostgen include headers.
- Hostgen: 378 readable units, 3,829 translated functions, 208 candidates,
  3 stubs, 1,136 without C; no unreadable units/index problems.
- Launcher checks were not run: its node_modules and Yarn are not
  installed in this checkout. Native and build-tool checks above passed.
- Strict no-card New Game probe `master-merge-verified`, using the same
  5200-frame limit and inputs, stops at `func_L00_002D2E60`,
  frame 2828, exit 2. Log:
  `.tools/native-run/master-merge-verified.log`. Gameplay remains blocked.

Exact next campaign action: review `func_L00_002D2E60` (0x4CC bytes) in
asm/overlays against nonmatching/shared/func_L00_002D2E60.c. Its source is
still INCLUDE_ASM in shared/vendor_002D1168.c and it is scheduled as a
draw callback by func_L00_002D3330. Recover the complete rendering behavior
and dependencies, validate the implementation, and rerun the strict probe.
The target is unclaimed; no missing functions have been bypassed.
The merge is local. Nothing has been pushed or published by this session.

### Vendor reflection mesh and native quads, 2026-10-10 continuation

After master integration `381cb5f`, reproduced `func_L00_002D2E60` at
frame 2828. Reconstructed the complete retail 0x4CC-byte draw callback in
native-only C: 102 transformed vertices, reflection UVs, distance/state
branches, transition timer and saved UVs, then 74 indexed four-corner
packets. The old nonmatching sketch has incorrect gp addresses, a truncated
texture return and separate locals where the renderer needs one 0x90-byte
record; it was not promoted into matching source. The native implementation
uses level gp 00166D00 and preserves the full 64-bit texture/blend words.

The level quad host entry was empty. It now uses the existing world-effect
quad renderer, whose record and optional matrix agree with both retail
entry points. Moved that shared adapter into host/effect_quad.c for direct
validation. The existing native renderer projects/clips and draws the
submitted quads; this is not a PS2 packet/microcode interpreter.

- Windows build and 56/56 CTest tests pass, including new vendor_draw and
  effect_quad fixtures: parent `.tools/native-vendor-build.log`.
- Vendor fixture also passes independently as a 32-bit executable:
  `.tools/vendor-draw-test32.exe`. Tests cover the 0x90-byte layout, all
  74 packets, signed alpha packing, 64-bit TEX0, distance boundaries,
  first-use/near/far paths, timer interpolation, UV snapshot and bounds.
- Host fixture verifies real submission for both entry points, optional
  matrix, full state, relocated texture upload records and cached sources.
- Reviewed generated guest C: correct level globals, four-byte pointer
  reads, packet layout and 64-bit texture return. Hostgen: 378 readable
  units, 3830 translated, 209 candidates, 3 stubs, 1135 without C; no
  unreadable units or index problems. Matching source/progress unchanged.
- Same strict no-card New Game input sequence and 5200-frame limit:
  `vendor-before` stops at the vendor callback, frame 2828, exit 2;
  `vendor-verified` passes it and stops at `func_00205270`, frame 2829,
  exit 2. Logs are in parent `.tools/native-run/`. Still no frame-3000
  screenshot or playable-gameplay claim. No missing calls bypassed.

Exact next action: review `func_00205270` in game/loaders.c against its
0x2AC-byte retail body in asm/nonmatchings/text. It selects a streamed
resource bank, loads its model data and rebuilds resource tables. Inspect
its loader callees, recover the full behavior and repeat the strict probe.
Vendor and level-quad claims released; unrelated work preserved. Local
commit only; no push and no assistant co-author trailer.

### Resource-bank loader, 2026-10-10 continuation

Vendor rendering is committed as `acf9521`. Its strict probe reproduced
`func_00205270`, the 0x2AC-byte resource-bank loader. Reconstructed the
complete routine in native-only C: already-loaded check, class-table scan,
automatic/explicit 0x18000-byte buffer selection, real WAD decompression,
model registration/relocation, all 16 signed material references, and the
first matching item of the 37-entry upgrade table. Upgrade state retains
both 64-bit writes at the last render group's packet tail. The actual WAD
and class-relocation callees run; no missing functions are bypassed.

- Windows build and 57/57 CTest tests pass:
  parent `.tools/native-bank-build.log`.
- The new resource_bank fixture also passes on i686:
  `.tools/resource-bank-test32.exe`. It asserts EE structure offsets and
  tests both buffers, toggle/explicit selection, repeat-load early return,
  class search/terminal index, call order, pre-relocation metadata,
  post-relocation pointers/current index, signed material filtering, full
  table bounds, first-match upgrade conditions and exact packet writes.
- Reviewed generated C: guest pointers and 0x10/0x20/0x4C record strides,
  executable/level data relocation and actual loader calls. Hostgen:
  378 readable units, 3831 translated, 210 candidates, 3 stubs, 1134
  without C; no unreadable units or index problems. No PS2 match claimed;
  matching source/progress remain unchanged.
- Same strict no-card New Game scenario, inputs and 5200-frame limit:
  `vendor-verified` stops at resource loading, frame 2829, exit 2;
  `bank-verified` passes it and stops at `func_0020EA70`, frame 2833,
  exit 2. Logs are in parent `.tools/native-run/`. No frame-3000
  screenshot, playable-gameplay claim or disabled missing-call traps.

Exact next action: inspect the complete 0x2D4-byte handwritten
`func_0020EA70` in asm/nonmatchings/text (mobyproc.c refers through
asm/handwritten/text), including its internal entry/tails. It updates the
moby grid rooted at D_001B7A60. Related level routine func_L00_00251B58
already has an UNCOMMITTED implementation and fixture in
hand/level_moby_grid.c and tests/moby_grid_test.c. Compare both retail
bodies and their globals before reusing behavior; preserve that existing
work and stage only the new verified fix. Then repeat the strict probe.

The bank claim is released. All 22 earlier dirty files were checked and
preserved (18 untracked C files byte-for-byte; tracked text with line endings
normalized for comparison). Only this fix, fixture/registration and handoff
are committed locally. No push or assistant credit trailer.

### Executable moby grid, 2026-10-10 continuation

Resource-bank loading is committed as `ee1eb65`. A fresh strict probe
`grid-before` hit the same func_0020EA70 blocker, this time at frame 393
in the title sequence (earlier bank-verified hit it at 2833 in level 0).
Reconstructed its full 0x2D4-byte body, register-only continuation labels,
and bitmap allocate/release callees 0020E9F0/0020E990 in native C.
Executable and level bodies each contain 181 instructions; the eight
instruction differences are global-address loads and code targets.
Existing UNCOMMITTED level_moby_grid.c and its fixture remain untouched.

The executable implementation maintains cell overlap, swap-with-last
removal, allocation order, resize thresholds and capacity-sized block
copies. Native invalid-range/allocation-exhaustion checks fail explicitly;
retail's missing-member/double-release traps remain fatal. This is native
recovery, not a byte-matched PS2 decompilation.

- Windows build and 58/58 CTest tests pass:
  parent `.tools/native-exe-grid-build.log`.
- Independent i686 fixture `.tools/exe-grid-test32.exe` passes. Tests
  cover bitmap allocation/fragmentation/last block/exhaustion, release
  across words, corruption traps, full 32-bit ID comparisons, boundary
  cells, overlap, shrink thresholds and 500 randomized membership changes
  checked against an independent membership/bitmap oracle.
- Hostgen: 378 readable units, 3837 translated functions (including three
  local helpers), 213 candidates, 3 stubs, 1131 without C; no unreadable
  units/index problems. Reviewed guest pointer and data relocations.
- Normal strict New Game probe `grid-verified` completes 5200 frames,
  exit 0, with no missing-function report. Inputs match the earlier
  campaign sequence, including the requested analog movement at 4000.
  Log: `.tools/native-run/grid-verified.log` in the parent workspace.
- Inspected `.tools/native-run/grid-verified-3000.png`: Ratchet and the
  first Veldin scene are visibly rendered. Level 0 has 291 mobys and 75
  effect quads. The pose remains finite. This is initial scene validation,
  not proof of playability: the sampled hero position stays at
  (132.09, 115.48, 31.43), including after the scripted movement interval.

Next action: finish the owner's requested fast test launch (existing
OPENRAC_DIRECT with --level 0, explicit movie skipping), verify it loads
Veldin through NewGameInit and the normal loader, then investigate why the
movement probe does not change the hero position. Compare frontend input
frame timing, pad socket/read state and the level's input/update path before
changing gameplay. Keep the full New Game run as a separate regression.
Grid claims released; earlier dirty files remain preserved. Local commit
only, no push, no assistant co-author trailer.

### Fast Veldin development launch, 2026-10-10 continuation

The executable moby-grid recovery is committed as `661f313`. The owner
requested avoiding menu navigation on each development run. Added opt-in
`--skip-movies` to the native executable; it logs each skipped movie and
returns the existing movie-skip result. Normal playback is unchanged.
The existing RAC1 PAL OPENRAC_DIRECT + --level path still calls NewGameInit
and the normal loader. No new matching code or missing-call bypass.

Workspace launchers (local, outside either Git repository):

- `./Run-Native.ps1 -Direct`: fresh Veldin start, skip movies, no card,
  isolated native-test-cards path, stop-on-missing, no frame limit.
- `./Test-Native.ps1`: same startup, uncapped, 2400-frame limit, screenshots
  every 800 frames, log/screenshots in `.tools/native-run/direct-test*`.
- `./Test-Native.ps1 -FullStartup`: retains the normal 5200-frame campaign
  input sequence and movie playback. Run it for startup regressions.
- Both fast launchers accept `-Level N`; only level 0 is verified here.

Both fast launchers send Cross at frontend frame 100 for five frames to
acknowledge the initial no-card warning. Without this input, the game
correctly waits before the title world/direct-start hook. Earlier
`fast-verified`/`fast-long` probes supplied input late and ended during
transition. `fast-complete` had no input and was manually terminated at
that warning; its frame-2000 screenshot identified the delay. These
earlier runs are not successful level validation.

Validation:

- Native build and 58/58 CTest tests pass (`.tools/native-fast-probe-build.log`).
- `Test-Native.ps1 -Name direct-verified -Frames 2400 -ShotEvery 800`
  completes, exit 0, no missing-function/error report. Level 0 is loaded
  by logged frame 1000; four movie skips are logged. Inspected
  `.tools/native-run/direct-verified-1600.png`: Ratchet and Veldin render
  with 291 mobys, 75 effect quads and a finite 111-joint pose.
- PowerShell parser checks pass for both launchers and Invoke-NativeProbe.
- `Run-Native.ps1 -Direct -Frames 1200` independently reaches level 0 by
  logged frame 1000, exits successfully and restores OPENRAC_DIRECT and
  OPENRAC_PRESS. Log: `.tools/native-run/direct-launcher.log`.
- Normal New Game validation remains the 5200-frame `grid-verified` run
  recorded above; movie skipping is explicitly excluded from that evidence.

This validates automatic entry into the first scene, not playable movement,
other levels, saves or cutscenes. Exact next action: run a direct probe with
`-Press '100:4000:5,1400:0:150:128:0'`, correlate frontend input frame indices
with the logged game frames, then trace pad socket/read state and the
level's input/update path to explain the unchanged hero position in the
normal movement probe. Preserve missing-function checks. Existing dirty
work remains preserved; local commit only, no push or assistant trailer.

### Controller routing, 2026-10-10 continuation

Fast-launch support is committed as `87ce8ad`. Reproduced unchanged hero
position with the direct `input-before` probe. Trace revealed pad reads
stopped after title frame 334 despite pad socket 0 remaining in state 1.
The catalogue folded the level UpdatePad(void) wrappers into sound wrapper
func_00216270. Generated code selected the pad callee but still supplied
OPENRAC_CODE(0x2177F0), a callback address, instead of the pad data address
0x13CA40. The real pad was never updated during gameplay.

Split all 19 pad wrappers onto existing func_00218908. Parent local audit
`.tools/audit-pad-routing.py` verified every 32-byte retail body equals
the executable pad wrapper except its correctly relocated jal; all other
instructions, including the fixed data address, are identical. Audit log:
`.tools/pad-routing-audit.log`. Matching source/catalogue unchanged.
Added optional frontend input trace and processed-pad diagnostics so raw
reports and game state can be compared without memory-write shortcuts.

Validation: Windows build and 58/58 CTest pass
(`.tools/native-pad-routing-build.log`). Hostgen tests: 25 discovered,
8 passed, 17 POSIX-only tests skipped on Windows. Generated table now
routes level-0 0x268738 and the other 18 copies to func_00218908 and
retains its executable entry. Strict `input-fixed` probe restores
controller reads after level loading at frame 958, then stops at missing
func_L00_00215A90, frame 1013, exit 2. The scripted movement was scheduled
for 1200; the owner confirmed moving the physical Xbox left stick just
before the stop. Hardware input now reaches gameplay; movement is not
yet validated because the hero-lean path is unimplemented.

Next action: recover func_L00_00215A90 (2084-byte procedural hero lean),
starting with nonmatching/shared/func_L00_00215A90.c and reviewing all
retail branches and four distinct joint-modifier setter destinations.
It is claimed by codex-pad-routing while implementation continues.
Existing dirty work preserved. No pushes or assistant credit trailers.

### Hero lean recovery, 2026-10-10 continuation

Controller routing is committed as `bdbeac1`. Recovered procedural hero
lean func_L00_00215A90 in native-only hand/level_hero_lean.c. Reviewed
all 0x824 retail bytes against the existing nonmatching candidate and
corrected its nonlinear gain (1.25 rather than 2), fallback selection
(+0x208C equal to 2 or 4, not primary state 4), and the one-ULP error in
the 25-degree clamp. Preserve all 12 call ordinals so hostgen resolves
the four distinct modifier setters from the retail call sites.

Validation: Windows build and 59/59 CTest pass
(`.tools/native-hero-lean-final-build.log`); the dedicated fixture also
passes as `.tools/hero-lean-test32.exe`. It covers primary-state priority,
fallback states, both walking branches, nonlinear gain, signed clamps,
speed weighting, airborne state 8, state 0x81 velocity-heading override,
threshold equality, angular offset, unchanged fields and setter parameters.
Generated C retains ordinals 0 through 11 and the correct guest globals.
Hostgen has 3838 translated functions, no unreadable units/index problems.

Strict final-source probe `lean-final` uses neutral left stick before 1200,
Cross at 100 and full forward at 1200 for 150 frames. At frame 1200 the
processed left stick is (0,-1), held direction 0x1000. The probe passes
the lean function and stops at func_L00_002293E8 (walk/run animation),
frame 1202, exit 2. Logs/screenshots are in parent `.tools/native-run/`.
Hero displacement and jump are still unverified; checks remain enabled.

Next action: recover the 0x390-byte walk/run animation function from
nonmatching/shared/func_L00_002293E8.c, including phase transfer between
the old and new sequences and actual 32-bit guest table strides. It is
claimed by codex-pad-routing while work continues. Matching checkout
unchanged; prior dirty work preserved. No push or assistant credit.

### Walk/run animation and first movement, 2026-10-10 continuation

Hero lean is committed as `5d23adf`. Recovered func_L00_002293E8 from
the existing candidate after reviewing all 0x390 bytes of retail code.
Native implementation preserves timer/lock gates, threshold hysteresis,
sequence phase transfer, transition offsets and mode-specific animation
rates. Sequence slots use pointer types so hostgen preserves 4-byte guest
strides while the standalone fixture also works on 64-bit hosts.

Windows build and 60/60 CTest pass (`.tools/native-hero-gait-build.log`).
The dedicated fixture also passes on i686 (`.tools/hero-gait-test32.exe`),
covering both transition directions, fractional phase truncation/wrapping,
gates and their recheck, threshold equality/scaling, hysteresis and rate
clamps. Reviewed generated pointer strides and relocated globals. Hostgen:
3839 translated, no unreadable units or index problems. Native-only work;
no matching source or progress changes.

Strict `gait-verified` probe uses the same neutral/forward input as
lean-final, plus Cross scheduled at 1500. Forward at 1200 now moves the
hero from (132.09,115.48,31.43) to (140.73,118.44,31.23) by frame 1300;
camera position moves from (125.8,113.3,33.0) to (134.3,116.2,33.1).
The pose stays finite. At frame 1330 it stops at func_L00_00214520, exit 2.
Jump was not reached. Log: `.tools/native-run/gait-verified.log`.
This is first measured native walking, not full gameplay validation.

Next action: implement func_L00_00214520, 0x2A0 bytes, movement-heading
adjustment/velocity damping. Candidate:
nonmatching/shared/func_L00_00214520.c. Review the typed hero offsets,
vector helpers and mode gates against retail, validate, then repeat the
same strict forward/jump probe. All earlier dirty work is preserved.
Local commits only, no push or assistant trailer.

### Heading adjustment and controller validation, 2026-10-10 continuation

Walk/run animation is committed as `ddebdc1`. Recovered native-only
func_L00_00214520, reviewing the complete 0x2A0 retail body against
nonmatching/shared/func_L00_00214520.c. Preserve heading-dependent turn
gain, mode and boost gates, airborne speed limit, planar vector lengths,
previous velocity, projection copy and alternate-mode helper ordering.
Use the actual resident hero symbol and float timestep global. Initialize
the unused scratch w lane; the final result preserves velocity w.

Windows build and 61/61 CTest pass
(`.tools/native-hero-heading-build.log`); the focused test also passes as
`.tools/hero-heading-test32.exe`. It asserts all used hero field offsets,
same/opposite/orthogonal turns, both zero-length cases, gain caps, heading,
boost gates, airborne cap, projection isolation and damping call order.
Generated C reviewed: guest-addressed locals and resident hero fields are
correct. Hostgen: 3840 translated functions, no unreadable units, problems
or index problems. No matching source or matching progress was changed.

Strict `heading-verified` repeats neutral left stick through 1199,
forward from 1200 for 150 frames and Cross scheduled at 1500. Walking
and camera displacement reproduce the previous result at frame 1300.
It passes the recovered routine, then stops at missing func_L00_0020D3A0
at frame 1330, exit 2. The walking probe still does not reach its jump.
Separate strict `heading-jump` keeps the stick neutral and presses Cross
at 1100: processed held 0x40, hero animation changes to sequence 7,
then missing func_L00_002147C0 stops execution at 1101, exit 2. A complete
jump is not verified. Logs/screenshots: parent `.tools/native-run/`.
The physical Xbox left-stick path was independently confirmed by the
owner earlier in this session; these two final probes use scripted input.

Exact next action: claim and recover func_L00_0020D3A0 (0x24C bytes),
the nearby surface-contact table lookup. Start with its candidate in
nonmatching/shared/, review all retail branches, preserve 32-byte guest
table entries and pointer-valued filters/results, and test rejection,
threshold equality and successful output writes. Repeat:
`Test-Native.ps1 -Name contact-verified -Frames 2200 -ShotEvery 500
-Press '0:0:1200:128:128,100:4000:5,1200:0:150:128:0,1500:4000:5'`.
Then recover func_L00_002147C0 (0x5A0 bytes, camera length/offset easing)
from its existing candidate and repeat the independent jump probe:
`Test-Native.ps1 -Name jump-verified -Frames 1700 -ShotEvery 250
-Press '0:0:1700:128:128,100:4000:5,1100:4000:5'`.
Both runs retain strict missing-function checks and development movie
skipping. They do not validate normal startup, other levels or full play.

Prior tracked edits and all 18 baseline untracked files remain preserved;
the latter were hash-checked against the pre-merge snapshot. Release the
heading claim after this local commit. No push or assistant credit trailer.

### Surface lookup recovery, 2026-10-10 continuation

Starting HEAD d26df14. Recovered native func_L00_0020D3A0 after reviewing
the full 0x24C retail body. Preserve exclusion/require filters, active
surface checks, radius equality, strict planar/height limits, first-hit
outputs and table count reloads. Fields and output are pointer typed;
filters retain the existing integer ABI and compare guest address bits.
This is native-only recovery, not a proven matching decompilation.

An initial pointer-typed signature conflicted with the source declaration;
Build-Native caught the unreadable unit. Corrected the interface and
rebuilt successfully: 62/62 CTest, i686 contact fixture, 3841 translated
functions and no hostgen problems/index problems/unreadable units.
Reviewed generated table: 32-byte entries, pointer at +16, index at +20,
and a four-byte guest output store. The helper func_L00_0025EFC0 remains
unimplemented; strict checks still stop if it is reached. Successful
surface hits are fixture-tested, not established by this runtime probe.

Strict contact-verified forward/jump probe passes the old frame-1330
blocker, moves the hero to (147.25,120.68,29.48) by frame 1500 and reaches
the scheduled jump. Stops at func_L00_002147C0, frame 1501, exit 2.
Logs: parent .tools/native-hero-contact-final-build.log and
.tools/native-run/contact-verified.log. No bypasses or memory-write cheats.

Next: recover func_L00_002147C0 from its candidate and full 0x5A0 retail
body, then repeat both forward/jump and neutral-stick jump probes. Correct
the earlier candidate/handoff interpretation: this is vertical movement,
jump impulse and gravity, not camera easing. func_L00_002342F8 returns
the component along the movement frame's up axis; 002343A0 sets it.
The routine is claimed by codex-contact during ongoing implementation.
Existing dirty work preserved; local commits only, no push or credit trailer.

### Vertical movement and first complete jump, 2026-10-10 continuation

Surface lookup is committed as 14c8eac. Recovered func_L00_002147C0 from
the full 0x5A0 retail body and existing candidate. Correct interpretation:
hero jump impulse, timed vertical acceleration, hover lift and gravity,
not camera easing. Keep original helper calls and their order, timer
gates, sequence sentinel, impulse accumulation and downward speed clamps.
Use resident hero/global symbols and a pointer-typed sequence table.
Native-only implementation; no matching source/progress changes.

Final Windows build and 63/63 CTest pass without compiler warnings:
.tools/native-hero-vertical-final-build.log. Focused fixture also passes
on i686, covering ramp activation/timeout, accumulated impulse, sequence
phase/duration/sentinel, hover thresholds/classification, gravity clamps
and an alternate up axis. Generated sequence offsets are 0x14/18/1C/20,
step stride 12 and folded timing-call ordinals 0..4 are preserved.
Hostgen: 3842 translated, no unreadable units or reported problems.

All probes retain strict missing-function checks and skip movies:
- vertical-jump: neutral stick, Cross at 1100, 1700 frames, exit 0.
- vertical-walk-jump: forward at 1200 for 150 frames, Cross at 1500,
  2200 frames, exit 0. Hero ends at (147.25,120.68,29.48).
- vertical-airborne: Cross at 1080, 1400 frames, exit 0. Height changes
  from 31.43 at 1000 to 32.75 at 1100, then returns to 31.43 by 1200.
  Reviewed the 1100 screenshot: visibly airborne, finite posed joints.
  This establishes one complete native jump and landing in Veldin.
- vertical-explore: forward for 700 frames from 1200, Square at 1450,
  Cross at 1600/1620. Stops at func_L00_00211A18 at 1450, exit 2.
  Attack and double jump are not yet validated by this probe.
Logs/screenshots are in parent .tools/native-run/.

Exact next action: recover func_L00_00211A18, a 24-byte object-type
predicate checking signed short +0xA6 against 0x47. Verify its null
branch to func_001E97C8 before implementation, then repeat:
Test-Native.ps1 -Name attack-verified -Frames 2400 -ShotEvery 200
-Press '0:0:1200:128:128,100:4000:5,1200:0:700:128:0,1450:8000:5,1600:4000:5,1620:4000:5'
No candidate exists in nonmatching/shared for this small function.
The contact projection dependency func_L00_0025EFC0 remains unimplemented
and was not reached. Other levels, saves, full combat and campaign remain
unverified. All 18 baseline untracked files hash-check unchanged; tracked
dirty work preserved. Local commits only, no push or assistant trailer.

### Attack object queries, 2026-10-10 continuation

Vertical movement is committed as b10216c. Recovered native object-type
predicate func_L00_00211A18 and module-object accessor func_L00_0020DAF8.
The predicate returns false for null, otherwise compares signed short
+0xA6 with 0x47. The accessor uses resident hero + slot*0x50, returns the
pointer at +0x1090 only if the state at +0x10B4 is 2, otherwise null.
Its catalogue body ends before the success and failure return tails at
0020DB1C and 0020DB24; both tails were reviewed and included in the C.

Local audit .tools/audit-hero-object.py verified all 19 predicate bodies
and their null return-zero targets, and all 19 complete accessor bodies
including both split tails. Audit output: .tools/hero-object-audit.log.
No retail bytes were committed. Generated code reviewed: signed short
load, 0x50 guest stride and four-byte pointer load/return. Native-only
recovery, not a byte-identical PS2 match. Matching checkout unchanged.

Windows build and 63/63 existing CTest pass:
.tools/native-hero-object-final-build.log. Hostgen has 3844 translated
functions with no unreadable units or reported problems. Predicate-only
attack-verified first exposed the accessor at frame 1450. Final strict
attack-object-verified passes both, continues through Square at 1450 and
Cross at 1600/1620, then stops at missing func_L00_00273090 at frame 1701,
exit 2. At 1700 the hero is at (169.59,125.53,29.48), joints finite.
This does not establish full combat or a complete double jump. The prior
isolated single-jump ascent/landing and walking probes passed separately.

Exact next action: claim and recover func_L00_00273090 (0x240 bytes),
particle type 53 update in shared/partupd_00272158.c. Candidate exists at
nonmatching/shared/func_L00_00273090.c. Review size/alpha interpolation,
byte wrap, signed lifetime counters, drift/gravity and kill boundaries
against the full retail body; add focused boundary/lifetime validation.
Repeat Test-Native.ps1 -Name particle53-verified -Frames 2400 -ShotEvery 200
-Press '0:0:1200:128:128,100:4000:5,1200:0:700:128:0,1450:8000:5,1600:4000:5,1620:4000:5'
Use strict missing-call checks. Logs/screenshots are in .tools/native-run/.
The separate contact projection helper func_L00_0025EFC0 is still absent.
No active game is left running. Release both object-query claims after
commit. Existing tracked edits and all 18 baseline untracked files are
preserved. No pushes, history rewrites or assistant credit trailers.

### Particle type 53 recovery, 2026-10-10 continuation

Starting HEAD 1c026ec. Recovered func_L00_00273090 in native-only
hand/level_particle53.c from the full 0x240 retail body and existing
candidate. Preserve two-phase size/alpha interpolation, RGB bytes,
wrapping byte accumulation, signed lifetime counters, position drift,
gravity ordering and inclusive position bounds [2,1021]. Out-of-bounds
particles are removed before decrementing their lifetime. No matching
source/progress changes or stub bypasses.

Windows build and 64/64 CTest pass (.tools/native-particle53-build.log).
The focused fixture also passes on i686 (.tools/particle53-test32.exe),
covering the phase boundary, odd alpha truncation, zero/negative lifetime,
byte wrap, unchanged fields, drift/gravity, each axis boundary and removal
ordering. Reviewed generated guest accesses. Hostgen: 3845 translated,
no unreadable units or reported problems. Existing dirty work preserved;
all 18 baseline untracked files still hash-check unchanged.

Strict particle53-verified attack/jump route passes the old frame-1701
stop. At frame 1800 hero position is (176.80,129.33,28.77), pose finite.
Stops at missing func_L00_001FF5B0 at frame 1817, exit 2. Log/screenshots:
parent .tools/native-run/particle53-verified*. Keep the same probe inputs.

Next action: recover the handwritten 0x58-byte planar speed limiter
func_L00_001FF5B0 from retail, including the shared return-zero branch.
It leaves output untouched for zero planar length or a requested limit
above the current length; equality takes the scaling path and returns 1.
Otherwise it scales x/y by limit/length and preserves z/w bits. Review
the integer sign test on the float difference and all caller signatures
(limit first, then output/input pointers). Validate aliases, no-write
paths and z/w preservation, then rerun the strict 2400-frame attack route.
Local commits only; no push or assistant credit trailer.

### Planar speed limiter, 2026-10-10 continuation

Particle type 53 is committed as 81a9e33. Recovered handwritten
func_L00_001FF5B0 in native C, including the shared return-zero tail.
Preserve no-write paths, equality returning 1, separate sqrt and divide,
the signed test of the float-difference bits, source/output aliasing and
exact z/w bits. Limit is the first argument, then output/input pointers.
This is a native replacement for VU assembly, not matching compiler C.

Build and 65/65 CTest pass (.tools/native-planar-limit-build.log); focused
fixture passes on i686. Tests include just-above/below limits, equality,
zero/negative limits, signed zero, unchanged output on early return,
overlapping buffers and signed-zero/NaN bit preservation in copied lanes.
Local audit .tools/audit-planar-limit.py confirms identical body and return
tail in all 19 catalogue copies (.tools/planar-limit-audit.log).
Generated signature, guest loads/stores, bit tests and openrac_fdiv were
reviewed. Hostgen: 3846 translated, no unreadable units or problems.

Strict planar-limit-verified route passes frame 1817, continues into a
fall (hero z=1.10 by 1900), then stops at func_L00_002862E0 at frame 1905,
exit 2. Log/screenshots are in parent .tools/native-run/. No missing-call
checks were bypassed. A fall/respawn is now the reproduced blocker; the
current route does not distinguish an intended ledge from collision issues.

Next action: recover func_L00_002862E0 (0x1B4 bytes), snapshot restoration
in shared/pause_00277208.c. Review the candidate against retail: its
unsigned-char snapshot declaration must not turn word reads into byte
reads, and pointer-valued copy/range arguments need native-safe types.
Handle both invalid-snapshot clearing and complete restoration, including
hero vectors, moby fields, status, mode-dependent search and callbacks.
Then repeat the strict 2400-frame route to test respawn. Existing dirty
work preserved. Local commits only, no push or assistant credit trailer.

### Checkpoint restoration, 2026-10-10 continuation

Planar limiter is committed as 9a0cdbe. Recovered func_L00_002862E0
(0x1B4) in native hand/level_checkpoint_restore.c. Reviewed the complete
retail body and its hero-switch callee. Preserve the full-word validity
flag, 0xC60 clear/copy, hero vector bits, moby fields, status bytes and
halfword, mode-dependent object search, and callback ordering. Correct
the candidate's omitted third argument: the found object remains in a2
and is consumed by func_L00_002110C0. Keep pointer arguments native-safe
and reload the hero object after callbacks. This is native-only recovery;
no matching source or progress change, and no missing-call bypass.

Final build and 66/66 CTest pass without compiler warnings:
.tools/native-checkpoint-restore-final-build.log. Focused fixture also
passes on i686 (.tools/checkpoint_restore-test32.exe). Covers invalid and
high-byte-valid flags, complete copies with guard bytes, unchanged source
and unrelated fields, vector NaN/signed-zero bits, mode 0/3 skipping,
mode 1/2 first-match/exhaustion/empty/reversed ranges, invalid-mode assert
callback, found-object argument, callback order and hero pointer reload.
Initial build exposed C89 loop-variable scope; fixed before final checks.
Generated guest pointers, full-word mode arguments, 0x100 search stride,
copy alias and retail sound-wrapper call routing were inspected. Hostgen:
3847 translated, no unreadable units, index problems or other problems.

Strict direct-Veldin probes, movies skipped, isolated no-card state,
OPENRAC_TRACE_INPUT=1, no keep-going:
- checkpoint-restore-verified, 2400 frames, ShotEvery 200, exit 0.
  Press: 0:0:1200:128:128,100:4000:5,1200:0:700:128:0,1450:8000:5,1600:4000:5,1620:4000:5
  Passes the old frame-1905 stop and respawns at (132.09,115.48,31.43)
  by frame 2000. Reviewed screenshot 2000: hero back beside the ship.
- checkpoint-resume-verified, 2700 frames, ShotEvery 100, exit 0.
  Same Press plus: ,1900:0:300:128:128,2200:0:150:128:0,2380:4000:5
  After respawn, moves to (147.25,120.68), jumps to z=30.80 at 2400,
  and lands at z=29.48 by 2500. Posed joints remain finite.
Logs/screenshots: parent .tools/native-run/<probe-name>*.
These prove this fall/respawn and subsequent movement/jump route, not
every saved-checkpoint mode or full combat/double-jump behavior. The
original fall still needs an intended-ledge versus collision assessment.

No new strict missing call was reached on these routes. Next action:
claim func_L00_0025EFC0 (0x39C), the remaining projection dependency in
shared/mobyutil_00258BC8.c; no nonmatching/shared candidate exists.
Review the complete retail routine and callees, and callers in
help_002180F8.c / help_0022A340.c. Preserve its three vector/path pointers,
segment-index and fraction outputs, integer mode, three float arguments
and integer return. Reproduce its branch behavior with a focused fixture
before implementing; do not treat its current void stub/zero coercion as
valid output. Then extend strict Veldin collision/combat exploration to
exercise it. Saves, other levels and the full campaign remain unverified.

Existing tracked work and all 18 baseline untracked C files are preserved
(hash-checked). No game left running. Release the restoration claim after
commit. Local commits only; no push or assistant credit trailer.

### Push and PR preparation, 2026-10-10

PR #4 was merged into upstream master. Fetched origin/fork; both master
and fork/setup/windows-native were cece44d. The local branch contains that
upstream tip. Existing commits are suitable for a new PR. No push or PR
creation was performed during this preparation. A fork push dry-run passed.
The excluded aacbd916 commit is not an ancestor; outgoing commits contain
no Codex co-author trailers, game bytes, toolchains or build products.

Clean detached checkout: parent .tools/push-verify-6a31139. Its code is
6a31139; documentation was advanced to 176116f, which fixes a stale PAL
revision/date in docs/SOURCES.md using tools/openrac.py tables. Generated
tables now have no diff. The final preparation handoff changes only docs.

Two Windows LLVM MinGW release configurations of the committed tree pass
the full PAL build and 61/61 committed CTest tests:
- Sibling rac1-decomp source: 3816 translated functions, no hostgen problems.
  Strict direct startup stops at func_L00_00251E30, frame 939, exit 2.
- Bundled games/rac1/pal source (import 6127cb448010): 3934 translated,
  no hostgen problems. Strict 2700-frame Veldin probe exits 0. Respawn at
  (132.09,115.48,31.43), subsequent walking, jump z=30.80 at frame 2400,
  landing z=29.48 by 2500, joints finite. This clean checkout demonstrates
  the committed port's tested route using its newer bundled source.
Build logs: .tools/next-pr-clean-{external,bundled}-build.log.
Probe logs/screenshots: .tools/native-run/next-pr-clean-{external,bundled}*.
Commands/settings recorded in .tools/verify-next-pr.ps1 and
.tools/probe-next-pr.ps1. Strict checks, movie skip, no-card isolated state;
no memory-write overrides. No running game remains.

Linux/WSL: hostgen 25/25 (no skips), tools 64/64, editor 75/75 pass.
Shared-file check and PAL/RAC4 progress checks pass. Native Windows Python
tools/editor runs encountered path/symlink limitations; Linux passes the
same suites. Initial Windows table generation had encoding/newline issues;
regenerated with Linux and verified against the final committed docs.
Some compiler warnings remain (callback casts and existing runtime/SDL
warnings); this is not a warning-free full rebuild. Launcher and other
game builds were not rerun; they have no changes in the outgoing diff.

Publication preparation: local .tools/next-native-pr-body.txt and
.tools/next-native-pr-request.json contain the proposed PR. Target
OpenRAC/OpenRAC:master from PeterFarber:setup/windows-native. Recheck the
remote tips before publishing; push normally, never force. No open PR
exists for this head as of this check. The user's request was preparation.

Campaign follow-up: compare the sibling checkout with the newer bundled
PAL import before recovering another apparently missing function. The
normal workspace remains configured against the sibling, as required;
no imported source, sibling source, or existing uncommitted implementation
was changed or included. All 18 baseline untracked C files hash-check
unchanged. Full combat, saves, other planets and campaign remain unverified.

Full completion requires all recoverable game code accounted for, no
unimplemented required native calls, documented native replacements for
console-specific assembly, matching audits passing without new mismatches,
and recorded native gameplay validation from New Game through the ending
with save/reload, level transitions, controls, graphics, audio and cutscenes.
Neither matching percentages alone nor a passing CTest suite proves this.

### Settings, combat, sound and window fixes, 2026-10-12

Seven reported problems, one commit or more each (subjects below). Built
from the sibling `rac1-decomp` as it stood (no change made there), checked
in level 0 through `OPENRAC_DIRECT` and on the normal New Game path, with
`--no-card`, a scratch `--settings` file and scripted pad input.

- Settings: `feat(renderer): a target wider than the game's 4:3 picture`,
  `feat(port): picture shape, frame rate and fullscreen as the player's
  settings`. Rows on the title's Options, Video page; port/README.md, "The
  player's settings". The tick stays 50 a second: frames between ticks are
  blended (camera, mobys, joint poses). The game's own 60 Hz video mode is
  not followed by the port's tick.
- Level 0's creatures and crates: `fix(hostgen): a definition that ends in
  a call returns that call's result` (the ground height of every walking
  creature was 0), `fix(rac1-pal): the collision queries test a posed
  class's joint primitives`, `feat(rac1-pal): hit spheres, the toad's hits,
  knockback, crates that break`.
- `fix(port): a deleted moby is not drawn` (state 0xFD: the black bolt).
- Sound effects: the hostgen fix above (distance volume and pan were 0),
  then `feat(rac1-pal): the pan's clamp and the occlusion line's length, in
  C`. Not listened to: checked by the commands the game sends
  (`OPENRAC_SOUND_LOG`).
- `fix(port): no blue strip as a story card ends`.
- `fix(port): the game goes on while its window is dragged` (the game on a
  fiber beside the event pump, Windows only).
- `fix(rac1-pal): the hero stands on a crate he lands on` (the catalogue
  folds three block accessors into func_L00_0025D390; split_places.tsv).

Left for the decompilation: func_L00_002D6610's functional C tests the
damage resolver's return value where retail tests its fifth argument (the
port overrides it in hand/level_creature_hits.c); the near misses for
func_L00_0025A8E8, func_L00_0025AA20, func_L00_0025D6F0, func_L00_002D1E68,
func_L00_00261B00 and func_L00_002D4EC0 are used from hand/ and are not in
nonmatching/functional.tsv; func_001F9BB0 and func_001F9E58 were written
from their callers, not from their instructions. Still without C:
func_L00_00254218 (the collision kernels' joint pose, replaced by the
port's evaluator) and func_L00_002788E0 (called while the pause menu's
pages are open: seen in these runs). Other levels' creatures were not
tried.

New switches: `OPENRAC_MOBY_LOG=N[:RADIUS]`, `OPENRAC_HIT_LOG`,
`OPENRAC_SOUND_LOG`, `OPENRAC_NO_PUMP_FIBER`.

### The hero's walk, bolts and volume, 2026-10-12 continuation

Three reports after the fixes above, each traced by measurement in level 0
(scripted, silent, `--no-card`); the method and the recipes are in
[RAC1_PAL_BUGFIX_GUIDE.md](RAC1_PAL_BUGFIX_GUIDE.md).

- The hero froze and jumped forward, and a dying creature's body did not
  move with its animation: `fix(sound): move a voice to its moby, not the
  moby to its voice` (the candidate for func_0022DD68; in `rac1-decomp`
  and in the in-tree copy). Found by bisecting the 150 functions of the
  inherited-result change down to func_0022DB00, whose result made sounds
  start. Stick held for 200 ticks: 0 ticks without movement, from about
  130.
- No bolts from crates or creatures: `fix(hostgen): keep a 128-bit cast
  128 bits wide`. Two crates: 22 bolts collected; one class 749 creature:
  6.
- `feat(port): a volume setting that reaches silence` (`volume`, F9 and
  F10). It does not explain the report behind it, music audible with the
  Windows mixer at zero, which is open.

Not a bug: the hero's standing pose is static until a random fidget.
Open and next: the guide's last section. The wrench against the hand has
not been looked at again; nobody has played these fixes yet.

### Levels 4 and 12: the hook stubs and the frame alignment, 2026-10-13

Two crashes reported as "level 4 (null host access) and level 12 (access at
0x3FA93F19)", started with `OPENRAC_DIRECT=1 OPENRAC_PRESS=100:4000:5` (without
the Cross press the run waits on the menu at level -1 and shows nothing).

- Level 12: `fix(port): the engine's calls to the executable's hook stubs go to
  each level's own function`. The executable's `func_001E9780..001E97C0` only
  return; every level fills them with a function of its own (level 12's
  `func_00230A90` calls, at the same instructions, the function that clears the
  draw callback lists, and the voice and particle hooks). The port had one copy
  of the engine code calling the stubs, so in the arrival cinematic one draw
  callback a frame piled up, 64 of them wrote their quads past the 1.1 MB
  display list into the cutscene's next bank, and the loader decompressed
  garbage. hostgen's `level_hooks` key and `openrac_hook_in_copy` read the
  level copy's own call. Found with `OPENRAC_TRACE_CD` (new: each disc read)
  and a guest write-protect of the bank, not by guessing.
- Level 4: `fix(hostgen): a local vector or matrix starts on a quadword in its
  frame`. The crash handler's "host address (nil)" was a general protection
  fault of a `movaps` on a misaligned frame local.
- Functional C in `rac1-decomp`: `func_L12_002C0310` (the near miss had two
  bugs: a distance stored where retail stores the moby's z, and a constant one
  ulp off), `func_L04_0029FEC8` (written from the instructions, 1,476 bytes),
  and the near miss `func_L00_00286068` accepted. All three are only believed
  correct: levels 4 and 12 have not been played far enough to exercise them.
- Backfill since (functional C by workers, each checked with `tools/equiv.py`,
  none yet exercised by a level run): func_L07_0030C3E0, func_L10_00299D18,
  func_L13_002C3840, func_L04_002CB800 (near misses repaired: a wrong global,
  swapped arguments, a wrong state byte), func_L01_002E1FE0, func_L12_002E8640,
  func_L12_002EB2E8, func_L01_002EF078, func_L00_002A5158, func_L00_0025EFC0,
  func_L03_002ED138, func_L11_00317598, and the near misses func_L00_00286068,
  func_L18_002EAB58. Also `fix(rac1-pal): register the two split variants of
  func_L00_0025D390` (code-address stops of levels 1, 5, 6, 12, 18).
- Next blocker of each level (trap on, 3,000 frames, direct start; 0 and 17
  exit 0): 1 and 11 func_L01_002F0040; 2 func_L02_002D5250; 3 func_L03_002D44A0;
  4 func_L01_002F6540; 5 func_L05_002D28D0; 6 func_L06_002E9AA8; 7
  func_L07_0030E370; 8 func_L08_002D55E0; 9 func_L09_002EEF80; 10
  func_L10_002BFBB8; 12 func_L01_0026EFB8; 13 func_L07_0031C2B0; 14 and 15
  func_L14_002B2A28; 16 func_L00_002CABC0; 18 func_L18_002F9D48.
- Open and next: those blockers, in that order; level 17's hero freeze.

### Every level past 3,000 ticks, 2026-10-14

All 19 levels now run 3,000 scripted ticks (`OPENRAC_DIRECT`, `--keep-going`) with no crash, hang
or call to an unknown code address. The fixes, each found by running a level:

- `func_00216270`'s 25 other places (wrappers on other functions and data) split out
  (`split_places.tsv`): the checkpoint restore crashed levels 2, 7, 9, 13 and 14.
- hostgen keeps a typedef's `aligned` attribute and aligns typedef'd vector locals: misaligned
  `movaps` in levels 1 and 4.
- The game's code runs rounding toward zero, as the EE's FPU does: a scale stepped by 0.1 went
  negative under round-to-nearest (level 6's grid assertion, level 11's stray calls).
- `func_0020E3D0` reads a key's infinite rate as the EE does (0 * rate = 0, not NaN): level 9 hung.
- Copies of one function in one program use their own globals (`openrac_data_in_copy`) and are
  called through the caller's own `jal`; a function's own place is found when C calls it directly;
  register moves are followed when pairing addresses: stray calls in levels 1, 11 and 13.

Still open: functions without C met in levels 2, 4-8, 10 and 13-16 (1 to 4 each), and none of
this is a playthrough. `OPENRAC_GUARD` (BUGFIX_GUIDE) finds who writes a corrupted value.

### Every level's start without a missing function, 2026-10-14 continuation

Workers per `games/rac1/pal/docs/AGENT_WORKFLOW.md` (Opus on functions over 1 KB through
`wave.py long`, one function each; Sonnet and Haiku on small ones), each told to end at
`equiv.py` FUNCTIONAL when not EXACT. In `rac1-decomp` (branch `feat/port-backfill`):

- EXACT, landed: func_L13_00309330, func_L06_002E9AA8, func_L06_002F91A8, func_L06_002FAE58,
  func_L10_002E1568, func_L00_002CABC0, func_L02_002D5250, func_L13_002BB3E8.
- Functional (near misses, `equiv.py` FUNCTIONAL or every flagged item read against retail):
  func_L07_003109D0, func_L14_002B2A28, func_L15_002C6950, func_L10_002E5E48, func_L05_003033A0,
  func_L05_00305778, func_002284E8, func_00220690.
- `fix(nonmatching): func_L13_002EE590 without alloca` and `feat(hostgen): a variable-length
  array whose length is a local` (func_L10_002CAD18 was a stub).

All 19 levels run 3,000 scripted ticks with no crash; the only functions without C they meet are
func_L02_00250A58 (levels 2 and 5) and func_L00_00254218 (level 4). Both are handwritten: the
first builds a VU1 packet, which the renderer does not draw (it takes GIF and VIF DIRECT only),
the second is the joint pose the port replaces. Exercised by a run: class 28 (Oltanis robots,
func_L14_002B2A28) chase, attack, die to the wrench and drop bolts in levels 14 and 15; class 233
(func_L15_002C6950) was only seen in its dormant state. Everything else is believed correct.

Bugs: none found yet in the new functions by running them. Found on the way: a struct passed by
value under another type name breaks the port build (fixed by a port copy of the candidate in
`nonmatching/functional/`), and a worker found a wrong argument order in its own draft by
matching. Not worth workers: the 29 small level entries the planner admitted were all fragments,
VU0 or trap code; libmpeg and the movie player are SDK code the port does not need.

Open: a playthrough; the frontier past each level's start (deeper scripted routes);
func_L08_002F7318 and func_L12_00304038 are stubs (VLA lengths that are expressions).
