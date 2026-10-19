# Assembly reconstruction status

The complete pinned USA boot **`.text` section** at `0x01E62500`, size
`0x79CD4` (498,900 bytes), has been rebuilt from private R5900 assembly and
compared byte for byte with the retail ELF: **498,900 / 498,900 bytes equal**.
The linked candidate `.text` SHA-256 is
`8dab904fd9dd57e1c3774c92b07fe0872445a99775e85dc62229e857a1a6830c`.
This expands the earlier 160-byte `MCP_LoadLevelModule__Fi` witness; it is
still an assembly baseline, **not** an authored C/C++ match.

The tool also copies the pinned boot's `.data`, `.sndata`, `.rodata` and
`.gcc_except_table` into **ignored local assembly**, links them at their
measured addresses, and reconstructs the zero-filled alignment gaps. The
byte sequence reconstructed from those linked sections matches all
**809,872 / 809,872 file-backed PT_LOAD bytes** of the original segment,
SHA-256
`4f4fb8fe218317dbd7a2f65ca4e64934d743d03e673dbd75220937cfc6fd47fb`.
Those non-code sections are copied reference bytes, not independently
decompiled source.

[`tools/boot_assembly.py`](../tools/boot_assembly.py) verifies the pinned boot
hash, decodes all 124,725 text words with optional `rabbitizer 1.16.2` in
R5900 mode, writes all game-derived source only under ignored
`build/boot-assembly/`, and assembles and links it at the measured addresses.
The local SN
`Ps2EeAs.exe` version `1.8.19.316` has SHA-256
`c839dd63facabe7b76573c114056be61eaa7b3aa2329dd546930ab8f98898c76`;
SN `ld.exe` version `2.9-ee-991111` has SHA-256
`80f3724a63ce3c77c8b00d075f5c49448636518700ae251e5d6a6c809bbf5f48`.
The tool checks both hashes before running either binary. No game-derived
assembly, ELF, object, disc image or proprietary tool is distributed.

Two measured decoder/assembler differences need explicit handling:

- `rabbitizer` does not name 50 R5900 two-register `sqrt.s` instructions;
  their exact encoding is mapped to `sqrt.s` mnemonics. A two-instruction
  local assembler probe confirmed the encoding, and the complete text
  comparison checks all 50 in place.
- SN's assembler inserts a retail-absent hazard `nop` before six COP1
  branches immediately following floating-point comparisons. Those six
  branches are emitted as exact `.word` values in the **ignored private
  assembly**. The other 124,719 words are emitted as instruction mnemonics.
  Without this narrow fallback, the linked `.text` is 24 bytes too long and
  fails the byte comparison. This does not change the shared assembler or any
  C compiler.

On Windows, with your own verified USA boot and locally available measured
tools, install optional `rabbitizer==1.16.2` in a Python environment and run:

```sh
python tools/boot_assembly.py baserom/SCUS_976.15 \
  --assembler /path/to/Ps2EeAs.exe --linker /path/to/ld.exe
```

The BSS and live runtime memory have **not** been compared, and none of the
21 SNR2 modules has a **complete** assembly baseline. The exact file-backed boot image is
not proof that the game's non-code data was independently reconstructed.
Complete runtime and level-program baselines, plus a qualified compiler
profile, remain prerequisites for a full-program C/C++ replacement check; no
ByteMatch progress is claimed from this assembly result.

The first module interval has since been tested separately: the candidate
`FRONTEND.REL` code region at offsets `0x80`–`0x27D00` reassembles
**162,944 / 162,944 bytes** from private mnemonics. Its jump sites agree
exactly with both SNR2 relocation sources
([frontend evidence](FRONTEND-ASSEMBLY.md)). This is not yet the whole REL
assembly baseline.

`LEVEL_01.REL` now has a separate, much larger candidate code interval at
`0x80`–`0x1EF180`: **2,027,776 / 2,027,776 bytes** reassemble exactly,
with complete local and named jump-site correspondence
([level 01 evidence](LEVEL-01-ASSEMBLY.md)). Neither result rebuilds a
complete REL. The same guarded procedure has since reproduced candidate
code intervals for **all 21 SNR2 placements**
([campaign evidence](SNR2-ASSEMBLY-CAMPAIGN.md)). Those bytes are counted by
placement, not as unique code or C progress.

A [provisional C integration witness](BOOT-C-WITNESS.md) has also replaced
the 16-byte boot body `MEMCARD_Busy__Fv` with a linked GNU EE C object while
preserving the entire boot `.text`. Its retail `PT_LOAD` header and raw
809,872-byte file-backed segment now also match. It is still under
`nonmatching/` because the remaining compiler, complete ELF metadata,
catalogue and runtime gates have not been completed.

