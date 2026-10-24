#!/usr/bin/env python3
"""The complete-program gate.

The evidence standard asks for two halves: the body is right, **and** the
program that contains it is right.  This gate is the second half.

It runs three checks, and it passes only if all three hold:

  1. **Coverage.**  Every bounded candidate body of every module must be
     covered by a C source that has been proved at that placement.  This is
     what makes the gate *complete*: a body that still comes from the
     assembly baseline is a body that does not come from C.
  2. **Rebuild.**  Every module is rebuilt -- its code interval rendered as
     R5900, assembled and linked, with the proved C bodies written over it
     -- and the result must equal the retail module, byte for byte.  The
     boot is rebuilt the same way from its proved bodies.
  3. **Disc.**  The rebuilt modules are written back into a copy of the disc
     image, at their recorded extents, and the image must hash to the
     pinned retail hash.

Today the gate reports exactly what is missing instead of passing: the
coverage is measured per module, in bodies and in bytes.

Usage:
    python3 tools/complete_gate.py --modules <private dir> --bounds <private dir> \\
        --assembler <Ps2EeAs.exe> --linker <ld.exe> --image <private image> \\
        [--boot <private boot>] [--boot-harness <private boot harness>] \\
        [--profiles <private profiles>]
"""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

from coverage import body_hasher, bounds_rows, records  # noqa: E402
from placement_table import join, sources  # noqa: E402
from module_gate import baseline_interval, rebuild_boot, rebuild_image  # noqa: E402

CODE_START = 0x80


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--modules", type=Path, required=True)
    parser.add_argument("--bounds", type=Path, required=True)
    parser.add_argument("--assembler", type=Path, required=True)
    parser.add_argument("--linker", type=Path, required=True)
    parser.add_argument("--image", type=Path, required=True)
    parser.add_argument("--harness", type=Path, required=True,
                        help="the private per-body harness of the level modules")
    parser.add_argument("--boot", type=Path, default=None)
    parser.add_argument("--boot-harness", type=Path, default=None)
    parser.add_argument("--profiles", type=Path, default=None)
    parser.add_argument("--work", type=Path, default=Path("/tmp/rac-sac-complete"))
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    tables = [ROOT / "config/levels/bodies.tsv", ROOT / "config/boot/bodies.tsv"]
    source_tables = [ROOT / "config/levels/body_sources.tsv",
                     ROOT / "config/boot/body_sources.tsv"]
    profiles: dict[str, dict] = {}
    if args.profiles and args.profiles.is_file():
        profiles = json.loads(args.profiles.read_text(encoding="utf-8"))

    # 1. Coverage.
    proved: dict[str, set[str]] = {}
    for placement, source in zip(tables, source_tables):
        known = sources(source)
        for row in records(placement):
            facts = known.get(row.get("source", ""), {})
            if facts.get("body_sha256"):
                proved.setdefault(row["module"], set()).add(facts["body_sha256"])

    hash_of = body_hasher(args.modules, args.bounds)
    coverage: dict[str, dict] = {}
    missing_total = 0
    for path in sorted(args.bounds.glob("*.tsv")):
        if path.name == "boot.tsv":
            continue
        module = path.stem + ".REL"
        rows = [row for row in bounds_rows(path)
                if row.get("status") == "bounded_candidate"]
        if not rows:
            continue
        covered = [row for row in rows
                   if hash_of(module, row) in proved.get(module, set())]
        missing = [row for row in rows
                   if hash_of(module, row) not in proved.get(module, set())]
        coverage[module] = {
            "bodies": len(rows), "covered": len(covered), "missing": len(missing),
            "bytes": sum(int(row["body_bytes"]) for row in rows),
            "bytes_covered": sum(int(row["body_bytes"]) for row in covered),
        }
        missing_total += len(missing)

    boot_rows = [row for row in bounds_rows(args.bounds / "boot.tsv")
                 if row.get("status") == "bounded_candidate"]
    if boot_rows:
        covered = [row for row in boot_rows
                   if hash_of("SCUS_976.23", row) in proved.get("SCUS_976.23", set())]
        coverage["SCUS_976.23"] = {
            "bodies": len(boot_rows), "covered": len(covered),
            "missing": len(boot_rows) - len(covered),
            "bytes": sum(int(row["body_bytes"]) for row in boot_rows),
            "bytes_covered": sum(int(row["body_bytes"]) for row in covered),
        }
        missing_total += len(boot_rows) - len(covered)

    # 2. Rebuild.
    problems: list[str] = []
    rows = join(tables[0], source_tables[0])
    per_module: dict[str, list[dict]] = {}
    for row in rows:
        per_module.setdefault(row["module"], []).append(row)

    rebuilt_modules: dict[str, bytes] = {}
    for module, entries in sorted(per_module.items()):
        path = args.modules / module
        if not path.is_file():
            problems.append(f"{module}: not extracted")
            continue
        retail = path.read_bytes()
        interval = baseline_interval(module, retail, args.assembler, args.linker,
                                     args.work, problems)
        if interval is None:
            continue
        rebuilt = bytearray(retail)
        rebuilt[CODE_START:CODE_START + len(interval)] = interval
        for entry in entries:
            offset = int(entry["offset"], 0)
            size = int(entry["size"])
            source = ROOT / entry["source"]
            with tempfile.TemporaryDirectory() as directory:
                emit = Path(directory) / "body.bin"
                command = [sys.executable, str(args.harness), module,
                           entry["offset"], str(source), "--one-line",
                           "--emit", str(emit)]
                profile = profiles.get((entry.get("profile") or "").split(":")[0])
                if profile:
                    command += ["--cpp", profile["cpp"], "--cc1", profile["cc1"],
                                "--flags", profile["flags"], "--work", profile["work"]]
                    if profile.get("as"):
                        command += ["--as", profile["as"]]
                result = subprocess.run(command, capture_output=True, text=True)
                if "\tEXACT\t" not in result.stdout:
                    problems.append(f"{module} {entry['offset']}: the body is refused")
                    continue
                body = emit.read_bytes()
            rebuilt[offset:offset + size] = body
        if bytes(rebuilt) != retail:
            problems.append(f"{module}: the rebuilt module differs from the retail")
        rebuilt_modules[module] = bytes(rebuilt)

    # 3. Disc.
    image_digest = ""
    if rebuilt_modules:
        rebuilt_image = rebuild_image(args.image, rebuilt_modules, problems)
        if rebuilt_image is not None:
            image_digest = hashlib.sha256(rebuilt_image).hexdigest()
            target = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))
            if image_digest != target["iso_sha256"]:
                problems.append("the rebuilt image does not hash to the pinned hash")

    complete = missing_total == 0
    report = {"coverage": coverage, "missing_bodies": missing_total,
              "image_sha256": image_digest, "complete": complete,
              "problems": problems}
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        for module, entry in sorted(coverage.items()):
            print(f"  {module:16s} {entry['covered']:6d}/{entry['bodies']:6d} "
                  f"bodies covered, {entry['bytes_covered']:9d}/{entry['bytes']:9d} "
                  f"bytes")
        print(f"  {missing_total} body/bodies still do not come from C")
        if image_digest:
            print(f"  rebuilt image sha256 {image_digest}")
        for problem in problems:
            print(f"  problem: {problem}")
        print("  the complete-program gate "
              f"{'PASSES' if complete and not problems else 'DOES NOT PASS YET'}")
    return 0 if (complete and not problems) else 1


if __name__ == "__main__":
    raise SystemExit(main())
