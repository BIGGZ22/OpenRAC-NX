# Save states

**The port has no save states today.** This document says what exists, what a
real one would have to capture, and why it is more feasible here than it
sounds — and where the hard parts are.

## 1. What exists, and what it is not

It is important to be precise about this, because three different things are
easy to confuse.

| Thing | What it is | Where |
|---|---|---|
| **The memory card** | The game's own save data: bolts, inventory, per-level flags, in the retail `save%d.bin` chunk format with a CRC-16. On the port a card is **a folder of ordinary files** on disk. | `port/game/common/lib/mc.c`; the format in `port/assets/disc/save_game.h` |
| **The game's checkpoint** | The level's own checkpoint block, copied in and out within game memory. Entirely inside the guest RAM. | `port/game/rac1-pal/hand/level_checkpoint_restore.c` |
| **A RAM dump** | The game's 32 MB of main memory written to a file. `OPENRAC_DUMP_RAM=FRAME` writes `ram_<frame>.bin`. The viewer can **read one back**. | `port/game/common/frontend.cpp:2320-2329`; `port/viewer/game_state.cpp` |
| **A save state** | A snapshot of the whole running machine that can be restored into a live game. | **Does not exist.** |

The RAM dump is the interesting one: **half of a save-state system already
works.** The port can write the game's memory to a file, and it can read such
a file back. What is missing is the other half — writing memory *into* a
running game.

## 2. Why a save state is unusually feasible here

This is a **native port**, not an emulator. That changes the problem
completely.

### 2.1 The game state *is* the memory

The port keeps **one 4 GB host reservation whose offsets are the PlayStation
2's own 32-bit addresses**. A pointer in the game is a `uint32_t` offset into
it — a `gaddr` (`port/runtime/include/openrac/guest.h:32`, and the header's
own preamble at `:5-12`).

| Region | Address | Size |
|---|---|---|
| `GUEST_RAM` | `0x00000000` | 32 MB — the globals, `.data`, `.bss`, the heap, the moby list, the display lists |
| `GUEST_RAM_UNCACHED` / `_ACCEL` | `0x20000000` / `0x30000000` | aliases of the same pages |
| `GUEST_EE_REGS` | `0x10000000` | 64 KB — the EE's registers, including the DMA channel blocks |
| `GUEST_VU_MEM` | `0x11000000` | 64 KB |
| `GUEST_GS_REGS` | `0x12000000` | 8 KB |
| `GUEST_SCRATCHPAD` | `0x70000000` | 16 KB — the DMEM |
| `GUEST_PORT` | `0xE0000000` | 16 MB — the game's stack (the first MB), the string literals, the statics |

— `port/runtime/include/openrac/guest.h:48-67`

So "the CPU state" an emulator has to serialise does not exist here: the
game's own C *is* the host code, and its variables live in that reservation.
**There are no host pointers inside the guest image.** The guest API is pure
offset arithmetic (`guest.h:86-90`), and hostgen lowers every pointer in the
translated code to a `gaddr`.

### 2.2 The game is single-threaded

`CreateThread` returns an id and the thread never runs
(`port/game/common/lib/kernel.c:144-182`, and the comment at `:5-9`). So there
is no thread state to serialise, and no race to freeze.

### 2.3 What has to be captured

