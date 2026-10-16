#!/usr/bin/env python3
"""Extract Ratchet & Clank levels from your own disc: PAL (SCES_509.16 v2.00) or NTSC-U
(SCUS_971.99 v1.00), told apart by the executable on it (disc.VERSIONS).

  extract.py survey ISO                       disc layout and references, as JSON
  extract.py godot ISO OUT [--level N ...]    a Godot 4 project of editable levels
  extract.py raw ISO OUT --level N            one level's sections, as stored and decoded
  extract.py port ISO OUT [--level N ...]     levels for the native port's viewer (port/viewer)

OUT must be a new directory under this repository's assets/ or build/,
both ignored by git. Output is built beside it and moved into place only
when complete. Never commit or share it: see LEGAL.md.
"""

import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import sys
import tempfile

from disc import LEVEL_COUNT, Disc
from formats import FormatError
from godot import LevelWriter, write_project
from spaceships import add_spaceships
from level import load_flight, load_level, load_title
from lighting import normal_table
from pack import pack
from port import PortLevelWriter

ROOT = Path(__file__).resolve().parents[1]


def output_path(path: Path, anywhere: bool = False, replace: bool = False) -> Path:
    """A new directory inside assets/ or build/, after resolving symlinks. ANYWHERE: any new
    directory (an installed launcher has no checkout to write in: it names its own data folder)."""
    resolved = path.resolve()
    if not anywhere and not any(root in resolved.parents for root in (ROOT / "assets", ROOT / "build")):
        raise FormatError("output must be a new directory under this repository's assets/ or build/")
    if resolved.exists() and not replace:
        raise FormatError(f"{resolved} already exists; choose a new directory")
    return resolved


def publish(path: Path, build, anywhere: bool = False, replace: bool = False) -> Path:
    """Run build(directory) in a temporary sibling, then rename it to path. REPLACE: a directory
    already there gives way to the new one once that is whole (never before: a failed run leaves
    the old one as it was)."""
    dest = output_path(path, anywhere, replace)
    dest.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=f".{dest.name}-", dir=dest.parent))
    try:
        build(temporary)
        if dest.exists():
            old = Path(tempfile.mkdtemp(prefix=f".{dest.name}-old-", dir=dest.parent))
            dest.rename(old / dest.name)
            temporary.rename(dest)
            shutil.rmtree(old)
        else:
            temporary.rename(dest)
    finally:
        if temporary.exists():
            shutil.rmtree(temporary)
    return dest


def export_level(iso: Path, info: dict, out: Path, lod: int) -> dict:
    """One level of a Godot project; runs in its own process."""
    with Disc(iso) as disc:
        return LevelWriter(out, load_level(disc, info)).write(lod)


def godot(iso: Path, survey: dict, levels: list[int], lod: int, jobs: int, out: Path, split: bool = False,
          region: str = "PAL") -> None:
    """Levels are independent, so they are written in parallel. SPLIT: a project per level,
    OUT/level_NN, each with only its own level to import and open."""
    where = {i: out / f"level_{i:02}" if split else out for i in levels}
    if split:
        for i in levels:
            write_project(where[i], [i], region)
    else:
        write_project(out, levels, region)
    with ProcessPoolExecutor(min(jobs, len(levels))) as pool:
        running = [pool.submit(export_level, iso, survey["levels"][i], where[i], lod) for i in levels]
        try:
            for done in as_completed(running):
                stats = done.result()
                print(f"level {stats['level']:02}: {stats['mesh_instances']} meshes placed, "
                      f"{stats['triangles']} triangles, {stats['textures']} textures", flush=True)
        except BaseException:
            pool.shutdown(cancel_futures=True)
            raise


def normals(disc: Disc) -> list[tuple[int, int]]:
    """The executable's (cos, sin) table, where the disc's version keeps it (lighting.py)."""
    return normal_table(disc.executable(), disc.version.normal_table)


def export_port_level(iso: Path, info: dict, out: Path, lod: int) -> dict:
    """One level for the port's viewer; runs in its own process."""
    with Disc(iso) as disc:
        level = load_level(disc, info)
        level.extra_moby_classes = add_spaceships(disc, disc.survey(), level)
        return PortLevelWriter(out, level, normals(disc)).write(lod)


def export_port_title(iso: Path, survey: dict, out: Path, lod: int) -> dict:
    """The title world as level 99 for the port's viewer (level.load_title)."""
    with Disc(iso) as disc:
        return PortLevelWriter(out, load_title(disc, survey), normals(disc)).write(lod)


def export_port_flight(iso: Path, survey: dict, out: Path, lod: int) -> dict:
    """The flight between planets' sky as level 98 for the port's viewer (level.load_flight)."""
    with Disc(iso) as disc:
        return PortLevelWriter(out, load_flight(disc, survey), normals(disc)).write(lod)


def port(iso: Path, survey: dict, levels: list[int], lod: int, jobs: int, out: Path) -> None:
    """OUT/level_NN/ per level (port.py), written in parallel like godot()."""
    out.mkdir(parents=True, exist_ok=True)
    with ProcessPoolExecutor(min(jobs, len(levels))) as pool:
        running = [pool.submit(export_port_level, iso, survey["levels"][i], out, lod) for i in levels]
        running.append(pool.submit(export_port_title, iso, survey, out, lod))
        running.append(pool.submit(export_port_flight, iso, survey, out, lod))
        try:
            for done in as_completed(running):
                stats = done.result()
                print(f"level {stats['level']:02}: {stats['mesh_instances']} meshes placed, "
                      f"{stats['triangles']} triangles, {stats['textures']} textures", flush=True)
        except BaseException:
            pool.shutdown(cancel_futures=True)
            raise


