#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 the OpenRAC contributors
"""A mod's archive, from the folder its build laid out (docs/port/MODS.md, sdk/README.md).

    package_mod.py build/package/my_mod [-o dist]       dist/<id>-<version>-<platform>.zip
    package_mod.py my-script-mod [-o dist]               dist/<id>-<version>.zip (a script mod: no build)
    package_mod.py --merge A.zip B.zip ... -o OUT.zip    one archive for all their platforms
    package_mod.py --check openrac-mod.json              the manifest checked, nothing written

A native mod is packaged from the folder its build laid out; a script mod ("type": "script",
MODS.md section 11) from its own folder: the manifest, its scripts, and the .luau, .json and .md
files and the license beside them (sub-folders too, for the modules it requires), one archive for
every platform.

The manifest is checked by the rules of MODS.md, section 3, the same the launcher and the port
apply; an archive that breaks one is not written. Standard library only.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
import zipfile
from pathlib import Path, PurePosixPath

MANIFEST = "openrac-mod.json"
FORMAT = 2
# The interface's major versions this SDK knows (openrac_mod.h, OPENRAC_MOD_API_MAJOR).
API_MAJORS = {1}
# The first interface with script mods: 1.1.
SCRIPT_API = (1, 1)
# What a script mod's archive carries of its folder, beside the manifest and its scripts.
SCRIPT_FILES = re.compile(r"\.(luau|json|md)$", re.IGNORECASE)
# Folders a script mod's archive never takes (a build, the SDK fetched, a repository's own).
SKIP_DIRS = {"build", "dist", "node_modules", "__pycache__"}
PLATFORMS = ("macos-arm64", "macos-x86_64", "linux-x86_64", "linux-aarch64", "windows-x86_64", "windows-arm64")
ID = re.compile(r"^[a-z0-9]+(-[a-z0-9]+)*$")
SEMVER = re.compile(r"^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(-[0-9A-Za-z.-]+)?$")
API = re.compile(r"^(0|[1-9]\d*)\.(0|[1-9]\d*)$")
REQUIREMENT = re.compile(r"^(>=|=)(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$")
KEY = re.compile(r"^[a-z0-9_]+$")
GAME = re.compile(r"^[a-z0-9]+$")
SETTING_TYPES = {"bool", "int", "float", "choice", "string", "keybind"}
# A keybind's names (interface 1.2): openrac/keys.h's table, beside this tool in the SDK.
_KEYS_H = Path(__file__).resolve().parent.parent / "include" / "openrac" / "keys.h"
BIND_NAMES = frozenset(re.findall(r'\{"([A-Za-z0-9]+)", ', _KEYS_H.read_text())) if _KEYS_H.is_file() else frozenset()


def bind_problem(value, most: int = 2) -> str | None:
    """Why a keybind's value is not one (names of keys.h, comma-separated, none twice, at most
    `most`), or None."""
    if not isinstance(value, str):
        return "is not text"
    names = [n for n in re.split(r"[,\s]+", value) if n]
    unknown = [n for n in names if n not in BIND_NAMES]
    if unknown:
        return f"names no key, button or wheel direction: {', '.join(unknown)}"
    if len(set(names)) != len(names):
        return "names one input twice"
    if len(names) > most:
        return f"has {len(names)} binds, more than {most}"
    return None
# Each game's hook points (MODS.md, section 4): bindings/<game>.ids.lock beside this tool in the SDK,
# a stable id per line with every alias it has had. A manifest's "hooks" may name either.
_BINDINGS = Path(__file__).resolve().parent.parent / "bindings"
HOOK_NAME = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
HOOK_ID = re.compile(r"^[a-z0-9]+(\.[a-z0-9_]+)+$")


def hook_names(game: str, bindings: Path | None = None) -> frozenset[str] | None:
    """Every name `game`'s hook points answer to (their ids and aliases), or None when this SDK has
    no table for it."""
    lock = (bindings or _BINDINGS) / f"{game}.ids.lock"
    if not GAME.match(game) or not lock.is_file():
        return None
    names = set()
    for line in lock.read_text(encoding="utf-8").splitlines():
        if line and not line.startswith("#"):
            cols = line.split("\t")
            names.add(cols[0])
            names.update(a for a in (cols[2].split(",") if len(cols) > 2 else []) if a)
    return frozenset(names)


def _check_hooks(hooks, games, errors: list[str], bindings: Path | None = None) -> None:
    """"hooks": the hook points the mod takes, each by its stable id (rac1.player.take_damage) or an
    alias (Hero_TakeDamage). One of the mod's games must have it, when this SDK has that game's table."""
    if hooks is None:
        return
    if not isinstance(hooks, list) or not all(isinstance(h, str) and (HOOK_NAME.match(h) or HOOK_ID.match(h))
                                               for h in hooks):
        errors.append("hooks: a list of hook points, by id (rac1.player.take_damage) or alias (Hero_TakeDamage)")
        return
    tables = [hook_names(g, bindings) for g in (games if isinstance(games, dict) else {})]
    known = [t for t in tables if t is not None]
    if not known:
        return
    for h in hooks:
        if not any(h in t for t in known):
            errors.append(f"hooks: {h} is not a hook point of {', '.join(sorted(games))} (docs/reference/hooks-<game>.md)")


# Files for people, carried beside the manifest.
EXTRAS = re.compile(r"^(README|LICENSE|COPYING)", re.IGNORECASE)


class ManifestError(ValueError):
    pass


def _number(v) -> bool:
    return isinstance(v, (int, float)) and not isinstance(v, bool)


def _check_setting(i: int, s, errors: list[str]) -> None:
    where = f"settings[{i}]"
    if not isinstance(s, dict):
        errors.append(f"{where}: not an object")
        return
    key = s.get("key")
    if not isinstance(key, str) or not KEY.match(key):
        errors.append(f"{where}: key must be lower case letters, digits and _")
    else:
        where = f"setting {key}"
    if not isinstance(s.get("name"), str) or not s["name"]:
        errors.append(f"{where}: no name")
    if "description" in s and not isinstance(s["description"], str):
        errors.append(f"{where}: description is not text")
    kind = s.get("type")
    if kind not in SETTING_TYPES:
        errors.append(f"{where}: type must be one of {', '.join(sorted(SETTING_TYPES))}")
        return
    if "default" not in s:
        errors.append(f"{where}: no default")
        return
    d = s["default"]
    if kind == "bool":
        if not isinstance(d, bool):
            errors.append(f"{where}: the default is not true or false")
    elif kind in ("int", "float"):
        is_int = isinstance(d, int) and not isinstance(d, bool)
        if not (is_int if kind == "int" else _number(d)):
            errors.append(f"{where}: the default is not {'an integer' if kind == 'int' else 'a number'}")
            return
        lo, hi = s.get("min"), s.get("max")
        for name, v in (("min", lo), ("max", hi)):
            if v is not None and not (_number(v) and (kind == "float" or isinstance(v, int))):
                errors.append(f"{where}: {name} is not {'an integer' if kind == 'int' else 'a number'}")
                return
        if lo is not None and hi is not None and lo > hi:
            errors.append(f"{where}: min is above max")
        if (lo is not None and d < lo) or (hi is not None and d > hi):
            errors.append(f"{where}: the default is out of range")
        if kind == "float" and "step" in s and not (_number(s["step"]) and s["step"] > 0):
            errors.append(f"{where}: step must be a positive number")
    elif kind == "choice":
        choices = s.get("choices")
        if not isinstance(choices, list) or len(choices) < 2:
            errors.append(f"{where}: a choice needs two choices or more")
            return
        values = []
        for c in choices:
            if not (isinstance(c, dict) and isinstance(c.get("value"), str) and isinstance(c.get("name"), str)):
                errors.append(f"{where}: each choice is {{value, name}}, both text")
                return
            values.append(c["value"])
        if len(set(values)) != len(values):
            errors.append(f"{where}: a value twice among the choices")
        if d not in values:
            errors.append(f"{where}: the default is not one of the choices")
    elif kind == "keybind":
        most = s.get("binds", 2)
        if not (isinstance(most, int) and not isinstance(most, bool) and most in (1, 2)):
            errors.append(f"{where}: binds must be 1 or 2")
            return
        why = bind_problem(d, most)
        if why:
            errors.append(f"{where}: the default {why}")
    elif kind == "string":
        if not isinstance(d, str):
            errors.append(f"{where}: the default is not text")
        m = s.get("max_length")
        if m is not None and not (isinstance(m, int) and not isinstance(m, bool) and m > 0):
            errors.append(f"{where}: max_length must be a positive integer")
        elif m is not None and isinstance(d, str) and len(d) > m:
            errors.append(f"{where}: the default is longer than max_length")


def _check_presets(presets, settings: dict, errors: list[str]) -> None:
    """"presets" (interface 1.2): [{id, name, values: {key: value}}], buttons in the launcher that
    set several keybind settings at once. Each value is one its keybind takes."""
    if presets is None:
        return
    if not isinstance(presets, list):
        errors.append("presets: a list")
        return
    ids = []
    for i, p in enumerate(presets):
        where = f"presets[{i}]"
        if not (isinstance(p, dict) and isinstance(p.get("id"), str) and KEY.match(p["id"])
                and isinstance(p.get("name"), str) and p["name"] and isinstance(p.get("values"), dict)):
            errors.append(f"{where}: {{id (a-z, 0-9, _), name, values}}")
            continue
        ids.append(p["id"])
        for key, value in p["values"].items():
            s = settings.get(key)
            if s is None or s.get("type") != "keybind":
                errors.append(f"preset {p['id']}: {key} is not a keybind setting of the mod")
                continue
            why = bind_problem(value, s.get("binds", 2) if isinstance(s.get("binds"), int) else 2)
            if why:
                errors.append(f"preset {p['id']}: {key} {why}")
    if len(set(ids)) != len(ids):
        errors.append("presets: an id twice")


def _check_refs(name: str, refs, errors: list[str], version: bool) -> None:
    if not isinstance(refs, list):
        errors.append(f"{name}: not a list")
        return
    for r in refs:
        if not isinstance(r, dict) or not isinstance(r.get("id"), str) or not ID.match(r["id"]):
            errors.append(f"{name}: each entry is {{id, ...}} with a mod's id")
            continue
        if version and "version" in r and not (isinstance(r["version"], str) and REQUIREMENT.match(r["version"])):
            errors.append(f"{name} {r['id']}: version must be >=X.Y.Z or =X.Y.Z")
        if not version and "reason" in r and not isinstance(r["reason"], str):
            errors.append(f"{name} {r['id']}: reason is not text")


def check_manifest(m, root: Path | None = None, bindings: Path | None = None) -> list[str]:
    """The rules a manifest breaks (none: an empty list). With `root`, the plugin files must be
    there for the platforms the manifest lists. `bindings`: where the games' hook tables are
    (the SDK's bindings/ by default)."""
    errors: list[str] = []
    if not isinstance(m, dict):
        return ["the manifest is not a JSON object"]
    if m.get("format") != FORMAT:
        if m.get("format") == 1:
            return ["format 1 is a data pack (editor/MODDING.md): it is added as a pack, not packaged here"]
        return [f"format must be {FORMAT}"]
    mid = m.get("id")
    if not isinstance(mid, str) or not ID.match(mid) or len(mid) > 64:
        errors.append("id: lower case letters and digits joined by single hyphens, at most 64 characters")
    if not isinstance(m.get("name"), str) or not m["name"].strip():
        errors.append("name: required")
    if not isinstance(m.get("version"), str) or not SEMVER.match(m["version"]):
        errors.append("version: MAJOR.MINOR.PATCH, with an optional -suffix")
    for f in ("author", "description", "license"):
        if f in m and not isinstance(m[f], str):
            errors.append(f"{f}: not text")
    if "homepage" in m and not (isinstance(m["homepage"], str) and m["homepage"].startswith("https://")):
        errors.append("homepage: an https:// address")
    games = m.get("games")
    if not isinstance(games, dict) or not games:
        errors.append("games: at least one game, with its discs")
    else:
        for g, versions in games.items():
            if not GAME.match(g):
                errors.append(f"games: {g!r} is not a game's id")
            if not isinstance(versions, list) or not versions or not all(
                    isinstance(v, str) and GAME.match(v) for v in versions):
                errors.append(f"games {g}: a list of at least one disc version (pal, ntsc)")
    kind = m.get("type", "native")
    if kind not in ("native", "script"):
        errors.append("type: native (the default) or script")
    scripts = m.get("scripts")
    if kind == "script":
        if "plugin" in m:
            errors.append("plugin: a script mod carries no library")
        api = m.get("api")
        if not isinstance(api, str) or not API.match(api):
            errors.append("api: MAJOR.MINOR, the interface the scripts are written for (required)")
        else:
            major, minor = (int(x) for x in api.split("."))
            if major not in API_MAJORS:
                errors.append(f"api: {api}: this SDK knows interface {', '.join(f'{x}.x' for x in sorted(API_MAJORS))}")
            elif (major, minor) < SCRIPT_API:
                errors.append(f"api: {api}: script mods need interface {SCRIPT_API[0]}.{SCRIPT_API[1]} or later")
        if not isinstance(scripts, list) or not scripts:
            errors.append("scripts: the .luau files to run, at least one")
        else:
            for path in scripts:
                p = PurePosixPath(path) if isinstance(path, str) else None
                if p is None or not path or p.is_absolute() or ".." in p.parts or "\\" in path \
                        or not path.endswith(".luau"):
                    errors.append(f"scripts: {path!r} is not a .luau file inside the mod")
                elif root is not None and not (root / p).is_file():
                    errors.append(f"scripts: {path} is not in the mod")
    elif scripts is not None:
        errors.append("scripts: only a script mod (\"type\": \"script\") has scripts")
    plugin = m.get("plugin")
    if kind == "script":
        pass
    elif plugin is not None:
        api = m.get("api")
        if not isinstance(api, str) or not API.match(api):
            errors.append("api: MAJOR.MINOR, the interface the mod was built for (required with a plugin)")
        elif int(api.split(".")[0]) not in API_MAJORS:
            errors.append(f"api: {api}: this SDK knows interface {', '.join(f'{x}.x' for x in sorted(API_MAJORS))}")
        if not isinstance(plugin, dict) or not plugin:
            errors.append("plugin: an object of platform: path, with at least one platform")
        else:
            for platform, path in plugin.items():
                if platform not in PLATFORMS:
                    errors.append(f"plugin: {platform!r} is not a platform ({', '.join(PLATFORMS)})")
                    continue
                p = PurePosixPath(path) if isinstance(path, str) else None
                if p is None or not path or p.is_absolute() or ".." in p.parts or "\\" in path:
                    errors.append(f"plugin {platform}: a relative path inside the mod")
                    continue
                if root is not None and not (root / p).is_file():
                    errors.append(f"plugin {platform}: {path} is not in the mod")
    elif "api" in m and not (isinstance(m["api"], str) and API.match(m["api"])):
        errors.append("api: MAJOR.MINOR")
    _check_hooks(m.get("hooks"), games, errors, bindings)
    settings = m.get("settings", [])
    if not isinstance(settings, list):
        errors.append("settings: a list")
    else:
        for i, s in enumerate(settings):
            _check_setting(i, s, errors)
        keys = [s.get("key") for s in settings if isinstance(s, dict)]
        dup = {k for k in keys if keys.count(k) > 1}
        if dup:
            errors.append(f"settings: a key twice ({', '.join(sorted(map(str, dup)))})")
        _check_presets(m.get("presets"), {s["key"]: s for s in settings if isinstance(s, dict) and "key" in s}, errors)
    _check_refs("requires", m.get("requires", []), errors, True)
    _check_refs("conflicts", m.get("conflicts", []), errors, False)
    load = m.get("load", {})
    if not isinstance(load, dict) or any(
            k not in ("after", "before") or not isinstance(v, list) or not all(isinstance(x, str) and ID.match(x) for x in v)
            for k, v in load.items()):
        errors.append("load: {after: [ids], before: [ids]}")
    if isinstance(mid, str):
        for name in ("requires", "conflicts"):
            if any(isinstance(r, dict) and r.get("id") == mid for r in m.get(name, []) or []):
                errors.append(f"{name}: the mod names itself")
    return errors


def read_manifest(path: Path) -> dict:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as e:
        raise ManifestError(f"{path}: cannot be read ({e})") from e


def _require(m, root: Path | None, where: str) -> None:
    errors = check_manifest(m, root)
    if errors:
        raise ManifestError(f"{where}:\n  " + "\n  ".join(errors))


def _script_files(folder: Path, m: dict) -> list[Path]:
    """A script mod's files: its scripts, and the .luau, .json and .md files and the license of its
    folder, hidden files and folders and builds left out."""
    out = {folder / PurePosixPath(p) for p in m.get("scripts", [])}

    def walk(at: Path) -> None:
        for p in sorted(at.iterdir()):
            if p.name.startswith(".") or p.is_symlink():
                continue
            if p.is_dir():
                if p.name not in SKIP_DIRS:
                    walk(p)
            elif SCRIPT_FILES.search(p.name) or (at == folder and EXTRAS.match(p.name)):
                out.add(p)

    walk(folder)
    out.discard(folder / MANIFEST)
    return [folder / MANIFEST] + sorted(out)


def _files(folder: Path) -> list[Path]:
    """What goes in the archive: the manifest, the plugin files, the files for people."""
    m = read_manifest(folder / MANIFEST)
    if m.get("type") == "script":
        return _script_files(folder, m)
    out = [folder / MANIFEST]
    for path in (m.get("plugin") or {}).values():
        out.append(folder / PurePosixPath(path))
    out += sorted(p for p in folder.iterdir() if p.is_file() and EXTRAS.match(p.name))
    return out


def archive_name(m: dict, platforms: list[str] | None = None) -> str:
    platforms = sorted((m.get("plugin") or {}).keys()) if platforms is None else platforms
    suffix = f"-{platforms[0]}" if len(platforms) == 1 else ""
    return f"{m['id']}-{m['version']}{suffix}.zip"


def _write_zip(out: Path, entries: dict[str, bytes]) -> None:
    out.parent.mkdir(parents=True, exist_ok=True)
    tmp = out.with_name(out.name + ".part")
    with zipfile.ZipFile(tmp, "w", zipfile.ZIP_DEFLATED) as z:
        for name in sorted(entries, key=lambda n: (n != MANIFEST, n)):
            info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            # Libraries keep their execute bit on the systems that read it.
            info.external_attr = (0o755 if name.startswith("lib/") else 0o644) << 16
            z.writestr(info, entries[name])
    tmp.replace(out)


def package(folder: Path, out_dir: Path) -> Path:
    """The archive of a mod's folder (as openrac_add_mod lays it out), in out_dir."""
    m = read_manifest(folder / MANIFEST)
    _require(m, folder, str(folder / MANIFEST))
    entries = {}
    for f in _files(folder):
        entries[f.relative_to(folder).as_posix()] = f.read_bytes()
    out = out_dir / archive_name(m)
    _write_zip(out, entries)
    return out


def _read_archive(path: Path) -> tuple[dict, dict[str, bytes]]:
    with zipfile.ZipFile(path) as z:
        names = [n for n in z.namelist() if not n.endswith("/")]
        if MANIFEST not in names:
            raise ManifestError(f"{path}: no {MANIFEST} at its root")
        entries = {n: z.read(n) for n in names}
    try:
        m = json.loads(entries[MANIFEST].decode("utf-8"))
    except ValueError as e:
        raise ManifestError(f"{path}: {MANIFEST} cannot be read ({e})") from e
    _require(m, None, f"{path}: {MANIFEST}")
    for platform, p in (m.get("plugin") or {}).items():
        if p not in entries:
            raise ManifestError(f"{path}: plugin {platform}: {p} is not in the archive")
    return m, entries


def merge(archives: list[Path], out: Path) -> Path:
    """One archive with every platform of `archives`, whose manifests must agree but for plugin."""
    if not archives:
        raise ManifestError("--merge: no archives")
    merged: dict | None = None
    entries: dict[str, bytes] = {}
    plugin: dict[str, str] = {}
    for path in archives:
        m, files = _read_archive(path)
        rest = {k: v for k, v in m.items() if k != "plugin"}
        if merged is None:
            merged = rest
        elif rest != merged:
            diff = sorted(k for k in set(rest) | set(merged) if rest.get(k) != merged.get(k))
            raise ManifestError(f"{path}: its manifest differs from the first's ({', '.join(diff)})")
        for platform, p in (m.get("plugin") or {}).items():
            if platform in plugin:
                raise ManifestError(f"{path}: {platform} is in two archives")
            plugin[platform] = p
        for name, data in files.items():
            if name == MANIFEST:
                continue
            if name in entries and entries[name] != data:
                raise ManifestError(f"{path}: {name} differs from an earlier archive's")
            entries[name] = data
    assert merged is not None
    if plugin:
        merged["plugin"] = {k: plugin[k] for k in PLATFORMS if k in plugin}
    _require(merged, None, f"{out}: the merged manifest")
    entries[MANIFEST] = (json.dumps(merged, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
    _write_zip(out, entries)
    return out


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("inputs", nargs="+", type=Path,
                    help="a mod's folder (or several), or with --merge its archives, or with --check manifests")
    ap.add_argument("-o", "--output", type=Path,
                    help="the folder the archives go in (default dist), or with --merge the archive")
    mode = ap.add_mutually_exclusive_group()
    mode.add_argument("--merge", action="store_true", help="join archives of several platforms into one")
    mode.add_argument("--check", action="store_true", help="check manifests, write nothing")
    a = ap.parse_args(argv)
    try:
        if a.check:
            for p in a.inputs:
                path = p / MANIFEST if p.is_dir() else p
                m = read_manifest(path)
                # A script mod's scripts are its sources, so they are there to check; a native mod's
                # library is not built yet.
                _require(m, path.parent if isinstance(m, dict) and m.get("type") == "script" else None, str(path))
                print(f"{path}: ok")
        elif a.merge:
            out = a.output
            if out is None:
                m, _ = _read_archive(a.inputs[0])
                out = Path("dist") / archive_name(m, [])
            print(merge(a.inputs, out))
        else:
            for folder in a.inputs:
                print(package(folder, a.output or Path("dist")))
    except ManifestError as e:
        print(f"package_mod: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
