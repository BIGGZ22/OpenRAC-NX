#!/usr/bin/env python3
"""
Version-specific C without a model: the shared C with what this version changed.

  python3 tools/version_clone.py --version jp [FUNC ...] [-P 12]      # inside the build container on a Mac

For each function whose shared C builds the primary's bytes but not this version's (status
`differs` or `refs` in config/versions/<K>/map.tsv), the shared C is compiled, placed and relocated
for this version exactly as tools/version_check.py does, and the result is compared with the
version's bytes word by word. Where they are the same size it derives the edits that explain every
difference, and tries each candidate with tools/version_src.py try:

  - numbers: an instruction that differs only in its immediate and is not relocated (a 50 against
    a 60 Hz count, a field offset, a flag): every literal of the shared C's definition that spells
    the old value takes the new one (tools/overlay_variants.py's number_changes()/candidates());
  - references: a relocated field (a call target, a %hi/%lo pair, a $gp offset) that reaches
    another address here than the translation gives: the symbol it names is renamed, in this
    definition only, to the one at the address this version uses with the same addend. A call
    goes to the function this version has there (its counterpart's name, or func_<TAG>_... for
    code only this version has); data takes the primary's name of what translates to it, else
    D_<TAG>_... (data only this version has). The renamed symbol gets the declaration of the one it
    replaces, written before the definition. A symbol whose uses would need two different targets
    is left alone.

A candidate is kept only when it is EXACT, as the function's version-specific definition
(src/versions/<K>/...). Functions whose size changed, or that differ in more than this, are listed
for people (or worker agents).
"""
import argparse
import os
import re
import struct
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
os.chdir(ROOT)
sys.path.insert(0, str(ROOT / "tools"))
import versions  # noqa: E402

PAL_MAIN = 0x15F000