| Item | Size | Source |
|---|---|---|
| EE main RAM | 32 MB | `GUEST_RAM` |
| Scratchpad | 16 KB | `GUEST_SCRATCHPAD` |
| EE registers | 64 KB | `GUEST_EE_REGS` |
| GS registers + VU memory | 72 KB | `GUEST_GS_REGS`, `GUEST_VU_MEM` |
| The port's region (stack + strings + statics) | 16 MB | `GUEST_PORT` |
| `openrac_guest_sp` — the stack pointer | 4 bytes | `port/runtime/guest.cpp:35` |
| The tick counters (`g_frame`, the window's `g->index`) | 16 bytes | `main.cpp:133`, `frontend.cpp:128` |

≈ **48 MB raw**, of which the meaningful part is the 32 MB of RAM. `Memory::clear()`
(`port/runtime/memory.cpp:301-307`) enumerates exactly the regions that must
be restored, and `Memory::bytes()` / `Memory::mapped()` give the bounds.

## 3. The four hard parts

The game's state is easy. These are not.

### 3.1 The overlay mirror — must fix, one line

`openrac_guest_function()` asks the host which overlay is loaded **on every
call** (`port/runtime/guest.cpp:78-81`), and the host answers from a file-scope
static:

```c
static int loaded_level = OPENRAC_OVERLAY_EXE;
...
loaded_level = GREF(int, CURRENT_LEVEL);
```

— `port/game/rac1/host/boot.c:13`, `:60`, and the accessor at `:76-78`

After a restore, the guest RAM holds the correct level word (`0x0015EE84`) but
the host's mirror is stale — so **every call through a code address would
resolve to the wrong function**.

**Fix:** re-derive `loaded_level` from the restored word on load, or have
`openrac_game_loaded_overlay()` read the guest RAM directly. One line either
way. This is the difference between "the save state works" and "the save state
corrupts the game".

### 3.2 The string and static allocators — the cold-start hazard

`openrac_guest_static()` and `openrac_guest_string()` hand out addresses from
`GUEST_STATICS` and `GUEST_STRINGS` **on first use, in call order**, and
cache them in per-call-site statics:

```cpp
gaddr g_strings_next = GUEST_STRINGS;          // guest.cpp:49
std::unordered_map<std::string, gaddr> g_strings;  // guest.cpp:48
...
static gaddr next = GUEST_STATICS;              // guest.cpp:176
```

The translated game code caches them too: hostgen emits a per-call-site
`static gaddr` (`port/tools/hostgen/lower.py:704-708`), and `GSTR` caches in
`guest.h:140-147`.

- **A warm restore** (same process, load the state you saved here) works: the
  cached addresses stay valid and the restored region contents are correct.
- **A cold restore** (a fresh process loads the state) does **not**: the
  first-use order differs, so the new process hands out *different* addresses,
  while the restored memory contains *pointers* to the snapshot's addresses.

**Fix:** carry the allocator state in the snapshot and re-seed it on load —
the strings map, `g_strings_next`, and the statics high-water mark. These are
file-local statics inside `guest.cpp` with no accessor today, so this needs a
small new API.

### 3.3 The renderer

Most of the world's content is **derived from guest RAM every frame**
(`frontend.cpp:2227`, `viewer/game_state.cpp`), so a restored RAM gives a
correct world. But four pieces of renderer state are not re-derivable:

- the **texture pool** — the port's model of the GS's memory: uploads by base
  pointer, loaded CLUTs, placements, converted textures
  (`port/renderer/texture_pool.h:182-194`);
- the **GIF/2D drawing registers**, which carry across frames
  (`port/renderer/direct.h:314-341`);
- the **frame snapshot** the game reads back (the pause menu's picture over
  the frozen frame) (`direct.h:146-150`, `frontend.cpp:1326-1398`);
- the frontend's **interpolation cache** and which level's geometry is
  uploaded (`frontend.cpp:74-109`, `:127-131`).

**Pragmatic fix:** on restore, rebuild the renderer from the level
(`frontend.cpp:641-750`) and replay the current frame's display list once to
re-seed the pool and the registers. Accept a one-frame visual discontinuity.

**Rigorous fix:** a serialiser for the pool and the interpreter. More work,
and not needed for practice.

The GL objects themselves (VAOs, VBOs, shaders, FBOs, textures) are **not**
serialised and must not be: they are rebuildable from the pool and the level.

### 3.4 The audio

`SoundPlayer` (banks, grain scripts, voices, master volumes) and the VAG
streams (music and speech) are entirely host-side and have no serialisation
accessors today:

- `port/audio/sound_player.h:131-143` — the voice and bank state;
- `port/game/common/lib/sound_engine.cpp:84-100` — `Stream::position` is a
  **sample offset** into the decoded track, which is the only faithful way to
  resume music mid-track.

**Pragmatic fix:** stop all sounds and restart the streams. The game's own
idea of "what is playing" is asked of the engine
(`openrac_sound_playing`, `port/game/common/include/openrac/sound_engine.h:35`), so the
game stays consistent; the music restarts from the top, which is audible but
not state-breaking.

**Exact fix:** serialise `SoundPlayer` and `Stream`, and take the engine's
lock (`sound_engine.cpp:31`) while restoring so the audio thread cannot render
mid-restore.

### 3.5 Also worth knowing

| | |
|---|---|
| **The memory card's open handles** | Up to 8 `FILE*`s and the per-port current directory (`port/game/common/lib/mc.c:136`, `:79`). The *files* survive on disk; the handles do not. Restoring the handle table is easy but needs new accessors. |
| **The IOP "stash"** | A 2 MB host array *outside* the 4 GB reservation holding what the game parked there via `sceSifSetDma` (`port/game/common/lib/kernel.c:242-243`). Small in practice; lost on restore. |
| **The wall clock** | `g_next_frame` and the frontend's `Pacing` must be rebased to "now", or the first tick after a load is due in the past. |
| **Determinism** | The port is not bit-deterministic by default, because the audio device pulls at the real clock's rate. `OPENRAC_AUDIO_TICKED=1` makes the sounds advance one tick's samples per tick, so every answer the game gets lands on the same tick (`port/game/common/main.cpp:477-490`). **A deterministic save-state system wants that flag on.** |
| **The floating-point environment** | Thread state, set at start (`port/runtime/ee.cpp:31-46`, `main.cpp:1392`). A warm restore keeps it; a cold one must set it again. |

## 4. Where to save and restore

The natural boundary is **the tick**.

`openrac_game_vsync()` is where the game's `sceGsSyncV` lands
(`port/game/common/lib/graph.c:66-71` → `port/game/common/main.cpp:651-809`).
At that instant the game's logic for the tick is finished and the frame has
been drawn from the guest RAM. It is the natural save point.

There is a second path: a game that never waits for vsync still ends its frame
by sending the display list, and the port manufactures the vertical blank
there (`main.cpp:811-824`). A save point must cover both.

**One subtlety worth getting right.** `sceGsSyncV` calls
`openrac_game_vsync()` and **then** `openrac_game_run_vsync_handlers()`
(`graph.c:68-70` → `kernel.c:54-63`). A snapshot taken *inside*
`openrac_game_vsync` is therefore "after the tick's game logic, **before** the
vertical-blank interrupt handlers". Restoring it and letting the game continue
re-runs those handlers against the restored state — consistent, but the frame
that was drawn at that moment was drawn from the pre-vblank state. For a
strictly post-vblank snapshot, the hook belongs at the end of
`openrac_game_run_vsync_handlers`, or as a new call right after
`graph.c:69-70`.

The alternative is the debug overlay's pause: `debug_overlay::holding()` and
`frontend::hold()` (`port/game/common/debug_overlay.h:85`,
`port/game/common/frontend.cpp:2959-2988`) stop the game inside the tick loop
and keep the window live. That is the natural **UI** place for the save-state
keys, next to F5 (pause) and F6 (step).

## 5. A design

### The file

```
openrac-savestate-<game>-<version>.state
```

A header followed by the regions. Roughly:

```c
struct SaveStateHeader {
    char     magic[8];        // "OPENRACS"
    uint32_t version;         // the format's version
    uint32_t game;            // "rac1"
    uint32_t disc;            // "pal" | "ntsc"
    uint32_t tick;            // the game's tick counter
    uint32_t window_index;    // the window's frame counter
    uint32_t guest_sp;        // openrac_guest_sp
    uint32_t loaded_level;    // the host's overlay mirror
    uint32_t region_count;
    /* then: the region table, the allocator state, the regions */
};
```

The allocator state — the strings map, `g_strings_next`, the statics
high-water mark — is what makes a **cold** restore possible. It is small (a
few hundred entries at most) and it is the difference between a save state
that only works in the process that made it and one that works anywhere.

### The keybindings

The convention in the emulator world is F1/F2/F3 (PCSX2: save, cycle slot,
load). For the port, F5/F6 are already pause and step, so the natural choice
is:

| Key | |
|---|---|
| **F10** | Save state (to the current slot) |
| **F11** | Cycle the slot — but F11 is full screen today |
| **F12** | Screenshot — taken |

So either a new pair (F9/F10 are volume) or a modifier. **Shift+F5** to save,
**Shift+F6** to load, **Shift+F7** to cycle the slot, keeps every existing
binding intact and is easy to remember next to pause and step.

Whatever is chosen, the port should show the state in the overlay's Game
section, the way a PCSX2 run shows the slot.

### Where it should live in the code

| Piece | Where |
|---|---|
| The dump/restore of the regions | `port/runtime/`, next to `Memory::clear()` — it knows the region list |
| The allocator state | `port/runtime/guest.cpp` — it owns the strings map and the statics counter |
| The hook into the tick | `port/game/common/main.cpp`, in `openrac_game_vsync()` |
| The keys and the slot list | `port/game/common/debug_overlay.cpp`, next to the pause and step handling |
| The overlay display | the overlay's Game section |
| The tests | `port/tests/runtime_test.cpp` — dump, clear, restore, compare |

### A note on the "no save states during a run" rule

This matters, and it is not hypothetical.

speedrun.com's site rules say: *"Tool-assisted emulator features such as
save-states, re-recording, or frame advance are not allowed"* during runs, and
*"Segmented runs (runs performed in multiple sittings, often with each segment
retried many times until it is made as optimal as possible) are most often
disallowed."*

