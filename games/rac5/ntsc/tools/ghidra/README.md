# Optional independent R5900 boundary pass

`ExportFunctionBounds.java` exports only function-entry and body-boundary
metadata from a private Ghidra R5900 analysis. It writes no disassembly or
game bytes. Ghidra and a compatible R5900 language extension must be
installed separately; neither is distributed by this repository.

First run `python tools/level_function_inventory.py --write
baserom/SCUS_976.15.iso`. It verifies the pinned image and creates private
`build/level-functions/*.code.bin` slices for all 21 modules. Keep those files
and the Ghidra project local. Example PowerShell headless invocation from the
repository root (replace the install and private-project paths):

```powershell
$repo = (Get-Location).Path
$inputs = Get-ChildItem build/level-functions -Filter '*.code.bin' -File |
  Sort-Object Name | ForEach-Object FullName
$rac5HeadlessArgs = @('D:\private\ghidra-projects', 'rac5-levels', '-import') +
  $inputs + @('-loader', 'BinaryLoader', '-loader-baseAddr', '0x80',
    '-loader-blockName', 'code', '-processor', 'r5900:LE:32:default',
    '-cspec', 'default', '-scriptPath', "$repo\tools\ghidra",
    '-postScript', 'ExportFunctionBounds.java', "$repo\build\level-functions",
    '-analysisTimeoutPerFile', '180', '-max-cpu', '4')
& 'C:\path\to\ghidra\support\analyzeHeadless.bat' @rac5HeadlessArgs
```

The script writes `<module>.code.bin.ghidra.tsv` in the ignored build
directory. Auto-analysis can misidentify embedded data and cannot read the
data sections omitted from these code slices; warnings about unreadable data
addresses are expected. Its boundaries are an **independent candidate view**,
not ground truth. Add the optional second script argument `ranges` to export
Ghidra's noncontiguous body-address ranges for private investigation. The
committed cross-check uses the default five-column form. Compare it with the
static call/relocation analysis using:

```sh
python tools/level_ghidra_crosscheck.py --write
python tools/level_translation_families.py --write baserom/SCUS_976.15.iso
```

For the optional flow-range review, rerun `-postScript` against each saved
Ghidra program with two script arguments: a private
`build/level-functions/ranges/` output directory and `ranges`. No reanalysis
is needed (`-process <program> -noanalysis -readOnly`). Then run
`python tools/level_flow_ranges.py --write`; its detailed worklists stay ignored.

Both commands commit only compact metadata summaries when the maintainer
chooses to publish them; detailed disagreement and group worklists remain
under ignored `build/level-functions/`.

## Full-module flow review

The code-only import cannot recover references from the SNR2 data region to
switch blocks. For an additional **private, independent review**, import the
complete pinned REL files into a *new* Ghidra project. The pre-script
[`PrepareFullModule.java`](PrepareFullModule.java) marks only the pinned code
interval executable; the header and data stay non-executable. This is still
auto-analysis, not a retail function catalogue.

```powershell
python tools/level_full_module_flow.py --extract baserom/SCUS_976.15.iso
$repo = (Get-Location).Path
$ghidraOut = "$repo\build\level-functions\full-ghidra"
New-Item -ItemType Directory -Force -Path $ghidraOut | Out-Null
$inputs = Get-ChildItem "$repo\build\level-functions\full" -Filter '*.REL' -File |
  Sort-Object Name | ForEach-Object FullName
$rac5FullArgs = @('D:\private\ghidra-projects', 'rac5-full-modules', '-import') +
  $inputs + @('-loader', 'BinaryLoader', '-loader-baseAddr', '0x0',
    '-loader-blockName', 'module', '-processor', 'r5900:LE:32:default',
    '-cspec', 'default', '-scriptPath', "$repo\tools\ghidra",
    '-preScript', 'PrepareFullModule.java', "$repo\config\levels\code_intervals.tsv",
    '-postScript', 'ExportFunctionBounds.java', $ghidraOut, 'ranges',
    '-postScript', 'ExportCandidateFlowRefs.java',
      "$repo\build\level-functions\ranges", $ghidraOut,
    '-analysisTimeoutPerFile', '180', '-max-cpu', '4')
& 'C:\path\to\ghidra\support\analyzeHeadless.bat' @rac5FullArgs
python tools/level_full_module_flow.py --write baserom/SCUS_976.15.iso --ghidra-dir $ghidraOut
python tools/level_full_module_flow.py --check
```

The reference export contains addresses and reference kinds only. The Python
audit cross-checks data references against the pinned packed 32-bit pointer
fixups before treating them as evidence. Computed jumps inside a Ghidra body
and cross-body calls to newly identified entries are separate review classes.
The raw BinaryLoader also creates one synthetic header entry at offset zero;
auto-analysis may propose spurious functions in non-executable data. The audit
counts and excludes these from code ownership, and rejects any function whose
body crosses the pinned code boundary.
The full REL files and detailed exports remain ignored; only compact counts
and input hashes may be committed.

With both code-only and full-module range exports available, the optional
split-boundary audit reuses them without another Ghidra run:

```sh
python tools/level_split_dedupe.py --write baserom/SCUS_976.15.iso --ghidra-dir build/level-functions/full-ghidra
python tools/level_split_dedupe.py --check
```

It requires two agreeing body analyses, a real return at each selected body
end, and zero-only *internal* gaps without relocation sites. Other parent
bytes remain unique. Only metadata summaries are tracked.
