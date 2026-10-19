#!/usr/bin/env python3
"""
Move a volatile store or based word load into the delay slot of the `jal` right after it,
as retail's assembler did for 2.9-ee (Sony SDK) objects.

2.9-ee brackets a volatile access with `.set volatile` / `.set
novolatile` (our build keeps them as comments) and will not put it in a
delay slot itself, so a `jal` straight after a volatile store goes out in
reorder mode, unfilled. Retail's assembler then filled that slot with
the access. GNU 2.10-ee-001003-1 also fills a call slot with a
volatile `lw` immediately before it (func_00121750). Our older assembler
leaves that load before the call and adds a nop. For stores it emits
`sw; jal; nop` where
retail has `jal; sw` (func_0012C990). For a store to a constant address
(an assembler macro), retail's assembler kept the `lui $at` before the
`jal` and put the `sw ...($at)` in the slot (func_00128F90).

This models those assembler actions. For a `jal` in reorder mode whose previous
instruction is a volatile store (`sb`/`sh`/`sw`/`sd`) or a based `lw`
that neither reads nor writes `$31`, the access (or a store macro's low half) moves
into the slot under noreorder.

The load rule requires --loads, selected for whole source objects whose
GNU 2.10-ee assembler profile is recorded in config/core_text.objects.
Older SDK objects retain their original store-only behavior.

usage: python tools/fix_volatile_slot.py [--loads] IN.s OUT.s
"""
import re
import sys

ACCESS = re.compile(r"^\s*(sb|sh|sw|sd|lw)\s+(\$\d+),(.+?)\s*$")
RELOC_BASED = re.compile(r"^%lo\([^()]+\)\(\$\d+\)$")
BASED = re.compile(r"^(-?\d+|0x[0-9a-fA-F]+)?\((\$\d+)\)$")


def is_code(line: str) -> bool:
    s = line.strip()
    return bool(s) and not s.startswith((".", "#")) and not s.endswith(":")


def main() -> None:
    loads = "--loads" in sys.argv[1:]
    src, dst = [arg for arg in sys.argv[1:] if arg != "--loads"]
    lines = open(src).read().split("\n")
    out, n, reorder = [], 0, True
    history = []
    for line in lines:
        s = line.strip()
        if s in (".set\tnoreorder", ".set noreorder"):
            reorder = False
        elif s in (".set\treorder", ".set reorder"):
            reorder = True
        if is_code(line):
            history.append((line, reorder))
        if reorder and re.match(r"^\s*jal\s+\w+\s*$", line):
            # the previous code line, with only the novolatile marker
            # (and blank lines) between it and the jal
            k = len(out) - 1
            between = []
            while k >= 0 and not is_code(out[k]):
                between.append(out[k].strip())
                k -= 1
            st = ACCESS.match(out[k].split("#")[0]) if k >= 0 else None
            marked = "#.set\tnovolatile" in between or "#.set novolatile" in between
            before = k - 1
            while before >= 0 and not out[before].strip():
                before -= 1
            opened = before >= 0 and out[before].strip() in ("#.set\tvolatile", "#.set volatile")
            adjacent = all(not item or item.startswith("#") for item in between)
            # Restrict load swapping to the verified reorder window. The
            # first load following an explicit noreorder instruction is
            # kept in place by GNU 2.10 (sceCdBreak's failure arm).
            load_window = len(history) >= 3 and history[-3][1]
            enabled = st and (st.group(1) != "lw" or (loads and load_window))
            if enabled and marked and opened and adjacent and st.group(2) != "$31" and "$31" not in st.group(3):
                op, reg, addr = st.groups()
                compact_addr = addr.replace(" ", "")
                based = BASED.match(compact_addr) or (op == "lw" and RELOC_BASED.match(compact_addr))
                if based:
                    slot = [out.pop(k)]
                    pre = []
                elif op != "lw" and re.match(r"^-?\d+$|^0x[0-9a-fA-F]+$", addr):
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
    print(f"fix_volatile_slot: {n} volatile access(es) moved into a jal slot {src} -> {dst}")


if __name__ == "__main__":
    main()
