#!/usr/bin/env python3
"""
Reproduce, on compiler output, how retail's linker dead-stripped
unreferenced functions.

The rule, measured on libgcc: an unreferenced function loses its first
floor(size / 8) * 8 bytes. A function whose size is a multiple of 8
vanishes; one whose size is 4 more than a multiple of 8 leaves its last
word behind -- the delay slot of its final jump -- followed by the
alignment nop (or, at the end of an object, the linker's 0xCDCDCDCD fill).

Evidence, all against Sony's prebuilt libgcc.a (tools/libgcc_ref.py):
- L__main at 0x11DF10 is __main.o minus the first 80 of
  __do_global_dtors' 84 bytes;
- the double-precision soft-float is dp-bit.o with __negdf2 (56 bytes)
  gone without a trace and dptofp (84) reduced to the `addiu $sp,$sp,0x40`
  at 0x1206A0;
- the single-precision object at 0x1206A8 is fp-bit.o with every function
  except __unpack_f and fptodp stripped: the eight of size 4 mod 8 left
  their last word, the five of size 0 mod 8 left nothing, in order.
The "orphan epilogue fragments" at the head of many objects and the
one-instruction "fallthrough fragments" across both segments have the same
shape (docs/DECOMP_PROGRESS.md).

usage: python tools/strip_dead.py IN.s IN.o OUT.s [--export SYM] FUNC[=NAME] ...

IN.o is IN.s assembled; it supplies each function's size. FUNC=NAME makes a
surviving word a function of that name, the way splat sees it in retail.
--export SYM makes a static function global, so the linker script can give
it its func_ name.
"""
import re
import sys

from elftools.elf.elffile import ELFFile

JUMP = re.compile(r"^\s*(j|jr)\s")


def sizes(obj_path: str) -> dict[str, int]:
    elf = ELFFile(open(obj_path, "rb"))
    return {s.name: s["st_size"] for s in elf.get_section_by_name(".symtab").iter_symbols()
            if s["st_info"]["type"] == "STT_FUNC"}


def span(lines: list[str], name: str) -> tuple[int, int]:
    """Line indices of NAME's label and of its .end directive."""
    try:
        start = next(i for i, l in enumerate(lines) if l.strip() == f"{name}:")
        end = next(i for i in range(start, len(lines))
                   if re.fullmatch(rf"\.end\s+{re.escape(name)}", lines[i].strip()))
    except StopIteration:
        sys.exit(f"strip_dead: {name} not found")
    return start, end


def strip(lines: list[str], name: str, size: int, remnant: str | None) -> list[str]:
    start, end = span(lines, name)
    if size % 8 == 0:
        kept = []
    elif size % 8 == 4:
        body = lines[start:end]
        jumps = [i for i, l in enumerate(body) if JUMP.match(l)]
        if not jumps:
            sys.exit(f"strip_dead: {name} has no final jump")
        slot = next((i for i in range(jumps[-1] + 1, len(body))
                     if body[i].strip() and not body[i].strip().startswith((".", "#", "$"))),
                    None)
        # No instruction after the final jump: the compiler left the delay
        # slot to the assembler, and SN's assembler fills a slot only from
        # after the branch -- at the end of a function, with a nop.
        word = body[slot] if slot is not None else "\tnop\n"
        kept = ["\t.set\tnoreorder\n", word, "\t.set\treorder\n"]
        if remnant:
            kept = [f"\t.globl\t{remnant}\n", f"\t.ent\t{remnant}\n", f"{remnant}:\n",
                    *kept, f"\t.end\t{remnant}\n"]
    else:
        sys.exit(f"strip_dead: {name} has size {size}, not a multiple of 4")
    if remnant and not kept:
        sys.exit(f"strip_dead: {name} (size {size}) leaves nothing to name {remnant}")
    out = lines[:start] + kept + lines[end + 1:]
    # The function's own .globl/.ent directives go too: nothing may name it.
    own = re.compile(rf"\s*\.(globl|ent|type)\s+{re.escape(name)}\b")
    return [l for l in out if not own.match(l)]


def main() -> None:
    args = sys.argv[1:]
    if len(args) < 3:
        sys.exit(__doc__)
    src, obj, dst, *rest = args
    exports, specs = [], []
    while rest:
        a = rest.pop(0)
        if a == "--export":
            exports.append(rest.pop(0))
        else:
            specs.append(a)
    size_of = sizes(obj)
    lines = open(src).readlines()
    for spec in specs:
        name, _, remnant = spec.partition("=")
        if name not in size_of:
            sys.exit(f"strip_dead: no function symbol {name} in {obj}")
        lines = strip(lines, name, size_of[name], remnant or None)
    for sym in exports:
        start, _ = span(lines, sym)
        lines.insert(start, f"\t.globl\t{sym}\n")
    open(dst, "w").writelines(lines)
    print(f"strip_dead: {src} -> {dst}: stripped {len(specs)}, exported {len(exports)}")


if __name__ == "__main__":
    main()
