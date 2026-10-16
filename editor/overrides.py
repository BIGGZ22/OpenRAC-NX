#!/usr/bin/env python3
"""Overrides packs: replacement textures, models and level edits the native
port loads on top of the extracted levels, without reading the disc again.

  overrides.py check SOURCE                      what a pack (.zip or folder) holds, and what is wrong with it
  overrides.py add SOURCE --into OVERRIDES       copy a pack in (the source is never changed)
  overrides.py list --into OVERRIDES [--json]    the packs, in the order they apply
  overrides.py enable|disable ID --into OVERRIDES
  overrides.py move ID up|down|first|last --into OVERRIDES
  overrides.py remove ID --into OVERRIDES        delete the copy `add` made
  overrides.py stack --into OVERRIDES --out MODS [--packed PACKED]
                                                 the folder the port's --mods reads
  overrides.py zip ID OUT.zip --into OVERRIDES   a pack as a .zip to share

A pack is a folder, or a .zip of one, with openrac-mod.json at its root (or
in the zip's one top folder) and files laid out as the extracted levels are:

  openrac-mod.json                  {"format": 1, "game": "rac1", "version": "pal",
                                     "name": ..., "author": ..., "description": ...,
                                     "levels_format": 2, "replaces": {...}}
  level_NN/textures/<name>.png      a texture, any size of the same shape
  level_NN/mobys/moby_<class>.glb   a class's model, as the port reads it
  level_NN/ties/tie_<class>.glb, level_NN/shrubs/shrub_<class>.glb
  level_NN/placements.json          ties and shrubs placed again (the level packer's)
  level_NN/gameplay_pal.wad, patches.tsv   a packed level (the level packer's)
  README*, LICENSE*, *.txt, *.md    read by people only

An OVERRIDES folder holds the packs that were added (packs/<id>/) and
overrides.json, their order and which are on: {"format": 1, "packs":
[{"id": ..., "enabled": true}, ...]}; a later pack wins over an earlier one.
The launcher reads and changes the same file (launcher/core/src/overrides.rs).

`stack` puts the level packer's output (PACKED) and every enabled pack, in
order, into MODS: the folder `--mods` names. The port draws each level's
files from MODS/level_NN/ where they are there, and from the extracted
levels otherwise (port/viewer/level.cpp), and reads the disc sectors
patches.tsv lists from the files beside it (port/game/common/lib/cdvd.c).
"""

import argparse
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import struct
import sys
import tempfile
import zipfile

MANIFEST = "openrac-mod.json"
STATE = "overrides.json"
MARKER = ".openrac-stack"
PACK_FORMAT = 1
GAME, VERSION = "rac1", "pal"
LEVELS_FORMAT = 2
FILES = re.compile(r"^(?:patches\.tsv|level_\d{2}/(?:textures/[A-Za-z0-9_.-]+\.png|(?:mobys|ties|shrubs)/"
                   r"[A-Za-z0-9_.-]+\.glb|placements\.json|gameplay_pal\.wad))$")
PEOPLE = re.compile(r"^(?:[^/]+\.(?:txt|md)|(?:README|LICENSE|CREDITS)[^/]*)$", re.IGNORECASE)
MAX_TOTAL = 4 << 30
MAX_FILES = 200_000


class PackError(ValueError):
    """A pack that cannot be used, said so its user can act on it."""


def describe(files) -> dict:
    """What a pack's files replace, per level: textures, models, and level edits."""
    out = {}
    for path in sorted(files):
        parts = path.split("/")
        if len(parts) < 2 or not parts[0].startswith("level_"):
            continue
        level = out.setdefault(parts[0], {})
        if parts[1] == "textures":
            level.setdefault("textures", []).append(parts[2][:-4])
        elif parts[1] in ("mobys", "ties", "shrubs"):
            level.setdefault("models", []).append(parts[2][:-4])
        else:
            level.setdefault("level", []).append(parts[1])
    return out


