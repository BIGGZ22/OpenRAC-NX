# Provisional C compiler witnesses, USA PS2 boot

Twenty small boot functions from several code areas were reconstructed as
independently authored C, initially in `nonmatching/boot/`. The pinned boot's `.text`
intervals and each local candidate ELF were compared at the same virtual
addresses, with no relocation masking. This is evidence that one **candidate**
toolchain and `-O2 -G0` can produce these limited intervals. It does not
identify the original compiler; the later full-boot gate supplies the
separate loaded-byte evidence for the accepted placements.
The `MEMCARD_Busy__Fv` source has since moved to `src/boot/` under the
[boot-only acceptance gate](BOOT-MATCH-SCOPE.md); two later reviews promoted 30
bodies each to the same directory, so 263 accepted boot placements now supply
their own compiled bytes. The remaining functions of this early table stay in
`nonmatching/` and contribute no accepted matches unless they were among the
reviewed 30.

| Source | Boot interval | Functions | Exact slice | Candidate slice SHA-256 |
| --- | --- | ---: | ---: | --- |
| `memcard_busy.c` | `0x01E627C0`–`0x01E627CF` | 1 | 16/16 | `9a09682cfc1e09f8c9a452d2e7035b6a6767223d7f834cb0683b8554de576c07` |
| `memcard_get_widescreen.c` | `0x01E62838`–`0x01E62847` | 1 | 16/16 | `4b91d4555501ea04fd2eb11f8deadafcb862c141f2a76abd2049694054472bd9` |
| `mcpi_msgboxes.c` | `0x01E64FE8`–`0x01E65027` | 4 | 64/64 | `784aa6333dbb2cbfcc6bfdaf07a64a906ad78f8489b64a796bdc935067f2a211` |
| `hig_io_read.c` | `0x01E65458`–`0x01E65477` | 1 | 32/32 | `86746200bee52dcb43166ff611e7f723dd37ee204c26758b89a4135a969f9117` |
| `mcpfont_string_length.c` | `0x01E67978`–`0x01E679B7` | 1 | 64/64 | `7bc5cb5ef40e15ad48e97d0f83d1e10ad38599e84c99f45d235e4563355cd973` |
| `mcp_string_length.c` | `0x01E67EE8`–`0x01E67F27` | 1 | 64/64 | `7bc5cb5ef40e15ad48e97d0f83d1e10ad38599e84c99f45d235e4563355cd973` |
| `camera_init.c` | `0x01E68630`–`0x01E68657` | 1 | 40/40 | `4ce3a071f1098892e7cddbd6f149546c2abc9804ac1b844e1667cda2fe644621` |
| `fast_mem_copy.c` | `0x01E68EE0`–`0x01E68EFF` | 1 | 32/32 | `fd8428d3a925e06646a38fd352c2c76629c4ed5d49e37bfb17b664b80d9941b7` |
| `mcptrans_is_complete.c` | `0x01E68F00`–`0x01E68F37` | 1 | 56/56 | `9cdb7c1d61aac09eba37aabf4ca5d3ead800fb8b19ae63c19e97e898353a7b1c` |
| `mcptrans_should_fade.c` | `0x01E68F38`–`0x01E68F47` | 1 | 16/16 | `f21c780dbce47a749e53f20d035d49d38309d91f3dd5c87efaf1a94b17fcea3a` |
| `rendereffect_reset.c` | `0x01E7D450`–`0x01E7D45F` | 1 | 16/16 | `be8dbcbf52cb4de173df838a870426003c0f217f97686e2321a4f0c7933cb420` |
| `fog_preupdate.c` | `0x01E7D4A8`–`0x01E7D4E7` | 1 | 64/64 | `f8a6560b8960c8d17085570344896f2e51ffea6285580cd0390579b61c6f7abe` |
| `buffer_cache_ptr.c` | `0x01E82EC0`–`0x01E82ECB` | 1 | 12/12 | `73f3c5c8a67f99ccf427cdcba54f5ed5229719f96d3306fbab5a19fe19175d05` |
| `system_clear_color.c` | `0x01E83DA0`–`0x01E83DAB` | 1 | 12/12 | `2b90771555a7809856671840436cc9bedf55d188e675dab389104dfd6f530c98` |
| `anim_buffer_used.c` | `0x01E8A1D0`–`0x01E8A1DB` | 1 | 12/12 | `bca983ad7bdf47446f8df906a43e7f1e9aae72de2021440011cd6b8943c8403a` |
| `globalwad_release_transition_bank.c` | `0x01E8FE80`–`0x01E8FEDF` | 1 | 96/96 | `daf80f2a29103e4a0a6dfffea346a53543864441d3002f2f4c4cdc87aff26ccd` |
| `snd_unlock_positional_array.c` | `0x01E91708`–`0x01E9171F` | 1 | 24/24 | `b70d0c2a72b4b88d10dc0d9b44ea595bcabacd7d3118ebab91aafae8a4fd019a` |

