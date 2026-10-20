# Deadlocked NTSC-U native startup

The real main menu renders and responds to controller input in the native
NTSC-U windowed target. The strict 1,800-frame test on 2026-10-20 completed
with exit code zero and no missing calls. The menu's 3D background and sound
remain incomplete; reaching the menu does not establish playable gameplay.
Intermittent menu drawing artifacts, including omitted unselected rows, also
remain visible.

## Verified menu run

Local evidence is in ignored `build/rac4-diagnostic-20261020/run-wide-glyphs/`:
`status.json`, `run.log`, `frames.jsonl` and three screenshots. Frame 1000
shows Single Player selected; frame 1500 shows Cooperative Play selected
after the Down press at frame 1400. Both images were visually inspected.
The frame log contains all 1,800 complete rows (frames 0 through 1799).

The run uses `OPENRAC_STRICT=1`, `OPENRAC_UNCAPPED=1`,
`OPENRAC_AUDIO_TICKED=1`, and the scripted input
`OPENRAC_PRESS=300:8:10,600:8:10,900:8:10,1200:8:10,1400:40:5,1600:10:5`.
From the repository root, with the local data installation available:

```text
build/rac4-window/openrac-rac4-ntsc.exe --data build/rac4-data --window --frames 1800 --no-card --skip-movies --stop-on-missing
```

`OPENRAC_SHOT=500:<output-prefix>` and `OPENRAC_SHOT_FROM=500` capture
frames 500, 1000 and 1500. `--debug-dump <path>` writes the frame metrics.
The hostgen suite passes on Windows: 108 tests, 59 skipped by the existing
platform conditions. The new synthetic test actually compiles and executes
the translated code, checking 64-bit results and arguments across calls.

The current-upstream Windows recheck also completed 1,800 frames with exit
code zero, no missing calls, and the same selection change at frame 1500.
The complete native build passed. CTest passed all 80 executed tests out of
95 registered tests; 15 were skipped by their existing platform/data checks.
This includes the native I/O and synchronous DMA completion contract tests.

## Frontend startup

The executable contains the front-end overlay. Its outer loop at
`0x00157C58` enters `0x00437EB0`, which keeps its own frame loop until the
word at `0x0021EDBC` changes. Forcing the loaded overlay to level zero does
not establish that the front end has started correctly.

## Native boundaries

- The card query at `0x0013A8C0` takes port, slot and three optional output
  pointers. It issues card RPC command 1. Completion at `0x0013A7A0` reports
  the command and result. Both use the shared native card service. Binding
  only `sceMcInit` leaves the retail RPC client uninitialized and the
  startup card state at `0x00171B0C` stuck at zero.
- `sceSifLoadModuleBuffer` at `0x0012CBC0` is a 28-byte wrapper identified
  against `libkernl.a/eeloadfile.o` with only actual relocation fields
  masked. It uses the existing native service boundary rather than the
  retail IOP loader's RPC and lock state.
- The 24 module descriptors at `0x003D7EB8` are 16-byte records: module id,
  pre-load callback, post-load callback, and cleanup callback. Their 14
  unique nonzero targets are explicit hostgen roots. In particular,
  module 5's post-load callback at `0x00460138` calls the controller
  initializer at `0x0048F960`.
- `host/pad.c` serves the older libpad API, including port lifetime,
  digital/analog configuration and the 32-byte read buffer. It reuses the
  shared pad payload conversion and adds libpad's status/id prefix. Only
  slot zero on each host port is available; no motors are advertised.
- The synchronous 989snd command boundary at `0x00158A30` keeps the retail
  caller's EE buffer initialization while replacing its IOP request/wait.
  Command zero uses the native sound-system service. Supported play/parameter
  commands return the native reply; unsupported commands report zero and
  a warning, as the native sound command API specifies.
