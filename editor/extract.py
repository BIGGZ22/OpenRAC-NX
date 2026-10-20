#!/usr/bin/env python3
"""Extract Ratchet & Clank levels from your own disc: PAL (SCES_509.16 v2.00) or NTSC-U
(SCUS_971.99 v1.00), told apart by the executable on it (disc.VERSIONS). Going Commando
(SCUS_972.68), Up Your Arsenal (SCUS_973.53) and Deadlocked (SCUS_974.65), NTSC-U, are read
too, in early access (wads.VERSIONS): survey, godot, port and raw work for them, pack does not yet.

  extract.py survey ISO                       disc layout and references, as JSON
  extract.py godot ISO OUT [--level N ...]    a Godot 4 project of editable levels
  extract.py raw ISO OUT --level N            one level's sections, as stored and decoded
  extract.py port ISO OUT [--level N ...]     levels for the native port's viewer (port/viewer)

OUT must be a new directory under this repository's assets/ or build/,
both ignored by git. Output is built beside it and moved into place only
when complete. Never commit or share it: see LEGAL.md.
"""

import argparse
from concurrent.futures import Executor, ProcessPoolExecutor, ThreadPoolExecutor, as_completed
import hashlib
import json
import os
from pathlib import Path
import pickle
import shutil
import struct
import subprocess
import sys
import tempfile

from disc import LEVEL_COUNT, Disc
from formats import FormatError
from godot import LevelWriter, write_project
from spaceships import add_spaceships
from level import load_flight, load_level, load_title
from level import load_wad_level
from lighting import normal_table
from pack import pack
from port import PortLevelWriter
import wads

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


def adopt_flat_export(out: Path, version: str, kind: str = "port") -> bool:
    """What was written before each version had a folder of its own sat straight in OUT's parent:
    the port's levels (assets/port/rac1, <data>/port), the Godot projects (assets/levels/rac1) and
    the level packer's output (assets/packed/rac1). When OUT does not exist yet and the parent holds
    that of VERSION, its level folders and the files beside them move into OUT, so the new layout
    starts from them (a Godot project keeps its edits) instead of leaving them beside it. Only PAL
    had the flat layout; the port's levels also say their version in their manifests. KIND is
    "port", "godot" or "packed". True when something moved."""
    parent = out.parent
    if out.exists() or not parent.is_dir():
        return False
    marker = {"port": "manifest.json", "godot": "project.godot", "packed": "gameplay_pal.wad"}[kind]
    levels = [d for d in sorted(parent.glob("level_*")) if d.is_dir() and (d / marker).is_file()]
    if not levels:
        return False
    if kind == "port":
        try:
            found = json.loads((levels[-1] / marker).read_text()).get("version", "pal")
        except (OSError, ValueError):
            return False
    else:
        found = "pal"
    if found != version:
        return False
    out.mkdir(parents=True)
    files = ("verified", "patches.tsv", "packed.json", "changes.txt")
    for item in sorted(parent.iterdir()):
        if (item.is_dir() and item.name.startswith("level_")) or (item.is_file() and item.name.startswith(files)):
            item.rename(out / item.name)
    return True


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


def worker_pool(jobs: int) -> Executor:
    """JOBS processes for the levels. Where Python has no process pools (Android's CPython has no
    sem_open, so no multiprocessing), each level is written by a Python of its own instead
    (SubprocessPool)."""
    try:
        return ProcessPoolExecutor(jobs)
    except (ImportError, NotImplementedError, OSError):
        return SubprocessPool(jobs)


# What SubprocessPool may run: the level exports below.
WORKER_FUNCTIONS = ("export_level", "export_port_level", "export_port_title", "export_port_flight",
                    "export_wad_level", "export_port_wad_level")
# A phone's memory, not its cores, limits how many levels are written at once.
SUBPROCESS_JOBS = 4


