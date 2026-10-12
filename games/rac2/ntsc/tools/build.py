#!/usr/bin/env python3
"""
Build the game's programs and compare them with the retail ones.

  python3 tools/build.py boot             # the boot executable
  python3 tools/build.py level 04         # one level's program
  python3 tools/build.py levels           # all 27
  python3 tools/build.py all              # the boot and all 27
  python3 tools/build.py all --jobs 12    # that many programs at once (default: half the cores, at most 8)

Each program is disassembled with splat into build/<program>/asm/, converted for SN
ProDG 2.0's assembler (tools/expand_asm.py) and assembled with it; the C in src/ is
compiled with the game's compiler (tools/compiler/) and every C function takes the
place of its assembly; then SN's linker links the program, and every loaded byte,
the segments and the entry point are compared with the retail program's. A program
that differs stops the build.

Inputs: baserom/ (tools/levels.py dump), toolchain/sn-prodg-2.0/ee/bin/{Ps2EeAs,ld}.exe
and build/compiler/tools/{cpp,cc1,as} (docs/BUILDING.md). The Windows tools run through
$RAC2_WINE (default `wine`; `{cwd}` is replaced by the directory they run in), the
32-bit Linux compiler through $RAC2_LINUX (default: directly).
"""
import hashlib
import json
import os
import re
import shlex
import shutil
import struct
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import elf  # noqa: E402
import levels  # noqa: E402

BUILD = ROOT / "build"
SN = ROOT / "toolchain/sn-prodg-2.0/ee/bin"
JOBS = os.cpu_count() or 4


def run(arguments: list[str], cwd: Path, log: Path) -> None:
    if arguments[0].lower().endswith(".exe"):
        arguments = shlex.split(os.environ.get("RAC2_WINE", "wine").replace("{cwd}", str(cwd))) + arguments
    with log.open("wb") as out:
        result = subprocess.run(arguments, cwd=cwd, stdout=out, stderr=subprocess.STDOUT, timeout=900)
    if result.returncode:
        sys.exit(f"{arguments[0]} failed; see {log}")


# --- splat and the linker script ------------------------------------------------------

def safe_name(name: str) -> str:
    return name.lstrip(".").replace(".", "_").replace("-", "_")


def splat_config(reference: Path, directory: Path, name: str) -> dict:
    """config/splat.yaml for REFERENCE: one segment per loaded section, the text as assembly
    and the rest as data. Returns the object names splat's linker script uses for each
    section, by file offset, and the ones the build gives them."""
    image = elf.read(reference)
    loads = [s for s in image["segments"] if s["type"] == elf.PT_LOAD]
    sections = sorted((s for s in image["sections"] if s["size"] and s["address"] and s["type"] != elf.SHT_NOBITS
                       and any(l["offset"] <= s["offset"] and s["offset"] + s["size"] <= l["offset"] + l["filesz"]
                               and s["address"] == l["address"] + s["offset"] - l["offset"] for l in loads)),
                      key=lambda s: s["offset"])
    (directory / "assets").mkdir(parents=True)
    shutil.copy2(reference, directory / "assets" / f"{name}.elf")
    (directory / "config").mkdir()
    options = {"platform": "ps2", "base_path": "..", "basename": name,
               "target_path": f"assets/{name}.elf", "asm_path": "asm", "src_path": "src",
               "build_path": "build", "create_asm_dependencies": True,
               "ld_script_path": "config/rac2.ld", "symbol_addrs_path": "config/symbol_addrs.txt",
               "undefined_funcs_auto_path": "config/undefined_funcs_auto.txt",
               "undefined_syms_auto_path": "config/undefined_syms_auto.txt",
               "find_file_boundaries": False, "disassemble_all": True}
    segments, renamed = [], {}
    for s in sections:
        code = s["name"] in (".text", "core.text")
        seg = f"{safe_name(s['name'])}_{s['offset']:X}"
        segment = {"name": seg, "type": "code" if code else "data", "start": s["offset"],
                   "vram": s["address"], "align": 16, "subsegments": [[s["offset"], "asm" if code else "data"]]}
        bss = [o for o in image["sections"] if o["type"] == elf.SHT_NOBITS and o["address"] == s["address"] + s["size"]]
        if bss:
            segment["bss_size"] = sum(o["size"] for o in bss)
        segments.append(segment)
        if code:
            renamed[f"build/asm/{s['offset']:X}.s.o"] = f"build/asm/{seg}.s.o"
        else:
            renamed[f"build/asm/data/{s['offset']:X}.data.s.o"] = f"build/asm/data/{seg}.data.s.o"
    for l in loads:
        end = l["offset"] + l["filesz"]
        if end < max(o["offset"] + o["filesz"] for o in loads) and not any(s["offset"] == end for s in sections):
            segments.append({"name": f"unloaded_gap_{end:X}", "type": "bin", "start": end})
    segments.sort(key=lambda s: s["start"])
    segments.append([max(l["offset"] + l["filesz"] for l in loads)])
    document = {"name": name, "sha1": hashlib.sha1(reference.read_bytes()).hexdigest(),
                "options": options, "segments": segments}
    (directory / "config/splat.yaml").write_text(json.dumps(document, indent=2) + "\n")
    (directory / "config/symbol_addrs.txt").write_text("")
    return renamed


