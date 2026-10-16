#!/usr/bin/env python3
"""
Compile core_text source files whole under each candidate compiler and
report every C function's verdict against retail, to tell which compiler
built each object.

  python tools/compiler_sweep.py                     # every src/core file
  python tools/compiler_sweep.py src/core/X.c ...    # just these
  python tools/compiler_sweep.py --modes v114,ee29 src/core/X.c
  python tools/compiler_sweep.py -j 8 ...            # parallel jobs

Modes (each compiler's output as it is; the build rewrites none of it,
docs/BUILD_FIDELITY.md):
  v114    SN's 2.95.3 build v1.14, the game compiler
  ee29    Sony's 2.9-ee, the SDK compiler

One line per file with the exact count under each mode, then each
function whose verdict differs between modes. The C is compiled as it
is: a file whose C was tuned against one compiler favours that one.
Work goes to build-sn/sweep/<file>/<mode>/. Runs where try_func.py
does (inside the Docker toolchain container on a Mac).
"""
import glob
import shutil
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import try_func as tf  # noqa: E402
from toolchain import sn  # noqa: E402

def compile_mode(mode: str, src: Path, work: Path) -> Path | None:
    work.mkdir(parents=True, exist_ok=True)
    c = work / "src.c"
    c.write_text(src.read_text(errors="replace"))
    obj = work / "obj.o"
    obj.unlink(missing_ok=True)
    a = work / "a.s"
    with open(work / "log.txt", "w") as log:
        if mode == "v114":
            ok = tf.run(sn(tf.CC, *tf.CFLAGS, "-S", "-o", str(a), str(c)), log)
        else:
            ok = tf.run(sn(tf.CC29, *tf.CFLAGS, tf.EE29_INC, "-S", "-o", str(a), str(c)), log)
        final = a
        ok = ok and tf.run(sn(tf.CC, *tf.CFLAGS, "-c", str(final), "-o", str(obj)), log)
    return obj if ok else None


def job(arg: tuple[str, str]) -> tuple[str, str, dict[str, str] | None]:
    path, mode = arg
    src = Path(path)
    obj = compile_mode(mode, src, Path("build-sn/sweep") / src.stem / mode)
    if obj is None:
        return path, mode, None
    names = []
    for line in src.read_text(errors="replace").splitlines():
        m = tf.DEF.match(line)
        if m and not line.rstrip().endswith(";"):
            names.append(m.group(1))
    res = {}
    for n in names:
        try:
            res[n] = tf.compare(n, "core_text", obj, False)
        except Exception as e:  # noqa: BLE001
            res[n] = f"ERR {e}"
    return path, mode, res


def main() -> None:
    args = sys.argv[1:]
    modes, jobs = ["v114", "ee29"], 6
    while args[:1] and args[0].startswith("-"):
        if args[0] == "--modes":
            modes, args = args[1].split(","), args[2:]
        elif args[0] == "-j":
            jobs, args = int(args[1]), args[2:]
        else:
            sys.exit(__doc__)
    files = args or sorted(glob.glob("src/core/*.c"))
    # Parallel Wine processes race to start a wineserver; start one first.
    if shutil.which("wineserver"):
        subprocess.run(["wineserver", "-p60"], stderr=subprocess.DEVNULL)
    out: dict[str, dict[str, dict[str, str] | None]] = {}
    with ProcessPoolExecutor(jobs) as ex:
        for path, mode, res in ex.map(job, [(f, m) for f in files for m in modes]):
            out.setdefault(path, {})[mode] = res
    for path in files:
        r = out[path]
        cells = [f"{m}=FAIL" if r[m] is None else
                 f"{m}={sum(v.startswith('EXACT') for v in r[m].values())}/{len(r[m])}"
                 for m in modes]
        print(f"{path:24s} " + "  ".join(cells))
        names = next((list(v) for v in r.values() if v), [])
        for n in names:
            vs = [r[m][n] if r[m] else "FAIL" for m in modes]
            if len({v.split()[0] for v in vs}) > 1:
                print(f"    {n}: " + " | ".join(f"{m} {v}" for m, v in zip(modes, vs)))


if __name__ == "__main__":
    main()
