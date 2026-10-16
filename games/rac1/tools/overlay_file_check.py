#!/usr/bin/env python3
"""
Builds level-code files the way tools/gen_progress_report.py does and
checks every C function in them strictly (tools/overlay_check.py), listing
every failure instead of stopping at the first. A function can be EXACT on
its own and break later in its file, when a neighbour landed after it
declares a symbol it uses another way (one MACRO_ADDR alias takes the $gp
form away from the whole file), or uses a symbol before the file declares it.

  python tools/overlay_file_check.py [FILE ...]   # default: src/overlays files changed since HEAD

Run it in the container (tools/docker/run.sh); exits 1 on any problem.
"""
import subprocess, sys
from pathlib import Path
sys.path.insert(0, "tools")
import gen_progress_report as g, try_func, overlay_check


def check_file(p: str) -> list[str]:
    """Problems in one file: a build failure, or each C function not EXACT."""
    path = Path(p)
    c_names = [n for n, is_c in FILE_MAP.get(path, []) if is_c]
    if not c_names:
        return []
    lines = path.read_text(errors="replace").splitlines()
    work = Path("build-sn/overlays/file_check") / path.parent.name / path.stem
    obj = try_func.build(c_names[0], "text", path, 0, len(lines) - 1, "\n".join(lines) + "\n", work)
    if obj is None:
        log = (work / "log.txt").read_text(errors="replace")
        errs = [l for l in log.splitlines()
                if "error" in l.lower() or "undeclared" in l or "prior to" in l or "conflicting" in l]
        return [f"BUILD FAIL {p}: " + " | ".join(errs[:4])]
    return [f"NOT EXACT {p}: {n} {v}" for n in c_names if (v := overlay_check.check(obj, n)) != "EXACT"]


FILE_MAP, _ = g.overlay_c_functions()

if __name__ == "__main__":
    from concurrent.futures import ProcessPoolExecutor
    import os
    changed = sys.argv[1:] or subprocess.run(["git", "diff", "--name-only", "HEAD", "--", "src/overlays"],
                                             capture_output=True, text=True).stdout.split()
    from toolchain import start_wineserver
    start_wineserver()      # one server for the pool, or Wine processes race to start one
    with ProcessPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        problems = [line for found in pool.map(check_file, sorted(changed)) for line in found]
    for line in problems:
        print(line)
    print(f"overlay_file_check: {len(changed)} files, {len(problems)} problems")
    sys.exit(1 if problems else 0)