def raw(disc: Disc, survey: dict, level_id: int, out: Path) -> None:
    level = load_level(disc, survey["levels"][level_id])
    files = {"level_header.bin": level.header, "core_index.bin": level.index,
             "core_data.bin": level.core, f"{disc.version.gameplay}.bin": level.gameplay}
    files.update({f"stored/{name}.bin": data for name, data in level.stored.items()})
    manifest = {"schema": 1, "level": survey["levels"][level_id], "overlay": level.overlay, "files": []}
    for name, data in files.items():
        (out / name).parent.mkdir(parents=True, exist_ok=True)
        (out / name).write_bytes(data)
        manifest["files"].append({"path": name, "bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()})
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("survey").add_argument("iso", type=Path)
    export = commands.add_parser("godot")
    export.add_argument("iso", type=Path)
    export.add_argument("out", type=Path)
    export.add_argument("--level", type=int, action="append", choices=range(LEVEL_COUNT),
                        help="a level to export (repeatable; default: all)")
    export.add_argument("--jobs", type=int, default=os.cpu_count() or 1,
                        help="levels to export at once (default: one per CPU)")
    export.add_argument("--terrain-lod", type=int, choices=(0, 2), default=0,
                        help="terrain detail: 0 finest (default), 2 coarsest")
    export.add_argument("--split", action="store_true",
                        help="a Godot project per level (OUT/level_NN): each opens in seconds")
    native = commands.add_parser("port")
    native.add_argument("iso", type=Path)
    native.add_argument("out", type=Path)
    native.add_argument("--level", type=int, action="append", choices=range(LEVEL_COUNT),
                        help="a level to export (repeatable; default: all)")
    native.add_argument("--jobs", type=int, default=os.cpu_count() or 1,
                        help="levels to export at once (default: one per CPU)")
    native.add_argument("--terrain-lod", type=int, choices=(0, 2), default=0,
                        help="terrain detail: 0 finest (default), 2 coarsest")
    native.add_argument("--replace", action="store_true",
                        help="OUT may exist: it is replaced by the new export once that is complete")
    native.add_argument("--anywhere", action="store_true",
                        help="OUT may be outside this repository (the installed launcher's data folder)")
    packing = commands.add_parser("pack")
    packing.add_argument("iso", type=Path)
    packing.add_argument("project", type=Path,
                         help="the Godot project the levels were edited in, or the folder of per-level projects")
    packing.add_argument("out", type=Path, help="where the packed levels go (under assets/ or build/)")
    dump = commands.add_parser("raw")
    dump.add_argument("iso", type=Path)
    dump.add_argument("out", type=Path)
    dump.add_argument("--level", type=int, required=True, choices=range(LEVEL_COUNT))
    args = parser.parse_args()
    try:
        with Disc(args.iso) as disc:
            survey = disc.survey()
            if args.command == "survey":
                json.dump(survey, sys.stdout, indent=2)
                print()
            elif args.command == "godot":
                levels = sorted(set(args.level or range(LEVEL_COUNT)))
                dest = publish(args.out, lambda out: godot(args.iso, survey, levels, args.terrain_lod, max(1, args.jobs), out,
                                                           args.split, disc.version.region))
                print(f"Godot project{'s, one per level' if args.split else ''}: {dest}")
            elif args.command == "port":
                levels = sorted(set(args.level or range(LEVEL_COUNT)))
                dest = publish(args.out, lambda out: port(args.iso, survey, levels, args.terrain_lod, max(1, args.jobs), out),
                               args.anywhere, args.replace)
                print(f"Levels for the port's viewer: {dest}")
            elif args.command == "pack":
                if disc.version.key != "pal":
                    # The packed file replaces the PAL gameplay copy, and mod packs are PAL's
                    # (overrides.py); the NTSC-U copy is not packed yet.
                    raise FormatError(f"packing edits reads the PAL disc only for now, not {disc.version.label}")
                # Its own output, written again after each edit: the one command that may write
                # into a directory that exists.
                dest = args.out.resolve()
                if not any(root in dest.parents for root in (ROOT / "assets", ROOT / "build")):
                    raise FormatError("output must be a directory under this repository's assets/ or build/")
                if not (args.project / "project.godot").is_file() and not list(args.project.glob("level_*/project.godot")):
                    raise FormatError(f"{args.project} is not the editor's Godot project or its folder of "
                                      "per-level projects (run `godot` first)")
                levels, mobys = pack(disc, survey, args.project, dest)
                print((dest / "changes.txt").read_text(), end="")
                print(f"Packed {levels} level(s), {mobys} moby edit(s): {dest}")
            else:
                dest = publish(args.out, lambda out: raw(disc, survey, args.level, out))
                print(f"Level {args.level} sections: {dest}")
    except (OSError, FormatError, struct.error) as exc:
        parser.exit(1, f"extract: {exc}\n")


if __name__ == "__main__":
    main()