class SubprocessPool(Executor):
    """A process pool without multiprocessing: each call runs `extract.py --worker` (this file, in
    the same Python), its function's name and arguments pickled in and its result pickled out,
    up to SUBPROCESS_JOBS at a time."""

    def __init__(self, jobs: int):
        self._threads = ThreadPoolExecutor(max(1, min(jobs, SUBPROCESS_JOBS)))

    def submit(self, fn, /, *args, **kwargs):
        return self._threads.submit(run_worker, fn.__name__, args, kwargs)

    def shutdown(self, wait: bool = True, *, cancel_futures: bool = False) -> None:
        self._threads.shutdown(wait, cancel_futures=cancel_futures)


def run_worker(name: str, args: tuple, kwargs: dict):
    done = subprocess.run([sys.executable, str(Path(__file__).resolve()), "--worker"],
                          input=pickle.dumps((name, args, kwargs)), capture_output=True, check=False)
    sys.stderr.write(done.stderr.decode(errors="replace"))
    if done.returncode:
        raise RuntimeError(f"{name} failed in its own process (exit status {done.returncode})")
    return pickle.loads(done.stdout)


def worker() -> None:
    """One call for SubprocessPool: (name, args, kwargs) on stdin, the result on stdout."""
    name, args, kwargs = pickle.load(sys.stdin.buffer)
    if name not in WORKER_FUNCTIONS:
        sys.exit(f"{name} is not one of the level exports")
    stdout = sys.stdout
    sys.stdout = sys.stderr  # what the export prints must not mix with its result
    try:
        value = globals()[name](*args, **kwargs)
    finally:
        sys.stdout = stdout
    result = stdout.buffer
    result.write(pickle.dumps(value))
    result.flush()


def export_level(iso: Path, info: dict, out: Path, lod: int) -> dict:
    """One level of a Godot project; runs in its own process."""
    with Disc(iso) as disc:
        return LevelWriter(out, load_level(disc, info)).write(lod)


def open_later(iso: Path) -> tuple[Disc, wads.WadDisc]:
    disc = Disc(iso)
    return disc, wads.WadDisc(disc, wads.identify(disc))


def export_wad_level(iso: Path, number: int, out: Path, lod: int) -> dict:
    """One Going Commando, Up Your Arsenal or Deadlocked level of a Godot project; runs in its
    own process."""
    disc, source = open_later(iso)
    with disc:
        return LevelWriter(out, load_wad_level(source, source.level(number))).write(lod)


def export_port_wad_level(iso: Path, number: int, out: Path, lod: int) -> dict:
    """One later game's level in the port's layout (for the overrides packs and the viewer)."""
    disc, source = open_later(iso)
    with disc:
        return PortLevelWriter(out, load_wad_level(source, source.level(number))).write(lod)


def godot(iso: Path, survey: dict, levels: list[int], lod: int, jobs: int, out: Path, split: bool = False,
          region: str = "PAL", game: str = "rac1") -> None:
    """Levels are independent, so they are written in parallel. SPLIT: a project per level,
    OUT/level_NN, each with only its own level to import and open. GAME: the disc's game; a later
    game's levels are read by number (wads.WadDisc.level), Ratchet & Clank's by survey."""
    where = {i: out / f"level_{i:02}" if split else out for i in levels}
    if split:
        for i in levels:
            write_project(where[i], [i], region, game)
    else:
        write_project(out, levels, region, game)
    with worker_pool(min(jobs, len(levels))) as pool:
        if game == "rac1":
            running = [pool.submit(export_level, iso, survey["levels"][i], where[i], lod) for i in levels]
        else:
            running = [pool.submit(export_wad_level, iso, i, where[i], lod) for i in levels]
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


