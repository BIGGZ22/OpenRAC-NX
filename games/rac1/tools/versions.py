#!/usr/bin/env python3
"""
The releases of the game this repository builds, and their programs (docs/VERSIONS.md).

  python3 tools/versions.py                         # the table of versions
  python3 tools/versions.py dump [--version K]      # programs from the disc image into baserom/
  python3 tools/versions.py config [--version K]    # config/versions/<K>/programs.tsv from the dump
  python3 tools/versions.py check [--version K]     # the dump against the committed programs.tsv

Every version is one disc image in baserom/ (named as in VERSIONS; hard-link it from wherever
your discs live, never commit it). Its programs are the boot executable and the 19 level
programs, the code overlays each level loads over the executable's `main` segment
(docs/OVERLAYS.md):

  baserom/<K>/<EXE>                          the boot executable as the disc stores it
  baserom/<K>/overlays/level_NN/<record>.bin  each record of level NN's program (lit, bss, data,
                                             vtbl, camvtbl, sndvtbl, text) and manifest.json

The primary version (PAL v2.00) keeps the paths it always had: baserom/SCES_509.16 and
baserom/overlays/ (tools/overlays.py dump writes the same files).

config/versions/<K>/programs.tsv holds what identifies each program: its SHA-1 as the disc
stores it (a level's after WAD decompression), its entry point, and each section's address, size
and SHA-1. Only hashes, addresses and sizes: never bytes.

A new release slots in with one more Version below (its key, its report id, its disc image and
its executable's SHA-1); everything else reads this table.
"""
import argparse
import hashlib
import json
import os
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools/extract"))

RECORD_NAMES = ("lit", "bss", "data", "vtbl", "camvtbl", "sndvtbl", "text")
LEVEL_COUNT = 19


@dataclass(frozen=True)
class Version:
    key: str            # short name: --version, config/versions/<key>/, src/versions/<key>/
    id: str             # the report's version id (ByteMatch's artifact <id>_report)
    label: str          # for people
    region: str
    iso: str            # the disc image's file name in baserom/
    exe: str            # the boot executable on the disc (SYSTEM.CNF's BOOT2)
    exe_sha1: str
    hz: int             # the video mode it was built for
    tag: str            # the prefix of names for functions only this version has (func_<tag>_...)
    primary: bool = False

    @property
    def base(self) -> Path:
        """baserom/<key>/, or baserom/ for the primary version."""
        return ROOT / "baserom" if self.primary else ROOT / "baserom" / self.key

    @property
    def exe_path(self) -> Path:
        return self.base / self.exe

    @property
    def overlays(self) -> Path:
        return self.base / "overlays"

    @property
    def config(self) -> Path:
        return ROOT / "config/versions" / self.key

    @property
    def report(self) -> Path:
        return ROOT / ("progress/report.json" if self.primary else f"progress/{self.key}/report.json")


VERSIONS = (
    Version("pal", "SCES_509.16", "PAL v2.00", "PAL (Europe)", "SCES_509.16.iso", "SCES_509.16",
            "79956931bd62fafd8d20fa2eae796dbaf2e15e83", 50, "", primary=True),
    Version("pal_v1", "SCES_509.16_v1", "PAL v1.00", "PAL (Europe)", "SCES_509.16_v1.00.iso", "SCES_509.16",
            "553a7f6c2c7a004d384d77b7139accbc52242149", 50, "E1"),
    Version("us", "SCUS_971.99", "NTSC-U v1.00", "NTSC-U (North America)", "SCUS_971.99.iso", "SCUS_971.99",
            "72fd1de379dafd8f243dceeb3be50980f3fd3cd5", 60, "U"),
    Version("jp", "SCPS_150.37", "NTSC-J v1.02", "NTSC-J (Japan)", "SCPS_150.37.iso", "SCPS_150.37",
            "0df83877d53e19d69ec82038e2aaa227f643178f", 60, "J"),
)
PRIMARY = next(v for v in VERSIONS if v.primary)
BY_KEY = {v.key: v for v in VERSIONS}
OTHERS = tuple(v for v in VERSIONS if not v.primary)
INTEGRATED = VERSIONS  # every version is built, checked and reported here


def get(key: str | None) -> Version:
    """The version named KEY (a key or a report id), or the primary one."""
    if key is None:
        return PRIMARY
    for v in VERSIONS:
        if key in (v.key, v.id):
            return v
    sys.exit(f"unknown version {key!r}: one of " + ", ".join(v.key for v in VERSIONS))


