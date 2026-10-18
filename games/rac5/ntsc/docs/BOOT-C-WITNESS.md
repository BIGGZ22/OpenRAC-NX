# Provisional whole-boot C replacement witness

`MEMCARD_Busy__Fv` at `0x01E627C0`–`0x01E627D0` is the first authored C
candidate linked **inside the complete boot assembly baseline**. The trial
GNU EE chain compiled the source in
[`src/boot/memcard_busy.c`](../src/boot/memcard_busy.c) with
`-O2 -G0`, without the optional spacing sentinel. Its 16-byte `.text`
replaced the retail assembly at that exact address. SN `ld.exe` linked that
real object between two private assembly sections; the object bytes were
not rewritten after compilation.

The resulting boot `.text` matches **498,900 / 498,900 bytes**, SHA-256
`8dab904fd9dd57e1c3774c92b07fe0872445a99775e85dc62229e857a1a6830c`.
With non-code sections copied from the pinned boot as in the
[assembly baseline](ASSEMBLY-STATUS.md), the logical file-backed load image
matches **809,872 / 809,872 bytes**, SHA-256
`4f4fb8fe218317dbd7a2f65ca4e64934d743d03e673dbd75220937cfc6fd47fb`.
The linked BSS is NOBITS at the retail address `0x01F28100` with the same
817,644-byte section size. No branch or jump in the pinned boot targets the
excluded 16-byte interval.

The linker recipe now reproduces the **complete retail `PT_LOAD` program
header**: file offset `128`, virtual and physical address `0x01E62500`,
file size `809872`, memory size `1627648`, flags `7`, and alignment `128`. The actual raw
809,872-byte segment at that file offset also matches byte for byte. SN `ld`
needs `-N` to avoid page-aligned padding, a separate 128-byte aligned input
section holding the unchanged first 32 boot instructions, and a 20-byte
allocated NOBITS tail after the unchanged BSS. This tail accounts for the
retail segment's memory extent without adding file-backed data or changing
the BSS section. It is a linker layout reconstruction, not a new C function.

[`tools/boot_c_witness.py`](../tools/boot_c_witness.py) reproduces this
experiment using the pinned boot, local SN assembler/linker, and a privately
compiled C object. It checks the normalized C source SHA-256
`16e4be48785a4f90e710cf9b5ffe596e071bd1041a41a8fab5936dbb1fca9e18`
and object SHA-256
`415800c82c13e302c09de6aa773a68f82414410e28375e6b726db591fbdeb674`,
as well as the measured SN tool hashes. Generated game-derived assembly,
data objects and linked ELF stay under ignored `build/boot-c-witness/`.

The object was produced from the source's bare filename on a local Linux
filesystem by the reconstructed GNU EE `cpp`/`cc1`/`as` chain, hashes
`2ac3d8d3ca177e6705ac2cbdd1bd9e9a7181ac3e40f6230dea6875c3218ec155`,
`37704f483fba7269791576879b3573445cd455ce0473e6d58a3bcceb45105f95`,
and `c71a15dba889fc273b0b699986de7316d056b94ae2155ad2c2573eaa62b8d902`.
The compiler was copied into an isolated private RAC5 directory before this
trial so concurrent RAC2 tool changes could not alter it. Rebuild the object
from the bare `memcard_busy.c` filename on a Linux filesystem with your own
lawful copies of those tools, then keep it under ignored `build/`:

```sh
cpp memcard_busy.c > memcard_busy.i
cc1 -quiet -O2 -G0 memcard_busy.i -o memcard_busy.s
as -o memcard_busy.o memcard_busy.s
```

Copy the object into the ignored workspace and run:

```sh
python tools/boot_c_witness.py baserom/SCUS_976.15 \
  --c-object build/memcard_busy.o \
  --assembler /path/to/Ps2EeAs.exe --linker /path/to/ld.exe
```

This historical one-object witness alone was provisional. The later
[full boot gate](BOOT-TRIAL-GATE.md) qualifies the reviewed boot placements
under a limited boot-only profile and counts 263 matches (19,048 bytes), starting
with this first 16-byte body. The direct SN-linker file
differs in section headers and non-loaded metadata, while the later packaging
gate reconstructs the complete ELF exactly. The original
compiler identity, complete runtime comparison and game-wide catalogue remain
open. The later [ByteMatch estimate](../progress/README.md) uses a provisional
denominator and does not count the other boot witnesses. The original compiler's
FOG loop mismatch motivated the
[isolated RAC5 threshold trial](COMPILER-LOOP-TRIAL.md) below.

