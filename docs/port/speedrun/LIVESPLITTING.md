# Live splitting a run

Splitting a run on OpenRAC works today. This is how to set it up, and how to
extend it when the default (one split a planet) is not enough.

## 1. What exists

The `live-splits` mod, in [OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods):

> Splits a LiveSplit timer as the player goes from planet to planet: the mod
> talks to LiveSplit's own server and splits at every planet, so a speedrunner
> does not have to touch the timer.

It is a **native mod** (a shared library the game loads at start, built
against OpenRAC's mod SDK). It works on both the PAL and NTSC-U discs.

What it does, from its own source:

| | |
|---|---|
| **Reads the game through exactly two things** | The `Level_LoadGameplay` hook point (`rac1.level.load_gameplay`) — the level's gameplay file is loaded, so a planet is reached — and `openrac_api.loaded_level()`, which is `-1` in the galaxy map, so the planet before it was left. |
| **Changes nothing** | No hook point of the game's is replaced. |
| **Sends four commands** | `reset`, `starttimer`, `split`, `setgametime`. |
| **Keeps the network off the game's thread** | The socket has a thread of its own; the game's thread only pushes commands into a queue. |
| **Does not split on death** | The game loads a level's gameplay again when Ratchet dies. The mod keeps the planet it last saw, so the timer runs on. |
| **Resets the run when the player goes back to a planet** | Going back to a planet the run has already been to resets the timer and starts it again — that is what happens when the player loads a save or begins a new game. |

## 2. Setting it up

### 2.1 Install the mod

In the OpenRAC launcher's **Mods** tab: **Add from a link**, and paste
`https://github.com/OpenRAC/rac1-mods`. The launcher lists the mods, checks
the download against the release's `SHA256SUMS`, and installs the build for
your platform. Switch `live-splits` on, choose its settings, and play.

The launcher has a **Purist mode** in its settings that starts the game with
no mods at all. A run submitted for verification should be made in Purist mode
(see [RULES.md](RULES.md)).

### 2.2 Start LiveSplit's server

In LiveSplit: **Settings → LiveSplit Server group → Startup Behavior →
Start TCP Server** (or **Start Websocket Server**), and set the **Server
Port** to the one the mod is looking for.

> **The default port is 27500, not LiveSplit's own default.** The mod's
> `port` setting defaults to `27500`
> (`rac1-mods/live-splits/openrac-mod.json:44-51`). LiveSplit's built-in
> server reads its port from `Settings.ServerPort`
> (`LiveSplit.Core/Options/Settings.cs:24`) and the deprecated standalone
> server component defaulted to `16834`
> (<https://github.com/LiveSplit/LiveSplit.Server>). **Check what your
> LiveSplit shows and either change LiveSplit's port or change the mod's
> `port` setting to match.** This is the single most common setup mistake.

### 2.3 Load a splits file

One segment a planet, in the game's own order. The mod splits at every
planet whatever the category — Any%, 100% and All Gold Bolts all go through
the nineteen planets, so the segments are the same.

| Level | Planet | | Level | Planet |
|---|---|---|---|---|
| 0 | Veldin | | 10 | Orxon |
| 1 | Novalis | | 11 | Pokitaru |
| 2 | Aridia | | 12 | Hoven |
| 3 | Kerwan | | 13 | Gemlik Base |
| 4 | Eudora | | 14 | Oltanis |
| 5 | Rilgar | | 15 | Quartu |
| 6 | Blarg Station | | 16 | Kalebo III |
| 7 | Umbris | | 17 | Drek's Fleet |
| 8 | Batalia | | 18 | Veldin (final) |
| 9 | Gaspar | | | |

The names are the port's own (`port/game/rac1/host/front_end.c:12-16`), and the
mod hard-codes the same list (`rac1-mods/live-splits/src/levels.hpp:11-15`).

### 2.4 Run

Start LiveSplit's server **before** the game. The mod keeps the commands it
is given while the server is not there and sends them when it comes, but a
timer that starts late is a timer that starts late.

With the default settings, the mod resets LiveSplit's timer and starts it when
the run's first planet is loaded, and splits at every planet after that. The
run ends when the game goes back to the galaxy map after Veldin, the last
planet, where the last boss is.

## 3. The mod's settings

`rac1-mods/live-splits/openrac-mod.json:13-71`:

| Setting | Default | |
|---|---|---|
| `split` | `reached` | **When a planet is reached**: the split is taken when the planet's level is loaded, so the flight to it and its loading are in the segment before. **When a planet is left**: the split is taken when the game goes back to the galaxy map, so they are in the segment after. |
| `start` | `first` | **At the first planet**: the mod resets LiveSplit's timer and starts it. **By hand**: the mod never resets or starts the timer; you start it yourself. |
| `game_time` | on | Sends the game's own time (its ticks) to LiveSplit, for the Game Time timing method. With the timer started by hand, the time is counted from the first planet. |
| `port` | `27500` | The port LiveSplit's server listens on. |
| `protocol` | `auto` | **Ask the server**: the mod tries TCP and, when that is not there, WebSocket. **TCP** or **WebSocket** pick one. |
| `trace` | off | For bug reports: a line in the game's log for every planet reached and left, and for the timer the mod starts. |

Turn `trace` on the first time you run it. It is the fastest way to see
whether the mod and the timer agree.

## 4. How the timing works

The mod sends `setgametime` with the game's own time: its tick count times the
game's tick length, read from the disc's own memory
(`D_0015EE7C` on PAL, `D_0015ED7C` on NTSC-U). It refreshes every 30 ticks
(`rac1-mods/live-splits/src/live_splits.cpp:179-184`).

Because the port runs the game at 60 Hz on both discs, game time and real time
are the same number here — see [TIMING.md](TIMING.md) §4.

**Which timing method should LiveSplit use?** Real Time and Game Time are
equivalent in the port. Use Game Time if you want the timer to survive a
pause or a reload; use Real Time if you want the simplest thing.

## 5. The LiveSplit server

The mod speaks LiveSplit's own line protocol, over TCP or over a WebSocket
(`/livesplit`). The commands it uses are a small part of what the server
offers.

The server's own implementation
(<https://github.com/LiveSplit/LiveSplit/blob/master/src/LiveSplit.Core/Server/CommandServer.cs>)
answers a command with a line of text, and the commands that matter for a
speedrun tool are:

| Command | |
|---|---|
| `reset` | Reset the timer |
| `starttimer` | Start it |
| `split` | Split |
| `setgametime <time>` | Set the game time (`M:SS.cc`) |
| `pausegametime` / `unpausegametime` / `alwayspausegametime` | Pause the game time — this is how "load removed" is implemented |
| `switchto gametime` / `switchto realtime` | Change the timing method |
| `getcurrentgametime`, `getcurrentrealtime`, `getfinaltime`, `getdelta` | Read the timer back |
| `setcustomvariable` | Write a custom variable into the run's metadata — useful for a segment or a route note |

The full list is 78 commands; the source above is the authority.

Two practical notes: the server's WebSocket service lives at the path
`/livesplit`, and the mod's "ask the server" mode works by sending `ping` and
reading the answer — a TCP server answers `pong`, a WebSocket server answers
an HTTP error, which is how the mod tells them apart
(`rac1-mods/live-splits/src/client.cpp:283-316`).

## 6. Writing your own splitter

The mod SDK gives a mod everything it needs to build a better splitter.
`sdk/include/openrac/openrac_mod.h` is the whole interface; the C++ wrapper
is `sdk/include/openrac/mod.hpp`.

### The API a splitter uses

| Symbol | What it gives you |
|---|---|
| `api->memory` | The base of the game's RAM. `memory + a` is the game address `a`. **Full read and write access to the game's state.** |
| `api->on_frame(fn, user)` | A callback after each game tick, in load order. This is where a timer ticks. |
| `api->on_pad(fn, user)` | A callback for each pad report, **before the game reads it**; the mod may rewrite the report. |
| `api->hook(name, signature, replacement, original)` | Take a hook point. The signature must match exactly or the load is refused. |
| `api->loaded_level()` | The level whose program is loaded, `-1` for none (the galaxy map / boot program). |
| `api->relocate_data(address)` | A global's address **for the code loaded now**. Ask every time — a level moves its globals. |
| `api->relocate_level_data(level, address)` | The same for a given level's program. |
| `api->function(name)` | A game function by its name in this disc's build. |
| `api->input()` | The keyboard and mouse for the tick about to run. |
| `api->setting<T>(key)` | A setting from the manifest. |
| `api->log` / `api->fail` | Write to the session log; refuse the load with a reason. |

### Hook points that make good split triggers

rac1 has **654 hook points** on both discs (`port/game/rac1-pal/libraries.tsv`,
654 rows with `hook` in the fourth column). The 17 hand-written ones:

| Hook | Id | Fires |
|---|---|---|
| `Level_LoadGameplay` | `rac1.level.load_gameplay` | Once per level start, as the gameplay file is loaded into the level heap. **The planet-is-reached trigger.** |
| `Vendor_InitLevelStock` | `rac1.vendor.init_level_stock` | At each level start — an independent level-start signal. |
| `Item_Give` | `rac1.item.give` | Every item the game gives (but the Persuader's). |
| `Planet_Unlock` | `rac1.planet.unlock` | A planet's coordinates are unlocked. |
| `GoldBolts_CountCollected` | `rac1.gold_bolt.count_collected` | The gold-bolt count (at most 40). |
| `Hero_TakeDamage` | `rac1.player.take_damage` | Every hit the hero takes but the instant kills. |
| `Moby_RecordDeath` | `rac1.moby.record_death` | A placed moby dies or a crate breaks. |
| `Hero_AdvanceAnimation` | `rac1.player.advance_animation` | Ratchet's animation advance, once a tick after his state update. |
| `Pad_ProcessReport` | `rac1.pad.process_report` | Each pad poll, before the game reads the pad. |

Plus **637 behaviour-catalogue hook points** — one per moby class update, e.g.
`rac1.moby.checkpoint_trigger.update` (class 805, the level's respawn
points — a natural in-level split), `rac1.moby.gold_bolt.update`,
`rac1.moby.infobot.update`, `rac1.moby.teleporter_pad.update`.

The full list with signatures and parameters is generated into
[`sdk/docs/reference/hooks-rac1.md`](../../../sdk/docs/reference/hooks-rac1.md).

### A minimal skeleton

```cpp
#include "openrac/games/rac1.hpp"
#include "openrac/mod.hpp"

namespace {
int g_last_level = -1;
}

void load(openrac::Mod& mod) {
    if (mod.game() != "rac1") {
        throw openrac::Error("this mod is for Ratchet & Clank");
    }

    // A planet is reached.
    mod.hook<openrac::rac1::Level_LoadGameplay>([](auto original, std::int32_t full) {
        // ... your split logic here ...
        return original(full);
    });

    // The game is in the galaxy map.
    mod.on_frame([&mod]() {
        const int level = mod.loaded_level();
        if (level != g_last_level) {
            // ... left the planet, or reached one ...
            g_last_level = level;
        }
    });
}

OPENRAC_MOD_CPP("my-splitter", "1.0.0", load)
```

The manifest is a JSON file, `openrac-mod.json`, in the mod's folder. The
format is documented in [MODS.md](../MODS.md) §"The manifest", and
`rac1-mods/live-splits/openrac-mod.json` is a complete, working example with
every setting type.

### Script mods

A **script mod** is a `.luau` file the game runs in a sandbox — nothing to
build, the same files on every platform. The sandbox exposes
`openrac.hook`, `openrac.on_frame`, `openrac.on_pad`, `openrac.read`,
`openrac.write`, `openrac.fields`, and more
(`sdk/docs/reference/luau-api.md`).

Two limits to know:

- **A 2 ms budget per call**, by default, overridable with
  `OPENRAC_SCRIPT_BUDGET_MS` (`port/game/common/mod_scripts.cpp:37-45`). A
  script that overruns it is stopped, and the port says which hook it died
  in.
- **No file IO, no sockets.** A script mod cannot talk to LiveSplit. A
  *native* mod can: it is an ordinary program. That is why `live-splits` is
  native.

## 7. What a splitter cannot do today

| Wanted | Why not |
|---|---|
| **Draw a split overlay in the game** | There is no drawing API for mods. The SDK has no text, no quad and no renderer hook; `MODS.md` lists a renderer hook as *proposed*. Use LiveSplit's own overlay, or an external one. |
| **Split on a specific in-level event without a hook point** | Read the game's memory instead. `api->memory` gives full access, and `api->relocate_data(address)` gives a global's address for the level loaded now. |
| **Split on anything in rac2, rac3 or rac4** | Those ports have **zero** hook points and no script binding, so a mod for them is refused with "this OpenRAC runs no hook point … for rac2". |
| **Run as a Luau script and talk to LiveSplit** | The sandbox has no sockets and no file IO. |
| **Ship an .asl for LiveSplit** | The official autosplitter index contains no Ratchet & Clank 1 entry at all — not for PS2, PS3 or Vita. The only Ratchet entry is *Rift Apart*. A port-specific autosplitter would be new work; the mod approach is better here because the port can emit the events itself. |
