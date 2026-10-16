#!/usr/bin/env python3
"""
Move a volatile store into the delay slot of the `jal` right after it,
as retail's assembler did for 2.9-ee (Sony SDK) objects.

2.9-ee brackets a volatile access with `.set volatile` / `.set
novolatile` (our build keeps them as comments) and will not put it in a
delay slot itself, so a `jal` straight after a volatile store goes out in
reorder mode, unfilled. Retail's assembler then filled that slot with
the store. Ours never fills a slot, so we get `sw; jal; nop` where
retail has `jal; sw` (func_0012C990). For a store to a constant address
(an assembler macro), retail's assembler kept the `lui $at` before the
`jal` and put the `sw ...($at)` in the slot (func_00128F90).

This makes that one rewrite. For a `jal` in reorder mode whose previous
instruction is a volatile store (`sb`/`sh`/`sw`/`sd`) that neither reads
nor writes `$31`, the store (or the low half of its expansion) moves
into the slot under noreorder.

usage: python tools/fix_volatile_slot.py IN.s OUT.s
"""
import re
import sys

STORE = re.compile(r"^\s*(sb|sh|sw|sd)\s+(\$\d+),(.+?)\s*$")
BASED = re.compile(r"^(-?\d+|0x[0-9a-fA-F]+)?\((\$\d+)\)$")


def is_code(line: str) -> bool:
    s = line.strip()
    return bool(s) and not s.startswith((".", "#")) and not s.endswith(":")


def main() -> None:
    src, dst = sys.argv[1:3]
    lines = open(src).read().split("\n")
    out, n, reorder = [], 0, True
    for line in lines:
        s = line.strip()
        if s in (".set\tnoreorder", ".set noreorder"):
            reorder = False
        elif s in (".set\treorder", ".set reorder"):
            reorder = True
        if reorder and re.match(r"^\s*jal\s+\w+\s*$", line):
            # the previous code line, with only the novolatile marker
            # (and blank lines) between it and the jal
            k = len(out) - 1
            between = []
            while k >= 0 and not is_code(out[k]):
                between.append(out[k].strip())
                k -= 1
            st = STORE.match(out[k].split("#")[0]) if k >= 0 else None
            marked = "#.set\tnovolatile" in between or "#.set novolatile" in between
            before = k - 1
            while before >= 0 and not out[before].strip():
                before -= 1
            opened = before >= 0 and out[before].strip() in ("#.set\tvolatile", "#.set volatile")
            if st and marked and opened and st.group(2) != "$31" and "$31" not in st.group(3):
                op, reg, addr = st.groups()
                based = BASED.match(addr.replace(" ", ""))
                if based:
                    slot = [out.pop(k)]
                    pre = []
                elif re.match(r"^-?\d+$|^0x[0-9a-fA-F]+$", addr):
                    a = int(addr, 0) & 0xFFFFFFFF
                    hi, lo = ((a + 0x8000) >> 16) & 0xFFFF, a & 0xFFFF
                    lo = lo - 0x10000 if lo >= 0x8000 else lo
                    out.pop(k)
                    pre = [f"\tlui\t$1,0x{hi:x}"]
                    slot = [f"\t{op}\t{reg},{lo}($1)"]
                else:
                    out.append(line)
                    continue
                out += ["\t.set\tnoreorder", "\t.set\tnomacro"] + pre + [line] + slot + [
                    "\t.set\tmacro", "\t.set\treorder"]
                n += 1
                continue
        out.append(line)
    open(dst, "w").write("\n".join(out))
    print(f"fix_volatile_slot: {n} volatile store(s) moved into a jal slot {src} -> {dst}")


if __name__ == "__main__":
    main()