def data_tails(reference: Path) -> dict[str, bytes]:
    """The last bytes of a load segment that ends off a word boundary, by section: splat's
    data files stop at the last whole word."""
    image = elf.read(reference)
    tails = {}
    for l in (s for s in image["segments"] if s["type"] == elf.PT_LOAD):
        end = l["offset"] + l["filesz"]
        if end % 4:
            s = next(s for s in image["sections"] if s["type"] != elf.SHT_NOBITS and s["offset"] + s["size"] == end)
            tails[f"{safe_name(s['name'])}_{s['offset']:X}"] = image["data"][end - end % 4:end]
    return tails


def adapt_linker(directory: Path, renamed: dict) -> None:
    """splat's linker script in the dialect of SN's 1990s GNU ld, one PT_LOAD per segment."""
    path = directory / "config/rac2.ld"
    text = path.read_text()
    text = re.sub(r"^\s*(unloaded_gap_\w+)_ROM_START = .*?\n\s*\1_VRAM_END = \.;\n", "\n", text,
                  flags=re.MULTILINE | re.DOTALL)
    text = re.sub(r"ALIGN\(\s*\.\s*,\s*(\d+)\s*\)", r"ALIGN(\1)", text)
    text = re.sub(r"ALIGN\(\s*([^,()]+?)\s*,\s*(\d+)\s*\)", r"((\1 + \2 - 1) & ~(\2 - 1))", text)
    text = re.sub(r"\n[ \t]*\. = ALIGN\(\d+\);(?=\n[ \t]*\w+_END = \.;)", "", text)
    text = re.sub(r"HIDDEN\(\s*(\w+)\s*=\s*([^)]+)\);", r"\1 = \2;", text)
    text = re.sub(r"\s*SUBALIGN\(\s*\d+\s*\)", "", text)
    addresses = dict(re.findall(r"^\s*(\.\S+)\s+(0x[0-9A-Fa-f]+)\s*:", text, re.MULTILINE))
    text = re.sub(r"=\s*ADDR\(\s*(\.\S+)\s*\)\s*;", lambda m: f"= {addresses.get(m.group(1), '0')};", text)
    for old, new in renamed.items():
        if (directory / new.replace("build/asm/", "asm_pp/").removesuffix(".o")).is_file():
            text = text.replace(old, new)
    reference = next((directory / "assets").glob("*.elf"))
    for name, tail in data_tails(reference).items():
        marker = f"{name}_DATA_END = .;"
        text = text.replace(marker, "\n        ".join(f"BYTE(0x{b:02X});" for b in tail) + "\n        " + marker)
    loads = [s for s in elf.read(reference)["segments"] if s["type"] == elf.PT_LOAD]
    header = "PHDRS\n{\n" + "".join(f"  load{i} PT_LOAD FLAGS({l['flags']});\n" for i, l in enumerate(loads)) + "}\n"

    def segment(m: re.Match) -> str:
        address = int(m.group(2), 16)
        i = next(i for i, l in enumerate(loads) if l["address"] <= address < l["address"] + l["memsz"])
        return m.group(1) + f" :load{i}" + m.group(3)
    text = re.sub(r"(\.[^\s]+\s+(0x[0-9A-Fa-f]+)\s*:[^{]*\{.*?\n\s*})([^\n]*)", segment, text, flags=re.DOTALL)
    path.write_text("INCLUDE config/undefined_symbols.ld\n" + header + text)


