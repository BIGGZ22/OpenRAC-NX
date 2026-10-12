#!/usr/bin/env python3
"""
The game's programs from the disc image, and the catalogue of their functions.

  python3 tools/levels.py dump        # baserom/SCUS_972.68 and baserom/levels/NN_name/ (local)
  python3 tools/levels.py catalogue   # config/levels/functions.tsv

`dump` reads baserom/SCUS_972.68.iso (USA v1.01) and writes the boot executable
to baserom/SCUS_972.68 and each level's program to baserom/levels/<NN_name>/, one
file per section plus manifest.json with their addresses. All of it is game data:
baserom/ is never committed.

A level's program is in its archive G/LEVELn.WAD: the word pair at +0x10 of the
archive's header gives the level data (sector offset, sector count); the data
opens with twelve (offset, size) slots, and slot 0 is the program, a list of
sections, each a 16-byte record (destination, size, type, entry point) followed by
its bytes (a NOBITS section's too). Every section of one program repeats the entry point; the list ends
where it changes (docs/LEVELS.md).

`catalogue` deduplicates the functions of the 27 programs against each other and
against the boot executable, comparing instructions with only their address
fields masked (identity()). Every distinct function gets one name:

  - a function the boot executable has keeps the boot's name (func_XXXXXXXX);
  - any other gets func_LNN_XXXXXXXX: its address in the lowest-numbered level
    that has it.

The catalogue holds names, sizes, fingerprints and addresses, never bytes, so it
is tracked.
"""
import gzip
import hashlib
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ISO = ROOT / "baserom/SCUS_972.68.iso"
BOOT = ROOT / "baserom/SCUS_972.68"
DUMP = ROOT / "baserom/levels"
LEVELS = ROOT / "config/levels"
CATALOGUE = LEVELS / "functions.tsv"
SECTION_NAMES = ("lit", "bss", "data", "vtbl", "camvtbl", "sndvtbl", "text")

# The 27 levels: disc level number -> name.
NAMES = {
    0: "aranos_tutorial", 1: "oozla", 2: "maktar_nebula", 3: "endako", 4: "barlow",
    5: "feltzin_system", 6: "notak", 7: "siberius", 8: "tabora", 9: "dobbo",
    10: "hrugis_cloud", 11: "joba", 12: "todano", 13: "boldan", 14: "aranos_prison",
    15: "gorn", 16: "snivelak", 17: "smolg", 18: "damosel", 19: "grelbin", 20: "yeedil",
    22: "dobbo_orbit", 23: "damosel_orbit", 24: "ship_shack", 25: "wupash_nebula",
    26: "jamming_array", 30: "insomniac_museum",
}


def level_dir(number: int) -> str:
    return f"{number:02d}_{NAMES[number]}"


