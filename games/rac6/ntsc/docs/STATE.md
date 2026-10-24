# STATE — the anchor of the workspace

> **Convention** : this file is the resume. Read it first, update it at every
> milestone. Everything else in the private workspace is disposable; this file
> is not.

Last update: **2026-10-20** (foundation lot delivered, wave 1 running, 26
placements proved at body level).

## 1. Where things are

| Element | Path | Role |
| --- | --- | --- |
| Public repository | this repository | authored sources, tools, documentation |
| Working clone | `LOCAL.md` (git-ignored) | the checkout the root integrates from |
| Local target | `LOCAL.md` (git-ignored) | the game image, the pinned hashes, the toolchain |
| Private workspace | `LOCAL.md` (git-ignored) | extracted modules, bounds, lots, agent dossiers |
| Anchor | `docs/STATE.md` | this file |

The private workspace is local-only on purpose: the project publishes written
source, tools and metadata, never retail bytes, listings or raw reports. Its
layout is described in `work/README.md`; the repository holds only the
templates of that layout.

## 2. What is pinned

Every value below is measured, and every measurement is repeatable with the
command written next to it.

- [x] **The target.** `SCUS_976.23`, *Ratchet & Clank: Secret Agent Clank*
      (USA, NTSC, `SYSTEM.CNF` version 1.00). ISO of 4 268 916 736 bytes,
      SHA-1 `6f4a37c8157e3e1214abed01406cbfce42e22cc6`, SHA-256
      `128abfa2fb8eb4dc253513d3beb9029dfb8698966ecc7b8266230e8739b8e908`.
      Pinned in `config/target.json`; the boot executable `SCUS_976.23` is
      1 104 076 bytes, SHA-256 `1341f13c…`.
      Verify with `python3 tools/disc.py inspect <image>`.
- [x] **The identity check.** `tools/disc.py` refuses an image whose boot
      file, size, boot hash or extents differ from `config/target.json`;
      `tools/rel.py` calls it before reading any module.
- [x] **The extraction.** The disc holds 31 SNR2 modules under `BIN/`:
      `FE.REL` (the frontend) and `LEVEL1.REL` … `LEVEL32.REL` (no 6, no 12),
      plus the boot ELF, the Bink videos, the FSB banks and `IMAGE.ZIP`.
      `tools/rel.py` reads the module tables; the modules themselves stay in
      the private workspace.
- [x] **The symbol and boundary sources.** The module symbol tables give the
      named entries; the relocation tables give the code boundary and the
      call targets. `tools/code_intervals.py` derives the code interval of
      every module from the jump relocations; `tools/function_inventory.py`
      turns the candidate entries into bounded bodies.
- [x] **The toolchain.** The GNU EE chain pinned by the sibling project
      (`gnu-ee-20261015`, `cpp`/`cc1`/`as`) with the SN ProDG linker, flags
      `-quiet -O2 -G0`. It is a **candidate** profile for this game: it
      reproduces retail bodies byte for byte, and it is not yet qualified on
      the complete program.
- [x] **The gates.** The **body gate** exists and is strict
      (`work/E0/harness.py`): it compiles one source, links it at the
      measured offset, resolves the fixups from the retail tables and
      compares every byte. The **placement gate** exists
      (`tools/placement_gate.py`): it checks that every proved body lies in
      the code interval, that its extent is the measured one, that no two
      placements of a module overlap, and that the published source still
      compiles to the retail bytes. The **module gate** exists
      (`tools/module_gate.py`): it rebuilds every module that carries proved
      bodies from its published sources and requires the result to equal the
      retail module, hash included; with `--image` it writes the rebuilt
      modules back into a copy of the disc, at their recorded extents, and
      requires the image to hash to the pinned retail hash -- it does, for
      the four modules that carry proved bodies. The **assembly baseline**
      is qualified
      for all 31 modules (`tools/assembly_baseline.py`): every candidate
      code interval rebuilds byte for byte, 69 483 520 bytes in total. The
      **complete-program gate** exists (`tools/complete_gate.py`) and it
      **refuses**: it requires that every bounded candidate body of every
      module come from a C source, and 299 311 of them still come from the
      assembly baseline.  Its coverage report (`tools/coverage.py`) is the
      number the campaign drives down, and no body is promoted until the
      gate passes.