def port(iso: Path, survey: dict, levels: list[int], lod: int, jobs: int, out: Path, game: str = "rac1") -> None:
    """OUT/level_NN/ per level (port.py), written in parallel like godot(). A later game's levels
    have no title world or flight to add."""
    out.mkdir(parents=True, exist_ok=True)
    with worker_pool(min(jobs, len(levels))) as pool:
        if game == "rac1":
            running = [pool.submit(export_port_level, iso, survey["levels"][i], out, lod) for i in levels]
            running.append(pool.submit(export_port_title, iso, survey, out, lod))
            running.append(pool.submit(export_port_flight, iso, survey, out, lod))
        else:
            running = [pool.submit(export_port_wad_level, iso, i, out, lod) for i in levels]
        try:
            for done in as_completed(running):
                stats = done.result()
                print(f"level {stats['level']:02}: {stats['mesh_instances']} meshes placed, "
                      f"{stats['triangles']} triangles, {stats['textures']} textures", flush=True)
        except BaseException:
            pool.shutdown(cancel_futures=True)
            raise


def raw_later(source: wads.WadDisc, level_id: int, out: Path) -> None:
    """raw() for a later game's level: its sections as stored and decoded."""
    level = load_wad_level(source, source.level(level_id))
    files = {"level_header.bin": level.header, "core_index.bin": level.index, "core_data.bin": level.core,
             "gameplay.bin": level.gameplay}
    if level.art is not None:
        files["art_instances.bin"] = level.art
    files.update({f"stored/{name}.bin": data for name, data in level.stored.items()})
    manifest = {"schema": 1, "game": source.version.game, "level": level_id, "overlay": level.overlay, "files": []}
    for name, data in files.items():
        (out / name).parent.mkdir(parents=True, exist_ok=True)
        (out / name).write_bytes(data)
        manifest["files"].append({"path": name, "bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()})
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")


def later_main(args, parser, disc: Disc, version: wads.Version) -> None:
    """The commands for Going Commando's, Up Your Arsenal's and Deadlocked's discs."""
    source = wads.WadDisc(disc, version)
    survey = source.survey()
    numbers = [level["id"] for level in survey["levels"] if "ranges" in level]
    if args.command == "survey":
        json.dump(survey, sys.stdout, indent=2)
        print()
        return
    if args.command == "pack":
        raise FormatError(f"packing levels back is Ratchet & Clank's only for now; {version.game}'s levels "
                          "can be extracted, opened in Godot and used for texture and model packs")
    wanted = sorted(set(args.level or numbers))
    unknown = [n for n in wanted if n not in numbers]
    if unknown:
        raise FormatError(f"{version.game} has no level {unknown[0]}; its levels are {', '.join(map(str, numbers))}")
    if args.command == "godot":
        dest = publish(args.out, lambda out: godot(args.iso, survey, wanted, args.terrain_lod, max(1, args.jobs), out,
                                                   args.split, version.region, version.game), args.anywhere)
        print(f"Godot project{'s, one per level' if args.split else ''}: {dest}")
    elif args.command == "port":
        dest = publish(args.out, lambda out: port(args.iso, survey, wanted, args.terrain_lod, max(1, args.jobs), out,
                                                  version.game), args.anywhere, args.replace)
        print(f"Levels in the port's layout (early access, not playable yet): {dest}")
    else:
        dest = publish(args.out, lambda out: raw_later(source, args.level[0], out))
        print(f"Level {args.level[0]} sections: {dest}")


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
    export.add_argument("--level", type=int, action="append",
                        help="a level to export (repeatable; default: all)")
    export.add_argument("--jobs", type=int, default=os.cpu_count() or 1,
                        help="levels to export at once (default: one per CPU)")
    export.add_argument("--terrain-lod", type=int, choices=(0, 2), default=0,
                        help="terrain detail: 0 finest (default), 2 coarsest")
    export.add_argument("--split", action="store_true",
                        help="a Godot project per level (OUT/level_NN): each opens in seconds")
    export.add_argument("--anywhere", action="store_true",
                        help="OUT may be outside this repository (the installed launcher's data folder)")
    native = commands.add_parser("port")
    native.add_argument("iso", type=Path)
    native.add_argument("out", type=Path)
    native.add_argument("--level", type=int, action="append",
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
    packing.add_argument("--anywhere", action="store_true",
                         help="OUT may be outside this repository (the installed launcher's data folder)")
    dump = commands.add_parser("raw")
    dump.add_argument("iso", type=Path)
    dump.add_argument("out", type=Path)
    dump.add_argument("--level", type=int, required=True, action="append")
    args = parser.parse_args()
    try:
        with Disc(args.iso) as disc:
            later = wads.identify(disc)
            if later is not None:
                later_main(args, parser, disc, later)
                return
            # Ratchet & Clank: its 19 levels, numbered 0 to 18.
            if getattr(args, "level", None) and any(n not in range(LEVEL_COUNT) for n in args.level):
                raise FormatError(f"Ratchet & Clank's levels are 0 to {LEVEL_COUNT - 1}")
            survey = disc.survey()
            if args.command == "survey":
                json.dump(survey, sys.stdout, indent=2)
                print()
            elif args.command == "godot":
                levels = sorted(set(args.level or range(LEVEL_COUNT)))
                if adopt_flat_export(args.out.resolve(), disc.version.key, "godot"):
                    # Projects made before the per-version folders, edits and all: kept, not
                    # written over.
                    print(f"moved the Godot projects made before into {args.out.resolve()}")
                    return
                dest = publish(args.out, lambda out: godot(args.iso, survey, levels, args.terrain_lod, max(1, args.jobs), out,
                                                           args.split, disc.version.region), args.anywhere)
                print(f"Godot project{'s, one per level' if args.split else ''}: {dest}")
            elif args.command == "port":
                levels = sorted(set(args.level or range(LEVEL_COUNT)))
                if args.replace and adopt_flat_export(args.out.resolve(), disc.version.key):
                    print(f"moved the levels extracted before into {args.out.resolve()}")
                dest = publish(args.out, lambda out: port(args.iso, survey, levels, args.terrain_lod, max(1, args.jobs), out),
                               args.anywhere, args.replace)
                print(f"Levels for the port's viewer: {dest}")
            elif args.command == "pack":
                # Its own output, written again after each edit: the one command that may write
                # into a directory that exists.
                dest = args.out.resolve()
                if adopt_flat_export(args.project.resolve(), disc.version.key, "godot"):
                    print(f"moved the Godot projects made before into {args.project.resolve()}")
                if adopt_flat_export(dest, disc.version.key, "packed"):
                    print(f"moved the packed levels made before into {dest}")
                if not args.anywhere and not any(root in dest.parents for root in (ROOT / "assets", ROOT / "build")):
                    raise FormatError("output must be a directory under this repository's assets/ or build/")
                if not (args.project / "project.godot").is_file() and not list(args.project.glob("level_*/project.godot")):
                    raise FormatError(f"{args.project} is not the editor's Godot project or its folder of "
                                      "per-level projects (run `godot` first)")
                for stats in [*args.project.glob("levels/*/level.json"), *args.project.glob("level_*/levels/*/level.json")]:
                    game = json.loads(stats.read_text()).get("game", "rac1")
                    if game != "rac1":
                        raise FormatError(f"{args.project} holds {game}'s levels, which are not packed back yet")
                levels, mobys = pack(disc, survey, args.project, dest)
                print((dest / "changes.txt").read_text(), end="")
                print(f"Packed {levels} level(s), {mobys} moby edit(s): {dest}")
            else:
                dest = publish(args.out, lambda out: raw(disc, survey, args.level[0], out))
                print(f"Level {args.level[0]} sections: {dest}")
    except (OSError, FormatError, struct.error) as exc:
        parser.exit(1, f"extract: {exc}\n")


if __name__ == "__main__":
    if sys.argv[1:] == ["--worker"]:
        worker()
    else:
        main()
