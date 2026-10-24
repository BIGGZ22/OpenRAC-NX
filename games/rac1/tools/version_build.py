#!/usr/bin/env python3
"""
Build a version's programs from their assembly and check each against the disc (docs/VERSIONS.md).

  python3 tools/version_build.py --version us [-P 8] [--rounds 3] [boot level_00 ...]   # all 20 without names
  python3 tools/version_build.py --version us --layout     # config/versions/us/boot_layout.json from the dump

Each program (`boot`, the executable; `level_NN`, level NN's code overlay) is written as
assembly from the dump (tools/versions.py dump), one .s per function as the version's function
table cuts it (config/versions/<K>/functions.tsv; the primary's report and catalogue for the
primary version), and its data sections as `.word` lines:

  asm/versions/<K>/<program>/<section>/<function>.s    each function of a code section
  asm/versions/<K>/<program>/<section>.parts/<addr>.s  the parts a code section is assembled in
  asm/versions/<K>/<program>/<section>.s               a data section
  build-sn/versions/<K>/<program>/                     objects, the linker script, the result

Every part is assembled with the toolchain's GNU assembler (binutils 2.10-ee `ee-as`,
toolchain/sn-prodg-24, a Linux program: native on Linux, through tools/docker/run.sh on a Mac),
the sections are linked at their addresses (`ee-ld`), and the program is put back together: the
executable as an ELF laid out as config/versions/<K>/boot_layout.json says (section and program
headers: a layout, no bytes), a level as its records (tools/versions.py overlay_program()). It
prints MATCH when the result's SHA-1 is config/versions/<K>/programs.tsv's: the program byte for
byte as the disc stores it.

The instructions come from rabbitizer (R5900, no pseudo-instructions); branch targets are local
labels and calls to a function of the same section use its name. What the assembler cannot give
back as it is (a VU0 macro instruction, a COP0 move, a short loop it pads, data among code)
becomes `.word` with the disassembly in a comment: --rounds rebuilds with the first differing
word of each section added to asm/versions/<K>/<program>/words.txt. A code section is assembled
in parts of about 8 KB that no branch crosses: the old assembler takes time quadratic in a
file's size.

This proves the dump, the function table and the assembly; the C in src/ is checked against each
version function by function (tools/version_check.py).
"""
import argparse
import bisect
import functools
import hashlib
import json
import os
import re
import struct
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import versions  # noqa: E402

BIN = Path(os.environ.get("EE_BIN", ROOT / "toolchain/sn-prodg-24/local/sce/ee/gcc/bin"))
ASFLAGS = ["-EL", "-mips3", "-mcpu=r5900"]
CODE = {"boot": ("core.text", ".text"), "level": ("text",)}
NO_R5900 = {"ldc1", "sdc1", "mfc0", "mtc0", "ll", "sc", "lld", "scd"}
CHUNK_WORDS = 2048
GPR = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
       "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
REG = re.compile(r"\$(%s)\b" % "|".join(GPR))
PRELUDE = """.set noreorder
.set noat
.set nomacro
.macro glabel label
    .global \\label
    .type \\label, @function
    \\label:
.endm
.text
"""
ERROR = re.compile(r"^(\S+?):(\d+): (?:Error|Warning):", re.M)
ADDR = re.compile(r"^/\* ([0-9A-F]{8}) [0-9A-F]{8} \*/")


def asm_root(v, program):
    return ROOT / "asm/versions" / v.key / program


def build_root(v, program):
    return ROOT / "build-sn/versions" / v.key / program