The four `MCPI` labels are consecutive 16-byte placements: two setters and
two getters. In those historical slice trials, `RAC5_WITNESS_PADDING` added a
function after the candidate body, so the assembler emits its final alignment
bytes. That extra function is neither compared nor asserted to represent
retail code. Those diagnostic probes have been archived privately and removed
from public sources. They are not permitted in current C evidence.
The newer `MCPTRANS_IsComplete` candidate covers two conditional branches and
multiple returns; `SND_UnlockPositionalArray` covers a conditional store
whose volatile read generates a branch-likely instruction. These strengthen
the compatibility evidence beyond simple accessors, but are still only local
interval checks.
The new `higIoRead`, `CAMERA_Init` and `FastMemCopy` witnesses exercise a
linked call and return-address save/restore; `FastMemCopy` also shifts an
argument into the call delay slot, while `CAMERA_Init` writes a global before
its call. They add 104 exact local bytes, bringing the trial total to
**280/280 bytes across eleven functions**. Five further candidates in
memory-card, render-effect, buffer, system and animation code reproduce 68/68
compiled body bytes in isolated links. Their twelve trailing alignment bytes
also match in the complete boot link, bringing the measured candidate spans to
**360/360 bytes across sixteen functions**. The isolated loop-threshold
trial adds FOG's 64/64 bytes, bringing the current provisional span total to
**424/424 bytes across seventeen functions**. A second loop pattern,
string length, matches 64/64 at each of two boot addresses under the same
isolated threshold-six compiler, bringing the provisional span total to
**552/552 bytes across nineteen functions**. A third short-loop shape with
an unfilled branch delay matches 96/96 in `GLOBALWAD_ReleaseTransitionBank`;
the original compiler yielded 69/96. The current provisional span total is
**648/648 bytes across twenty functions**. Isolated slice equality alone did
not qualify a whole-program match. A later
[provisional integration witness](BOOT-C-WITNESS.md) linked the
`MEMCARD_Busy__Fv` C object into the entire boot assembly and kept the full
file-backed bytes exact. A [combined integration witness](BOOT-C-WITNESS.md#combined-provisional-c-witness)
now links every authored body into the boot at once and preserves the exact raw
`PT_LOAD` bytes, program header and ELF entry; linking stays provisional, and
only the reviewed placements in the explicit gate set are counted.

## Campaign expansion

A later authoring campaign grew the same mechanism from twenty bodies to
**477 compiler-produced C objects**, after hybrid instruction-assembly experiments
were excluded. The catalogue identifies 477 global function bodies covering 30,316
code bytes plus 884 zero alignment bytes; every object span expands a global
function symbol. Objects can contain multiple bodies.
One body,
`dlib_displist_direct_end__FP13DLIB_DISPLIST`, carries the per-source option
`-mno-check-zero-division` recorded in
[`config/cflags.txt`](../config/cflags.txt), and ten more are built by two
further pinned compilers through the same file's selectors
([route P](COMPILER-STATUS.md#a-second-pinned-compiler-route-p),
[route Q](COMPILER-STATUS.md#the-sony-sdk-compiler-route-q)).
Sources retain their retail ELF symbols, and the linked ELF must keep the complete
856,120-byte boot byte-identical. **61 of these bodies** have since completed
[promotion review](BOOT-PROMOTION.md) and now live in `src/boot/`: together with
`MEMCARD_Busy__Fv` they supply 19,048 boot `.text` bytes from their own compiled
placements, while every other body remains an uncounted witness. The generated
[`config/boot/witness_boundaries.tsv`](../config/boot/witness_boundaries.tsv)
catalogue lists each identifiable linked body with its compiled extent, following zero
alignment and the kind of control transfer it ends in (`jr_ra`, `jump`,
`jr_reg`, `break`, `syscall`, or `call`). A terminal call does not prove that its
callee cannot return. No inline assembly, fixed-register bindings or raw instruction
words are permitted, including system-call wrappers. Such routines stay in the
assembly baseline without C credit; C can still call them normally.

Measured limits of the current pinned chain, from the campaign's failures:
GCC 2.9 register-allocation and scheduling orders still block a minority of
bodies, and two compiler settings inherited from the RAC2 reconstruction make a
measured set of functions unreachable without a separate compiler profile. The
first is a neutralised R5900 wide-integer anchor that sets the 16-byte
register-save stride retail uses (`sq`/`lq` frames); the second is disabled
sibling calls, where retail emits a direct `j` for some tail calls.

`FOG_PreUpdate__Fv` at `0x01E7D4A8` exposed a short-loop padding mismatch in
the original trial `cc1`: two `nop` instructions before the backward branch,
where retail has one. An [isolated RAC5-only compiler patch](COMPILER-LOOP-TRIAL.md)
changes the loop threshold from seven to six instructions and reproduces the
entire **64/64-byte** body, without post-processing the object. All sixteen
earlier function witnesses keep identical object hashes under that patched
compiler. This is a stronger profile control, not proof of the original
compiler. The FOG function itself remains unaccepted.
`GLOBALWAD_ReleaseTransitionBank__Fv` exercises a backward branch whose
retail delay slot remains `nop`, covering the unfilled-delay path of that
loop-padding trial ([details](COMPILER-LOOP-TRIAL.md)).
The globals' current names and types in the C files are hypotheses; their
link-time addresses are measured from the pinned instructions and recorded in
`config/boot/witness-*.ld`. The table deliberately contains no code bytes.

The first sixteen witnesses used the locally reconstructed GNU EE
`cpp`/`cc1`/`as` chain also available for RAC2 research, with SHA-256 respectively
`2ac3d8d3ca177e6705ac2cbdd1bd9e9a7181ac3e40f6230dea6875c3218ec155`,
`37704f483fba7269791576879b3573445cd455ce0473e6d58a3bcceb45105f95`,
and `c71a15dba889fc273b0b699986de7316d056b94ae2155ad2c2573eaa62b8d902`.
SN `ld.exe` SHA-256 was
`80f3724a63ce3c77c8b00d075f5c49448636518700ae251e5d6a6c809bbf5f48`.
These are **trial tool identities**, not a claim about the original retail
compiler. Do not distribute proprietary binaries.
The loop and combined twenty-function trials use the same `cpp`/`as`, but
the isolated RAC5 `cc1` SHA-256
`9614eceecbcb1f7a64cf3a4ad3c827b3e257ab6b1f934e42fcbc4ded0f0584d6`.
The first thirteen object hashes remain identical under it.

The original **individual slice** trials used archived diagnostic sources, with
`cpp -DRAC5_WITNESS_PADDING`, `cc1 -quiet -O2 -G0`, then `as`. These are historical
instructions, not the current contribution route; use `boot_trial_gate.py` for
the present public sources. The earlier trials linked each
resulting object with SN `ld -T config/boot/witness-*.ld`, matching
the C source and linker script names. Keep every object and ELF in ignored
`build/`. The `cc1` source and object must reside on the Linux filesystem if
your 32-bit chain cannot stat files under `/mnt/d`. Then run:

```sh
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/memcard_busy.elf 0x01E627C0 0x10
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/memcard_get_widescreen.elf 0x01E62838 0x10
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/mcpi_msgboxes.elf 0x01E64FE8 0x40
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/hig_io_read.elf 0x01E65458 0x20
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/mcpfont_string_length.elf 0x01E67978 0x40
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/mcp_string_length.elf 0x01E67EE8 0x40
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/camera_init.elf 0x01E68630 0x28
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/fast_mem_copy.elf 0x01E68EE0 0x20
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/mcptrans_is_complete.elf 0x01E68F00 0x38
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/mcptrans_should_fade.elf 0x01E68F38 0x10
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/rendereffect_reset.elf 0x01E7D450 0x10
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/buffer_cache_ptr.elf 0x01E82EC0 0xC
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/system_clear_color.elf 0x01E83DA0 0xC
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/anim_buffer_used.elf 0x01E8A1D0 0xC
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/globalwad_release_transition_bank.elf 0x01E8FE80 0x60
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/snd_unlock_positional_array.elf 0x01E91708 0x18
```

For the combined whole-boot witness, use the seventeen C sources **without**
`-DRAC5_WITNESS_PADDING` and with the isolated RAC5 `cc1`; compile them from
bare filenames on the Linux filesystem, and run `tools/boot_c_campaign.py` as documented in
[`BOOT-C-WITNESS.md`](BOOT-C-WITNESS.md#combined-provisional-c-witness).
Its pinned object hashes guard the exact trial inputs.

The `FOG_PreUpdate__Fv` control uses the isolated patched `cc1` described in
[`COMPILER-LOOP-TRIAL.md`](COMPILER-LOOP-TRIAL.md), then the same assembler
and `config/boot/witness-fog-preupdate.ld`. Its comparison command now exits
successfully:

```sh
python tools/compare_boot_slice.py baserom/SCUS_976.15 build/fog_preupdate.elf 0x01E7D4A8 0x40
```

The checker verifies the boot hash pinned in `config/target.json`, ELF
architecture and requested executable `.text` range, then reports only the
equal-byte count and candidate-slice hash. The complete boot text baseline
and the combined twenty-function C-object link with exact raw `PT_LOAD` bytes,
program header and ELF entry now exist; compiler/profile, reviewed function catalogue,
complete ELF metadata and runtime checks still prevent counted matches. No
ByteMatch report is warranted yet.
