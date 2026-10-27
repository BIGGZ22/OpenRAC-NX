# The host build of a game's decompiled C

This is how the port grows out of the interpreter
([DESIGN.md](../docs/DESIGN.md), section 3, route C). `port.py` compiles a
game's decompiled C for the host, as a library; the runtime loads it and runs
each decompiled function in place of the retail one, on the same memory
image. Whatever is not decompiled yet, or is left out below, the interpreter
keeps running. Nothing here changes how the decompilations build or match.

```sh
python3 runtime/port/port.py build rac1/pal          # about ten seconds
build/runtime/openrac-boot DISC.iso --hooks runtime/games/SCES_509.16.hooks --window \
    --native build/port/rac1-pal/libopenrac-native.dylib
```

It needs LLVM (clang with the wasm32 target, `wasm-ld`) and WABT (`wasm2c`);
with Homebrew, `brew install llvm lld wabt`. The library is written to
`build/port/<game>-<version>/` and holds code built from the repository's C
only. The sizes and checksums of the retail functions it stands in for are
read from your own disc's files, which `tools/openrac.py setup` and the
game's own set-up put in `games/<game>/<version>/baserom/`.

State on 2026-10-09, Ratchet & Clank (PAL): 352 of 371 source files are in
the library (the game, its levels, the core library and the port's own C for
the assembly routines), 2,471 functions standing in for 21,466 places in the
boot program and the 19 level programs. Every level starts and draws with
host code: 500 to 570 functions run in each (5 to 19 million calls in the
first 40 seconds), and checked both ways (below) none differs from the
retail code. Two calls in three are host code.

## How it works

1. **32-bit pointers.** The C is written for the console: pointers are 4
   bytes and data sits at the retail program's addresses. Each source file is
   compiled to WebAssembly (wasm32), where both still hold, and `wasm2c`
   turns the result into C for the host. The module's memory is the guest's
   whole address space, mapped so that a guest address is an offset from one
   base (`src/ps2/memory.*`).
2. **One module a source file.** A call that leaves the file goes through the
   EE's registers, as the console's calling convention has them
   (`src/sys/native_abi.h`): integers in registers 4 to 11, floats in FPU
   registers 12 to 19, the rest on the stack. So it reaches retail code or
   another module alike, calls through function pointers work (a pointer in
   the game's memory is a guest address), and a function that two files
   declare with different prototypes does no harm.
3. **Retail globals and function addresses** are values the host supplies:
   the code is built position independent, and each symbol named after its
   address (`D_0013D9B4`, `func_L00_0025B4D0`) resolves to that address.
4. **Bound by checksum.** A host function runs at an address only while the
   guest's memory there holds the retail code it was written from; this is
   checked when the address is first reached and again after the program
   says it changed code (a level loading). A level's functions are therefore
   used in that level only, and nothing is used on another version's disc.
5. **A program for each level.** Ratchet & Clank loads a program of its own
   for each level, and each has its own copy of the engine at its own
   addresses: the function the boot program has at `001F9EE8` is at
   `001FF6A8` in level 0 and at `00221538` in level 1, and the globals it
   uses moved as well. The decompilation has such a function once, under the
   name of one place. The library has it once too, and binds it at every
   copy.

   What a name stands for in each level's program is read from the retail
   code (`levels.py`). Two copies of a function are the same instructions
   apart from the addresses in them: a call's target, the two halves of an
   address, an offset from the global pointer. The same instruction in both
   copies gives an address in one program and its counterpart in the other.
   The decompilation's catalogue (`config/overlays/functions.tsv`) says which
   functions are copies of which. A function is bound at a copy only when
   the copy is the same instructions, every function it calls is where the
   source file's names lead in that level, and so is every global its code
   names; a call to another function of the same file needs that one bound
   there too. The runtime finds which level's program is loaded by a
   checksum of the start of its code and tells the library, whenever code
   changed.
6. **The console's arithmetic and devices.** Float operations go through the
   runtime's model of the console's FPU (no infinity, cut towards zero), so
   host code leaves what the retail code leaves. Loads and stores at the
   device addresses go to the runtime's register model. Host code's own
   stack and data are in the first megabyte of main memory, the kernel's on
   the console, because the game hands local buffers to the DMA controller
   like any other address.

`include/` holds the `common.h` and `include_asm.h` that stand in front of a
game's own in this build: the same types, the steering macros of the retail
compiler made empty, assembly never included, and the three quadword helpers
in plain C. `long` is 8 bytes on the console, so the build defines it as
`long long` and reads an integer constant marked `L` as `LL` (`0xFE00L << 46`
is a 64-bit shift there).

## Checking a function against the retail code

```sh
build/runtime/openrac-boot DISC.iso --hooks ... --frames 1000 --no-card \
    --native LIBRARY --native-check 4
```

checks each function's first four calls: the retail function runs in the
interpreter, the machine is put back, the host function runs, and what the
two left in memory and in the result register is compared. A function that
differs is handed back to the interpreter and listed at the end with the
first difference. A function that differs is run a third time, by the host code over a stack
filled with other bytes: if the result changes, the function reads a local
it never set (a three-float vector handed to a routine that reads four), and
it is listed as `unset` instead of `differs`. No host build can leave what
the retail code leaves there; such a function stays with the interpreter
until its C sets what it reads. A call that reaches outside memory (a
library function, a device) cannot be run twice and is not compared; the
count is printed. A
retail function that does not come back without a timed event (it waits for
an interrupt) is not compared either: all it did is taken back and the
interpreter makes the call. A host function that reaches outside memory
where the retail one did not counts as differing.

`--native-range FIRST:LAST` and `--native-skip FILE` use only part of the
library, for finding by halving which function changes a picture.

Native code takes no emulated time, so a load that took thirty fields
interpreted takes fewer: pictures of the same field number are not
comparable between the two ways of running. Compare what does not move (a
menu's text), or use the check above.

## The game's assembly routines

The game's small vector and number routines (add two vectors, the length of
one, a matrix times a vector, an angle wrapped to a turn) are hand-written
assembly for the vector unit and the FPU. No C compiles to them, so the
decompilation keeps them as assembly, and they are the most called code in
the game. `hand/<game>-<version>/` has them in C, written from the
instructions of each routine: the same operations in the same order, so
that rounding agrees. They are built and bound like any other source file,
at every copy in every level, and the check compares them with the retail
routines like any other function (31 so far; all pass in the levels they were checked in).

What they do not reproduce is what a routine leaves in the vector unit's own
registers, so a routine whose result depends on what an earlier one left
there is not written yet (the cross product stores a fourth field it never
computed), nor are the ones that call the vector unit's microprograms (sine,
cosine, the matrix builders).

