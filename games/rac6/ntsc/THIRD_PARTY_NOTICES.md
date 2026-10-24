# Third-party notices

The authored material of this repository is GPL-3.0 (`LICENSE`).

## Reused from the sibling OpenRAC projects

The following files are adapted from the OpenRAC decompilation projects, which
are GPL-3.0. The copyright and licence of those projects are retained.

| File here | Origin | Adaptation |
| --- | --- | --- |
| `tools/check_repository.py` | `OpenRAC/rac5-sm-decomp`, `tools/check_repository.py` | rewritten for this repository's layout and forbidden paths; the checks are structural, as upstream |
| `tools/lot_table.py` | `OpenRAC/rac5-sm-decomp`, wave kit `w18/E1/shortlist18.py` and the lot runner | new tool: the lot-table schema, a structural checker, and a shortlist cutter written for this repository's wave protocol |
| `tools/disc.py` | `OpenRAC/rac5-sm-decomp`, `tools/disc.py` | rewritten: the reader of the ISO9660 image, the file list, the extraction of one file, and the hashes; the target check reads `config/target.json` of this repository |
| `tools/elf.py` | `OpenRAC/rac5-sm-decomp`, `tools/elf.py` | rewritten: the ELF structure, the symbol table and the function symbols, for the boot executable of this disc |
| `tools/rel.py` | `OpenRAC/rac5-sm-decomp`, `tools/rel.py` | adapted: the SNR2 module reader of this disc (the modules are under `BIN/`, named `FE.REL` and `LEVEL<n>.REL`), the catalogues and the target check |
| `tools/code_intervals.py` | `OpenRAC/rac5-sm-decomp`, `tools/snr2_assembly.py` | adapted: the derivation of the candidate code interval of a module from its jump relocations, without the assembler step; the catalogue is metadata only |
| `tools/function_inventory.py` | `OpenRAC/rac5-sm-decomp`, `tools/level_function_inventory.py` | adapted: the candidate entries and their bounded bodies, the retail symbol names, and a branch decoder checked against `rabbitizer` 1.16.2 (28 210 edges, no difference) |
| `tools/shortlist.py` | `OpenRAC/rac5-sm-decomp`, wave kit `w17/E0/shortlist.py` | adapted: the work queue of a wave, ranked by leverage (the number of placements that carry the same raw body) |
| `tools/r5900_assembly.py` | `OpenRAC/rac5-sm-decomp`, `tools/r5900_assembly.py` | adapted: the rendering of a code interval as R5900 assembly, with the two forms the SN assembler does not encode like the retail code (the accumulator vector instructions and the COP1 branches) written as raw words |
| `tools/assembly_baseline.py` | `OpenRAC/rac5-sm-decomp`, `tools/snr2_assembly.py` | adapted: the assembly baseline of the SNR2 modules -- render, assemble, link, compare with the retail interval -- and the metadata catalogue of the result |
| `tools/c_source_policy.py` | `OpenRAC/rac5-sm-decomp`, `tools/c_source_policy.py` | adapted: the rejection of inline assembly, of global assembly blocks, of register variables and of assembler directives, with the symbol-label exception of the upstream |
| `AGENTS.md` | `OpenRAC/rac5-sm-decomp`, `AGENTS.md` | the seven upstream rules, plus three rules added here (pure C evidence, authored form, one writer) and the rewritten wording |
| `docs/METHOD.md`, `docs/EVIDENCE.md` | the method of `OpenRAC/rac2-gc-decomp` and `OpenRAC/rac5-sm-decomp` | rewritten for a platform-independent protocol |
| `PROMPT.md` | the wave briefs of `OpenRAC/rac5-sm-decomp` | rewritten as a platform-independent starter prompt |

Copyright (C) the OpenRAC contributors. This program is free software: you can
redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

## Not included

No source, header, sample or tool from any proprietary SDK is included here,
and no leaked or NDA source is used. The game, its assets and any proprietary
toolchain are not part of this repository; see `LEGAL.md`.
