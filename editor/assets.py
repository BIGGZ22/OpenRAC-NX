#!/usr/bin/env python3
"""Export a level's models and textures as files a modder opens in Blender,
and read edited ones back as an overrides pack the native port loads.

  assets.py export LEVELS OUT --level N [--level N ...] [--what mobys|textures|scenery|all] [--class C ...]
  assets.py import EDITED --levels LEVELS (--out PACK | --into OVERRIDES) [--name NAME] [--author WHO]

LEVELS is the levels folder the port draws from (`editor/extract.py port`:
assets/port/rac1/pal, or <data>/port/pal for an installed launcher). Exporting reads
it and never writes there; importing reads the edited folder and writes a
new pack (overrides.py), never the levels themselves.

What an export holds (OUT, one folder per level):

  export.json                   what was exported, from which levels
  README.txt                    how to edit it and bring it back
  level_NN/textures/*.png       the level's textures (all of them with --what textures)
  level_NN/textures/textures.json   each texture's size and hash as exported
  level_NN/mobys/moby_<class>_<name>/   one folder per moby class (Ratchet is class 0)
      moby_<class>.gltf, .bin   mesh, UVs, vertex colours, materials naming
                                ../../textures/*.png, skin and animations
      asset.json                the asset's manifest: what it is, where it came from
  level_NN/ties/tie_<class>/, level_NN/shrubs/shrub_<class>/   the same, without skins

The glTF files are in glTF's own axes (Y up), so Blender shows a model
standing; moby classes are at the size the game draws them (the class
scale applied). The import turns both back.

The import finds what changed against the manifests and checks it:

- A texture may be any size (an upscale) with the shape the game's has; one
  of another shape is refused with the sizes that would fit. An edited
  texture replaces the same picture in every level that has it.
- A model is re-read against its original skeleton: vertices are skinned
  to the joints by name (joint_NNN), at most four per vertex (the strongest
  kept, weights normalised); unknown joints, vertices without weights and
  meshes without UVs are refused with what to do. Animations stay the
  game's (the port animates from the game's own data). A moby model
  replaces its class in every level whose class has the same skeleton,
  unless its asset.json says "levels": [N, ...].

Nothing here is ever committed (docs/policy/SOURCING.md): exports and packs
hold the game's data.
"""

import argparse
from dataclasses import dataclass, field
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile

import gltf_read
from gltf import Gltf
from gltf_read import GltfError, invert, multiply, point, vector
from mesh import Mesh
import pngio
from pngio import PngError
from port import FORMAT as LEVELS_FORMAT  # the levels these assets come from

ASSET_FORMAT = 1
GAME, VERSION = "rac1", "pal"
ROOT = Path(__file__).resolve().parents[1]
WHAT = ("mobys", "textures", "scenery", "all")
FAMILIES = {"mobys": ("moby",), "scenery": ("tie", "shrub"), "textures": (), "all": ("moby", "tie", "shrub")}
# Game axes (Z up) to glTF's (Y up): (x, y, z) -> (x, z, -y); column-major.
TO_GLTF = [1.0, 0, 0, 0, 0, 0, -1.0, 0, 0, 1.0, 0, 0, 0, 0, 0, 1.0]
HALF = math.sqrt(0.5)
Q_TO_GLTF = (-HALF, 0.0, 0.0, HALF)  # the same turn as a quaternion: -90 degrees about X
JOINT = re.compile(r"^joint_(\d{3})(?:_post)?$")
MAX_SIDE = 8192
README = """\
Ratchet & Clank (PAL) assets, exported by OpenRAC for modding.

Never share these files: they are your disc's data. Share the overrides
pack you make from them only with people who own the game (the pack is
what the import makes; it holds only what you changed).

Editing
- Textures: level_NN/textures/*.png. Edit or upscale them in any image
  editor and save as PNG under the same name. Keep each picture's shape
  (width : height): 128 x 64 may become 512 x 256, not 512 x 512.
- Models: open a level_NN/<kind>/<asset>/*.gltf in Blender (File > Import >
  glTF 2.0). Edit the mesh, its UVs and weights; keep the armature and its
  bone names (joint_000...). Export with File > Export > glTF 2.0, format
  "glTF Separate" or "glTF Binary", into the same folder (replace the file
  or add a new one beside it). Leave "+Y Up" on.
  The bones' animations are the game's own and come back unchanged.

Bringing your edits back
- In the launcher: Mods > Models and textures > Import your edits, and
  choose this folder.
- Or: python3 editor/assets.py import <this folder> --levels <levels> --out <pack>

asset.json and textures.json record what was exported; leave them as they are.
"""


class AssetError(ValueError):
    """Something the export or import refuses, said so a modder can act on it."""


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def file_hash(path: Path) -> str:
    return sha256(path.read_bytes())


def slug(text: str) -> str:
    return re.sub(r"[^a-z0-9]+", "_", text.lower()).strip("_")[:40]


def output_dir(path: Path, replace: bool = False) -> Path:
    """A new folder (REPLACE: or one this tool wrote before), outside the repository's tracked
    files: under assets/ or build/ when it is inside the checkout."""
    path = path.resolve()
    inside = ROOT in path.parents or path == ROOT
    if inside and not any(root in path.parents for root in (ROOT / "assets", ROOT / "build")):
        raise AssetError(f"{path}: inside this repository, write only under assets/ or build/ "
                         "(game data must never be committed)")
    if path.exists() and not replace:
        raise AssetError(f"{path} already exists; choose a new folder")
    return path


def read_json(path: Path) -> dict:
    try:
        return json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as exc:
        raise AssetError(f"cannot read {path}: {exc}") from None


def level_manifest(levels: Path, level: int) -> dict:
    path = levels / f"level_{level:02}" / "manifest.json"
    if not path.is_file():
        raise AssetError(f"{levels} has no level {level:02}: get the levels ready first "
                         "(editor/extract.py port, or the launcher's \"Get the levels ready\")")
    manifest = read_json(path)
    if manifest.get("format") != LEVELS_FORMAT:
        raise AssetError(f"level {level:02} in {levels} was extracted by another version (format "
                         f"{manifest.get('format')}, this tool reads {LEVELS_FORMAT}): get the levels ready again")
    return manifest


# ---- encoding accessors -------------------------------------------------------------------

CODES = {5120: "b", 5121: "B", 5122: "h", 5123: "H", 5125: "I", 5126: "f"}


def encode(rows: list[tuple], accessor: dict) -> bytes:
    code = CODES[accessor["componentType"]]
    if accessor.get("normalized") and code != "f":
        scale = gltf_read.SCALE[accessor["componentType"]]
        low = -scale if code in "bh" else 0
        flat = [int(max(low, min(scale, round(v * scale)))) for row in rows for v in row]
    else:
        flat = [v for row in rows for v in row]
    return struct.pack(f"<{len(flat)}{code}", *flat)


