# Candidate assembly intervals across all USA PS2 SNR2 modules

All **21 pinned `LVL/*.REL` placements** now have a candidate module-relative
code interval that reassembles byte for byte from private R5900 mnemonics.
Together those intervals contain **36,341,888 bytes counted by placement**.
This is **not a unique-code total or decompilation progress**: `LEVEL_15.REL`
and `LEVEL_21.REL` have identical complete interval SHA-256 values, leaving
20 distinct full-interval hashes, and other modules can share smaller pieces.
No function boundaries or authored C/C++ matches are inferred.

[`tools/snr2_assembly.py`](../tools/snr2_assembly.py) derives a candidate
end for each pinned module by rounding the last 26-bit jump-relocation site
**plus its delay slot** up to the next `0x80` boundary. The start is `0x80`,
the value of the measured header field at `0x28`. This boundary rule is a
measured **candidate**, not a proof that every word has been classified as
code. Before accepting it, the tool checks all of the following for each
module:

- The full disc and module hashes agree with the pinned USA target.
- Every word in the candidate interval decodes as a valid R5900 instruction;
  every relative branch target stays inside it.
- The zero-target J/JAL site set exactly equals the named type-4 relocation
  site set, and the nonzero-target J/JAL set exactly equals the packed local
  type-2 site set. Neither comparison may have missing or extra sites.
- The measured SN assembler and linker reproduce **every byte** of the
  original, unrelocated candidate interval at the same module-relative
  address. Optional `rabbitizer==1.16.2` maps R5900 `sqrt.s` forms; no raw
  instruction-word fallback was needed in any of the 21 intervals.

The generated metadata-only
[`config/levels/code_intervals.tsv`](../config/levels/code_intervals.tsv)
records each module's end, byte count, exact SHA-256 and relocation counts.
It contains no game bytes. With your own lawful pinned USA image and locally
available measured tools, reproduce the whole campaign and check that table:

```sh
python tools/snr2_assembly.py baserom/SCUS_976.15.iso --all \
  --assembler /path/to/Ps2EeAs.exe --linker /path/to/ld.exe
```

`--write-catalogue` refreshes the table **only after all 21 private assembly
comparisons pass**. The generated source, objects and ELFs remain under
ignored `build/snr2-assembly/`. The earlier frontend command remains an
entry point for its one placement.

These are **pre-relocation code-interval** baselines. Module headers, data,
symbol/relocation tables and bytes outside the candidate intervals have not
been independently reconstructed. The full relocated images of only
`FRONTEND.REL` and `LEVEL_01.REL` were separately compared with paused EE
memory ([runtime evidence](RELOCATED-IMAGE.md)); no new runtime comparison
was made for the other 19 placements. The module intervals are not a
complete-program assembly or C/C++ matching gate, and they add **zero**
ByteMatch progress.
