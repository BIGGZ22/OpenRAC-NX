# SNR2 function-boundary and shared-code audit

[`tools/level_function_inventory.py`](../tools/level_function_inventory.py)
analyzes all 21 hash-pinned USA `LVL/*.REL` modules. It gathers possible
function starts from direct `jal` targets, the exported `lib_main` entry and
code addresses in packed local pointer relocations. It partitions every
candidate code interval between those starts, checks `jr $ra` return/delay
slots and zero tails, and rejects a clean boundary when a direct branch or
jump enters its interior from another interval. This is a **candidate-entry
inventory**, not a declaration that every row is a real function. Pointer
tables can contain jump-table basic blocks; valid instructions can also occur
in embedded data.

The pinned private run produced:

| Classification | Candidate intervals |
| --- | ---: |
| Direct-call/export start with bounded return and zero tail | 74,051 |
| Relocated pointer start with bounded return and zero tail | 22,321 |
| No return before the next candidate start | 19,510 |
| Nonzero bytes after the last observed return | 8,474 |
| **All candidate entry intervals** | **124,356** |

The 74,051 stronger bounded candidates cover **26,702,516 body bytes by
placement**. The full candidate code intervals cover **36,341,888 bytes by
placement**. Their gap is not automatically unimplemented code: it includes
uncertain function bodies, alignment and possible embedded data.

For every bounded candidate, the tool records a raw body SHA-256 and a second
SHA-256 after masking only fields at measured relocation sites. The masked
hash also includes each site's offset, relocation kind, named symbol and
addend. This exposes code that moves between levels without publishing code
bytes. **Masked equality is only a review lead**: two different globals or
callees may have the same masked instructions. Groups containing multiple
occurrences in one module are especially ambiguous and are not collapsed in
the conservative lead measure.

Raw, byte-identical bounded bodies account for **23,765,324 distinct candidate
bytes**, compared with 26,702,516 placement bytes. The strictly exact audit
also drops the complete identical code interval of `LEVEL_21.REL` after
keeping `LEVEL_15.REL`, then deduplicates non-overlapping raw-identical
bounded bodies only when a group occurs at most once per retained module.
That leaves an **upper bound of 33,168,892 candidate interval bytes**. It is
still an upper bound because relocated addresses can hide more shared code,
and the input intervals may contain data. The much smaller **4,106,316-byte
relocation-masked conservative lead** applies only to the stronger bounded
bodies and is *not* a proven lower bound or a report denominator.

With your lawful pinned USA disc, run:

```sh
python tools/level_function_inventory.py --write baserom/SCUS_976.15.iso
python tools/level_function_inventory.py --check
python tools/progress_status.py --write
```

The tracked [`function_inventory_summary.json`](../config/levels/function_inventory_summary.json)
contains metadata and counts only. Detailed per-module TSVs and
`exact_duplicate_groups.tsv` / `masked_review_leads.tsv` stay under ignored
`build/level-functions/` because they are large, generated research worklists.
`--check` validates the public snapshot and its source-catalogue hashes without
the game; the private `--write` run is needed to reverify the analysis itself.

