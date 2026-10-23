#!/usr/bin/env python3
"""
Files the games share. While each project also lives in its own repository,
a shared file stays in every game that builds with it; shared/files.json
names the copies, and this tool keeps them the same file.

  python3 tools/shared.py check        every group's copies are identical (CI runs this)
  python3 tools/shared.py find         files identical in two or more games that the manifest does not list
  python3 tools/shared.py sync PATH    make the other copies of PATH's group equal to PATH

A group is {"what": ..., "copies": [paths]}: the same file at several paths.
"related" entries name files that began as copies and now differ on purpose,
with the reason; `check` only confirms they still exist. When a sync from a
project's own repository changes one copy, `check` fails: decide whether the
change belongs to every game (`sync` it) or the files have parted ways (move
the group to "related" and say why).
"""
import json
import sys
from collections import defaultdict

from openrac_common import digest, paths, proc

ROOT = paths.repo_root()
MANIFEST = ROOT / "shared/files.json"
MIN_SIZE = 300          # bytes: smaller identical files (stand-in headers, .gitkeep) are not worth tracking


def manifest() -> dict:
    return json.loads(MANIFEST.read_text())


def problems() -> list[str]:
    """What is wrong with the manifest against the tree: a missing copy, or copies that differ."""
    out, data = [], manifest()
    for group in data["groups"]:
        paths = [ROOT / p for p in group["copies"]]
        missing = [p for p in paths if not p.is_file()]
        out += [f"missing: {p.relative_to(ROOT)} ({group['what']})" for p in missing]
        if not missing and len({digest.sha1(p) for p in paths}) > 1:
            out.append(f"copies differ: {', '.join(group['copies'])} ({group['what']})")
    for entry in data.get("related", []):
        out += [f"missing: {p} ({entry['why']})" for p in entry["files"] if not (ROOT / p).is_file()]
    return out


def identical_across_games() -> list[list[str]]:
    """Groups of tracked files with the same content in two or more game versions."""
    tracked = proc.git(ROOT, "ls-files", "games", check=False).split("\n")
    by_content = defaultdict(list)
    for name in tracked:
        path = ROOT / name
        if len(name.split("/")) >= 4 and path.is_file() and path.stat().st_size >= MIN_SIZE:
            by_content[digest.sha1(path)].append(name)
    return sorted(sorted(g) for g in by_content.values() if len({"/".join(n.split("/")[1:3]) for n in g}) > 1)


def find() -> None:
    listed = {tuple(sorted(g["copies"])) for g in manifest()["groups"]}
    fresh = [g for g in identical_across_games() if tuple(g) not in listed]
    for group in fresh:
        print("  " + "\n  ".join(group) + "\n")
    print(f"{len(fresh)} group(s) of identical files not in shared/files.json")


def sync(path: str) -> None:
    for group in manifest()["groups"]:
        if path in group["copies"]:
            for other in group["copies"]:
                if other != path and digest.sha1(ROOT / other) != digest.sha1(ROOT / path):
                    (ROOT / other).write_bytes((ROOT / path).read_bytes())
                    print(f"{other}: now equal to {path}")
            return
    sys.exit(f"{path} is not a copy in shared/files.json")


def main() -> None:
    args = sys.argv[1:]
    if args == ["check"]:
        found = problems()
        print("\n".join(found) if found else f"shared files: {len(manifest()['groups'])} groups, all copies identical")
        sys.exit(1 if found else 0)
    elif args == ["find"]:
        find()
    elif len(args) == 2 and args[0] == "sync":
        sync(args[1])
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
