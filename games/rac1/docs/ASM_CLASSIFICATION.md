# Original assembly classification

Some retail code was written as assembly, and some four-byte words are
leftovers from functions stripped by the linker. Matching those bytes by
compiling C would misrepresent their origin. The tracked lists are:

- `config/handwritten_asm.txt`: 212 functions marked handwritten by the
  disassembler (83,520 retail bytes).
- `config/linker_remnants.txt`: 145 entries of dead-strip remnants (3,260
  retail bytes).

On 2026-10-17, 33 VU0 helpers in fastfunc.o (func_001F9BC8..func_001FA8A8)
were added to `config/handwritten_asm.txt`: retail's fastfunc is hand-written
noreorder assembly (see `tools/ps2eeas_nops.py`), with hand-placed nops, `$at`
used as a scratch register and `mtc1` in branch delay slots, which no C build
produces. They are original assembly, not C waiting to be matched.

On 2026-10-18, seven libc string routines in core (func_001150D4, func_00116428,
func_001165B8, func_001166FC, func_00116810, func_00116948, func_00116B00: the
strlen/strcpy/strncpy family) were added: they use 128-bit MMI compares (psubb,
pnor, pand, pcpyud) that ee-gcc 2.9 has no patterns for, so they are original
assembly.

The handwritten functions and the first 69 remnants were taken from the two
corresponding `tools/triage.py` buckets on 2026-09-27, and 22 remnants were
split off the functions splat had folded them into the same day. The
`fallthrough fragment` and `epilogue fragment` buckets hold both runs of
remnants that splat named as one function and pieces of functions cut in
the wrong place. `tools/overlay_remnants.py --exe` applies the rule below
(see "Level code") to the executable; the 54 runs it accepted were added on
2026-10-07, and what it rejects stays unmatched as a boundary to fix.

`tools/setup_asm.sh` regenerates `asm/` from the contributor's own baserom,
then `tools/organize_asm.py` moves the listed files into
`asm/handwritten/<segment>/` and `asm/remnants/<segment>/`. Compatibility
links remain under `asm/nonmatchings/<segment>/` for the differ and retail
inventory. No retail assembly bytes are committed to Git.

The source uses `ASM_FUNC` and `LINKER_REMNANT` to include these files
without changing the linked image. The progress report counts the listed
original assembly as finished work while the build audit continues to
count exact C matches separately. Changing this published progress policy
requires upstream review.

## Level code

Level code has the same leftovers, and `config/overlays/linker_remnants.txt`
lists them (84 catalogue entries, 1,752 bytes; an entry shared between
levels is one name with many places). The source marks each with
`LINKER_REMNANT("asm/overlays", name)` where its `INCLUDE_ASM` stub was, and
the report counts it as finished, so a file or a level whose functions are
all matched is not held open by stray words.

The level catalogue cuts functions apart more often than the executable's
listing does, so the list is not taken from a triage bucket. It is what
`tools/overlay_remnants.py` derives from the level dumps, and
`tools/overlay_remnants.py --check` compares the two. A stripped function
of size 4 mod 8 leaves its last word, then an alignment nop or the linker's
`0xCDCDCDCD` fill (`tools/strip_dead.py`), so an entry is a run of remnants
when it is not a piece of a joined function and, in every level it is
placed in:

- it starts on an 8-byte boundary, where a stripped function began; each
  of its words there is not a jump or branch (no delay slot is one), each
  word between is zero or fill, and at least one word is neither;
- the code before it is finished: going back over earlier remnants (a
  word, then zero or fill), there is a `jr $31` and its delay slot, a tail
  jump `j` to the start of a function, or the start of the code;
- nothing reaches it: no branch or jump lands on one of its words, and no
  word of code or data holds one of their addresses.

In the executable the rule picks out nine of the ten words `src/libgcc`
reproduces by stripping GCC's own source the way retail's linker did; the
tenth is a nop, which no rule can tell from alignment. The last two tests tell remnants from a piece of a function the catalogue cut
in the wrong place: of the eight 4-byte level entries that fail, three
follow code that has not returned, one is the delay slot of the return
before it, two are already joined pieces, and two are off the 8-byte
boundary. A trap block that its function branches to fails the last test.

