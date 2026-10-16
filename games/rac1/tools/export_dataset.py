#!/usr/bin/env python3
"""
Export every exactly matched C function as a training pair for a
decompilation model: the retail assembly as input, the matching C as
output, one JSON object per line (the instruction/input/output layout
QLoRA trainers read).

  python tools/export_dataset.py                       # rac1_decomp_dataset.jsonl
  python tools/export_dataset.py -o pairs.jsonl --meta
  python tools/export_dataset.py --no-context --no-comments

Which functions: those defined in C under src/ (by their func_XXXXXXXX
name) that progress/report.json scores 100%. Hand-written assembly and
linker remnants count as finished there too, but have no C and are
skipped. Run it after `bash tools/setup_asm.sh` and a report that is
current with src/.

The output C is the function itself, by default with its own leading
comment and the declarations it depends on from its file (externs,
typedefs, #defines), because matching often hangs on how a global is
declared (MACRO_ADDR, NOT_SDA, aliases). Those rules live in
include/common.h, which is not repeated per pair.

Two compilers built the retail image, and the instruction names the one
that applies: SN GCC 2.95.3 (SN BUILD v1.14) for game code, Sony's EE GCC
2.9-ee-991111 for SDK objects marked ee29 in config/core_text.objects.
Both at -O2 -G2.

Functions ported from Lombyte (MIT) are left out unless --include-ported
is given; they carry that project's license notice.

The input column is disassembly of your own retail executable, like
asm/, which this repository never distributes. The output file is
ignored by git.
"""
import argparse
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
FUNC_RE = re.compile(r"func_[0-9A-F]{8}")
IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
KEYWORDS = {
    "if", "else", "for", "while", "do", "switch", "case", "default", "return",
    "break", "continue", "goto", "sizeof", "struct", "union", "enum", "typedef",
    "extern", "static", "const", "volatile", "unsigned", "signed", "int", "char",
    "short", "long", "float", "double", "void", "register", "inline",
    "__inline__", "__asm__", "__attribute__", "MACRO_ADDR", "NOT_SDA",
}

GAME_CC = ("SN Systems ProDG GCC 2.95.3 (SN BUILD v1.14), flags -O2 -G2, "
           "MIPS EABI")
SDK_CC = ("Sony EE GCC 2.9-ee-991111 (the PS2 SDK's own compiler), flags "
          "-O2 -G2, MIPS EABI")
INSTRUCTION = ("Decompile this MIPS R5900 (PlayStation 2 Emotion Engine) "
               "assembly from Ratchet & Clank (2002, PAL SCES_509.16) into C "
               "that compiles to byte-identical code with {cc}.")


def split_top_level(text):
    """Yield (start, end, kind) for each top-level item: 'func' for a
    function definition, 'decl' for anything ending in ';' at depth 0,
    'pp' for a preprocessor line. Comments and literals are skipped."""
    i, n = 0, len(text)
    start = None
    depth = 0
    saw_paren_close = False
    while i < n:
        c = text[i]
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
            continue
        if c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            i = j + 1
            continue
        if depth == 0 and start is None:
            if c.isspace():
                i += 1
                continue
            if c == "#":
                j = i
                while True:
                    k = text.find("\n", j)
                    if k < 0:
                        k = n
                        break
                    if text[k - 1] != "\\":
                        break
                    j = k + 1
                yield i, k, "pp"
                i = k
                continue
            start = i
            saw_paren_close = False
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0 and saw_paren_close:
                yield start, i + 1, "func"
                start = None
        elif c == ")" and depth == 0:
            saw_paren_close = True
        elif c == ";" and depth == 0:
            yield start, i + 1, "decl"
            start = None
        elif c == "=" and depth == 0:
            saw_paren_close = False
        i += 1


def strip_comments(s):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", s, flags=re.S)


def strip_strings(s):
    return re.sub(r'"(?:\\.|[^"\\])*"', '""', s)


def declared_names(item):
    """Names an extern/typedef/#define declares (heuristic)."""
    s = strip_comments(item)
    if s.startswith("#define"):
        m = IDENT_RE.search(s, len("#define"))
        return {m.group(0)} if m else set()
    if s.startswith("#"):
        return set()
    if re.match(r"\s*(INCLUDE_ASM|INCLUDE_RODATA|ASM_FUNC|LINKER_REMNANT|__asm__)\b", s):
        return set()
    s = re.sub(r"__asm__\s*\([^)]*\)|__attribute__\s*\(\(.*?\)\)|\b(MACRO_ADDR|NOT_SDA)\b",
               " ", s, flags=re.S)
    names = set()
    braces = parens = 0
    toks = re.findall(r"[A-Za-z_][A-Za-z0-9_]*|[{}()\[\];,=*]", s)
    for i, (a, b) in enumerate(zip(toks, toks[1:] + [";"])):
        if a == "{":
            braces += 1
        elif a == "}":
            braces -= 1
        elif a == "(":
            parens += 1
        elif a == ")":
            parens -= 1
        elif braces == 0 and IDENT_RE.fullmatch(a) and a not in KEYWORDS \
                and b in (";", "[", "(", ",", "=", ")") \
                and (parens == 0 or (parens == 1 and b == ")"
                                     and toks[i - 2:i] == ["(", "*"])):
            names.add(a)
    if s.lstrip().startswith("typedef"):
        m = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*(\[[^\]]*\])*\s*;\s*$", s)
        if m:
            names.add(m.group(1))
    return names - KEYWORDS