## Combined provisional C witness

**Historical qualification:** the measurements in this section describe the initial
20-body campaign. The current pure-C campaign links 635 objects and catalogues 635
bodies; every object span expands a global function symbol. Its current counts and
source policy are in [BOOT-TRIAL-GATE.md](BOOT-TRIAL-GATE.md). These are different
measures: one object can contain more than one body. Inline-assembly experiments
are excluded from the current campaign and retained privately.

The initial full-boot link was repeated with **twenty authored C
functions** from seventeen exact-slice source groups in
[`COMPILER-WITNESSES.md`](COMPILER-WITNESSES.md). Compile these sources with
the same pinned GNU EE `cpp`/`as` and the isolated
[RAC5 short-loop `cc1` trial](COMPILER-LOOP-TRIAL.md), `-O2 -G0`, and **without**
`RAC5_WITNESS_PADDING`; place the resulting seventeen `.o` files in an ignored
local directory. Run [`tools/boot_c_campaign.py`](../tools/boot_c_campaign.py)
with that directory:

```sh
python tools/boot_c_campaign.py baserom/SCUS_976.15 \
  --objects-dir build/boot-c-objects \
  --assembler /path/to/Ps2EeAs.exe --linker /path/to/ld.exe
```

For a single command that first recompiles those objects with the pinned
private tools, use the [boot trial gate](BOOT-TRIAL-GATE.md).

The tool pins each public C source and private object by SHA-256, splits the
verified boot assembly around the seventeen object intervals, and links those
objects without rewriting their instructions. Eight objects end four bytes
before the next measured label. SN `ld` adds the four zero alignment bytes
between input sections; those bytes match the retail boot. Branch labels in
the replaced intervals are rebound to their measured addresses. The linked
ELF symbol table places all twenty C functions at the expected boot addresses:
their compiled bodies total 604 bytes, with 44 zero alignment bytes across
the 648-byte set of next-label intervals. The linked
result matches the entire boot `.text` **498,900 / 498,900**, the raw
file-backed `PT_LOAD` **809,872 / 809,872**, its complete program header, the
retail entry address `0x01E62508`, and the original BSS section placement and
size. Generated assembly, copied
non-code sections, objects and ELF stay under ignored `build/`.

[`tools/boot_witness_boundaries.py`](../tools/boot_witness_boundaries.py)
separately audits the linked ELF's twenty global C function symbols against
the pinned boot and generates
[`config/boot/witness_boundaries.tsv`](../config/boot/witness_boundaries.tsv):

```sh
python tools/boot_witness_boundaries.py baserom/SCUS_976.15 \
  build/boot-c-campaign/boot_multi.elf
```

For each initial witness it verified the compiled symbol start/size, a final
`jr $ra` with its delay slot inside the body, zero bytes up to the next boot
label, and no **direct** boot branch or jump into that alignment. The result
is 604 body bytes plus 44 zero alignment bytes. The compiled symbol size is
the candidate's body extent; it does not prove the retail build used the
same source or assigned exactly that function size.

The current generator records the observed terminal transfer instead of assuming
every body ends with `jr $ra`. A terminal call is recorded as `call`, without
asserting that its callee cannot return; it is not independent boundary qualification.

The direct SN-linked ELF still differs. The retail file has 31 VU `.DVP` sections outside
its `PT_LOAD`. Their [metadata audit and private reproduction](DVP-OVERLAYS.md)
matches all 31 section contents and types. That direct-link file is 20 bytes
shorter than retail and retains section-layout differences. The later
[`boot_elf_pack.py`](../tools/boot_elf_pack.py) repairs those metadata
differences and produces a complete byte-identical ELF. The earlier PCSX2
smoke test is documented separately; it does not establish full gameplay.

`FOG_PreUpdate__Fv`, the two string-length witnesses and
`GLOBALWAD_ReleaseTransitionBank__Fv` use the isolated
[RAC5 short-loop compiler trial](COMPILER-LOOP-TRIAL.md); the earlier sixteen
retain their prior object hashes under that trial.

The integration does **not** accept a body merely by linking it. Acceptance
follows the [promotion review](BOOT-PROMOTION.md): 263 reviewed placements
(19,048 bytes) are counted, and every other linked body remains provisional.
Complete ELF metadata, runtime behavior and the game-wide
catalogue remain open. The non-code sections still come from the pinned game
image; this does not constitute a reconstructed game build or ByteMatch
progress.