def quat_multiply(a, b) -> tuple:
    ax, ay, az, aw = a
    bx, by, bz, bw = b
    return (aw * bx + ax * bw + ay * bz - az * by, aw * by - ax * bz + ay * bw + az * bx,
            aw * bz + ax * by - ay * bx + az * bw, aw * bw - ax * bx - ay * by - az * bz)


# ---- export -----------------------------------------------------------------------------------


def to_gltf_axes(document: gltf_read.Document, scale: float) -> tuple[dict, bytes]:
    """A port GLB (game axes, model units) as a glTF in glTF's axes at `scale`: positions and
    normals turned, inverse bind matrices, root nodes and their animation keys with them.
    Returns the new JSON and its one buffer."""
    doc = json.loads(json.dumps(document.doc))
    turn = [v * (scale if i % 4 < 3 and i < 12 else 1) for i, v in enumerate(TO_GLTF)]
    unturn = invert(turn)
    roots = set(doc.get("scenes", [{}])[doc.get("scene", 0)].get("nodes", []))
    joints = {j for skin in doc.get("skins", []) for j in skin["joints"]}
    transforms = {}

    def claim(index, how):
        if transforms.get(index, how) != how:
            raise AssetError(f"{document.path.name}: accessor {index} is shared in a way this export cannot turn")
        transforms[index] = how

    for mesh in doc.get("meshes", []):
        for primitive in mesh["primitives"]:
            attributes = primitive["attributes"]
            if "POSITION" in attributes:
                claim(attributes["POSITION"], "point")
            if "NORMAL" in attributes:
                claim(attributes["NORMAL"], "vector")
    for skin in doc.get("skins", []):
        if "inverseBindMatrices" in skin:
            claim(skin["inverseBindMatrices"], "inverse_bind")
    for animation in doc.get("animations", []):
        for channel in animation["channels"]:
            node, path = channel["target"].get("node"), channel["target"]["path"]
            if node in roots and node in joints:
                claim(animation["samplers"][channel["sampler"]]["output"], path)
    for index in roots:
        node, source = doc["nodes"][index], document.doc["nodes"][index]
        if index not in joints and "skin" in node:
            continue                                   # a skinned mesh's own node does not count
        for key in ("matrix", "translation", "rotation", "scale"):
            node.pop(key, None)
        if index in joints and "matrix" not in source:
            # A skeleton's root turns (its children follow): T(t) R S becomes T(Ct) (q_C q) (s S).
            t, r, s = node_trs(source)
            node.update(translation=list(point(turn, t)), rotation=list(quat_multiply(Q_TO_GLTF, r)),
                        scale=[v * scale for v in s])
        elif index in joints:
            node["matrix"] = multiply(turn, gltf_read.local_matrix(source))
        else:
            local = multiply(multiply(turn, gltf_read.local_matrix(source)), unturn)  # turned with its vertices
            if any(abs(x - y) > 1e-9 for x, y in zip(local, gltf_read.IDENTITY)):
                node["matrix"] = local

    def turned(rows, how):
        if how == "point":
            return [point(turn, r) for r in rows]
        if how == "vector":
            return [normalized(vector(turn, r)) for r in rows]
        if how == "inverse_bind":
            return [tuple(multiply(list(r), unturn)) for r in rows]
        if how == "translation":
            return [point(turn, r) for r in rows]
        if how == "rotation":
            return [quat_multiply(Q_TO_GLTF, r) for r in rows]
        if how == "scale":
            return [tuple(v * scale for v in r) for r in rows]
        return rows

    buffer, views, accessors = bytearray(), [], []
    for i, accessor in enumerate(doc.get("accessors", [])):
        rows = document.accessor(i)
        if i in transforms:
            rows = turned(rows, transforms[i])
        payload = encode(rows, accessor)
        buffer.extend(bytes(-len(buffer) % 4))
        views.append({"buffer": 0, "byteOffset": len(buffer), "byteLength": len(payload)})
        buffer.extend(payload)
        out = {k: v for k, v in accessor.items() if k not in ("bufferView", "byteOffset", "min", "max")}
        out["bufferView"] = len(views) - 1
        if "min" in accessor:
            stored = list(struct.iter_unpack(f"<{len(rows[0])}{CODES[accessor['componentType']]}", payload)) \
                if accessor["componentType"] == 5126 else rows
            out["min"] = [min(r[k] for r in stored) for k in range(len(rows[0]))]
            out["max"] = [max(r[k] for r in stored) for k in range(len(rows[0]))]
        accessors.append(out)
    for image in doc.get("images", []):
        image.pop("bufferView", None)
        image.pop("mimeType", None)
    doc["accessors"], doc["bufferViews"] = accessors, views
    buffer.extend(bytes(-len(buffer) % 4))
    doc["buffers"] = [{"byteLength": len(buffer)}]
    doc["asset"] = {"version": "2.0", "generator": "OpenRAC asset export"}
    return doc, bytes(buffer)


def node_trs(node: dict) -> tuple:
    return (tuple(node.get("translation", (0, 0, 0))), tuple(node.get("rotation", (0, 0, 0, 1))),
            tuple(node.get("scale", (1, 1, 1))))


def normalized(v) -> tuple:
    length = math.sqrt(sum(x * x for x in v))
    return tuple(x / length for x in v) if length > 1e-12 else (0.0, 0.0, 1.0)


@dataclass
class LevelExport:
    """One level of an export: its folder, and the textures its models use."""
    level: int
    levels: Path
    dir: Path
    textures: dict = field(default_factory=dict)  # name -> entry of textures.json

    def texture(self, source: Path) -> str:
        """Copy a level texture into the export once; its name."""
        name = source.stem
        if name not in self.textures:
            data = source.read_bytes()
            target = self.dir / "textures" / source.name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
            width, height = pngio.size(data)
            self.textures[name] = {"file": source.name, "size": [width, height], "sha256": sha256(data)}
        return name


