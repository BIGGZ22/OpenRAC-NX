# common

The bottom layer of the port: what every module may use, and nothing that belongs to one of them.
The log, the crash report, the statistics, the environment variables, the system's description, a
JSON reader, the console's float arithmetic, and small helpers. It draws nothing, opens no window
and includes no other module of the port; a helper only one module uses stays in that module.

Library `openrac_common`. Each header has a namespace of its own under `openrac` (the table below);
`joining_thread.h` and `result.h` declare their types in `openrac` itself.

## Interface

| Header | Declares |
|---|---|
| [`log.h`](log.h) | `openrac::log`: the port's log, its levels, the session file and the fatal error |
| [`line_ring.h`](line_ring.h) | `openrac::log::LineRing`: the log's last lines, kept in memory for a crash report |
| [`crash.h`](crash.h) | `openrac::crash`: the crash report, written from a signal handler, and the game's state it names |
| [`env.h`](env.h) | `openrac::env`: the table of every `OPENRAC_*` variable the port reads (its kind, what holds when it is unset, what it does) and their typed reads |
| [`stats.h`](stats.h) | `openrac::stats`: counters, timed sections and frames for the debug overlay, and the environment probes |
| [`system_info.h`](system_info.h) | `openrac::system_info`: the system, processor and memory a report names |
| [`build_info.h`](build_info.h) | `openrac::build_info`: OpenRAC's version and the checkout's commit, written at each build |
| [`json.h`](json.h) | `openrac::json`: a JSON reader (a tree of values, parsed whole) for the editor's exports, mod manifests and the tools' files |
| [`ps2_float.h`](ps2_float.h) | `openrac::ps2`: the PS2's FPU and vector-unit float arithmetic on raw bit patterns, for the asset readers, the world renderers and the runtime's C entry points (`openrac/ps2float.h`) |
| [`float_encoding.h`](float_encoding.h) | `openrac::float_encoding`: IEEE floats packed to the small float formats of half-float metadata and irradiance channels |
| [`joining_thread.h`](joining_thread.h) | `openrac::JoiningThread`: a thread its owner joins when it goes (`std::jthread` without the stop token) |
| [`result.h`](result.h) | `openrac::Result<T>` and `openrac::Error`: what a public function that can fail returns, a value or an error message; the port's own `std::expected` (C++23) under its names, `Result<void>` included |
| [`c_boundary.h`](c_boundary.h) | `openrac::c_boundary`: the wrapper a C entry point written in C++ runs its body through, which stops every exception, reports it and returns the entry point's failure value |

`process.cpp` implements the process's memory and processor time that `stats.h` declares.

## Depends on

Nothing of the port's: `common` is the lowest layer (section 18 of
[CODING_CONVENTIONS.md](../docs/CODING_CONVENTIONS.md)). It uses the C++ standard library and the
system's own interfaces (`dladdr` and the backtrace on Unix, `dbghelp` on Windows, the Android
compatibility library for the backtrace below API 33).

## State and threads

- The log is one per process, guarded by its own lock: any thread may write to it.
- The crash report's buffers are set up once by `crash::configure()`; `crash::report()` then runs in
  a signal handler without allocating or locking. Each thread that runs game code calls
  `crash::prepare_thread()` once.
- The statistics are called from the thread that runs the game and draws, and from no other.
- `env` reads every variable of its table once, the first time any is asked for, and keeps the
  values for the life of the program: any thread may read them. `env::system_variable` reads a
  variable the port does not own at each call.
- `json` and `ps2` hold no state: a parsed `json::Value` belongs to its caller, and the float
  functions are pure.

## Errors

- `result.h` is the form new interfaces across the port report failure in (section 13.3 of
  [CODING_CONVENTIONS.md](../docs/CODING_CONVENTIONS.md)): `Result<T>` holds the value or an
  `Error` whose message follows section 17.3, and `value()` or `error()` on the wrong side throws
  `std::bad_variant_access`, a mistake of the caller's rather than a way to learn of the failure.
- `c_boundary::call` keeps exceptions from crossing into C (section 13.4). It knows no reporter
  above `common`, so the caller names one: the game side passes `openrac_game_crash`
  (`game_host.h`), which writes the crash report; the host side keeps the default, which logs the
  report as an error, and goes on with the failure value. The report reads `<where>: <what>`, or
  `<where>: unknown exception` for a thrown value that is not a `std::exception`, built in a
  fixed buffer so that a `std::bad_alloc` can still be reported.
- `json::parse` and `json::parse_file` return `false` and say where in their `std::string& error`.
- `log::fatal` writes the crash report and ends the program; it is for what the program cannot go
  on from.
- The float arithmetic has no failure: a division by zero gives ±`kMax`, as the console's does.
- `env` has no failure: an unset variable reads as the caller's fallback, and a value that is not a
  number reads as 0, as `std::strtoll` and `std::strtod` read it.

## Configuration

`env.cpp`'s table lists every `OPENRAC_*` variable the port reads, and
[docs/port/DEBUGGING.md](../../docs/port/DEBUGGING.md) documents each one; the `env` test holds the
two to the same list. `runtime/`, `platform/` and `renderer/` (outside `renderer/vulkan/`) read
their variables through `env.h`; `game/`, `renderer/vulkan/`, `android/` and `enhanced/` still call
`getenv` and move onto it later (docs/restructure/PROGRESS.md, P2-09). Within `common`,
`OPENRAC_LOG` sets the log's level (`log.cpp`), and `stats::probe` reads each `OPENRAC_*` switch a
module declares through it.

## Tests

| ctest | Proves |
|---|---|
| `log` | the log's sessions and kept runs, its memory of last lines, and the crash report's pieces |
| `joining_thread` | an owner going, or assigned over, waits for its thread |
| `result` | a value or an error through each accessor, the wrong side throwing, copies and moves (a move-only value included), and `Result<void>` |
| `c_boundary` | a body that returns passes through unreported; a `std::exception` and a non-standard thrown value are reported and the failure value returned; a void body; a long message cut to the buffer; the default and a null reporter write to the log |
| `env` | each kind of variable reads as the `getenv` calls it replaced read it, the values are read once, and DEBUGGING.md documents the table's variables and no other |
| `viewer` | the JSON reader's values and its errors (with the viewer's own checks) |
| `mod_manifest`, `mod_loader`, `mod_scripts` | the mod manifests and status files, read through the JSON reader |
| `assets_world_collision` | the float model against values the console computes, beside the collision queries that use it |
| `geometry_moby_animation`, `geometry_tfrag`, `geometry_shrub_lighting`, ... | the float model in the game's lighting and animation, to the byte |
| `ps2float` | the C entry points over the float model, as hostgen's translated C calls them |

## Further reading

- [docs/port/HARDWARE_SEMANTICS.md](../../docs/port/HARDWARE_SEMANTICS.md): where the console's
  float arithmetic differs from the host's, and when the port reproduces it.
- [docs/port/DEBUGGING.md](../../docs/port/DEBUGGING.md): the log, the crash reports and the
  switches.