def leading_comment(text, start):
    """The block comment directly above `start`, blank lines allowed."""
    head = text[:start].rstrip()
    if not head.endswith("*/"):
        return ""
    j = head.rfind("/*")
    return head[j:] + "\n" if j >= 0 else ""


def ee29_units():
    units = set()
    for line in (REPO / "config" / "core_text.objects").read_text().splitlines():
        f = line.split()
        if len(f) >= 3 and f[2] == "ee29":
            units.add(Path(f[0]).stem)
    return units


def exact_functions():
    report = json.loads((REPO / "progress" / "report.json").read_text())
    out = {}
    for unit in report["units"]:
        for fn in unit.get("functions", []):
            if fn.get("fuzzy_match_percent") == 100.0:
                out[fn["name"]] = unit["name"]
    return out


def asm_path(name):
    for seg in ("text", "core_text"):
        p = REPO / "asm" / "nonmatchings" / seg / f"{name}.s"
        if p.exists():
            return p
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("-o", "--output", default="rac1_decomp_dataset.jsonl")
    ap.add_argument("--no-context", action="store_true",
                    help="emit the function alone, without its declarations")
    ap.add_argument("--no-comments", action="store_true",
                    help="strip comments from the C")
    ap.add_argument("--include-ported", action="store_true",
                    help="keep functions adapted from Lombyte (MIT)")
    ap.add_argument("--meta", action="store_true",
                    help="add a 'meta' object (name, file, compiler, size)")
    args = ap.parse_args()

    exact = exact_functions()
    sdk = ee29_units()
    rows, missing_asm, ported = [], 0, 0

    for src in sorted((REPO / "src").rglob("*.c")):
        text = src.read_text(encoding="utf-8", errors="replace")
        items = list(split_top_level(text))
        for idx, (s, e, kind) in enumerate(items):
            if kind != "func":
                continue
            body = text[s:e]
            m = re.match(r"[^{(]*?\b(func_[0-9A-F]{8})\s*\(", body, re.S)
            if not m or m.group(1) not in exact:
                continue
            name = m.group(1)
            comment = leading_comment(text, s)
            if "Lombyte" in comment and not args.include_ported:
                ported += 1
                continue
            asm = asm_path(name)
            if asm is None:
                missing_asm += 1
                continue

            parts = []
            if not args.no_context:
                need = set(IDENT_RE.findall(strip_comments(body))) - {name}
                chosen = []
                for s2, e2, k2 in reversed(items[:idx]):
                    decl = text[s2:e2]
                    if k2 == "func":
                        # static inline helpers the function calls
                        hm = re.match(r"static\b[^{(]*?\b([A-Za-z_]\w*)\s*\(", decl)
                        names = {hm.group(1)} if hm else set()
                    else:
                        names = declared_names(decl)
                    if names & need:
                        chosen.append(decl)
                        need |= set(IDENT_RE.findall(strip_strings(strip_comments(decl))))
                seen = set()
                for decl in reversed(chosen):
                    if decl not in seen:
                        seen.add(decl)
                        parts.append(decl)
            code = "\n".join(parts) + ("\n\n" if parts else "") + comment + body
            if args.no_comments:
                code = re.sub(r"\n{3,}", "\n\n", strip_comments(code)).strip()

            unit = exact[name]
            cc = SDK_CC if unit.startswith("core/") and Path(unit).name in sdk else GAME_CC
            row = {
                "instruction": INSTRUCTION.format(cc=cc),
                "input": asm.read_text(encoding="utf-8").strip(),
                "output": code.strip(),
            }
            if args.meta:
                row["meta"] = {"name": name, "file": str(src.relative_to(REPO)),
                               "unit": unit, "compiler": cc}
            rows.append(row)

    if not rows:
        sys.exit("no pairs found: generate asm/ (tools/setup_asm.sh) and the report first")
    with open(args.output, "w", encoding="utf-8") as f:
        for row in rows:
            f.write(json.dumps(row) + "\n")
    print(f"wrote {len(rows)} pairs to {args.output}")
    if ported:
        print(f"skipped {ported} functions ported from Lombyte (--include-ported keeps them)")
    if missing_asm:
        print(f"warning: {missing_asm} exact functions have no .s file; run tools/setup_asm.sh")


if __name__ == "__main__":
    main()
