# Reproducible boot C trial gate

The pinned [compiler trial](../config/boot/compiler_trial.json) identifies the
locally reconstructed GNU EE `cpp`/`cc1`/`as` binaries by SHA-256, the
RAC5-only short-loop patch, and the exact `-O2 -G0` flags. It is accepted
**only for its reviewed boot placements** under the full loaded-byte criterion
([scope](BOOT-MATCH-SCOPE.md)); the original compiler is not identified.
The reviewed sources are in `src/boot/`, the profile lists them explicitly in
`accepted_sources`, and the campaign enumerates the same set in
`ACCEPTED_SOURCES`. Every other body remains under `nonmatching/`.

The pinned 2.9 preprocessors read a line continuation as a backslash followed
by a line feed, so a CRLF checkout turns it into `stray '\' in program` on any
body with a macro continuation. `.gitattributes` pins `*.c` and `*.h` to LF for
that reason; a Windows checkout without it cannot reproduce the gate.

The current campaign contains 635 compiler-produced C objects. Its verified
catalogue records 635 global function bodies (80,548 bytes plus 1,228 zero alignment
bytes); no object span is left without a usable global-function extent.
These counts are distinct from the 605 accepted boot placements (74,760 bytes).
Per-source compiler and options measured for one file are listed in
[`config/cflags.txt`](../config/cflags.txt) and applied by the gate; the profile
keeps its global flags. 158 entries hand their file to another pinned compiler:
twenty-nine to SN ProDG 3.01 `ee-gcc2953`
([route P](COMPILER-STATUS.md#a-second-pinned-compiler-route-p)), 116 to the
Sony SDK compiler `2.9-ee-991111`
([route Q](COMPILER-STATUS.md#the-sony-sdk-compiler-route-q)) and ten to its
`div.s`-padded profile, and three to the pre-patch GNU trial profile ([third chain](COMPILER-STATUS.md#a-third-gnu-chain-the-pre-patch-trial-cc1)); 72 entries also select another pinned assembler.

Both original and preprocessed C inputs must pass `tools/c_source_policy.py`:
no instruction assembly, fixed registers or raw instruction words. The standalone
linking campaign also checks its source inputs. Hybrid experiments are kept
privately and excluded from the C substitutions; their original intervals stay
in the assembly baseline. C may still call platform services normally. A PC port
will need platform-specific service implementations, a separate task from proving
the original PS2 bytes.

With your own lawful copy of the pinned USA boot and matching local tools,
run this in WSL from the repository checkout. The compiler intermediates must
be on WSL's **Linux filesystem**; the SN executables can run through WSL's
Windows interop when that is available:

```sh
python3 tools/boot_trial_gate.py baserom/SCUS_976.15 \
  --cpp /path/to/cpp --cc1 /path/to/cc1 --as /path/to/as \
  --assembler /mnt/c/path/to/Ps2EeAs.exe \
  --linker /mnt/c/path/to/ld.exe \
  --extra-bin prodg2953=/mnt/c/path/to/ProDG/usr/local/sce/ee/gcc/bin \
  --extra-bin sdk991111=/home/you/rac5-sdk-compiler \
  --output-dir /home/you/rac5-boot-trial
```

The command checks the patch and tool identities, copies each public C source
to the Linux work directory, compiles **every campaign object**, and requires
their object hashes to match the measured campaign. Six objects assembled by
`as=ps2eeas` have two explicitly pinned whole-object SHA-256 values. A private
comparison of all 543 freshly compiled objects with their previously pinned
versions found exactly one differing byte per variant, always inside
non-loadable `.mdebug`; `.text` and relocations are unchanged. Both full-object
hashes for each affected source are listed in `config/boot/compiler_trial.json`,
and any third value is rejected. No
compiler output is edited or normalized to make this pass. `--extra-bin
SELECTOR=DIRECTORY` gives the directory holding one pinned extra compiler,
named by the selector that uses it and required exactly when the campaign's
`config/cflags.txt` selects it; the gate checks that compiler's driver,
`cpp` and `cc1` hashes and keeps every other object on the pinned chain. Each
pin states the host its driver runs on. SN ProDG 3.01 (`prodg2953`) is a
Windows driver: it is staged in the Windows-visible ignored
`build/boot-prodg-stage/` and reached through WSL's Windows interop. The Sony
`2.9-ee-991111` SDK compiler (`sdk991111`) is a Linux driver, and its 32-bit
front end cannot stat a DrvFs path, so it is staged under `--output-dir`,
which must therefore be on the Linux filesystem for either route; a pin may
also name its own staging directory, or `--extra-stage SELECTOR=DIRECTORY`
overrides it. Either driver's output is assembled by the same pinned GNU `as`
as every other object, so one assembler still produces the whole campaign. It
exports only the objects to
ignored `build/boot-compiled-objects/`, then checks the committed boot symbol
and DVP metadata, links the objects into the complete private boot assembly,
and rechecks every candidate body boundary the catalogue records. It requires exact boot
`.text` (**498,900/498,900**), raw `PT_LOAD` (**809,872/809,872**), entry,
program header and BSS placement. It then regenerates the ELF container from
[`config/boot/elf_layout.json`](../config/boot/elf_layout.json) and the DVP
catalogue, and requires exact full-file SHA-256 (**856,120/856,120 bytes**).
Witness counts come from the verified boundary catalogue, not a second frozen
compiler-profile counter. The `exit` column records the observed terminal transfer:
`call` does not establish that its callee never returns or qualify a function boundary.
The non-code loaded sections still come
from the user's pinned boot; generated game-derived material and tool binaries
are never committed.

The result is in ignored `build/boot-trial-gate/report.json`. A passing report
has `status: "passed_boot_only"`, `raw_load_and_header_exact: true`,
`whole_elf_exact: true`, `counted_matches: 605` and `counted_bytes: 74760`.
The counted bytes are each accepted source's compiled `body_bytes`. A complete
body may have one following zero word supplied by the linker's `ALIGN(8)`;
the boundary catalogue verifies the retail zeros and direct-flow/label safety,
and that word earns no C credit. A source that covers only a prefix of the
function or lacks a reviewed normal return keeps no acceptance.
It also records a `promotion_input_sha256` fingerprint so the
[per-body promotion audit](BOOT-PROMOTION.md) can reject an old receipt after
source, owner, compiler-profile or boundary changes.
Each run overwrites an old report with `status: "incomplete"` before checking
tools, so a failed run cannot leave a stale pass. The packer also removes its
old output before rebuilding. The original, unpatched `cc1` was rejected;
a one-byte-corrupted linked ELF was also rejected without leaving an exact
output. A rerun with the pinned tools passed.

This gate makes the limited boot build **repeatable and falsifiable**. The
accepted scope is the pinned boot and its reviewed placements; it does
not prove original compiler identity, other boot functions, level programs,
or full gameplay. The [DVP and section-layout gap](DVP-OVERLAYS.md) is
resolved by packaging; the original loaded non-code sections are still inputs.
The private report is not yet a ByteMatch percentage. Any later source or toolchain
change must rerun this gate; a failure withdraws the match until repaired.

An earlier isolated PCSX2 smoke test of a stripped ELF linked from all twenty C
witnesses and the pinned disc reached the frontend module's `SNR2` header at
EE address `0x00F97B80`; its first 32 bytes there matched a retail run. The
stripped ELF reported CRC `FA686353`, versus `8661F7BA` for retail. This
confirms boot-to-frontend progress only. The newly packaged ELF is file-exact
and has the measured retail CRC `8661F7BA`, but full gameplay has not been retested.
The isolated emulator was stopped afterward.

The gate is byte-exact under either route state: disabling the second compiler
(compiling its five sources with the pinned chain instead) leaves every
pre-existing object hash unchanged but makes those five objects differ from
their pins, so `compile_boot_trial.py` refuses them.

The first acceptance was `MEMCARD_Busy__Fv`: one measured boot placement,
a 16-byte C body, no trailing alignment and an exact complete boot ELF. Thirty
more bodies were accepted after their own boundary, symbol, caller and behaviour
review, each supplying its whole owned range. Their acceptance rationale and
rollback rule are in [`BOOT-MATCH-SCOPE.md`](BOOT-MATCH-SCOPE.md). The remaining
linked C bodies stay provisional until each receives its own review, even though
they participate in this exact load-image build.
