#!/usr/bin/env python3
"""
Put a candidate that try_func.py passed into src/: it replaces the
function's INCLUDE_ASM line, or its current definition when the function
is already C (a near-miss being fixed).

  python tools/apply_candidate.py func_X cand.c
  python tools/apply_candidate.py func_X cand.c --comment "why it matches"
  python tools/apply_candidate.py func_X cand.c --drop-note --comment "..."

--comment puts a block comment right above the definition, replacing a
one-line comment the candidate has there. --drop-note removes the block
comment that ends right above the stub or definition, blank lines aside:
the old revert or near-miss note that the match makes obsolete. Without
--comment, a stub's trailing name comment (`INCLUDE_ASM(...); /* Name */`)
is kept, unless the candidate has a comment of its own above the
definition. Extern and #include lines the file already has are left out.

The full build decides, as always: run tools/build_sn.sh afterwards.
"""
import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from try_func import find_stub, STUB  # noqa: E402
import provenance  # noqa: E402

TRAILING = re.compile(r"\)\s*;\s*(/\*.*\*/)\s*$")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("name")
    ap.add_argument("candidate")
    ap.add_argument("--comment")
    ap.add_argument("--drop-note", action="store_true")
    a = ap.parse_args()

    _seg, src, first, last = find_stub(a.name)
    sample = provenance.sony_sample(Path(a.candidate).read_text())
    if sample:
        sys.exit(f"{a.candidate}: mentions '{sample}', a name from Sony's sample source (docs/MOVIE.md)")
    if provenance.is_movie_file(src) and not provenance.movie_provenance(a.name):
        sys.exit(f"{a.name} lands in {src}: movie code needs a row in "
                 f"config/movie_provenance.tsv (docs/MOVIE.md)")
    lines = src.read_text().splitlines()
    is_stub = STUB.match(lines[first]) is not None
    m = TRAILING.search(lines[first]) if is_stub else None
    name_comment = m.group(1) if m else None

    cand = Path(a.candidate).read_text().rstrip("\n").splitlines()
    # The file already declares most of what a candidate carries: keep only
    # the extern lines not declared above it, and one blank line where others
    # went. A declaration further down doesn't count: C needs it first.
    existing = {l.strip() for l in lines[:first]}
    cand = [l for l in cand if not (l.startswith(("extern ", "#include")) and l.strip() in existing)]
    cand = [l for i, l in enumerate(cand) if l.strip() or (i and cand[i - 1].strip())]
    while cand and not cand[0].strip():
        cand.pop(0)
    # The definition's first line: its signature may wrap onto more lines.
    d = next((i for i, l in enumerate(cand)
              if re.match(rf"^(?!extern\b)[A-Za-z_][\w \t\*]*\b{a.name}(?:_r)?\s*\(", l)
              and not l.rstrip().endswith(";")), None)
    if d is None:
        sys.exit(f"{a.candidate}: no definition of {a.name}")
    if a.comment:
        if d > 0 and re.match(r"^\s*/\*.*\*/\s*$", cand[d - 1]):
            cand.pop(d - 1)
            d -= 1
        body = a.comment.strip().splitlines()
        block = ["/* " + body[0]] + ["   " + l if l else "" for l in body[1:]]
        block[-1] += " */"
        cand[d:d] = block
    elif (name_comment and name_comment not in "\n".join(cand)
          and not (d > 0 and cand[d - 1].rstrip().endswith("*/"))):
        cand.insert(d, name_comment)

    start = first
    note = first - 1
    while note > 0 and not lines[note].strip():  # A note may sit a blank line up.
        note -= 1
    if a.drop_note and lines[note].rstrip().endswith("*/"):
        opener = note
        while "/*" not in lines[opener]:
            opener -= 1
        # Only a comment standing alone is a note, never one trailing code.
        if lines[opener].lstrip().startswith("/*"):
            start = opener
    lines[start:last + 1] = cand
    src.write_text("\n".join(lines) + "\n")
    print(f"{a.name}: {src}:{start + 1} ({'stub' if is_stub else 'definition'} replaced, {len(cand)} lines)")


if __name__ == "__main__":
    main()