- [ ] **The tests.** The repository guard exists
      (`python3 tools/check_repository.py`). The unit tests of the tools are
      still missing.

## 3. State of the campaign

- **Foundation (E0) is delivered**: 31 modules read, 409 233 candidate entry
  intervals, 64 714 676 bytes of bounded bodies by placement, the shortlist
  (930 targets, 453 of them shared with another placement), 62 lot tables,
  the harness, and its negative control (a body with the wrong value is
  refused at the exact byte that differs).
- **The assembly baseline is qualified**: the code interval of each of the
  31 modules rebuilds byte for byte (69 483 520 bytes). Every C body is
  therefore measured against a code region that is known to be the retail
  code.
- **Wave 1 is done** (lots L01, L03, L05, L07): 49 placements of 28
  distinct sources are proved and published in `src/levels/bodies/`, with
  the placement table `config/levels/bodies.tsv` (1 760 bytes by
  placement). The root re-measured every claim before publishing it, and
  the placement gate passes on the result.
- **The boot scope is open.** The boot is a linked ELF without a symbol
  table, so its candidates are measured, not named: 3 821 intervals (1 665
  call targets and 3 620 code pointers in the data), 3 214 of them bounded
  (`config/boot/summary.json`). The boot lots proved 91 placements of 20
  distinct bodies, published in `src/boot/bodies/` with the table
  `config/boot/bodies.tsv`. The boot rebuilt with those bodies has the
  pinned retail hash.
- **Wave 2, wave 3, wave 4 and wave 5 are running** (thirty-two lots):
  2 470 placements of 168 distinct sources are proved and published (the
  level modules and the boot), 54 564 bytes by placement. The root
  re-measured every claim before publishing it, and the placement gate
  passes on the result.
- **The families were expanded.** A body that is proved once is carried by
  many placements of the disc; every one of them is **re-measured on its
  own bytes** with the published source, and a placement whose relocation
  context differs is simply refused. The expansion added more than 200
  placements from the sources already proved.
- The proofs are **body-level**. The program-level gate is pending, so the
  published numbers are not promoted and no percentage is published.

### The pinned chain is a neutralised build

The chain the campaign uses is the *no-pad* build of a reconstructed EE
compiler tree.  The tree it comes from carries an `ASTRA` patch series that
reproduces the SN retail behaviour of the R5900; the no-pad build
neutralised one of its anchors:

```
  /* the ASTRA P1 anchor, in the tree the chain comes from */
  if (TARGET_MIPS5900)
    for (regno = GP_REG_FIRST; regno <= GP_REG_LAST; regno++)
      mips_reg_mode[regno] = TImode;      /* 16-byte stack slots */

  /* the same anchor, neutralised in the pinned build */
  if (0 && TARGET_MIPS5900)
    mips_reg_mode[0] = TImode;            /* 8-byte stack slots */
```

That is the whole story of the two scopes: the level modules were built
with 8-byte slots and the boot with 16-byte ones, and the *same tree* holds
both configurations.  The un-neutralised build (with the blanket two-nop
division rule of the sibling project) is a candidate route for the boot; it
is registered in the private profile table as `astra-divpad`, and it is not
a profile until it is measured.

The tree carries a single machine description, with no floating-point
hazard model: the reconstructed chain has no way to emit the retail's
division padding at all.

### A class that had never been measured

The retail's prologue saves the **general purpose registers before the
floating-point ones** -- `sd $s0, 0x10($sp)`, `sd $s2, 0x20`, `sd $s4,
0x30`, `sd $ra, 0x38`, then `swc1 $f23, 0x58`, `swc1 $f22, 0x50`, `swc1
$f21, 0x48`, `swc1 $f20, 0x40` (measured on LEVEL11.REL 0x07F620).  The
pinned chain does the opposite: its `mips.c` carries a fix made for
another game ("RAC2: FPR saves precede GPR saves").