def words(b):
    return struct.unpack("<%dI" % (len(b) // 4), b[:len(b) // 4 * 4])


# --- what the program holds --------------------------------------------------------------------

def layout_path(v):
    return v.config / "boot_layout.json"


def write_layout(v):
    """The executable's ELF header, program headers and section headers (no section bytes)."""
    b = v.exe_path.read_bytes()
    hdr = dict(zip(("type", "machine", "version", "entry", "phoff", "shoff", "flags", "ehsize", "phentsize",
                    "phnum", "shentsize", "shnum", "shstrndx"), struct.unpack_from("<HHIIIIIHHHHHH", b, 16)))
    hdr["ident"] = b[:16].hex()
    phdrs = [dict(zip(("type", "offset", "vaddr", "paddr", "filesz", "memsz", "flags", "align"),
                      struct.unpack_from("<8I", b, hdr["phoff"] + i * hdr["phentsize"])))
             for i in range(hdr["phnum"])]
    secs = []
    for i in range(hdr["shnum"]):
        f = struct.unpack_from("<10I", b, hdr["shoff"] + i * hdr["shentsize"])
        secs.append(dict(zip(("name_offset", "type", "flags", "addr", "offset", "size", "link", "info", "align",
                              "entsize"), f)))
    named = versions.elf_sections(b)
    for s, n in zip(secs, named):
        s["name"] = n["name"]
    lay = {"size": len(b), "header": hdr, "program_headers": phdrs, "sections": secs}
    # everything outside the headers and the sections is zero, so the layout and the sections give
    # the file back
    covered = bytearray(len(b))
    for lo, n in [(0, 0x34), (hdr["phoff"], hdr["phnum"] * hdr["phentsize"]),
                  (hdr["shoff"], hdr["shnum"] * hdr["shentsize"])] + \
                 [(s["offset"], s["size"]) for s in secs if s["type"] not in (0, 8)]:
        covered[lo:lo + n] = b"\1" * n
    stray = [i for i in range(len(b)) if not covered[i] and b[i]]
    if stray:
        sys.exit(f"{v.key}: {len(stray)} bytes of the executable lie outside its sections (first at {stray[0]:#x})")
    v.config.mkdir(parents=True, exist_ok=True)
    layout_path(v).write_text(json.dumps(lay, indent=1) + "\n")
    print(f"wrote {layout_path(v).relative_to(ROOT)}")


def elf_from_layout(lay, contents):
    """The executable from its layout and {section name: bytes}."""
    h = lay["header"]
    out = bytearray(lay["size"])
    out[:16] = bytes.fromhex(h["ident"])
    struct.pack_into("<HHIIIIIHHHHHH", out, 16, *(h[k] for k in (
        "type", "machine", "version", "entry", "phoff", "shoff", "flags", "ehsize", "phentsize", "phnum",
        "shentsize", "shnum", "shstrndx")))
    for i, p in enumerate(lay["program_headers"]):
        struct.pack_into("<8I", out, h["phoff"] + i * h["phentsize"], *(p[k] for k in (
            "type", "offset", "vaddr", "paddr", "filesz", "memsz", "flags", "align")))
    for i, s in enumerate(lay["sections"]):
        struct.pack_into("<10I", out, h["shoff"] + i * h["shentsize"], *(s[k] for k in (
            "name_offset", "type", "flags", "addr", "offset", "size", "link", "info", "align", "entsize")))
        if s["type"] not in (0, 8) and s["size"]:
            data = contents[(i, s["name"])]
            if len(data) != s["size"]:
                raise ValueError(f"{s['name']}: built {len(data)} bytes, layout {s['size']}")
            out[s["offset"]:s["offset"] + s["size"]] = data
    return bytes(out)


def sections_of(v, program):
    """[(key, name, address, bytes, linked, code)] of a program: key is unique (an index for the
    executable's sections, several share a name), linked is whether ld places it at its address
    (the rest are assembled alone), code whether it is disassembled."""
    if program == "boot":
        b = v.exe_path.read_bytes()
        out = []
        for i, s in enumerate(versions.elf_sections(b)):
            if s["type"] in (0, 8) and s["type"] == 0:
                continue
            data = b"" if s["type"] == 8 else b[s["offset"]:s["offset"] + s["size"]]
            linked = s["type"] in (1, 8) and s["addr"] != 0
            out.append(((i, s["name"]), s["name"], s["addr"], data if s["type"] != 8 else bytes(s["size"]),
                        linked, s["name"] in CODE["boot"]))
        return out
    lid = int(program[6:])
    _, recs = versions.level_records(v, lid)
    return [((i, r["name"]), r["name"], r["address"], r["data"], True, r["name"] in CODE["level"])
            for i, r in enumerate(recs)]


def function_starts(v, program):
    """{address: name} of a program's functions."""
    out = {}
    if v.primary:
        import overlays as ov
        if program == "boot":
            for name, addr, size, _b in ov.exe_functions(everything=True):
                out[addr] = name
        else:
            lid = int(program[6:])
            for name, _kind, _size, places in ov.read_catalogue():
                for lv, a in places:
                    if lv == lid:
                        out.setdefault(a, name)
        return out
    import version_map
    want = "boot" if program == "boot" else int(program[6:])
    for e in version_map.read(v):
        for p, a in e["places"]:
            if p == want:
                out.setdefault(a, e["name"])
    return out


# --- assembly ---------------------------------------------------------------------------------

def chunks_of(ins, base, order):
    import rabbitizer  # noqa: F401
    n = len(ins)
    cover = [0] * (n + 2)
    for i, x in enumerate(ins):
        if x.isValid() and x.isBranch():
            t = (x.getBranchVramGeneric() - base) // 4
            lo, hi = min(i, t), max(i, t)
            if 0 <= lo and hi < n:
                cover[lo + 1] += 1
                cover[hi + 1] -= 1
    crossed, c = [False] * (n + 1), 0
    for k in range(n + 1):
        c += cover[k]
        crossed[k] = c > 0
    cuts, last = [base], base
    for a in order[1:]:
        k = (a - base) // 4
        if a % 16 == 0 and not crossed[k] and (a - last) // 4 >= CHUNK_WORDS:
            cuts.append(a)
            last = a
    return cuts


def code_section(base, data, starts, as_words, label):
    """([(name, start, text)], [part starts]) of a code section."""
    import rabbitizer
    rabbitizer.config.pseudos_enablePseudos = False
    rabbitizer.config.misc_expandJalr = True
    w = words(data)
    n = len(w)
    ins = [rabbitizer.Instruction(x, base + 4 * i, category=rabbitizer.InstrCategory.R5900)
           for i, x in enumerate(w)]
    order = sorted(a for a in starts if base <= a < base + 4 * n)
    if not order or order[0] != base:
        order.insert(0, base)
    cuts = chunks_of(ins, base, order)
    part = lambda addr: bisect.bisect_right(cuts, addr)
    names = {a: starts[a] for a in order if a in starts}
    names.setdefault(base, "%s_%08X" % (label, base))
    # a name used twice in one section (the same function at two places) gets its address
    seen = {}
    for a in order:
        seen[names[a]] = seen.get(names[a], 0) + 1
    for a in order:
        if seen[names[a]] > 1:
            names[a] = "%s_at_%08X" % (names[a], a)
    symbols = {a: names[a] for a in order}
    labels = set()
    for i, x in enumerate(ins):
        if x.isValid() and (x.isBranch() or (x.isJump() and not x.isJrRa() and x.isJType())):
            t = x.getBranchVramGeneric() if x.isBranch() else x.getInstrIndexAsVram()
            if base <= t < base + 4 * n and not (x.isJType() and t in symbols) and part(t) == part(base + 4 * i):
                labels.add(t)
    out = []
    for k, a in enumerate(order):
        end = order[k + 1] if k + 1 < len(order) else base + 4 * n
        lines = ["glabel %s" % names[a]]
        for addr in range(a, end, 4):
            i = (addr - base) // 4
            x = ins[i]
            if addr in labels:
                lines.append(".L%08X:" % addr)
            word = w[i]
            text = None
            if addr not in as_words and x.isValid() and x.isImplemented() and not x.isDouble() \
                    and x.getOpcodeName() not in NO_R5900:
                try:
                    if x.isBranch():
                        t = x.getBranchVramGeneric()
                        text = x.disassemble(immOverride=".L%08X" % t) if t in labels else None
                    elif x.isJType():
                        t = x.getInstrIndexAsVram()
                        if t in symbols and part(t) == part(addr):
                            text = x.disassemble(immOverride=symbols[t])
                        elif t in labels and part(t) == part(addr):
                            text = x.disassemble(immOverride=".L%08X" % t)
                        else:
                            text = None
                    else:
                        text = x.disassemble()
                except Exception:
                    text = None
            if text is None or "\n" in text:
                dis = x.disassemble() if x.isValid() else "?"
                lines.append("/* %08X %08X */ .word 0x%08X  /* %s */" % (addr, word, word, dis.replace("*/", "* /")))
            else:
                lines.append("/* %08X %08X */ %s" % (addr, word, text))
        out.append((names[a], a, "\n".join(lines) + "\n"))
    return out, cuts


def data_text(data):
    w = words(data)
    out = [".text"]
    for i in range(0, len(w), 4):
        out.append(".word " + ", ".join("0x%08X" % x for x in w[i:i + 4]))
    tail = data[len(w) * 4:]
    if tail:
        out.append(".byte " + ", ".join("0x%02X" % c for c in tail))
    return "\n".join(out) + "\n"


def safe(name):
    return re.sub(r"[^A-Za-z0-9_]", "_", name.lstrip("."))


def read_words_txt(v, program):
    p = asm_root(v, program) / "words.txt"
    if not p.exists():
        return set()
    return {int(l.split()[0], 16) for l in p.read_text().splitlines() if l.strip() and not l.startswith("#")}


def write_program(v, program):
    """The program's assembly. Returns [(key, name, address, linked, [.s parts], size)]."""
    root = asm_root(v, program)
    root.mkdir(parents=True, exist_ok=True)
    starts = function_starts(v, program)
    as_words = read_words_txt(v, program)
    out = []
    for key, name, addr, data, linked, code in sections_of(v, program):
        stem = "%02d_%s" % (key[0], safe(name))
        if code:
            d, pd = root / stem, root / (stem + ".parts")
            d.mkdir(exist_ok=True)
            pd.mkdir(exist_ok=True)
            label = "%s_%s" % (program, safe(name))
            funcs, cuts = code_section(addr, data, starts, as_words, label)
            parts = [[] for _ in cuts]
            for fname, fa, text in funcs:
                fp = d / (fname + ".s")
                body = "/* %s %s %s 0x%08X */\n%s" % (v.key, program, name, fa, text)
                if not fp.exists() or fp.read_text() != body:
                    fp.write_text(body)
                parts[bisect.bisect_right(cuts, fa) - 1].append('.include "%s"' % fp.relative_to(ROOT))
            files = []
            for k, incs in enumerate(parts):
                fp = pd / ("%08X.s" % cuts[k])
                fp.write_text(PRELUDE + "\n".join(incs) + "\n")
                files.append(fp)
        else:
            fp = root / (stem + ".s")
            fp.write_text(data_text(data))
            files = [fp]
        out.append((key, name, addr, linked, files, len(data)))
    return out


# --- building ---------------------------------------------------------------------------------

def flatten(src, dst):
    """The .s with its .include lines expanded and registers as numbers (binutils 2.10-ee knows
    only $sp, $ra, $gp, $fp and $at by name)."""
    text = src.read_text()
    text = re.sub(r'^\.include "([^"]+)"$', lambda m: (ROOT / m.group(1)).read_text(), text, flags=re.M)
    text = REG.sub(lambda m: "$%d" % GPR.index(m.group(1)), text)
    dst.write_text(text)


def as_errors(flat, log_text):
    lines = flat.read_text().split("\n")
    out = set()
    for f, n in ERROR.findall(log_text):
        if os.path.normpath(f) == os.path.normpath(str(flat)):
            m = ADDR.match(lines[int(n) - 1])
            if m:
                out.add(int(m.group(1), 16))
    return out


def text_of(obj):
    from elftools.elf.elffile import ELFFile
    with open(obj, "rb") as f:
        s = ELFFile(f).get_section_by_name(".text")
        return s.data() if s else b""


def linked_sections(elf):
    from elftools.elf.elffile import ELFFile
    out = {}
    with open(elf, "rb") as f:
        for s in ELFFile(f).iter_sections():
            if s.name.startswith("v."):
                out[int(s.name[2:])] = s.data()
    return out


def build(v, program, log):
    """(ok, message, {section key: addresses to make .word})."""
    secs = write_program(v, program)
    d = build_root(v, program)
    d.mkdir(parents=True, exist_ok=True)
    refused, objs = {}, {}
    for key, name, addr, linked, files, size in secs:
        objs[key] = []
        for k, src in enumerate(files):
            flat = d / ("%02d_%s_%d.s" % (key[0], safe(name), k))
            obj = flat.with_suffix(".o")
            flatten(src, flat)
            cmd = [str(BIN / "ee-as"), *ASFLAGS, "-o", str(obj), str(flat)]
            r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
            log.write("+ %s\n%s%s" % (" ".join(cmd), r.stdout, r.stderr[-20000:]))
            if r.returncode:
                bad = as_errors(flat, r.stdout + r.stderr)
                if not bad:
                    return False, "ee-as failed on %s" % src.relative_to(ROOT), {}
                refused.setdefault(key, set()).update(bad)
            objs[key].append(obj)
    if refused:
        return False, "ee-as refused %d instructions" % sum(len(x) for x in refused.values()), refused
    ld = ["SECTIONS", "{"]
    for key, name, addr, linked, files, size in secs:
        if linked:
            ld.append("    v.%d 0x%X : { %s }  /* %s */" % (key[0], addr, " ".join("%s(.text)" % o for o in objs[key]), name))
    ld += ["    /DISCARD/ : { *(*) }", "}"]
    script = d / "program.ld"
    script.write_text("\n".join(ld) + "\n")
    out = d / "program.elf"
    cmd = [str(BIN / "ee-ld"), "-T", str(script), "-o", str(out)]
    r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    log.write("+ %s\n%s%s" % (" ".join(cmd), r.stdout, r.stderr))
    if r.returncode:
        return False, "ee-ld failed", {}
    linked = linked_sections(out)
    built = {}
    for key, name, addr, is_linked, files, size in secs:
        b = linked.get(key[0], b"") if is_linked else b"".join(text_of(o) for o in objs[key])
        built[key] = b[:size] if len(b) >= size else b
    diffs, where = {}, []
    for key, name, addr, data, is_linked, code in sections_of(v, program):
        got = built[key]
        if got != data:
            k = next((i for i in range(0, min(len(got), len(data)), 4) if got[i:i + 4] != data[i:i + 4]),
                     min(len(got), len(data)))
            diffs[key] = {addr + k} if code else set()
            where.append("%s at 0x%X (%d bytes built, %d on the disc)" % (name, addr + k, len(got), len(data)))
    if diffs:
        return False, "differs: " + "; ".join(where[:4]), {k: a for k, a in diffs.items() if a}
    if program == "boot":
        lay = json.loads(layout_path(v).read_text())
        contents = {(i, s["name"]): built.get((i, s["name"]), b"") for i, s in enumerate(lay["sections"])}
        result = elf_from_layout(lay, contents)
    else:
        entry, recs = versions.level_records(v, int(program[6:]))
        result = versions.overlay_program(entry, [dict(r, data=built[(i, r["name"])]) for i, r in enumerate(recs)])
    (d / "program.bin").write_bytes(result)
    h = hashlib.sha1(result).hexdigest()
    want = next(r[5] for r in versions.read_programs(v) if r[0] == program and r[1] == "-")
    if h != want:
        return False, "the program's SHA-1 is %s, the disc's %s" % (h, want), {}
    return True, "MATCH: sha1 %s" % h, {}


def one(program, key, rounds):
    v = versions.get(key)
    d = build_root(v, program)
    d.mkdir(parents=True, exist_ok=True)
    with open(d / "build.log", "w") as log:
        for rnd in range(rounds + 1):
            ok, msg, fix = build(v, program, log)
            log.write("round %d: %s\n" % (rnd, msg))
            log.flush()
            if ok or not fix or rnd == rounds:
                return program, ok, msg + ("" if not rnd else " (after %d rounds)" % rnd)
            with open(asm_root(v, program) / "words.txt", "a") as f:
                f.write("".join("%08X %s\n" % (x, k[1]) for k, addrs in fix.items() for x in sorted(addrs)))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("programs", nargs="*")
    ap.add_argument("--version", required=True)
    ap.add_argument("-P", "--parallel", type=int, default=4)
    ap.add_argument("--rounds", type=int, default=3)
    ap.add_argument("--layout", action="store_true")
    a = ap.parse_args()
    v = versions.get(a.version)
    if a.layout:
        write_layout(v)
        return
    names = a.programs or versions.program_names()
    bad = 0
    with ProcessPoolExecutor(a.parallel) as ex:
        for program, ok, msg in ex.map(functools.partial(one, key=v.key, rounds=a.rounds), names):
            print("%s %s: %s" % (v.key, program, msg), flush=True)
            bad += not ok
    print("%s: %d of %d programs match" % (v.key, len(names) - bad, len(names)))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
