#!/usr/bin/env python3
"""
The functions still to match, to pick work by size (docs/AGENT_WORKFLOW.md).

  python3 tools/todo.py                    # a summary by size tier
  python3 tools/todo.py --max 400          # functions of at most 400 bytes
  python3 tools/todo.py --min 401 --max 1000 --limit 40
  python3 tools/todo.py --min 1001 --kind shared

Lists the functions that are INCLUDE_ASM lines in src/, with their size, how many programs
have them (a level function shared by 27 levels is worth 27 placements) and their file. The
ones with the most places come first, then the smallest. Needs no game data.
"""
import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import levels  # noqa: E402
import progress  # noqa: E402

TIERS = (("small (Haiku)", 0, 400), ("medium (Sonnet)", 401, 1000), ("large (Opus)", 1001, 1 << 30))


def todo() -> list[tuple[str, int, int, str, str]]:
    """(name, size, places, kind, file) of every function without C."""
    info = {n: (s, 1, "boot") for n, _, s in levels.read_boot_functions()}
    places = {}
    for name, kind, size, _, where in levels.read_catalogue():
        if kind == "boot":
            places[name] = len(where)
        else:
            info[name] = (size, len(where), kind)
    out = []
    for path, fns in progress.source_functions().items():
        for name, has_c in fns:
            if not has_c:
                size, n, kind = info[name]
                out.append((name, size, n + places.get(name, 0), kind, path))
    return out


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--min", type=int, default=0, help="least size in bytes")
    ap.add_argument("--max", type=int, default=None, help="greatest size in bytes")
    ap.add_argument("--kind", choices=("boot", "shared", "level"), help="only this kind")
    ap.add_argument("--limit", type=int, default=50, help="how many to list (default 50)")
    args = ap.parse_args()
    rows = todo()
    if args.max is None and args.min == 0 and not args.kind:
        print(f"{len(rows)} functions without C, {sum(r[1] for r in rows):,} bytes")
        for label, lo, hi in TIERS:
            tier = [r for r in rows if lo <= r[1] <= hi]
            print(f"  {label:<16} {len(tier):>5} functions  {sum(r[1] for r in tier):>10,} bytes")
        return
    hi = args.max if args.max is not None else 1 << 30
    rows = [r for r in rows if args.min <= r[1] <= hi and (not args.kind or r[3] == args.kind)]
    rows.sort(key=lambda r: (-r[2], r[1], r[0]))
    print(f"{len(rows)} functions; the first {min(args.limit, len(rows))}:")
    print("name                    size  places  kind    file")
    for name, size, n, kind, path in rows[:args.limit]:
        print(f"{name:<22} {size:>5}  {n:>6}  {kind:<6}  {path}")


if __name__ == "__main__":
    main()
