#!/usr/bin/env python3
"""Overrides packs: replacement textures, models and level edits the native
port loads on top of the extracted levels, without reading the disc again.

  overrides.py check SOURCE                      what a pack (.zip or folder) holds, and what is wrong with it
  overrides.py add SOURCE --into OVERRIDES       copy a pack in (the source is never changed)
  overrides.py list --into OVERRIDES [--json]    the packs, in the order they apply
  overrides.py enable|disable ID --into OVERRIDES
  overrides.py move ID up|down|first|last --into OVERRIDES
  overrides.py remove ID --into OVERRIDES        delete the copy `add` made
  overrides.py stack --into OVERRIDES --out MODS --levels LEVELS [--packed PACKED]
                                                 the folder the port's --mods reads, for the
                                                 levels it plays (either disc's)
  overrides.py zip ID OUT.zip --into OVERRIDES   a pack as a .zip to share

A pack is a folder, or a .zip of one, with openrac-mod.json at its root (or
in the zip's one top folder) and files laid out as the extracted levels are:

  openrac-mod.json                  {"format": 1, "game": "rac1" (or "rac2", "rac3", "rac4"),
                                     "version": "pal" or "ntsc" (the others: "ntsc"),
                                     "name": ..., "author": ..., "description": ...,
                                     "levels_format": 3, "replaces": {...}, "originals": {...}}
  level_NN/textures/<name>.png      a texture, any size of the same shape
  level_NN/mobys/moby_<class>.glb   a class's model, as the port reads it
  level_NN/ties/tie_<class>.glb, level_NN/shrubs/shrub_<class>.glb
  level_NN/terrain.glb              the level's terrain, as the port reads it (any vertex count)
  level_NN/placements.json         ties and shrubs placed again (the level packer's)
  level_NN/gameplay_<version>.wad, patches.tsv   a packed level (the level packer's)
  README*, LICENSE*, *.txt, *.md    read by people only

The files are named as the levels of the disc the pack was made from
("version") name them. "originals" says what each texture and model
replaces, so that a pack applies to the other disc's levels too, where
the same textures have other names: for a texture, {"sha256": the
original picture's PNG hash} (or {"new": true} for a model's own
picture); for a model, {"kind", "class", "vertices": a hash of the
original's vertex positions, "textures": {name: hash of each level
texture it draws with}}; for the terrain, the same with "kind": "terrain".

Going Commando's, Up Your Arsenal's and Deadlocked's packs (game rac2, rac3, rac4; early access)
hold textures and models only: their levels are not packed back yet, so a level edit in one is
left out. A pack is stacked only for levels of its own game.

An OVERRIDES folder holds the packs that were added (packs/<id>/) and
overrides.json, their order and which are on: {"format": 1, "packs":
[{"id": ..., "enabled": true}, ...]}; a later pack wins over an earlier one.
The launcher reads and changes the same file (launcher/core/src/overrides.rs).

`stack` puts the level packer's output (PACKED) and every enabled pack, in
order, into MODS: the folder `--mods` names. A pack made from the levels of
the disc LEVELS were read from goes in as it is. One made from the other
disc's goes where it matches (cross_disc): each texture wherever LEVELS have
the same original picture, each model where the level has its class with
the same skeleton (or, without one, the same vertices), the terrain where
the level's has the same vertices (both discs' levels have the same
terrain), their textures named as LEVELS name them; what belongs to one disc's data (a packed gameplay
file, patches.tsv, placements.json) or has no match is left out and listed. The port draws each level's
files from MODS/level_NN/ where they are there, and from the extracted
levels otherwise (port/viewer/level.cpp), and reads the disc sectors
patches.tsv lists from the files beside it (port/game/common/lib/cdvd.c).
"""

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import struct
import sys
import tempfile
import zipfile

import disc
import games
from port import FORMAT