None of the bodies proved so far saved a floating-point register, so the
class had never been measured -- the HUD and DRAW family was out of reach
without anyone noticing.  The route that opens it is **one line** of the
compiler (the condition of the save pass); the root measured its control
over the whole table, 206 of the 206 sources of the level modules stay
exact, the four that differ being the bodies that need another route.

The class holds **30 125 bounded bodies** -- nearly ten per cent of the
program, and the largest class left.  It is registered as a profile
(`config/toolchain.json`, id `gnu-ee-20261015-gprfirst`) and the wave 6
works it (lots `w6c/F*`).

### The program rebuilds byte for byte

The complete-program gate rebuilds every module -- its code interval
rendered as R5900 assembly, assembled with the assembler of the SN,
linked, with the proved C bodies written over it -- and writes the result
back into a copy of the disc image.  The rebuilt image hashes to

```
128abfa2fb8eb4dc253513d3beb9029dfb8698966ecc7b8266230e8739b8e908
```

which is the hash of the retail disc, pinned at the start of the
campaign.  **The program is reconstructed byte for byte**: the proved
bodies sit exactly where the retail has them, and the rest of each
module is what the retail has.

The gate still refuses, and it must.  What it measures is *where the
program comes from*, not whether it is right: 297 318 bodies still come
from the assembly baseline rather than from a C source.  The
reconstruction proves the first half of the question -- the bytes are
exact -- and the coverage is the second.

### The vector bodies are not reproducible in pure C

A research agent closed the class of the bodies written in the vector unit's
instructions (`lqc2`, `sqc2`, `vmulax`, `vdiv`, `vwaitq`).  The evidence:

- the machine description of the pinned chain holds 341 instruction
  patterns and **zero** occurrence of `lqc2`, `sqc2`, `vmulax`, `vdiv`,
  `vwaitq` or of the word "vector"; the same is true of `mips.h`, `mips.c`
  and of the whole compiler tree;
- there is no legal way in: `TARGET_SWITCHES` and `TARGET_OPTIONS` carry no
  vector option, the ISA ceiling is MIPS IV, there is no target `pragma`,
  and the only wide mode the target accepts is `mode(TI)`, which gives the
  integer quadword `lq`/`sq` and not COP2;
- the four compilers of the workstation leave no vector trace in their
  binaries (`strings` finds none), while `mul.s`, `div.s` and `cvt.s.w` are
  there;
- the source the sibling project actually built has a machine description
  whose hash is identical to the pinned one: the COP2 instructions are known
  to the *assembler* only, never to the compiler.

The class is therefore **not** a source-form question: 64 bytes of COP2
appear at the same address of the boot and in thirty levels, and a scalar C
body compiles to pure FPU.  It can only reopen if a vector mode becomes
visible in C for MIPS **and** the machine description carries the COP2
patterns **and** the "pure C, no asm, no `.word`" rule is lifted.  Until
then it is a closed class, not a pending one.

### The boot's long tail

The boot holds 3 821 bounded candidate intervals -- 563 872 bytes of
bodies, more than every level module together -- and 111 of its bodies
are proved.  Measured over the 2 166 prologues of the boot, the frames
are distributed as:

- **1 374** in 16-byte slots with `sq`/`lq` -- the route `prodg2953` (63 %);
- **373** in 16-byte slots with `sd`/`ld` -- the route `sdk991111`;
- **351** in 8-byte slots with `sd`/`ld` -- the pinned chain;
- **68** without a frame -- exact on every route.

The first lot of that tail proved **15 of its 15** targets, which makes
the boot the most productive seam of the campaign: 208 lots of it are
waiting (the private kit holds them).

### The boot and the modules are not the same build

The boot keeps its frames in **16-byte slots** (`sq`/`lq`, `$ra` on top),
where most of the level modules keep **8-byte slots** (`sd`/`ld`). The cause
is the register mode of the target: the GNU EE 2.9 tree of the pinned chain
neutralises the R5900 anchor `mips_reg_mode[0] = TImode`, and two other EE
compilers keep it.

