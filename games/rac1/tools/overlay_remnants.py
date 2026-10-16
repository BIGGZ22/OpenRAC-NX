#!/usr/bin/env python3
"""
Linker remnants (docs/ASM_CLASSIFICATION.md): what retail's linker left of
the functions it stripped. A stripped function of size 4 mod 8 leaves its
last word, the delay slot of its return, then an alignment nop or the
linker's 0xCDCDCDCD fill (tools/strip_dead.py); one of size 0 mod 8 leaves
nothing. Splat and the level catalogue give a run of these words a name of
its own, or several. No C produces them, so the lists name them and the
report counts them as finished:

  config/overlays/linker_remnants.txt   level code, checked here
  config/linker_remnants.txt            the executable's

  python3 tools/overlay_remnants.py            # every level entry that passes, and its places
  python3 tools/overlay_remnants.py --check    # the level list is exactly the entries that pass
  python3 tools/overlay_remnants.py --write    # rewrite the level list (the source lines are yours)
  python3 tools/overlay_remnants.py --exe      # executable entries that pass and are not listed yet

Reads the level dumps (tools/overlays.py dump) and baserom/SCES_509.16. An
entry passes when it is not a piece of a joined function
(config/overlays/joined.tsv) and, in every place it has:

  - it starts on an 8-byte boundary, and each of its words there is not a
    jump or branch (no delay slot is one), while each word between is zero
    or linker fill; at least one word is neither;
  - the code before it is finished: going back over earlier leftovers (a
    word, then zero or fill), there is a `jr $31` and its delay slot, a
    tail jump `j` to the start of a function, or the start of the code;
  - nothing reaches it: no branch or jump lands on one of its words and no
    word of code or data holds one of their addresses.

The last two tests tell a run of remnants from a piece of a function the
catalogue cut in the wrong place, which is a boundary to fix (joined.tsv):
a piece follows code that has not returned, or is the target of the
function's own branches (a trap block, a loop after an early return).
The executable's list was taken from tools/triage.py and upstream's own
splits before this rule existed; `--exe` only proposes additions to it.
"""
import json
import struct
import sys
from pathlib import Path

import overlays

LIST = overlays.ROOT / "config/overlays/linker_remnants.txt"
EXE_LIST = overlays.ROOT / "config/linker_remnants.txt"
JOINED = overlays.ROOT / "config/overlays/joined.tsv"
JR_RA = 0x03E00008
FILL = 0xCDCDCDCD
PAD = (0, FILL)
HEADER = """\
# Level code's linker remnants: what retail's linker left of the functions it stripped (each
# one's last delay slot, then an alignment nop or fill). Kept as original assembly and counted as
# finished, as config/linker_remnants.txt does for the executable (docs/ASM_CLASSIFICATION.md).
# tools/overlay_remnants.py states the rule and checks this list against the level dumps;
# the source marks each with LINKER_REMNANT("asm/overlays", name).
"""


def is_jump(w: int) -> bool:
    op = w >> 26
    return (op in (1, 2, 3, 4, 5, 6, 7, 20, 21, 22, 23)            # regimm, j, jal, beq..bgtzl
            or (op == 0 and (w & 0x3F) in (8, 9))                  # jr, jalr
            or (op == 0x11 and (w >> 21) & 0x1F == 8))             # bc1


def reached(chunks) -> set[int]:
    """Every address a branch or jump lands on, and every word value, of CHUNKS:
    (base, bytes, is_code)."""
    out = set()
    for base, data, code in chunks:
        for i in range(0, len(data) - 3, 4):
            w, = struct.unpack_from("<I", data, i)
            out.add(w)
            if not code:
                continue
            op = w >> 26
            if op in (2, 3):
                out.add(((base + i) & 0xF0000000) | (w & 0x3FFFFFF) << 2)
            elif is_jump(w) and op:
                off = w & 0xFFFF
                out.add(base + i + 4 + 4 * (off - 0x10000 if off & 0x8000 else off))
    return out


class Image:
    """One address space: the code to test, everything that could reach into it,
    and where functions start (for tail jumps)."""

    def __init__(self, code, everything, starts):
        self.code, self.reached, self.starts = code, reached(everything), starts

    def word(self, address: int) -> int | None:
        for base, data in self.code:
            if base <= address < base + len(data):
                return struct.unpack_from("<I", data, address - base)[0]
        return None

    def verdict(self, address: int, size: int) -> str | None:
        """Why the SIZE bytes at ADDRESS are not a run of remnants, or None."""
        if address % 8:
            return "not on an 8-byte boundary"
        if all(self.word(at) in PAD for at in range(address, address + size, 4)):
            return "only zero words or fill"
        for at in range(address, address + size, 4):
            w = self.word(at)
            if at % 8 == 0 and is_jump(w):
                return "a jump or branch where a delay slot was"
            if at % 8 and w not in PAD:
                return "an instruction where the alignment nop was"
            if at in self.reached:
                return "reached by a branch, jump or pointer"
        at = address
        while True:
            x, y = self.word(at - 8), self.word(at - 4)
            if x is None:
                return None                                  # the start of the code
            if y == JR_RA:
                return "the delay slot of the return before it"
            if x == JR_RA or (y in PAD and self.word(at - 12) == JR_RA):
                return None                                  # jr $31, its delay slot, alignment
            if x >> 26 == 2 and ((at - 8) & 0xF0000000 | (x & 0x3FFFFFF) << 2) in self.starts:
                return None                                  # a tail jump and its delay slot
            if y in PAD and not is_jump(x):
                at -= 8                                      # an earlier remnant
                continue
            return "no finished function before it"


