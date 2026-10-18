# First complete SNR2 program gates

`tools/level_program_gate.py` verifies `FRONTEND.REL` and `LEVEL_01.REL`
against the pinned USA PS2 disc. Both already have independently measured
module-relative candidate code bounds, exact SN assembly interval rebuilds,
and paused-PCSX2 relocated-image SHA-256 baselines. The tool assembles each
candidate interval again, places its linked bytes into the full private SNR2
module, requires the **entire module** to match the retail SHA-256, then
relocates it at the previously measured base and requires the previously
measured loaded-image SHA-256 and unresolved-import set. All generated module
files and the report stay under ignored `build/level-program-gate/`.

With a lawful copy of the pinned USA disc and the measured local SN tools:

```sh
python tools/level_program_gate.py baserom/SCUS_976.15.iso \
  --boot baserom/SCUS_976.15 \
  --assembler /path/to/Ps2EeAs.exe --linker /path/to/ld.exe
```

The measured results are:

| Program | Candidate code interval | Complete module | Relocated image |
| --- | ---: | ---: | ---: |
| `FRONTEND.REL` | 162,944/162,944 | 2,052,794/2,052,794 | SHA-256 `9d97396d…` |
| `LEVEL_01.REL` | 2,027,776/2,027,776 | 3,901,822/3,901,822 | SHA-256 `a64703fd…` |

The rest of each module still comes from the pinned retail input; the code
interval itself is reconstructed from private assembly. This is a
**complete-program integration baseline for future C replacements**, not a
new C match, an identified level compiler, a function catalogue, or fresh
runtime gameplay proof. The relocated SHA-256 values were previously compared
with paused EE memory at the observed bases
([evidence](RELOCATED-IMAGE.md)). The gate starts by marking its report
`incomplete` with zero matches, and publishes `passed_assembly_only` only
after both complete files and loaded-image hashes pass. A later C replacement
must preserve the entire SNR2 file and every applicable placement before it
can be counted.