**But the level modules are not homogeneous either.** A survey of the
spacing of the stack slots (`tools/slot_spacing.py`) reads every function
prologue of the 31 modules and finds two families:

- **129 856 prologues** reserve their slots **8 bytes apart** -- the
  family the pinned chain reproduces;
- **7 208 prologues** reserve them **16 bytes apart** or more -- 5 % of
  the functions, spread over every module.

Both families save with `sd`, which stores eight bytes: it is the *spacing*
of the slots that differs, not the width of the store. The wide family
holds **5 822 bounded bodies** -- the largest class left in the campaign --
and the route that reproduces it is the one the boot already uses,
**`sdk991111`** (Sony EE-GCC 2.9): the wave-5 lot `w5/L02` measured 84 of
the 400 bytes of FE.REL 0x0C5EA8 identical with it, the first difference
being a branch distance and not the frame. The wave 6 works that family
(lots `w6b/V*`).

Reading the spacing takes care, and the survey was wrong three times
before it was right: only the *stores* of the prologue count (the loads of
the epilogue do not), the window has to stop at the first call and at the
second frame adjustment (or the next function leaks in), and the argument
registers the frame saves (`sd $a0,0x60($sp)`) count as well.

| profile | chain | what it reproduces |
| --- | --- | --- |
| `gnu-ee-20261015` | the reconstructed GNU EE chain of the sibling project | the level modules (8-byte slots) |
| `prodg2953` | SN ProDG ee-gcc 2.95.2, from the compiler depot of the sibling workspace | the boot (`sq`/`lq` frames) |
| `sdk991111` | Sony EE-GCC 2.9-ee-991111-01, from the same depot | the boot (16-byte `sd` frames) |

The control is clean: the four frame-less bodies of the boot lot are exact
on the three chains alike -- they do not discriminate -- and the four that
carry a frame are exact only on their own chain. The profile is therefore
chosen **per source**, and the placement table records it per row. The
three profiles are pinned by hash in `config/toolchain.json`.

This is the strongest toolchain evidence of the campaign so far: the retail
build used at least two compiler configurations, and the campaign can
now name them.

The converse is measured too: the boot's two chains do **not** apply to the
level modules. Tried on a parked module body, both differ from the retail
at the very first byte -- the prologue -- because the modules keep 8-byte
slots where those chains keep 16-byte ones. The division padding of the
modules therefore belongs to a third configuration, still unnamed.

**And the boot is not homogeneous either.** `tools/slot_survey.py` measures
the slot width of every bounded body of the boot:

| slot width | bodies | where |
| --- | --- | --- |
| 16 bytes (`sq`/`lq`) | 1 146 | contiguous runs, the largest 0x00133518..0x00137D10 |
| 8 bytes (`sd`/`ld`) | 651 | contiguous runs, the largest around 0x0019B658 |
| both | 267 | mixed bodies |
| neither | 1 150 | bodies without a frame |

The two families live in **contiguous bands**, which is the signature of
two compilations inside the same executable. The route is therefore chosen
per body, and the survey is what tells which family a body belongs to.

### The bugs of the retail are part of the program

The boot's `memmove` is **buggy**: the forward copy adds the length to the
source, the backward copy adds it to the destination. A matching body has
to reproduce the bug, not fix it -- the original behaviour is the
behaviour of the retail program. The same goes for the `break`
instructions the division survey counted (854 of them): they are the
retail's own, not ours.

This is the one place where the campaign must be careful not to "improve"
the game: a fixed bug is a different program, and the hash would say so.

### The divisions of the retail code

A wave-2 lot measured a body that was one instruction short: the retail
puts a `nop` between the setup of the divisor and the `div.s`. Before that
becomes a compiler route, it has to be *measured* across the whole program --
`tools/div_survey.py` does it:

- the 31 modules hold **63 443 divisions**, of which 51 719 are `div.s`;
- of those `div.s`, **71.2 % carry two `nop`, 7.7 % three** (a branch delay
  slot plus two), 7.2 % one, and **13.8 % none**;
