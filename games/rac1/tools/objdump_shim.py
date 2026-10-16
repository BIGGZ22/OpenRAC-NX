#!/usr/bin/env python3
"""Minimal objdump stand-in for asm-differ (see diff_settings.py).

Why not a real objdump: the only MIPS objdump on this machine is the SN
toolchain's ee-objdump, which (a) has no -z, so it collapses every run of
zero words to "..." -- silently dropping the nops that most of our
near-misses are about -- and (b) rejects asm-differ's combined -Dz flag.

This implements just the one call shape asm-differ's binary mode makes:
  objdump -m mips:5900 -Dz -bbinary -EL --start-address=A --stop-address=B FILE
and prints modern-binutils-style rows (`   addr:\tRAW \tmnem\targs`),
decoded with rabbitizer's R5900 tables. Every word is printed, zeros too.
"""
import re, sys
import rabbitizer as rz

rz.config.misc_opcodeLJust = 0

def main(argv):
    start = stop = None
    path = argv[-1]
    for a in argv[:-1]:
        if a.startswith("--start-address="):
            start = int(a.split("=", 1)[1], 0)
        elif a.startswith("--stop-address="):
            stop = int(a.split("=", 1)[1], 0)
        elif a == "--version":
            print("objdump_shim (rabbitizer)"); return 0
    data = open(path, "rb").read()
    start = start or 0
    stop = min(stop if stop is not None else len(data), len(data))
    out = [f"\n{path}:     file format binary\n\n\nDisassembly of section .data:\n"]
    out.append(f"{start:016x} <.data+0x{start:x}>:")
    for addr in range(start & ~3, stop - 3, 4):
        word = int.from_bytes(data[addr:addr + 4], "little")
        ins = rz.Instruction(word, vram=addr, category=rz.InstrCategory.R5900)
        text = ins.disassemble()
        parts = text.split(None, 1)
        mnem = parts[0] if parts else "nop"
        args = parts[1] if len(parts) > 1 else ""
        if ins.isBranch() or ins.isUnconditionalBranch():
            # Absolute target in the flattened image's address space.
            # rabbitizer's last operand is the target expression; replace it.
            ops = [o.strip() for o in args.split(", ")][:-1]
            args = ",".join(ops + ["0x%x" % ins.getBranchVramGeneric()])
        elif ins.isJumpWithAddress():
            args = "0x%x" % ins.getInstrIndexAsVram()
        args = args.replace("$", "").replace(", ", ",")
        row = f"{addr:8x}:\t{word:08x} \t{mnem}"
        out.append(row + (f"\t{args}" if args else ""))
    sys.stdout.write("\n".join(out) + "\n")
    return 0

sys.exit(main(sys.argv[1:]))
