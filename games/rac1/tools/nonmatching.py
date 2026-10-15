#!/usr/bin/env python3
"""
Near misses, shared: the best attempt at a level function that isn't exact
yet, kept as nonmatching/<source dir>/<func>.c (docs/NONMATCHING.md).
Nothing builds these files into the game: the function's retail assembly
stays in its src/overlays file until a candidate is EXACT. They are here so
the work is versioned and visible, packets start from them, and the
progress report can show how close each one is (fuzzy_match_percent).

  python tools/nonmatching.py stage func_LNN_X=CANDIDATE.c ...
      Builds each candidate in its file (tools/try_func.py's pipeline,
      strict tools/overlay_check.py), repairing declarations that clash
      with the file as it is now, and writes the staged file with its
      verdict and notes. A candidate that doesn't compile, or is EXACT
      (that's for wave.py salvage), is not staged.
  python tools/nonmatching.py check
      Re-checks every staged file against its source as it is now, rewrites
      each header's verdict, and regenerates nonmatching/README.md.
  python tools/nonmatching.py annotate
      Rewrites every header from what it already says, with no build: the
      current layout, and a line when tools/integrate.py would refuse the
      candidate as written (a #define, an expression alias).

Run it in the container (tools/docker/run.sh); builds run in parallel.
score() is what tools/gen_progress_report.py calls.
"""
from __future__ import annotations
import re
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

DIR = ROOT / "nonmatching"
WORK = ROOT / "build-sn/nonmatching"
HEADER_END = " */\n"
VERDICT = re.compile(r"^ \* Best so far: (.*?)(?: \(|, checked|$)", re.M)


def staged_path(name: str, source: Path) -> Path:
    return DIR / source.parent.name / f"{name}.c"


def staged() -> dict[str, Path]:
    """name -> its staged file, for every file under nonmatching/."""
    return {p.stem: p for p in sorted(DIR.glob("*/func_*.c"))}


def body(text: str) -> str:
    """A staged file's candidate, without its header."""
    return text.split(HEADER_END, 1)[1] if text.startswith("/* NON_MATCHING") and HEADER_END in text else text


def verdict_of(path: Path) -> str:
    m = VERDICT.search(path.read_text(errors="replace"))
    return m.group(1) if m else "?"


def closeness(verdict: str) -> float:
    """fuzzy_match_percent for a verdict: the share of bytes that match at
    the right size; 0 for a size mismatch, which moves every byte after it."""
    if verdict.startswith("EXACT"):
        return 100.0
    m = re.match(r"BYTES (\d+)/(\d+)", verdict)
    return round(100.0 * (1 - int(m.group(1)) / int(m.group(2))), 2) if m else 0.0


def build(name: str, text: str) -> tuple[str, str]:
    """(verdict, candidate text) for TEXT built in NAME's file, after up
    to five rounds of repair: an extern the file now declares another way
    is dropped, a typedef the file now also defines is renamed."""
    import try_func
    import overlay_check
    seg, src, first, last = try_func.find_stub(name)
    work = WORK / name
    for _ in range(5):
        obj = try_func.build(name, seg, src, first, last, text, work)
        if obj is not None:
            return overlay_check.check(obj, name), text
        log = (work / "log.txt").read_text(errors="replace")
        clash = set(re.findall(r"conflicting types for `(\w+)'", log)) - {name}
        redef = set(re.findall(r"redefinition of `(\w+)'", log))
        before = text
        if clash:
            text = "".join(l for l in text.splitlines(keepends=True)
                           if not (l.startswith("extern") and any(re.search(rf"\b{c}\b", l) for c in clash)))
        for x in redef:
            text = re.sub(rf"\b{x}\b", f"{x}_{name[-6:]}", text)
        if text == before:
            break
    return "COMPILE failed", text


def banned(text: str) -> str:
    """What tools/integrate.py would refuse in TEXT ('' if nothing): a
    staged candidate may carry one; it has to go before the function lands."""
    import integrate
    import tempfile
    with tempfile.NamedTemporaryFile("w", suffix=".c", delete=False) as f:
        f.write(text)
    try:
        return integrate.banned(f.name)
    finally:
        Path(f.name).unlink()


def header(name: str, source: Path, verdict: str, notes: list[str], refused: str = "",
           checked: str = "", broken: str = "") -> str:
    share = f" ({closeness(verdict):.1f}% of the bytes match)" if verdict.startswith("BYTES") else ""
    rel = source.relative_to(ROOT) if source.is_absolute() else source
    out = [f"/* NON_MATCHING {name} -- {rel}",
           f" * Best so far: {verdict}{share}, checked {checked or time.strftime('%Y-%m-%d')}.",
           " * Not built into anything: the retail assembly stays in the source file",
           " * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.",
           ]
    if refused:
        out.append(f" * Cannot land as written ({refused}): rewrite that in plain C first.")
    if broken:
        out.append(f" * {broken}")
    if notes:
        out.append(" * What the last attempts found:")
        out += [f" *   {n.replace('*/', '* /')}" for n in notes]
    return "\n".join(out) + "\n" + HEADER_END