_levels = {}


def level_image(level: int) -> Image:
    if level not in _levels:
        d = overlays.DUMP / f"level_{level:02d}"
        if not (d / "text.bin").exists():
            sys.exit(f"{d}: no dump (python3 tools/overlays.py dump)")
        records = [r for r in json.loads((d / "manifest.json").read_text())["records"]
                   if r["name"] != "bss"]
        chunks = [(r["address"], (d / f"{r['name']}.bin").read_bytes(), r["name"] == "text")
                  for r in records]
        core = [(a, b, n == "core.text") for n, (a, b) in exe_sections().items()
                if n.startswith("core.") and n != "core.bss"]     # the executable's library stays
        starts = {address for _, _, _, places in catalogue() for lv, address in places if lv == level}
        _levels[level] = Image([(a, b) for a, b, code in chunks if code], chunks + core, starts)
    return _levels[level]


_sections = {}


def exe_sections() -> dict[str, tuple[int, bytes]]:
    if not _sections:
        _sections.update((n, s) for n, s in overlays.elf_sections(overlays.ELF).items()
                         if s[0] and n not in (".reginfo", "core.bss", ".bss"))
    return _sections


_catalogue = []


def catalogue():
    if not _catalogue:
        _catalogue.extend(overlays.read_catalogue())
    return _catalogue


def joined_pieces() -> set[str]:
    return {p for line in JOINED.read_text().splitlines() if line and not line.startswith("#")
            for p in line.split("\t", 1)[1].split()}


def verdicts() -> list[tuple[str, int, int, str | None]]:
    """(name, size, places, verdict) of every level-code catalogue entry."""
    pieces = joined_pieces()
    out = []
    for name, kind, size, places in catalogue():
        if kind == "exe":
            continue
        if name in pieces:
            out.append((name, size, len(places), "a piece of a joined function"))
            continue
        found = [level_image(level).verdict(address, size) for level, address in places]
        out.append((name, size, len(places), next((v for v in found if v), None)))
    return out


def listed(path: Path = LIST) -> list[str]:
    if not path.exists():
        return []
    return [l.strip() for l in path.read_text().splitlines() if l.strip() and not l.startswith("#")]


def exe_proposals() -> list[tuple[str, int]]:
    """segment/name and size of every executable entry that passes and that the source still
    includes as unmatched assembly (src/libgcc reproduces its own remnants from GCC's source)."""
    import re
    stub = re.compile(r'INCLUDE_ASM\("asm/nonmatchings/(core_text|text)",\s*(func_[0-9A-F]{8})\)')
    stubs = {f"{seg}/{name}" for path in (overlays.ROOT / "src").rglob("*.c")
             for seg, name in stub.findall(path.read_text(errors="replace"))}
    secs = exe_sections()
    code = [secs["core.text"], secs[".text"]]
    names = {}
    for seg in ("core_text", "text"):
        for p in sorted((overlays.ROOT / "asm/nonmatchings" / seg).glob("func_*.s")):
            m = re.search(r"nonmatching (func_[0-9A-F]{8}), (0x[0-9A-F]+)", p.read_text(errors="replace"))
            if m:
                names[f"{seg}/{m.group(1)}"] = int(m.group(2), 16)
    image = Image(code, [(a, b, n in ("core.text", ".text")) for n, (a, b) in secs.items()],
                  {int(n.split("_")[-1], 16) for n in names})
    out = []
    for entry, size in names.items():
        if entry not in stubs:
            continue
        address = int(entry.split("_")[-1], 16)
        if image.verdict(address, size) is None:
            out.append((entry, size))
    return out


def main() -> None:
    if "--exe" in sys.argv[1:]:
        rows = exe_proposals()
        for entry, size in rows:
            print(f"{entry}  {size}")
        print(f"{len(rows)} executable entries ({sum(s for _, s in rows)} bytes) pass and are not listed")
        return
    rows = verdicts()
    passing = sorted(name for name, _, _, v in rows if v is None)
    if "--write" in sys.argv[1:]:
        LIST.write_text(HEADER + "".join(f"{n}\n" for n in passing))
        print(f"{LIST.relative_to(overlays.ROOT)}: {len(passing)} remnants")
        return
    if "--check" in sys.argv[1:]:
        have = set(listed())
        for name in sorted(have - set(passing)):
            print(f"listed, but not a remnant: {name}")
        for name in sorted(set(passing) - have):
            print(f"a remnant, but not listed: {name}")
        if have != set(passing):
            sys.exit(1)
        print(f"{len(passing)} level remnants, as listed")
        return
    for name, size, places, v in rows:
        if v is None:
            print(f"{name}  {size:4d} bytes  {places:3d} places")
    print(f"{len(passing)} level entries ({sum(s for n, s, _, v in rows if v is None)} bytes) are remnants")


if __name__ == "__main__":
    main()
