#!/usr/bin/env python3
"""
Carries a matched function into another version that holds the same machine
code: takes the C from the project that matched it, with the declarations it
needs, and renames every symbol to the target's. The result is a candidate for
the target project's own check, never a match by itself.

  python3 tools/port.py FROM TO [NAME ...] [--out DIR]     e.g. rac1/ntsc rac1/pal
  python3 tools/port.py FROM TO [NAME ...] --check [--target-dir DIR] [--jobs N]

It reads the `same` rows of shared/xmap/ports/FROM--TO.tsv (tools/xmap.py):
functions FROM has matched whose code TO also has, at another address, and has
not matched. NAME limits it to some of them, by the name either project uses.
It writes, under build/port/FROM--TO/ unless --out says otherwise:

  <target name>.c   the candidate
  MANIFEST          `<target name> <candidate>` lines, the form rac1/pal's tools/integrate.py reads
  ports.tsv         every row with what happened to it

How a symbol is translated. The two functions are the same instructions, so
the n-th address one forms is the n-th the other forms (mips.references). A
symbol the source C uses has an address in the source version; the references
that reach it give the address in the target, and the target names it its own
way. Nothing is looked up in a table of data symbols, so it works for any
global the function touches.

How a global is declared follows the code as well: one the function reaches
only through $gp is small data in the target, one it reaches with lui and %lo
is not, whatever the source project wrote to get there (its compiler differs).
A global the source declares MACRO_ADDR keeps that: the assembler then picks
the form of each access, which the target's check has to reproduce.

--check then runs the target project's own check on every candidate (in
--target-dir, or the version's directory under games/), reads the compiler's
complaints about names its file already uses, writes hints, and tries again,
up to four rounds. results.tsv has each function's verdict and MANIFEST.exact
the ones that passed; landing them is the target project's own step.

`<target name>.hints.json` next to a candidate, {"alias": [...], "rename":
[...]}, changes how it is written: a symbol in `alias` is declared under a
private name with an assembler label (for a file that already declares it
with another type), and a type in `rename` gets a suffix (for a file that
already has one of that name). --check writes them.

Each version's side is the "port" entry of its games/<game>/game.json:

  as a source   sources, include, defines (what to read and how to preprocess
                it), gp, catalogue and symbols (where its names are),
                shared_headers (headers the target has its own copy of),
                refuse_headers (headers whose contents do not travel), types
                and phrases (spellings to translate), credit
  as a target   gp, catalogue, report, level_data_from and names (how it names
                an address), alias and small_data (how it declares: rac1/pal
                by plain names and a short read through a cast, rac1/ntsc
                under private names with an sda attribute), headers,
                never and never_sources (what it does not take), allow (a
                construct a named function may keep after review), check

A check is either a command over a manifest (rac1/pal's tools/integrate.py),
or `"kind": "guard"`: the candidate is put under `#else` of a NON_MATCHING
guard in the target's own file, as rac1/ntsc keeps pending C, checked there
and taken out again; what passes is also written as exact.patch against the
target's tree. "alternatives" lists other readings of a rule to try on a
candidate that misses.
"""
import json
import re
import subprocess
import sys
from bisect import bisect_right
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import mips  # noqa: E402
import xmap  # noqa: E402
from openrac_common import paths, tables  # noqa: E402

ROOT = paths.repo_root()
PORTS = ROOT / "shared/xmap/ports"
ADDRESSED = re.compile(r"^(D|FUN|func|jtbl)_(?:L(\d\d)_)?([0-9A-Fa-f]{8})$")
IDENT = re.compile(r"[A-Za-z_]\w*")
KEYWORDS = set("""auto break case char const continue default do double else enum extern float for goto if
    inline int long register return short signed sizeof static struct switch typedef union unsigned void
    volatile while __inline__ __inline __attribute__ __asm__ asm __volatile__ __extension__ __const
    __signed__ __typeof__ typeof""".split())
SMALL_TYPES = {"char", "short", "s8", "u8", "s16", "u16"}        # two bytes or less: small data by size alone
REACH = 0x100000         # a reference this far above a symbol is not into that object (a constant such as 0x20000000)


class Skip(Exception):
    """Why one function was not ported."""


# --- The C side ----------------------------------------------------------------

