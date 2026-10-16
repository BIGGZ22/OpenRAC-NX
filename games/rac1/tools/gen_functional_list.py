#!/usr/bin/env python3
"""
The functional-C list ByteMatch shows beside the progress report.

Functional C (nonmatching/functional.tsv, docs/NONMATCHING.md) does what a
retail function does without matching its bytes. It is never in src/ and
never in the progress report. ByteMatch can show it on its own line under
the Code bar, apart from the matched numbers, if CI uploads this file in the
same artifact as report.json. ByteMatch sizes each function from the report
and leaves out any that are matched by now.

Each function's confidence is read from its file's first line,
`/* functional: high|medium|low. ... */`; files without that line (near
misses kept as functional C) are listed without one.

Usage:
  python tools/gen_functional_list.py              write progress/functional.json
  python tools/gen_functional_list.py -o PATH      write it somewhere else

Needs no toolchain, baserom or build: only the list and the C files.
"""
import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LIST = ROOT / "nonmatching" / "functional.tsv"
CONFIDENCE = re.compile(r"functional:\s*(high|medium|low)\b", re.IGNORECASE)


def entries():
    for n, line in enumerate(LIST.read_text(encoding="utf-8").splitlines(), 1):
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        cols = line.split("\t")
        name = cols[0].strip()
        path = cols[1].strip() if len(cols) > 1 else ""
        entry = {"name": name}
        if path:
            entry["file"] = path
            try:
                with open(ROOT / path, encoding="utf-8", errors="replace") as f:
                    m = CONFIDENCE.search(f.readline())
                if m:
                    entry["confidence"] = m.group(1).lower()
            except OSError:
                print(f"{LIST.name}:{n}: {path} does not exist", file=sys.stderr)
        yield entry


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("-o", "--output", default=str(ROOT / "progress" / "functional.json"))
    args = ap.parse_args()
    functions = list(entries())
    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps({"functions": functions}, indent=1) + "\n", encoding="utf-8")
    rated = sum(1 for f in functions if "confidence" in f)
    print(f"{out}: {len(functions)} functions, {rated} with a confidence")


if __name__ == "__main__":
    main()