def asm_names(text: str) -> str:
    """splat's names in the assembly with their own prefixes, f_ and d_, so that no assembly
    symbol has the name of a C one (C names its functions func_ and its data D_)."""
    return re.sub(r"\b(func|D)_([0-9A-F]{8})\b", lambda m: ("f_" if m.group(1) == "func" else "d_") + m.group(2), text)


def undefined_symbols(directory: Path, extra: dict[str, int] | None = None,
                      aliases: dict[str, str] | None = None) -> None:
    """config/undefined_symbols.ld: splat's undefined symbols, every D_XXXXXXXX the assembly
    uses without defining at its address, and EXTRA."""
    values = {}
    for name in ("undefined_syms_auto.txt", "undefined_funcs_auto.txt"):
        path = directory / "config" / name
        if path.is_file():
            for line in path.read_text().splitlines():
                line = line.split("//")[0].strip()
                if "=" in line:
                    symbol, value = line.rstrip(";").split("=", 1)
                    values[asm_names(symbol.strip())] = value.strip()
    used, defined = set(), set()
    for source in (directory / "asm_pp").rglob("*.s"):
        text = source.read_text()
        used.update(re.findall(r"\b[df]_[0-9A-F]{8}\b", text))
        defined.update(re.findall(r"^\s*(\w+):", text, re.MULTILINE))
    for symbol in used - defined:
        values[symbol] = "0x" + symbol[2:]
    for symbol, value in (extra or {}).items():
        values[symbol] = f"{value:#x}"
    for symbol, expression in (aliases or {}).items():
        values[symbol] = expression
    (directory / "config/undefined_symbols.ld").write_text(
        "".join(f"{s} = {v};\n" for s, v in sorted(values.items())))


# --- C ----------------------------------------------------------------------------------

COMPILER = ROOT / "build/compiler/tools"
DEFAULT_FLAGS = "-O2 -G0 -ffunction-sections"
GP = 0x001AEFF0                        # the boot's and every level's $gp
LEVEL_BASE = 0x001A7C80                # a level's program replaces the boot's memory from here
INSTRUCTION = re.compile(r"/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+[0-9A-Fa-f]{8}\s*\*/")
TEXT_HEADER = '.set noat\n.set noreorder\n.section .text, "ax"\n'


SDK_COMPILER = ROOT / "build/compiler/sdk"


def flags_for(source: Path) -> str:
    """config/cflags.txt: `glob<TAB>flags` lines, the first that matches; else the default,
    with -G8 for a file named *-g8.c. Flags starting with "sdk" select the SDK compiler."""
    import fnmatch
    relative = source.relative_to(ROOT).as_posix()
    path = ROOT / "config/cflags.txt"
    if path.is_file():
        for line in path.read_text().splitlines():
            if line.strip() and not line.startswith("#"):
                pattern, flags = line.split("\t", 1)
                if fnmatch.fnmatch(relative, pattern):
                    return flags
    return DEFAULT_FLAGS.replace("-G0", "-G8") if source.stem.endswith("-g8") else DEFAULT_FLAGS