def export_model(export: LevelExport, family: str, class_id: int, entry: dict) -> dict:
    """One class's model as a glTF folder with its asset.json; returns the manifest."""
    source = export.levels / f"level_{export.level:02}" / entry["mesh"]
    data = source.read_bytes()
    document = gltf_read.read(source)
    scale = float(entry.get("scale", 1.0)) if family == "moby" else 1.0
    doc, buffer = to_gltf_axes(document, scale)
    name = entry.get("name")
    folder = f"{family}_{class_id}" + (f"_{slug(name)}" if name else "")
    out = export.dir / f"{family}s" / folder
    out.mkdir(parents=True)
    stem = f"{family}_{class_id}"
    textures = {}
    for image in doc.get("images", []):
        texture = document.image_path({"uri": image["uri"]})
        if texture is None or not texture.is_file():
            raise AssetError(f"{source}: its texture {image.get('uri')} is missing: get the levels ready again")
        texture_name = export.texture(texture)
        image["uri"] = f"../../textures/{texture.name}"
        textures[texture_name] = export.textures[texture_name]["size"]
    doc["buffers"][0]["uri"] = f"{stem}.bin"
    (out / f"{stem}.bin").write_bytes(buffer)
    (out / f"{stem}.gltf").write_text(json.dumps(doc, indent=1, allow_nan=False) + "\n")
    skeleton = None
    if document.list("skins"):
        names = [document.list("nodes")[j].get("name", "") for j in document.list("skins")[0]["joints"]]
        skeleton = {"joints": len({JOINT.match(n).group(1) for n in names if JOINT.match(n)}), "names": names}
    manifest = {
        "format": ASSET_FORMAT, "game": GAME, "version": VERSION, "levels_format": LEVELS_FORMAT,
        "level": export.level, "kind": family, "class": class_id, "name": name,
        "source": entry["mesh"], "source_sha256": sha256(data),
        "scale": scale, "model": f"{stem}.gltf",
        "files": {f"{stem}.gltf": file_hash(out / f"{stem}.gltf"), f"{stem}.bin": sha256(buffer)},
        "textures": textures, "skeleton": skeleton,
        # A moby class's model goes to every level whose class has this skeleton; a list of level
        # numbers here keeps it to those.
        "levels": "all" if family == "moby" else [export.level],
        "axes": "glTF (Y up); the game's are Z up: (x, y, z) in the game is (x, z, -y) here",
    }
    (out / "asset.json").write_text(json.dumps(manifest, indent=1) + "\n")
    return manifest


def unique(path: Path) -> Path:
    """PATH, or PATH-2, PATH-3...: the first that is not there yet."""
    candidate, n = path, 2
    while candidate.exists():
        candidate, n = path.with_name(f"{path.name}-{n}"), n + 1
    return candidate


def export(levels: Path, out: Path, level_ids: list[int], what: str, classes: list[int] | None = None) -> dict:
    """Write the export (see the module's docstring) into the new folder OUT."""
    if what not in WHAT:
        raise AssetError(f"--what is one of {', '.join(WHAT)}")
    out = output_dir(out)
    manifests = {level: level_manifest(levels, level) for level in level_ids}
    temporary = Path(tempfile.mkdtemp(prefix=f".{out.name}-", dir=out.parent if out.parent.exists() else None))
    counts = {"models": 0, "textures": 0}
    try:
        for level, manifest in manifests.items():
            export_level = LevelExport(level, levels, temporary / f"level_{level:02}")
            export_level.dir.mkdir(parents=True)
            for family in FAMILIES[what]:
                for key, entry in sorted(manifest["classes"].get(family, {}).items(), key=lambda kv: int(kv[0])):
                    if entry.get("mesh") and (classes is None or int(key) in classes):
                        export_model(export_level, family, int(key), entry)
                        counts["models"] += 1
            if what in ("textures", "all"):
                for name in manifest.get("textures", []):
                    export_level.texture(levels / f"level_{level:02}" / name)
            textures = {"format": ASSET_FORMAT, "game": GAME, "version": VERSION, "levels_format": LEVELS_FORMAT,
                        "level": level, "textures": dict(sorted(export_level.textures.items()))}
            (export_level.dir / "textures").mkdir(exist_ok=True)
            (export_level.dir / "textures" / "textures.json").write_text(json.dumps(textures, indent=1) + "\n")
            counts["textures"] += len(export_level.textures)
            print(f"level {level:02}: {sum(1 for _ in export_level.dir.glob('*s/*/asset.json'))} models, "
                  f"{len(export_level.textures)} textures", flush=True)
        if counts["models"] == 0 and counts["textures"] == 0:
            raise AssetError("nothing to export: no class with a model was chosen")
        summary = {"format": ASSET_FORMAT, "tool": "openrac-assets", "game": GAME, "version": VERSION,
                   "levels_format": LEVELS_FORMAT, "id": os.urandom(8).hex(), "what": what,
                   "levels": sorted(level_ids), **counts}
        (temporary / "export.json").write_text(json.dumps(summary, indent=1) + "\n")
        (temporary / "README.txt").write_text(README)
        out.parent.mkdir(parents=True, exist_ok=True)
        temporary.rename(out)
    finally:
        if temporary.exists():
            shutil.rmtree(temporary)
    return summary


# ---- import -----------------------------------------------------------------------------------


@dataclass
class Report:
    """What the import did, asset by asset, in words."""
    lines: list[str] = field(default_factory=list)
    refused: int = 0
    warnings: int = 0

    def note(self, text: str) -> None:
        self.lines.append(text)
        print(text, flush=True)

    def warn(self, text: str) -> None:
        self.warnings += 1
        self.note(f"warning: {text}")

    def refuse(self, text: str) -> None:
        self.refused += 1
        self.note(f"refused: {text}")


class LevelsIndex:
    """The extracted levels, for finding the same texture or skeleton in every level."""

    def __init__(self, levels: Path):
        self.levels = levels
        self.manifests = {}
        for path in sorted(levels.glob("level_*/manifest.json")):
            try:
                manifest = json.loads(path.read_text())
            except (OSError, json.JSONDecodeError):
                continue
            if manifest.get("format") == LEVELS_FORMAT and isinstance(manifest.get("level"), int):
                self.manifests[manifest["level"]] = manifest
        self._by_hash = None
        self._skeletons = {}

    def original(self, level: int, name: str) -> bytes:
        """A level texture's PNG as extracted."""
        return (self.levels / f"level_{level:02}" / "textures" / f"{name}.png").read_bytes()

    def signature(self, level: int, class_id: int) -> str:
        """A hash of a class model's vertex positions."""
        _, _, document = self.skeleton(level, class_id)
        digest = hashlib.sha256()
        for position in original_vertices(document)[0]:
            digest.update(struct.pack("<3f", *position))
        return digest.hexdigest()

    def by_hash(self) -> dict[str, list[tuple[int, str]]]:
        """A texture's PNG hash -> every (level, name) with that picture."""
        if self._by_hash is None:
            self._by_hash = {}
            for level in self.manifests:
                for path in sorted((self.levels / f"level_{level:02}" / "textures").glob("*.png")):
                    self._by_hash.setdefault(file_hash(path), []).append((level, path.stem))
        return self._by_hash

    def skeleton(self, level: int, class_id: int):
        """(joint names in skin order, inverse bind matrices) of a moby class in a level, or None."""
        key = (level, class_id)
        if key not in self._skeletons:
            entry = self.manifests.get(level, {}).get("classes", {}).get("moby", {}).get(str(class_id))
            result = None
            if entry and entry.get("mesh"):
                document = gltf_read.read(self.levels / f"level_{level:02}" / entry["mesh"])
                if document.list("skins"):
                    skin = document.list("skins")[0]
                    names = [document.list("nodes")[j].get("name", "") for j in skin["joints"]]
                    result = (names, document.accessor(skin["inverseBindMatrices"]), document)
                else:
                    result = ([], [], document)
            self._skeletons[key] = result
        return self._skeletons[key]


