#!/usr/bin/env python3
"""
Give each jump table in compiled game code a section of its own, named
after the retail table it replaces, so rac1.ld.sh can place it at
retail's address (see tools/jump_tables.py).

GCC writes a switch's table into `.rdata` right after the indirect jump:

    .rdata
    .align  4
  $L12:
    .word   $L8
    ...
    .text

The k-th table in a function becomes `.rodata.<k-th jtbl_ label that
retail's copy of the function uses>`. A compiled table with no retail
counterpart cannot match, and is an error rather than an orphan section
the linker would put somewhere of its own choosing. So is any other
read-only data (a string literal): nothing places it at its retail
address yet, so game code declares its strings `extern`.

With --core, preserve ordinary constants and move only the core tables
explicitly assigned named input sections in config/core_rodata.txt. This
lets the linker retain retail's padding between originally separate tables.

usage: python tools/fix_jump_tables.py [--core] IN.s OUT.s
"""
import re
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from jump_tables import retail_tables  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
SCALAR_CONFIG = ROOT / "config/overlays/scalar_literals.tsv"


def scalar_literals():
    """Explicit 32-bit compiler constants, verified against retail bytes and references.

    A placement is not permission to change the generated value or instructions.
    Unknown/ambiguous literals, unaligned addresses and non-retail values fail closed.
    """
    rows = {}
    targets = {}
    if not SCALAR_CONFIG.exists():
        return rows
    for lineno, line in enumerate(SCALAR_CONFIG.read_text().splitlines(), 1):
        if not line or line.startswith("#"):
            continue
        fields = line.split()
        if len(fields) != 3:
            raise ValueError(f"{SCALAR_CONFIG}:{lineno}: expected function symbol word")
        func, symbol, bits = fields
        fm = re.fullmatch(r"func_L(\d{2})_[0-9A-F]{8}", func)
        sm = re.fullmatch(r"D_L(\d{2})_([0-9A-F]{8})", symbol)
        if not fm or not sm or fm[1] != sm[1] or not re.fullmatch(r"[0-9A-Fa-f]{8}", bits):
            raise ValueError(f"{SCALAR_CONFIG}:{lineno}: invalid scalar mapping")
        level, address = int(sm[1]), int(sm[2], 16)
        if address % 4:
            raise ValueError(f"{symbol}: scalar address is not aligned to 4")
        word = int(bits, 16)
        key = (func, word)
        if key in rows or (symbol in targets and targets[symbol] != word):
            raise ValueError(f"{func}: ambiguous scalar mapping for {bits}")
        asm = ROOT / "asm/overlays" / (func + ".s")
        if not asm.exists() or not re.search(r"%(?:hi|lo)\(" + re.escape(symbol) + r"\)", asm.read_text()):
            raise ValueError(f"{func}: retail does not reference {symbol}")
        folder = ROOT / "baserom/overlays" / f"level_{level:02d}"
        manifest = json.loads((folder / "manifest.json").read_text())
        records = [r for r in manifest["records"] if r["type"] != 8
                   and r["address"] <= address and address + 4 <= r["address"] + r["bytes"]]
        if len(records) != 1:
            raise ValueError(f"{symbol}: scalar is not in one initialized retail record")
        rec = records[0]
        raw = (folder / (rec["name"] + ".bin")).read_bytes()
        offset = address - rec["address"]
        if raw[offset:offset + 4] != word.to_bytes(4, "little"):
            raise ValueError(f"{symbol}: scalar value differs from retail")
        rows[key] = symbol
        targets[symbol] = word
    return rows


def scalar_owners(lines):
    """Which compiled functions reference each local constant label."""
    owners, func = {}, None
    for line in lines:
        if (m := ENT.match(line)):
            func = m[1]
        elif END.match(line):
            func = None
        elif func:
            for label in re.findall(r"\$LC\d+\b", line):
                owners.setdefault(label, set()).add(func)
    return owners


