#!/usr/bin/env python3
"""
Writes each near miss's remaining differences, for a near wave's packets
(tools/wave.py plan --overlay --near): builds build-sn/try/<func>/best.c
in its file and checks it strictly (tools/overlay_check.py), saving the
verdict and the differing instructions to build-sn/try/<func>/BEST_DIFF.txt.
No try_func run is logged, so it costs the worker none of its budget.

  python tools/near_diffs.py func_LNN_XXXXXXXX ...

Run it in the container (tools/docker/run.sh); builds run in parallel.
"""
import contextlib, io, sys
from pathlib import Path
sys.path.insert(0, "tools")
import try_func, overlay_check

TRY = Path("build-sn/try")


def diff_one(name: str) -> str:
    best = TRY / name / "best.c"
    if not best.exists():
        return f"{name}: no best.c"
    seg, src, first, last = try_func.find_stub(name)
    obj = try_func.build(name, seg, src, first, last, best.read_text(), TRY / name / "bestdiff")
    shown = io.StringIO()
    if obj is None:
        verdict = "COMPILE failed"
    else:
        with contextlib.redirect_stdout(shown):
            verdict = overlay_check.check(obj, name, True)
    (TRY / name / "BEST_DIFF.txt").write_text(f"{verdict}\n{shown.getvalue()}")
    return f"{name}: {verdict}"


if __name__ == "__main__":
    from concurrent.futures import ProcessPoolExecutor
    import os
    from toolchain import start_wineserver
    start_wineserver()      # one server for the pool, or Wine processes race to start one
    with ProcessPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        for line in pool.map(diff_one, sys.argv[1:]):
            print(line, flush=True)