- the padding follows a multi-cycle floating-point operation in almost
  every case (`mtc1`, `mul.s`, `swc1`, `madd.s`, `mov.s`: padded every
  time), while an integer operation such as `addiu` is usually bare;
- but the rule is **not** a function of the preceding instruction alone
  (`addiu` and `cvt.s.w` appear on both sides), **not** of the encoding of
  the division (the same `div.s` is 80 % padded in one module and bare in
  another), and **not** an alignment rule either: the address of a division
  is uniformly distributed modulo 8 whatever the padding, and only 48.6 %
  of them are 8-byte aligned.

The conclusion about the *rule* is a negative one, and it is useful: a
blanket "insert two nops before every division" patch -- the route the
sibling project built for its own game -- cannot be right for every body,
because 21 % of the retail's divisions carry one nop or none, and the count
is a decision of the scheduler rather than a function of the code around it.

**The blanket route is a route nonetheless**, and the campaign uses it where
it holds.  Two things were measured on it:

- it is a **superset** of the pinned chain on the level modules: every one
  of the 158 sources the pinned chain proves is still exact with it
  (158/158, no break);
- it **completes** the bodies whose divisions all take the two-nop form --
  `MobyUtil_IsShotClear__FPC4VEC3T0b` (256 bytes) is exact with it, and
  stays parked with the pinned chain, whose compiler emits no `nop` at all.

The route is therefore recorded as a profile with its cause, its provenance
and its control (`config/toolchain.json`, id `divpad`), and it is used per
source, the way the sibling project does.

**And the contextual rule exists.**  A second route (`divpad-contextuel`)
was built on a copy of the pinned tree, with a 226-line patch that gives
the division family the retail's two instruction slots (`dslot` in
`mips.md`, a `MACHINE_DEPENDENT_REORG_AFTER_DBR` hook) and fills them
from the context: three nops when the producer is a branch, one when it
is a floating-point operation the division reads, none when it is a
conversion, two otherwise.  The retail's `rsqrt.s` family always takes
the two slots.

- it is a **superset** of the pinned chain on the level modules: 191 of
  the 191 sources stay exact, no break;
- it **proves the class the blanket rule misses** -- the bodies whose
  division carries a *single* nop -- `GADGET_GetParametricXPToNextLevel`
  among them, now published with its six placements.

Its rule is a *model* of the retail's behaviour rather than a
reproduction of it: over the 51 719 divisions of the program it agrees
with the retail on 78.2 per cent, 93 per cent of the single-nop class and
99 per cent of the three-nop class.  The class it serves worst is the
one with no nop at all (30 per cent), where the division sits right
against its producer.

