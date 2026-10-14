#!/usr/bin/env python3
"""
Puts together the copy of OpenRAC an installed launcher carries, so that a
player needs nothing but the image of their own disc: no checkout, no Python,
no compiler.

  python3 tools/bundle_standalone.py OUT [--game-build DIR] [--python DIR]

OUT (launcher/src-tauri/resources/openrac for an installer) gets:

  STANDALONE                 what this is and what it was made from
  tools/, editor/            the set-up and the level tools, as they are here
  games/*/game.json          the catalogue the launcher lists
  progress/summary.json      how far each decompilation is
  launcher/actions.json      the player's actions only, writing to the
                             launcher's data folder (a carried copy is read-only)
  port/build/release/        the game built for this system (--game-build:
                             the folder tools/build_port.py built into), with
                             the libraries beside it
  python/                    a Python to run the tools (--python: an unpacked
                             python-build-standalone "install_only" archive)

The launcher finds it in its resources (launcher/core/src/detect.rs,
bundled_root) and uses it when there is no checkout. Without --game-build or
--python the copy is made without them, which is enough to try the launcher's
side against it (OPENRAC_ROOT=OUT); an installer needs both.
"""
import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# What a player's launcher offers for a version, from launcher/actions.json. The rest (the
# decompilation's builds and checks, the level editor, building the port) needs a checkout.
KEEP = ("port-levels", "play")
# Where the carried copy writes instead of the checkout's assets/: the game's own default.
LEVELS = "{data}/port"
GAMES = ("openrac-rac1-pal",)


def player_actions() -> dict:
    """launcher/actions.json cut down to what works from a carried copy."""
    source = json.loads((ROOT / "launcher/actions.json").read_text())
    out = {key: value for key, value in source.items() if key not in ("repository", "versions")}
    out["repository"] = []
    out["versions"] = {}
    for key, actions in source["versions"].items():
        kept = []
        for action in actions:
            if action["id"] not in KEEP:
                continue
            action = dict(action)
            if action.get("state") != "planned":
                args = action.get("args", [])
                if action["id"] == "port-levels":
                    # editor/extract.py port DISC OUT: OUT is the data folder's, wherever it is.
                    action["args"] = args[:3] + [LEVELS, "--anywhere"]
                    action["artifact"] = f"{LEVELS}/level_99/manifest.json"
                    action["description"] = ("Reads the levels from your disc into the form the game draws: the worlds "
                                             "behind the menus and all 19 levels. About half a minute and 1 GB, once.")
                else:
                    at = args.index("--levels") if "--levels" in args else -1
                    if at >= 0:
                        action["args"] = args[:at] + args[at + 2:]      # the game looks in <data>/port by itself
                    action["needs"] = [{"path": f"{LEVELS}/level_99/manifest.json", "from": need["from"]}
                                       for need in action.get("needs", [])]
                    # The game is started in the data folder: what it writes beside itself (a
                    # screenshot) has a place there, and the copy has no folder for each version.
                    action["cwd"] = "{data}"
                action.pop("todo", None)
            kept.append(action)
        out["versions"][key] = kept
    return out


def copy_tree(source: Path, dest: Path, keep) -> None:
    for path in sorted(source.rglob("*")):
        rel = path.relative_to(source)
        if path.is_file() and "__pycache__" not in rel.parts and keep(rel):
            (dest / rel).parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, dest / rel)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("out", type=Path)
    parser.add_argument("--game-build", type=Path, help="the build folder holding the game (port/build/release)")
    parser.add_argument("--python", type=Path, help="an unpacked standalone Python (its python/ folder)")
    args = parser.parse_args()
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
    copy_tree(ROOT / "editor", out / "editor",
              lambda rel: not rel.name.startswith("test_") and rel.suffix in (".py", ".tsv", ".gd", ""))
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
        for game in GAMES:
            for path in sorted(args.game_build.glob(f"{game}*")):
                if path.is_file() and path.suffix in ("", ".exe", ".pdb") and path.suffix != ".pdb":
                    shutil.copy2(path, dest / path.name)
                    carried.append(path.name)
        # The libraries the game was linked against as files of their own (SDL3 when it is not
        # built into the program).
        for pattern in ("*.dll", "*.dylib", "*.so", "*.so.*"):
            for path in sorted(args.game_build.glob(pattern)):
                shutil.copy2(path, dest / path.name, follow_symlinks=True)
                carried.append(path.name)
        if not any(name.startswith(GAMES[0]) for name in carried):
            sys.exit(f"{args.game_build} has no {GAMES[0]}: build it first (tools/build_port.py rac1-pal)")
    if args.python:
        if not any((args.python / rel).is_file() for rel in ("bin/python3", "python.exe")):
            sys.exit(f"{args.python} is not an unpacked standalone Python (no bin/python3 or python.exe)")
        shutil.copytree(args.python, out / "python", symlinks=True,
                        ignore=shutil.ignore_patterns("__pycache__", "test", "tests", "idlelib", "tkinter", "ensurepip",
                                                      "*.a", "include", "share"))
        carried.append("python")

    commit = subprocess.run(["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    (out / "STANDALONE").write_text(
        "The copy of OpenRAC an installed launcher carries (tools/bundle_standalone.py).\n"
        f"commit: {commit or 'unknown'}\ncarries: {', '.join(carried) or 'no game, no Python'}\n")
    size = sum(p.stat().st_size for p in out.rglob("*") if p.is_file())
    print(f"{out}: {size / 1e6:.0f} MB; carries {', '.join(carried) or 'no game, no Python'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
