#!/usr/bin/env python3
"""
How GCC 2.95's global register allocator treated one function (in the container).

  bash tools/docker/run.sh python tools/regalloc.py func_X CANDIDATE.c

For register near misses: every instruction is right but a variable sits in
another register. The choice comes from numbers the compiler computes, so
rewriting the C blind rarely gets there. This builds CANDIDATE.c in the
function's own file exactly as tools/try_func.py does (same flags, file
flags included), with `-dlg` added to the compile step, and prints for each
pseudo that global allocation handled, in allocation order: its reference
count, live length, priority and the hard register it got. `-dlg` only
makes the compiler write the allocator's dumps (to the current directory;
they are moved into build-sn/regalloc/<func>/; concurrent runs wait on a lock); the
code is the same. The verdict at the end is try_func's (overlay_check for level code).

How the allocator decides (GCC 2.95 global.c):
  - pseudos are allocated in decreasing priority,
        floor_log2(refs) * refs / live_length
    (allocno_compare truncates it, then prefers the lower pseudo number);
  - a pseudo takes the first register that nothing already allocated and
    live at the same time holds; a parameter prefers its incoming register
    but loses it to a higher-priority local that overlaps it.
So when retail keeps an argument in another register, some local outranked
it and overlaps it: give that local a shorter live range or one reference
more or less (compute it later, use it once, declare it in the block that
uses it), then run this again before spending a build on it.

Adapted from rac3-uya-decomp's tools/regalloc.py (GPL-3.0), which found the
rule on the same compiler family; that project works directly with this one.
"""
import math
import re
import time
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import overlay_check  # noqa: E402
import try_func  # noqa: E402

_run = try_func.run


def run_with_dumps(cmd, log):
    """try_func.run, with the allocator dumps switched on for the compile step only."""
    if "-S" in cmd:
        i = cmd.index("-S")
        cmd = cmd[:i] + ["-dlg"] + cmd[i:]
    return _run(cmd, log)


def section(dump: str, name: str) -> str:
    """The part of a GCC dump that belongs to function NAME."""
    m = re.search(rf"^;; Function {re.escape(name)}\b.*?(?=^;; Function |\Z)", dump, re.M | re.S)
    return m.group(0) if m else ""


def main() -> None:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) != 2:
        sys.exit(__doc__)
    name, cand = args
    seg, src, first, last = try_func.find_stub(name)
    work = Path("build-sn/regalloc") / name
    try_func.run = run_with_dumps
    # GCC writes its dumps to the current directory under the input's base name,
    # so concurrent runs would take each other's dumps: hold a lock until they are moved.
    work.parent.mkdir(parents=True, exist_ok=True)
    lock = work.parent / ".lock"            # a directory: mkdir is atomic across containers
    while True:
        try:
            lock.mkdir()
            break
        except FileExistsError:
            if time.time() - lock.stat().st_mtime > 600:    # a run that died holding it
                lock.rmdir()
            time.sleep(2)
    try:
        for ext in ("lreg", "greg"):
            Path(f"src.c.{ext}").unlink(missing_ok=True)
        obj = try_func.build(name, seg, src, first, last, Path(cand).read_text(), work)
        for ext in ("lreg", "greg"):
            dump = Path(f"src.c.{ext}")
            if dump.exists():
                dump.replace(work / dump.name)
    finally:
        lock.rmdir()
    if obj is None:
        sys.exit(f"{name}: COMPILE failed, see {work}/log.txt")
    lreg = section((work / "src.c.lreg").read_text(errors="replace"), name)
    greg = section((work / "src.c.greg").read_text(errors="replace"), name)
    if not lreg or not greg:
        sys.exit(f"{name}: no allocator dump for it in {work} (is it defined in {cand}?)")
    info = {}
    for m in re.finditer(r"^Register (\d+) used (\d+) times across (\d+) insns(.*?)\.$", lreg, re.M):
        n, refs, live, rest = int(m.group(1)), int(m.group(2)), int(m.group(3)), m.group(4)
        pri = (int(math.log2(refs)) if refs else 0) * refs / live if live else 0
        info[n] = dict(refs=refs, live=live, pri=pri, user="user var" in rest)
    alloc = re.search(r"regs to allocate: ([\d ]+)", greg)
    order = [int(x) for x in alloc.group(1).split()] if alloc else []
    disp_text = greg.split(";; Register dispositions:")[1].split(";; Hard regs")[0] \
        if ";; Register dispositions:" in greg else ""
    disp = {int(a): int(b) for a, b in re.findall(r"(\d+) in (\d+)", disp_text)}
    print(f"{name}: global allocation order (pseudo -> hard register; MIPS numbers, 4 = $a0, 16 = $s0):")
    for p in order:
        i = info.get(p, {})
        print("  pseudo %-4d -> $%-3s refs %-3s live %-4s priority %.3f%s"
              % (p, disp.get(p, "?"), i.get("refs"), i.get("live"), i.get("pri", 0),
                 "  (named variable)" if i.get("user") else ""))
    if try_func.OVERLAY_NAME.match(name):
        verdict = overlay_check.check(obj, name, False)
    else:
        verdict = try_func.compare(name, seg, obj, False)
    print(f"{name}: {verdict}   ({src})")


if __name__ == "__main__":
    main()
