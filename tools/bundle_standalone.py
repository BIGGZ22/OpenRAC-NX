#!/usr/bin/env python3
"""
Puts together the copy of OpenRAC an installed launcher carries, so that a
player needs nothing but the image of their own disc: no checkout, no Python,
no compiler.

  python3 tools/bundle_standalone.py OUT [--game-build DIR] [--python DIR]
  python3 tools/bundle_standalone.py OUT --android [--python DIR]

OUT (launcher/src-tauri/resources/openrac for an installer) gets:

  STANDALONE                 what this is and what it was made from
  tools/, editor/            the set-up and the level tools, as they are here
                             (tools/openrac_common with them: they import it)
  port/enhanced/tools/probes the probe baker's Python half (bake_all.py)
  port/enhanced/lighting/profiles  per-level settings included by native level export
  games/*/game.json          the catalogue the launcher lists
  progress/summary.json      how far each decompilation is
  launcher/actions.json      the player's actions only, writing to the
                             launcher's data folder (a carried copy is read-only)
  port/build/release/        the game built for this system (--game-build:
                             the folder tools/build_port.py built into), with
                             the libraries beside it; on macOS each game is its
                             application bundle, apps/openrac-<id>/<title>.app.
                             A Linux program goes without its debug info
                             (its symbols stay, for a crash report's backtrace):
                             three quarters of its size, which every installer
                             would compress and every player download
  python/                    a Python to run the tools (--python: an unpacked
                             python-build-standalone "install_only" archive)

--android makes the copy the Android launcher carries in its APK
(tools/build_android.py; docs/port/ANDROID.md). The games and the Python
program are libraries of the APK there, which the launcher links into place
when it unpacks the copy (OpenracSetup.kt): the copy has no port/build/, and
--python names a PYTHONHOME (lib/python3.N with its lib-dynload), not a
python-build-standalone archive.

The launcher finds it in its resources (launcher/core/src/detect.rs,
bundled_root) and uses it when there is no checkout. Without --game-build or
--python the copy is made without them, which is enough to try the launcher's
side against it (OPENRAC_ROOT=OUT); an installer needs both.
"""
import argparse
import json
import shutil
import sys
from pathlib import Path

from openrac_common import files, paths, proc

ROOT = paths.repo_root()

# What a player's launcher offers for a version, from launcher/actions.json. The rest (the
# decompilation's builds and checks, building the port, the tools' tests) needs a checkout.
PLAY = ("port-levels", "port-levels-check", "probe-bake", "play")
# Mods: the level editor (Godot projects of the disc's levels, packed back into the game's data),
# models and textures, packs, and playing with them. In a checkout they work in its assets/; the
# carried copy is read-only, so they work in the data folder instead (MOD_FOLDERS).
MODS = ("editor-extract", "editor-preview", "editor-import", "editor-open", "editor-pack",
        "assets-export", "assets-import", "overrides-add", "overrides-stack", "editor-show-packs",
        "editor-hide-packs", "play-edited")
KEEP = PLAY + MODS
# Where a checkout's folders for version <game>/<version> go in the carried copy, longest first:
# the levels the port draws, the level editor's projects, and the packed levels.
MOD_FOLDERS = (("{root}/assets/port/", "port"), ("assets/port/", "port"), ("assets/levels/", "levels"),
               ("{root}/assets/packed/", "packed"), ("assets/packed/", "packed"))
# Where the carried copy writes instead of the checkout's assets/port/<game>/<version>. {data} is the
# game's, shared by its versions: each version's levels are <data>/port/<version>, which is also where
# the program looks without --levels.
LEVELS_ROOT = "{data}/port"
# The programs carried when the build has them; the first is required.
GAMES = ("openrac-rac1-pal", "openrac-rac1-ntsc")
# The tools carried beside them when the build has them: the enhanced lighting's probe baker
# (tools/build_port.py --probe-baker), Embree built into it.
TOOLS = ("openrac-probe-bake",)
# Its Python half (bake_all.py and what it imports), without the tests.
PROBE_TOOLS = "port/enhanced/tools/probes"
# Authored settings travel with the exporter, rather than depending on a source checkout.
PROBE_PROFILES = "port/enhanced/lighting/profiles"


def levels_folder(key: str) -> str:
    """The data folder's place for version KEY's levels: <data>/port/<version>."""
    return f"{LEVELS_ROOT}/{key.split('/')[-1]}"


def in_data_folder(text: str, key: str) -> str:
    """TEXT with version KEY's checkout folders (MOD_FOLDERS) moved into the data folder:
    assets/levels/rac1/pal/level_{level} becomes {data}/levels/pal/level_{level}."""
    version = key.split("/")[-1]
    for prefix, kind in MOD_FOLDERS:
        text = text.replace(f"{prefix}{key}", f"{{data}}/{kind}/{version}")
    return text


def moved(value, key: str):
    """VALUE (an action's field: a string, a list or a dict of them) with in_data_folder applied."""
    if isinstance(value, str):
        return in_data_folder(value, key)
    if isinstance(value, list):
        return [moved(v, key) for v in value]
    if isinstance(value, dict):
        return {k: moved(v, key) for k, v in value.items()}
    return value