def words(b):
    return struct.unpack("<%dI" % (len(b) // 4), b[:len(b) // 4 * 4])


def target_path(v, name, src):
    return ROOT / "src/versions" / v.key / str(Path(src))[len("src/"):-2] / f"{name}.c"


def definition(src, name):
    """(first line, last line, lines) of NAME's definition in SRC (srcnames' canonical text)."""
    import srcnames
    lines = srcnames.read_c(src, errors="replace").splitlines()
    first = next((i for i, l in enumerate(lines)
                  if re.match(rf"^(?!extern\b)[A-Za-z_].*\b{name}(?:_r)?\s*\(", l) and not l.rstrip().endswith(";")), None)
    if first is None:
        return None
    depth, seen = 0, False
    for last in range(first, len(lines)):
        depth += lines[last].count("{") - lines[last].count("}")
        seen = seen or "{" in lines[last]
        if seen and depth == 0:
            return first, last, lines
    return None


def declaration(sym, lines):
    """A file-level declaration of SYM: a line of the shared file, else of a header."""
    pat = re.compile(rf"^(?:extern\b|[A-Za-z_])[^;{{}}]*\b{sym}\b[^;{{}}]*;\s*(?://.*|/\*.*)?$")
    for l in lines:
        if pat.match(l) and not l.startswith((" ", "\t")):
            return l.strip()
    for h in sorted((ROOT / "include").rglob("*.h")):
        for l in h.read_text(errors="replace").splitlines():
            if pat.match(l):
                return l.strip()
    return None


class Namer:
    def __init__(self, v, table):
        import version_check as vc
        self.v, self.t, self.vc = v, table, vc
        self.by_place = {}
        for e in vc.version_map.read(v):
            for sp, a in e["places"]:
                self.by_place.setdefault((sp, a), e["name"])
        self.pal_names = {}
        for name, places in vc.primary_places().items():
            for sp, a in places:
                self.pal_names.setdefault((sp, a), name)

    def space(self, ctx, x, main):
        return ctx if ctx != "boot" and x >= main else "boot"

    def name(self, ctx, x, call):
        """The C name this version's address X takes, as code in CTX refers to it."""
        v, t = self.v, self.t
        sp = self.space(ctx, x, t.x_main)
        e = self.by_place.get((sp, x))
        if e is not None and (not re.match(r"^func_L\d\d_", e) or e.startswith(f"func_L{ctx:02d}_" if ctx != "boot" else "func_L")):
            if not e.startswith(f"func_{v.tag}_") and t(ctx, self.vc.primary_places()[e][0][1]) != x:
                pass
            return e
        if call:
            return None
        p = t.inverse(ctx, x)
        if p is not None:
            psp = ctx if ctx != "boot" and p >= PAL_MAIN else "boot"
            if (psp, p) in self.pal_names:
                return self.pal_names[(psp, p)]
            return f"D_L{ctx:02d}_{p:08X}" if psp != "boot" else f"D_{p:08X}"
        return f"D_{v.tag}_L{ctx:02d}_{x:08X}" if sp != "boot" else f"D_{v.tag}_{x:08X}"


def ref_edits(v, table, namer, name, ctx, ours, theirs, fields):
    """{old symbol: new symbol} that turn OURS' relocated fields into THEIRS', or None."""
    sext = lambda i: i - 0x10000 if i & 0x8000 else i
    w = lambda b, o: int.from_bytes(b[o:o + 4], "little")
    want = {}
    for kind, offs, sym, got, addend in fields:
        if any(o is not None and w(ours, o) != w(theirs, o) for o in offs):
            if not sym:
                return None
            if kind == "hilo":
                if offs[1] is None:
                    return None
                x = ((w(theirs, offs[0]) & 0xFFFF) << 16) + sext(w(theirs, offs[1]) & 0xFFFF)
            elif kind == "j":
                x = ((w(theirs, offs[0]) & 0x3FFFFFF) << 2) | (got & 0xF0000000)
            elif kind == "gp":
                x = table.x_gp + sext(w(theirs, offs[0]) & 0xFFFF)
            else:
                return None
            base = (x - addend) & 0xFFFFFFFF
            if want.setdefault(sym, (kind, base))[1] != base:
                return None
    out = {}
    for sym, (kind, base) in want.items():
        new = namer.name(ctx, base, kind == "j")
        if new is None:
            new = (f"func_{v.tag}_{base:08X}" if namer.space(ctx, base, table.x_main) == "boot"
                   else f"func_{v.tag}_L{ctx:02d}_{base:08X}")
        if new == sym:
            return None
        out[sym] = new
    return out


def one(key, name):
    import overlay_variants as ovv
    import version_check as vc
    import version_src
    v = versions.get(key)
    if key not in _TABLES:
        _TABLES[key] = vc.Table(v)
    table = _TABLES[key]
    e = vc.entries_by_name(v)[name]
    mine = sorted(e["places"], key=lambda p: (-1 if p[0] == "boot" else p[0], p[1]))
    pp = vc.primary_places()[name]
    sp, xa = next((p for p in mine if p[0] in {q[0] for q in pp}), mine[0])
    pa = sorted(a for s, a in pp if s == sp)
    if not pa:
        return name, "skipped: no place in the same program"
    size = vc.primary_sizes()[name]
    if size != e["size"]:
        return name, f"skipped: size {size} here {e['size']}"
    src = vc.read_map(v)[name]["source"]
    found = definition(src, name)
    if found is None:
        return name, "skipped: definition not found"
    first, last, lines = found
    body = "\n".join(lines[first:last + 1])
    # the shared C, relocated for this version, against this version's bytes
    cands = [c for c in vc.object_index().get(name, []) if not c[0].startswith("build-sn/versions/")]
    if not cands:
        return name, "skipped: no object"
    theirs = vc.code_bytes(v, sp, xa, size)
    try:
        ours, masked, osize, unresolved = vc.relocate(cands[0][0], name, sp, pa[0], table, table.x_gp, theirs + bytes(64))
    except Exception as ex:      # noqa: BLE001
        return name, f"skipped: {type(ex).__name__}"
    if osize != size or unresolved:
        return name, "skipped: does not place"
    fields, touched = vc.relocate.fields, vc.relocate.touched
    renames = ref_edits(v, table, Namer(v, table), name, sp, ours, theirs, fields)
    if renames is None:
        return name, "skipped: references that one rename cannot explain"
    # numbers: only the words that are not relocated
    na = tuple(x for i, x in enumerate(words(ours)) if not any(i * 4 + k in touched for k in range(4)))
    nb = tuple(x for i, x in enumerate(words(theirs)) if not any(i * 4 + k in touched for k in range(4)))
    changes = ovv.number_changes(na, nb)
    if changes is None:
        return name, "skipped: differs in more than numbers and references"
    externs = []
    for old, new in renames.items():
        if not re.search(rf"\b{old}\b", body):
            return name, f"skipped: {old} is not spelled in the C"
        body = re.sub(rf"\b{old}\b", new, body)
        if not any(re.search(rf"\b{new}\b", l) for l in lines[:first]):
            decl = declaration(old, lines)
            if decl is None:
                return name, f"skipped: no declaration of {old} to copy"
            externs.append(re.sub(rf"\b{old}\b", new, decl))
    work = ROOT / "build-sn/versions" / v.key / "clone" / name
    work.mkdir(parents=True, exist_ok=True)
    verdict = "no literal for the changed numbers"
    for k, cand in enumerate(ovv.candidates(body, changes) if changes else [body]):
        if cand == "\n".join(lines[first:last + 1]):
            continue
        text = "\n".join(dict.fromkeys(externs)) + ("\n" if externs else "") + cand + "\n"
        path = work / f"clone{k}.c"
        path.write_text(text)
        verdict = version_src.try_one(v, name, path)
        if verdict.startswith("EXACT"):
            out = target_path(v, name, src)
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_text(text)
            return name, verdict + f" -> {out.relative_to(ROOT)}"
        if k >= 15:
            break
    return name, verdict


_TABLES = {}
_SLOT = None


def worker_prefix(slot, prefixes):
    """A worker's own Wine prefix (RAC1_WINEPREFIXES, separated by ':'): one wineserver per
    prefix serialises its compiles, so parallel workers each take one."""
    with slot.get_lock():
        i = slot.value
        slot.value += 1
    os.environ["RAC1_WINEPREFIX"] = prefixes[i % len(prefixes)]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("names", nargs="*")
    ap.add_argument("--version", required=True)
    ap.add_argument("-P", "--parallel", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--shard", default="0/1", help="I/N: only every N-th function from the I-th (several machines)")
    a = ap.parse_args()
    v = versions.get(a.version)
    import version_check as vc
    import version_src
    have = version_src.overrides(v)
    todo = a.names or sorted(n for n, r in vc.read_map(v).items()
                             if r["status"] in ("differs", "refs") and r["source"].startswith("src/") and n not in have)
    i, n = map(int, a.shard.split("/"))
    todo = todo[i::n]
    from toolchain import start_wineserver
    start_wineserver()
    import multiprocessing
    prefixes = [p for p in os.environ.get("RAC1_WINEPREFIXES", "").split(":") if p]
    kw = {}
    if prefixes:
        kw = {"initializer": worker_prefix, "initargs": (multiprocessing.Value("i", 0), prefixes)}
    exact = 0
    with ProcessPoolExecutor(a.parallel, **kw) as ex:
        for name, verdict in ex.map(one, [v.key] * len(todo), todo):
            print(f"{name}: {verdict}", flush=True)
            exact += verdict.startswith("EXACT")
    print(f"{v.key}: {exact} of {len(todo)} cloned exactly")


if __name__ == "__main__":
    main()
