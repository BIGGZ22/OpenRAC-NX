#!/usr/bin/env python3
"""
Compile ONE candidate function through the real per-segment pipeline, for a
decomp-permuter compile.sh (see tools/permuter_setup.py). Runs inside the
build container (needs Wine and the SN toolchain, like tools/try_func.py).

  python3 tools/permuter_compile.py --name func_X IN.c -o OUT.o

IN.c is decomp-permuter's current candidate: the whole preprocessed,
stripped translation unit tools/permuter_setup.py built as base.c, with the
target function's body possibly rewritten by the permuter. This pulls just
that function's definition back out and hands it to tools/try_func.py's own
build() -- the same function try_func.py uses for `python tools/try_func.py
func_X candidate.c` -- so a candidate compiles here exactly as it would
there: same flags (file_cflags), same fix_orphan_hi/ps2eeas_nops passes.

The function's location (which segment, which source file, which lines to
replace) is re-resolved by name on every call, not cached from setup time:
another function landing in the same source file while a permuter run is in
progress must not desync this from stale line numbers.

--decls FILE prepends FILE's text to the extracted function before
compiling: the candidate's own declarations (tools/permuter_setup.py's
decls.c), which decomp-permuter's base.c also carries but only as part of
a preprocessed, reformatted, strip_other_fns'd whole file that this script
would otherwise have to re-parse to pull the right ones back out. The
permuter only ever mutates statements inside the target function's body,
never file-scope declarations, so re-supplying them verbatim from setup
time is exact, not an approximation.

Exits nonzero, with try_func's own log on stderr, if the candidate doesn't
compile -- decomp-permuter treats that as a candidate to discard, the same
as any other compile failure.
"""
import argparse
import re
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import try_func as tf  # noqa: E402


def extract_function(text: str, name: str) -> str:
    """The braces-matched definition of NAME in TEXT, in try_func's own
    style (find_stub/find_overlay_stub): the first non-extern, non-`;`-
    terminated line that names NAME, through the line where its braces
    balance back to zero."""
    lines = text.splitlines()
    def_re = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b" + re.escape(name) + r"\s*\(")
    for i, line in enumerate(lines):
        if def_re.match(line) and not line.rstrip().endswith(";"):
            depth, seen = 0, False
            for j in range(i, len(lines)):
                depth += lines[j].count("{") - lines[j].count("}")
                seen = seen or "{" in lines[j]
                if seen and depth == 0:
                    return "\n".join(lines[i:j + 1])
    sys.exit(f"permuter_compile: no definition of {name} found in the candidate")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--name", required=True, help="func_XXXXXXXX or func_LNN_XXXXXXXX")
    ap.add_argument("--decls", help="declarations to prepend (tools/permuter_setup.py's decls.c)")
    ap.add_argument("--defname", help="C name the function is really defined under (an alias of --name)")
    ap.add_argument("infile")
    ap.add_argument("-o", dest="out", required=True)
    args = ap.parse_args()

    seg, src, first, last = tf.find_stub(args.name)
    candidate = extract_function(Path(args.infile).read_text(), args.name)
    if args.defname:
        candidate = re.sub(r"\b" + re.escape(args.name) + r"\b", args.defname, candidate, count=1)
    if args.decls:
        candidate = Path(args.decls).read_text() + candidate

    with tempfile.TemporaryDirectory(prefix="permcompile_") as tmp:
        work = Path(tmp)
        obj = tf.build(args.name, seg, src, first, last, candidate, work)
        if obj is None:
            log = work / "log.txt"
            sys.stderr.write(log.read_text(errors="replace") if log.exists() else "compile failed\n")
            sys.exit(1)
        Path(args.out).write_bytes(obj.read_bytes())


if __name__ == "__main__":
    main()
