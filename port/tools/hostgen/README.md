# hostgen

hostgen writes a decompilation's C again, as C that runs natively in the
port. It is how the decompiled games become programs: their C is the C
that matched the console's executable, byte for byte, and hostgen keeps its
meaning while moving it to a 64-bit PC.

```sh
python3 port/tools/hostgen/hostgen.py --game port/game/rac1-pal/hostgen.json \
    --source games/rac1 --out build/hostgen/rac1
python3 -m unittest discover -s port/tools/hostgen     # its tests (needs Clang)
```

The port's build runs it ([port/cmake/Games.cmake](../../cmake/Games.cmake));
running it by hand is for reading its report. The decompilation is never
changed: hostgen reads prepared copies under `--out/prep`.

## What it does

Game memory is a 4 GB reservation whose offsets are the console's addresses
([guest.h](../../runtime/include/openrac/guest.h)), the model OpenGOAL uses
for Jak's pointers. hostgen writes the C against it:

| In the decompilation | In hostgen's C |
|---|---|
| a pointer, of any type | a `gaddr`: the 32-bit game address it held on the console |
| `*p`, `p->f`, `p[i]` | an access to game memory: `GREF(T, addr)`, `((S *)G(p))->f` |
| pointer arithmetic | scaled by the pointee's size, which is the console's: every record keeps its layout, since its pointers are 4-byte `gaddr`s too |
| a global (`D_0013E650`, or a named one with its address) | the object at its retail address in game memory |
| a literal address made a pointer (`*(s32 *)0x1610A8`, `((S *)0x160F08)->f`) | the global a name at that address would be, relocated the same way: in a range the levels move (`relocate`), the loaded level's copy, not the executable's under it |
| a local whose address is taken, a local array, struct or union | a slot in a frame on the game stack (`GFRAME`), given back when the function returns; an array or record with no machine mode of its own (a `float v[3]`) gets a slot aligned to 16 and rounded up to 16 bytes, as the console's compiler gave it, since the game writes whole quadwords into such locals |
| a string literal | copied into game memory once (`GSTR`) |
| a direct call | a call to the callee's **definition**, its arguments matched to the definition's parameters as the EE passes them (below) |
| a call through a pointer | `GFN`: the table of functions by code address, per level program |
| a function's address | its code address, as a number |
| `qcopy`, `qcopy_nc`, `qzero` (the decompilation's inline assembly) | the runtime's host versions |

Because addresses keep their meaning, the C's habits work unchanged:
absolute addresses, a global plus a large offset into its neighbours, the
uncached alias of RAM, the scratchpad at `0x70000000`, pointers held in
`int`s.

### Calls

The decompilation's files often declare a callee differently from its
definition (they were written to match machine code, one function at a
time). On the EE that is harmless where the registers line up, so hostgen
does what the EE did:

- Integers (and pointers) and floats travel in separate registers. The
  call site's arguments are split by kind and given to the definition's
  parameters by kind, in order, so `f(float, int)` reaches `f(int, float)`
  correctly.
- A parameter the call does not pass was whatever its register held: often
  the caller's own argument in the same register ("passed through"). hostgen
  passes the caller's parameter in that position, or 0, and counts these
  in its report.
- A result the caller reads but the callee does not return is 0.
- A call by name that goes through a code address (a name the catalogue
  folded, another level's function, a level copy's own callee, a function
  cast to another type) uses the type of the function found there, its
  arguments matched the same way, whenever the call site's declaration
  puts a kind at another position. On the EE, System V and AArch64 that
  order does not matter; on Windows x64 an argument's position decides
  its register, and rac1/ntsc 0.8.0 crashed on Rilgar where a creature
  declared level 0's `scale(float *out, float *a, float len)` as
  `(float, void *, void *)`. [abicheck.py](abicheck.py) reads the generated
  C, `functions.c` and `game_protos.h` and lists every call through a
  known address whose type differs from the function there by position or
  by the kind of its result; the port's ctest runs it for each game
  (`<id>_call_abi`).

### What it does not translate

A function with inline assembly other than `sync` and the quadword
helpers, or another construct hostgen does not write, becomes a stub that
logs that it ran; the report lists each one and why. Functions with no C at
all (still assembly in the decompilation, or libraries) are stubs too,
unless the game's `libraries.tsv` says the port writes them itself, or
their assembly is there to translate (below).

## Functions with no C: C from their assembly

The decompilations keep what they have no C for as assembly files that each
game's setup writes from the player's own executable (splat and
spimdisasm: rac1/pal's `asm/`, the NTSC decomp's `config/us/...`). None of
it is in git. Where it is there and the game's `hostgen.json` turns it on
(`"asm"`, below), [asmc.py](asmc.py) writes C for every stub it has
assembly for, from the instructions, so the port runs the function instead
of a stub. C in the decompilation (matched, a candidate, a draft, the
port's own) always wins: only stubs are translated, and a translation goes
away when its function gets C. Without the assembly or the retail program
(CI has no disc) nothing changes.

### The retail program, where the assembly is not

The assembly files are only on a machine that ran the decompilation's
setup; a player's machine has the disc. So the same instructions are also
read from the retail program itself ([retail.py](retail.py)): the
executable's code sections and every level's program (the records the game
loads over the executable), from the player's disc image or from the
executable and level dumps the decompilation's setup placed. Where both are
there, the files' lines come first (their text has splat's symbols) and the
retail bytes fill the rest, which also lets a translation run on into a
neighbour that has C (a function the catalogue cut short, a shared tail).

| Where it comes from | |
|---|---|
| `hostgen.json` `"asm"` `"retail"` | `{"exe": ..., "dump": ...}` (or `{"image": ...}`), relative to the decompilation the assembly is read from: rac1/pal's `baserom/SCES_509.16` and `baserom/overlays`, the NTSC decomp's `config/us/SCUS_971.99` and `config/us/overlays` |
| `--retail-image ISO`, `--retail-exe ELF` and `--retail-dump DIR` | in place of those |
| `OPENRAC_<ID>_RETAIL_IMAGE` | the build's: the player's disc image. `tools/build_port.py GAME --disc IMAGE` sets it, and the launcher's "Build the game" passes the disc it set up |

What the text says beyond the words is worked out as spimdisasm does: where
functions start (every function hostgen knows, by its places); which
immediates are halves of an address (a `lui` and the `addiu`, `ori`, load or
store that completes it before the register is written again, followed
through a branch whose delay slot holds the `lui`), named as the
decompilations name them (the function there, `D_<addr>`, `D_Lnn_<addr>` for
a level's own data, `jtbl_...`); and jump tables (a `jr` through a word
loaded from such an address after an index was added). On 2026-10-18 the
translations of rac1/pal and rac1/ntsc from the disc images alone were the
same C as with the assembly files, but for three PAL functions where the
pairing differs from splat's text and reaches the same address (a `lui`
splat left unpaired, a global named with another offset, a call by address
rather than by name).