def compile_c(source: Path) -> Path:
    """build/obj/<source>.o, compiled again only when the source or its flags changed."""
    out = BUILD / "obj" / (source.relative_to(ROOT).as_posix() + ".o")
    flags = flags_for(source)
    stamp = out.with_suffix(".flags")
    if out.is_file() and out.stat().st_mtime >= source.stat().st_mtime and stamp.is_file() \
            and stamp.read_text() == flags and all(out.stat().st_mtime >= h.stat().st_mtime
                                                   for h in (ROOT / "include").rglob("*.h")):
        return out
    out.parent.mkdir(parents=True, exist_ok=True)
    runner = shlex.split(os.environ.get("RAC2_LINUX", ""))
    if flags.startswith("sdk"):
        # the SDK compiler's driver (integrated cpp), then the game compiler's assembler
        script = (f"set -e; t=$(mktemp -d); cd $t; cp '{source}' x.c; "
                  f"'{SDK_COMPILER}/xgcc' -B'{SDK_COMPILER}/' -S {flags[3:]} -I'{ROOT}/include' -I'{SDK_COMPILER}/include' "
                  f"-I'{source.parent}' x.c -o x.s; '{COMPILER}/as' -o x.o x.s; cp x.o '{out}'.$$; mv '{out}'.$$ '{out}'; rm -rf $t")
    else:
        script = (f"set -e; t=$(mktemp -d); cd $t; cp '{source}' x.c; "
                  f"'{COMPILER}/cpp' -I'{ROOT}/include' -I'{source.parent}' x.c > x.i; "
                  f"'{COMPILER}/cc1' -quiet {flags} x.i -o x.s; '{COMPILER}/as' -o x.o x.s; "
                  f"cp x.o '{out}'.$$; mv '{out}'.$$ '{out}'; rm -rf $t")
    result = subprocess.run([*runner, "bash", "-c", script], capture_output=True, text=True)
    if result.returncode:
        sys.exit(f"{source.relative_to(ROOT)} does not compile:\n{result.stderr[-3000:]}")
    stamp.write_text(flags)
    return out


def program_functions(name: str) -> dict[str, list[tuple[int, int]]]:
    """function name -> [(address, size)] of its places in program NAME."""
    out = {}
    if name == "boot":
        for fname, address, size in levels.read_boot_functions():
            out.setdefault(fname, []).append((address, size))
        return out
    number = int(name[:2])
    for fname, _, size, _, places in levels.read_catalogue():
        for level, address in places:
            if level == number:
                out.setdefault(fname, []).append((address, size))
    return out


def sources_for(name: str) -> list[Path]:
    """The C a program links: the boot's (src/core/, src/game/) and the levels' shared C, since
    each program holds copies of functions of the others, and a level's own."""
    found = []
    for d in ("src/core", "src/game", "src/levels/shared"):
        found += sorted((ROOT / d).rglob("*.c"))
    if name != "boot":
        found += sorted((ROOT / "src/levels" / name).rglob("*.c"))
    return found


def sext(x: int) -> int:
    return x - 0x10000 if x & 0x8000 else x


def address_in_name(symbol: str) -> int | None:
    """The address a name states: func_XXXXXXXX, D_XXXXXXXX, or one of them at its end."""
    m = re.search(r"(?:^|_)(?:func|FUN|D)_([0-9A-F]{8})$", symbol)
    return int(m.group(1), 16) if m else None


