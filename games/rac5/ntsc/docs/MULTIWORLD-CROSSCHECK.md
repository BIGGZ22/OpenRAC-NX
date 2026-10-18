# Cross-check against the public Size Matters PS2 address map

This is a **metadata-only** comparison of our pinned USA boot executable
(`SCUS_976.15`, SHA-256 in `config/target.json`) with the public
[MultiworldGG Size Matters address map](https://github.com/MultiworldGG/MultiworldGG/blob/c3a271b329e39e795a97927b7c155dc27511b101/worlds/rac_size_matters/core/address_maps/us_addresses.py)
and its [loader gate](https://github.com/MultiworldGG/MultiworldGG/blob/c3a271b329e39e795a97927b7c155dc27511b101/worlds/rac_size_matters/core/patches/loader_gate.py),
at upstream commit `c3a271b329e39e795a97927b7c155dc27511b101`.
No third-party code or game bytes are imported.

| MultiworldGG USA address | Its role | Local independent evidence | Status |
| --- | --- | --- | --- |
| `0x01EDDAB8` | loader state | Boot `.sndata` names it `g_substate`, in `.data`. | Exact address/name correspondence; behavior still needs a trace. |
| `0x01EDDAD4` | transition gate | Boot name `g_FadeLevel`, in `.data`. | Exact correspondence. |
| `0x01EDDAE4` | target/loading planet | Boot name `g_LevelToLaunch`, in `.data`; changed to `15` and `21` during the corresponding live loads. | Address and load behavior corroborated. |
| `0x01EDDAE8` | load thread | Boot name `g_GameLoadThread`, in `.data`. | Exact correspondence. |
| `0x01FBE750` | loaded-module table | Boot name `modules`, in `.bss`; a live level load populated handle 1 at entry stride `0x418`. | Base and active flag observed; remaining fields unverified. |
| `0x01F4A744` | new-planet start load | Four bytes after boot name `g_McpGlobals` (`0x01F4A740`), in `.bss`. | A private write to `15` or `21` during a new game's first level requested that level after the opening cutscene. |
| `0x01F4C768`–`0x01F4C770` | bolt count/current planet/load request | Inside boot `.bss`, without individual names in the exported table. | The middle word at `0x01F4C76C` became `15` and `21` after the respective live loads; the other fields remain unverified. |
| `0x00F805C0`, `0x00F80E2C` | Pokitaru player state/health | Outside boot ELF sections. | Likely resident level memory; no static REL-to-RAM mapping has been established. |

The loader-gate source also records a 12-word signature at `0x01E66400`,
including the instruction at `0x01E66414` (`0x24040005`). Reading the pinned
boot's `.text` bytes through its ELF section offset shows **all 12 words
agree exactly**. This confirms that this specific MultiworldGG USA loader site
applies to our boot. It does not validate the gate's runtime behavior or the
rest of the address map. The `0x01E66414` site lies in boot `.text`; the
boot symbol table does not name the containing function at that address.

The public [Size Matters New Game hook](https://github.com/Panguu/Archipelago-RaCSM/blob/SM-Main/worlds/rac_size_matters/core/patches/starting_planet.py)
also identifies `FRONTEND.REL` module offset `0x2F80` as an initializer and
`0x3024` as a state-write site inside it. Its first 12 bytes agree with our
pinned USA frontend module. An independent Ghidra pass identifies a contiguous
476-byte candidate body at `0x2F80`; the [level C gate](LEVEL-C-WITNESS.md)
checks independently authored C at that placement against the complete module
and measured relocated-image hash. The external hook was a research lead;
no patch instructions or Archipelago code are imported.

## Giant Clank level IDs: live USA load cross-check

[Panguu's planet catalogue at the inspected commit](https://github.com/Panguu/Archipelago-RaCSM/blob/8c76030426de3b12e3c99a4306b4c8379c9d6792/worlds/rac_size_matters/core/planets.py)
labels `0x0F` as Giant Clank (Metalis) and `0x15` as Giant Clank
(Challax). In our pinned US boot, the live loader's ID-indexed path table maps
these IDs to `cdrom0:/LVL/LEVEL_15.REL` and `LEVEL_21.REL`, respectively.
We also tested both targets in an isolated PCSX2 session with a locally created
new game; no external save or patched disc was used:

| Catalogue label | Requested ID | Loader path | Observed after transition |
| --- | --- | --- | --- |
| Giant Clank (Metalis) | `0x0F` (15) | `LEVEL_15.REL` | `g_LevelToLaunch=15`, current planet `15`, active module handle 1 at `0x00DF7380` |
| Giant Clank (Challax) | `0x15` (21) | `LEVEL_21.REL` | `g_LevelToLaunch=21`, current planet `21`, active module handle 1 at `0x00DF7380` |

For each test, we restored the same private savestate taken during the opening
level, wrote the target ID to the observed new-planet request field, and waited
for the normal cutscene-to-level transition. The request field returned to its
idle value `0xFFFFFFFF` after the load. This independently verifies the
ID-to-REL mapping and that both modules can load in the US game. The *Giant
Clank* names come from the external catalogue; this test did not complete
either mission or prove every Giant Clank gameplay state. Private savestates,
screenshots, game bytes, and probe scripts are not part of this repository.

MultiworldGG selects maps for US `SCUS-97615`, EU `SCES-55019`, and JP
`SCPS-15120` in its [address-map selector](https://github.com/MultiworldGG/MultiworldGG/blob/c3a271b329e39e795a97927b7c155dc27511b101/worlds/rac_size_matters/core/address_maps/__init__.py).
The other regions are useful comparisons, but our matching target remains the
verified USA image. Their addresses cannot be substituted into it.

## Reproduction and next check

Regenerate the local names with `python tools/boot_symbols.py catalogue
baserom/SCUS_976.15`; compare the addresses above with
`config/boot/symbols.tsv`. Inspect the pinned boot with `python tools/elf.py
baserom/SCUS_976.15 --json` to locate `.text`, `.data` and `.bss`. The
12-word signature was checked as little-endian words read at the `.text` file
offset corresponding to `0x01E66400`.

The live loads establish the level IDs and module bases. A future loader trace
can still record resident size and entrypoint, then compare per-level address
maps with measured bounds. This metadata does not count as a C match or
compiler-profile evidence.