def scalar_section(lines, i, owners, configured):
    """Recognize one scalar pool; keep its label/value and only place its section."""
    if i + 4 >= len(lines) or not re.fullmatch(r"\.align\s+2", lines[i + 1].strip()):
        return None
    lm = re.fullmatch(r"(\$LC\d+):", lines[i + 2].strip())
    wm = re.fullmatch(r"\.word\s+0x([0-9A-Fa-f]{8})(?:\s*#.*)?", lines[i + 3].strip())
    if not lm or not wm or lines[i + 4].strip() != ".text":
        return None
    users = owners.get(lm[1], set())
    word = int(wm[1], 16)
    if not users or any((func, word) not in configured for func in users):
        return None
    targets = {configured[(func, word)] for func in users}
    if len(targets) != 1:
        raise ValueError(f"{lm[1]}: ambiguous scalar placement among {sorted(users)}")
    return '.section\t.rodata.' + targets.pop() + ',"a"\n'


def validate_scalar_section(func, section):
    """Independently validate the object's scalar section during strict comparison."""
    symbol = section.name.removeprefix(".rodata.")
    configured = scalar_literals()
    expected = [word for (owner, word), target in configured.items()
                if owner == func and target == symbol]
    if len(expected) != 1:
        raise ValueError(f"{func}: unconfigured scalar section {section.name}")
    if section["sh_addralign"] != 4 or section.data() != expected[0].to_bytes(4, "little"):
        raise ValueError(f"{func}: scalar section value or alignment differs from its declaration")
    return symbol


ENT = re.compile(r"^\s*\.ent\s+(\S+)")
END = re.compile(r"^\s*\.end\s+(\S+)")
RDATA = re.compile(r"^\s*\.rdata\s*$")
TABLE_WORD = re.compile(r"^\s*\.(word|gpword)\s+\$L\d+\s*$")


def is_table(lines: list[str], i: int) -> bool:
    """`.rdata`, an optional `.align`, a label, then `.word $Ln` entries."""
    j = i + 1
    if j < len(lines) and lines[j].strip().startswith(".align"):
        j += 1
    return (j + 1 < len(lines) and re.match(r"^\$L\d+:\s*$", lines[j].strip()) is not None
            and TABLE_WORD.match(lines[j + 1]) is not None)


def main() -> None:
    core = "--core" in sys.argv[1:]
    args = [arg for arg in sys.argv[1:] if arg != "--core"]
    src, dst = args
    configured = set()
    if core:
        from core_rodata import sections
        configured = {section.removeprefix(".rodata.") for _obj, section, _labels in sections()
                      if section.startswith(".rodata.jtbl_")}
    lines = Path(src).read_text().splitlines(keepends=True)
    # Ordinary executable builds need no level dumps unless a scalar pool is present.
    scalars = None
    owners = scalar_owners(lines)
    func, used, moved, scalar_count = None, 0, 0, 0
    for i, line in enumerate(lines):
        if (m := ENT.match(line)):
            func, used = m.group(1), 0
        elif END.match(line):
            func = None
        elif RDATA.match(line) and not (func and is_table(lines, i)):
            if core:
                continue
            try:
                if scalars is None:
                    scalars = scalar_literals()
                scalar = scalar_section(lines, i, owners, scalars)
            except ValueError as error:
                sys.exit(f"fix_jump_tables: error: {error}")
            if scalar:
                lines[i] = "\t" + scalar
                scalar_count += 1
                continue
            sys.exit(f"fix_jump_tables: error: {src}:{i + 1}: read-only data that is not a jump "
                     f"table (a string literal?) -- declare it extern instead")
        elif RDATA.match(line):
            labels = retail_tables(func)
            if core and not any(label in configured for label in labels):
                continue
            if used >= len(labels):
                sys.exit(f"fix_jump_tables: error: {func} has jump table #{used + 1}, retail's has "
                         f"{len(labels)} -- it cannot match")
            label = labels[used]
            used += 1
            if core and label not in configured:
                continue
            lines[i] = f'\t.section\t.rodata.{label},"a"\n'
            moved += 1
    Path(dst).write_text("".join(lines))
    print(f"fix_jump_tables: {moved} jump table(s), {scalar_count} scalar(s) {src} -> {dst}")


if __name__ == "__main__":
    main()
