# libgcc

Retail links GCC's runtime library, `libgcc.a`, into `core_text` at
0x11DFE8-0x1206A0. Those functions were not written for the game. They are
GCC's own sources built by Sony's EE toolchain, so they are rebuilt here
from GCC's sources rather than decompiled.

All of it is built with **Sony's `2.9-ee-991111` compiler**, not the game's
GCC 2.95.3 (SN v1.14). It goes **through the 2.9-ee driver** (`ee-gcc.exe`),
never `cc1` directly. The driver passes the target predefines (`__mips__`,
`__R5900__`, ...), and `longlong.h` chooses its MIPS multiply and divide
primitives from those. Without them `__divdi3` and `__muldi3` come out wrong.

Each `L_*` module is its own object, like the members of `libgcc.a`. That is
what reproduces retail's alignment gaps between modules. Modules that do not
match yet stay as asm stubs in `nonmatching_*.c`, in their link position.
The linker filled two gaps with `0xCDCDCDCD`, and those are kept as stubs
too. The link order is `config/core_text.objects`, which the build and the
tools read. Per-module facts for the progress report are in
`tools/libgcc_units.py`.

## Sources (GPL v2 with the libgcc linking exception, see each file's header)

| file | origin | changes |
|---|---|---|
| `libgcc2.c` | GCC trunk at `31cf01446d` (1999-09-09) | none |
| `gbl-ctors.h` | same revision | none |
| `longlong.h` | same revision (unchanged from 1999-06 to 1999-12-06) | none |
| `fp-bit.c` | GCC 2.95.3 release | two, marked in place: `nan()` uses one shared `__thenan_df`, because every retail reference goes to 0x1597F0 (trunk moved it to its own object on 1999-09-13); and `unpack_d`'s `NO_DENORMALS` test, backported from trunk `2672543458` (Cygnus, 2000-03-16), which Sony's tree already had |
| `include/` | ours | stand-ins for the build-tree headers (`tconfig.h` etc.) that libgcc2.c includes |

Why 1999-09-09: retail's `__do_global_ctors` never registers the
destructors, and that revision is the last one where the call goes
through `ON_EXIT`, which `gbl-ctors.h` leaves empty on a target without
`HAVE_ATEXIT`/`NEED_ATEXIT`; trunk made it an unconditional `atexit` on
1999-09-15. Every other module compiles identically from 1999-06 to
1999-11, so nothing else constrains the revision.

The soft-float code is `fp-bit.c` built twice as whole files, the way
Sony's `libgcc.a` has it: `dp-bit.o` (double) and `fp-bit.o` (`-DFLOAT`),
with `-DFLOAT_BIT_ORDER_MISMATCH -DNO_DENORMALS -DUS_SOFTWARE_GOFAST`
(`FP_DEFS` in `Makefile.sn`). GCC's MIPS makefile fragments define
`FLOAT_BIT_ORDER_MISMATCH` for little-endian, the EE FPU flushes denormals,
and the GOFAST names (`dpadd`, `dpmul`, `fpadd`, `fptodp`, ...) are what
Sony's compilers call. Built this way both objects are function for
function identical to Sony's (all 30 functions, same offsets). Retail's
linker then dead-stripped what the game never calls; see below.

## Sony's libgcc.a is the reference

Every EE compiler directory in both toolchain mirrors ships the same
prebuilt `libgcc.a`, and it is what retail linked: `_divdi3.o`,
`_moddi3.o`, `_udivdi3.o`, `_umoddi3.o`, `__main.o`'s `__do_global_ctors`
and `__main`, `_pure.o`, and every retail function of `dp-bit.o` match
retail byte for byte with relocated fields masked, as do `__unpack_f` and
`fptodp` from `fp-bit.o`. So a module can be checked against Sony's
object directly, without a link:

```
python tools/libgcc_ref.py retail                  # which archive function matches which retail address
python tools/libgcc_ref.py diff build-sn/libgcc/dp-bit_whole.o __pack_d
```

## Retail's linker dead-stripped what nothing calls