def point_at(args: list[str], levels: str) -> list[str]:
    """ARGS with --levels naming LEVELS."""
    if "--levels" not in args:
        return list(args)
    at = args.index("--levels")
    return args[:at + 1] + [levels] + args[at + 2:]


def player_actions() -> dict:
    """launcher/actions.json cut down to what works from a carried copy."""
    source = json.loads((ROOT / "launcher/actions.json").read_text())
    out = {key: value for key, value in source.items() if key not in ("repository", "versions")}
    out["repository"] = []
    out["versions"] = {}
    for key, actions in source["versions"].items():
        levels = levels_folder(key)
        kept = []
        for action in actions:
            if action["id"] not in KEEP:
                continue
            action = dict(action)
            if action.get("state") != "planned":
                args = action.get("args", [])
                if action["id"] == "port-levels" and not key.startswith("rac1/"):
                    # A later game's levels, read for packs (early access): no worlds behind the
                    # menus, and the first level's manifest is its own.
                    action["args"] = args[:3] + [levels, "--anywhere", "--replace"]
                    action["artifact"] = moved(action["artifact"], key)
                    action["output"] = levels
                elif action["id"] == "port-levels":
                    # editor/extract.py port DISC OUT: OUT is the data folder's, wherever it is.
                    action["args"] = args[:3] + [levels, "--anywhere", "--replace"]
                    action["artifact"] = f"{levels}/level_99/manifest.json"
                    action["output"] = levels
                    action["description"] = ("Reads the levels from your disc into the form the game draws: the worlds "
                                             "behind the menus and all 19 levels. About half a minute and 1 GB, once.")
                elif action["id"] == "probe-bake":
                    # From the data folder's levels into the data folder's bakes ({data}/probes/<v>).
                    at = args.index("--levels-dir")
                    action["args"] = args[:at + 1] + [levels] + args[at + 2:]
                    action["needs"] = [{"path": f"{levels}/verified-3.json", "from": need["from"]}
                                       for need in action.get("needs", [])]
                elif action["id"] == "port-levels-check":
                    action["args"] = point_at(args, levels)
                    action["makes"] = f"{levels}/verified-3.json"
                    action["needs"] = [{"path": f"{levels}/level_99/manifest.json", "from": need["from"]}
                                       for need in action.get("needs", [])]
                elif action["id"] in MODS:
                    for field in ("args", "artifact", "needs", "makes", "output"):
                        if field in action:
                            action[field] = moved(action[field], key)
                    if args[:2] in (["editor/extract.py", "godot"], ["editor/extract.py", "pack"]):
                        # The editor writes only under a checkout's assets/ unless told otherwise.
                        action["args"] = action["args"] + ["--anywhere"]
                    if action["id"] == "play-edited":
                        # Started where Play is (below), for the same reason.
                        action["cwd"] = "{data}"
                else:
                    action["args"] = point_at(args, levels)
                    action["needs"] = [{"path": f"{levels}/verified-3.json", "from": need["from"]}
                                       for need in action.get("needs", [])]
                    # The game is started in the data folder: what it writes beside itself (a
                    # screenshot) has a place there, and the copy has no folder for each version.
                    action["cwd"] = "{data}"
                action.pop("todo", None)
            kept.append(action)
        out["versions"][key] = kept
    return out


def strip_debug(path: Path) -> bool:
    """Drops the debug info of an ELF program (Linux), keeping its symbols; whether it did. A
    program of another system, or no strip tool on the PATH, leaves the file as it is."""
    with path.open("rb") as f:
        if f.read(4) != b"\x7fELF":
            return False
    tool = shutil.which("llvm-strip") or shutil.which("strip")
    if tool is None:
        print(f"no strip tool: {path.name} keeps its debug info", file=sys.stderr)
        return False
    proc.run([tool, "--strip-debug", path])
    return True