@dataclass
class Pack:
    """The pack being written: files by path inside it, and what they replace."""
    files: dict = field(default_factory=dict)        # "level_NN/textures/x.png" -> bytes
    sources: dict = field(default_factory=dict)      # the same path -> the asset it came from

    def put(self, path: str, data: bytes, source: str, report: Report) -> None:
        if path in self.files and self.files[path] != data:
            report.warn(f"{path} comes from both {self.sources[path]} and {source}; {source}'s is kept")
        self.files[path] = data
        self.sources[path] = source


def check_texture(name: str, data: bytes, original: list[int], report: Report, where: str) -> bool:
    """An edited texture: a PNG of the original's shape (any size). False when refused."""
    try:
        width, height, pixels = pngio.read(data)
    except PngError as exc:
        report.refuse(f"{where}: {exc}")
        return False
    ow, oh = original
    if width * oh != height * ow:
        g = math.gcd(ow, oh)
        fits = ", ".join(f"{ow * k} x {oh * k}" for k in (2, 4, 8) if max(ow, oh) * k <= MAX_SIDE)
        report.refuse(f"{where} is {width} x {height}, but the game's {name} is {ow} x {oh} "
                      f"({ow // g}:{oh // g}); keep that shape" + (f", for example {fits}" if fits else ""))
        return False
    if max(width, height) > MAX_SIDE:
        report.refuse(f"{where} is {width} x {height}; at most {MAX_SIDE} on a side is taken")
        return False
    if width < ow:
        report.warn(f"{where} is smaller than the game's ({width} x {height} for {ow} x {oh})")
    return True


def import_textures(edited: Path, level: int, index: LevelsIndex, pack: Pack, report: Report) -> dict:
    """The textures of one exported level that differ from what was exported. Returns
    name -> (data, original hash) for those taken, so models can name them."""
    folder = edited / f"level_{level:02}" / "textures"
    manifest_path = folder / "textures.json"
    taken = {}
    if not manifest_path.is_file():
        return taken
    manifest = read_json(manifest_path)
    for name, entry in manifest.get("textures", {}).items():
        path = folder / entry["file"]
        if not path.is_file():
            report.warn(f"level {level:02}: {entry['file']} is gone from the export; the game's is kept")
            continue
        data = path.read_bytes()
        if sha256(data) == entry["sha256"]:
            continue
        if not check_texture(name, data, entry["size"], report, f"level {level:02} texture {entry['file']}"):
            continue
        taken[name] = (data, entry["sha256"])
        targets = index.by_hash().get(entry["sha256"]) or [(level, name)]
        for target_level, target_name in targets:
            pack.put(f"level_{target_level:02}/textures/{target_name}.png", data, f"texture {name}", report)
        width, height = pngio.size(data)
        others = len(targets) - 1
        report.note(f"texture {name} ({width} x {height}) replaces the game's in {len(targets)} place"
                    f"{'s' if len(targets) != 1 else ''}" + (f" (the same picture in {others} more)" if others else ""))
    return taken


def edited_model(folder: Path, manifest: dict) -> Path | None:
    """The model file to read back: the exported one if it changed, or the newest glTF/GLB put
    beside it; None when nothing changed."""
    exported = folder / manifest["model"]
    files = manifest.get("files", {})
    changed = [p for p in folder.iterdir() if p.suffix.lower() in (".gltf", ".glb") and p.is_file()
               and not (p.name in files and file_hash(p) == files[p.name])]
    bin_name = Path(manifest["model"]).with_suffix(".bin").name
    if not changed and exported.is_file() and (folder / bin_name).is_file() \
            and file_hash(folder / bin_name) != files.get(bin_name):
        changed = [exported]
    if not changed:
        return None
    return max(changed, key=lambda p: p.stat().st_mtime)


@dataclass
class Vertex:
    position: tuple
    normal: tuple
    uv: tuple
    colour: tuple | None
    light: float | None
    skin: tuple | None  # ((joint number, weight 0..1), ...)


def nearest(points: list[tuple], cell: float = 0.05):
    """A lookup of the nearest of POINTS (a uniform grid, widened until it finds one)."""
    grid = {}
    for i, p in enumerate(points):
        grid.setdefault(tuple(int(math.floor(c / cell)) for c in p), []).append(i)

    def find(p) -> int:
        centre = tuple(int(math.floor(c / cell)) for c in p)
        for radius in range(0, 64):
            best, distance = None, None
            for dx in range(-radius, radius + 1):
                for dy in range(-radius, radius + 1):
                    for dz in range(-radius, radius + 1):
                        if max(abs(dx), abs(dy), abs(dz)) != radius:
                            continue
                        for i in grid.get((centre[0] + dx, centre[1] + dy, centre[2] + dz), ()):
                            d = sum((a - b) ** 2 for a, b in zip(points[i], p))
                            if distance is None or d < distance:
                                best, distance = i, d
            if best is not None:
                return best
        return min(range(len(points)), key=lambda i: sum((a - b) ** 2 for a, b in zip(points[i], p)))
    return find


def original_vertices(document: gltf_read.Document) -> tuple[list, list, list]:
    """The original model's vertices in game axes: positions, colours and light slots."""
    positions, colours, lights = [], [], []
    for node in document.list("nodes"):
        if "mesh" not in node:
            continue
        for primitive in document.list("meshes")[node["mesh"]]["primitives"]:
            attributes = primitive["attributes"]
            rows = document.accessor(attributes["POSITION"])
            positions += rows
            colours += document.accessor(attributes["COLOR_0"]) if "COLOR_0" in attributes else [None] * len(rows)
            lights += [r[0] for r in document.accessor(attributes["_LIGHT_SLOT"])] \
                if "_LIGHT_SLOT" in attributes else [None] * len(rows)
    return positions, colours, lights


def joint_parents(document: gltf_read.Document) -> dict[int, int]:
    return {c: i for i, node in enumerate(document.list("nodes")) for c in node.get("children", [])}