MANIFEST = "openrac-mod.json"
STATE = "overrides.json"
MARKER = ".openrac-stack"
PACK_FORMAT = 1
GAME = "rac1"  # the game of a pack whose manifest does not say
VERSIONS = tuple(v.key for v in disc.VERSIONS)  # Ratchet & Clank's discs: pal, ntsc (games.VERSIONS for every game)
LEVELS_FORMAT = FORMAT
# The oldest levels format whose models a pack's models are drawn with: 2 gave the mobys their
# metal packets and chrome and glass maps; 3 only added classes (port.py FORMAT).
MODELS_SINCE = 2
FILES = re.compile(r"^(?:patches\.tsv|level_\d{2}/(?:textures/[A-Za-z0-9_.-]+\.png|(?:mobys|ties|shrubs)/"
                   r"[A-Za-z0-9_.-]+\.glb|terrain\.glb|placements\.json|gameplay_(?:" + "|".join(VERSIONS) + r")\.wad"
                   r"|(?:core|index)_(?:" + "|".join(VERSIONS) + r")\.bin))$")
TEXTURE = re.compile(r"^level_(\d{2})/textures/([A-Za-z0-9_.-]+)\.png$")
MODEL = re.compile(r"^level_(\d{2})/(moby|tie|shrub)s/\2_(\d+)\.glb$")
TERRAIN = re.compile(r"^level_(\d{2})/terrain\.glb$")
IMAGE = re.compile(r"((?:\.\./)?textures/)([A-Za-z0-9_.-]+)\.png")  # a port GLB's image: a level texture
PACKED = "packed.json"  # the level packer's note of the disc it packed (pack.py)
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
        elif parts[1] == "terrain.glb":
            level.setdefault("models", []).append("terrain")
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
    if not games.known(manifest.get("game"), manifest.get("version", "pal" if manifest.get("game") == GAME else None)):
        raise PackError(f"{where} is not a pack for a game OpenRAC reads ({manifest.get('game')}/"
                        f"{manifest.get('version')})")
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
            if manifest["game"] != GAME and not (TEXTURE.match(rel) or MODEL.match(rel) or TERRAIN.match(rel)):
                # Early access: the later games' levels are not packed back, so only textures and
                # models of theirs are used.
                ignored.append(rel)
                problems.append(f"{rel}: a level edit; {games.title(manifest['game'])}'s levels are not packed "
                                "back yet, so only its textures and models are used")
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
                        if TERRAIN.match(rel):
                            if document.list("skins"):
                                raise ValueError("a skinned mesh; the terrain has no bones")
                            if not any(p.get("mode", 4) == 4 for node in document.list("nodes") if "mesh" in node
                                       for p in document.list("meshes")[node["mesh"]]["primitives"]):
                                raise ValueError("no triangles")
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
        made = manifest.get("levels_format", LEVELS_FORMAT)
        if (not isinstance(made, int) or made < MODELS_SINCE) and any(r.endswith(".glb") for r in used):
            problems.append(f"its models were made for levels of format {made}, and these are {LEVELS_FORMAT}: "
                            "they may not fit; ask its author for a new version")
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


# ---- the other disc --------------------------------------------------------------------------


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def glb_parts(data: bytes) -> tuple[dict, bytes]:
    """A GLB's JSON and binary chunk."""
    import gltf_read
    if data[:4] != struct.pack("<I", gltf_read.GLB_MAGIC):
        raise PackError("not a .glb file")
    at, doc, binary = 12, None, b""
    while at + 8 <= len(data):
        size, kind = struct.unpack_from("<2I", data, at)
        if kind == gltf_read.JSON_CHUNK:
            doc = json.loads(data[at + 8:at + 8 + size].decode("utf-8"))
        elif kind == gltf_read.BIN_CHUNK:
            binary = data[at + 8:at + 8 + size]
        at += 8 + size + (-size % 4)
    if doc is None:
        raise PackError("a .glb file without its JSON")
    return doc, binary


def glb_join(doc: dict, binary: bytes) -> bytes:
    text = json.dumps(doc, separators=(",", ":"), allow_nan=False).encode()
    text += b" " * (-len(text) % 4)
    binary += bytes(-len(binary) % 4)
    return (struct.pack("<3I", 0x46546C67, 2, 28 + len(text) + len(binary))
            + struct.pack("<I4s", len(text), b"JSON") + text + struct.pack("<I4s", len(binary), b"BIN\0") + binary)


def model_textures(doc: dict) -> list[str]:
    """The level textures a port GLB draws with: its images' ../textures/<name>.png (a class model's)
    or textures/<name>.png (the terrain's)."""
    names = []
    for image in doc.get("images", []):
        found = IMAGE.fullmatch(image.get("uri", ""))
        if found and found.group(2) not in names:
            names.append(found.group(2))
    return names