class Placement:
    """The C functions a program takes, where, and the symbol values their relocations need."""

    def __init__(self, name: str, image: dict):
        self.name, self.image = name, image
        self.functions = program_functions(name)
        self.placed = {}          # C symbol -> (address, size, object path, section name)
        self.differ = []          # (C symbol, reason)
        self.values = {}          # symbol -> value
        self.objects = []

    def retail(self, address: int, size: int) -> bytes:
        for seg in self.image["segments"]:
            if seg["type"] == elf.PT_LOAD and seg["address"] <= address < seg["address"] + seg["filesz"]:
                at = seg["offset"] + address - seg["address"]
                return self.image["data"][at:at + size]
        return b""

    def expected(self, symbol: str) -> int | None:
        """The value a symbol must have here: a function of this program its place, else the
        address its name states."""
        if symbol in self.functions and len(self.functions[symbol]) == 1:
            return self.functions[symbol][0][0]
        if re.fullmatch(r"func_L\d\d_([0-9A-F]{8})", symbol) or symbol.startswith("r1_"):
            return None          # r1_: a name carried from rac1, its rac2 value read from the bytes
        m = re.fullmatch(r"(?:D|f)_L(\d\d)_([0-9A-F]{8})", symbol)
        if m:        # data or code of level NN: there at its address, elsewhere where it reads
            return int(m.group(2), 16) if self.name[:2] == m.group(1) else None
        address = address_in_name(symbol)
        if address is not None and self.name != "boot" and address >= LEVEL_BASE:
            # a boot address in the memory a level replaces: the level has its own copy there
            return None
        return address

    def derive(self, obj: dict, index: int, address: int) -> tuple[dict, str | None]:
        """The symbol values that make section INDEX, placed at ADDRESS, read as the retail
        bytes there; or why it cannot."""
        sec = obj["sections"][index]
        data, ref = sec["data"], self.retail(address, len(sec["data"]))
        if len(ref) != len(data):
            return {}, "outside the program"
        relocs = sorted(obj["relocations"].get(index, []))
        touched = {o for o, _, _ in relocs}
        for at in range(0, len(data), 4):
            if at not in touched and data[at:at + 4] != ref[at:at + 4]:
                return {}, f"instruction at +{at:#x} differs"
        values = {}

        def agree(symbol: str, value: int) -> str | None:
            value &= 0xFFFFFFFF
            if values.setdefault(symbol, value) != value:
                return f"{symbol} needs two values"
            return None

        def name_of(sym: int) -> str:
            """A symbol's name; a section symbol (a call or address within the same object)
            becomes "§" and the section's name, whose start the linker places."""
            s = obj["symbols"][sym]
            return s["name"] if s["type"] != 3 else "§" + obj["sections"][s["section"]]["name"]
        word = lambda b, at: struct.unpack_from("<I", b, at)[0]
        i = 0
        while i < len(relocs):
            at, kind, sym = relocs[i]
            symbol = name_of(sym)
            mine, theirs = word(data, at), word(ref, at)
            if kind == elf.R_MIPS_26:
                if (mine ^ theirs) & 0xFC000000:
                    return {}, f"jump at +{at:#x} differs"
                err = agree(symbol, ((address & 0xF0000000) | ((theirs & 0x3FFFFFF) << 2))
                            - ((mine & 0x3FFFFFF) << 2))
            elif kind == elf.R_MIPS_HI16:
                # value = retail address - addend, both from the %hi and its first %lo
                j = next((k for k in range(i + 1, len(relocs)) if relocs[k][1] == elf.R_MIPS_LO16
                          and relocs[k][2] == sym), None)
                if j is None:
                    return {}, f"%hi at +{at:#x} has no %lo"
                lo_at = relocs[j][0]
                if (mine ^ theirs) & 0xFFFF0000:
                    return {}, f"%hi at +{at:#x} differs"
                target = ((theirs & 0xFFFF) << 16) + sext(word(ref, lo_at) & 0xFFFF)
                addend = ((mine & 0xFFFF) << 16) + sext(word(data, lo_at) & 0xFFFF)
                err = agree(symbol, target - addend)
            elif kind == elf.R_MIPS_LO16:
                err = None            # checked below, once every %hi has given its value
            elif kind == elf.R_MIPS_GPREL16:
                if (mine ^ theirs) & 0xFFFF0000:
                    return {}, f"gp access at +{at:#x} differs"
                err = agree(symbol, GP + sext(theirs & 0xFFFF) - sext(mine & 0xFFFF))
            elif kind == elf.R_MIPS_32:
                err = agree(symbol, theirs - mine)
            else:
                return {}, f"relocation type {kind}"
            if err:
                return {}, err
            i += 1
        for at, kind, sym in relocs:
            if kind != elf.R_MIPS_LO16:
                continue
            symbol = name_of(sym)
            mine, theirs = word(data, at), word(ref, at)
            if symbol not in values:
                return {}, f"%lo at +{at:#x} without a %hi"
            if (mine ^ theirs) & 0xFFFF0000 or (values[symbol] + sext(mine & 0xFFFF) - theirs) & 0xFFFF:
                return {}, f"%lo at +{at:#x} differs"
        for symbol, value in values.items():
            if symbol.startswith("§"):
                continue
            want = self.expected(symbol)
            if want is not None and want != value:
                return {}, f"{symbol} would be at {value:#010x}"
        return values, None

    def place(self) -> None:
        taken, needs = set(), {}       # needs: C symbol -> {section name: address}
        for source in sources_for(self.name):
            path = compile_c(source)
            obj = elf.read_object(path)
            for sym in obj["symbols"]:
                if sym["type"] != 2 or sym["name"] not in self.functions:
                    continue
                index = sym["section"]
                section = obj["sections"][index]["name"]
                # its own section, or a unit's .text that holds just it (no -ffunction-sections)
                alone = section == ".text" and sym["value"] == 0 and sym["size"] == obj["sections"][index]["size"]
                if section != ".text." + sym["name"] and not alone:
                    self.differ.append((sym["name"], f"in section {section}"))
                    continue
                for address, size in self.functions[sym["name"]]:
                    if address in taken:
                        continue
                    have = obj["sections"][index]["size"]
                    if have != size:
                        # zero padding after the function, in the C or in the program
                        lo, hi = min(have, size), max(have, size)
                        if any(obj["sections"][index]["data"][lo:]) or any(self.retail(address + lo, hi - lo)):
                            self.differ.append((sym["name"], f"{have} bytes, expected {size}"))
                            break
                        size = have
                    values, why = self.derive(obj, index, address)
                    clash = next((s for s, v in values.items() if not s.startswith("§")
                                  and self.values.get(s, v) != v), None)
                    if why or clash:
                        self.differ.append((sym["name"], why or f"{clash} needs two values"))
                        break
                    self.values.update({k: v for k, v in values.items() if not k.startswith("§")})
                    needs[sym["name"]] = {k[1:]: v for k, v in values.items() if k.startswith("§")}
                    self.placed[sym["name"]] = (address, size, path, section)
                    taken.add(address)
                    break
        # A call within one object goes to the section's placement: it must be the right one.
        changed = True
        while changed:
            changed = False
            where = {sec: a for a, _, _, sec in self.placed.values()}
            for name, need in needs.items():
                if name in self.placed and any(where.get(sec) != a for sec, a in need.items()):
                    bad = next(sec for sec, a in need.items() if where.get(sec) != a)
                    self.differ.append((name, f"needs {bad} at its place, which is not C"))
                    del self.placed[name]
                    changed = True
        self.objects = sorted({o for _, _, o, _ in self.placed.values()})