def program_names() -> list[str]:
    return ["boot"] + [f"level_{i:02d}" for i in range(LEVEL_COUNT)]


# --- the disc ---------------------------------------------------------------------------------

def read_disc(v: Version) -> tuple[bytes, dict[int, dict]]:
    """(the boot executable, {level: {"entry_point", "raw", "records": [...]}}) from V's disc."""
    from disc import Disc, SECTOR, TOC_LBA, TOC_SIZE
    from formats import overlay_sections, span, unpack
    from level import decoded
    iso = ROOT / "baserom" / v.iso
    if not iso.exists():
        sys.exit(f"missing baserom/{v.iso} (link your own disc image there; see docs/VERSIONS.md)")
    with Disc(iso) as disc:
        exe = next((f for f in disc.files() if f["name"] == v.exe), None)
        if exe is None:
            sys.exit(f"baserom/{v.iso}: no {v.exe} on the disc")
        boot = disc.read(exe["lba"] * SECTOR, exe["bytes"])
        if hashlib.sha1(boot).hexdigest() != v.exe_sha1:
            sys.exit(f"baserom/{v.iso}: {v.exe} is not {v.label}'s (SHA-1 {hashlib.sha1(boot).hexdigest()})")
        toc = disc.read(TOC_LBA * SECTOR, TOC_SIZE)
        levels = {}
        for lid in range(LEVEL_COUNT):
            r = disc.level_header(toc, lid)["ranges"]["data"]
            data = disc.sectors(r["lba"], r["sectors"])
            offset, size = unpack("<ii", data, 0)
            raw = decoded(span(data, offset, size))
            ov = overlay_sections(raw)
            if len(ov["sections"]) != len(RECORD_NAMES):
                sys.exit(f"level {lid}: {len(ov['sections'])} overlay records, expected 7")
            records = [dict(rec, name=name, data=raw[rec["offset"]:rec["offset"] + rec["bytes"]])
                       for name, rec in zip(RECORD_NAMES, ov["sections"])]
            levels[lid] = {"entry_point": ov["entry_point"], "raw": raw, "records": records}
    return boot, levels


def overlay_program(entry: int, records: list[dict]) -> bytes:
    """A level's program as the disc stores it (after decompression): each record as
    (address, size, type, entry point) and its bytes (formats.overlay_sections)."""
    return b"".join(struct.pack("<4I", r["address"], len(r["data"]), r["type"], entry) + r["data"]
                    for r in records)


def dump(v: Version) -> None:
    boot, levels = read_disc(v)
    v.base.mkdir(parents=True, exist_ok=True)
    v.exe_path.write_bytes(boot)
    for lid, lvl in levels.items():
        if overlay_program(lvl["entry_point"], lvl["records"]) != lvl["raw"]:
            sys.exit(f"level {lid}: the records do not give back the program")
        d = v.overlays / f"level_{lid:02d}"
        d.mkdir(parents=True, exist_ok=True)
        manifest = {"level": lid, "entry_point": lvl["entry_point"], "records": []}
        for rec in lvl["records"]:
            (d / f"{rec['name']}.bin").write_bytes(rec["data"])
            manifest["records"].append({"name": rec["name"], "address": rec["address"],
                                        "bytes": rec["bytes"], "type": rec["type"]})
        (d / "manifest.json").write_text(json.dumps(manifest, indent=1) + "\n")
    print(f"{v.key}: {v.exe_path.relative_to(ROOT)} and {LEVEL_COUNT} levels in {v.overlays.relative_to(ROOT)}")


# --- reading a dump ------------------------------------------------------------------------------

def elf_sections(b: bytes) -> list[dict]:
    """The ELF's section headers: [{name, type, flags, addr, offset, size, link, info, align, entsize}]."""
    shoff, = struct.unpack_from("<I", b, 0x20)
    entsize, count, names_at = struct.unpack_from("<3H", b, 0x2E)
    strtab = struct.unpack_from("<10I", b, shoff + names_at * entsize)[4]
    out = []
    for i in range(count):
        f = struct.unpack_from("<10I", b, shoff + i * entsize)
        name = b[strtab + f[0]:b.index(b"\0", strtab + f[0])].decode()
        out.append(dict(zip(("name", "type", "flags", "addr", "offset", "size", "link", "info", "align",
                             "entsize"), (name, *f[1:]))))
    return out


def boot_sections(v: Version) -> dict[str, tuple[int, bytes]]:
    """name -> (address, bytes) of the boot executable's sections."""
    b = v.exe_path.read_bytes()
    return {s["name"]: (s["addr"], b[s["offset"]:s["offset"] + s["size"]] if s["type"] != 8 else b"")
            for s in elf_sections(b) if s["name"]}


