# LEVEL_01.REL assembly interval, USA PS2

The pinned `LVL/LEVEL_01.REL` has a **candidate code interval** from module
offset `0x80` through `0x1EF17F` (end-exclusive `0x1EF180`). All
**2,027,776 / 2,027,776 bytes** in this interval were reassembled from
private R5900 instruction mnemonics and compared with the original,
unrelocated REL bytes. The linked interval SHA-256 is
`c9d7ff3dbaef21313780e8608e48a4c07e6bd8308be82f678ecec58089ae47c8`.
No game-derived assembly, REL, object or ELF is committed.

The measured boundary has several independent checks. The SNR2 header field
at `0x28` is `0x80`; the final observed control-transfer delay slot at
`0x1EF104` is followed by 30 zero instruction words through `0x1EF17C`.
The first invalid R5900 word after the boundary is at `0x1EF1A8`, inside
the following data area. All words inside the candidate interval decode as
valid R5900 instructions, and every relative branch stays inside it.
More importantly, the **16,166** nonzero-target jump sites exactly equal the
packed local 26-bit fixup sites, and the **4,226** zero-target jump sites
exactly equal the named type-4 relocation sites. Both comparisons found no
missing or extra sites. Zero targets remain import placeholders until the
SNR2 loader applies named relocations.

[`tools/snr2_assembly.py`](../tools/snr2_assembly.py) verifies the full USA
disc and module hashes, the boundary checks, both relocation-site sets and
the measured SN tool hashes. It decodes 506,944 words with optional
`rabbitizer==1.16.2`, including 1,188 R5900 `sqrt.s` forms mapped to
assembler mnemonics. No raw instruction-word fallback is needed. SN
`Ps2EeAs.exe` and `ld.exe` build at the module-relative address `0x80`;
generated source and binaries stay under ignored `build/snr2-assembly/`.
With your own lawful USA image and locally available measured tools, run:

```sh
python tools/snr2_assembly.py baserom/SCUS_976.15.iso LEVEL_01.REL \
  --assembler /path/to/Ps2EeAs.exe --linker /path/to/ld.exe
```

This is an exact **pre-relocation candidate code interval**, not a complete
LEVEL_01.REL assembly baseline or function catalogue. The module header,
data, symbol tables and packed stream outside it are still original bytes.
The independent [relocated-image comparison](RELOCATED-IMAGE.md) previously
matched the full level 01 loader output to paused EE memory at one observed
base; this assembly experiment did not repeat that runtime comparison.
It identifies no matching C/C++ functions and adds no ByteMatch progress.
The same guarded workflow has since verified
[candidate intervals in all 21 SNR2 placements](SNR2-ASSEMBLY-CAMPAIGN.md).