def vertices(path: Path) -> str:
    """A hash of a model's vertex positions, in the order the file has them."""
    import gltf_read
    document = gltf_read.read(path)
    digest = hashlib.sha256()
    for node in document.list("nodes"):
        if "mesh" in node:
            for primitive in document.list("meshes")[node["mesh"]]["primitives"]:
                for position in document.accessor(primitive["attributes"]["POSITION"]):
                    digest.update(struct.pack("<3f", *position))
    return digest.hexdigest()


def skeleton(path: Path) -> dict[int, tuple]:
    """A model's joints by game number (nodes named joint_NNN) -> inverse bind matrix."""
    import gltf_read
    document = gltf_read.read(path)
    if not document.list("skins"):
        return {}
    skin = document.list("skins")[0]
    matrices = document.accessor(skin["inverseBindMatrices"]) if "inverseBindMatrices" in skin else []
    out = {}
    for slot, j in enumerate(skin["joints"]):
        found = re.fullmatch(r"joint_(\d{3})", document.list("nodes")[j].get("name", ""))
        if found and slot < len(matrices):
            out.setdefault(int(found.group(1)), matrices[slot])
    return out


def same_skeleton(pack: dict, level: dict) -> bool:
    """A pack's model fits a level's class: the level's joints, each with its bind pose."""
    return bool(level) and all(n in pack and all(abs(a - b) < 1e-4 for a, b in zip(pack[n], m))
                               for n, m in level.items())


def class_textures(levels: Path, level: int) -> dict[tuple[str, int], list[str]]:
    """Each class model of a level, and its terrain (("terrain", 0)) -> the level textures it draws
    with, in its file's order."""
    manifest = read_json(levels / f"level_{level:02}" / "manifest.json")
    out = {}
    terrain = manifest.get("terrain")
    if isinstance(terrain, str) and (levels / f"level_{level:02}" / terrain).is_file():
        out[("terrain", 0)] = model_textures(glb_parts((levels / f"level_{level:02}" / terrain).read_bytes())[0])
    for kind, classes in manifest.get("classes", {}).items():
        for number, entry in classes.items():
            mesh = entry.get("mesh") if isinstance(entry, dict) else None
            if mesh and (levels / f"level_{level:02}" / mesh).is_file():
                out[(kind, int(number))] = model_textures(glb_parts((levels / f"level_{level:02}" / mesh)
                                                                    .read_bytes())[0])
    return out


def originals(files, levels: Path) -> dict:
    """What each texture and model among FILES (pack paths -> bytes) replaces in LEVELS, the levels
    they were made from: the "originals" a pack records so that it applies to the other disc too.

    The two discs differ a little in some pictures (Ratchet's own skin among them), so a texture also
    records where the level's models use it ("used_by": kind, class, place in the model's list of
    textures, the list's length) and its size, and a model the textures its class has: on the other
    disc the same class's model names its version of each picture in the same place."""
    out, used = {}, {}
    for rel in sorted(files):
        texture, model, terrain = TEXTURE.match(rel), MODEL.match(rel), TERRAIN.match(rel)
        if terrain:
            # The terrain: matched on the other disc by its vertices (the two discs' are the same).
            level = levels / f"level_{terrain.group(1)}"
            original = level / (read_json(level / "manifest.json").get("terrain") or "terrain.glb")
            if not original.is_file():
                continue
            doc, _ = glb_parts(files[rel])
            names = {name: sha256((level / "textures" / f"{name}.png").read_bytes()) for name in model_textures(doc)
                     if (level / "textures" / f"{name}.png").is_file()}
            out[rel] = {"kind": "terrain", "class": None, "vertices": vertices(original), "textures": names,
                        "class_textures": model_textures(glb_parts(original.read_bytes())[0])}
        elif texture:
            level = int(texture.group(1))
            original = levels / f"level_{level:02}" / "textures" / f"{texture.group(2)}.png"
            if not original.is_file():
                out[rel] = {"new": True}
                continue
            import pngio
            data = original.read_bytes()
            if level not in used:
                used[level] = class_textures(levels, level)
            out[rel] = {"sha256": sha256(data), "size": list(pngio.size(data)),
                        "used_by": [[kind, number, names.index(texture.group(2)), len(names)]
                                    for (kind, number), names in sorted(used[level].items())
                                    if texture.group(2) in names]}
        elif model:
            level = levels / f"level_{model.group(1)}"
            original = level / rel.split("/", 1)[1]
            if not original.is_file():
                continue
            doc, _ = glb_parts(files[rel])
            names = {}
            for name in model_textures(doc):
                picture = level / "textures" / f"{name}.png"
                if picture.is_file():         # a level texture (the model's own pictures are not)
                    names[name] = sha256(picture.read_bytes())
            out[rel] = {"kind": model.group(2), "class": int(model.group(3)), "vertices": vertices(original),
                        "textures": names, "class_textures": model_textures(glb_parts(original.read_bytes())[0])}
    return out


