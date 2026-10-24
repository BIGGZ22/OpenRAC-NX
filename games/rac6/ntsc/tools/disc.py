#!/usr/bin/env python3
"""Read a PS2 disc image: identity, file list, and extraction of single files.

The image is the local target and never leaves the private workspace; this
tool only reports what a campaign needs to pin it: the volume identity, the
``SYSTEM.CNF`` content, the recorded file extents and the hashes.

Only plain ISO9660 is read (the format the retail disc uses).  The reader is
deliberately small and strict: a malformed record raises instead of being
skipped, so a wrong image cannot be measured silently.

Adapted from the disc reader of ``OpenRAC/rac5-sm-decomp`` (GPL-3.0); see
``THIRD_PARTY_NOTICES.md``.

Usage:
    python3 tools/disc.py inspect <image> [--json]
    python3 tools/disc.py list <image> [--tsv]
    python3 tools/disc.py hash <image>
    python3 tools/disc.py extract <image> <path> --out <file>
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
from dataclasses import dataclass
from pathlib import Path

SECTOR_SIZE = 2048
BLOCK_SIZE = 8 * 1024 * 1024


@dataclass(frozen=True)
class Entry:
    """One ISO9660 directory record."""

    name: str
    extent: int
    size: int
    directory: bool
    parent: str

    @property
    def end(self) -> int:
        return self.extent * SECTOR_SIZE + self.size

    @property
    def path(self) -> str:
        """The ISO9660 path of the entry, from the root of the image."""
        return f"{self.parent}/{self.name}" if self.parent else self.name


class IsoDisc:
    """A read-only view of an ISO9660 image."""

    def __init__(self, path: Path):
        self.path = Path(path)
        self.size = self.path.stat().st_size
        self.file = self.path.open("rb")
        try:
            pvd = self.read(16 * SECTOR_SIZE, SECTOR_SIZE)
            if pvd[:7] != b"\x01CD001\x01":
                raise ValueError("not an ISO9660 primary volume descriptor")
            self.volume_id = pvd[40:72].decode("ascii", "replace").strip()
            self.declared_size = struct.unpack_from("<I", pvd, 80)[0] * SECTOR_SIZE
            self.root = self._entry(pvd, 156, "")
        except Exception:
            self.file.close()
            raise

    def __enter__(self) -> "IsoDisc":
        return self

    def __exit__(self, *_exc: object) -> None:
        self.file.close()

    def read(self, offset: int, size: int) -> bytes:
        if offset < 0 or size < 0 or offset + size > self.size:
            raise ValueError(f"extent [{offset}, {offset + size}) exceeds image size {self.size}")
        self.file.seek(offset)
        data = self.file.read(size)
        if len(data) != size:
            raise ValueError("short read from image")
        return data

    @staticmethod
    def _entry(data: bytes, offset: int, parent: str) -> Entry:
        length = data[offset]
        if length < 34 or offset + length > len(data):
            raise ValueError("invalid ISO9660 directory record")
        extent = struct.unpack_from("<I", data, offset + 2)[0]
        size = struct.unpack_from("<I", data, offset + 10)[0]
        flags = data[offset + 25]
        name_length = data[offset + 32]
        if offset + 33 + name_length > offset + length:
            raise ValueError("ISO9660 record name exceeds its length")
        raw_name = data[offset + 33:offset + 33 + name_length]
        name = raw_name.decode("ascii", "replace").split(";", 1)[0]
        if name == "\x00":
            name = ""
        return Entry(name, extent, size, bool(flags & 2), parent)

    def entries(self, directory: Entry) -> dict[str, Entry]:
        if not directory.directory:
            raise ValueError(f"{directory.name} is not a directory")
        data = self.read(directory.extent * SECTOR_SIZE, directory.size)
        result: dict[str, Entry] = {}
        offset = 0
        while offset < len(data):
            if data[offset] == 0:
                offset = (offset // SECTOR_SIZE + 1) * SECTOR_SIZE
                continue
            entry = self._entry(data, offset, directory.path)
            offset += data[offset]
            if entry.name in ("\x00", "\x01"):
                continue
            if entry.name in result:
                raise ValueError(f"duplicate ISO9660 name {entry.name}")
            result[entry.name] = entry
        return result

    def walk(self) -> list[Entry]:
        """Return every file and directory of the image, parents before children."""
        found: list[Entry] = []
        seen: set[tuple[int, int]] = set()

        def visit(directory: Entry) -> None:
            key = (directory.extent, directory.size)
            if key in seen:
                return
            seen.add(key)
            found.append(directory)
            for entry in self.entries(directory).values():
                if entry.directory:
                    visit(entry)
                else:
                    found.append(entry)

        visit(self.root)
        return found
    def find(self, path: str) -> Entry:
        current = self.root
        for part in path.replace("\\", "/").strip("/").split("/"):
            if not part or part in (".", ".."):
                raise ValueError(f"invalid ISO9660 path: {path}")
            current = self.entries(current)[part.upper()]
        return current

    def file_bytes(self, path: str) -> bytes:
        entry = self.find(path)
        if entry.directory:
            raise ValueError(f"{path} is a directory")
        return self.read(entry.extent * SECTOR_SIZE, entry.size)

    def system_cnf(self) -> str:
        return self.file_bytes("SYSTEM.CNF").decode("ascii", "replace")

    def inspect(self) -> dict[str, object]:
        """Summarise the image: identity, boot file, version, and extent coverage."""
        system = self.system_cnf()
        boot = re.search(r"(?im)^\s*BOOT2\s*=\s*cdrom0:\\([^;\s]+)", system)
        version = re.search(r"(?im)^\s*VER\s*=\s*(\S+)", system)
        video = re.search(r"(?im)^\s*VMODE\s*=\s*(\S+)", system)
        files = [entry for entry in self.walk() if not entry.directory]
        highest = max((entry.end for entry in self.walk()), default=0)
        return {
            "volume_id": self.volume_id,
            "boot_file": boot.group(1) if boot else None,
            "version": version.group(1) if version else None,
            "video_mode": video.group(1) if video else None,
            "actual_size": self.size,
            "declared_size": self.declared_size,
            "missing_sectors": max(0, self.declared_size - self.size) // SECTOR_SIZE,
            "file_count": len(files),
            "highest_extent_end": highest,
            "all_recorded_extents_present": highest <= self.size,
            "system_cnf": system,
        }


def hash_image(path: Path) -> tuple[str, str]:
    """Return the SHA-1 and SHA-256 of a file, read in blocks."""
    sha1, sha256 = hashlib.sha1(), hashlib.sha256()
    with Path(path).open("rb") as source:
        while block := source.read(BLOCK_SIZE):
            sha1.update(block)
            sha256.update(block)
    return sha1.hexdigest(), sha256.hexdigest()


def command_inspect(args: argparse.Namespace) -> int:
    with IsoDisc(args.image) as disc:
        report = disc.inspect()
    if args.json:
        print(json.dumps(report, indent=2))
        return 0
    print(f"volume id      : {report['volume_id']}")
    print(f"boot file      : {report['boot_file']}")
    print(f"version / vmode: {report['version']} / {report['video_mode']}")
    print(f"size           : {report['actual_size']} declared {report['declared_size']}")
    print(f"files          : {report['file_count']}")
    print(f"extents present: {report['all_recorded_extents_present']}")
    return 0


def command_list(args: argparse.Namespace) -> int:
    with IsoDisc(args.image) as disc:
        entries = [entry for entry in disc.walk() if not entry.directory]
    if args.tsv:
        print("path\textent\tsize")
        for entry in entries:
            print(f"{entry.path}\t{entry.extent}\t{entry.size}")
        return 0
    for entry in sorted(entries, key=lambda item: item.path):
        print(f"{entry.size:>12}  {entry.path}")
    return 0


def command_hash(args: argparse.Namespace) -> int:
    sha1, sha256 = hash_image(args.image)
    print(f"size    : {Path(args.image).stat().st_size}")
    print(f"sha1    : {sha1}")
    print(f"sha256  : {sha256}")
    return 0


def command_extract(args: argparse.Namespace) -> int:
    with IsoDisc(args.image) as disc:
        data = disc.file_bytes(args.path)
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    if out.exists() and out.read_bytes() == data:
        print(f"already extracted: {out}")
        return 0
    out.write_bytes(data)
    print(f"extracted {args.path} -> {out} ({len(data)} bytes, SHA-256 {hashlib.sha256(data).hexdigest()})")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)

    inspect = commands.add_parser("inspect", help="report the identity of the image")
    inspect.add_argument("image", type=Path)
    inspect.add_argument("--json", action="store_true")
    inspect.set_defaults(run=command_inspect)

    listing = commands.add_parser("list", help="list the files of the image")
    listing.add_argument("image", type=Path)
    listing.add_argument("--tsv", action="store_true")
    listing.set_defaults(run=command_list)

    hashing = commands.add_parser("hash", help="hash the image")
    hashing.add_argument("image", type=Path)
    hashing.set_defaults(run=command_hash)

    extract = commands.add_parser("extract", help="extract one file of the image")
    extract.add_argument("image", type=Path)
    extract.add_argument("path")
    extract.add_argument("--out", type=Path, required=True)
    extract.set_defaults(run=command_extract)

    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    raise SystemExit(main())