An unreferenced function lost its first `floor(size/8)*8` bytes: one whose
size is a multiple of 8 vanished, one of size 4 mod 8 left its last word
(its final jump's delay slot) and the alignment nop behind.
`tools/strip_dead.py` applies that to the compiler's output. It accounts
for every "odd" spot in retail's libgcc:

- **L__main**: `__do_global_dtors` (84 bytes) left the `addiu` at
  0x11DF10 (`func_0011DF10`), before `__do_global_ctors` and `__main`.
- **dp-bit.o**: `__negdf2` (56) vanished from between `dptoul` and
  `__make_dp`; `dptofp` (84) left `func_001206A0`, then linker fill.
- **fp-bit.o** (0x1206A8-0x1207B8): everything but `__unpack_f` and
  `fptodp` was stripped. `__pack_f`, `_fpadd_parts`, `fpsub`, `fpmul`,
  `__fpcmp_parts_f`, `fpcmp`, `fptosi` and `__make_fp` (4 mod 8) left one
  word each, in order (`func_001206A8`, `func_00120740`-`func_00120770`);
  `fpadd`, `fpdiv`, `sitofp`, `fptoui` and `__negsf2` (0 mod 8) left
  nothing.

The object after fp-bit.o (0x1207B8, `src/core/001207B8.c`) is not
libgcc: its two functions program VIF1 and DMA registers.

## Modules

| retail | module | function | status |
|---|---|---|---|
| 0x11DF10 | `L__main` | (`__do_global_dtors`'s surviving delay slot), `__do_global_ctors`, `__main` | **exact**; see "dead-stripped" below |
| 0x11DFE8 | `L_divdi3` | `__divdi3` | **exact**; its static `__clz_tab` goes into core_rdata at 0x152B18, see `tools/split_data_s.py` |
| 0x11E6D8 | `L_fixunsdfdi` | `__fixunsdfdi` | **exact** |
| 0x11E7C8 | `L_floatdidf` | `__floatdidf` | **exact** |
| 0x11E860 | `L_moddi3` | `__moddi3` | **exact** (built with Linux 2.9-ee-991111-01) |
| 0x11EEC8 | `L_muldi3` | `__muldi3` | **exact** |
| 0x11EF28 | `L_udivdi3` | `__udivdi3` | **exact** (built with Linux 2.9-ee-991111-01) |
| 0x11F4F8 | `L_umoddi3` | `__umoddi3` | **exact** (built with Linux 2.9-ee-991111-01) |
| 0x11FA38 | `dp-bit.o` | `__pack_d`, `__unpack_d`, `_fpadd_parts`, `dpadd`, `dpsub`, `dpmul`, `dpdiv`, `__fpcmp_parts_d`, `dpcmp`, `litodp`, `dptoli`, `dptoul`, `__make_dp`, and `dptofp`'s remnant | **exact**; `__negdf2` stripped |
| 0x1206A8 | `fp-bit.o` | `__unpack_f`, `fptodp`, and the remnants of eight stripped functions | **exact** |

The three remaining libgcc2 stubs share one residual, and it is not the
source. Against Sony's objects, ours compile to the same instructions
except that Sony's reserve stack no instruction touches: `__udivdi3` has a
0x10 frame where ours has none, `__umoddi3` 0x30 where ours has 0x10,
`__moddi3` 0x40 where ours has 0x20; `__divdi3`, built from the same
inline `__udivmoddi4`, has no extra space in either. Measured on
2026-09-23:

- every `libgcc2.c` revision from 1999-06-11 to 1999-11-23 (15 of them,
  each with its own `longlong.h`) compiles these modules identically,
  and the December 1999 `longlong.h` rewrite makes them worse;
- no flag moves the frame (`-g`/`-g1`/`-g2`/`-g3`/`-ggdb`, `-O1`/`-O3`/
  `-Os`, `-fexceptions`, `-G0`/`-G8`, and 20 `-f` options were tried),
  apart from `-fno-inline`, which changes the code entirely.

Sony's objects were built on Linux (their debug info names
`/usr/local/sce/ee/gcc/build/gcc/` and `../../src/gcc/libgcc2.c`), while
the mirrors hold the Windows `2.9-ee-991111b/r4` `cc1`. Dead stack slots
with identical code point at that build difference. These stay stubs
unless a matching Linux `cc1` turns up.

## L__main was dead-stripped by retail's linker

Retail's L__main object is Sony's `__main.o` with the first 80 bytes cut
off: `__do_global_dtors` is gone except its last word, the `addiu
$sp,$sp,0x20` in the delay slot of its `jr $ra`, followed by the alignment
`nop`, then `__do_global_ctors` and `__main` intact. Nothing references
`__do_global_dtors` here (the constructors do not register it), so the
linker dropped it, and dropped it one instruction short.
`tools/strip_dead.py` does the same to the compiler's output, and names
the surviving word `func_0011DF10` the way splat sees it. See
docs/DECOMP_PROGRESS.md for the rest of the image.

The module's statics follow retail: `__main`'s `initialized` is placed at
retail's D_001597EC inside core_bss, and the stripped function's static
pointer (its `.data`) is discarded, since retail's copy is in the data
blob and nothing references it.

## Status

- All functions in libgcc now build 100% byte-exact from GCC source using Sony's Linux `2.9-ee-991111-01` compiler (`__moddi3`, `__udivdi3`, `__umoddi3` matched).