## Assembler inventory and equivalence (2026-10-16)

The C objects and the assembly baseline do **not** use the same assembler, and
that separation is now measured rather than assumed.

**Objects.** Every campaign object is assembled by the pinned GNU `as`
(`c71a15db…`, `gnu-ee-trial-20261015`). Eleven further assemblers from the local
toolchains were compared on the same `.s` inputs: the full inventory is a private
reproducible artifact, and the result is that the pinned binary is the only one
of twelve that reproduces the retail bytes across all 411 campaign objects.
Divergent objects per alternative: one behavioural twin with **0/411**
(`cda1a4e4`, a useful second witness against a corrupted pin), one ProDG 2.0
`ee-as.exe` and the recovered Sony `ee-as` with **1/411**, one ProDG 3.01
`ee/bin/as.exe` with **2/411**, and the SN `Ps2EeAs` 1.8 and 1.9 with
**194/411** and **201/411**. Two distinct binaries are both called `ee-as.exe`
in local documentation; they differ (confirmed by hash) and they disagree on
hazard insertion.

**Assembly baseline.** The baseline is assembled by SN `Ps2EeAs` 2.0
(`c839dd63…`, `ps2eeas 1.8.19.316`). The choice is forced, not preferred: the six
GNU variants reject the file outright (`illegal operands 'padduw'` — they do not
know the R5900 MMI mnemonics), and `Ps2EeAs` 3.01 emits 499,608 bytes instead of
498,900 by inserting 177 hazard nops and shifting branch offsets. Only the pinned
2.0 build reproduces the 498,900-byte `.text`.

**Hazard `nop` insertion is an assembler property.** `cc1` writes its `#nop`
markers as inert comments — an experiment with the marker, without it, and with
an ordinary comment produces identical output for every assembler tried. The real
`nop` is inserted by each assembler's own reorder-mode hazard analysis, and the
rules differ per hazard class and per binary (for example `mtc1` before `swc1`,
`mtc1` before `cvt.s.w`, and short backward branches).

**The pinned `as` is not universal.** Three measured counter-examples, each
reproducible, keep the twelve-binary comparison from being a blanket verdict:

1. **A hazard rule.** The pinned `as` inserts a `nop` after `mfc1 $v0,$f12` when
   the next instruction consumes the result, and retail does not; retail keeps
   the `nop` only when the consumer is a branch. Two other binaries reproduce
   retail here.
2. **A `dli` expansion.** For `dli 0x800fffffffffffff` the pinned `as` emits seven
   instructions (`ori` first) where retail emits six (`lui` first). The
   `dli`-matches-retail result above therefore holds for the constants measured,
   not for every 64-bit immediate.
3. **A `static const float` load.** GNU `as` reads it from the literal pool (two
   instructions) where retail inlines the value (`lui`/`ori`/`mtc1`, four
   instructions); only SN `Ps2EeAs` inlines it.

Those axes are independent: the SDK-family GNU binaries fix hazards and `dli`
but pool floats, while `Ps2EeAs` inlines floats but differs on hazards. No
single binary covers both, so the choice belongs **per object**, the same way
the per-source flags in `config/cflags.txt` work — not as a global pin change.

That selector now exists: `config/cflags.txt` takes `as=NAME` beside the
compiler selector, and the profile pins the two extra binaries in
`extra_assemblers` ([route R](COMPILER-STATUS.md#the-assembler-is-a-per-source-choice-too-route-r)).
Two bodies use it, the 416 pre-existing objects are untouched, and the gate
links 477 objects into the exact complete boot. The measurements that made it
necessary: `__pack_d` (304 bytes) is 249/304 under the pinned `as` and 300/300
under the SDK assembler, `sceKernelGetSystemTimeLow__Fv` (160 bytes) is 155/156
under the pinned `as` and 160/160 under `Ps2EeAs`; of the 679 objects compared,
the SDK assembler moves 19 and `Ps2EeAs` 313, which is why neither replaces the
pinned chain.

**Constant encoding.** The pinned `as` encodes 32-bit float literals faithfully:
`0.05f` becomes `0x3D4CCCCD` and `0.050000004f` becomes `0x3D4CCCCE`, and three
other assemblers produce the same two words, so no rounding is involved. For
64-bit immediates, `dli` is expanded inline to `ori`/`dsll` (matching retail at
`0x01E862D4`), while a multi-chunk `li.d` — which `cc1` emits for a `double`
operand — goes through a literal pool under GNU `as` and is expanded inline by
`Ps2EeAs`. That difference is a per-object assembler choice, not a wall: the
retail bytes are reachable with the pinned chain whenever the value can be a
64-bit integer rather than a `double`.
