#!/usr/bin/env python3
"""
Version-specific C: where another version's bytes differ from what the shared C builds
(docs/VERSIONS.md, "Shared and version-specific C").

  python3 tools/version_src.py build --version jp              # build-sn/versions/jp/src/**/obj.o
  python3 tools/version_src.py try --version jp FUNC cand.c     # one candidate: its verdict on jp

Every function has one C source, shared by every version: its file under src/. Only where a
version's code differs (a 60 Hz constant, a moved field, a call to code the version changed) does
it get its own definition, in

  src/versions/<K>/<path of the shared file without .c>/<function>.c

holding that function's definition alone (and any declaration it needs that the shared file lacks).
It is built the way retail built that function: inside its own translation unit. The shared file
is compiled with each of the version's definitions put in place of the shared ones, through the
same pipeline and with the same flags as the shared file (tools/try_func.py's build(),
config/file_cflags.txt by the shared file's path), so a version can never build a function with
options or a context its object did not have. tools/version_check.py then checks the result
against the version's bytes; `copy` there means a version-specific definition the shared C does
not need (the check fails).

Run it inside the build container on a Mac (`bash tools/docker/run.sh python tools/version_src.py ...`).
"""
import argparse
import collections
import os
import re
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
os.chdir(ROOT)
sys.path.insert(0, str(ROOT / "tools"))
import versions  # noqa: E402


def overrides(v):
    """{function: version-specific file} for version V."""
    root = ROOT / "src/versions" / v.key
    out = {}
    for p in sorted(root.rglob("func_*.c")) if root.exists() else []:
        out[p.stem] = p
    return out


def shared_file(v, path):
    """The shared file a version-specific file stands in for."""
    rel = path.relative_to(ROOT / "src/versions" / v.key)
    return Path("src") / (str(rel.parent) + ".c")


def spliced(v, src, funcs):
    """SRC's text with each of FUNCS' version-specific definitions in place of the shared one."""
    import try_func
    lines = src.read_text(errors="replace").splitlines()
    spans = []
    for name, path in funcs.items():
        seg, s, first, last = try_func.find_stub(name)
        if Path(s).as_posix() != src.as_posix():
            sys.exit(f"{path}: {name} is defined in {s}, not {src}")
        spans.append((first, last, path.read_text().rstrip("\n").splitlines()))
    for first, last, body in sorted(spans, reverse=True):
        lines[first:last + 1] = body
    return "\n".join(lines) + "\n"


def segment_of(src):
    import libgcc_units
    for seg, srcs in libgcc_units.SEGMENT_SOURCES.items():
        if src.as_posix() in [Path(s).as_posix() for s in srcs]:
            return seg
    return "text"


def build_file(key, src, funcs, work):
    """Compile SRC with FUNCS ({name: path}) spliced in, into WORK/obj.o. Returns (src, error or None)."""
    import try_func
    v = versions.get(key)
    text = spliced(v, Path(src), {n: Path(p) for n, p in funcs.items()})
    n = len(Path(src).read_text(errors="replace").splitlines())
    obj = try_func.build(next(iter(funcs)), segment_of(Path(src)), Path(src), 0, n - 1, text, Path(work))
    if obj is None:
        return src, (Path(work) / "log.txt").read_text(errors="replace")[-2000:]
    return src, None


def objects_dir(v):
    return ROOT / "build-sn/versions" / v.key / "src"


def build(v, only=None):
    groups = collections.defaultdict(dict)
    for name, path in overrides(v).items():
        if only and name not in only:
            continue
        groups[shared_file(v, path)][name] = str(path)
    from toolchain import start_wineserver
    start_wineserver()
    jobs = [(v.key, str(src), funcs, str(objects_dir(v) / str(src)[:-2])) for src, funcs in groups.items()]
    bad = 0
    with ProcessPoolExecutor(os.cpu_count() or 4) as ex:
        for src, err in ex.map(build_file, *zip(*jobs)) if jobs else []:
            if err:
                bad += 1
                print(f"*** {src} with {v.key}'s definitions failed to build:\n{err}")
    print(f"{v.key}: {len(jobs) - bad} of {len(jobs)} files with version-specific C built")
    return bad == 0


_TABLES = {}


def try_one(v, name, cand):
    """Splice CAND in as NAME's version-specific definition (with the version's others) and check it."""
    import try_func
    import version_check as vc
    seg, src, first, last = try_func.find_stub(name)
    funcs = {n: str(p) for n, p in overrides(v).items() if shared_file(v, p) == Path(src)}
    funcs[name] = str(cand)
    work = ROOT / "build-sn/versions" / v.key / "try" / name
    _, err = build_file(v.key, str(src), funcs, str(work))
    if err:
        return "COMPILE " + err.strip().splitlines()[-1][:200]
    vc.EXTRA_OBJECTS[name] = str(work / "obj.o")
    e = vc.entries_by_name(v).get(name)
    if e is None:
        return "no such function in " + v.key
    if v.key not in _TABLES:
        _TABLES[v.key] = vc.Table(v)
    status, close, note = vc.check_one(v, _TABLES[v.key], name, "src/versions/" + v.key, e)
    return note


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("command", choices=("build", "try"))
    ap.add_argument("args", nargs="*")
    ap.add_argument("--version", required=True)
    a = ap.parse_args()
    v = versions.get(a.version)
    if a.command == "build":
        sys.exit(0 if build(v, set(a.args) or None) else 1)
    name, cand = a.args
    print(f"{name}: {try_one(v, name, cand)}")


if __name__ == "__main__":
    main()