def notes_for(name: str) -> list[str]:
    """The last lines of the function's NOTES.md files (workers' and arms')."""
    lines = []
    for p in sorted((ROOT / "build-sn/try" / name).glob("**/NOTES.md")):
        lines += [l.strip() for l in p.read_text(errors="replace").splitlines() if l.strip()]
    return [l[:110] for l in lines[-8:]]


def _stage_one(job: tuple[str, str]) -> str:
    import try_func
    name, cand = job
    _seg, src, _f, _l = try_func.find_stub(name)
    verdict, text = build(name, body((ROOT / cand).read_text(errors="replace")))
    if verdict.startswith("EXACT"):
        return f"{name}: EXACT, not staged (land it with wave.py salvage)"
    if verdict.startswith(("COMPILE", "LINK", "RODATA")):
        return f"{name}: {verdict}, not staged"
    out = staged_path(name, src)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(header(name, src, verdict, notes_for(name), banned(text)) + text)
    return f"{name}: staged, {verdict}"


def _check_one(path: Path) -> str:
    import try_func
    name = path.stem
    text = path.read_text(errors="replace")
    try:
        _seg, src, _f, _l = try_func.find_stub(name)
    except SystemExit:
        return f"{name}: no stub left (landed?), remove {path.relative_to(ROOT)}"
    verdict, cand = build(name, body(text))
    head = text.split(HEADER_END, 1)[0]
    old_notes = [l[5:] for l in head.splitlines() if l.startswith(" *   ")]
    old = verdict_of(path)
    if verdict.startswith(("COMPILE", "LINK", "RODATA")) and old.startswith(("BYTES", "SIZE")):
        # The file changed under the candidate (a neighbour landed with
        # other declarations): keep what it measured, say it needs fixing.
        when = re.search(r"checked (\d{4}-\d\d-\d\d)", head)
        broken = (f"No longer builds in its file ({verdict}, {time.strftime('%Y-%m-%d')}): "
                  "match its declarations to the file's first.")
        path.write_text(header(name, src, old, old_notes, banned(cand),
                               when.group(1) if when else "", broken) + cand)
        return f"{name}: {verdict}, kept {old}"
    path.write_text(header(name, src, verdict, old_notes, banned(cand)) + cand)
    return f"{name}: {verdict}"


def annotate(path: Path) -> None:
    """Rewrites PATH's header from what it already says (no build): the
    current header layout, and whether the candidate can land as written."""
    text = path.read_text(errors="replace")
    head, cand = text.split(HEADER_END, 1)
    m = re.match(r"/\* NON_MATCHING (\S+) -- (\S+)", head)
    notes = [l[5:] for l in head.splitlines() if l.startswith(" *   ")]
    when = re.search(r"checked (\d{4}-\d\d-\d\d)", head)
    broken = re.search(r"^ \* (No longer builds in its file .*)$", head, re.M)
    path.write_text(header(m.group(1), Path(m.group(2)), verdict_of(path), notes, banned(cand),
                           when.group(1) if when else "", broken.group(1) if broken else "") + cand)


def score(name: str, path: Path) -> float:
    """fuzzy_match_percent of a staged file against its source as it is."""
    verdict, _ = build(name, body(path.read_text(errors="replace")))
    return closeness(verdict)


def index() -> None:
    """nonmatching/README.md: one row per staged function, closest first."""
    import dossier
    catalogue = dossier.load_overlay_catalogue()
    rows = []
    for name, path in staged().items():
        v = verdict_of(path)
        size = catalogue.get(name, (None, 0))[1]
        rows.append((-closeness(v), name, path.parent.name, size, v, path))
    rows.sort()
    out = ["# Near misses", "",
           "Generated by `tools/nonmatching.py`; see [docs/NONMATCHING.md](../docs/NONMATCHING.md).",
           "Each file is the best attempt so far at a level function that isn't exact yet.", "",
           "| Function | Directory | Size | Best so far | Bytes matching |",
           "|---|---|---|---|---|"]
    for neg, name, d, size, v, path in rows:
        share = f"{-neg:.1f}%" if v.startswith("BYTES") else "-"
        flag = " (cannot land as written)" if "Cannot land as written" in path.read_text(errors="replace") else ""
        out.append(f"| [`{name}`]({d}/{name}.c) | {d} | {size} | {v}{flag} | {share} |")
    out.append("")
    (DIR / "README.md").write_text("\n".join(out))


def main() -> None:
    from concurrent.futures import ProcessPoolExecutor
    import os
    from toolchain import start_wineserver
    if sys.argv[1:2] == ["annotate"]:
        for path in staged().values():
            annotate(path)
        index()
        return
    if sys.argv[1:2] == ["stage"]:
        jobs = [tuple(a.split("=", 1)) for a in sys.argv[2:]]
        fn, items = _stage_one, jobs
    elif sys.argv[1:2] == ["check"]:
        fn, items = _check_one, list(staged().values())
    else:
        sys.exit(__doc__)
    start_wineserver()
    with ProcessPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        for line in pool.map(fn, items):
            print(line, flush=True)
    index()


if __name__ == "__main__":
    main()