On 2026-10-19, 23 more were listed: the remaining fastfunc.o scalar helpers
(`$at` scratch, `max.s`/`min.s`/`mul.s` in `jr` delay slots, hand-placed nops,
`adda.s` 0.25+0.25 rounding), func_0020C210/func_0020C230 in miscproc (`$at`
address, mid-function jump), func_0023C910 (labelled hand-written: `ei` and a
mid-function `.align`) and memcmp func_001151B4 (MMI `pxor`/`pcpyud`).

On 2026-10-21, memcpy func_00115248, memmove func_001152F8 and memset
func_001153FC were listed. Their code uses no instruction a compiler could not
emit (`lq`/`sq`, `ld`/`sd`, byte loops), so the disassembler does not mark them,
but the SDK's own `libc.a` (in the toolchain, `ee/lib/libc.a`) says where they
came from: the debug section of each member names its source file, and
`memcpy.o`, `memmove.o` and `memset.o` were assembled from
`newlib/libc/machine/r5900/memcpy.S`, `memmove.S` and `memset.S`. The members
already listed as original assembly read the same way (`memcmp.o`, `strlen.o`,
`strcpy.o`, `strncpy.o`: `r5900/*.S`), while the libc functions matched in C
name C files (`mallocr.o`: `stdlib/mallocr.c`, `mbtowc_r.o`:
`stdlib/mbtowc_r.c`). The members' code is retail's, byte for byte.

The same day func_0011AE1C was listed: the 4-byte `jr $31` after the listed
sceSifWriteBackDCache (func_0011AD70) is that function's last word. In the
SDK's `libkernl.a`, `sifcmd.o` gives sceSifWriteBackDCache a symbol size of
176 bytes (0x698..0x748, retail 0x11AD70..0x11AE20, the next member being
sifrpc.o's sceSifInitRpc at func_0011AE20), with the hand-written local labels
`loop1`, `eight`, `loop8` and `last`; `last:` is the `jr $31; nop` at 0x11AE14,
and the stray `jr $31` follows it inside the function.

### Hand-written level functions

Level code also has functions written as assembly. `config/overlays/handwritten_asm.txt`
lists them (67 entries, covering 94 catalogue entries and 51,748 bytes once joined pieces
are counted). The source marks each with `ASM_FUNC("asm/overlays", name)` where its
`INCLUDE_ASM` stub was. As with remnants, the report counts it as finished and checks the
list against the source. A joined function (`config/overlays/joined.tsv`) is listed under its
first name, and its later pieces are finished with it, as they are when a joined
function is matched in C.

The list uses the executable's rule: a function the disassembler marks "Handwritten
function" (it holds an instruction ee-gcc 2.9 does not emit: a trapping `addi`/`teq`, VU0
macro instructions, MMI, `$at` as a working register). That gives 87 entries, 67 after
joined pieces fold into their first names. Two joined functions are listed under first names the
disassembler did not mark, because their other pieces are marked and the first
pieces hold VU0/MMI instructions themselves: func_L00_001FD1D8 (`lqc2`) and
func_L00_00257F4C (`prot3w`). Two are listed by hand:

- func_L00_00251328 (48 bytes) has a `nop` before `jr $31` with the delay slot filled
  (`sd`), a slot no ee-gcc 2.9 schedule leaves empty;
- func_L00_00251358 (48 bytes) uses `$at` as an ordinary register for a `dsrl32` result, which
  ee-gcc 2.9 never allocates, and leaves the `jr $31` slot empty.

Cross-check against the NTSC-U decompilation (Lombyte, MIT): 87 of the 89 entries (the
87 marked ones and these two) are assembly there as well. They are either `INCLUDE_ASM`
stubs or copies of executable functions it keeps under `assembly/`, and it documents the
two hand-listed ones as "hand-written in retail" for the same reasons. The other two have no
NTSC-U counterpart: func_L00_0024EC20 and func_L01_00262BB0 are `teq $0, $0, 0`
trap stubs.

