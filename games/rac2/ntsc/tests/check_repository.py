#!/usr/bin/env python3
"""Toolchain-free repository checks (no game data needed).

  python3 tests/check_repository.py

1. config/levels/functions.tsv and config/boot/functions.tsv parse: right column count,
   unique names, kinds in {boot, shared, level}, places NN:XXXXXXXX of a known level.
2. Every function of the two catalogues appears in src/**/*.c exactly once, as its C or as an
   INCLUDE_ASM marker (rac1-decomp's layout), and nothing else does.
3. Nothing of the removed system comes back: no gzipped or generated data, no generated sources
   or fragments, no per-level copies, no per-batch or campaign documents, no file over 2.5 MB.

Exits nonzero, listing every problem, on failure.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import levels  # noqa: E402

KINDS = {"boot", "shared", "level"}
PLACE = re.compile(r"^(\d\d):([0-9A-F]{8})$")
NAME = r"(?:func_(?:L[0-9]{2}_)?[0-9A-F]{8}|sceIpuRestartDMA|_sysbitFlush)"
DEFINITION = re.compile(rf"^(?:[A-Za-z_][^;=#]*?\b)?({NAME})\s*\(")    # the type may be on the line before


def check_catalogues(errors: list) -> set:
    names = set()
    boot_file = ROOT / "config/boot/functions.tsv"
    level_file = ROOT / "config/levels/functions.tsv"
    seen = {}
    for n, line in enumerate(boot_file.read_text().splitlines(), 1):
        if not line.strip() or line.startswith("#"):
            continue
        where = f"{boot_file.relative_to(ROOT)}:{n}"
        cols = line.split("\t")
        if len(cols) != 3:
            errors.append(f"{where}: {len(cols)} columns, expected 3")
            continue
        name, address, size = cols
        if not re.fullmatch(r"[0-9A-F]{8}", address) or not size.isdigit():
            errors.append(f"{where}: bad address or size in {line!r}")
        if name in seen:
            errors.append(f"{where}: duplicate name {name} (first at line {seen[name]})")
        seen.setdefault(name, n)
        names.add(name)
    seen = {}
    for n, line in enumerate(level_file.read_text().splitlines(), 1):
        if not line.strip() or line.startswith("#"):
            continue
        where = f"{level_file.relative_to(ROOT)}:{n}"
        cols = line.split("\t")
        if len(cols) != 6:
            errors.append(f"{where}: {len(cols)} columns, expected 6")
            continue
        name, kind, size, fp, count, places = cols
        if kind not in KINDS:
            errors.append(f"{where}: kind {kind!r} not in {sorted(KINDS)}")
        if not size.isdigit() or not count.isdigit():
            errors.append(f"{where}: size or level count is not a number")
        if not re.fullmatch(r"[0-9a-f]{16}", fp):
            errors.append(f"{where}: bad fingerprint {fp!r}")
        if name in seen:
            errors.append(f"{where}: duplicate name {name} (first at line {seen[name]})")
        seen.setdefault(name, n)
        numbers = set()
        for p in places.split(","):
            m = PLACE.match(p)
            if not m:
                errors.append(f"{where}: place {p!r} is not NN:XXXXXXXX")
            elif int(m.group(1)) not in levels.NAMES:
                errors.append(f"{where}: place {p!r} is in unknown level {int(m.group(1))}")
            else:
                numbers.add(int(m.group(1)))
        if count.isdigit() and int(count) != len(numbers):
            errors.append(f"{where}: levels column {count}, places span {len(numbers)}")
        names.add(name)
    return names


def check_sources(known: set, errors: list) -> None:
    defined = {}
    for path in sorted((ROOT / "src").rglob("*.c")):
        text = path.read_text(encoding="utf-8", errors="replace")
        for n, line in enumerate(text.splitlines(), 1):
            m = DEFINITION.match(line)
            if m and not line.rstrip().endswith(";") and not line.startswith(("extern", "typedef")):
                defined.setdefault(m.group(1), []).append(f"{path.relative_to(ROOT)}:{n}")
        for m in re.finditer(rf'^(?:INCLUDE_ASM|ASM_FUNC|LINKER_REMNANT)\("[^"]*",\s*({NAME})\);', text, re.M):
            n = text[:m.start()].count("\n") + 1
            defined.setdefault(m.group(1), []).append(f"{path.relative_to(ROOT)}:{n} (asm)")
    for name, where in sorted(defined.items()):
        if len(where) > 1:
            errors.append(f"{name} appears {len(where)} times: {', '.join(where)}")
        if name not in known:
            errors.append(f"{name} ({where[0]}) is in neither catalogue")
    for name in sorted(known - set(defined)):
        errors.append(f"{name} is in a catalogue but in no file of src/")
    c = sum(1 for w in defined.values() if not w[0].endswith("(asm)"))
    print(f"{len(defined)} functions in src/: {c} in C, {len(defined) - c} as INCLUDE_ASM")


# What the repository must not grow again (AGENTS.md, "Do not bring back what was removed").
FORBIDDEN_DIRS = ("candidates/", "scripts/", "trials/", "progress/levels/", "config/function-catalog/",
                  "config/level-native/", "config/function-evidence/")
FORBIDDEN_DOC = re.compile(r"(?i)(-C-LOT|-NATIVE-LOT|EVIDENCE|REGISTER|CAMPAIGN|RESERVATION|QUEUE|RECEIPT)")
MAX_BYTES = 2_500_000


def check_hygiene(errors: list) -> None:
    import subprocess
    files = subprocess.run(["git", "ls-files"], cwd=ROOT, capture_output=True, text=True).stdout.split()
    if not files:          # not a git checkout: look at the tree
        files = [p.relative_to(ROOT).as_posix() for p in ROOT.rglob("*") if p.is_file() and ".git/" not in p.as_posix()]
    for f in files:
        path = ROOT / f
        if not path.exists():        # deleted in the working tree, not yet committed
            continue
        if f.endswith((".gz", ".cfrag", ".iso", ".elf", ".bin", ".o")):
            errors.append(f"{f}: no {Path(f).suffix} files in the repository (generated or game data)")
        if f.startswith(FORBIDDEN_DIRS):
            errors.append(f"{f}: this directory belongs to the removed system (AGENTS.md)")
        if f.startswith("docs/") and FORBIDDEN_DOC.search(f):
            errors.append(f"{f}: no per-batch, evidence or campaign documents (AGENTS.md)")
        if f.startswith("progress/") and f != "progress/report.json":
            errors.append(f"{f}: progress/ holds only report.json, written by tools/progress.py")
        if path.is_file() and path.stat().st_size > MAX_BYTES and f != "assets/rac2-logo.png":
            errors.append(f"{f}: {path.stat().st_size} bytes; generated data stays out of git")
        if f.startswith("src/") and f.endswith((".c", ".h")):
            if re.search(r"\bLVL_\d+_[A-Z]", path.read_text(errors="replace")):
                errors.append(f"{f}: per-level copies (LVL_ names); one definition per function (AGENTS.md)")


def main() -> int:
    errors: list = []
    check_hygiene(errors)
    known = check_catalogues(errors)
    print(f"{len(known)} catalogue names")
    check_sources(known, errors)
    if errors:
        print(f"\n{len(errors)} problem(s):", file=sys.stderr)
        for e in errors[:200]:
            print("  " + e, file=sys.stderr)
        return 1
    print("repository checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