- The movie entry at `0x004EEEB0` takes sector, byte count, an EE buffer
  allocator, audio channel and console display mode. `host/movie.c` uses
  the shared native player, whose buffers and window need no EE allocator.
  The game keeps its surrounding movie transitions and receives the native
  player's normal-completion/skip result. This also connects `--skip-movies`
  to Deadlocked's movie path instead of entering its IPU polling loop.

## Register-forwarding wrappers

The 35 game files listed in `skip` have matching C definitions that omit
incoming registers or results used by the retail callers. They are translated
from their instructions for the host; the matching source stays unchanged.
For example, `00496B68` forwards one argument to `004970F8`, and `00496C30`
forwards two to `00497EB8`. They do not register their own addresses as callbacks.
`005005E8` forwards a second argument to `00500AE8` and returns its result;
losing this result leaves an unrelated pointer in place of the lookup result
and produces a null method call. `00506B70` forwards its argument to `005040B0`.
`00500D30` also forwards the incoming float register to `00453C68`.

The texture descriptor producers `00448F98`, `004FB4E0` and `00449350` return all 64 bits
of TEX0, including its upper palette fields. Their `returns_64` entries keep
the translated returns and their callers from truncating the descriptor.
`test_catalogue` compiles and runs a synthetic two-function example that
checks both halves of the result and an ordinary 32-bit return.
The glyph builders `0044A410`, `0044A5C8` and `0044A7D0` also take complete
64-bit TEX0 arguments. Their verified types are in `asm.signatures`. A native
hardware watchpoint on the glyph packet traced the truncation to argument 6
of `0044A410`, called from `00454A70`. The synthetic test also passes a wide
result into another translated function and checks its upper half there.

## Current debugging landmarks

The splash/card state is `0x0021E1F0`, its timer is `0x0021E1F4`, and the
startup gate is `0x0021E1EC`. With the native card query and completion
bindings, a strict 1,800-frame test with no inserted card advances the
splash/card state from 1 to 2. That run still has no visible menu and no
host controller reads.

After binding module loading and registering the lifecycle callbacks, a
bounded strict run reaches the actual controller initializer. Without the
native pad boundary it reports a driver-version mismatch. It then reaches
the sound initializer's synchronous RPC and waits for an absent IOP reply;
that is the path the synchronous sound boundary replaces.

With native pad and synchronous sound boundaries, the strict windowed run
reads scripted Start presses and moves the splash/card state through 2,
4, 6 and 7. Its startup gate becomes zero at frame 903. At frame 910 it
then enters the retail movie decoder (`0x004EEEB0` -> `0x004EF078` ->
`0x004F1D20`) and polls the unavailable IPU; the bounded probe is terminated.
The frame-905 capture is still black. With the native movie boundary the
next strict probe actually skips sector 604384 and reaches frame 917. After
fixing the two object wrappers, the null method becomes the genuine indirect
entry `0050E2A8`, which returns 2. Its nine-slot virtual table at
`003D36F8..003D3718` is explicitly rooted, including that entry missing from
the function catalogue. The next missing method, `0050DE38`, belongs to
another frontend virtual table. The seven nine-slot tables based at
`003D35D8`, `003D3610`, `003D3648`, `003D3698`, `003D36F0`, `003D3728` and
`003D3768` have their slots at base + 8. Together with the two-slot helper
at `003D37A0` they contribute 52 unique method roots. The local switch table
at `003D3680..003D3693` is not included.

With these roots the strict test completes all 1,800 frames, including
frontend draws after frame 917, without missing calls. The final glyph
descriptor fixes then produce the visible, navigable main menu documented
above. The earlier black captures remain diagnostic history, not the
current rendering result.

The asset-free `rac4_io` CTest exercises the actual host pad and sound
adapters with mock host devices: closed/open/disconnected ports, packet
boundaries, digital/analog modes, every button bit and pressure channel,
and synchronous command results without an IOP wait. Build `rac4_io_test`
and run `ctest --test-dir <build> --output-on-failure -R '^rac4_io$'`.

Changing a startup state word is not a fix for a missing input service.