def read_edited(document: gltf_read.Document, original, joint_count: int, scale: float, report: Report,
                where: str) -> list[tuple[str, list[Vertex], list[tuple], dict]] | None:
    """The edited file's meshes in the game's model space: (node name, vertices, triangles as
    (a, b, c, material index)), materials. None (with the reason reported) when it cannot be mapped."""
    turn = [v * (scale if i % 4 < 3 and i < 12 else 1) for i, v in enumerate(TO_GLTF)]
    unturn = invert(turn)
    names, game_ibms = (original[0], original[1]) if original else ([], [])
    game_bind = {}
    for slot, name in enumerate(names):
        match = JOINT.match(name)
        if match:
            game_bind.setdefault(int(match.group(1)), invert(list(game_ibms[slot])))
    nodes = document.list("nodes")
    parents = joint_parents(document)
    meshes = []
    for n, node in enumerate(nodes):
        if "mesh" not in node:
            continue
        label = node.get("name") or document.list("meshes")[node["mesh"]].get("name") or f"node {n}"
        mapping = None   # skin slot -> (game joint or None, the bone's name)
        rigid = None     # a mesh parented to a bone: (game joint, matrix)
        if "skin" in node:
            skin = document.list("skins")[node["skin"]]
            ibms = document.accessor(skin["inverseBindMatrices"]) if "inverseBindMatrices" in skin \
                else [tuple(gltf_read.IDENTITY)] * len(skin["joints"])
            mapping, spaces, cache = [], {}, {}
            for slot, joint in enumerate(skin["joints"]):
                match = JOINT.match(nodes[joint].get("name", ""))
                number = int(match.group(1)) if match else None
                if number is not None and number < joint_count and number in game_bind:
                    mapping.append((number, nodes[joint]["name"]))
                    # M_j^-1, from the game's model space to the file's mesh space, by game joint.
                    spaces[number] = invert(multiply(game_bind[number], list(ibms[slot])))
                else:
                    mapping.append((None, nodes[joint].get("name") or f"node {joint}"))
        elif joint_count:
            at = n
            while at in parents and rigid is None:
                at = parents[at]
                match = JOINT.match(nodes[at].get("name", ""))
                if match and int(match.group(1)) < joint_count and int(match.group(1)) in game_bind:
                    number = int(match.group(1))
                    rigid = (number, multiply(multiply(game_bind[number], invert(document.node_world(at))),
                                              document.node_world(n)))
            if rigid is None:
                report.refuse(f"{where}: {label} is not bound to the armature: give it weights (Ctrl+P > With "
                              "Automatic Weights) or join it into the skinned mesh")
                return None
        world = multiply(unturn, document.node_world(n)) if mapping is None and rigid is None else None
        vertices, triangles = [], []
        for p, primitive in enumerate(document.list("meshes")[node["mesh"]]["primitives"]):
            if primitive.get("mode", 4) != 4:
                report.refuse(f"{where}: {label} has lines or points; export triangles only")
                return None
            attributes = primitive["attributes"]
            if "TEXCOORD_0" not in attributes:
                report.refuse(f"{where}: {label} has no UV map; unwrap it (or keep the one it came with)")
                return None
            positions = document.accessor(attributes["POSITION"])
            normals = document.accessor(attributes["NORMAL"]) if "NORMAL" in attributes else None
            uvs = document.accessor(attributes["TEXCOORD_0"])
            colours = document.accessor(attributes["COLOR_0"]) if "COLOR_0" in attributes else None
            lights = document.accessor(attributes["_LIGHT_SLOT"]) if "_LIGHT_SLOT" in attributes else None
            sets = []
            k = 0
            while f"JOINTS_{k}" in attributes and f"WEIGHTS_{k}" in attributes:
                sets.append((document.accessor(attributes[f"JOINTS_{k}"], normalize=False),
                             document.accessor(attributes[f"WEIGHTS_{k}"])))
                k += 1
            base = len(vertices)
            unknown, unweighted, trimmed = set(), 0, 0
            for i, position in enumerate(positions):
                normal = normals[i] if normals else (0.0, 0.0, 1.0)
                skin = None
                if mapping is not None:
                    influences = {}
                    for joints, weights in sets:
                        for j, w in zip(joints[i], weights[i]):
                            if w <= 1e-6:
                                continue
                            if j >= len(mapping) or mapping[j][0] is None:
                                unknown.add(mapping[j][1] if j < len(mapping) else f"joint {j}")
                                continue
                            influences[mapping[j][0]] = influences.get(mapping[j][0], 0.0) + w
                    if not influences:
                        unweighted += 1
                    strongest = sorted(influences.items(), key=lambda kv: -kv[1])
                    if len(strongest) > 4:
                        trimmed += 1
                        strongest = strongest[:4]
                    total = sum(w for _, w in strongest) or 1.0
                    skin = tuple((j, w / total) for j, w in strongest)
                    bind = unskin(spaces, skin, cache) if skin else gltf_read.IDENTITY
                    game_position, game_normal = point(bind, position), normalized(vector(bind, normal))
                elif rigid is not None:
                    skin = ((rigid[0], 1.0),)
                    game_position, game_normal = point(rigid[1], position), normalized(vector(rigid[1], normal))
                else:
                    game_position, game_normal = point(world, position), normalized(vector(world, normal))
                vertices.append(Vertex(game_position, game_normal, tuple(uvs[i][:2]),
                                       tuple(colours[i]) + ((1.0,) if len(colours[i]) == 3 else ()) if colours else None,
                                       lights[i][0] if lights else None, skin))
            if unknown:
                report.refuse(f"{where}: {label} is weighted to bones the game's skeleton does not have: "
                              f"{', '.join(sorted(map(str, unknown))[:8])}. Keep the bones joint_000 to "
                              f"joint_{joint_count - 1:03} and move those weights onto them")
                return None
            if unweighted:
                report.refuse(f"{where}: {unweighted} vertices of {label} have no bone weights: weight-paint them "
                              "(or use Ctrl+P > With Automatic Weights)")
                return None
            if trimmed:
                report.warn(f"{where}: {trimmed} vertices of {label} had more than four bones; the four "
                            "strongest are kept, weights normalised")
            if "indices" in primitive:
                corners = [r[0] for r in document.accessor(primitive["indices"], normalize=False)]
            else:
                corners = list(range(len(positions)))
            if len(corners) % 3:
                report.refuse(f"{where}: {label}'s triangles are cut short")
                return None
            for t in range(0, len(corners), 3):
                a, b, c = (base + corners[t + k] for k in range(3))
                triangles.append((a, b, c, primitive.get("material")))
            if normals is None:
                flat_normals(vertices, triangles, base)
        meshes.append((label, vertices, triangles))
    if not meshes:
        report.refuse(f"{where}: the file has no mesh")
        return None
    return meshes


