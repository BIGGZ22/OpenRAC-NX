# runtime

What the games' C runs on: game memory (one 4 GB reservation whose offsets are the console's
addresses), the calls translated C makes into it, the EE's registers and operations as that C uses
them, the game's executable copied into memory at its addresses, the fatal faults, and the C entry
point to the port's `OPENRAC_*` variables. It draws nothing, plays nothing and knows no game; what a
game is lives in `game/` and in the code hostgen writes.

Library `openrac_runtime`; the console's float arithmetic for C is the separate library
`openrac_ps2float` (`ps2float.cpp`), which links it. The C contracts are in `include/openrac/`
beside two C++ headers of the runtime's own (section 14.1 of
[CODING_CONVENTIONS.md](../docs/CODING_CONVENTIONS.md)).

## Interface

| Header | Kind | Declares |
|---|---|---|
| [`guest.h`](include/openrac/guest.h) | C contract | game memory as translated C sees it (`gaddr`, `G()`, the game stack, string literals), the table of functions by code address, the overlays |
| [`ee.h`](include/openrac/ee.h), [`ee_ops.h`](include/openrac/ee_ops.h) | C contract | the R5900's instructions, the FPU's divide and roots, the vector unit's registers, and what happens where the console would have trapped |
| [`ps2float.h`](include/openrac/ps2float.h) | C contract | the EE FPU's single-precision arithmetic in software (`openrac_ps2float`) |
| [`host_env.h`](include/openrac/host_env.h) | C contract | `openrac_env_string`: the value of an `OPENRAC_*` variable of `common/env.h`'s table, for C |
| [`memory.h`](include/openrac/memory.h) | C++ | `openrac::runtime::Memory`: the reservation, its map of backed regions, and the crash handler's install |
| [`elf.h`](include/openrac/elf.h) | C++ | `openrac::runtime`: the game's executable read and its segments copied into memory |

Each contract is implemented in the file named for it (section 14.3): `guest.cpp`, `ee.cpp`,
`ps2float.cpp`, `host_env.cpp`. `memory.cpp`, `elf.cpp` and `crash.cpp` (the fault handler that
`memory.h` installs) are the runtime's C++.

### The C entry point to the environment variables

`host_env.h` is how C reads an `OPENRAC_*` variable: C cannot include `common/env.h`, and the port
reads every such variable through that one table (section 16), so C asks by name and gets back
the raw value the table kept, or `NULL` when it is unset. It is the value and nothing more, so each
C site keeps the test it made of its old read: `openrac_env_string("OPENRAC_TRACE_CD") != NULL`
takes `0` as set, as before, even where the table's row would read the flag another way. A name the
table does not have is a programming error: it is logged once as an error and reads as unset, so a
misspelt switch stays off and says so, and the run goes on. A new variable read from C is a row of
the table (and of [DEBUGGING.md](../../docs/port/DEBUGGING.md)) like any other.

## Depends on

`common` (the log, the crash report, the environment table, the float model), the layer below it
(section 18). It uses the system's memory interfaces (`mmap` over `memfd_create` or `shm_open` on
Unix, `VirtualAlloc2` and `MapViewOfFile3` on Windows) and its fault signals or structured exceptions.

## State and threads

- Game memory is one reservation per process (`Memory::get()`), made before the game's code runs
  and kept until the program ends.
- `guest.cpp` keeps the string literals, the table of functions, the list of functions without C
  and the overlays' relocation, each behind its own lock; the address of the last unknown call is
  per thread. The game stack is the game thread's.
- `ee.cpp`'s reports of traps and lost jumps are guarded by a lock.
- `host_env.cpp` holds the names it has reported, behind a lock; the values are `common/env.h`'s,
  read once and kept. Any thread may call it.

## Errors

- `read_elf` and `load_elf` return `false` and say why in their `std::string* reason`; `read_file`
  returns `false`.
- A fault in game memory, an illegal instruction, a trap, an abort or an exception nobody caught
  writes a crash report (`common/crash.h`) and ends the program with status 3 (`crash.cpp`).
- With `OPENRAC_STRICT`, a call to a code address no function of the port has, or a trap, ends the
  program; without it, the call goes to a function that reports it and the trap is reported once,
  and the game goes on. A jump through a register to an address the tables do not list is reported
  once and returns, strict or not.
- `openrac_env_string` has no failure: an unset variable is `NULL`, and so is a name the table does
  not have, which is logged once.

## Configuration

The runtime reads `OPENRAC_STRICT`, `OPENRAC_TRACE_MISSING` and `OPENRAC_TRACE_RELOCATION` through
`common/env.h`, and hands C every variable of that table through `host_env.h`; each is documented
in [docs/port/DEBUGGING.md](../../docs/port/DEBUGGING.md).

## Tests

| ctest | Proves |
|---|---|
| `runtime` | game memory and its aliases, the game stack and string literals from C, the function table, the executable's loading |
| `host_env` | from C: a table variable's raw value, `0` and the empty string as set, `NULL` when unset, one string kept from the first read, and `NULL` for a name the table does not have |
| `env` | the table `host_env` reads through, its lookup by name included |
| `ps2float` | the C entry points over the float model, as hostgen's translated C calls them |

## Further reading

- [docs/port/DESIGN.md](../../docs/port/DESIGN.md): game memory as OpenGOAL's offsets, and why
  every function the port runs is compiled from C.
- [docs/port/HARDWARE_SEMANTICS.md](../../docs/port/HARDWARE_SEMANTICS.md): where the console's
  arithmetic differs from the host's.
- [docs/port/DEBUGGING.md](../../docs/port/DEBUGGING.md): the switches and the crash reports.
