#!/usr/bin/env python3
"""
Check a batch of candidates and put the exact ones into src/.

  python tools/integrate.py MANIFEST [MANIFEST ...]           # check only
  python tools/integrate.py MANIFEST [MANIFEST ...] --apply   # check, then apply

--apply writes under the landing lock of tools/claims.py.

A manifest has one line per function, `func_X path/to/candidate.c`
(blank lines and # comments are skipped). Each candidate goes through
tools/try_func.py; the ones that come back EXACT are applied with
tools/apply_candidate.py --drop-note (the candidate's own comment above
its definition replaces the old note). Nothing else is touched. Run the
full build afterwards (tools/build_sn.sh): it is the real check.

Runs where try_func does (inside tools/docker/run.sh on a Mac).
"""
import re
import subprocess
import sys

import claims
import provenance

# Upstream bans these (docs/LLM_DECOMP_INSTRUCTIONS.md): only file-scope
# aliases and padding directives may use __asm__. Retail's vector copy is
# qcopy() in include/common.h; a candidate calls it and writes no asm.
BANNED = [(re.compile(r"\bregister\b[^;{]*__asm__\s*\("), "register pin"),
          (re.compile(r'__asm__\s*(?:volatile\s*|__volatile__\s*)?\(\s*""'), "empty-asm barrier"),
          (re.compile(r'__asm__\s*(?:volatile\s*|__volatile__\s*)?\(\s*"[^".][^"]*[\s$:][^"]*"'), "inline asm"),
          (re.compile(r"\bwhile\s*\(\s*0\s*\)"), "do/while (0) barrier"),
          # An alias names one symbol; "D_x+0x34D" makes the assembler do
          # address arithmetic the C should do (docs/QUEUE.md, Rules).
          (re.compile(r'__asm__\s*\(\s*"[A-Za-z_][A-Za-z0-9_]*[^A-Za-z0-9_"][^"]*"\s*\)'), "expression alias"),
          # A candidate's #define lands in the whole file: one renamed a
          # declaration a neighbour links against. Such a candidate (newlib's
          # own macros, say) is landed by hand after review.
          (re.compile(r"(?m)^\s*#\s*define\b"), "#define in a candidate")]


def banned(path: str) -> str:
    raw = open(path).read()
    text = re.sub(r"/\*.*?\*/", "", raw, flags=re.S)
    reasons = [reason for pattern, reason in BANNED if pattern.search(text)]
    sample = provenance.sony_sample(raw)   # comments count: the sample's names must not appear at all
    if sample:
        reasons.append(f"Sony sample name '{sample}' (docs/MOVIE.md)")
    return ", ".join(reasons)


def main() -> None:
    args = sys.argv[1:]
    apply = "--apply" in args
    owner = "integrate"
    if "--lock-owner" in args:          # a caller that already holds the landing lock
        i = args.index("--lock-owner")
        owner = args[i + 1]
        del args[i:i + 2]
    # --trust: candidates a wave already recorded EXACT; skip compiling each
    # again, the caller re-checks the files it landed into (wave.py land
    # --batch runs tools/overlay_file_check.py on them).
    trust = "--trust" in args
    manifests = [a for a in args if a not in ("--apply", "--trust")]
    if not manifests:
        sys.exit(__doc__)
    rows = []
    for path in manifests:
        for line in open(path):
            line = line.split("#")[0].strip()
            if line:
                name, cand = line.split()[:2]
                rows.append((name, cand))

    exact = []
    for name, cand in rows:
        reason = banned(cand)
        if reason:
            print(f"{name:14s} {'REFUSED':22s} {cand} ({reason})")
            continue
        if trust:
            verdict = "EXACT (trusted)"
        else:
            r = subprocess.run([sys.executable, "tools/try_func.py", name, cand, "--no-budget"],
                               capture_output=True, text=True)
            verdict = (r.stdout.strip().splitlines() or ["COMPILE failed"])[-1]
            verdict = verdict.split(": ", 1)[-1].split("   (")[0]
        print(f"{name:14s} {verdict:22s} {cand}")
        if verdict.startswith("EXACT"):
            exact.append((name, cand))

    print(f"{len(exact)}/{len(rows)} exact")
    if apply and exact:
        took = claims.lock(owner)       # other agents land into the same files (tools/claims.py)
        try:
            for name, cand in exact:
                r = subprocess.run([sys.executable, "tools/apply_candidate.py", name, cand,
                                    "--drop-note"], capture_output=True, text=True)
                print((r.stdout or r.stderr).strip())
        finally:
            if took:
                claims.unlock(owner)


if __name__ == "__main__":
    main()
