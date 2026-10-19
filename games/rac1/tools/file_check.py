#!/usr/bin/env python3
"""
Checks whole source files: builds each file once, the way try_func.py builds
a candidate, and compares every C function in it with retail. The per-file
check for edits that must not change any byte (renames, types, field
access), in the executable as in level code.

  python tools/file_check.py FILE [...]

Executable files (src/game, src/core, src/libgcc) are compared as
try_func.py compares them: relocated fields filled with retail's addresses,
so a different symbol or addend shows. Level files (src/overlays) are
checked as tools/overlay_file_check.py checks them (tools/overlay_check.py
places and resolves everything), without its two-minute catalogue scan.
Like try_func, this is a filter: `bash tools/build_sn.sh` (full build, link
at retail's addresses, image audit) is the gate for executable files.

Run it in the container (tools/docker/run.sh); exits 1 on any problem.
"""
import sys
from pathlib import Path

sys.path.insert(0, "tools")
import srcnames  # noqa: E402
import try_func  # noqa: E402
from libgcc_units import SEGMENT_SOURCES  # noqa: E402


def segment_of(path: Path):
    for seg, srcs in SEGMENT_SOURCES.items():
        if path.as_posix() in {Path(s).as_posix() for s in srcs}:
            return seg
    return None


def c_functions(path: Path, overlay: bool = False) -> list[str]:
    """Address names of the functions PATH defines in C, in file order."""
    lines = srcnames.read_c(path, errors="replace").splitlines()
    pattern = try_func.OVERLAY_DEF if overlay else try_func.DEF
    out = []
    for i, line in enumerate(lines):
        m = pattern.match(line)
        if not m or line.rstrip().endswith(";") or m.group(1) in out:
            continue
        nxt = next((l for l in lines[i + 1:] if l.strip()), "")
        if "{" in line or nxt.lstrip().startswith("{"):
            out.append(m.group(1))
    return out


def check_file(path: Path, seg: str) -> list[str]:
    overlay = seg == "overlay"
    names = c_functions(path, overlay)
    if not names:
        return []
    text = path.read_text(errors="replace")
    n = len(text.splitlines())
    work = Path("build-sn/file_check") / path.parent.name / path.stem
    obj = try_func.build(names[0], "text" if overlay else seg, path, 0, n - 1, text, work)
    if obj is None:
        log = (work / "log.txt").read_text(errors="replace")
        errs = [l for l in log.splitlines()
                if "error" in l.lower() or "undeclared" in l or "conflicting" in l]
        return [f"BUILD FAIL {path}: " + " | ".join(errs[:4])]
    problems = []
    for name in names:
        if overlay:
            verdict = try_func.overlay_check.check(obj, name)
        else:
            verdict = try_func.compare(name, seg, obj, False)
        if not verdict.startswith("EXACT"):
            problems.append(f"NOT EXACT {path}: {name} {verdict}")
    print(f"{path}: {len(names)} C functions, {len(problems)} not exact")
    return problems


def main() -> None:
    files = [Path(a) for a in sys.argv[1:]]
    if not files:
        sys.exit(__doc__)
    from toolchain import start_wineserver
    start_wineserver()
    problems = []
    for p in files:
        seg = "overlay" if p.parts[:2] == ("src", "overlays") else segment_of(p)
        if seg is None:
            problems.append(f"UNKNOWN {p}: not a source of any segment")
            continue
        problems += check_file(p, seg)
    for line in problems:
        print(line)
    print(f"file_check: {len(files)} files, {len(problems)} problems")
    sys.exit(1 if problems else 0)


if __name__ == "__main__":
    main()