def split_text(text: str, bodies: dict[int, tuple[str, int, str]]) -> tuple[list[tuple[str, str]], dict[str, str]]:
    """Cut one assembly file at the functions in BODIES (address -> (C symbol, size, object
    input)): [("asm", text) or ("c", input)], and the labels removed with a body, each as an
    expression of its C symbol."""
    lines = text.splitlines(keepends=True)
    pieces, labels, current, i = [], {}, [], 0
    while i < len(lines):
        m = re.fullmatch(r"\.globl (f_([0-9A-F]{8}))", lines[i].strip())
        if not m or int(m.group(2), 16) not in bodies:
            current.append(lines[i])
            i += 1
            continue
        address = int(m.group(2), 16)
        symbol, size, owner = bodies[address]
        seen, j, pending = 0, i, []
        while j < len(lines) and seen < size:
            ins = INSTRUCTION.search(lines[j])
            if ins:
                if int(ins.group(1), 16) != address + seen:
                    sys.exit(f"{symbol}: the assembly at {address:#x} is not contiguous")
                seen += 4
            else:
                lab = re.match(r"^\s*([A-Za-z_.$][\w.$]*):", lines[j])
                if lab:
                    pending.append((lab.group(1), seen))
                elif re.match(r"^\s*\.section", lines[j]):
                    sys.exit(f"{symbol}: a section change inside the body at {address:#x}")
            j += 1
        if seen != size:
            sys.exit(f"{symbol}: the assembly at {address:#x} is shorter than {size} bytes")
        for label, offset in pending:
            labels[label.lstrip(".")] = symbol if offset == 0 else f"{symbol} + {offset:#x}"
        if "".join(current).strip():
            pieces.append(("asm", "".join(current)))
        pieces.append(("c", owner))
        current, i = [], j
    if INSTRUCTION.search("".join(current)):
        pieces.append(("asm", "".join(current)))
    # A local label (.L) used in one piece and defined in another becomes a global XL_ name.
    asm = [k for k, p in enumerate(pieces) if p[0] == "asm"]
    defined = {}
    for k in asm:
        for m in re.finditer(r"(?m)^\s*\.?L([0-9A-Fa-f]+):", pieces[k][1]):
            defined.setdefault(m.group(1), k)
    crossing = {m.group(1) for k in asm for m in re.finditer(r"\.?L([0-9A-Fa-f]+)\b", pieces[k][1])
                if defined.get(m.group(1), k) != k}
    crossing |= {l[1:] for l in labels if l.startswith("L") and re.fullmatch(r"L[0-9A-Fa-f]+", l)}
    for k in asm:
        body = re.sub(r"\.?L([0-9A-Fa-f]+)\b", lambda m: "XL_" + m.group(1) if m.group(1) in crossing
                      else m.group(0), pieces[k][1])
        body = re.sub(r"(?m)^(\s*)XL_([0-9A-Fa-f]+):", lambda m: f"{m.group(1)}.globl XL_{m.group(2)}\n"
                      f"{m.group(1)}XL_{m.group(2)}:" if defined.get(m.group(2)) == k else m.group(0), body)
        pieces[k] = ("asm", body)
    labels = {("XL_" + l[1:] if l[1:] in crossing and l.startswith("L") else l): v for l, v in labels.items()}
    return pieces, labels