So **save states are a practice tool**, not a run tool. A run assembled from
saved segments is a segmented run. See [RULES.md](RULES.md).

The port's own tooling already takes this seriously in the other direction:
the regression harness runs with `OPENRAC_UNCAPPED=1` and
`OPENRAC_AUDIO_TICKED=1` so two runs of one build are the same tick for tick
(`docs/port/REGRESSION.md`, "Repeatable runs") — that is determinism for
*testing*, not a licence to run.

## 6. Verdict

**A save state is very feasible, and the port is unusually well placed for
one** — the game state is one 32 MB array the port already dumps and already
knows how to read back.

| | |
|---|---|
| **Trivial** | Dumping and restoring the regions; the stack pointer; the tick counters; the overlay mirror (one line). |
| **Easy** | A warm restore (same process). Rebasing the wall clock. Taking the memory-card handles. |
| **Medium** | The renderer's fidelity (the pool and the interpreter); the audio's fidelity (the voices and the streams). Both degrade rather than break. |
| **Hard** | A cold restore, which needs the string and static allocator state carried in the snapshot. |

The pieces that make it cheap are already in place: the 4 GB reservation with
game addresses, the region map in `guest.h`, `Memory::clear()` as the model
of what to restore, the RAM dump as the format, and the viewer's reader for
it.

## 7. Related reading

| | |
|---|---|
| The runtime's memory model | `port/runtime/include/openrac/guest.h`, `port/runtime/memory.cpp` |
| The executable loader | `port/runtime/elf.cpp` — `openrac_guest_restore_executable` (`elf.cpp:133-147`) restores the executable's image, which is a small cousin of this problem |
| The overlay's pause and step | `port/game/common/debug_overlay.cpp:2076-2097` |
| The dump and the viewer | `port/game/common/frontend.cpp:2320-2329`, `port/viewer/game_state.cpp` |
| How emulators do it | PCSX2's `SaveState.cpp` enumerates what a PS2 snapshot must hold: the CPU registers, the EE RAM, the GS, the VU, the GIF, the IOP, the file handles |
| What a savestate *is* | higan's write-up: <https://higan.readthedocs.io/en/stable/concepts/save-states> — the best short explanation of the idea |
| The rules | [RULES.md](RULES.md) |
