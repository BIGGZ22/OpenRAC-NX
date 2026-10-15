#!/usr/bin/env python3
"""
Put a load-delay nop after a two-instruction macro load whose next
instruction reads the loaded register, as the assembler 989snd was built
with did.

In reorder mode the compiler writes a global load as a macro (`lw $2,SYM`)
and leaves load delays to the assembler (its `#nop` comments). For a symbol
outside $gp range the macro expands to `lui $2,%hi(SYM)` / `lw $2,%lo(SYM)($2)`.
989snd's assembler followed such an expansion with a nop whenever the next
instruction used $2 (func_0012E060: the `beq` testing D_0015EE00); after an
ordinary one-instruction load it added none (func_0012DDC0 has `lw $2,0($5)`
straight before `lw $3,0($2)`, and `$gp` loads right before their branches).
Ours never adds either.

A symbol counts as outside $gp range when its `.extern` size is over the -G
threshold (2) or it has none. Only reorder-mode code is touched.

usage: python tools/fix_macro_load_delay.py IN.s OUT.s
"""
import re
import sys

G = 2
LOAD = re.compile(r"^\s*(lw|lh|lhu|lb|lbu|ld|lwu)\s+(\$\d+),([A-Za-z_]\w*)(?:[+-]\w+)?\s*$")
REGS = re.compile(r"\$\d+")


def is_code(line: str) -> bool:
    s = line.strip()
    return bool(s) and not s.startswith((".", "#")) and not s.endswith(":")


def main() -> None:
    src, dst = sys.argv[1:3]
    lines = open(src).read().split("\n")
    sizes = {}
    for l in lines:
        m = re.match(r"^\s*\.extern\s+(\w+),\s*(\d+)", l)
        if m:
            sizes[m.group(1)] = int(m.group(2))
    out, n, reorder = [], 0, True
    for i, line in enumerate(lines):
        s = line.strip()
        if s in (".set\tnoreorder", ".set noreorder"):
            reorder = False
        elif s in (".set\treorder", ".set reorder"):
            reorder = True
        out.append(line)
        if not reorder:
            continue
        m = LOAD.match(line.split("#")[0])
        if not m or sizes.get(m.group(3), 0) <= G and m.group(3) in sizes:
            continue
        reg = m.group(2)
        j = i + 1
        while j < len(lines) and not is_code(lines[j]):
            if lines[j].strip().startswith(".set"):
                break
            j += 1
        if j >= len(lines) or not is_code(lines[j]):
            continue
        nxt = lines[j].split("#")[0].split(None, 1)
        if len(nxt) < 2:
            continue
        ops = nxt[1].split(",")
        # registers the next instruction reads: all operands but a
        # destination (first operand of anything other than a store/branch)
        op = nxt[0]
        reads = ops if op.startswith(("s", "b")) and op not in ("sll", "sra", "srl", "slt", "sltu",
                                                                  "sltiu", "slti", "subu", "sllv",
                                                                  "srav", "srlv") else ops[1:]
        if any(reg in REGS.findall(o) for o in reads):
            out += ["\t.set\tnoreorder", "\tnop", "\t.set\treorder"]
            n += 1
    open(dst, "w").write("\n".join(out))
    print(f"fix_macro_load_delay: {n} load-delay nop(s) {src} -> {dst}")


if __name__ == "__main__":
    main()
