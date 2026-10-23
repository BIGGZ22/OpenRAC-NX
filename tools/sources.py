#!/usr/bin/env python3
"""
Imports and syncs the sister projects whose trees live under games/
(docs/SOURCES.md).

Each version in games/<game>/game.json names its source project: the
repository, branch and local checkout, the commit last brought in, and
the paths left out. An import copies one commit's tree (git archive),
never a working tree, so nothing uncommitted in a checkout comes along.
A sync applies the project's diff since the recorded commit, so edits
made here (docs/SOURCES.md lists them) survive, and a hunk that touches
one is left as a .rej file to resolve by hand.

  python3 tools/sources.py status                    how far each checkout is past its recorded commit
  python3 tools/sources.py import GAME/VERSION [REV] copy REV (default: the checkout's HEAD) into an empty directory
  python3 tools/sources.py sync GAME/VERSION [REV]   apply the diff from the recorded commit to REV

A checkout is looked for at the path in game.json (~ is your home), or
in $OPENRAC_SOURCE_<GAME>_<VERSION> (e.g. OPENRAC_SOURCE_RAC1_NTSC).
After an import or sync, review the result and commit it together with
the updated game.json.
"""
import io
import json
import os
import subprocess
import sys
import tarfile
import time
from pathlib import Path

from openrac_common import games, paths, proc

ROOT = paths.repo_root()


def manifests() -> dict[str, tuple[Path, dict]]:
    """'game/version' -> (its game.json, the whole manifest), for every version with a source.
    The versions of one game share its manifest, so record() writes back all of it."""
    return {f"{game['id']}/{version}": (games.manifest_path(game["id"], ROOT), game)
            for game, version, info in games.versions(ROOT) if info.get("source")}


def checkout(key: str, source: dict) -> Path:
    env = "OPENRAC_SOURCE_" + key.replace("/", "_").upper()
    return Path(os.environ.get(env) or os.path.expanduser(source["checkout"]))


def version_dir(game: dict, version: str) -> Path:
    """Where a version's tree lives: its "dir" in game.json (rac1's PAL tree is games/rac1 itself,
    the merged game), else games/<game>/<version>."""
    return ROOT / games.version_dir(game, version)


def excluded(path: str, exclude: list[str]) -> bool:
    return any(path == e or path.startswith(e.rstrip("/") + "/") for e in exclude)


def do_import(key: str, rev: str | None) -> None:
    path, game = manifests()[key]
    version = key.split("/")[1]
    source = game["versions"][version]["source"]
    repo, dest = checkout(key, source), version_dir(game, version)
    if dest.exists() and any(p.name != ".gitkeep" for p in dest.iterdir()):
        sys.exit(f"{dest.relative_to(ROOT)} is not empty: use sync")
    commit = proc.git(repo, "rev-parse", rev or "HEAD")
    data = proc.git(repo, "archive", "--format=tar", commit, binary=True)
    exclude = source.get("exclude", [])
    count = 0
    with tarfile.open(fileobj=io.BytesIO(data)) as tar:
        members = [m for m in tar.getmembers() if not excluded(m.name, exclude)]
        dest.mkdir(parents=True, exist_ok=True)
        tar.extractall(dest, members=members, filter="data")
        count = sum(m.isfile() for m in members)
    record(path, game, version, commit, repo)
    print(f"{key}: imported {count} files from {source['name']} at {commit[:10]}")


def do_sync(key: str, rev: str | None) -> None:
    path, game = manifests()[key]
    version = key.split("/")[1]
    source = game["versions"][version]["source"]
    repo, dest = checkout(key, source), version_dir(game, version)
    commit = proc.git(repo, "rev-parse", rev or "HEAD")
    if commit == source["commit"]:
        print(f"{key}: already at {commit[:10]}")
        return
    specs = [f":(exclude){e}" for e in source.get("exclude", [])]
    diff = proc.git(repo, "diff", "--binary", source["commit"], commit, "--", ".", *specs, binary=True)
    rejects_before = set(dest.rglob("*.rej"))
    # git takes the directory with /: on Windows, games\rac1 made every path of the patch "invalid".
    apply = subprocess.run(["git", "apply", "--reject", f"--directory={dest.relative_to(ROOT).as_posix()}"],
                           input=diff, cwd=ROOT, capture_output=True)
    sys.stderr.write(apply.stderr.decode())
    if apply.returncode and not set(dest.rglob("*.rej")) - rejects_before:
        # Nothing applied (not some hunks rejected): the recorded commit stays, or the next sync
        # would skip what this one never brought in.
        raise SystemExit(f"{key}: the diff did not apply; {source['commit'][:10]} stays recorded")
    old = source["commit"]
    record(path, game, version, commit, repo)
    state = "with rejected hunks (*.rej) to resolve" if apply.returncode else "cleanly"
    print(f"{key}: applied {old[:10]}..{commit[:10]} {state}")


def record(path: Path, game: dict, version: str, commit: str, repo: Path) -> None:
    source = game["versions"][version]["source"]
    source["commit"] = commit
    source["date"] = proc.git(repo, "log", "-1", "--format=%cs", commit)
    source["imported"] = time.strftime("%Y-%m-%d")
    path.write_text(json.dumps(game, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def do_status() -> None:
    for key, (_, game) in manifests().items():
        source = game["versions"][key.split("/")[1]]["source"]
        repo = checkout(key, source)
        if not (repo / ".git").exists():
            print(f"{key}: no checkout at {repo}")
            continue
        ahead = proc.git(repo, "rev-list", "--count", f"{source['commit']}..HEAD")
        print(f"{key}: {source['name']} at {source['commit'][:10]} ({source['date']}); "
              f"checkout {repo} is {ahead} commits past it")


def main() -> None:
    args = sys.argv[1:]
    if args[:1] == ["status"]:
        do_status()
    elif args[:1] in (["import"], ["sync"]) and len(args) in (2, 3):
        (do_import if args[0] == "import" else do_sync)(args[1], args[2] if len(args) == 3 else None)
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