class Iso:
    """Just enough ISO9660 to find files by path."""

    def __init__(self, path: Path):
        self.f = open(path, "rb")

    def read(self, lba: int, size: int, offset: int = 0) -> bytes:
        self.f.seek(lba * 2048 + offset)
        return self.f.read(size)

    def entries(self, lba: int, size: int) -> dict[str, tuple[int, int]]:
        d = self.read(lba, size)
        out, i = {}, 0
        while i < len(d):
            if d[i] == 0:
                i = (i // 2048 + 1) * 2048
                continue
            at, length = struct.unpack_from("<I", d, i + 2)[0], struct.unpack_from("<I", d, i + 10)[0]
            name = d[i + 33:i + 33 + d[i + 32]].decode("latin1").split(";")[0]
            out[name] = (at, length)
            i += d[i]
        return out

    def find(self, path: str) -> tuple[int, int]:
        pvd = self.read(16, 2048)
        at, length = struct.unpack_from("<I", pvd, 158)[0], struct.unpack_from("<I", pvd, 166)[0]
        for part in path.split("/"):
            at, length = self.entries(at, length)[part]
        return at, length


def archives(iso: Iso) -> dict[int, int]:
    """level number -> LBA of its archive. The number is the archive's own (+0x08); it is
    the file's except for G/LEVEL21.WAD, which holds level 30."""
    out = {}
    for n in range(len(NAMES)):
        lba, _ = iso.find(f"G/LEVEL{n}.WAD")
        out[struct.unpack_from("<I", iso.read(lba, 16), 8)[0]] = lba
    if sorted(out) != sorted(NAMES):
        sys.exit(f"unexpected level numbers {sorted(out)}")
    return out


def program(iso: Iso, lba: int) -> dict:
    """The program in the level archive at LBA: {"entry", "sections": [{name, address, size,
    type, data}]}."""
    head = iso.read(lba, 0x60)
    data_sector, _ = struct.unpack_from("<II", head, 0x10)
    slots = iso.read(lba, 0x60, data_sector * 2048)
    at = data_sector * 2048 + struct.unpack_from("<I", slots, 0)[0]
    sections, entry = [], None
    while True:
        dest, size, kind, ep = struct.unpack("<4I", iso.read(lba, 16, at))
        if entry is None:
            entry = ep
        if ep != entry or len(sections) == len(SECTION_NAMES):
            break
        # A NOBITS section (.bss) still has its bytes in the archive.
        body = iso.read(lba, size, at + 16)
        sections.append({"name": SECTION_NAMES[len(sections)], "address": dest, "size": size,
                         "type": kind, "data": body})
        at += 16 + size
    if len(sections) != len(SECTION_NAMES):
        sys.exit(f"archive at {lba}: {len(sections)} sections, expected {len(SECTION_NAMES)}")
    return {"entry": entry, "sections": sections}


def dump() -> None:
    iso = Iso(ISO)
    lba, size = iso.find("SCUS_972.68")
    BOOT.write_bytes(iso.read(lba, size))
    for number, at in sorted(archives(iso).items()):
        p = program(iso, at)
        d = DUMP / level_dir(number)
        d.mkdir(parents=True, exist_ok=True)
        manifest = {"level": number, "name": NAMES[number], "entry": p["entry"], "sections": []}
        for s in p["sections"]:
            if s["type"] != 8:
                (d / f"{s['name']}.bin").write_bytes(s["data"])
            manifest["sections"].append({k: s[k] for k in ("name", "address", "size", "type")})
        (d / "manifest.json").write_text(json.dumps(manifest, indent=1) + "\n")
    print(f"wrote {BOOT.relative_to(ROOT)} and {len(NAMES)} levels to {DUMP.relative_to(ROOT)}")


# Link-dependent instruction fields.
MEM = {0x1A, 0x1B, 0x1E, 0x1F, *range(0x20, 0x30), 0x31, 0x36, 0x37, 0x39, 0x3E, 0x3F}


def words(b: bytes) -> tuple:
    return struct.unpack(f"<{len(b) // 4}I", b[:len(b) // 4 * 4])


def trim(b: bytes) -> bytes:
    while b.endswith(b"\0\0\0\0"):
        b = b[:-4]
    return b


LEVEL_BASE = 0x1A7C80          # a level's program replaces the boot's memory from here
BOOT_HI = LEVEL_BASE >> 16      # an address half below this points into memory every program shares


def identity(b: bytes, address: int = 0) -> bytes:
    """One function's words with only its address fields masked: what two copies must
    share to be the same function. Masked: jump targets, the high half of a RAM address
    (lui), $gp-relative offsets, and offsets on a register that holds an address (one a
    lui loaded with an address half, or copied or computed from one). Compared: every
    other immediate, so two routines that differ in a constant, a stack slot or a field
    offset on a pointer argument stay two functions. Large offsets on other registers
    stay masked, since an address half can reach one through a spill.

    An address in the memory below the levels' (the boot's resident code and data) is the same
    in every program, so a jump there, and a lui loading the high half of such an address with
    the offsets used on it, are compared: two routines that use different resident globals
    are two functions. ADDRESS is where the function starts, for its jump targets."""
    w = words(trim(b))
    tainted, grew = set(), True
    while grew:
        grew = False
        for x in w:
            op, rs, rt, rd = x >> 26, (x >> 21) & 31, (x >> 16) & 31, (x >> 11) & 31
            new = None
            if op == 0x0F and 0x10 <= x & 0xFFFF < 0x200:
                new = rt
            elif op in (0x09, 0x19) and rs in tainted:
                new = rt
            elif op == 0 and x & 0x3F in (0x21, 0x2D, 0x25) and (rs in tainted or rt in tainted):
                new = rd
            if new and new not in tainted:
                tainted.add(new)
                grew = True
    out, resident = [], {}         # register -> its lui loaded a resident address half
    for k, x in enumerate(w):
        op, rs, rt, imm = x >> 26, (x >> 21) & 31, (x >> 16) & 31, x & 0xFFFF
        if op in (2, 3):
            target = ((address + 4 * k) & 0xF0000000) | ((x & 0x3FFFFFF) << 2)
            if not (address and target < LEVEL_BASE):
                x &= 0xFC000000
        elif op == 0x0F:
            resident[rt] = 0x10 <= imm < BOOT_HI
            if 0x10 <= imm < 0x200 and not resident[rt]:
                x &= 0xFFFF0000
        elif rs in resident and resident[rs] and (op in MEM or op in (0x09, 0x0D, 0x19)) and rs != 29:
            pass                     # the low half of a resident address
        elif rs == 28 and op >= 8:
            x &= 0xFFFF0000
        elif rs == 28 and op >= 8:
            x &= 0xFFFF0000
        elif (op in MEM or op in (0x09, 0x0D, 0x19)) and rs != 29:
            small = imm < 0x1000 or imm >= 0xF000
            if rs in tainted or (rs != 0 and op != 0x0D and not small):
                x &= 0xFFFF0000
        out.append(x)
    return struct.pack(f"<{len(out)}I", *out)


def fingerprint(b: bytes, address: int = 0) -> str:
    return hashlib.sha1(identity(b, address)).hexdigest()[:16]


BOOT_FUNCTIONS = ROOT / "config/boot/functions.tsv"
BOOT_PAIRS = ROOT / "config/levels/boot_pairs.tsv"


def boot_image() -> tuple[bytes, list[tuple[int, int, int]]]:
    """The boot executable and its loadable segments (vaddr, file offset, file size)."""
    elf = BOOT.read_bytes()
    phoff, phnum = struct.unpack_from("<I", elf, 0x1C)[0], struct.unpack_from("<H", elf, 0x2C)[0]
    segs = []
    for i in range(phnum):
        kind, off, va, _, size = struct.unpack_from("<5I", elf, phoff + 32 * i)
        if kind == 1:
            segs.append((va, off, size))
    return elf, segs


def boot_bytes(address: int, size: int) -> bytes:
    elf, segs = boot_image()
    for va, off, n in segs:
        if va <= address < va + n:
            return elf[off + address - va: off + address - va + size]
    raise ValueError(f"{address:#x} is not in the boot executable")


def level_text(directory: Path) -> tuple[int, bytes]:
    m = json.loads((directory / "manifest.json").read_text())
    text = next(s for s in m["sections"] if s["name"] == "text")
    return text["address"], (directory / "text.bin").read_bytes()


def old_extents(program: str) -> list[tuple[int, int]]:
    """Function extents from the catalogues this project kept before (config/function-catalog/,
    one gzipped file per program): only for the first catalogue."""
    out = []
    for line in gzip.open(ROOT / f"config/function-catalog/{program}.ndjson.gz", "rt"):
        r = json.loads(line)
        if isinstance(r, list):
            out.append((r[0], r[1]))
    return out


def read_boot_functions() -> list[tuple[str, int, int]]:
    rows = []
    for line in BOOT_FUNCTIONS.read_text().splitlines():
        if not line.startswith("#"):
            name, address, size = line.split("\t")[:3]
            rows.append((name, int(address, 16), int(size)))
    return rows


def read_catalogue() -> list[tuple[str, str, int, str, list[tuple[int, int]]]]:
    rows = []
    for line in CATALOGUE.read_text().splitlines():
        if line.startswith("#"):
            continue
        name, kind, size, fp, _, places = line.split("\t")
        rows.append((name, kind, int(size), fp,
                     [(int(p.split(":")[0]), int(p.split(":")[1], 16)) for p in places.split(",")]))
    return rows


def catalogue() -> None:
    """config/boot/functions.tsv and config/levels/functions.tsv from the function extents:
    the old catalogues the first time (--from-old), these files after."""
    from_old = "--from-old" in sys.argv
    elf, segs = boot_image()
    text_va, text_off, text_size = segs[0]
    if from_old:
        boot = [(f"func_{a:08X}", a, n) for a, n in old_extents("boot")
                if text_va <= a < text_va + text_size]
    else:
        boot = read_boot_functions()
    boot_by_fp = {}
    for name, a, n in sorted(boot, key=lambda r: r[1]):
        boot_by_fp.setdefault((fingerprint(boot_bytes(a, n), a), n), []).append(name)
    extents = {}
    if from_old:
        for number in NAMES:
            base, data = level_text(DUMP / level_dir(number))
            extents[number] = [(a, n) for a, n in old_extents(f"{number}_{NAMES[number]}")
                               if base <= a < base + len(data)]
    else:
        for _, _, size, _, places in read_catalogue():
            for number, a in places:
                extents.setdefault(number, []).append((a, size))
    found = {}
    for number in sorted(extents):
        base, data = level_text(DUMP / level_dir(number))
        for a, n in sorted(extents[number]):
            body = data[a - base:a - base + n]
            if not trim(body):
                continue
            # the size too: the same instructions with different padding are two functions
            entry = found.setdefault((fingerprint(body, a), n), {"size": n, "places": []})
            entry["places"].append((number, a))
    # Boot functions paired by hand with level places the code alone cannot tell apart.
    pairs = {}
    if BOOT_PAIRS.is_file():
        for line in BOOT_PAIRS.read_text().splitlines():
            if not line.startswith("#"):
                name, places = line.split("\t")
                for p in places.split(","):
                    pairs[(int(p[:2]), int(p[3:], 16))] = name
    # Two places in one program are two functions (identical code, e.g. two empty methods):
    # the k-th copy in each level, by address, is one function (the levels link their shared
    # code in one order). It is named after the boot's function only when there is one copy
    # each: the boot's order is not the levels'.
    rows = []
    for key, e in found.items():
        fp = key[0]
        per_level, paired = {}, {}
        for number, a in sorted(e["places"]):
            if (number, a) in pairs and pairs[(number, a)] in boot_by_fp.get(key, ()):
                paired.setdefault(pairs[(number, a)], []).append((number, a))
            else:
                per_level.setdefault(number, []).append(a)
        for name, places in sorted(paired.items()):
            rows.append((name, "boot", e["size"], fp, places))
        if not per_level:
            continue
        copies = max(len(v) for v in per_level.values())
        for k in range(copies):
            places = [(number, v[k]) for number, v in sorted(per_level.items()) if len(v) > k]
            if copies == 1 and len(boot_by_fp.get(key, ())) == 1 and not paired:
                name, kind = boot_by_fp[key][0], "boot"
            else:
                number, a = places[0]
                name = f"func_L{number:02d}_{a:08X}"
                kind = "level" if len({p[0] for p in places}) == 1 else "shared"
            rows.append((name, kind, e["size"], fp, places))
    rows.sort(key=lambda r: r[4][0])
    BOOT_FUNCTIONS.parent.mkdir(parents=True, exist_ok=True)
    BOOT_FUNCTIONS.write_text("# name\taddress\tsize\n" +
                              "".join(f"{n}\t{a:08X}\t{s}\n" for n, a, s in sorted(boot, key=lambda r: r[1])))
    LEVELS.mkdir(parents=True, exist_ok=True)
    lines = ["# name\tkind\tsize\tfingerprint\tlevels\tplaces (level:address)"]
    for name, kind, size, fp, places in rows:
        lines.append(f"{name}\t{kind}\t{size}\t{fp}\t{len({p[0] for p in places})}\t" +
                     ",".join(f"{l:02d}:{a:08X}" for l, a in places))
    CATALOGUE.write_text("\n".join(lines) + "\n")
    placed = sum(r[2] * len(r[4]) for r in rows)
    count = {k: sum(1 for r in rows if r[1] == k) for k in ("boot", "shared", "level")}
    size = {k: sum(r[2] for r in rows if r[1] == k) for k in ("boot", "shared", "level")}
    print(f"boot: {len(boot)} functions, {sum(r[2] for r in boot)} bytes")
    print(f"levels: {sum(len(r[4]) for r in rows)} placements, {placed} bytes; {len(rows)} distinct: " +
          ", ".join(f"{count[k]} {k} ({size[k]} bytes)" for k in count))


def main() -> None:
    commands = {"dump": dump, "catalogue": catalogue}
    if len(sys.argv) < 2 or sys.argv[1] not in commands:
        sys.exit(__doc__)
    commands[sys.argv[1]]()


if __name__ == "__main__":
    main()