def unskin(spaces: dict, skin: tuple, cache: dict) -> list[float]:
    """The matrix that takes a vertex of SKIN from the file's mesh space to the game's model space.

    Each bone j gives M_j = (the game's bind pose of j) x (the file's inverse bind matrix of j).
    Blender's importer moves each vertex into its own armature's rest pose by blending, with the
    vertex's weights, the bones' changes of rest (it cannot hold a sheared rest pose, which a few
    of the game's joints have); its exporter writes that. So the vertex comes back through the
    inverse of the blend, (sum of w_j M_j^-1)^-1, which for bones that rest where the game's do is
    M_j itself."""
    if skin not in cache:
        blend = [0.0] * 16
        for j, w in skin:
            blend = [a + w * b for a, b in zip(blend, spaces[j])]
        blend[15] = 1.0
        cache[skin] = invert(blend)
    return cache[skin]


def flat_normals(vertices: list[Vertex], triangles: list[tuple], base: int) -> None:
    """Smooth normals for a primitive exported without any."""
    sums = {}
    for a, b, c, _ in triangles:
        if a < base:
            continue
        pa, pb, pc = (vertices[v].position for v in (a, b, c))
        ab = [pb[k] - pa[k] for k in range(3)]
        ac = [pc[k] - pa[k] for k in range(3)]
        n = (ab[1] * ac[2] - ab[2] * ac[1], ab[2] * ac[0] - ab[0] * ac[2], ab[0] * ac[1] - ab[1] * ac[0])
        for v in (a, b, c):
            s = sums.setdefault(v, [0.0, 0.0, 0.0])
            for k in range(3):
                s[k] += n[k]
    for v, s in sums.items():
        vertices[v].normal = normalized(s)


@dataclass
class EditedTexture:
    """A texture an edited model names: the level's own (name), or a picture of its own."""
    name: str | None
    data: bytes | None
    cutout: bool
    metal: bool


def model_textures(document: gltf_read.Document, manifest: dict, index: "LevelsIndex", export_textures: dict,
                   edited_textures: dict, report: Report, where: str) -> dict | None:
    """Material index -> EditedTexture. A material whose picture is one of the level's textures
    (same pixels) names it; an edited picture of a level texture is taken as that texture's edit;
    a new picture travels with the model."""
    out = {}
    for m, material in enumerate(document.list("materials")):
        name = material.get("name", "")
        cutout = material.get("alphaMode") == "MASK"
        metal = name.startswith("metal_")
        pbr = material.get("pbrMetallicRoughness", {})
        texture_index = pbr.get("baseColorTexture", {}).get("index")
        if texture_index is None:
            out[m] = EditedTexture("moby_untextured" if "moby_untextured" in export_textures else None, None,
                                   cutout, metal)
            continue
        source = document.list("textures")[texture_index].get("source")
        image = document.list("images")[source]
        try:
            data = document.image_bytes(image)
        except (GltfError, OSError) as exc:
            report.refuse(f"{where}: {exc}")
            return None
        # The picture's own name says which level texture it is (Blender keeps the file's name); a
        # picture of another name is the model's own, whatever its material is called.
        path = document.image_path(image)
        stem = path.stem if path is not None else re.sub(r"\.png$", "", image.get("name") or "")
        known = stem if stem in export_textures else None
        if known is not None:
            entry = export_textures[known]
            if sha256(data) == entry["sha256"] or (known in edited_textures and edited_textures[known][0] == data):
                out[m] = EditedTexture(known, None, cutout, metal)
                continue
            try:
                same = pngio.read(data)[2] == pngio.read(index.original(manifest["level"], known))[2]
            except (PngError, OSError):
                same = False
            if same:
                out[m] = EditedTexture(known, None, cutout, metal)
                continue
            if not check_texture(known, data, entry["size"], report, f"{where}: the picture of {name}"):
                return None
            edited_textures[known] = (data, entry["sha256"])
            out[m] = EditedTexture(known, None, cutout, metal)
            report.note(f"{where}: {name}'s picture differs from the game's {known}: taken as an edit of {known}")
            continue
        try:
            pngio.read(data)
        except PngError as exc:
            report.refuse(f"{where}: the picture of {name}: {exc}")
            return None
        out[m] = EditedTexture(None, data, cutout, metal)
    return out


def import_model(folder: Path, manifest: dict, index: LevelsIndex, edited_textures: dict,
                 export_textures: dict, pack: Pack, report: Report) -> None:
    """Read one edited model back and write its replacement GLB for every level it goes to."""
    level, family, class_id = manifest["level"], manifest["kind"], manifest["class"]
    where = f"level {level:02} {family} {class_id}" + (f" ({manifest['name']})" if manifest.get("name") else "")
    path = edited_model(folder, manifest)
    if path is None:
        return
    source = index.levels / f"level_{level:02}" / manifest["source"]
    if not source.is_file() or file_hash(source) != manifest["source_sha256"]:
        report.refuse(f"{where}: the levels were read from the disc again since this export and differ; "
                      "export again and redo the edit on the new export")
        return
    try:
        document = gltf_read.read(path)
    except (GltfError, OSError, ValueError) as exc:
        report.refuse(f"{where}: {path.name}: {exc}")
        return
    original_doc = gltf_read.read(source)
    original = None
    joint_count = 0
    if original_doc.list("skins"):
        skin = original_doc.list("skins")[0]
        names = [original_doc.list("nodes")[j].get("name", "") for j in skin["joints"]]
        original = (names, original_doc.accessor(skin["inverseBindMatrices"]))
        joint_count = 1 + max(int(JOINT.match(n).group(1)) for n in names if JOINT.match(n))
    try:
        meshes = read_edited(document, original, joint_count, float(manifest.get("scale", 1.0)), report,
                             f"{where}: {path.name}")
    except GltfError as exc:
        report.refuse(f"{where}: {path.name}: {exc}")
        return
    if meshes is None:
        return
    textures = model_textures(document, manifest, index, export_textures, edited_textures, report,
                              f"{where}: {path.name}")
    if textures is None:
        return
    fill_missing(meshes, original_doc, report, where)
    # Where it goes: every level whose class has the same skeleton, or the levels named.
    wanted = manifest.get("levels", [level])
    if wanted == "all" and family == "moby":
        targets = [lv for lv in sorted(index.manifests) if same_skeleton(index, level, lv, class_id)]
    elif wanted == "all":
        targets = [level]
    else:
        targets = [lv for lv in wanted if lv in index.manifests]
    if level not in targets:
        targets.insert(0, level)
    for target in targets:
        target_entry = index.manifests[target]["classes"].get(family, {}).get(str(class_id))
        if not target_entry or not target_entry.get("mesh"):
            continue
        names = texture_names(textures, index, level, target, export_textures, edited_textures, pack, report, where,
                              family)
        glb = port_glb(meshes, textures, names, original_doc if target == level else
                       gltf_read.read(index.levels / f"level_{target:02}" / target_entry["mesh"]),
                       f"{family}_{class_id}")
        pack.put(f"level_{target:02}/{target_entry['mesh']}", glb, where, report)
    triangles = sum(len(t) for _, _, t in meshes)
    report.note(f"{where}: {path.name} ({triangles} triangles) replaces the model in {len(targets)} level"
                f"{'s' if len(targets) != 1 else ''}")