# --- programs ----------------------------------------------------------------------------

def level_reference(number: int) -> Path:
    """The level's program as an executable, from its sections in baserom/levels/."""
    d = levels.DUMP / levels.level_dir(number)
    m = json.loads((d / "manifest.json").read_text())
    path = BUILD / "reference" / f"{levels.level_dir(number)}.elf"
    if path.is_file() and path.stat().st_mtime >= max(f.stat().st_mtime for f in d.iterdir()):
        return path           # current: tools/check.py runs side by side must not rewrite it
    sections = [{"name": "." + s["name"] if s["name"] in ("text", "data", "bss") else s["name"],
                 "address": s["address"], "type": s["type"],
                 "data": (d / f"{s['name']}.bin").read_bytes() if s["type"] != elf.SHT_NOBITS else b"\0" * s["size"]}
                for s in m["sections"]]
    path.parent.mkdir(parents=True, exist_ok=True)
    fresh = path.with_suffix(f".{os.getpid()}.tmp")
    elf.write_exec(fresh, m["entry"], sections)
    os.replace(fresh, path)   # whole or not at all, for a reader in another process
    return path


def build_program(name: str, reference: Path) -> tuple[int, int]:
    """Build program NAME; returns (C functions placed, their bytes)."""
    directory = BUILD / name
    if directory.exists():
        shutil.rmtree(directory)
    renamed = splat_config(reference, directory, "program")
    image = elf.read(reference)
    placement = Placement(name, image)
    # splat starts a function at every place of a catalogued one, named f_XXXXXXXX so that no
    # assembly label has the name of a C function
    (directory / "config/symbol_addrs.txt").write_text("".join(
        f"f_{a:08X} = 0x{a:08X}; // type:func size:0x{n:X}\n"
        for a, n in sorted({p for places in placement.functions.values() for p in places})))
    run([sys.executable, "-m", "splat", "split", "config/splat.yaml"], directory, directory / "split.log")
    run([sys.executable, str(ROOT / "tools/expand_asm.py"), "--src", "asm", "--dst", "asm_pp"],
        directory, directory / "expand.log")
    adapt_linker(directory, renamed)
    skipped = {old.removeprefix("build/asm/").removesuffix(".o") for old, new in renamed.items()
               if (directory / new.replace("build/asm/", "asm_pp/").removesuffix(".o")).is_file()}
    sources = [s for s in sorted((directory / "asm_pp").rglob("*.s"))
               if s.relative_to(directory / "asm_pp").as_posix() not in skipped]
    for source in sources:
        source.write_text(asm_names(source.read_text()))

    placement.place()
    extra, script = dict(placement.values), (directory / "config/rac2.ld").read_text()
    aliases = {}
    if placement.placed:
        bodies = {a: (sym, n, f"{o.relative_to(directory, walk_up=True).as_posix()}({sec})")
                  for sym, (a, n, o, sec) in placement.placed.items()}
        kept = []
        for source in sources:
            text = source.read_text()
            inside = {int(x, 16) for x in re.findall(r"^\.globl f_([0-9A-F]{8})$", text, re.MULTILINE)}
            mine = {a: v for a, v in bodies.items() if a in inside}
            if not mine:
                kept.append(source)
                continue
            old = "build/asm/" + source.relative_to(directory / "asm_pp").as_posix() + ".o(.text);"
            if script.count(old) != 1:
                sys.exit(f"{name}: cannot find {old} in the linker script")
            pieces, labels = split_text(text, mine)
            aliases.update(labels)
            inputs = []
            for k, (kind, body) in enumerate(pieces):
                if kind == "c":
                    inputs.append(body)
                    continue
                piece = directory / "asm_pp/pieces" / f"{source.stem}_{k}.s"
                piece.parent.mkdir(exist_ok=True)
                piece.write_text(body if body.lstrip().startswith(".set") else TEXT_HEADER + body)
                kept.append(piece)
                inputs.append("build/asm/" + piece.relative_to(directory / "asm_pp").as_posix() + ".o(.text);")
            script = script.replace(old, "\n        ".join(inputs))
            script = re.sub(re.escape(old[:-len("(.text);")]) + r"\(\.(?:data|rodata|bss)\);", "", script)
        sources = kept
        discard = "".join(f"        {o.relative_to(directory, walk_up=True).as_posix()}(*)\n" for o in placement.objects)
        end = script.rindex("}")
        script = script[:end] + "    /DISCARD/ :\n    {\n" + discard + "    }\n" + script[end:]
        (directory / "config/rac2.ld").write_text(script)
        for sym, (a, n, o, sec) in placement.placed.items():
            aliases[f"f_{a:08X}"] = sym
        extra["_gp"] = GP

    def assemble(source: Path) -> Path:
        out = directory / "build/asm" / (source.relative_to(directory / "asm_pp").as_posix() + ".o")
        out.parent.mkdir(parents=True, exist_ok=True)
        run([str(SN / "Ps2EeAs.exe"), "-o", str(out), str(source)], directory, out.with_suffix(".log"))
        return out
    with ThreadPoolExecutor(JOBS) as pool:
        objects = list(pool.map(assemble, sources))
    undefined_symbols(directory, extra, aliases)
    out = directory / "build/program.elf"
    # No object goes on the command line: the script names every one it places, which loads
    # it, and a Windows program takes 32,767 characters, fewer than a thousand C functions need.
    run([str(SN / "ld.exe"), "-T", "config/rac2.ld", "-e", hex(image["entry"]),
         "-Map", "build/program.map", "-o", "build/program.elf"],
        directory, directory / "link.log")
    same, why = elf.compare_loads(reference, out)
    c_bytes = sum(v[1] for v in placement.placed.values())
    print(f"{name}: {'OK' if same else 'DIFFERS'}: {why}; {len(placement.placed)} C functions ({c_bytes} bytes)"
          + (f", {len(placement.differ)} do not match" if placement.differ else ""))
    for sym, why_not in placement.differ:
        print(f"  {sym}: {why_not}")
    if not same:
        sys.exit(1)
    (directory / "placed.json").write_text(json.dumps(
        {"program": name, "functions": {sym: [a, n] for sym, (a, n, _, _) in sorted(placement.placed.items())},
         "differ": dict(placement.differ),
         "values": {k: f"{v:#010x}" for k, v in sorted(placement.values.items())}}, indent=1) + "\n")
    return len(placement.placed), c_bytes


def main() -> None:
    args = sys.argv[1:]
    if args == ["boot"]:
        build_program("boot", levels.BOOT)
    elif len(args) == 2 and args[0] == "level":
        number = int(args[1])
        build_program(levels.level_dir(number), level_reference(number))
    elif args[:1] in (["levels"], ["all"]) and (len(args) == 1 or len(args) == 3 and args[1] == "--jobs"):
        # The programs are independent: several at once, each in a process of its own.
        jobs = int(args[2]) if len(args) == 3 else max(1, min(8, JOBS // 2))
        programs = ([["boot"]] if args[0] == "all" else []) + [["level", str(n)] for n in sorted(levels.NAMES)]

        def one(program: list[str]) -> tuple[int, str]:
            done = subprocess.run([sys.executable, __file__, *program], capture_output=True, text=True)
            return done.returncode, (done.stdout + done.stderr).rstrip()

        failed = 0
        with ThreadPoolExecutor(jobs) as pool:
            for code, text in pool.map(one, programs):       # printed in program order
                print(text, flush=True)
                failed += bool(code)
        if failed:
            sys.exit(f"{failed} of {len(programs)} programs failed")
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