def blank(text: str) -> str:
    """TEXT with comments, strings and character constants replaced by spaces,
    newlines kept: the same length, so offsets in one are offsets in the other."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(c + "".join("\n" if ch == "\n" else " " for ch in text[i + 1:j - 1]) + c if j - i >= 2 else c)
            i = j
            continue
        else:
            out.append(c)
            i += 1
            continue
        out.append("".join("\n" if ch == "\n" else " " for ch in text[i:j]))
        i = j
    return "".join(out)


def closing(clean: str, i: int) -> int:
    """Index just past the bracket that closes the one at CLEAN[i]."""
    pairs, depth = {"(": ")", "{": "}", "[": "]"}, 0
    close = pairs[clean[i]]
    for j in range(i, len(clean)):
        if clean[j] == clean[i]:
            depth += 1
        elif clean[j] == close:
            depth -= 1
            if not depth:
                return j + 1
    return len(clean)


def without(clean: str, pattern: str) -> str:
    """CLEAN with every `PATTERN ( ... )` blanked: __attribute__((...)), __asm__("...")."""
    out, i = clean, 0
    for m in re.finditer(pattern + r"\s*\(", clean):
        if m.start() < i:
            continue
        i = closing(clean, m.end() - 1)
        out = out[:m.start()] + " " * (i - m.start()) + out[i:]
    return out


def items(text: str) -> list[dict]:
    """The file-scope pieces of preprocessed C: {start, end, origin, kind,
    names, static}. kind is `function` (a definition), `typedef`, `tag` (a
    struct, union or enum declared alone), `decl` (an object or a prototype)
    or `asm`. names are what the piece declares. origin is the file a
    preprocessor line marker last named."""
    clean = blank(text)
    out, origin, i, n = [], "", 0, len(text)
    while i < n:
        if clean[i].isspace():
            i += 1
            continue
        if clean[i] == "#":                                     # a line marker or a directive
            j = clean.find("\n", i)
            j = n if j < 0 else j
            m = re.match(r'#\s*(?:line\s+)?\d+\s+"([^"]*)"', text[i:j])
            origin = m.group(1) if m else origin
            i = j
            continue
        j, body = i, False
        while j < n:
            c = clean[j]
            if c in "([":
                j = closing(clean, j)
            elif c == "{":
                body = clean[:j].rstrip().endswith(")")         # a function's body, not a struct's or an initializer's
                j = closing(clean, j)
                if body:
                    break
            elif c == ";":
                j += 1
                break
            else:
                j += 1
        out.append({"start": i, "end": j, "origin": origin, **describe(clean[i:j], body)})
        i = j
    return out


def declarators(clean: str) -> list[str]:
    """One declaration split at its top-level commas, braces blanked."""
    flat, i = [], 0
    while i < len(clean):
        if clean[i] in "{":
            j = closing(clean, i)
            flat.append(" " * (j - i))
            i = j
        else:
            flat.append(clean[i])
            i += 1
    flat = "".join(flat).rstrip().rstrip(";")
    parts, depth, last = [], 0, 0
    for k, c in enumerate(flat):
        depth += c in "(["
        depth -= c in ")]"
        if c == "," and not depth:
            parts.append(flat[last:k])
            last = k + 1
    return parts + [flat[last:]]


def declared(part: str) -> str | None:
    """The name one declarator declares: `(*name)` for a pointer to function,
    else the last identifier before its first bracket or initializer."""
    part = part.split("=")[0]
    m = re.search(r"\(\s*\*+\s*([A-Za-z_]\w*)\s*\)\s*\(", part)
    if m:
        return m.group(1)
    head = re.split(r"[(\[]", part, maxsplit=1)[0]
    names = [w for w in IDENT.findall(head) if w not in KEYWORDS]
    return names[-1] if names else None


def describe(clean: str, body: bool) -> dict:
    plain = without(without(clean, r"\b__attribute__"), r"\b(?:__asm__|asm)")
    first = IDENT.match(plain.lstrip())
    words = IDENT.findall(plain.split("{")[0])
    names = set(re.findall(r"\b(?:struct|union|enum)\s+([A-Za-z_]\w*)\s*\{", plain))
    for m in re.finditer(r"\benum\b[^{;]*\{([^}]*)\}", plain):
        names |= {e.split("=")[0].strip() for e in m.group(1).split(",")} - {""}
    static = "static" in words
    if clean.lstrip().startswith(("__asm__", "asm")):
        return {"kind": "asm", "names": set(), "static": False}
    if body:
        return {"kind": "function", "names": {declared(plain.split("{")[0])} - {None}, "static": static}
    kind = "typedef" if first and first.group() == "typedef" else "decl"
    found = {declared(p) for p in declarators(plain)} - {None}
    if kind == "decl" and re.fullmatch(r"\s*(?:struct|union|enum)\s+\w+\s*\{.*\}\s*;?\s*", plain, re.S):
        return {"kind": "tag", "names": names, "static": False}
    forward = re.fullmatch(r"\s*(?:struct|union|enum)\s+(\w+)\s*;?\s*", plain)
    if kind == "decl" and forward:                      # `struct X;` declares the tag, not an object
        return {"kind": "tag", "names": {forward.group(1)}, "static": False}
    return {"kind": kind, "names": names | found, "static": static}


def idents(clean: str) -> set[str]:
    return set(IDENT.findall(clean)) - KEYWORDS


def substitute(text: str, clean: str, table: dict[str, str], phrases: list = ()) -> str:
    """TEXT with each identifier that is a key of TABLE replaced, and each
    (pattern, replacement) of PHRASES, outside comments and strings."""
    spans = [(m.start(), m.end(), table[m.group()]) for m in IDENT.finditer(clean) if m.group() in table]
    for pattern, new in phrases:
        spans += [(m.start(), m.end(), new) for m in re.finditer(pattern, clean)]
    out, last = [], 0
    for start, end, new in sorted(spans):
        if start >= last:
            out.append(text[last:start] + new)
            last = end
    return "".join(out) + text[last:]


def preprocess(path: Path, include: list[Path], defines: list[str]) -> str:
    cmd = ["cc", "-E", "-C", "-undef", "-nostdinc", "-x", "c", "-Wno-everything",
           *[f"-I{d}" for d in include], *[f"-D{d}" for d in defines], str(path)]
    done = subprocess.run(cmd, capture_output=True, text=True)
    if done.returncode:
        raise Skip("the source file does not preprocess: " + (done.stderr.strip().splitlines() or ["?"])[0][:120])
    return done.stdout


# --- The two versions ----------------------------------------------------------

def catalogue(path: Path) -> tuple[dict, dict]:
    """A functions.tsv (rac1's two projects share the format): name -> {level:
    [addresses]} (a level can hold two copies of one function), and (level,
    address) -> name."""
    places, at = {}, {}
    for cols in tables.read_tsv(path):
        if len(cols) < 6:
            continue
        for place in cols[5].split(","):
            level, addr = place.split(":")
            places.setdefault(cols[0], {}).setdefault(int(level), []).append(int(addr, 16))
            at[(int(level), int(addr, 16))] = cols[0]
    return places, at


def side(key: str) -> dict:
    version = xmap.versions()[key]
    cfg = version.get("port")
    if not cfg:
        raise SystemExit(f"{key} has no \"port\" entry in its game.json")
    out = dict(cfg, key=key, gp=int(cfg["gp"], 16), programs=xmap.program_files(version), loaded={})
    out["places"], out["at"] = catalogue(ROOT / cfg["catalogue"]) if "catalogue" in cfg else ({}, {})
    out["symbols"] = {}
    if "symbols" in cfg and (ROOT / cfg["symbols"]).is_file():
        for m in re.finditer(r"^(\w+)\s*=\s*0x([0-9A-Fa-f]+)", (ROOT / cfg["symbols"]).read_text(), re.M):
            out["symbols"][m.group(1)] = int(m.group(2), 16)
    out["names_at"] = {}
    for addr, _size, name, _done in xmap.known(key, version).get("boot", []):      # the project's own names for its functions
        out["symbols"].setdefault(name, addr)
        out["names_at"].setdefault(addr, name)
    out["functions"] = set()
    if "report" in cfg and (ROOT / cfg["report"]).is_file():
        report = json.loads((ROOT / cfg["report"]).read_text())
        out["functions"] = {f["name"] for u in report["units"] for f in u.get("functions", [])}
    return out


def code(v: dict, program: str, address: int, size: int) -> bytes:
    if program not in v["programs"]:
        raise Skip(f"{v['key']}'s {program} is not on disk (docs/engine/SHARED_CODE.md, \"Running it\")")
    if program not in v["loaded"]:
        v["loaded"][program] = xmap.load(*v["programs"][program])
    for start, data in v["loaded"][program].values():
        if start <= address < start + len(data):
            return data[address - start: address - start + size]
    raise Skip(f"{address:08X} is not in {v['key']}'s {program}")


def level_of(program: str) -> int | None:
    return int(program[6:]) if program.startswith("level:") else None


def source_addresses(v: dict, name: str, level: int | None) -> list[int]:
    """Where the source version has the symbol NAME, seen from LEVEL's program:
    one address, or several for a function the level holds more than once."""
    if level is not None and name in v["places"]:
        return v["places"][name].get(level, [])
    m = ADDRESSED.match(name)
    if m:
        return [int(m.group(3), 16)]
    addr = v["symbols"].get(name)
    if addr is not None and level is not None:
        moved = v["places"].get(f"FUN_{addr:08x}") or v["places"].get(f"func_{addr:08X}")
        return moved.get(level, []) if moved else [addr]
    return [] if addr is None else [addr]


NAMES = {"function": "func_{addr:08X}", "data": "D_{addr:08X}", "level_data": "D_L{level:02d}_{addr:08X}"}


def target_name(v: dict, addr: int, level: int | None, function: bool) -> str | None:
    """What the target project calls ADDR in LEVEL's program: its catalogue's
    name for level code, else a name made from the address by the patterns in
    its "port" entry ("names"; rac1/pal's when it gives none)."""
    if level is not None and (level, addr) in v["at"]:
        return v["at"][(level, addr)]
    names = {**NAMES, **v.get("names", {})}
    limit = int(v["level_data_from"], 16)
    as_function = v.get("names_at", {}).get(addr) if level is None else None
    as_function = as_function or names["function"].format(addr=addr)
    if function or as_function in v["functions"]:
        # A resident function keeps its address in every program; level code is named by the catalogue alone.
        return as_function if level is None or addr < limit else None
    return names["data"].format(addr=addr) if level is None or addr < limit else names["level_data"].format(addr=addr, level=level)


# --- One function --------------------------------------------------------------

def link_name(text: str) -> str | None:
    m = re.search(r'\b(?:__asm__|asm)\s*\(\s*"([^"]+)"\s*\)', text)
    return m.group(1) if m else None


def pieces(text: str, name: str) -> tuple[list[dict], dict]:
    """The definition of NAME in preprocessed TEXT and the file-scope pieces it
    needs, in file order. A static function it calls comes whole; another
    function of the file comes as its prototype."""
    clean, all_items = blank(text), items(text)
    for it in all_items:
        it["clean"] = clean[it["start"]:it["end"]]
        it["text"] = text[it["start"]:it["end"]]
    target = next((it for it in all_items if it["kind"] == "function" and name in it["names"]), None)
    if not target:
        raise Skip(f"no definition of {name} in its file")
    # The comment right above the definition says what the function does: it travels with it.
    gap = text[max([it["end"] for it in all_items if it["end"] <= target["start"]], default=0):target["start"]]
    above = re.search(r"/\*((?:(?!\*/).)*)\*/\s*$", gap, re.S)
    target["about"] = " ".join(above.group(1).replace("\n *", "\n").split()) if above and not gap[:above.start()].strip() else ""
    need, chosen, grew = idents(target["clean"]), [], True
    while grew:
        grew = False
        for it in all_items:
            if it is target or it in chosen or it["kind"] == "asm" or not it["names"] & need:
                continue
            if it["kind"] == "function" and not it["static"]:
                it = dict(it, kind="decl", proto=True, clean=it["clean"].split("{")[0].rstrip() + ";",
                          text=it["text"][:it["clean"].index("{")].rstrip() + ";")
                if any(c.get("proto") and c["names"] == it["names"] for c in chosen):
                    continue
            chosen.append(it)
            need |= idents(it["clean"])
            grew = True
    return sorted(chosen, key=lambda it: it["start"]), target


def align(src_refs: list, dst_refs: list) -> list[tuple[str, int, int, int]]:
    """(kind, address in the source, address in the target, instruction index) per reference."""
    if [(i, k) for i, k, _ in src_refs] != [(i, k) for i, k, _ in dst_refs]:
        raise Skip("the two versions form their addresses at different instructions")
    return [(k, a, b, i) for (i, k, a), (_, _, b) in zip(src_refs, dst_refs)]


def through_macro(words: tuple, i: int) -> bool:
    """Whether the access at instruction I has the shape of the assembler's own
    expansion of `lw $2,SYM`: through $at, or a load whose base is the register
    it loads, right after that register's `lui`. The compiler, splitting an
    address itself, loads through another register."""
    w = words[i]
    op, rs, rt = w >> 26, (w >> 21) & 31, (w >> 16) & 31
    if rs == 1:
        return True
    return op in (*range(0x20, 0x28), 0x37) and rs == rt and i > 0 and words[i - 1] >> 16 == 0x3C00 | rt


def resolve(links: dict[str, list[int]], pairs: list) -> dict[str, tuple[int, set, list]]:
    """link name -> (its address in the target, how the code reaches it: `call`,
    `data`, `gp`, and the instructions that do). LINKS gives each name's address in the source version (more
    than one for a function its level holds twice: the one the code reaches
    counts). A reference belongs to the nearest symbol at or below it; of a
    symbol's references the nearest decides, and the ones that then disagree
    are another object's (a jump table, a string) and are left alone; so is
    one a megabyte or more above every symbol, which is a constant, not an
    address."""
    order = sorted((a, n) for n, addrs in links.items() for a in addrs)
    starts = [a for a, _ in order]
    seen: dict[str, list] = {}
    for kind, src, dst, *where in pairs:
        k = bisect_right(starts, src) - 1
        if k < 0 or kind == "call" and starts[k] != src or src - starts[k] >= REACH:
            continue
        for addr, name in order[bisect_right(starts, starts[k] - 1):k + 1]:      # every name at that address
            seen.setdefault(name, []).append((src - addr, dst - (src - addr), kind, *where))
    out = {}
    for name in links:
        if name not in seen:
            raise Skip(f"the code never reaches {name}")
        base = min(seen[name])[1]
        if len({r[1] for r in seen[name] if r[0] == 0 and r[2] == "call"}) > 1:
            raise Skip(f"{name} names two functions this code calls (the source lists them as one)")
        mine = [r for r in seen[name] if r[1] == base]
        out[name] = (base, {r[2] for r in mine}, [(r[2], r[3]) for r in mine if len(r) > 3])
    return out


def object_parts(it: dict, name: str) -> tuple[str, str]:
    """(type, what follows the name) of one object in a declaration: `extern
    s32 x[2] __attribute__((sda));` gives ("s32", "[2]"), and `extern char
    *a, b[4];` gives ("char *", "") for a and ("char", "[4]") for b."""
    plain = without(without(it["clean"], r"\b__attribute__"), r"\b(?:__asm__|asm)").rstrip().rstrip(";")
    flat, i = [], 0                                     # the same text with every { ... } blanked: only declarators left to read
    while i < len(plain):
        j = closing(plain, i) if plain[i] == "{" else i + 1
        flat.append(" " * (j - i) if plain[i] == "{" else plain[i])
        i = j
    flat = "".join(flat)
    cuts, depth = [0], 0
    for k, c in enumerate(flat):
        depth += c in "(["
        depth -= c in ")]"
        if c == "," and not depth:
            cuts.append(k + 1)
    spans = list(zip(cuts, [c - 1 for c in cuts[1:]] + [len(flat)]))
    first = re.search(rf"\b{re.escape(declared(flat[spans[0][0]:spans[0][1]]) or name)}\b", flat[:spans[0][1]])
    stop = cut = first.start() if first else spans[0][1]
    while cut and plain[cut - 1] in " \t\n*":             # the first declarator's own stars are not the type's
        cut -= 1
    base = re.sub(r"\b(?:extern|static)\b", " ", plain[:cut])
    for start, end in spans:
        chunk = flat[start:end]
        m = re.search(rf"\b{re.escape(name)}\b", chunk)
        if m and declared(chunk) == name:
            lead = chunk[:m.start()] if start else flat[cut:stop]
            stars = "*" * lead.count("*")
            return " ".join(base.split()) + (" " + stars if stars else ""), chunk[m.end():].split("=")[0].strip()
    raise Skip(f"cannot read the declaration of {name}")


def port(row: dict, src: dict, dst: dict, hints: dict) -> tuple[str, str]:
    """(target name, candidate text) for one port row, or raises Skip."""
    s_level, d_level = level_of(row["from_program"]), level_of(row["to_program"])
    s_addr, d_addr, size = int(row["from_address"], 16), int(row["to_address"], 16), int(row["size"])
    name = target_name(dst, d_addr, d_level, True)
    if name not in dst["functions"]:
        raise Skip(f"the target lists no function at {d_addr:08X}")
    home = re.fullmatch(r"\w+?_L(\d\d)_([0-9A-Fa-f]{8})", name)
    if home and (int(home.group(1)), int(home.group(2), 16)) != (d_level, d_addr):
        # The target's catalogue calls this place another copy of NAME; its symbols are named from NAME's own place.
        raise Skip(f"the target counts this place as a copy of {name}")
    if not home and d_level is not None:
        raise Skip(f"a level's copy of the executable's {name}: it is ported in the executable")
    path = definition_file(src, row["from_name"])
    text = preprocess(path, [ROOT / d for d in src["include"]], src.get("defines", []))
    chosen, target = pieces(text, row["from_name"])
    d_words = mips.words(code(dst, row["to_program"], d_addr, size))
    pairs = align(mips.references(code(src, row["from_program"], s_addr, size), s_addr, src["gp"]),
                  mips.references(code(dst, row["to_program"], d_addr, size), d_addr, dst["gp"]))

    # What each kept declaration links against, and where that is in the source version.
    for it in chosen:
        if Path(it["origin"]).name in src.get("refuse_headers", []):
            raise Skip(f"uses {Path(it['origin']).name}, which is not carried to other projects")
    shared = [Path(p).name for p in src.get("shared_headers", [])]
    drop = [it for it in chosen if Path(it["origin"]).name in shared]
    chosen = [it for it in chosen if it not in drop]
    used = idents(target["clean"]) | set().union(*[idents(it["clean"]) for it in chosen if it["kind"] != "decl"])
    links: dict[str, list[int]] = {}
    symbol_of: dict[str, str] = {}              # C identifier -> link name
    labels, source_macro = {}, set()            # a label or MACRO_ADDR on any declaration of a name holds for all of them
    for it in chosen:
        if it["kind"] == "decl" and link_name(it["text"]):
            labels.update({ident: link_name(it["text"]) for ident in it["names"]})
        if it["kind"] == "decl" and '(".sdata")' in it["text"]:
            source_macro |= it["names"]
    for it in chosen:
        if it["kind"] != "decl" or it["static"]:
            continue
        for ident in it["names"]:
            if ident not in used and not it.get("proto"):
                continue
            symbol_of[ident] = labels.get(ident, ident)
            links[symbol_of[ident]] = source_addresses(src, symbol_of[ident], s_level)
            if not links[symbol_of[ident]]:
                raise Skip(f"no address for {symbol_of[ident]} in the source version")
    found = resolve({n: a for n, a in links.items() if s_addr not in a}, pairs)
    for link, addrs in links.items():           # the function itself, under whatever name its file declares it
        if s_addr in addrs:
            found[link] = (d_addr, {"call"}, [])

    rename = {row["from_name"]: name}
    phrases = [tuple(p) for p in src.get("phrases", [])]
    for old, new in src.get("types", {}).items():
        rename[old] = new
    taken: dict[str, list[str]] = {}            # target name -> the identifiers that carry it
    lines, suffix = [], f"{d_addr:08X}"[-5:]
    declared_once: set[str] = set()
    always_alias = dst.get("alias") == "always"         # the target's files declare everything under private names
    sda = dst.get("small_data") == "sda"                # ... and mark small data with an attribute
    for word in hints.get("rename", []):
        rename[word] = f"{word}_{suffix}"
    for it in chosen:
        if it["kind"] != "decl" or it["static"]:
            lines.append(it)
            continue
        emitted = []
        for ident in sorted(it["names"]):
            if ident not in symbol_of:
                continue
            link = symbol_of[ident]
            base, kinds, where = found[link]
            if base == d_addr and s_addr in links[link]:        # the function itself: its definition declares it
                rename[ident] = name
                continue
            if ident in declared_once:                          # a file may declare one name once per function that uses it
                continue
            declared_once.add(ident)
            function = bool(it.get("proto")) or re.search(rf"\b{re.escape(ident)}\s*\(", it["clean"]) is not None
            new = target_name(dst, base, d_level, function and kinds <= {"call"})
            if not new:
                raise Skip(f"the target has no name for {link} at {base:08X}")
            # A second C name for one symbol, or a symbol the target's file declares with another
            # type, is declared under a private name with an assembler label.
            alias = always_alias or new in taken or new in hints.get("alias", [])
            local = f"{new}_{suffix}{chr(ord('a') + len(taken[new])) if taken.get(new) else ''}" if alias else new
            if len(taken.get(new, [])) > 20:
                raise Skip(f"more than twenty names for {new}")
            taken.setdefault(new, []).append(local)
            label = f' __asm__("{new}")' if alias else ""
            if function:
                proto = without(it["clean"], r"\b(?:__asm__|asm)")
                proto = substitute(proto, proto, {**rename, ident: local}, phrases).rstrip().rstrip(";").rstrip()
                emitted.append(" ".join(proto.split()) + label + ";")
                rename[ident] = local
                continue
            kind_type, tail = object_parts(it, ident)
            kind_type = substitute(kind_type, kind_type, rename, phrases)
            base_words = [w for w in IDENT.findall(kind_type) if w not in ("const", "volatile", "signed", "unsigned")]
            small = not tail and "*" not in kind_type and len(base_words) == 1 and base_words[0] in SMALL_TYPES
            # MACRO_ADDR (both rac1 projects define it alike) leaves the form of each access to the
            # assembler, which is how one global comes to be reached both ways. Keep it where the
            # source has it, and add it where every lui access has the assembler's shape: the source
            # project's compiler gets that shape without being told, the target's does not.
            lui_form = [i for kind, i in where if kind == "data"]
            if hints.get("macro_addr", dst.get("macro_addr")) == "mixed":     # only where the code has both forms
                macro = "gp" in kinds and "data" in kinds
            else:
                macro = ident in source_macro or not tail and bool(lui_form) and all(through_macro(d_words, i) for i in lui_form)
            if sda:
                mark = " MACRO_ADDR" if macro else " __attribute__((sda))" if "gp" in kinds and "data" not in kinds else ""
                emitted.append(f"extern {kind_type} {local}{tail}{label}{mark};")
                rename[ident] = local
                continue
            if "gp" in kinds and "data" not in kinds and not small and not macro:
                # The target's convention for a word in small data: a short, read through a cast.
                if tail and not re.fullmatch(r"\[[^\]]*\]", tail):
                    raise Skip(f"{link} is small data with a declarator this tool does not rewrite")
                emitted.append(f"extern short {local}{label};")
                rename[ident] = f"((({kind_type} *)&{local}))" if tail else f"(*({kind_type} *)&{local})"
                continue
            keep_out = " NOT_SDA" if small and "gp" not in kinds and not macro else ""
            emitted.append(f"extern {kind_type} {local}{tail}{label}{keep_out}{' MACRO_ADDR' if macro else ''};")
            rename[ident] = local
        lines.append({"kind": "emitted", "text": "\n".join(emitted)})

    out = []
    for it in lines:
        if it["kind"] == "emitted":
            out.append(it["text"])
        else:
            out.append(substitute(it["text"], it["clean"], rename, phrases).strip())
    credit = src["credit"].format(file=path.relative_to(ROOT / src["root"]).as_posix(), name=row["from_name"])
    body = substitute(target["text"], target["clean"], rename, phrases).strip()
    about = target["about"] + ("" if target["about"].endswith((".", "!", "?")) or not target["about"] else ".")
    text = "\n".join(t for t in out if t) + f"\n\n/* {about + chr(10) + '   ' if about else ''}{credit} */\n{body}\n"
    for find, replace in (('__attribute__((section(".data")))', "NOT_SDA"), ('__attribute__((section(".sdata")))', "MACRO_ADDR")):
        text = text.replace(find, replace)
    allowed = dst.get("allow", {}).get(name, [])        # a reviewed exception, by name (newlib's own macro bodies)
    banned = [why for pattern, why in BANNED if why not in allowed and re.search(pattern, blank_comments(text))]
    if banned:
        raise Skip("needs a hand: " + ", ".join(banned))
    code_only = blank(text)
    needs = [header for word, header in dst.get("headers", {}).items() if re.search(rf"\b{word}\b", code_only)]
    return name, "".join(f'#include "{h}"\n' for h in sorted(set(needs))) + text


# What a candidate may not contain in any of the projects (docs/policy): the checks rac1/pal's integrate.py makes.
BANNED = [(r"\bregister\b[^;{]*__asm__\s*\(", "a register pin"),
          (r'__asm__\s*(?:volatile\s*|__volatile__\s*)?\(\s*"[^"]*[\s$:][^"]*"', "inline assembly"),
          (r"\bwhile\s*\(\s*0\s*\)", "do/while (0)"),
          (r"(?m)^\s*#\s*define\b", "a #define")]


def blank_comments(text: str) -> str:
    return re.sub(r"/\*.*?\*/", "", text, flags=re.S)


_definitions: dict[str, dict[str, Path]] = {}


def definition_file(src: dict, name: str) -> Path:
    """The source project's file that defines NAME."""
    index = _definitions.get(src["key"])
    if index is None:
        index = _definitions[src["key"]] = {}
        for path in sorted((ROOT / src["sources"]).rglob("*.c")):
            for it in items(path.read_text(errors="replace")):
                if it["kind"] == "function":
                    for found in it["names"]:
                        index.setdefault(found, path)
    if name not in index:
        raise Skip(f"no file under {src['sources']} defines {name}")
    return index[name]


def rows(src_key: str, dst_key: str) -> list[dict]:
    path = PORTS / f"{xmap.slug(src_key)}--{xmap.slug(dst_key)}.tsv"
    if not path.is_file():
        raise SystemExit(f"no {path.relative_to(ROOT)}: run python3 tools/xmap.py ports {src_key} {dst_key}")
    head, *body = tables.read_tsv(path)
    return [dict(zip(head, cells)) for cells in body]


def functions_under(folder: Path) -> set[str]:
    """Every function with an assembly stub or a body in FOLDER's C files (not the ones they only call)."""
    found = set()
    for path in folder.rglob("*.c"):
        text = path.read_text(errors="replace")
        found |= set(re.findall(r"^\s*INCLUDE_ASM\([^)]*?\b(\w+)\s*\)", text, re.M))
        for it in items(text):
            if it["kind"] == "function":
                found |= it["names"]
    return found


def generate(src: dict, dst: dict, out_dir: Path, only: set[str]) -> dict[str, tuple[str, str, str]]:
    """Writes the candidates; returns target name -> (source name, size, result)."""
    skip = set()
    for listing in dst.get("never", []):
        skip |= set((ROOT / listing).read_text().split())
    for folder in dst.get("never_sources", []):
        skip |= functions_under(ROOT / folder)
    table = {}

    def own_level(row: dict) -> bool:
        """Whether the row's place is the one the target's name for the function is made from."""
        level = level_of(row["to_program"])
        named = re.match(r"\w+?_L(\d\d)_", target_name(dst, int(row["to_address"], 16), level, True) or "")
        return not named or int(named.group(1)) == level

    # A target function with several places has a row for each: the place its name comes from goes first and stands.
    for row in sorted(rows(src["key"], dst["key"]), key=lambda r: not own_level(r)):
        if row["kind"] != "same":
            continue
        d_level = level_of(row["to_program"])
        name = target_name(dst, int(row["to_address"], 16), d_level, True) or f"{row['to_program']}:{row['to_address']}"
        if only and not {name, row["from_name"]} & only:
            continue
        if name in table:
            continue
        result = "candidate"
        hints_path = out_dir / f"{name}.hints.json"
        hints = json.loads(hints_path.read_text()) if hints_path.is_file() else {}
        try:
            if name in skip:
                raise Skip("excluded by the target project")
            name, text = port(row, src, dst, hints)
            (out_dir / f"{name}.c").write_text(text)
        except Skip as why:
            result = str(why)
        except Exception as error:              # one odd file must not stop the run
            result = f"tool error: {type(error).__name__}: {error}"
        table[name] = (row["from_name"], row["size"], result)
    return table


COMPLAINT = re.compile(r"(?:conflicting types for|redefinition of|redeclaration of|previous (?:implicit )?declaration of) `(?:(?:struct|union|enum) )?(\w+)'"
                       r"|`(\w+)' redeclared as different kind of symbol")
VERDICT = re.compile(r"^(\S+)\s+(.+?)\s+(/\S+\.c)(?:\s+\((.*)\))?$")


def run_guarded(dst: dict, target_dir: Path, out_dir: Path, names: list[str]) -> dict[str, str]:
    """The verdict of a target whose check reads pending C from its own source
    file (rac1/ntsc: under `#else` of a NON_MATCHING guard, the assembly stub
    staying in force). Each candidate is put there, checked and taken out
    again, one at a time: the tree is left as it was found."""
    cfg, verdicts = dst["check"], {}
    files = {p: p.read_text() for p in sorted((target_dir / cfg["sources"]).rglob("*.c"))}
    for name in names:
        stub = cfg["stub"].format(name=name)
        path = next((p for p, text in files.items() if stub in text), None)
        if path is None:
            verdicts[name] = "the target has no assembly stub for it"
            continue
        text = files[path]
        if text[:text.index(stub)].rstrip().endswith("#ifndef NON_MATCHING"):
            verdicts[name] = "the target already has pending C for it"
            continue
        body = (out_dir / f"{name}.c").read_text()
        try:
            path.write_text(text.replace(stub, f"#ifndef NON_MATCHING\n{stub}\n#else\n{body}#endif /* NON_MATCHING */", 1))
            done = subprocess.run(cfg["command"].format(name=name), shell=True, cwd=target_dir, capture_output=True, text=True)
        finally:
            path.write_text(text)
        (out_dir / f"{name}.log").write_text(done.stdout + done.stderr)
        try:
            answer = json.loads(done.stdout[done.stdout.index("{"):])
            verdicts[name] = "EXACT" if answer.get("ok") else str(answer.get("verdict") or answer.get("error") or "not exact")
            verdicts[name] = "COMPILE failed" if verdicts[name].startswith("compile failed") else verdicts[name].splitlines()[0]
        except ValueError:
            verdicts[name] = "COMPILE failed"
        print(f"{name:20s} {verdicts[name]}", flush=True)
    return verdicts


def guarded_patch(dst: dict, target_dir: Path, out_dir: Path, names: list[str]) -> str:
    """The passing candidates of a guard-checked target as one patch against
    its tree, each in place of its assembly stub. The tree is not touched."""
    import difflib
    cfg = dst["check"]
    old = {p: p.read_text() for p in sorted((target_dir / cfg["sources"]).rglob("*.c"))}
    new = dict(old)
    for name in names:
        stub = cfg["stub"].format(name=name)
        path = next((p for p, text in new.items() if stub in text), None)
        if path:
            new[path] = new[path].replace(stub, (out_dir / f"{name}.c").read_text().rstrip("\n"), 1)
    out = []
    for path in old:
        if old[path] != new[path]:
            rel = path.relative_to(target_dir).as_posix()
            out += difflib.unified_diff(old[path].splitlines(True), new[path].splitlines(True), f"a/{rel}", f"b/{rel}")
    return "".join(out)


def run_check(dst: dict, target_dir: Path, out_dir: Path, names: list[str], jobs: int) -> dict[str, str]:
    """The target project's verdict on each named candidate, several at a time."""
    if dst["check"].get("kind") == "guard":
        return run_guarded(dst, target_dir, out_dir, names)
    chunks = [names[k::jobs] for k in range(jobs) if names[k::jobs]]
    running = []
    for k, chunk in enumerate(chunks):
        manifest = out_dir / f"MANIFEST.{k}"
        manifest.write_text("".join(f"{n} {out_dir / (n + '.c')}\n" for n in chunk))
        running.append(subprocess.Popen(dst["check"]["command"].format(manifest=manifest), shell=True, cwd=target_dir,
                                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True))
    verdicts = {}
    for proc in running:
        for line in proc.communicate()[0].splitlines():
            m = VERDICT.match(line.strip())
            if m:
                verdicts[m.group(1)] = m.group(2).strip() + (f" ({m.group(4)})" if m.group(4) else "")
    return verdicts


def check(src: dict, dst: dict, target_dir: Path, out_dir: Path, only: set[str], jobs: int) -> None:
    table = generate(src, dst, out_dir, only)
    todo = [n for n, (_f, _s, result) in table.items() if result == "candidate"]
    verdicts: dict[str, str] = {}
    for attempt in range(4):
        print(f"round {attempt + 1}: checking {len(todo)} candidates in {target_dir}", flush=True)
        verdicts.update(run_check(dst, target_dir, out_dir, todo, jobs))
        again = []
        for name in todo:
            log = target_dir / dst["check"]["log"].format(name=name) if "log" in dst["check"] else out_dir / f"{name}.log"
            if not verdicts.get(name, "").startswith("COMPILE") or not log.is_file():
                continue
            hints_path = out_dir / f"{name}.hints.json"
            hints = json.loads(hints_path.read_text()) if hints_path.is_file() else {"alias": [], "rename": []}
            before = json.dumps(hints, sort_keys=True)
            for m in COMPLAINT.finditer(log.read_text(errors="replace")):
                word = m.group(1) or m.group(2)
                if word == name:                # a candidate cannot change the prototype its file already has
                    verdicts[name] = "the target's file already declares it with other types"
                    continue
                kind = "alias" if re.match(r"(?:func|D|jtbl)_", word) else "rename"
                if word not in hints[kind]:
                    hints[kind].append(word)
            if json.dumps(hints, sort_keys=True) != before:
                hints_path.write_text(json.dumps(hints, indent=1) + "\n")
                again.append(name)
        if not again:
            break
        redone = generate(src, dst, out_dir, set(again))
        table.update(redone)
        todo = [n for n in again if redone.get(n, ("", "", ""))[2] == "candidate"]
    # Where the target's rule for a declaration is not settled, the other reading gets a try
    # ("alternatives" in its check); a candidate keeps the reading that passes.
    for other in dst["check"].get("alternatives", []):
        open_ones = [n for n, (_f, _s, result) in table.items() if result == "candidate" and not verdicts.get(n, "").startswith("EXACT")]
        kept = {n: ((out_dir / f"{n}.c").read_text(), (out_dir / f"{n}.hints.json").read_text() if (out_dir / f"{n}.hints.json").is_file() else None)
                for n in open_ones}
        for n in open_ones:
            hints = json.loads(kept[n][1]) if kept[n][1] else {"alias": [], "rename": []}
            (out_dir / f"{n}.hints.json").write_text(json.dumps({**hints, **other}, indent=1) + "\n")
        redone = generate(src, dst, out_dir, set(open_ones))
        again = [n for n in open_ones if redone.get(n, ("", "", ""))[2] == "candidate"]
        print(f"another reading {other}: checking {len(again)} candidates", flush=True)
        tried = run_check(dst, target_dir, out_dir, again, jobs)
        for n in open_ones:
            if tried.get(n, "").startswith("EXACT"):
                verdicts[n] = tried[n]
            else:                               # no better: the first reading's candidate stands
                (out_dir / f"{n}.c").write_text(kept[n][0])
                if kept[n][1] is None:
                    (out_dir / f"{n}.hints.json").unlink(missing_ok=True)
                else:
                    (out_dir / f"{n}.hints.json").write_text(kept[n][1])
    lines = ["name\tfrom_name\tsize\tresult"]
    for name, (from_name, size, result) in table.items():
        lines.append(f"{name}\t{from_name}\t{size}\t{verdicts.get(name, result) if result == 'candidate' else result}")
    (out_dir / "results.tsv").write_text("\n".join(lines) + "\n")
    exact = [n for n in table if verdicts.get(n, "").startswith("EXACT")]
    (out_dir / "MANIFEST.exact").write_text("".join(f"{n} {out_dir / (n + '.c')}\n" for n in exact))
    if dst["check"].get("kind") == "guard":             # what passed, ready to hand to the target project
        (out_dir / "exact.patch").write_text(guarded_patch(dst, target_dir, out_dir, exact))
    size = sum(int(table[n][1]) for n in exact)
    print(f"{len(exact)} of {len(table)} pass the target's check ({size:,} bytes): {out_dir / 'results.tsv'}")


def option(args: list[str], flag: str) -> str | None:
    if flag not in args:
        return None
    i = args.index(flag)
    value = args[i + 1]
    del args[i:i + 2]
    return value


def main() -> None:
    args = sys.argv[1:]
    out, target_dir, jobs = option(args, "--out"), option(args, "--target-dir"), int(option(args, "--jobs") or 6)
    checking = "--check" in args
    args = [a for a in args if a != "--check"]
    if len(args) < 2:
        sys.exit(__doc__)
    src_key, dst_key, only = args[0], args[1], set(args[2:])
    src, dst = side(src_key), side(dst_key)
    pair = f"{xmap.slug(src_key)}--{xmap.slug(dst_key)}"
    if checking:
        # The check runs inside the target project (its container mounts only that tree), so the candidates go there.
        target = Path(target_dir).resolve() if target_dir else ROOT / dst["root"]
        out_dir = Path(out).resolve() if out else target / dst["check"]["scratch"] / pair
        out_dir.mkdir(parents=True, exist_ok=True)
        check(src, dst, target, out_dir, only, jobs)
        return
    out_dir = Path(out).resolve() if out else ROOT / "build/port" / pair
    out_dir.mkdir(parents=True, exist_ok=True)
    table = generate(src, dst, out_dir, only)
    ready = [n for n, (_f, _s, result) in table.items() if result == "candidate"]
    (out_dir / "MANIFEST").write_text("".join(f"{n} {out_dir / (n + '.c')}\n" for n in ready))
    (out_dir / "ports.tsv").write_text("name\tfrom_name\tsize\tresult\n" + "".join(
        f"{n}\t{f}\t{size}\t{result}\n" for n, (f, size, result) in table.items()))
    print(f"{len(ready)} candidates of {len(table)} rows in {out_dir}")


if __name__ == "__main__":
    main()