def copy_games(build: Path, dest: Path) -> list[str]:
    """The game programs in BUILD, copied to DEST; their names. On macOS a game is an application
    bundle named by its title in apps/openrac-<id>/ (port/cmake/MacApp.cmake), carried whole; the
    build folder's link to its program is not."""
    carried = []
    for game in GAMES + TOOLS:
        apps = build / "apps" / game
        if any(apps.glob("*.app")):
            shutil.copytree(apps, dest / "apps" / game, symlinks=True, dirs_exist_ok=True)
            carried.append(f"apps/{game}")
            continue
        for path in sorted(build.glob(f"{game}*")):
            if path.is_file() and path.suffix in ("", ".exe"):
                shutil.copy2(path, dest / path.name)
                strip_debug(dest / path.name)
                carried.append(path.name)
    return carried


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("out", type=Path)
    parser.add_argument("--game-build", type=Path, help="the build folder holding the game (port/build/release)")
    parser.add_argument("--python", type=Path, help="an unpacked standalone Python (its python/ folder)")
    parser.add_argument("--android", action="store_true", help="the Android launcher's copy (tools/build_android.py)")
    args = parser.parse_args()
    if args.android and args.game_build:
        sys.exit("--android carries no game build: the games are the APK's libraries")
    out = args.out.resolve()
    # The folder in the launcher's resources is kept in git with a note in it: the note stays.
    if out.exists() and any(p.name != "README.txt" for p in out.iterdir()):
        if not (out / "STANDALONE").is_file():
            sys.exit(f"{out} holds something else; remove it or name another folder")
        for path in out.iterdir():
            if path.name != "README.txt":
                shutil.rmtree(path) if path.is_dir() else path.unlink()
    out.mkdir(parents=True, exist_ok=True)

    # The tools, without their tests; the catalogue; the progress the library shows.
    for name in ("openrac.py", "extractor.py"):
        (out / "tools").mkdir(exist_ok=True)
        shutil.copy2(ROOT / "tools" / name, out / "tools" / name)
    # The helpers they import, found from the copy's own tools/ (they find the copy's root by
    # the STANDALONE note written below).
    files.copy_tree(ROOT / "tools/openrac_common", out / "tools/openrac_common", lambda rel: rel.suffix == ".py")
    files.copy_tree(ROOT / "editor", out / "editor",
              lambda rel: not rel.name.startswith("test_") and rel.suffix in (".py", ".tsv", ".gd", ""))
    # Enhanced graphics' baker's Python half; on Android its program is the APK's library
    # (libopenrac-probe-bake.so, tools/build_android.py).
    files.copy_tree(ROOT / PROBE_TOOLS, out / PROBE_TOOLS,
                    lambda rel: not rel.name.startswith("test_") and rel.suffix in (".py", ".json"))
    files.copy_tree(ROOT / PROBE_PROFILES, out / PROBE_PROFILES,
                    lambda rel: rel.name == "probe-bake.json")
    for manifest in sorted((ROOT / "games").glob("*/game.json")):
        dest = out / manifest.relative_to(ROOT)
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(manifest, dest)
    (out / "progress").mkdir(exist_ok=True)
    shutil.copy2(ROOT / "progress/summary.json", out / "progress/summary.json")
    (out / "launcher").mkdir(exist_ok=True)
    (out / "launcher/actions.json").write_text(json.dumps(player_actions(), indent=2) + "\n")
    for name in ("LICENSE", "THIRD_PARTY_NOTICES.md"):
        if (ROOT / name).is_file():
            shutil.copy2(ROOT / name, out / name)

    carried = []
    if args.game_build:
        dest = out / "port/build/release"
        dest.mkdir(parents=True, exist_ok=True)
        carried += copy_games(args.game_build, dest)
        # The libraries the game was linked against as files of their own (SDL3 when it is not
        # built into the program).
        for pattern in ("*.dll", "*.dylib", "*.so", "*.so.*"):
            for path in sorted(args.game_build.glob(pattern)):
                shutil.copy2(path, dest / path.name, follow_symlinks=True)
                carried.append(path.name)
        # The licenses of what the carried tools have built in (Embree in the probe baker).
        if (args.game_build / "licenses").is_dir() and any(name.startswith(TOOLS) for name in carried):
            shutil.copytree(args.game_build / "licenses", dest / "licenses", dirs_exist_ok=True)
            carried.append("licenses")
        if not any(name.split("/")[-1].startswith(GAMES[0]) for name in carried):
            sys.exit(f"{args.game_build} has no {GAMES[0]}: build it first (tools/build_port.py rac1-pal)")
    if args.python and args.android:
        if not any(args.python.glob("lib/python3.*/os.py")):
            sys.exit(f"{args.python} is not a PYTHONHOME for Android (no lib/python3.N/os.py)")
        shutil.copytree(args.python, out / "python", symlinks=True,
                        ignore=shutil.ignore_patterns("__pycache__", "test", "tests", "idlelib", "tkinter", "ensurepip"))
        carried.append("python (Android)")
    elif args.python:
        if not any((args.python / rel).is_file() for rel in ("bin/python3", "python.exe")):
            sys.exit(f"{args.python} is not an unpacked standalone Python (no bin/python3 or python.exe)")
        shutil.copytree(args.python, out / "python", symlinks=True,
                        ignore=shutil.ignore_patterns("__pycache__", "test", "tests", "idlelib", "tkinter", "ensurepip",
                                                      "*.a", "include", "share"))
        carried.append("python")

    commit = proc.git(ROOT, "rev-parse", "HEAD", check=False)
    (out / "STANDALONE").write_text(
        "The copy of OpenRAC an installed launcher carries (tools/bundle_standalone.py).\n"
        f"commit: {commit or 'unknown'}\ncarries: {', '.join(carried) or 'no game, no Python'}\n"
        + ("for: Android (the games and Python are the APK's libraries)\n" if args.android else ""))
    size = sum(p.stat().st_size for p in out.rglob("*") if p.is_file())
    print(f"{out}: {size / 1e6:.0f} MB; carries {', '.join(carried) or 'no game, no Python'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