def same_skeleton(index: LevelsIndex, level: int, other: int, class_id: int) -> bool:
    """Whether a moby class is the same model in two levels: the same joints and bind pose, or for
    a class without a skeleton the same vertices."""
    a, b = index.skeleton(level, class_id), index.skeleton(other, class_id)
    if a is None or b is None or a[0] != b[0]:
        return False
    if not a[0]:
        return index.signature(level, class_id) == index.signature(other, class_id)
    return all(abs(x - y) < 1e-4 for ra, rb in zip(a[1], b[1]) for x, y in zip(ra, rb))


def fill_missing(meshes, original_doc: gltf_read.Document, report: Report, where: str) -> None:
    """Vertex colours (the game's light multipliers) and light slots the edited file lacks, from
    the original's nearest vertex: Blender may drop them."""
    positions, colours, lights = original_vertices(original_doc)
    if not positions:
        return
    find = nearest(positions)
    filled = 0
    for _, vertices, _ in meshes:
        for v in vertices:
            if v.colour is None or v.light is None:
                i = find(v.position)
                if v.colour is None:
                    v.colour = colours[i] if colours[i] is not None else None
                if v.light is None:
                    v.light = lights[i]
                filled += 1
    if filled:
        report.note(f"{where}: {filled} vertices took their colour and light slot from the original's nearest")


def texture_names(textures: dict, index: LevelsIndex, level: int, target: int, export_textures: dict,
                  edited_textures: dict, pack: Pack, report: Report, where: str, family: str) -> dict:
    """Material index -> the texture name the model uses in level TARGET, writing pictures of its
    own into the pack (<family>_ovr_<hash>: the port samples a moby_* texture as the game's moby
    textures)."""
    names = {}
    for m, texture in textures.items():
        if texture.name is not None and target == level:
            names[m] = texture.name
            continue
        if texture.name is not None:
            # The same picture in the other level, by the hash of the original.
            original = export_textures[texture.name]["sha256"] if texture.name in export_textures else None
            match = next((n for lv, n in index.by_hash().get(original, []) if lv == target), None)
            if texture.metal:
                match = texture.name  # each level's own chrome and glass maps
            if match is not None:
                names[m] = match
                continue
            data = edited_textures[texture.name][0] if texture.name in edited_textures \
                else index.original(level, texture.name)
        else:
            data = texture.data
        name = f"{family}_ovr_{sha256(data)[:16]}"
        pack.put(f"level_{target:02}/textures/{name}.png", data, where, report)
        names[m] = name
    return names


def port_glb(meshes, textures: dict, names: dict, original_doc: gltf_read.Document, stem: str) -> bytes:
    """The replacement model as the port reads a class GLB (port.py, godot.moby_glb): game axes,
    model units, one node per mesh, skinned to joint nodes joint_NNN in the game's numbering with
    the original's inverse bind matrices; materials name ../textures/<name>.png."""
    gltf = Gltf()
    main = Mesh(stem, normals=[], colours=[], light_slots=[])
    metal = Mesh(f"{stem}_metal", normals=[], colours=[], light_slots=[])
    skinned = any(v.skin for _, vertices, _ in meshes for v in vertices)
    for target in (main, metal):
        target.skins = [] if skinned else None
    keys = {}
    for _, vertices, triangles in meshes:
        local = {}
        for a, b, c, material in triangles:
            texture = textures.get(material, EditedTexture("moby_untextured", None, False, False))
            target = metal if texture.metal else main
            corners = []
            for v in (a, b, c):
                if (id(target), v) not in local:
                    vertex = vertices[v]
                    local[(id(target), v)] = len(target.positions)
                    target.positions.append(vertex.position)
                    target.normals.append(vertex.normal)
                    target.uvs.append(vertex.uv)
                    target.colours.append(tuple(vertex.colour) if vertex.colour is not None else (1.0, 1.0, 1.0, 1.0))
                    target.light_slots.append(-1 if vertex.light is None else vertex.light)
                    if skinned:
                        target.skins.append(quantise(vertex.skin or ((0, 1.0),)))
                corners.append(local[(id(target), v)])
            name = names.get(material) or "moby_untextured"
            key = (name, texture.cutout, texture.metal)
            keys[key] = None
            target.add_face(key, tuple(corners))
    materials = {key: gltf.material(key[0], f"../textures/{key[0]}.png", cutout=key[1]) for key in keys}
    mesh_index = gltf.mesh(main, materials) if main.triangles else None
    metal_index = gltf.mesh(metal, materials) if metal.triangles else None
    skin = None
    if skinned:
        nodes = original_doc.list("nodes")
        source_skin = original_doc.list("skins")[0]
        ibms = original_doc.accessor(source_skin["inverseBindMatrices"])
        by_number = {}
        for slot, j in enumerate(source_skin["joints"]):
            match = JOINT.match(nodes[j].get("name", ""))
            if match and nodes[j]["name"] == f"joint_{int(match.group(1)):03}":
                by_number[int(match.group(1))] = (j, ibms[slot])
        count = max(by_number) + 1
        first = len(gltf.doc["nodes"])
        parents = joint_parents(original_doc)
        for number in range(count):
            j, _ = by_number.get(number, (None, None))
            node = nodes[j] if j is not None else {}
            children = [first + int(JOINT.match(nodes[c]["name"]).group(1)) for c in node.get("children", [])
                        if nodes[c].get("name", "") in (f"joint_{k:03}" for k in by_number)
                        and JOINT.match(nodes[c]["name"])]
            parent = parents.get(j) if j is not None else None
            is_root = parent is None or not JOINT.match(nodes[parent].get("name", ""))
            fields = {key: node[key] for key in ("translation", "rotation", "scale", "matrix") if key in node}
            gltf.node(f"joint_{number:03}", root=is_root, **fields, **({"children": children} if children else {}))
        skin = gltf.skin(list(range(first, first + count)),
                         [list(by_number[n][1]) if n in by_number else gltf_read.IDENTITY for n in range(count)])
    if mesh_index is not None:
        gltf.node(stem, mesh_index, **({"skin": skin} if skin is not None else {}))
    if metal_index is not None:
        gltf.node(f"{stem}_metal", metal_index, **({"skin": skin} if skin is not None else {}))
    return gltf.glb()