Two marked functions are left out for now: func_L00_002569F8 and func_L00_00257848, which
together are the level programs' copy of MobyProc (5,152 bytes, as `func_00212658` is). The
native port replaces the executable's MobyProc with its own code, but not yet this copy (its
`no_c.tsv` says it needs C), so it stays an `INCLUDE_ASM` stub until the port runs it.

On 2026-10-19, the last five fastfunc.o functions were listed: the VU0 helpers
func_001F9B50, func_001F9F90, func_001F9FA8 and func_001F9FC0 (`qmtc2`/`vcallms`/`vsqrt` with
`$at` scratch and hand-placed nops), and func_001FA898, a float-to-int helper that
converts in place in `$f12` where every ee-gcc truncation picks its own scratch register.
Also listed then: func_00125380 (Sony libvu0's `_sceVu0ecossin`, a stand-alone asm routine taking
its input in `$vf6`/`$7`) and func_001F7DD8 (drawquad: MMI packing, a never-taken `bnez $0`,
hand-placed nops), the same shape as its level copies listed upstream.

On 2026-10-21, func_L00_002001D8 (24 bytes, the level programs' fastfunc.o copy, between the
listed func_L00_001FFED8 and func_L00_00200210) was listed by hand: it splits a float into
its whole part (stored through `$4`) and fraction with `cvt.w.s` and `cvt.s.w` both in `$f0`,
where ee-gcc 2.95.3 and 2.9-ee move the truncated value through a GPR (`mfc1`/`mtc1`), and it
leaves the `jr $31` slot empty with `sub.s` right before it, a slot both compilers fill (they
put the store there). Lombyte keeps its NTSC-U copy (FUN_L00_00200228) as assembly too.

On 2026-10-19, twelve more level functions were listed by hand: the VU0 leaf helpers of the
level copies of fastfunc (`shared/fastfunc_001FEF78.c` and level 6's `fastfunc_002172B0.c`) and
func_L00_00251468. Each has a VU0 instruction in a branch or `jr` delay slot, or `$at` as a
working register: func_L00_001FF240, 001FF270, 001FF2C8, 001FF2F8, 001FF328, 001FF4B0,
001FF500, 001FF610, 001FFCF8, 00200210, 00251468 and func_L06_002172B0.
Then func_L00_001FF3E0 (`mtc1` in the `jr` delay slot, `$4` as scratch) and func_L00_001FFDD0 (an
empty `jr` slot after four `sqc2`, which the assembler fills in any compiled copy).

On 2026-10-21, two more level functions were listed by hand. Each is the level copy of an executable
function that `config/handwritten_asm.txt` already lists:

- func_L02_002100E8 (48 bytes, `shared/draw_0020BF88.c`, in seven levels) is func_001F9AF0 aimed at
  DMA channel 0x1000D400 instead of 0x1000D000 (`config/overlays/variants.tsv` pairs them): `$at`
  holds the channel base for four `sw`, with a hand-placed `nop` after the first;
- func_L00_0024EBF8 (40 bytes, `shared/miscproc_0024E798.c`, in all 19 levels) is the first 40 bytes
  of func_0020C758: a busy-wait on bit 8 of the word at `$3` with a `nop` after the load and after
  the `andi`, empty delay slots and a `j` back to its own entry. The level catalogue cut that
  function in two. The `teq` trap block after it is listed already: func_L00_0024EC20 in eight
  levels, the 12-byte func_L01_00262BB0 in the other eleven.

Lombyte keeps both NTSC-U copies (FUN_L02_002101b8 and FUN_L00_0024dbc0) as intentional assembly too.

### Corrected level boundaries, 2026-10-23

Three corrected entries now pass the existing remnant test at every place:
`func_L00_001EB1A8` (4 bytes, 19 placements),
`func_L08_002DB438` (4 bytes, one placement), and
`func_L00_002EDB58` (12 bytes, three placements). The first started on an
alignment nop four bytes too early; the other two included independent
empty/return-zero functions referenced by moby/camera dispatch tables.
The fill-only
`func_L06_00216EE4` entry is removed from the function catalogue, not
classified as a remnant or C. `tools/overlay_boundary_fixes.py --check`
verifies the bytes, preceding returns and references at all 27 places.
See docs/OVERLAYS.md for the explicit denominator correction.