def read_manifest(data: bytes, where: str) -> dict:
    try:
        manifest = json.loads(data.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise PackError(f"{where}: {MANIFEST} is not readable JSON ({exc})") from None
    if not isinstance(manifest, dict):
        raise PackError(f"{where}: {MANIFEST} is not a JSON object")
    if manifest.get("format") != PACK_FORMAT:
        raise PackError(f"{where}: pack format {manifest.get('format')!r}; this OpenRAC reads {PACK_FORMAT}")
    if (manifest.get("game"), manifest.get("version")) != (GAME, VERSION):
        raise PackError(f"{where} is for {manifest.get('game')}/{manifest.get('version')}, not "
                        f"Ratchet & Clank PAL ({GAME}/{VERSION})")
    name = manifest.get("name")
    if not isinstance(name, str) or not name.strip() or len(name) > 120:
        raise PackError(f"{where}: {MANIFEST} needs a \"name\" (up to 120 characters)")
    for key in ("author", "description"):
        if key in manifest and not isinstance(manifest[key], str):
            raise PackError(f"{where}: \"{key}\" in {MANIFEST} must be text")
    return manifest


class Source:
    """A pack's files, from a folder or a zip: relative path -> how to read it."""

    def __init__(self, path: Path):
        self.path = Path(path)
        self.zip = None
        if self.path.is_dir():
            root = self.path
            if not (root / MANIFEST).is_file():
                inner = [d for d in root.iterdir() if d.is_dir() and (d / MANIFEST).is_file()]
                if len(inner) != 1:
                    raise PackError(f"{self.path} has no {MANIFEST}: it is not an overrides pack")
                root = inner[0]
            self.root = root
            self.files = {}
            for file in sorted(root.rglob("*")):
                if file.is_symlink() or not file.is_file():
                    continue
                self.files[file.relative_to(root).as_posix()] = file
        elif self.path.is_file() and zipfile.is_zipfile(self.path):
            self.zip = zipfile.ZipFile(self.path)
            names = [i for i in self.zip.infolist() if not i.is_dir()]
            if len(names) > MAX_FILES:
                raise PackError(f"{self.path.name} holds {len(names)} files; too many for a pack")
            for info in names:
                name = info.filename
                if name.startswith(("/", "\\")) or "\\" in name or ".." in PurePosixPath(name).parts \
                        or re.match(r"^[A-Za-z]:", name):
                    raise PackError(f"{self.path.name} has a file outside its folder ({name}); not opened")
            if sum(i.file_size for i in names) > MAX_TOTAL:
                raise PackError(f"{self.path.name} unpacks to more than 4 GB; not opened")
            prefix = ""
            top = {n.filename for n in names}
            if MANIFEST not in top:
                found = [n for n in top if n.endswith("/" + MANIFEST) and n.count("/") == 1]
                if len(found) != 1:
                    raise PackError(f"{self.path.name} has no {MANIFEST} at its root: it is not an overrides pack")
                prefix = found[0][:-len(MANIFEST)]
            self.files = {i.filename[len(prefix):]: i for i in names if i.filename.startswith(prefix)}
        else:
            raise PackError(f"{self.path} is neither a folder nor a .zip file")

    def read(self, rel: str) -> bytes:
        item = self.files[rel]
        return self.zip.read(item) if self.zip is not None else item.read_bytes()

    def close(self) -> None:
        if self.zip is not None:
            self.zip.close()


def check(path: Path) -> tuple[dict, list[str], list[str], list[str]]:
    """A pack's manifest, the files it would use, the files it ignores, and its problems
    (a file that cannot be read, a model with joints the port cannot map). Raises PackError when
    it cannot be used at all."""
    import gltf_read
    import pngio
    source = Source(path)
    try:
        manifest = read_manifest(source.read(MANIFEST), source.path.name)
        used, ignored, problems = [], [], []
        for rel in sorted(source.files):
            if rel == MANIFEST:
                continue
            if not FILES.match(rel):
                if not PEOPLE.match(rel):
                    ignored.append(rel)
                continue
            data = source.read(rel)
            try:
                if rel.endswith(".png"):
                    width, height = pngio.size(data)
                    if not 0 < width <= 8192 or not 0 < height <= 8192:
                        raise ValueError(f"{width} x {height} is larger than 8192 on a side")
                elif rel.endswith(".glb"):
                    with tempfile.TemporaryDirectory() as scratch:
                        file = Path(scratch) / "model.glb"
                        file.write_bytes(data)
                        document = gltf_read.read(file)
                        for skin in document.list("skins"):
                            for j in skin["joints"]:
                                name = document.list("nodes")[j].get("name", "")
                                if not re.match(r"^joint_\d{3}(?:_post)?$", name):
                                    raise ValueError(f"a joint named {name!r}; the port numbers joints joint_NNN")
                elif rel.endswith(".json"):
                    json.loads(data.decode("utf-8"))
                elif rel == "patches.tsv":
                    for line in data.decode("utf-8").splitlines():
                        if line and not line.startswith("#") and len(line.split("\t")) != 3:
                            raise ValueError(f"the line {line!r} is not sector, count, file")
            except (ValueError, pngio.PngError, gltf_read.GltfError, UnicodeDecodeError, struct.error) as exc:
                problems.append(f"{rel}: {exc}")
                continue
            used.append(rel)
        if manifest.get("levels_format", LEVELS_FORMAT) != LEVELS_FORMAT and any(r.endswith(".glb") for r in used):
            problems.append(f"its models were made for levels of format {manifest.get('levels_format')}, and these "
                            f"are {LEVELS_FORMAT}: they may not fit; ask its author for a new version")
        if not used:
            raise PackError(f"{source.path.name} replaces nothing: it has no file laid out as the levels are "
                            "(level_NN/textures/*.png, level_NN/mobys/*.glb...)")
        return manifest, used, ignored, problems
    finally:
        source.close()


# ---- the overrides folder -----------------------------------------------------------------


def ensure(folder: Path) -> Path:
    folder = Path(folder)
    (folder / "packs").mkdir(parents=True, exist_ok=True)
    return folder


def load_state(folder: Path) -> dict:
    path = Path(folder) / STATE
    if not path.is_file():
        return {"format": 1, "packs": []}
    try:
        state = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as exc:
        raise PackError(f"{path} cannot be read ({exc})") from None
    if state.get("format") != 1 or not isinstance(state.get("packs"), list):
        raise PackError(f"{path} is not an overrides list this OpenRAC reads")
    # Packs whose folder went away drop out; folders not listed are not used.
    state["packs"] = [p for p in state["packs"] if isinstance(p, dict) and isinstance(p.get("id"), str)
                      and (Path(folder) / "packs" / p["id"] / MANIFEST).is_file()]
    return state


def save_state(folder: Path, state: dict) -> None:
    path = Path(folder) / STATE
    temporary = path.with_suffix(".json.tmp")
    temporary.write_text(json.dumps(state, indent=1) + "\n")
    os.replace(temporary, path)


def pack_id(name: str, taken) -> str:
    base = re.sub(r"[^a-z0-9]+", "-", name.lower()).strip("-")[:48] or "pack"
    candidate, n = base, 2
    while candidate in taken:
        candidate, n = f"{base}-{n}", n + 1
    return candidate


def install(staged: Path, folder: Path, replace_export: str | None = None) -> str:
    """Move a checked pack folder (STAGED) into FOLDER's packs and list it, enabled, last. A pack
    made from the same export (REPLACE_EXPORT) takes the place of the older one, keeping its
    place and state."""
    folder = ensure(folder)
    manifest = json.loads((staged / MANIFEST).read_text())
    state = load_state(folder)
    taken = {p.name for p in (folder / "packs").iterdir()}
    existing = None
    if replace_export:
        for entry in state["packs"]:
            try:
                other = json.loads((folder / "packs" / entry["id"] / MANIFEST).read_text())
            except (OSError, json.JSONDecodeError):
                continue
            if other.get("export") == replace_export:
                existing = entry["id"]
    if existing:
        old = folder / "packs" / f".{existing}-old"
        (folder / "packs" / existing).rename(old)
        staged.rename(folder / "packs" / existing)
        shutil.rmtree(old)
        save_state(folder, state)
        return existing
    new = pack_id(manifest["name"], taken)
    staged.rename(folder / "packs" / new)
    state["packs"].append({"id": new, "enabled": True})
    save_state(folder, state)
    return new


def add(source_path: Path, folder: Path) -> tuple[str, list[str], list[str]]:
    """Check a pack and copy it in. Returns its id, the files ignored and its problems."""
    manifest, used, ignored, problems = check(source_path)
    folder = ensure(folder)
    source = Source(source_path)
    try:
        with tempfile.TemporaryDirectory(prefix=".add-", dir=folder / "packs") as scratch:
            staged = Path(scratch) / "pack"
            for rel in [MANIFEST] + [r for r in source.files if PEOPLE.match(r)] + used:
                target = staged / rel
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(source.read(rel))
            new = install(staged, folder)
    finally:
        source.close()
    return new, ignored, problems


def move(state: dict, pack: str, where: str) -> None:
    ids = [p["id"] for p in state["packs"]]
    if pack not in ids:
        raise PackError(f"no pack {pack}")
    at = ids.index(pack)
    entry = state["packs"].pop(at)
    to = {"up": max(0, at - 1), "down": min(len(ids) - 1, at + 1), "first": 0, "last": len(ids) - 1}[where]
    state["packs"].insert(to, entry)


def remove(folder: Path, pack: str) -> None:
    state = load_state(folder)
    if pack not in [p["id"] for p in state["packs"]]:
        raise PackError(f"no pack {pack}")
    state["packs"] = [p for p in state["packs"] if p["id"] != pack]
    save_state(folder, state)
    shutil.rmtree(Path(folder) / "packs" / pack)


# ---- stacking ---------------------------------------------------------------------------------


def place(source: Path, target: Path) -> None:
    """A hard link where the file system allows one, else a copy."""
    target.parent.mkdir(parents=True, exist_ok=True)
    try:
        os.link(source, target)
    except OSError:
        shutil.copy2(source, target)


def layer_files(root: Path) -> dict[str, Path]:
    return {p.relative_to(root).as_posix(): p for p in sorted(root.rglob("*"))
            if p.is_file() and not p.is_symlink() and FILES.match(p.relative_to(root).as_posix())}


def stack(folder: Path | None, out: Path, packed: Path | None = None) -> dict:
    """Write MODS (out) from the packer's output and the enabled packs; returns what it holds."""
    out = Path(out).resolve()
    if out.exists() and not (out / MARKER).is_file():
        raise PackError(f"{out} exists and was not made by this tool; name another folder")
    layers = []
    if packed is not None and Path(packed).is_dir():
        layers.append(("your packed levels", "packed", Path(packed)))
    if folder is not None and Path(folder).is_dir():
        for entry in load_state(folder)["packs"]:
            if entry.get("enabled", True):
                root = Path(folder) / "packs" / entry["id"]
                name = json.loads((root / MANIFEST).read_text()).get("name", entry["id"])
                layers.append((name, entry["id"], root))
    out.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=f".{out.name}-", dir=out.parent))
    owners, patches, notes = {}, {}, []
    try:
        for name, ident, root in layers:
            for rel, path in layer_files(root).items():
                if rel == "patches.tsv":
                    for line in path.read_text().splitlines():
                        fields = line.split("\t")
                        if line.startswith("#") or len(fields) != 3:
                            continue
                        key = (int(fields[0]), int(fields[1]))
                        if key in patches and patches[key][1] != ident:
                            notes.append(f"{name} replaces the level file {patches[key][1]} put at sector {key[0]}")
                        patches[key] = (fields[2], ident)
                    continue
                if rel in owners and owners[rel] != ident:
                    notes.append(f"{rel}: {name} wins over {owners[rel]}")
                    (temporary / rel).unlink()
                place(path, temporary / rel)
                owners[rel] = ident
        (temporary / "patches.tsv").write_text(
            "# first sector\tsectors\tfile (beside this list)\n"
            + "".join(f"{first}\t{count}\t{file}\n" for (first, count), (file, _) in sorted(patches.items())))
        summary = {"format": 1, "layers": [{"id": ident, "name": name} for name, ident, _ in layers],
                   "files": dict(sorted(owners.items())), "notes": notes}
        (temporary / "stack.json").write_text(json.dumps(summary, indent=1) + "\n")
        (temporary / MARKER).write_text("made by editor/overrides.py stack; it is replaced each time\n")
        if out.exists():
            old = Path(tempfile.mkdtemp(prefix=f".{out.name}-old-", dir=out.parent))
            out.rename(old / out.name)
            temporary.rename(out)
            shutil.rmtree(old)
        else:
            temporary.rename(out)
    finally:
        if temporary.exists():
            shutil.rmtree(temporary)
    return summary


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("check").add_argument("source", type=Path)
    for name in ("add", "list", "enable", "disable", "move", "remove", "stack", "zip"):
        command = commands.add_parser(name)
        if name == "add":
            command.add_argument("source", type=Path)
        if name in ("enable", "disable", "move", "remove", "zip"):
            command.add_argument("id")
        if name == "move":
            command.add_argument("where", choices=("up", "down", "first", "last"))
        if name == "zip":
            command.add_argument("zip", type=Path)
        command.add_argument("--into", type=Path, required=True, help="the overrides folder")
        if name == "list":
            command.add_argument("--json", action="store_true")
        if name == "stack":
            command.add_argument("--out", type=Path, required=True, help="the folder the port's --mods reads")
            command.add_argument("--packed", type=Path, help="the level packer's output, beneath the packs")
    args = parser.parse_args(argv)
    try:
        if args.command == "check":
            manifest, used, ignored, problems = check(args.source)
            print(f"{manifest['name']}" + (f" by {manifest['author']}" if manifest.get("author") else ""))
            for level, what in describe(used).items():
                print(f"  {level}: " + ", ".join(f"{len(v)} {k}" for k, v in what.items()))
            for rel in ignored:
                print(f"  ignored: {rel} (not a file the game reads)")
            for problem in problems:
                print(f"  problem: {problem}")
            return 1 if problems else 0
        if args.command == "add":
            new, ignored, problems = add(args.source, args.into)
            for rel in ignored:
                print(f"ignored: {rel} (not a file the game reads)")
            for problem in problems:
                print(f"left out: {problem}")
            print(f"Added {args.source.name} as {new}, on, last in the order.")
            return 0
        if args.command == "stack":
            summary = stack(args.into, args.out, args.packed)
            for note in summary["notes"]:
                print(f"note: {note}")
            print(f"Stacked {len(summary['layers'])} layer(s), {len(summary['files'])} file(s): {args.out.resolve()}")
            return 0
        state = load_state(args.into)
        if args.command == "list":
            if args.json:
                print(json.dumps(state, indent=1))
            for i, entry in enumerate(state["packs"], 1):
                manifest = json.loads((args.into / "packs" / entry["id"] / MANIFEST).read_text())
                print(f"{i}. {entry['id']}: {manifest['name']} ({'on' if entry.get('enabled', True) else 'off'})")
            return 0
        if args.command == "remove":
            remove(args.into, args.id)
            print(f"Removed {args.id}.")
            return 0
        if args.command == "zip":
            root = args.into / "packs" / args.id
            if not (root / MANIFEST).is_file():
                raise PackError(f"no pack {args.id}")
            if args.zip.exists():
                raise PackError(f"{args.zip} already exists")
            with zipfile.ZipFile(args.zip, "w", zipfile.ZIP_DEFLATED) as z:
                for path in sorted(root.rglob("*")):
                    if path.is_file():
                        z.write(path, path.relative_to(root).as_posix())
            print(f"Wrote {args.zip}: share it only with people who own the game.")
            return 0
        if args.id not in [p["id"] for p in state["packs"]]:
            raise PackError(f"no pack {args.id}")
        if args.command == "move":
            move(state, args.id, args.where)
        else:
            for entry in state["packs"]:
                if entry["id"] == args.id:
                    entry["enabled"] = args.command == "enable"
        save_state(args.into, state)
        return 0
    except (PackError, OSError, zipfile.BadZipFile) as exc:
        print(f"overrides: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