## Any level, and what to decompile next

A run starts in the first level. To start in another, keep the word that
holds the level to load at the level's number while the game leaves the
menu (`--write FRAME:ADDRESS:VALUE[:FRAMES]`, hexadecimal address and value;
for the first game's PAL disc the word is at `15EE84`):

```sh
build/runtime/openrac-boot DISC.iso --hooks runtime/games/SCES_509.16.hooks --no-card \
    --press 100:4000:5 --press 450:8:5 --press 520:4000:5 --press 620:4000:5 \
    --write 600:15EE84:5:400 --frames 2600 --native LIBRARY --native-check 4
```

`--native-calls FILE` counts every call by level and address and writes the
counts at the end. `port.py wanted GAME/VERSION FILE...` reads such files
and lists the guest functions that were called with no host function
standing in, the busiest first, each with its name in the decompilation,
its size and why it is not host code: not decompiled, or decompiled and
left out. That is the order in which decompiling a function takes the most
work away from the interpreter.

## Functions left to the interpreter

Besides the lists below, the build leaves out the functions the runtime
answers itself (the hooks table of the disc, `runtime/games/SERIAL.hooks`):
host code calls a function of its own source file directly, so a decompiled
caller would otherwise run the library function's body where the runtime
means to answer. It also leaves out functions defined with `...`, whose
further arguments are where the console's compiler put them.

`leave/<game>-<version>.txt` lists decompiled functions that the build
leaves out, each with the reason. They match the retail bytes and still do
something else on a host. The fix is in the decompilation, checked against
the match, and the line goes when it is made. Two causes so far:

- **A 64-bit value passing through declarations typed as 32 bits**, which a
  register move on the console carries whole. `wide/<game>-<version>.txt`
  names the functions whose integer result is 64 bits wide whatever a file
  declares (one so far, the texture register value); the build reads every
  declaration of them as returning `long`, which is enough where the caller
  stores the result in a 64-bit place. A function that keeps the result in
  an `int` of its own, or takes it in an `int` parameter, is found from the
  unoptimised compiler output (the 64-bit result cut to 32 bits) and left
  out; the names are in `left_by_narrowing.txt` (20 today).
- **Two functions under one name.** Functions that differ only in the globals
  they use (the four that add a callback to one of four lists) have one
  fingerprint, and the catalogue has them as one name with several places in
  a level. A source file's name can stand for one of them only; a function
  whose retail code means another is left out, and the names are written to
  `left_by_copy.txt` (29 today). The fix is a name for each. (Such a
  function is itself bound place by place: at each copy that uses the
  globals its own code uses.)
- **Locals laid out for a callee.** A function fills a structure that
  nothing reads and passes the address of the local next to it: the callee
  reads across both, which works only with the retail compiler's stack
  layout (the original source had one structure). `port.py` finds these
  itself, from the unoptimised compiler output: a function with an array or
  structure that it neither reads nor hands on (stored to, or never touched)
  is left out, and the names are written to `left_by_stack_layout.txt`
  beside the library (20 today). A local that is merely too small for what
  a callee does with it cannot be seen that way; the check finds those, and
  they are listed in `leave/`.

Host code calls a function of its own source file directly, so a function
that calls a left one directly is left as well (`left_by_call.txt`).

Source files that do not compile for the host are reported by `port.py` and
left out as a whole: today 14 of the first game's, each for a function
declared with two different prototypes in one file, which the retail
compiler accepted.

## What is not done

- The other games: `SOURCE_DIRS` in `port.py` lists the first game only.
- Speed: every float operation is a call into the model, and every load and
  store tests for a device address.
- The both-ways check does not cover functions that reach outside memory.