def quantise(skin: tuple) -> tuple:
    """Weights 0..1 as 256ths summing to 256 (the GLB writer's form), largest remainders first."""
    raw = [(j, w * 256) for j, w in skin]
    whole = [(j, int(w)) for j, w in raw]
    short = 256 - sum(w for _, w in whole)
    order = sorted(range(len(raw)), key=lambda k: -(raw[k][1] - whole[k][1]))
    for k in order[:short]:
        whole[k] = (whole[k][0], whole[k][1] + 1)
    return tuple((j, w) for j, w in whole if w > 0) or ((skin[0][0], 256),)


def import_edits(edited: Path, levels: Path, report: Report) -> Pack:
    """Everything changed in an export folder, as the files of a pack."""
    summary_path = edited / "export.json"
    if not summary_path.is_file():
        raise AssetError(f"{edited} is not an export of OpenRAC's (it has no export.json): choose the folder "
                         "the export made")
    summary = read_json(summary_path)
    if (summary.get("game"), summary.get("version")) != (GAME, VERSION):
        raise AssetError(f"{edited} holds {summary.get('game')}/{summary.get('version')}, not {GAME}/{VERSION}")
    if summary.get("format") != ASSET_FORMAT:
        raise AssetError(f"{edited} was exported in format {summary.get('format')}; this tool reads {ASSET_FORMAT}")
    index = LevelsIndex(levels)
    pack = Pack()
    for level in summary.get("levels", []):
        if level not in index.manifests:
            raise AssetError(f"{levels} has no level {level:02} of format {LEVELS_FORMAT}: get the levels ready")
        textures_manifest = edited / f"level_{level:02}" / "textures" / "textures.json"
        export_textures = read_json(textures_manifest).get("textures", {}) if textures_manifest.is_file() else {}
        edited_textures = import_textures(edited, level, index, pack, report)
        for manifest_path in sorted((edited / f"level_{level:02}").glob("*s/*/asset.json")):
            manifest = read_json(manifest_path)
            if manifest.get("format") != ASSET_FORMAT or manifest.get("level") != level:
                report.refuse(f"{manifest_path}: not an asset manifest of this export")
                continue
            import_model(manifest_path.parent, manifest, index, edited_textures, export_textures, pack, report)
        # A picture a model's material took as an edit of a level texture goes where that texture is.
        for name, (data, original_hash) in edited_textures.items():
            for target_level, target_name in index.by_hash().get(original_hash) or [(level, name)]:
                path = f"level_{target_level:02}/textures/{target_name}.png"
                if path not in pack.files:
                    pack.put(path, data, f"texture {name}", report)
    return pack


def write_pack(pack: Pack, out: Path, name: str, author: str, description: str, source_id: str | None) -> dict:
    """The pack folder (overrides.py's format) with its openrac-mod.json."""
    import overrides
    replaces = overrides.describe(pack.files)
    manifest = {"format": overrides.PACK_FORMAT, "game": GAME, "version": VERSION, "name": name,
                "author": author, "description": description, "levels_format": LEVELS_FORMAT,
                "replaces": replaces}
    if source_id:
        manifest["export"] = source_id
    out.mkdir(parents=True)
    for path, data in sorted(pack.files.items()):
        target = out / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    (out / overrides.MANIFEST).write_text(json.dumps(manifest, indent=1) + "\n")
    return manifest


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    out = commands.add_parser("export", help="models and textures as glTF and PNG files")
    out.add_argument("levels", type=Path, help="the extracted levels (assets/port/rac1/pal, or <data>/port/pal)")
    out.add_argument("out", type=Path, help="a new folder for the export")
    out.add_argument("--level", type=int, action="append", required=True, help="a level (repeatable)")
    out.add_argument("--what", choices=WHAT, default="mobys",
                     help="mobys (characters, enemies, items), textures (every texture), scenery (ties and "
                          "shrubs) or all (default: mobys)")
    out.add_argument("--class", dest="classes", type=int, action="append", help="only this class (repeatable)")
    out.add_argument("--unique", action="store_true",
                     help="when OUT exists, write OUT-2 (OUT-3...) instead: an earlier export is never touched")
    back = commands.add_parser("import", help="read an edited export back as an overrides pack")
    back.add_argument("edited", type=Path, help="the export folder, edited")
    back.add_argument("--levels", type=Path, required=True, help="the extracted levels the export came from")
    where = back.add_mutually_exclusive_group(required=True)
    where.add_argument("--out", type=Path, help="a new folder for the pack")
    where.add_argument("--into", type=Path, help="an overrides folder (overrides.py): the pack is added to it")
    back.add_argument("--name", help="the pack's name (default: the export folder's)")
    back.add_argument("--author", default="", help="who made it")
    back.add_argument("--description", default="", help="a sentence about it")
    back.add_argument("--strict", action="store_true", help="write nothing when anything is refused")
    args = parser.parse_args(argv)
    try:
        if args.command == "export":
            target = unique(args.out.resolve()) if args.unique else args.out
            summary = export(args.levels, target, sorted(set(args.level)), args.what, args.classes)
            # The launcher opens the folder this last line names.
            print(f"Exported {summary['models']} models and {summary['textures']} textures: {target.resolve()}")
            return 0
        report = Report()
        edited = args.edited.resolve()
        pack = import_edits(edited, args.levels, report)
        if not pack.files:
            print("Nothing to import: no file in the folder differs from what was exported."
                  + (f" ({report.refused} refused, above.)" if report.refused else ""))
            return 1
        if report.refused and args.strict:
            print(f"{report.refused} refused (above); nothing was written (--strict).")
            return 1
        name = args.name or edited.name
        source_id = read_json(edited / "export.json").get("id")
        if args.into is not None:
            import overrides
            with tempfile.TemporaryDirectory(prefix=".import-", dir=overrides.ensure(args.into)) as scratch:
                staged = Path(scratch) / "pack"
                write_pack(pack, staged, name, args.author, args.description, source_id)
                pack_id = overrides.install(staged, args.into, replace_export=source_id)
            where_to = f"{args.into.resolve()} as {pack_id}"
        else:
            write_pack(pack, output_dir(args.out), name, args.author, args.description, source_id)
            where_to = str(args.out.resolve())
        textures = sum(1 for p in pack.files if "/textures/" in p)
        models = len(pack.files) - textures
        tail = f"; {report.refused} refused (see above)" if report.refused else ""
        print(f"Imported {models} model file(s) and {textures} texture file(s) into {where_to}{tail}")
        return 0
    except (AssetError, GltfError, PngError, OSError) as exc:
        print(f"assets: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
