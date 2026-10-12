#!/usr/bin/env python3
"""
Check one function's C quickly, without building whole programs.

  python3 tools/check.py func_XXXXXXXX          # or func_LNN_XXXXXXXX
  python3 tools/check.py func_XXXXXXXX --diff   # also the instructions side by side

Compiles the file in src/ that defines it and tries the function at each of its places (the
boot, and every level that has it) the way tools/build.py places it: the same instructions,
with symbol values read from the retail bytes under its relocations. A function that passes
here still has to link: tools/build.py is the check that counts.
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import build  # noqa: E402
import elf  # noqa: E402
import levels  # noqa: E402


def source_of(name: str) -> Path | None:
    for f in sorted((ROOT / "src").rglob("*.c")):
        for line in f.read_text(errors="replace").splitlines():
            if build.re.match(rf"^(?:[A-Za-z_][^;=#]*?\b)?{name}\s*\(", line) and not line.rstrip().endswith(";") \
                    and not line.startswith(("extern", "typedef")):
                return f
    return None


def disassemble(data: bytes, address: int) -> list[str]:
    import rabbitizer
    out = []
    for i in range(0, len(data), 4):
        word = int.from_bytes(data[i:i + 4], "little")
        out.append(rabbitizer.Instruction(word, vram=address + i).disassemble())
    return out


def main() -> None:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) != 1:
        sys.exit(__doc__)
    name, diff = args[0], "--diff" in sys.argv
    source = source_of(name)
    if source is None:
        sys.exit(f"no file in src/ defines {name} (is it still an INCLUDE_ASM line?)")
    obj = elf.read_object(build.compile_c(source))
    sym = next((s for s in obj["symbols"] if s["name"] == name and s["type"] == 2), None)
    if sym is None:
        sys.exit(f"{source.relative_to(ROOT)} compiled, but has no function {name}")
    programs = ["boot"] if any(n == name for n, _, _ in levels.read_boot_functions()) else []
    for row in levels.read_catalogue():
        if row[0] == name:
            programs += [levels.level_dir(lv) for lv, _ in row[4]]
    if not programs:
        sys.exit(f"{name} is in neither catalogue")
    index = sym["section"]
    ok = 0
    for program in programs:
        image = elf.read(levels.BOOT if program == "boot" else build.level_reference(int(program[:2])))
        placement = build.Placement(program, image)
        address, size = placement.functions[name][0]
        have = obj["sections"][index]["size"]
        why = None
        if have != size:
            why = f"{have} bytes, expected {size}"
        else:
            _, why = placement.derive(obj, index, address)
        print(f"{program}: {'OK' if why is None else why}")
        ok += why is None
        if why and diff:
            mine = disassemble(obj["sections"][index]["data"], address)
            theirs = disassemble(placement.retail(address, size), address)
            for k in range(max(len(mine), len(theirs))):
                a = mine[k] if k < len(mine) else ""
                b = theirs[k] if k < len(theirs) else ""
                print(f"  {address + 4 * k:08X}  {a:<40} {'  ' if a == b else '| '}{b}")
            diff = False          # once is enough
    print(f"{name}: {ok} of {len(programs)} places match ({source.relative_to(ROOT)})")


if __name__ == "__main__":
    main()
