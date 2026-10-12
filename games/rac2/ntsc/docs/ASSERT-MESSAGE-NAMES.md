# Assert-message carriers: 597 recorded annotations

Measured on 3 October 2026 against the USA v1.01 retail programs identified by
`config/target.json` and
`config/overlays.json`.

The second naming pass adds **495 function annotations, using 33 distinct labels,
across 25 level overlays**. Combined with the previous 102 entries, the public table
contains **597 unique `(program, address)` entries**. Every one of the 495 new entries
and every one of the 102 previous entries was compared with the current Ghidra function
name at the exact entry address; all 597 agreed. The two passes have no overlapping keys.

[`assert-message-names.tsv`](assert-message-names.tsv) contains only program identifiers,
virtual entry addresses and analyst-assigned structural labels. It contains no game
messages, extracted instructions or assets.

## First pass: the remaining 59 messages

**Historical investigation, 2–3 October 2026:** after the dispatch-table pass
described in [`MOBY-DISPATCH-TABLES.md`](MOBY-DISPATCH-TABLES.md), a selected set
of **59 diagnostic messages** remained without a carrying function in the analyzed
retail boot. The comparison used the retail target and the **9 September 2003 PAL
review disc**, whose boot identifier is `SCES_516.07`. These 59 cases were resolved
as a localization/classification task; that did not mean 59 additional EE functions
had been identified or that every diagnostic in either game image had been analyzed.

### Method and extraction correction

1. **Our own ISO9660 reader and level-archive extractor.** The September prototype's
   level table was located at `RC2.HDR + 0x5000 + i*0x3000`, with the level LBA at
   `+0x0004` and scene LBA at `+0x1804`. Its LBAs were checked against the actual
   ISO9660 directory. Each level archive contains chained `(dest_addr, copy_size,
   section_type, entry_point)` records followed by their content, covering `.lit`,
   `.bss`, `.data`, `lvl.vtbl`, `lvl.camvtbl`, `lvl.sndvtbl` and `.text`.
2. **32 of the 59 selected messages occur inside retail level overlays.** Their
   referencing functions were located in the corresponding retail programs already
   loaded in Ghidra. This explains why a boot-only investigation could not identify
   their overlay carriers.
3. **Naming guard.** The first pass changed only a generic `FUN_…` function receiving
   exactly one distinct proposed label from the selected diagnostic patterns.
   Existing names and functions receiving conflicting selected labels were preserved.
   This selection-based guard is not proof that a function references exactly one
   message across the entire image; the scripts searched the first occurrence of each
   selected pattern and followed Ghidra's existing references.

The first report recorded **27 levels and 57,070,528 extracted bytes**. This was an
incomplete extraction, corrected on **3 October 2026**: the section-header plausibility
test rejected a valid **8-byte section** because it required at least `0x10` bytes.
That stopped the chain before the following `.text` in **10 of the 27 levels**.
The corrected corpus has **27 levels, 189 sections (seven per level) and
75,369,728 bytes**, recovering **18,299,200 bytes**. The seven-section layout is
therefore the corrected result, rather than a property fully observed in the first
truncated corpus. The byte totals are retained here to explain the correction.

The missing content was level code. The diagnostic-message comparison was rerun on
the complete corpus and its previously reported counts were unchanged; it does not
turn the old incomplete extraction into a complete one retroactively.

### First-pass result: 102 annotations

**102 functions were named across 25 overlays.** Shared-library code appears as a
separate copy in each level, so one role can occur in several programs:

| Name | Functions |
| --- | ---: |
| `DirectionalLightsOverflow` | 25 |
| `CameraCollPrimTestGridOutOfBounds` | 25 |
| `LevelDoorLimitExceeded` | 25 |
| `NpcShowMessageBadId` | 10 |
| `PathSetWtoDistNullPath` | 7 |
| `ThermanatorBallsNeededReport` | 5 |
| `PathDrawInvalidIndex` | 2 |
| `MusicPitchDecreased` | 2 |
| `CutsceneFinished` | 1 |
| **Total** | **102** |

The historical persistence check reran the pass after saving: those functions were
reported as already named. On 3 October 2026, all 102 entries were also checked
against the current Ghidra function names. Their keys and the nine-name count table
remain present in [`assert-message-names.tsv`](assert-message-names.tsv), alongside
the second-pass additions.

**Thermanator cross-confirmation:** two related diagnostic messages are referenced
by `UpdateMoby_3212`, whose existing name was derived independently from the
prototype's class identifiers in the dispatch-table pass. Together, the identifier
evidence and diagnostic context support the identification of **class 3212 as the
Thermanator**. The function's existing dispatch name was preserved; the two
messages did not trigger two competing automatic renames. This corroboration
does not recover an original designer/source symbol for its implementation.

### The other 27 cases: evidence for IOP library data

