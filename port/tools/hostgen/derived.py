#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The names a decompilation's own build derives, kept beside the game's configuration.

Going Commando's C carries names whose value its build reads from the retail bytes: the
ones from rac1-decomp that state rac1's address (`r1_func_0011DB98`, `r1_D_00130420`) and
the family-qualified ones (`F<hash>_FUN_<addr>`). The build writes them to
`build/boot/placed.json` (`"values"`); hostgen needs them (as aliases and as symbols), or the
functions that use them become stubs. hostgen.json's `"derived"` says where they are:

    "derived": {"build": "build/boot/placed.json", "table": "derived_names.json"}

`build` is the decompilation's own table (relative to it), read when that checkout has been
built; otherwise `table`, a copy beside hostgen.json committed with the port (only names and
addresses, the same kind of data as a decompilation's symbol_addrs.txt). With neither,
hostgen stops instead of writing a program whose functions are stubs.

    python3 port/tools/hostgen/derived.py --game port/game/rac2-ntsc/hostgen.json \\
        --source <a built rac2-decomp checkout>        # write the table again
    python3 port/tools/hostgen/derived.py --game port/game/rac2-ntsc/hostgen.json \\
        --source games/rac2/ntsc --check               # does it cover what the C uses?

The table also lists the names the C uses that the build derived no value for (in
functions its build leaves as assembly), so a check, and hostgen, can tell a table that
falls behind the C (a sync without regenerating it) from those.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

# The names whose values only the decompilation's build knows.
DERIVED_NAME = re.compile(r"\b(?:r1_(?:func|D)_(?:L\d\d_)?[0-9A-Fa-f]{8}|F[0-9a-f]{8}_(?:FUN|func|D)_[0-9A-Fa-f]{8})\b")


class Missing(Exception):
    """hostgen.json names derived values, and neither table is there."""


def resolve(source: Path, config_dir: Path, cfg: dict) -> Path | None:
    """The table of derived values to read: the decompilation's build's, else the committed
    copy beside the configuration; None when the game has none ("derived" absent)."""
    d = cfg.get("derived")
    if not d:
        return None
    build = source / d["build"] if d.get("build") else None
    if build is not None and build.is_file():
        return build
    table = config_dir / d["table"] if d.get("table") else None
    if table is not None and table.is_file():
        return table
    raise Missing(
        f"{cfg.get('id', config_dir.name)}: no table of the names the decompilation's build derives: "
        f"neither {build} (a build of the decompilation) nor {table} (the port's copy) is there. "
        f"Without it the functions that use those names become stubs. Regenerate the copy with "
        f"port/tools/hostgen/derived.py --game {config_dir / 'hostgen.json'} --source <a built checkout>.")


def used_names(source: Path, cfg: dict) -> set[str]:
    """The derived names the decompilation's C and headers use."""
    out: set[str] = set()
    for d in sorted(set(cfg.get("sources", ["src"]) + cfg.get("includes", ["include", "src"]))):
        root = source / d
        if not root.is_dir():
            continue
        for p in root.rglob("*"):
            if p.suffix in (".c", ".h") and p.is_file():
                out.update(DERIVED_NAME.findall(p.read_text(encoding="utf-8", errors="replace")))
    return out


def uncovered(source: Path, cfg: dict, table: dict) -> list[str]:
    """The derived names the C uses that TABLE neither values nor lists as unresolved."""
    known = set(table.get("values", {})) | set(table.get("unresolved", []))
    return sorted(used_names(source, cfg) - known)


def source_commit(source: Path) -> str | None:
    try:
        r = subprocess.run(["git", "rev-parse", "HEAD"], cwd=source, capture_output=True, text=True, timeout=15)
    except (OSError, subprocess.TimeoutExpired):
        return None
    if r.returncode != 0:
        return None
    return r.stdout.strip() or None


def make_table(source: Path, cfg: dict, build: dict, commit: str | None) -> dict:
    """The committed form of a build's table: its values, where they come from, and the names
    the C uses that it has no value for."""
    values = {k: v for k, v in sorted(build.get("values", {}).items()) if isinstance(v, str)}
    return {
        "about": "Names and the addresses the decompilation's own build derived for them from the "
                 "retail bytes. Written by port/tools/hostgen/derived.py; do not edit.",
        "source": {"commit": commit, "program": build.get("program"), "table": cfg["derived"]["build"]},
        "values": values,
        "unresolved": sorted(used_names(source, cfg) - set(values)),
    }


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--game", type=Path, required=True, help="the game's hostgen.json")
    ap.add_argument("--source", type=Path, required=True, help="the decompilation (built, unless --check)")
    ap.add_argument("--check", action="store_true",
                    help="only check that the committed table covers every derived name the C uses")
    args = ap.parse_args(argv)
    cfg = json.loads(args.game.read_text())
    d = cfg.get("derived")
    if not d or not d.get("table") or not d.get("build"):
        print(f"derived: {args.game} has no \"derived\" with \"build\" and \"table\"", file=sys.stderr)
        return 2
    table_path = args.game.parent / d["table"]
    source = args.source.resolve()
    if args.check:
        if not table_path.is_file():
            print(f"derived: {table_path} is missing", file=sys.stderr)
            return 1
        missing = uncovered(source, cfg, json.loads(table_path.read_text()))
        if missing:
            print(f"derived: {table_path} has no value for {len(missing)} names the C uses "
                  f"({', '.join(missing[:8])}{', ...' if len(missing) > 8 else ''}); regenerate it", file=sys.stderr)
            return 1
        print(f"derived: {table_path} covers the C")
        return 0
    build_path = source / d["build"]
    if not build_path.is_file():
        print(f"derived: {build_path} is missing: build the decompilation first", file=sys.stderr)
        return 1
    table = make_table(source, cfg, json.loads(build_path.read_text()), source_commit(source))
    table_path.write_text(json.dumps(table, indent=1) + "\n")
    print(f"derived: {table_path}: {len(table['values'])} values, {len(table['unresolved'])} names unresolved "
          f"(from {build_path})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