class Target:
    """The levels a stack is for: their disc, and each level's textures by picture."""

    def __init__(self, levels: Path):
        import assets
        self.index = assets.LevelsIndex(Path(levels))
        if not self.index.manifests:
            raise PackError(f"{levels} holds no levels this OpenRAC reads: get the levels ready first")
        self.levels = self.index.levels
        self.version = self.index.version
        self.game = self.index.game or GAME
        self.names = {}                       # (level, name) -> picture hash
        for picture, places in self.index.by_hash().items():
            for place in places:
                self.names[place] = picture

    def places(self, picture: str) -> list[tuple[int, str]]:
        return self.index.by_hash().get(picture, [])

    def in_level(self, level: int, picture: str) -> str | None:
        return next((n for lv, n in self.places(picture) if lv == level), None)

    def model(self, level: int, kind: str, class_id: int | None) -> str | None:
        """A class model's file in a level (kind "terrain": the level's terrain), or None."""
        if kind == "terrain":
            terrain = self.index.manifests.get(level, {}).get("terrain")
            return terrain if isinstance(terrain, str) else None
        entry = self.index.manifests.get(level, {}).get("classes", {}).get(kind, {}).get(str(class_id))
        return entry.get("mesh") if entry else None

    def textures_of(self, level: int, kind: str, class_id: int) -> list[str]:
        """The level textures a class model here draws with, in its file's order."""
        mesh = self.model(level, kind, class_id)
        path = self.levels / f"level_{level:02}" / mesh if mesh else None
        return model_textures(glb_parts(path.read_bytes())[0]) if path is not None and path.is_file() else []

    def size(self, level: int, name: str) -> list[int] | None:
        import pngio
        path = self.levels / f"level_{level:02}" / "textures" / f"{name}.png"
        return list(pngio.size(path.read_bytes())) if path.is_file() else None

    def counterpart(self, level: int, entry: dict) -> str | None:
        """The texture here that takes the place of a picture the other disc has a version of its
        own of: the one its class models name where the original's name it (one answer only), of
        the original's size."""
        found = set()
        for kind, number, at, count in entry.get("used_by", []):
            names = self.textures_of(level, kind, number)
            if len(names) == count:
                found.add(names[at])
        if len(found) != 1:
            return None
        name = found.pop()
        return name if self.size(level, name) == entry.get("size") else None