The remaining **27 of the 59** selected messages were located in boot-image data
sections: `core.data` / `core.rdata` in the September prototype, with corresponding
retail boot data. The cluster contains identifiers associated with IOP-side
libraries, including `989snd`, `libcdvd`, `libdma`, `libpad2`, `libdbc` / `libmc`
and `SIF`, together with library-version records and `sceDbc*` / `SifDmaAddr`
identifiers. Their game text and version-banner contents are not reproduced here.

The historical controls were:

- **0/27** selected addresses recovered as targets of `lui`/`addiu` pairs in the
  retail boot `.text`, and **0/27** in the September prototype boot `.text`.
- Of **2,982 distinct addresses** constructed by those pairs in the prototype
  boot `.text`, the scanner recovered the known witness counts exactly:
  **119/119, 115/115 and 110/110**.
- The available Ghidra reference analysis reported no cross-reference to those
  27 strings.

Together, these observations support the classification as **IOP-library string
data packaged in the EE boot image**, rather than diagnostics attached to an
identified EE function. They are static evidence, not verification of a particular
module's upload or execution, and absence from this instruction-pair/reference
analysis does not exclude every indirect address construction. No EE function was
named from these 27 cases. Thus the original 59-case investigation accounts for
**32 overlay-localized cases plus 27 library-data cases**, without treating data
classification as source-code recovery.

### First-pass limits and provenance

The first pass preserved conflicting selected labels, including four debug-camera
controls, sound-mismatch cases and the Thermanator pair. A diagnostic-derived label
describes an observed relationship, not everything the function does. Its overlay
identifications used dispatch identifiers and diagnostic references; the first pass
made no byte-level prototype-to-retail code-correspondence claim.

The source of this historical pass was the **9 September 2003 PAL review disc
(`SCES_516.07`)**, read with our own tools, plus the pinned retail overlay programs.
The reverse-engineered archive format and the detailed working evidence remain in
the project notes. No original source symbols were recovered by this naming method.

## What the labels mean

These are annotations of functions that reference a diagnostic message. They are
**not original source symbols**, inferred function signatures, or proof that the
entire function implements the condition named in its label. A reference can occur in
one error branch of a much larger routine. The `Report` suffix describes the observed
diagnostic relationship; it does not establish that reporting is the routine's only job.

For example, level 0 entry `0x00369008`, currently labelled `TfragTextureOverflow`,
builds a DMA chain and checks a threshold within that work. Level 0 entry `0x00321878`,
labelled `BoltsSpawnNegativeCount`, contains a larger spawning routine. These labels
are useful search anchors and should be refined only after reviewing the whole body.
They must not be promoted directly to decompilation API names.

## Second pass: 495 additions and current checks

The private second-pass journal records 495 successful renames and zero failures.
Its 495 rename records have 495 distinct program/address keys. The input selection
has 33 diagnostic patterns, each associated with one analyst label. The pass visited
the boot and 25 overlays; the boot contributed **zero new entries**, because its
relevant functions already had names. Thus 26 programs visited is compatible with
25 programs contributing to this export.

The naming script preserved existing non-generic names and rejected a function when
the selected patterns supplied more than one different label. Its search used the
first matching occurrence of each selected pattern in a program and followed existing
Ghidra references. This is a narrower property than proving that the function contains
exactly one diagnostic string in the entire binary. The export verification establishes
name fidelity; it does not repair missing references or certify every semantic decision.

The previous pass contributed 102 annotations across 25 overlays. Its independent
class-identifier cross-check remains documented in
[`MOBY-DISPATCH-TABLES.md`](MOBY-DISPATCH-TABLES.md). Functions with conflicting
diagnostic labels and names already assigned by another method were preserved.

No game text is necessary to reproduce the application step. The privately owned
retail image and analysis database remain the source for semantic review.

## Applying or verifying the table

[`ApplyAssertMessageNames.java`](../tools/ghidra/ApplyAssertMessageNames.java)
accepts a program identifier and the TSV path. It previews by default; the explicit
third argument `--apply` enables renaming. It checks the complete level `.text`
SHA-256 against its recorded retail fingerprint before processing names, resolves a
virtual-address or raw-file-offset import only when exactly one mapping matches,
requires a function at the exact entry address, and never overwrites an existing
non-generic name. Conflicting rows fail before any renames.

Example arguments for the level 0 overlay:

```text
0_aranos_tutorial docs/assert-message-names.tsv
```

With `--apply`, review the summary and save the program explicitly after inspecting
the changes. The script does not create functions, infer signatures, touch the boot,
transfer prototype addresses, or apply the cross-build anchor survey.

## Separate cross-build evidence

The Aug6/retail survey is qualified separately in
[`AUG6-RETAIL-ANCHORS.md`](AUG6-RETAIL-ANCHORS.md). Matching code windows, diagnostic
annotations and class-table identifiers are different forms of evidence. None of them
alone establishes a byte-exact C reconstruction or a playable runtime.