Besides glabel functions, [asm.py](asm.py) reads a function spimdisasm took
for data (its words as `.word` under a `dlabel` named for the function, in
`.text`) as code, and a stub whose address is inside another function's code
(a second entry with no label) translates from there when the decompilation
has an `INCLUDE_ASM` for it or the program's code calls, jumps to or forms
that address: a name whose address only another version's numbering gives
(rac1/ntsc's C keeps a few PAL names) is not one.

[asm.py](asm.py) reads the files: the word in each instruction's comment is
what is decoded (every R5900 encoding, MMI and VU0 macro included); the text
only says where functions and labels start and which immediates are a
symbol's `%hi` / `%lo`. All files of one program (the executable, or one
level's) are one map of addresses, so a function whose code goes on into a
neighbour's file still has all of it.

The C keeps the EE's registers as locals ([ee.h](../../runtime/include/openrac/ee.h),
[ee_ops.h](../../runtime/include/openrac/ee_ops.h)):

| On the EE | In the C |
|---|---|
| a general register | a 64-bit local; a second 64-bit half where the function uses 128-bit loads, stores or MMI |
| HI, LO (and HI1, LO1), SA | locals |
| an FPU register, its condition bit, its accumulator | a float local, an int, a float |
| VU0's registers (vf, vi, ACC, Q, I) | `openrac_vu0`: one for the program, as hand-written routines leave results there |
| a branch target | a label |
| a delay slot | runs after the branch's condition is read and before it jumps; a likely branch's only when taken |
| `$sp` | the game stack: `openrac_guest_sp` at entry, given back at every return; each call sees it at the caller's `$sp` |
| `$gp` | the game's (`"gp"`); an access through it is relocated like a global |
| the parameters | `$a0`-`$t3` (then the stack) and `$f12`-`$f19`, by the prototype hostgen has for the stub, with the argument registers its code reads first that the prototype leaves out (callers then pass them as hostgen passes a caller's own through); for a function declared nowhere, those registers alone |
| `jr $ra` | returns `$v0` or `$f0` as the prototype says |
| `jal` | hostgen's call path: the callee's definition by its prototype (arguments from the registers by kind, then the stack a doubleword each), its place for a name the catalogue folded, the level's function for a hook stub; the result lands in `$v0` or `$f0`. A callee with no C that takes inputs in other registers (a hand-written helper) runs in place instead, and its `jr $ra` comes back to the call |
| `j` | a goto, unless it goes to a function with C (a tail call); a hand-written routine's continuation in the next file is followed |
| `jalr`, a jump through a register | through `GFN` (eight integer registers, and eight float ones when the function sets any), or, where an argument's position decides its register whatever its type (Windows x64, `OPENRAC_REGS_THUNKS`), through the callee's thunk in `functions.c`, which gives the registers to its parameters as the EE passes them (`openrac_guest_call_regs`, [guest.h](../../runtime/include/openrac/guest.h)); a jump table (the tables and `jlabel`s beside the function) is a `switch` on the offset into the copy loaded |
| `%hi` / `%lo` of a symbol | relocated as the C's globals and function addresses are (`Unit.global_address`, `code_address`) |
| `div.s`, `sqrt.s`, `rsqrt.s`, `cvt.w.s` | the EE's: no infinities, no NaN, saturation |
| a `double` (a parameter, an argument, a result) | software on the EE: its 64 bits in one general register, in the integer registers' order; the host's `double` (`openrac_ee_d`, `openrac_ee_dbits`) |
| `ei`, `di` | nothing: the port takes no interrupts (its handlers run at set points) |
| `break`, `teq` and the other traps | `openrac_ee_break`: logged, and the function goes on |

What it cannot write keeps the stub, and the report lists the function and
why: system calls, COP0 (but `ei`, `di`), VU0 microprograms (`vcallms`) and
clipping, a few rare MMI operations, a variadic definition, records by
value, and code that reads a register no caller passes (a hand-written
routine's own convention, or a fragment of a function with C that the
catalogue named).

### Which of them need C at all (no_c.py)

[no_c.py](no_c.py) sorts every function hostgen has no C for, or wrote from
its assembly, into what needs no C (a linker remnant, padding, a piece of a
function with C, another spelling, something nothing can run), what the host
replaces on purpose, and what needs C, from the retail program itself: every
call, jump, branch, address formed in code and data word that refers to it,
in the executable and in every level, with hostgen's own call graph. Its
table is each game's `no_c.tsv` ([port/game](../../game/README.md#the-functions-with-no-c)):

```sh
python3 port/tools/hostgen/no_c.py --game port/game/rac1-pal/hostgen.json \
    --gen port/build/release/games/rac1-pal/gen --source games/rac1 \
    --image <the PAL disc image> --out port/game/rac1-pal/no_c.tsv
```

### Checking a translation against retail

The same C, written for the console's own compiler (`--asm-console DIR`,
with [include/asmc_console.h](include/asmc_console.h): memory and calls by
retail address, VU0's real registers), is a candidate for rac1-decomp's
`tools/fcheck.py`, which runs it and retail from captured call states and
compares what they leave. `--asm-console-names FILE` writes the functions
named there, with C or not, which checks the translator on more of the
program than its stubs:

```sh
python3 port/tools/hostgen/hostgen.py --game port/game/rac1-pal/hostgen.json \
    --source games/rac1 --out build/hostgen/rac1 --asm on \
    --asm-console ../rac1-decomp/build-sn/fcheck/asmc --asm-console-names names.txt
cd ../rac1-decomp && python3 tools/fcheck.py func_001F98C0 build-sn/fcheck/asmc/func_001F98C0.c
```

## Preparing the sources

[prep.py](prep.py) copies the files it reads and changes the copies without
moving a line: `long` becomes `long long` (the EE's long is 64-bit; `long
double` stays), file-scope `__asm__` (padding for the matching build) is
blanked, and strings that run across lines are split.
[clangast.py](clangast.py) has Clang read each file for a 32-bit MIPS
target, so every type has the console's size and layout. Where Clang
refuses what GCC 2.95 accepted, the copy is fixed from Clang's own
diagnostics:

- a call before the callee's declaration gets GCC's implicit `int f()`
  (through an alias, so the later declaration stays as it is);
- a later declaration that contradicts an earlier one, and the calls after
  it, get an alias of the same symbol;
- an asm label given after a function's first use is dropped, and the name
  followed to the label's symbol;
- inline assembly whose constraints Clang does not know (`=j`, `=h`) is
  replaced by a call hostgen recognises, and the function becomes a stub;
- a call with too few arguments to a K&R function passes zeros.

Game typedefs named like the host's standard types (`size_t`, `uint32_t`)
are renamed.

## A game's configuration

`port/game/<id>/hostgen.json` ([port/game/README.md](../../game/README.md)
describes every key). What hostgen itself reads:

| Key | |
|---|---|
| `sources`, `skip` | the C to translate (`src`, without `src/libgcc`: the host compiler has its own) |
| `includes`, `defines` | how to read it, relative to the decompilation |
| `names` | how the decompilation names code and data by address: regular expressions with an `addr` group (and `overlay` for a level's code). The defaults (`func_`, `func_Lnn_`, `D_`, `D_Lnn_`) also cover Going Commando's current `master` |
| `aliases` | headers in the decompilation of `#define NAME func_ADDR` lines, its readable names for address-named symbols. Each name is written as its address name in hostgen's copy of the sources, outside comments, strings and `#define` lines ([prep.py](prep.py)), so the passes that know game symbols by address see every definition and call. rac1/pal's is `include/names.h` |
| `symbols` | splat symbol files (`name = 0x00123456;`): named globals and functions with their addresses |
| `places` | where each level function sits in each level (`config/overlays/functions.tsv` of rac1/pal) |
| `groups` | parts of the tree that are programs of their own, loaded over part of the executable (Up Your Arsenal's menu, `src/frontbin`): their definitions are their own symbols (`func_00385750__frontbin`) in their own overlay, and their calls find their own functions first |
| `candidates` | a table in the decompilation (`name<TAB>file` lines, the file relative to the decompilation) of C for functions it still has as assembly: near misses that do what the retail function does without matching its bytes. Each goes at the end of its file in place of the stub ([prep.py](prep.py)); the report counts them. A missing table means none. rac1/pal's is `nonmatching/functional.tsv`, the near misses its `tools/equiv.py` finds to be the same program |
| `hand` | a table beside the game's configuration (`name<TAB>file`, the file relative to the table) of the port's own C for functions the decompilation has as assembly: placed like a candidate, and before one for the same function |
| `asm` | `{"enabled": true, "dirs": [...], "gp": "0x00166D00", "files": 8}`: translate stubs from the assembly under `dirs` (relative to the decompilation; missing ones are skipped), with `$gp` at `gp`, into `asm_0.c` ... (`files` of them). Off when absent or `enabled` is false; `--asm on` / `--asm off` overrides `enabled`, and `--asm-source DIR` (the build's `OPENRAC_<ID>_ASM_SOURCE`) reads the directories from another copy of the decompilation, the one the game's setup ran in. `"retail"` names the retail program to read where the files are not ([above](#the-retail-program-where-the-assembly-is-not)) |
| `drafts` | `{"guard": "NON_MATCHING", "units": "drafts.tsv"}`: files of the decompilation (one a line, from its top) whose own pending C, kept under the `#else` of `#ifndef <guard>` around the assembly stub, is read in place of the stub. For a decompilation that keeps drafts it never compiles (the NTSC decomp); a file whose stub a candidate replaced keeps the candidate |
| `level_hooks` | stubs of the executable that only return, which every level's program fills with a function of its own (rac1/pal's `func_001E9780` to `func_001E97C0`). A call to one from engine code the levels also carry goes to what the loaded level's copy calls at the same instruction (`openrac_hook_in_copy`, [guest.h](../../runtime/include/openrac/guest.h)); in the executable's own program it is the stub |
| `entries` | `{"func_L06_0024A080": "0x20"}`: functions whose C begins that many bytes into their catalogue place (rac1/pal's HUD gauge draw: the catalogue starts it at four `addiu $sp, $sp, N; nop` pairs the function before left in front of it, its callers pass `func_L06_0024A080 + 0x20`, and its C is the body). `functions.c` registers each place plus the offset too, so a call through that address reaches the C |
| `libraries` | the game's table of library entry points ([port/game/rac1-pal/libraries.tsv](../../game/rac1-pal/libraries.tsv)) |
| `library_api` | the shared replacements' table, `../common/libraries.tsv` by default |
| `id`, `title`, `game`, `serial`, `frame_rate`, `entry`, `overlay_hook` | the game's description and its entry, written into `game_info.c` |
| `roots` | where the frontier report starts (the game's `main`, its title and level loops) |

`libraries.tsv` says, for each library function the game calls, whether the
decompilation's C runs as it is (`game`), the port answers it with the
shared replacement of that name (`host`: hostgen binds the address to it in
`libraries.c`, and callers are matched to its signature), the C runs under a
host function of the same name (`wrap`: the C is kept as `<name>__game`), or a
host version is still to be written (`todo`).

Two spellings of one address (`FUN_0020c828` and `func_0020C828`) are one
function: a call to a name without C goes to the function with C at the same
place, and so does a call through the address (`functions.c` lists the one
with C). A level function is one named `func_Lnn_...` or `FUN_Lnn_...`.

## Output

| File | |
|---|---|
| `<unit>.c` | each file of the decompilation, written again |
| `game_protos.h` | every function of the program, with the signature calls are matched to |
| `stubs.c` | the functions with no C yet |
| `asm_<n>.c` | the functions with no C, translated from their assembly (when `asm` is on) |
| `functions.c` | every function by code address, per level program (`openrac_game_register_functions`), and, where `OPENRAC_REGS_THUNKS` is on, each one's thunk for a call with the EE's registers |
| `libraries.c` | the game's library addresses bound to the shared replacements |
| `game_info.c` | the game's description (`openrac_game`), its entry (`openrac_game_run`) and, unless the game has its own, its overlay hook |
| `report.json` `literal_addresses` | each function's literal addresses in the relocated range and what each became (`OPENRAC_DATA(...)`, `OPENRAC_LDATA(...)`, or `kept` in a function of one level, whose addresses are that level's) |
| `report.json` `without_c_places` | each function with no C or with C from its assembly: where it sits, and the spelling with C its calls go to instead ([no_c.py](no_c.py) reads it) |
| `report.md`, `report.json` | what was translated, what became a stub and why, and **the frontier**: the functions without C that the program reaches from its roots, nearest first. That list is where decompiling moves the port on. |
| `report.json` `call_graph`, `call_site_mismatches` | what each translated function calls or takes the address of, and the direct calls whose site disagrees with the callee's definition (arity, kinds, result): [priority.py](priority.py) ranks the weak functions gameplay reaches from them ([BACKFILL.md](../../../docs/workflow/BACKFILL.md#picking-what-to-backfill-first)) |

Names other than `func_...` get a `game_` prefix in the generated C, so
the game's own `memcpy` or `main` never meets the host's.

## Files

| File | |
|---|---|
| [hostgen.py](hostgen.py) | the command: prepare, index every unit, translate every unit, write the program files and the report |
| [prep.py](prep.py) | preparing the sources |
| [clangast.py](clangast.py) | running Clang, fixing what it refuses, following its JSON's locations |
| [ctype.py](ctype.py) | C types as Clang prints them, parsed and printed back for the host |
| [program.py](program.py) | what the whole program defines: signatures, addresses, typedefs |
| [lower.py](lower.py) | the translation of one unit |
| [asm.py](asm.py), [asmc.py](asmc.py) | reading the decompilation's assembly, and C from it for the functions with no C |
| [retail.py](retail.py), [test_retail.py](test_retail.py) | the same instructions from the retail program (the disc image, or the executable and level dumps) |
| [no_c.py](no_c.py), [test_no_c.py](test_no_c.py) | which functions with no C need C: each game's `no_c.tsv` |
| [tests/mips.py](tests/mips.py) | the tests' small assembler: assembly files written the way the setup writes them |
| [test_hostgen.py](test_hostgen.py), [tests/harness.c](tests/harness.c) | small programs in the console's C, translated, built, run and checked |
| [priority.py](priority.py), [test_priority.py](test_priority.py) | what to backfill first: the weak functions each level program reaches, ranked |