def cross_disc(root: Path, manifest: dict, target: Target) -> tuple[dict, list[str]]:
    """A pack made from the other disc's levels, for TARGET: target path -> a file or bytes, and what
    was left out, each with why."""
    files = layer_files(root)
    recorded = manifest.get("originals") if isinstance(manifest.get("originals"), dict) else {}
    out, skipped = {}, []
    # Textures: wherever the target has the original picture.
    edited = {}                               # original picture -> {level: replacement}
    unknown = {}                              # replacement's hash -> [(level, name, file)]: no original recorded
    for rel, path in files.items():
        texture = TEXTURE.match(rel)
        if not texture:
            continue
        entry = recorded.get(rel, {})
        if entry.get("new") or "_ovr_" in texture.group(2):
            continue                          # a model's own picture: it goes with the model
        if isinstance(entry.get("sha256"), str):
            edited.setdefault(entry["sha256"], {})[int(texture.group(1))] = path
        else:
            unknown.setdefault(sha256(path.read_bytes()), []).append((int(texture.group(1)), texture.group(2), path))
    for picture, by_level in edited.items():
        places = target.places(picture)
        for lv, name in places:
            out[f"level_{lv:02}/textures/{name}.png"] = by_level.get(lv) or next(iter(by_level.values()))
        if places:
            continue
        # This disc's version of the picture differs: where the same class models use it instead.
        for lv, path in sorted(by_level.items()):
            name = target.counterpart(lv, recorded.get(f"level_{lv:02}/textures/{path.stem}.png", {}))
            if name is not None:
                out[f"level_{lv:02}/textures/{name}.png"] = path
            else:
                skipped.append(f"level_{lv:02}/textures/{path.stem}.png (no level here has that picture)")
    for group in unknown.values():
        # A pack from before originals were recorded: the names it replaces must be one picture here.
        pictures = {target.names.get((lv, name)) for lv, name, _ in group}
        if len(pictures) == 1 and None not in pictures:
            edited.setdefault(pictures.pop(), {}).update({lv: path for lv, _, path in group})
            for lv, name in target.places(target.names[(group[0][0], group[0][1])]):
                out[f"level_{lv:02}/textures/{name}.png"] = group[0][2]
        else:
            skipped += [f"level_{lv:02}/textures/{name}.png (not the same picture here)" for lv, name, _ in group]
    # Models: by class, where the level has it with the same skeleton; the terrain where the level's
    # has the same vertices.
    for rel, path in files.items():
        model, terrain = MODEL.match(rel), TERRAIN.match(rel)
        if model:
            level, kind, class_id = int(model.group(1)), model.group(2), int(model.group(3))
        elif terrain:
            level, kind, class_id = int(terrain.group(1)), "terrain", None
        else:
            continue
        mesh = target.model(level, kind, class_id)
        if not mesh:
            skipped.append(f"{rel} (the level has no " + ("terrain" if terrain else "such class") + " here)")
            continue
        entry = recorded.get(rel, {})
        theirs = skeleton(target.levels / f"level_{level:02}" / mesh)
        if theirs:
            fits = same_skeleton(skeleton(path), theirs)
        else:
            fits = entry.get("vertices") == vertices(target.levels / f"level_{level:02}" / mesh)
        if not fits:
            skipped.append(f"{rel} (" + ("the level's terrain is another" if terrain else "the class is another model")
                           + " here)")
            continue
        doc, binary = glb_parts(path.read_bytes())
        names, missing = {}, []
        for name in model_textures(doc):
            own = files.get(f"level_{level:02}/textures/{name}.png")
            if name.startswith("metal_"):
                names[name] = name            # each level's own chrome and glass maps
            elif own is not None and (recorded.get(own.relative_to(root).as_posix(), {}).get("new")
                                      or "_ovr_" in name):
                out[f"level_{level:02}/textures/{name}.png"] = own
                names[name] = name
            else:
                picture = entry.get("textures", {}).get(name) \
                    or recorded.get(f"level_{level:02}/textures/{name}.png", {}).get("sha256")
                if picture is None and not recorded and target.names.get((level, name)):
                    names[name] = name        # a pack from before originals: by name
                    continue
                here = target.in_level(level, picture) if picture else None
                theirs = target.textures_of(level, kind, class_id)
                ours = entry.get("class_textures", [])
                if here is None and name in ours and len(ours) == len(theirs):
                    here = theirs[ours.index(name)]   # this disc's version of the class's picture
                if here is not None:
                    names[name] = here
                elif picture in edited:       # not in this level here, but the pack has its picture
                    data = next(iter(edited[picture].values())).read_bytes()
                    names[name] = f"{kind}_ovr_{sha256(data)[:16]}"
                    out[f"level_{level:02}/textures/{names[name]}.png"] = data
                else:
                    missing.append(name)
        if missing:
            skipped.append(f"{rel} (draws with {', '.join(missing)}, a picture this level does not have here)")
            continue
        if any(old != new for old, new in names.items()):
            for image in doc.get("images", []):
                found = IMAGE.fullmatch(image.get("uri", ""))
                if found and found.group(2) in names:
                    image["uri"] = f"{found.group(1)}{names[found.group(2)]}.png"
            for material in doc.get("materials", []):
                if material.get("name") in names:
                    material["name"] = names[material["name"]]
            out[f"level_{level:02}/{mesh}"] = glb_join(doc, binary)
        else:
            out[f"level_{level:02}/{mesh}"] = path
    for rel in files:
        if not TEXTURE.match(rel) and not MODEL.match(rel) and not TERRAIN.match(rel):
            skipped.append(f"{rel} (it belongs to the other disc's level data)")
    return out, skipped