def level_records(v: Version, lid: int) -> tuple[int, list[dict]]:
    """(entry point, [{name, address, bytes, type, data}]) of level LID's program from the dump."""
    d = v.overlays / f"level_{lid:02d}"
    m = json.loads((d / "manifest.json").read_text())
    return m["entry_point"], [dict(r, data=(d / f"{r['name']}.bin").read_bytes()) for r in m["records"]]


def level_text(v: Version, lid: int) -> tuple[int, bytes]:
    """(address, bytes) of level LID's text record."""
    _, recs = level_records(v, lid)
    t = next(r for r in recs if r["name"] == "text")
    return t["address"], t["data"]


def gp_value(v: Version) -> int:
    """$gp: the executable's .reginfo gives it (ri_gp_value, the sixth word)."""
    addr, b = boot_sections(v)[".reginfo"]
    return struct.unpack_from("<6I", b)[5]


# --- programs.tsv --------------------------------------------------------------------------------

def sha1(b: bytes) -> str:
    return hashlib.sha1(b).hexdigest()


def program_rows(v: Version) -> list[tuple]:
    """(program, section, address, size, type, sha1) rows; the program's own row has section '-'."""
    rows = []
    b = v.exe_path.read_bytes()
    entry, = struct.unpack_from("<I", b, 0x18)
    rows.append(("boot", "-", entry, len(b), 0, sha1(b)))
    for s in elf_sections(b):
        if s["name"]:
            data = b"" if s["type"] == 8 else b[s["offset"]:s["offset"] + s["size"]]
            rows.append(("boot", s["name"], s["addr"], s["size"], s["type"], sha1(data)))
    for lid in range(LEVEL_COUNT):
        entry, recs = level_records(v, lid)
        prog = overlay_program(entry, recs)
        rows.append((f"level_{lid:02d}", "-", entry, len(prog), 0, sha1(prog)))
        for r in recs:
            rows.append((f"level_{lid:02d}", r["name"], r["address"], r["bytes"], r["type"], sha1(r["data"])))
    return rows


def programs_path(v: Version) -> Path:
    return v.config / "programs.tsv"


def write_config(v: Version) -> None:
    v.config.mkdir(parents=True, exist_ok=True)
    head = (f"# {v.label} ({v.id}): its programs and their sections, from the dump (tools/versions.py config).\n"
            "# A program's own row has section '-' and its entry point, size and SHA-1 as the disc stores it\n"
            "# (a level's after WAD decompression). Hashes, addresses and sizes only.\n"
            "# program\tsection\taddress\tsize\ttype\tsha1\n")
    programs_path(v).write_text(head + "".join(f"{p}\t{s}\t{a:08X}\t{n}\t{t}\t{h}\n"
                                               for p, s, a, n, t, h in program_rows(v)))
    print(f"wrote {programs_path(v).relative_to(ROOT)}")


def read_programs(v: Version) -> list[tuple]:
    rows = []
    for line in programs_path(v).read_text().splitlines():
        if line and not line.startswith("#"):
            p, s, a, n, t, h = line.split("\t")
            rows.append((p, s, int(a, 16), int(n), int(t), h))
    return rows


def check(v: Version) -> bool:
    want = read_programs(v)
    have = program_rows(v)
    bad = [w for w, h in zip(want, have) if w != h] + ([("count",)] if len(want) != len(have) else [])
    for r in bad[:10]:
        print(f"  {v.key}: differs: {r}")
    print(f"{v.key}: {'the dump is the committed programs' if not bad else f'{len(bad)} rows differ'}"
          f" ({sum(1 for r in want if r[1] == '-')} programs)")
    return not bad


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("command", nargs="?", choices=("dump", "config", "check"))
    ap.add_argument("--version", action="append", help="a version key or id (default: all)")
    a = ap.parse_args()
    if not a.command:
        print(f"{'key':<8} {'id':<16} {'label':<14} {'hz':>3}  disc image")
        for v in VERSIONS:
            print(f"{v.key:<8} {v.id:<16} {v.label:<14} {v.hz:>3}  baserom/{v.iso}" + ("  (primary)" if v.primary else ""))
        return
    vs = [get(k) for k in a.version] if a.version else list(VERSIONS)
    ok = True
    for v in vs:
        if a.command == "dump":
            dump(v)
        elif a.command == "config":
            write_config(v)
        else:
            ok &= check(v)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
