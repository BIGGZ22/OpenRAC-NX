#!/usr/bin/env python3
"""
Holds the build to docs/BUILD_FIDELITY.md.

  python3 tools/check_build_fidelity.py      # exit 1 with the reasons if the build strays

The steps between the compiler and the assembler may only do what an
assembler or a linker does, and each one is listed in BUILD_FIDELITY.md with
the evidence for it. This check reads every `tools/*.py` that Makefile.sn's
recipes and tools/try_func.py's build() run and fails when:

  - one of them is not in ALLOWED (a new step needs a section in
    BUILD_FIDELITY.md, evidence from the real tool or from retail, and its
    measured dependence, before it goes on the list);
  - a step removed for rewriting compiler output comes back (REMOVED), or
    per-function flags do (GCC 2.95 takes options per translation unit);
  - an entry of config/file_cflags.txt names a file that does not exist;
  - BUILD_FIDELITY.md does not mention a step on the list.

Standard library only, so CI runs it as part of
`tools/gen_progress_report.py --check`.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOC = ROOT / "docs/BUILD_FIDELITY.md"

# What each allowed step reproduces (BUILD_FIDELITY.md has the evidence and the numbers).
ALLOWED = {
    "ps2eeas_nops": "assembler: SN ps2eeas's short-loop and FPU-hazard nops",
    "ps2eeas_dli": "assembler: ps2eeas's expansion of 64-bit constants",
    "fix_macro_load_delay": "assembler: the load-delay nop after a macro load (989snd, wad)",
    "fix_volatile_slot": "assembler: delay-slot filling of a volatile store before a call (2.9-ee objects)",
    "check_macro_slots": "assembler: a macro access in a delay slot assembled $gp-relative",
    "fix_orphan_hi": "assembler: %hi relocations resolved at assembly time",
    "fix_jump_tables": "linker placement: a switch's table at retail's address",
    "strip_dead": "linker: dead-stripping of unreferenced libgcc functions",
}
# Not transforms: they choose flags or generate lists, and change no instruction.
HELPERS = {"file_cflags", "sn_regnames", "jump_tables"}
# Removed 2026-10-07 for changing what the compiler emitted (BUILD_FIDELITY.md, "Removed").
REMOVED = {"fix_core_spills", "fix_tail_calls", "fix_trunc_slot", "func_cflags"}

TOOL = re.compile(r"tools/([a-z0-9_]+)\.py")


def build_steps() -> dict[str, set[str]]:
    """tool -> where the build runs it."""
    found = {}
    make = (ROOT / "Makefile.sn").read_text()
    for line in make.splitlines():
        if line.startswith("\t"):                       # recipe lines only, not prerequisites
            for t in TOOL.findall(line):
                found.setdefault(t, set()).add("Makefile.sn")
    tf = (ROOT / "tools/try_func.py").read_text()
    start = tf.index("def build(")
    end = tf.index("\ndef ", start + 1)
    for t in TOOL.findall(tf[start:end]):
        found.setdefault(t, set()).add("tools/try_func.py build()")
    return found


def main() -> None:
    problems = []
    steps = build_steps()
    doc = DOC.read_text() if DOC.exists() else ""
    if not doc:
        problems.append(f"{DOC.relative_to(ROOT)} is missing")
    for tool, where in sorted(steps.items()):
        if tool in REMOVED:
            problems.append(f"{tool}.py is run again ({', '.join(sorted(where))}): it was removed for "
                            "rewriting compiler output (docs/BUILD_FIDELITY.md, \"Removed\")")
        elif tool not in ALLOWED and tool not in HELPERS:
            problems.append(f"{tool}.py runs between the compiler and the assembler ({', '.join(sorted(where))}) "
                            "but is not an allowed step: document it in docs/BUILD_FIDELITY.md with its "
                            "evidence and measured dependence, then add it to ALLOWED")
    for tool in sorted(REMOVED):
        if (ROOT / f"tools/{tool}.py").exists():
            problems.append(f"tools/{tool}.py exists again")
    if (ROOT / "config/func_cflags.txt").exists():
        problems.append("config/func_cflags.txt exists: flags apply to whole files (config/file_cflags.txt)")
    for tool in sorted(ALLOWED):
        if f"{tool}.py" not in doc:
            problems.append(f"docs/BUILD_FIDELITY.md does not describe {tool}.py")
    cfg = ROOT / "config/file_cflags.txt"
    if cfg.exists():
        for n, line in enumerate(cfg.read_text().splitlines(), 1):
            line = line.split("#", 1)[0].strip()
            if line and not (ROOT / line.split()[0]).is_file():
                problems.append(f"config/file_cflags.txt:{n}: no file {line.split()[0]}")
    if problems:
        print("build fidelity check failed:")
        for p in problems:
            print("  " + p)
        sys.exit(1)
    print(f"build fidelity: {len([t for t in steps if t in ALLOWED])} documented assembler/linker steps, "
          "no compiler-output rewrites, flags per file only")


if __name__ == "__main__":
    main()