def stack(folder: Path | None, out: Path, packed: Path | None = None, levels: Path | None = None) -> dict:
    """Write MODS (out) from the packer's output and the enabled packs, for LEVELS (the levels the
    game will play; without them every pack goes in as it is); returns what it holds."""
    out = Path(out).resolve()
    if out.exists() and not (out / MARKER).is_file():
        raise PackError(f"{out} exists and was not made by this tool; name another folder")
    target = Target(levels) if levels is not None else None
    if (out.parent / MARKER).is_file():
        # A stack from before each disc had a folder of its own (<data>/mods): made again here.
        shutil.rmtree(out.parent)
    layers = []
    if packed is not None and Path(packed).is_dir():
        made = read_json(Path(packed) / PACKED).get("version", "pal") if (Path(packed) / PACKED).is_file() else "pal"
        layers.append(("your packed levels", "packed", Path(packed), made))
    if folder is not None and Path(folder).is_dir():
        for entry in load_state(folder)["packs"]:
            if entry.get("enabled", True):
                root = Path(folder) / "packs" / entry["id"]
                manifest = read_json(root / MANIFEST)
                layers.append((manifest.get("name", entry["id"]), entry["id"], root, manifest))
    out.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=f".{out.name}-", dir=out.parent))
    owners, patches, notes, skipped = {}, {}, [], {}
    try:
        for name, ident, root, manifest in layers:
            made = manifest if isinstance(manifest, str) else manifest.get("version", "pal")
            game = GAME if isinstance(manifest, str) else manifest.get("game", GAME)
            if target is not None and game != target.game:
                # Another game's textures and models never fit these levels.
                skipped[ident] = sorted(layer_files(root))
                notes.append(f"{name} is a pack for {games.title(game)}, not for {games.title(target.game)}: left out")
                continue
            if target is None or made == target.version:
                files = layer_files(root)
            elif ident == "packed":
                skipped[ident] = sorted(layer_files(root))
                notes.append("your packed levels were packed from the other disc's levels, so they are left "
                             "out: pack them again")
                continue
            else:
                files, skipped[ident] = cross_disc(root, manifest, target)
                if skipped[ident]:
                    shown = skipped[ident][:6]
                    notes.append(f"{name} was made from the other disc's levels; left out, with no match here: "
                                 + "; ".join(shown) + (f"; and {len(skipped[ident]) - 6} more" if
                                                       len(skipped[ident]) > 6 else ""))
            for rel, source in files.items():
                if rel == "patches.tsv":
                    for line in source.read_text().splitlines():
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
                if (temporary / rel).exists():
                    (temporary / rel).unlink()
                if isinstance(source, bytes):
                    (temporary / rel).parent.mkdir(parents=True, exist_ok=True)
                    (temporary / rel).write_bytes(source)
                else:
                    place(source, temporary / rel)
                owners[rel] = ident
        (temporary / "patches.tsv").write_text(
            "# first sector\tsectors\tfile (beside this list)\n"
            + "".join(f"{first}\t{count}\t{file}\n" for (first, count), (file, _) in sorted(patches.items())))
        summary = {"format": 1, "version": target.version if target else None,
                   "layers": [{"id": ident, "name": name} for name, ident, _, _ in layers],
                   "files": dict(sorted(owners.items())), "notes": notes,
                   "skipped": {k: v for k, v in skipped.items() if v}}
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


def read_json(path: Path) -> dict:
    try:
        value = json.loads(Path(path).read_text())
    except (OSError, json.JSONDecodeError) as exc:
        raise PackError(f"{path} cannot be read ({exc})") from None
    return value if isinstance(value, dict) else {}


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
            command.add_argument("--levels", type=Path, required=True,
                                 help="the levels the game plays (either disc's): packs made from the other "
                                      "disc's go where they match")
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
            summary = stack(args.into, args.out, args.packed, args.levels)
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
