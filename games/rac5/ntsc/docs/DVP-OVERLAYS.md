# Boot VU overlays and exact ELF packaging

The pinned USA boot has one file-backed `PT_LOAD` segment at file offsets
`128`–`810000`. Its 31 `.DVP` sections are outside that segment. The
[binutils MIPS definitions](https://mirror.iscas.ac.cn/git/sourceware.org/git/binutils-gdb/-/blame/7ae26f2731023f088ea805a39e47d1680055f88e/include/elf/mips.h?page=1)
identify type `0x7FFFF420` as a **VU overlay table** and `0x7FFFF421` as a
**VU overlay**. These are not merely generic ELF debug-section names.

For this boot, 29 overlay sections contain **41,824 zero bytes**. Their table
is 360 bytes: 30 twelve-byte records, with one section name used twice. The
1,036-byte string table holds the names. Thus all 31 DVP sections total
**43,220 bytes**. Every record's load address covers a nonzero window inside
the boot's already-loaded `.data`; its VU address is either `0` or `0x800`.
The overlay sections themselves have virtual address zero and no `SHF_ALLOC`
flag. These are measured properties of `SCUS_976.15`, not claims about other
games or the original VU source.

[`tools/boot_dvp.py`](../tools/boot_dvp.py) checks those properties against
the hash-pinned boot. It generates only the metadata catalogue
[`config/boot/dvp_overlays.tsv`](../config/boot/dvp_overlays.tsv), with section
names, sizes, string offsets, load addresses and VU addresses. Given lawful
local SN assembler/linker copies, it can generate a private synthetic object:

```sh
python tools/boot_dvp.py catalogue baserom/SCUS_976.15
python tools/boot_dvp.py rebuild baserom/SCUS_976.15 \
  --assembler /path/to/Ps2EeAs.exe --linker /path/to/ld.exe
```

The rebuild creates the zero overlay sections and regenerates the table and
string table from that metadata under ignored `build/dvp-overlays/`. All
**31/31 linked DVP sections** match the retail bytes, type, flags, address and
alignment. A separate private whole-boot link including those regenerated
sections still matches the ELF entry, program header and raw `PT_LOAD` bytes.
That direct SN-linker output has 41 sections like retail, but its full file is
**856,100** rather than **856,120 bytes**: section order, some flags/alignment,
`.reginfo` placement, and the temporary BSS-tail section differ. The later
[`tools/boot_elf_pack.py`](../tools/boot_elf_pack.py) reconstructs this file
container from a C-linked `PT_LOAD` and the generated numeric
[`elf_layout.json`](../config/boot/elf_layout.json). It regenerates `.DVP`
payloads, `.reginfo`, section names and headers without copying the retail
file. The packaged **856,120-byte ELF is byte-identical** to the pinned boot.
The loaded non-code sections still come from the user's private boot; this is
an exact packaging result, not VU or game-data decompilation.

There is a concrete emulator consequence. PCSX2's
[`ElfObject::GetCRC`](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/Elfheader.cpp)
XORs 32-bit words over the **whole ELF file**, and PCSX2 documents that
[patches are keyed by game serial and executable CRC](https://pcsx2.net/docs/advanced/writing-patches/).
The retail boot reports `8661F7BA`; the private stripped C-linked ELF without
DVP sections reports `FA686352`. The full-file-exact packaged ELF has the
measured retail CRC `8661F7BA`. In an earlier isolated PCSX2 run using the pinned
disc, that DVP-free ELF reached the frontend module's `SNR2` header in EE
memory at `0x00F97B80`; a retail run reached the same header with the same
first 32 bytes. This is a **boot-to-frontend smoke test**, not a full gameplay,
rendering, hardware or patch-equivalence test. PCSX2 also documents import of
[`.symtab`, `.mdebug` and `.sndata` symbols](https://pcsx2.net/docs/advanced/debugger/),
so linked ELF metadata can affect the debugger even when loaded bytes agree.

For C matching, keep the exact entry and loaded program-byte check separate
from whole-file identity and tool behavior. The `.DVP` data can now be
regenerated privately for the baseline. This metadata result alone does not
count a C match. The [boot-only gate](BOOT-MATCH-SCOPE.md) now accepts 263
reviewed boot placements (19,048 bytes) and requires the entire packaged ELF to
match.
