#!/usr/bin/env python3
"""
Set up a decomp-permuter directory for one function. decomp-permuter
(tools/ext/decomp-permuter, tools/permuter_bootstrap.sh) randomly rewrites a
C function -- reorders statements, adds temporaries, retypes things, swaps
branches -- and keeps whatever compiles closer to retail. It is the tool for
a register-allocation or scheduling tie that no hand rewrite moves (a
WORKFLOW.md "known wall"): hundreds or thousands of automatic rewrites of
the same statements where a permute.py enumeration or a manual reordering
already stopped short.

  python3 tools/permuter_setup.py func_X build-sn/try/func_X/cand4.c

  # then, inside the build container (needs Wine + the SN toolchain; run
  # tools/permuter_bootstrap.sh once first):
  bash tools/docker/run.sh bash tools/permuter_run.sh build-sn/permuter/func_X -j4 --stop-on-zero

CANDIDATE.c is the same kind of file tools/try_func.py takes: the function
definition, plus whatever extern declarations it needs beyond what its real
source file already provides above it -- your best try_func.py attempt so
far, not a fresh stub; the permuter improves on a near-miss, it doesn't
write one from scratch. Writes build-sn/permuter/<func>/:

  base.c       the function's real source file with CANDIDATE.c spliced
               into it in place of the stub (as tools/try_func.py would for
               a --try run), preprocessed into one flat translation unit --
               so decomp-permuter's AST tool (pycparser) can read it -- and
               reduced to just the target function's body, with everything
               else in the file as bare prototypes (decomp-permuter's own
               tools/strip_other_fns.py). This is what the permuter mutates.
  target.o     retail: the function's own INCLUDE_ASM stub, compiled
               through the exact same real per-segment pipeline as every
               candidate object below, so only the target function's bytes
               ever differ between the two sides of a comparison.
  compile.sh   hands the permuter's current mutated base.c to
               tools/permuter_compile.py, which pulls the target function's
               definition back out and recompiles it through
               tools/try_func.py's own build() -- the same file_cflags.py flags /
               fix_orphan_hi.py / ps2eeas_nops.py passes try_func.py itself
               runs -- so a score of 0 here means the same thing try_func.py
               calls EXACT. Confirm any score-0 result with try_func.py
               before touching src/: the permuter never does, and never
               will (there is no path from here to src/).
  settings.toml

Only works on a function that is still an INCLUDE_ASM stub in its real
source file: target.o is built from that stub's retail bytes. A function
already rewritten to near-miss C in src/ (a landed attempt being refined
further) has no INCLUDE_ASM left to build target.o from -- use
tools/try_func.py directly for that case; this tool doesn't handle it.

Run this under `bash tools/docker/run.sh python3 tools/permuter_setup.py
...`. base.c's own preprocessing is plain macro expansion and works with
the host's own `cpp`/`gcc -E` (see --cpp), but target.o needs Wine and the
SN compiler, which only the container has, so there is no reason to split
the two steps across host and container.

Related tools:
  tools/permute.py   enumerates every ordering of a handful of statements
                      YOU mark with /*P*/ -- exact and exhaustive, good for
                      "these N stores are in some wrong order" (up to about
                      8 marked lines), useless past that and useless for
                      anything that isn't a pure reordering.
  this tool           hands the whole function to decomp-permuter's
                      randomizer (reorderings, temporaries, retyping,
                      branch rewrites) and scores every attempt against
                      retail automatically. Reach for it on a
                      register-allocation tie or a scheduling near-miss
                      that resists hand rewriting; reach for permute.py
                      first on a small, known reordering, since it is exact
                      where the permuter is only ever suggestive.
"""
import argparse
import importlib.util
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PERMUTER = ROOT / "tools" / "ext" / "decomp-permuter"


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("name", help="func_XXXXXXXX or func_LNN_XXXXXXXX")
    ap.add_argument("candidate", help="a try_func.py-style candidate .c file")
    ap.add_argument("--out", default=str(ROOT / "build-sn" / "permuter"))
    ap.add_argument("--cpp", default="gcc -E -P", help="host preprocessor for base.c (default: gcc -E -P)")
    args = ap.parse_args()

    if not PERMUTER.joinpath("permuter.py").exists():
        sys.exit(f"{PERMUTER} not set up; run tools/permuter_bootstrap.sh first "
                 "(bash tools/docker/run.sh bash tools/permuter_bootstrap.sh)")

    sys.path.insert(0, str(ROOT / "tools"))
    import try_func as tf  # noqa: E402

    name = args.name
    seg, src, first, last = tf.find_stub(name)
    stub_lines = src.read_text(errors="replace").splitlines()
    stub_text = "\n".join(stub_lines[first:last + 1])
    if "INCLUDE_ASM" not in stub_text:
        sys.exit(f"{name}: not an INCLUDE_ASM stub in {src} (already C) -- "
                 "no retail bytes here to build target.o from. Use tools/try_func.py instead.")

    candidate_text = Path(args.candidate).read_text()

    d = Path(args.out) / name
    d.mkdir(parents=True, exist_ok=True)

    # The candidate's own declarations (everything above the function
    # itself -- try_func.py's convention: "the function definition, plus
    # any extern declarations it needs"), saved so compile.sh can put them
    # back. strip_other_fns.py below keeps them in base.c (for pycparser to
    # resolve types against), but the permuter's compile.sh only ever hands
    # tools/permuter_compile.py the mutated FUNCTION back -- the permuter
    # doesn't touch file-scope declarations, so these are safe to re-supply
    # verbatim on every compile rather than re-extracting them from a
    # mutated, stripped, reformatted file each time.
    func_line = re.search(r"^(?!extern\b)[A-Za-z_].*?\b" + re.escape(name) + r"\s*\(",
                          candidate_text, re.M)
    defname = ""
    if func_line is None:
        # A candidate may define the function under another C name that is an
        # alias of it (`void impl(...) __asm__("func_X");`), the way try_func
        # candidates do when the source file already declares func_X with
        # another prototype. The permuter's base.c calls the function func_X
        # (so pycparser and the scorer see the right symbol); compile.sh
        # turns the name back into the alias before compiling.
        alias = re.search(r"\b(\w+)\s*\([^;{]*\)\s*__asm__\s*\(\s*\"" + re.escape(name) + r"\"\s*\)\s*;",
                          candidate_text)
        if alias:
            defname = alias.group(1)
            own = None
            for m in re.finditer(r"^(?!extern\b)[A-Za-z_].*?\b" + re.escape(defname) + r"\s*\(",
                                 candidate_text, re.M):
                line_end = candidate_text.find("\n", m.start())
                if not candidate_text[m.start():line_end].rstrip().endswith(";"):
                    own = m
                    break
            if own is not None:
                candidate_text = (candidate_text[:own.start()]
                                  + re.sub(r"\b" + re.escape(defname) + r"\b", name,
                                           candidate_text[own.start():], count=1))
                func_line = re.search(r"^(?!extern\b)[A-Za-z_].*?\b" + re.escape(name) + r"\s*\(",
                                      candidate_text, re.M)
    if func_line is None:
        sys.exit(f"{name}: no definition of {name} found in {args.candidate}")
    (d / "decls.c").write_text(candidate_text[:func_line.start()])

    # ---- base.c: candidate spliced into its real file, preprocessed and
    # trimmed to just the target function (decomp-permuter's own manual
    # recipe, USAGE.md: cpp -P -D'__attribute__(x)=', then
    # strip_other_fns.py). -DPERMUTER empties every OTHER INCLUDE_ASM stub
    # in the same file to nothing (include/include_asm.h already has this
    # guard) instead of the raw `.include "asm/..."` __asm__ block pycparser
    # can't read. ----
    spliced = "\n".join(stub_lines[:first] + [candidate_text.rstrip("\n")] + stub_lines[last + 1:])
    tmp_in = d / "_spliced.c"
    tmp_in.write_text(spliced)
    cpp_cmd = args.cpp.split() + [
        "-DPERMUTER", "-D__attribute__(x)=", "-D__extension__=",
        "-I", str(ROOT / "include"), "-I", str(ROOT), str(tmp_in),
    ]
    p = subprocess.run(cpp_cmd, capture_output=True, text=True)
    tmp_in.unlink()
    if p.returncode:
        sys.exit("preprocessing base.c failed:\n" + p.stderr)

    sys.path.insert(0, str(PERMUTER))
    strip_other_fns = load("strip_other_fns", PERMUTER / "strip_other_fns.py")
    base = strip_other_fns.strip_other_fns(p.stdout, name)
    if re.search(r"^(?!extern\b)[A-Za-z_].*?\b" + re.escape(name) + r"\s*\(", base, re.M) is None:
        sys.exit(f"{name}: not found in preprocessed base.c -- does the candidate define {name}?")
    (d / "base.c").write_text(base)

    # ---- target.o: the stub's own INCLUDE_ASM line, through the exact
    # same pipeline every candidate.o below goes through (tf.build), so
    # only the target function's bytes ever differ between the two ----
    target_work = d / "_target_build"
    shutil.rmtree(target_work, ignore_errors=True)
    obj = tf.build(name, seg, src, first, last, stub_text, target_work)
    if obj is None:
        sys.exit("building target.o (the retail stub) failed:\n"
                 + (target_work / "log.txt").read_text(errors="replace"))
    shutil.copy(obj, d / "target.o")
    shutil.rmtree(target_work, ignore_errors=True)

    # ---- compile.sh ----
    sh = f"""#!/usr/bin/env bash
# Generated by tools/permuter_setup.py for {name}. decomp-permuter invokes
# this as: compile.sh in.c -o out.o (see tools/permuter_compile.py, which
# does the actual work through tools/try_func.py's own build()).
set -e
DIR="$(cd "$(dirname "${{BASH_SOURCE[0]}}")" && pwd)"
cd {ROOT}
exec python3 tools/permuter_compile.py --name {name} {("--defname " + defname + " ") if defname else ""}--decls "$DIR/decls.c" "$1" -o "$3"
"""
    sh_path = d / "compile.sh"
    sh_path.write_text(sh)
    sh_path.chmod(0o755)

    (d / "settings.toml").write_text(
        f'func_name = "{name}"\n'
        f'compiler_type = "gcc"\n'
        f'objdump_command = "mips-linux-gnu-objdump -drz -m mips:5900"\n'
    )

    print(str(d))
    print(f"next: bash tools/docker/run.sh bash tools/permuter_run.sh {d} "
          f"-j{os.cpu_count() or 2} --stop-on-zero")


if __name__ == "__main__":
    main()
