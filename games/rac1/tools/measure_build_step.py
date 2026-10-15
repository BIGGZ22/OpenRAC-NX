#!/usr/bin/env python3
"""
How many matches depend on a build step (docs/BUILD_FIDELITY.md, rule 4).

  bash tools/docker/run.sh python tools/measure_build_step.py STEP [STEP ...]

STEP is a tool the build runs between the compiler and the assembler, named
as in `tools/<STEP>.py` (e.g. ps2eeas_nops). Two scratch worktrees of HEAD
are made under build-sn/measure/: one as it is, one with each STEP replaced
by a pass-through that copies its input to its output. In both, every source
file is built once (tools/try_func.py's pipeline) and every C function is
checked against retail on its own: executable functions with relocated
fields masked, level functions with tools/overlay_check.py. A function that
is exact as it is and not without the steps depends on them. Layout effects
are left out on purpose; the full build (tools/build_sn.sh) covers those.

Takes about as long as two progress reports. The worktrees are removed at
the end; the per-function verdicts stay in build-sn/measure/<name>.json.
"""
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build-sn/measure"
PASS = '''import shutil, sys
args = [a for a in sys.argv[1:] if not a.startswith("-")]
if "--" in sys.argv or not args:
    sys.exit(0)
src, dst = args[0], args[-1]
if src != dst and not dst.endswith(".o"):
    shutil.copy(src, dst)
'''

# Runs inside a worktree: every C function's verdict, as JSON on stdout.
MEASURE = r'''
import json, os, re, sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path
sys.path.insert(0, "tools")
import gen_progress_report as gpr
import try_func

ALIAS = {m.group(1): m.group(2) for m in re.finditer(r"^#define\s+(\w+)\s+(func_[0-9A-F]{8})\b",
         Path("include/names.h").read_text(), re.M)}
DEF_ANY = re.compile(r"^(?!extern\b|static\b|typedef\b|#)[A-Za-z_][\w \t\*]*?\b(\w+)\s*\([^;]*$", re.M)

def exe_file(job):
    seg, src = job
    text = Path(src).read_text(errors="replace")
    stubs = set(gpr.STUB.findall(text))
    names = set(gpr.FUNC_DEF.findall(text)) | {ALIAS[m] for m in DEF_ANY.findall(text) if m in ALIAS}
    names = sorted(n for n in names if n not in stubs)
    if not names:
        return {}
    lines = text.splitlines()
    obj = try_func.build(names[0], seg, Path(src), 0, len(lines) - 1, "\n".join(lines) + "\n",
                         Path("build-sn/measure-work") / Path(src).stem)
    if obj is None:
        return {n: "BUILD" for n in names}
    out = {}
    for n in names:
        try:
            out[n] = try_func.compare(n, seg, obj, False)
        except BaseException as e:
            out[n] = "ERR " + type(e).__name__
    return out

jobs = [(seg, str(src)) for seg, srcs in gpr.SEGMENT_SOURCES.items() for src in srcs
        if not str(src).startswith("src/libgcc")]
result = {}
gpr.start_wineserver()
with ProcessPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
    for part in pool.map(exe_file, jobs):
        result.update(part)
level = gpr.overlay_match_results(gpr.overlay_file_map())
result.update({n: ("EXACT" if v == 100.0 else "MISS") for n, v in level.items()})
print(json.dumps(result))
'''


def worktree(name: str) -> Path:
    path = OUT / name
    if path.exists():
        subprocess.run(["git", "worktree", "remove", "--force", str(path)], cwd=ROOT)
    subprocess.run(["git", "worktree", "add", "-q", "--detach", str(path), "HEAD"], cwd=ROOT, check=True)
    for d in ("asm", "baserom", "toolchain"):
        (path / d).symlink_to(ROOT / d)
    return path


def measure(path: Path) -> dict:
    run = subprocess.run([sys.executable, "-c", MEASURE], cwd=path, capture_output=True, text=True)
    if run.returncode:
        sys.exit(f"{path}: measurement failed:\n{run.stderr[-3000:]}")
    return json.loads(run.stdout.strip().splitlines()[-1])


def main() -> None:
    steps = sys.argv[1:]
    if not steps:
        sys.exit(__doc__)
    for s in steps:
        if not (ROOT / f"tools/{s}.py").exists():
            sys.exit(f"no tools/{s}.py")
    OUT.mkdir(parents=True, exist_ok=True)
    name = "-".join(steps)
    base, test = worktree("base"), worktree(name)
    for s in steps:
        (test / f"tools/{s}.py").write_text(PASS)
    try:
        before, after = measure(base), measure(test)
    finally:
        for p in (base, test):
            subprocess.run(["git", "worktree", "remove", "--force", str(p)], cwd=ROOT)
    (OUT / f"{name}.json").write_text(json.dumps({"with": before, "without": after}, indent=0))
    report = json.loads((ROOT / "progress/report.json").read_text())
    size = {f["name"]: int(f["size"]) for u in report["units"] for f in u["functions"]}
    total = int(report["measures"]["total_code"])
    lost = sorted(n for n, v in before.items()
                  if str(v).startswith("EXACT") and not str(after.get(n, "")).startswith("EXACT"))
    b = sum(size.get(n, 0) for n in lost)
    exact = sum(1 for v in before.values() if str(v).startswith("EXACT"))
    print(f"{' + '.join(steps)}: {len(lost)} of {exact} exact functions depend on it, "
          f"{b:,} bytes ({100 * b / total:.2f}% of the code)")
    for n in lost:
        print(f"  {n}\t{size.get(n, 0)}\t{after.get(n)}")


if __name__ == "__main__":
    main()