**The MIPS level is not that route.** The EE target refuses `-mips1` and
`-mips2` outright ("Only MIPS-III or MIPS-IV CPUs can support 64 bit gp
registers", the R5900 has 64-bit general purpose registers), and `-mips3`
and `-mfp32` change nothing in the body. The hazard nop belongs to the
scheduler of the retail compiler, and it is not reachable by a flag on
this target. Both measurements are in `config/toolchain.json`, under
`trials`.

### What the first wave measured about the compiler

Four classes of difference showed up, and they are worth keeping:

| Class | Example | Reading |
| --- | --- | --- |
| one bit of a branch | `LEVEL10.REL:0x0490C0`, 255/256 bytes: the retail uses the *likely* form of the COP1 branch | the same routine is exact in `LEVEL1.REL:0x0490C8` and in `LEVEL19.REL:0x0490C0`; the modules are near-copies, not copies |
| a copy in a distinct register | `GADGET_IsModToBeInstalledAtSlot__FUiUiUi`, 128 bytes | **solved by lot L35**: the retail keeps the result of the comparison in `$v1` and copies it into `$s2`. The source form is a local pointer to the array, an `int` temporary, and an `unsigned long long` return. The compiler then produces the copy and the single-instruction return. It was parked in eight lots before that form was found -- it was a *form* question, not a toolchain one |
| register and scheduling | `LEVEL10.REL:0x033B10` (now proved) | the retail computes the address in two steps (`data+0x30` then `+slot*4`) and keeps the comparison result in the register of the comparison: a difference of source *form*, not of option |
| early return | `LEVEL11.REL:0x0337E8`, 255/256 bytes | the retail skips the restore of `$ra` on one path: a *sibling call* shape |
| the order of the writes | `ICONHORMENU_AddItem`, 400 bytes, exact in four modules | the retail writes the stack arguments **before** the register ones; the natural order of the parameters allocates the registers differently. The order of the writes decides the allocation |
| a comparison kept in a register | `GADGET_IsGadgetOfChar`, 256 bytes, parked in every module | the retail develops `x >= C` into a *value* (`slti` + `xori`) and keeps the flag in `$v1`; the pinned chain develops it into a *branch* and turns `if (c) d = 1;` into `movn`. **Nine EE chains were tried** (the pinned one, ProDG 2.95.2, the SDK 2.9, the three of the depot, the three of the sibling project) and all of them behave the same way: this is the hardest open body, and the difference is not a toolchain one |

A candidate profile change needs a measured cause, a known provenance and a
regression control before it is used. The `-mdebugc` route has the first
two; the control run over the 49 proved placements is the next step.

**The `-mdebugc` control is measured.** Recompiling the 49 proved bodies
with `-O2 -G0 -mdebugc` keeps **49/49** of them exact: the option breaks
nothing that is proved. It does **not** unlock the parked bodies either:
on the two `sltiu` bodies the first difference moves *earlier* (body byte
48 to body byte 4), because the option also changes the frame. The verdict
is therefore: a candidate route, not a profile. Reproduce it with
`work/E0/trial_mdebugc.py` and `work/E0/trial_mdebugc_bodies.sh`.

**And the route was the wrong question.** Lot L35 later found the *source
form* of the same body, and the body is now exact at every placement of its
family, with the pinned chain and without any option. The `sltiu` idiom
was never a toolchain question: the retail keeps the result of the
comparison in `$v1` and copies it into `$s2`, and the copy is what the
source form has to ask for. The option was a red herring that cost eight
lots their best candidate -- the method says to recover the authentic form
first, and this is what that means.

## 4. What is next

1. **Close the coverage.**  The complete-program gate now measures what it
   needs: 29 046 of the 323 138 bounded candidate bodies come from a proved
   C source (283 452 of 64 714 676 bytes by placement), and **180 593
   distinct bodies** are still to translate -- a family counts once, because
   one source serves every placement that carries it.  The gate passes when
   that number is zero.
2. **The `-mdebugc` route.** The measured cause is known, the regression
   control over the proved placements is done (49/49), and the route is
   refuted by the source form that lot L35 found.
3. **The remaining classes.** The `div.s` hazard pad is a scheduler
   question; the switch tree of `GADGET_IsGadgetOfChar` is not a toolchain
   question (nine chains agree); the vector bodies need a machine
   description that the pinned compiler does not have.
4. **The boot executable.** Its own scope is open: 244 of 3 214 bounded
   bodies are covered, and the two slot widths mean two chains.
5. **The queue is fresh.** `tools/shortlist.py --exclude` leaves out the
   bodies that are already proved, so the next wave works on fresh targets.

## 5. Resume checklist

If a session is interrupted:

1. `ls work/w1/*/RAPPORT.md` and read the reports.
2. `cat work/w1/*/resultats.tsv` for the measured table.
3. A body proved but not integrated: re-prove it with the harness, then hand
   it to the root. Do not re-run the foundation lot.
4. Update this file before stopping.

## 6. Open decisions

- The reference is the USA disc; a European revision could be a second
  target, but the pins of `config/target.json` are per-image.
- The program is one boot executable plus 31 relocatable modules; the
  complete-program gate has to treat both.
- The retail compiler identity is still unknown: the pinned chain reproduces
  the bodies it was measured on, and nothing more.
