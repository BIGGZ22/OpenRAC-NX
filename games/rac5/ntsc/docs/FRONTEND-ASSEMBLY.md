# FRONTEND.REL assembly interval, USA PS2

The pinned `LVL/FRONTEND.REL` has a **candidate code interval** from module
offset `0x80` through `0x27CFF` (end-exclusive `0x27D00`). All **162,944 / 162,944
bytes** in that interval were reassembled from private R5900 instruction
mnemonics and compared directly with the original, unrelocated REL bytes.
The linked candidate interval SHA-256 is
`7a20f37eb7d3658d34f89fa56bb44d7263d8d7a9b3beb86cca577647d0309828`.
No game-derived assembly, REL, object or ELF is committed.

Evidence for this boundary is stronger than instruction validity alone. The
header field at `0x28` is `0x80`; the last observed control-transfer delay
slot is followed by 18 zero instruction words through `0x27CFC`; the word at
`0x27D00` does not decode as a valid R5900 instruction. All branches in the
interval target locations inside it. Most decisively, the **2,470**
nonzero-target jump sites are exactly the 2,470 packed local 26-bit fixup
sites, while the **434** zero-target jump sites are exactly the 434 named
type-4 relocation sites. The two site sets have no missing or extra entries.
The 434 zero targets remain import placeholders until the SNR2 loader applies
its named relocations.

[`tools/frontend_assembly.py`](../tools/frontend_assembly.py), a compatibility
entry point for the shared [`tools/snr2_assembly.py`](../tools/snr2_assembly.py), verifies the
complete USA disc and frontend module hashes, both jump-site sets, and the
measured SN tool hashes. It decodes 40,736 words with optional
`rabbitizer==1.16.2`, including three R5900 `sqrt.s` forms that need an
explicit decoder mapping. Unlike the boot assembly baseline, this interval
needs **no raw instruction-word fallback**. SN `Ps2EeAs.exe` and `ld.exe`
build it at its module-relative address `0x80`; all generated files remain
under ignored `build/snr2-assembly/frontend/`. Run with your own lawful USA image
and locally available measured tools:

```sh
python tools/frontend_assembly.py baserom/SCUS_976.15.iso \
  --assembler /path/to/Ps2EeAs.exe --linker /path/to/ld.exe
```

This establishes an exact **pre-relocation interval**, not a complete
FRONTEND.REL assembly baseline or a function catalogue. The header, data,
symbol tables, packed stream and any other bytes outside this interval are
still taken from the original module. The independent
[relocated-image comparison](RELOCATED-IMAGE.md) established that the loader
output for the full frontend matched paused EE memory at one observed base;
this new assembly experiment did not repeat that runtime comparison.
It identifies no matching C/C++ functions and adds no ByteMatch progress.
The same assembly workflow now covers a separate
[candidate interval in LEVEL_01.REL](LEVEL-01-ASSEMBLY.md).
It has since been applied to [all 21 SNR2 placements](SNR2-ASSEMBLY-CAMPAIGN.md).
