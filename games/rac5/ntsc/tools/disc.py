#!/usr/bin/env python3
"""Inspect an ISO9660 image and extract the pinned PS2 boot executable locally.

Only ISO9660 level-one directory records are needed for the current disc.
This tool never writes retail bytes outside the ignored ``baserom/`` directory.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SECTOR_SIZE = 2048


@dataclass(frozen=True)
class Entry:
    name: str
    extent: int
    size: int
    directory: bool

    @property
    def end(self) -> int:
        return self.extent * SECTOR_SIZE + self.size


class IsoDisc:
    def __init__(self, path: Path):
        self.path = path
        self.size = path.stat().st_size
        self.file = path.open("rb")
        try:
            pvd = self.read(16 * SECTOR_SIZE, SECTOR_SIZE)
            if pvd[:7] != b"\x01CD001\x01":
                raise ValueError("not an ISO9660 primary volume descriptor")
            self.volume_id = pvd[40:72].decode("ascii", "replace").strip()
            self.declared_size = struct.unpack_from("<I", pvd, 80)[0] * SECTOR_SIZE
            self.root = self._entry(pvd, 156)
            if not self.root.directory:
                raise ValueError("ISO9660 root record is not a directory")
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
    def _entry(data: bytes, offset: int) -> Entry:
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
        return Entry(name, extent, size, bool(flags & 2))

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
            entry = self._entry(data, offset)
            offset += data[offset]
            if entry.name not in ("\x00", "\x01"):
                if entry.name in result:
                    raise ValueError(f"duplicate ISO9660 name {entry.name}")
                result[entry.name] = entry
        return result

    def find(self, path: str) -> Entry:
        current = self.root
        for part in path.replace("\\", "/").split("/"):
            if not part or part in (".", ".."):
                raise ValueError("invalid ISO9660 path")
            current = self.entries(current)[part.upper()]
        return current

    def file_bytes(self, path: str) -> bytes:
        entry = self.find(path)
        if entry.directory:
            raise ValueError(f"{path} is a directory")
        return self.read(entry.extent * SECTOR_SIZE, entry.size)

    def extent_summary(self) -> tuple[int, int, int]:
        """Return (file count, highest file end, highest directory end)."""
        files = 0
        max_file_end = 0
        max_directory_end = self.root.end
        seen: set[tuple[int, int]] = set()

        def visit(directory: Entry, depth: int) -> None:
            nonlocal files, max_file_end, max_directory_end
            if depth > 16:
                raise ValueError("ISO9660 directory nesting is unexpectedly deep")
            key = (directory.extent, directory.size)
            if key in seen:
                return
            seen.add(key)
            for entry in self.entries(directory).values():
                if entry.directory:
                    max_directory_end = max(max_directory_end, entry.end)
                    visit(entry, depth + 1)
                else:
                    files += 1
                    max_file_end = max(max_file_end, entry.end)

        visit(self.root, 0)
        return files, max_file_end, max_directory_end

    def inspect(self) -> dict[str, object]:
        system = self.file_bytes("SYSTEM.CNF").decode("ascii", "replace")
        boot = re.search(r"(?im)^\s*BOOT2\s*=\s*cdrom0:\\([^;\s]+)", system)
        if not boot:
            raise ValueError("SYSTEM.CNF has no BOOT2 cdrom0 entry")
        version = re.search(r"(?im)^\s*VER\s*=\s*(\S+)", system)
        video = re.search(r"(?im)^\s*VMODE\s*=\s*(\S+)", system)
        files, max_file_end, max_directory_end = self.extent_summary()
        return {
            "volume_id": self.volume_id,
            "boot_file": boot.group(1),
            "version": version.group(1) if version else None,
            "video_mode": video.group(1) if video else None,
            "actual_size": self.size,
            "declared_size": self.declared_size,
            "missing_sectors": max(0, self.declared_size - self.size) // SECTOR_SIZE,
            "file_count": files,
            "highest_file_end": max_file_end,
            "highest_directory_end": max_directory_end,
            "all_recorded_extents_present": max(max_file_end, max_directory_end) <= self.size,
        }


def extract_boot(disc: IsoDisc, output: Path, *, allow_short_image: bool) -> str:
    info = disc.inspect()
    target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
    if info["boot_file"] != target["boot_file"]:
        raise ValueError(f"wrong boot executable: {info['boot_file']}")
    if not allow_short_image:
        verify_disc(disc, target, info)
    data = disc.file_bytes(str(info["boot_file"]))
    digest = hashlib.sha256(data).hexdigest()
    if len(data) != target["boot_size"] or digest != target["boot_sha256"].lower():
        raise ValueError("boot size/hash differs from the pinned provisional target")
    if output.exists():
        if output.read_bytes() != data:
            raise FileExistsError(f"{output} already exists with different bytes")
        return digest
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(data)
    return digest


def hash_image(path: Path) -> tuple[str, str]:
    sha1, sha256 = hashlib.sha1(), hashlib.sha256()
    with path.open("rb") as source:
        while block := source.read(8 * 1024 * 1024):
            sha1.update(block)
            sha256.update(block)
    return sha1.hexdigest(), sha256.hexdigest()


def verify_disc(disc: IsoDisc, target: dict[str, object], info: dict[str, object] | None = None) -> None:
    info = info or disc.inspect()
    if info["boot_file"] != target["boot_file"] or info["version"] != target["system_cnf_version"]:
        raise ValueError("disc boot path/version differs from the pinned target")
    if info["video_mode"] != "NTSC" or info["actual_size"] != info["declared_size"]:
        raise ValueError("disc video mode or ISO9660 length differs from the pinned target")
    if info["actual_size"] != target["iso_size"] or not info["all_recorded_extents_present"]:
        raise ValueError("disc size or file extents differ from the pinned target")
    sha1, sha256 = hash_image(disc.path)
    if sha1 != target["iso_sha1"] or sha256 != target["iso_sha256"]:
        raise ValueError("disc SHA-1/SHA-256 differs from the pinned target")
    boot = disc.file_bytes(str(target["boot_file"]))
    if len(boot) != target["boot_size"] or hashlib.sha256(boot).hexdigest() != target["boot_sha256"]:
        raise ValueError("boot size/hash differs from the pinned target")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("inspect", "verify", "extract-boot"))
    parser.add_argument("image", type=Path)
    parser.add_argument("--allow-short-image", action="store_true")
    parser.add_argument("--out", type=Path, default=ROOT / "baserom/SCUS_976.15")
    args = parser.parse_args()
    with IsoDisc(args.image) as disc:
        if args.command == "inspect":
            print(json.dumps(disc.inspect(), indent=2))
        elif args.command == "verify":
            target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
            verify_disc(disc, target)
            print(f"verified {target['serial']}: complete ISO and boot hashes match config/target.json")
        else:
            digest = extract_boot(disc, args.out, allow_short_image=args.allow_short_image)
            print(f"boot extracted locally: {args.out} (SHA-256 {digest})")


if __name__ == "__main__":
    main()