Before a game-wide [ByteMatch objdiff report](https://github.com/Veradictus/ByteMatch/blob/master/docs/getting-started.md)
is published, the uncertain rows need function/control-flow review and masked
families need target-equivalence checks. No exact unique-code denominator or
game-wide percentage is claimed from this audit.

## Independent function-boundary cross-check

A separate private Ghidra 12.1.3 project imported all 21 pinned candidate
code slices at module offset `0x80` with an R5900 language extension. The
authored [`ExportFunctionBounds.java`](../tools/ghidra/ExportFunctionBounds.java)
exports only entry/body metadata. The
[`level_ghidra_crosscheck.py`](../tools/level_ghidra_crosscheck.py) tool
requires the Ghidra body to be contiguous from the same candidate entry and
to end at the independently inferred return boundary. A nonzero tail after
that boundary stays unclassified but does not invalidate the agreed body.
The identical
`LEVEL_15`/`LEVEL_21` code intervals produced identical Ghidra export hashes,
an additional repeatability control.

Across all modules, **91,350 candidate boundaries** and **25,823,716 body
bytes by placement** agree exactly between the two methods. Ghidra alone
proposed **2,689** entries; the static call/pointer audit alone proposed
**27,692**. Of the agreeing bodies, **7,891** have a nonzero tail beyond the
return and are kept in a separate review class. The differences are private
review worklists. Ghidra's raw-code
import excludes module data and can miss calls or misclassify embedded data;
agreement is corroboration, not proof that every other address is non-code.
Among 1,655 shared entries with no return before the next candidate start,
**1,634** have a Ghidra body that is noncontiguous or crosses that next start.
These cannot be sized correctly by a simple next-entry rule; they need a
control-flow/body-range review. The remaining 21 appear as short terminal
call/tail forms and are also left unaccepted.
An optional body-range export now records Ghidra's actual address sets rather
than only minimum and maximum addresses.
[`level_flow_ranges.py`](../tools/level_flow_ranges.py) finds **32,805,452
bytes by placement** assigned to Ghidra function bodies and **1,590 static
candidate starts inside another Ghidra body**. Of these, **1,209** also have a
direct non-call branch/jump from that same Ghidra body to the candidate start.
This independently supports an internal-block interpretation; the other **381**
have no such direct edge and remain particularly important for indirect-flow
and table review. Neither class is automatically removed from the candidate
inventory. A code-only
Ghidra import can also leave real address-taken code unowned; the range pass
does not prove an exhaustive function inventory.
See [`tools/ghidra/README.md`](../tools/ghidra/README.md) for reproducing the
analysis with a lawful image and local Ghidra installation.

### Full-module flow review

An isolated second Ghidra pass imported each **complete pinned REL** while
marking only the known code interval executable. Unlike the code-only import,
this exposes data pointers and some computed jump/call references. The
[`level_full_module_flow.py`](../tools/level_full_module_flow.py) audit verifies
the pinned module hash and relocation sites, checks computed-flow instruction
forms, and assigns all **1,590** previously inside-body candidate starts to
review categories:

| Full-module evidence | Candidate starts |
| --- | ---: |
| New Ghidra function entry with a cross-body call | 21 |
| Packed data pointer and same-body computed jump | 1,285 |
| Same-body direct control-flow edge | 34 |
| Inside the newly identified called function's body | 21 |
| Contiguous predecessor with no terminating transfer | 229 |

Among the **381** starts that lacked a direct edge in the first pass, the
corresponding counts are **21, 110, 0, 21 and 229**. The 21 called entries are
one per module; they were hidden by the code-only function grouping. These
categories settle which evidence to review for each disagreement, but do not
promote Ghidra's auto-analysis to accepted retail function boundaries. The
229 linear cases still lack a recovered indirect reference and need particular
care before any function split. The full import also invented **21** synthetic
header entries and **5** functions in non-executable data; these 26 are
excluded. The full-module function and reference exports for identical
`LEVEL_15`/`LEVEL_21` intervals have identical hashes, a repeatability check.
All complete REL files and detailed worklists remain private. The tracked
[`full_module_flow_summary.json`](../config/levels/full_module_flow_summary.json)
contains only counts and hashes, never program bytes. See the
[reproduction commands](../tools/ghidra/README.md#full-module-flow-review).

## Structural translation leads

[`level_translation_families.py`](../tools/level_translation_families.py)
filters masked groups to one placement per module, requires every boundary
to agree with Ghidra, preserves named relocation symbols and addends, and
requires local HI16/LO16 data references to move by one consistent
module-specific delta when the body has HI16 data references. If two data
regions move by different offsets, an alternate review path requires an
identical **64-byte data window unique within each module payload** at every
corresponding HI16 target; unpaired LO16 values must follow one of the
verified low-half shifts. With no HI16 targets, the data shift is zero and
unpaired LO16 values must still agree. A further
closed local-callgraph pass requires each
J/JAL target either to be the same relative offset within the body or to
identify a Ghidra-corroborated callee family already accepted by those rules.
Unknown code targets, 32-bit pointers, differing target families and open
callgraph dependencies remain excluded. The audit also considers pointer-only
and nonzero-tail bodies when their complete body boundary agrees with Ghidra;
their uncertain surrounding bytes are not included. **2,415 groups** passed
the combined structural checks, including **958** closed local-callgraph
groups, **496** groups involving a pointer-only or nonzero-tail placement, and
**36** groups requiring unique data windows. They contain **8,146,612**
placement body bytes but only **687,372** body bytes
when one representative per group is counted. Their additional candidate
savings beyond raw-identical groups are **5,444,816 bytes**, giving a
**27,724,076-byte conditional structural-lead scenario** over the original
candidate intervals. It assumes those leads really share code; this number is
*not* a proved upper bound on unique game code or a progress denominator.
The saving uses connected components of exact and structural body links, so
overlapping evidence cannot subtract a placement twice.

The largest lead has a **32,796-byte** body in all twenty numbered levels.
Ghidra agrees with its boundary in every placement. Each placement has the
same 82 named call relocations, while all 47 local HI16 data references
translate by one consistent offset for that level; the remaining unpaired
low reference follows the same offset modulo 16 bits. This strongly supports
one shared code template, while the game-level role and source identity still
require review. Local code targets that do not satisfy the callgraph closure,
nonuniform data translations and uncertain function ownership remain in the
unresolved worklists. This pass narrows disagreements; it does not finish the
game-wide function catalogue.

## Splitting oversized candidate intervals

Some static next-entry intervals appear to contain several functions. The
[`level_split_dedupe.py`](../tools/level_split_dedupe.py) review only splits an
interval, including pointer-only and nonzero-tail parents, at each selected
sub-body for which the code-only and full-module
Ghidra analyses agree, its end contains `jr $ra` plus the delay slot, and any
gap *inside that sub-body* contains only zero bytes with no relocation site.
Bytes outside selected sub-bodies are left unique even if they look similar.
The pinned private pass found **3,729 parent intervals by placement**
(6,813,016 body bytes) with **6,245** qualifying segments spanning 6,518,180
bytes; the other **294,836 body bytes** stay uncollapsed. Reference/translation and
callgraph checks closed **176** split-segment families plus **243** existing
static families whose calls now resolve to those segments. Another 131 raw-
identical split groups served as target controls. Detailed addresses remain
in ignored worklists.

For relocations into initialized data, the split audit also accepts the
unique, matching 64-byte windows used by the static audit. All-zero windows
need different evidence: a target within 32 bytes must have two distinct
already accepted families pointing to it at the same relative sites in both
modules, each edge must identify just one target per module, and the full
span between target and anchor must remain zero. This additional conditional
rule resolves **80 groups** spanning **915,944 placement bytes**. It uses
earlier structural families only as anchors; neither the anchors nor these
new groups are certified source-level equivalents. The private
`split_translation_review_leads.tsv` lists every newly closed family.

When some placements in a masked family have different data objects, the
audit tries a pairwise-compatible subgroup instead of rejecting all of them.
The local-callgraph closure then requires the callee to be corroborated in
**every module occupied by its caller**. Of 162 provisional subgroup leads,
31 pass this closure, covering 455,924 placement bytes. One 796-byte family
closes in 15 levels; its 9,196-byte caller closes in 13 of those levels. The
other five callee placements remain outside that subgroup. This keeps the
partial family from being treated as a shared source in levels where its data
identity still differs.

The audit removes each selected parent from exact duplicate grouping before
adding its disjoint verified segments, while leaving the residual bytes
unique. It then recomputes connected components of exact and structural links.
Exact candidate savings rise from **1,846,532** to **3,019,468 bytes** and
combined candidate savings from **7,291,348** to **11,543,440 bytes**. This
yields a **23,471,984-byte conditional candidate scenario**, 4,252,092 bytes
below the prior scenario. The calculation never counts a parent and its split
segments at the same time. Ghidra-derived starts and structural equivalence
remain review leads, so this is **not** a qualified unique-code denominator or
a ByteMatch percentage. The compact
[`split_dedupe_summary.json`](../config/levels/split_dedupe_summary.json)
records counts and input hashes only.

## Contiguous switch-body review

Some statically labelled `no_return` entries are dispatchers for pointer-only
case labels rather than separate tiny functions. The
[`level_switch_bodies.py`](../tools/level_switch_bodies.py) audit accepts a
contiguous full-module Ghidra body only when it starts at a direct-call entry,
ends in `jr $ra` and its delay slot, contains only pointer-labelled static
entries, and has no competing Ghidra function, external direct edge into an
interior block, or overlap with a selected split parent. Any code-only Ghidra
body at the start must agree on the full outer range. The pinned private pass
finds **1,117 placements** comprising **1,784,320 body bytes**. Ninety-eight
relocation-masked groups recur between modules, with 1,539,908 possible
placement-byte savings before data and callgraph checks. Their private
worklist is pinned by [`switch_body_summary.json`](../config/levels/switch_body_summary.json).
This is function-ownership evidence, not a counted C match.

## Optimistic relocation-masked shape scenario

The separate [`level_shape_scenario.py`](../tools/level_shape_scenario.py)
audit answers how low the candidate-byte count could go if every body with
the same relocation-masked instructions and fixup descriptors shared a code
shape. It rehashes every static, selected split and reviewed switch body
directly from the pinned private ISO. A parent or switch case label is removed
before its disjoint replacement is added. Of **35,015,424** retained
candidate-interval bytes, **33,594,892** are represented by 103,037 body
units; the other **1,420,532** stay unique.

The 103,037 units yield **9,388 masked shapes**. Keeping as many instances of
each shape as the maximum present in any one module gives an optimistic
**5,756,360-byte scenario**. Counting only one instance per masked shape would
give **5,637,892 bytes**, but 337 shapes repeat within a module, so that lower
figure may collapse distinct placements. Using raw bytes instead of masked
relocations gives 31,627,388 bytes under the same maximum-per-module rule.
These are alternative *shape-counting assumptions*, not bounds that prove
source equivalence. Matching masked code can still refer to different data,
contain uncertain boundaries or require distinct C definitions. The
[`shape_scenario_summary.json`](../config/levels/shape_scenario_summary.json)
is deliberately separate from the conditional structural review and from
qualified C progress; none supplies a ByteMatch denominator yet.

## Repeated shapes with code context and data anchors

The [context-translation audit](../tools/level_context_translation.py)
examines functions whose masked instruction shape recurs within a level. It
replaces reviewed split parents with their segments and contiguous switch
bodies with one function unit, preserving exact-hash gap markers for unselected
bytes, then compares a candidate's immediate preceding and
following code shapes across modules. A context must occur once per module.
Static bodies normally need the code-only Ghidra boundary. When that pass
has no function at a direct-call entry, the audit also accepts a matching
full-module Ghidra body; a contradictory code-only function still rejects the
candidate. It also admits called `nonzero_tail` bodies when full-module Ghidra
confirms the precise body, leaving their trailing bytes separate. This
hash-pinned second path admits 2,144 candidate placements
(356,256 body bytes) across the retained modules before data and call checks.
Split segments and switch bodies inherit their separate code-flow and
full-module Ghidra reviews. Both
placements need
the same local-call shape; every external local callee must already have an
accepted placement in both modules or be independently paired in the same
context graph. A placement pair is admitted only after all of its callee pairs
close, so a caller cannot borrow a callee from a different level. Data targets
encoded as local HI/LO pairs into code are treated as function dependencies:
both halves must lie inside the body, the target must be a reviewed function
entry, and the final global component must contain both target placements.
Other HI/LO pairs are checked as data targets. An exact 64-byte window may be
unique after the target or centered on it. A short zero run can be identified
by its unique matching nonzero flanks and the target's offset inside it. When
the following bytes differ, one uniquely mapped reference within the same
short run can also fix the target's offset if at least two independent accepted
families support that reference. For larger zero regions, two distinct
accepted-family references within 1 KiB
must uniquely map with the same shift, use at least two independent families,
and have only zero bytes between each reference and the target in both modules.
Otherwise, a target needs an unambiguous accepted-family reference or matching
shifts bracketed within 64 KiB by two independent accepted families.
For 38 retained sites, the loader's HI16 lookup crosses a function boundary;
the audit uses a local `lui`/delay-slot `addiu` pair only when both sites carry
the expected relocations, the register matches, and the local low immediate
equals the low immediate selected by the packed loader stream.

Using levels 01, 16 and 08 as independent references across the **19 retained
numbered levels**, the callgraph closes 5,919 provisional context families.
The global union rejects 138 that would connect two distinct placements from
the same module. Removing those links can invalidate callers: five further
passes remove 1,959 caller placements and 150 families whose callees no longer
share a final component. The result is **5,631 conditional families**, of
which 62 include reviewed switch bodies, spanning **18,604,584 bytes by
placement**. Independent audits find no remaining same-module collision or
uncovered local callee or code pointer. Of 96,341 context/data candidate pairs, 62,676
initially close in the callgraph; 31,430 of those
need a paired callee first. The data
window cache is keyed by module as well as address, so reused offsets in
different levels cannot share a signature accidentally. Unioning these links
with exact and previously accepted structural groups avoids double counting.
It adds **6,614,624 bytes of candidate savings** and reduces the conditional
scenario from 23,471,984 to **16,857,360 bytes**. These links remain
conditional until shared source and the full denominator are qualified.
The private worklist and pinned ISO/Ghidra exports allow the pairings to be
reproduced. Code context and data anchors still do not establish shared C
source for every placement; this is not a qualified denominator.

## Site-paired code-role review

The [role-graph audit](../tools/level_role_graph.py) tests a broader form of
structural sharing across the 20 retained modules. The 13 reference modules
listed in the summary reproduce the same candidate savings as a private run
using all 20, while reducing the cost of regeneration. Each proposed pair has
the same relocation-masked
body and an independently reviewed boundary. Its data targets require the
previously accepted data witnesses; internal J/JAL targets must have identical
body-relative offsets, and external code-relocation sites must correspond.
Matched callees of the same body shape close recursively, including mutually
dependent pairs. A callee with different code may occupy the same **call
role** only when both boundaries are reviewed. Noncontiguous called roles need
matching relative ranges and agreement between code-only and full-module
Ghidra exports. Code pointers into an unresolved static interval must identify
the same offset within a parent of the same masked shape.

A global union rejects 473 pairs that would join two placements from one
module. It then removes 11 pairs with ambiguous code-role mappings and 1,048
callers whose dependencies lose their final component over seven rounds. The
remaining **141,319 site-paired leads** add **6,768,700 bytes** of conditional
candidate savings beyond the stricter context audit, yielding a
**10,088,660-byte role-mapped scenario**. An independent private audit rehashes
24,781 participating placements from the pinned modules and checks their data,
code edges, roles and final union. The generated private worklist and public
summary retain the input hashes for reproduction.

These are normalized machine-code relationships, not proof that variant
callees have the same source. The game-wide function catalogue, source
equivalence and C gates remain incomplete. Neither this scenario nor the
5,756,360-byte optimistic shape scenario is a qualified ByteMatch denominator
or percentage.
