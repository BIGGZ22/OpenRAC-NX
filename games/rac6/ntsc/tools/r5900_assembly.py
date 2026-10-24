#!/usr/bin/env python3
"""Render a measured R5900 code interval as assembly, for the baseline gate.

This module holds no game byte: it turns instruction words into the assembly
a caller then assembles and compares with the retail code.  The rendered
listing is retail-derived and stays in the private workspace.

It needs ``rabbitizer`` 1.16.2, the same version the sibling project pins.
"""

from __future__ import annotations

import hashlib
import re
import subprocess
from pathlib import Path

ASSEMBLER_SHA256 = "c839dd63facabe7b76573c114056be61eaa7b3aa2329dd546930ab8f98898c76"
LINKER_SHA256 = "80f3724a63ce3c77c8b00d075f5c49448636518700ae251e5d6a6c809bbf5f48"
RABBITIZER_VERSION = "1.16.2"
FUNC_TARGET = re.compile(r"func_[0-9A-Fa-f]{1,8}")


def is_cop1_branch(word: int) -> bool:
    """Tell whether a word is a COP1 conditional branch.

    On the R5900 the COP1 branch is the opcode 0x11 with the register field
    8; the offset is the 16-bit immediate.
    """
    return word >> 26 == 0x11 and (word >> 21) & 0x1F == 8


def check_tool(path: Path, expected: str) -> None:
    """Refuse a tool whose bytes differ from the measured profile."""
    if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
        raise ValueError(f"tool hash does not match the measured profile: {path.name}")


def render_assembly(words: tuple[int, ...], start: int, end: int, *,
                    symbol: str = "module_text_start") -> tuple[str, dict[str, int]]:
    """Render the words of a code interval as R5900 assembly.

    The named jumps of an SNR2 module have a zero target in the file: their
    target is in the relocation table, so they are rendered as a jump to
    ``T_ZERO`` and the caller defines it.  A local jump keeps its label.
    """
    try:
        import rabbitizer
    except ImportError as error:
        raise ValueError("optional rabbitizer 1.16.2 is required") from error
    if rabbitizer.__version__ != RABBITIZER_VERSION:
        raise ValueError(f"this witness requires rabbitizer {RABBITIZER_VERSION}")

    rabbitizer.config.pseudos_enablePseudos = False
    rabbitizer.config.regNames_vr4300Cop0NamedRegisters = False
    rabbitizer.config.regNames_userFpcCsr = False

    instructions = [rabbitizer.Instruction(word, start + index * 4,
                                           rabbitizer.InstrCategory.R5900)
                    for index, word in enumerate(words)]
    invalid = [start + 4 * index for index, item in enumerate(instructions)
               if not item.isValid()]
    if invalid:
        raise ValueError(f"invalid R5900 instruction at 0x{invalid[0]:08X}")

    branches = {item.getBranchVramGeneric() for item in instructions if item.isBranch()}
    jumps = {item.getInstrIndexAsVram() for item in instructions
             if item.isJumpWithAddress()}
    labels = branches | jumps
    source = [".set noat", ".set noreorder", ".section .text",
              f".globl {symbol}", f"{symbol}:"]
    sqrt_count = zero_jump_count = raw_count = 0
    for index, (word, instruction) in enumerate(zip(words, instructions)):
        address = start + index * 4
        if address in labels:
            source.append(f"L_{address:08X}:")
        name = instruction.getOpcodeName()
        mnemonic = instruction.disassemble().strip()
        # Two forms the SN assembler does not encode the way the retail code
        # holds them: the vector instructions that write the accumulator, and
        # the COP1 branches.  They are written as raw words, which keeps the
        # size of the interval exact.
        if "$ACC" in mnemonic or is_cop1_branch(word):
            source.append(f"    .word 0x{word:08X}")
            raw_count += 1
            continue
        if name == "c1":
            # Rabbitizer does not name the R5900 two-register sqrt.s form.
            if ((word >> 26) != 0x11 or (word >> 21) & 31 != 0x10
                    or (word >> 11) & 31 != 0 or word & 63 != 4):
                source.append(f"    .word 0x{word:08X}")
                raw_count += 1
                continue
            mnemonic = f"sqrt.s $f{(word >> 6) & 31}, $f{(word >> 16) & 31}"
            sqrt_count += 1
        if instruction.isBranch():
            target = instruction.getBranchVramGeneric()
            if not start <= target < end:
                source.append(f"    .word 0x{word:08X}")
                raw_count += 1
                continue
            mnemonic = (mnemonic.rsplit(",", 1)[0] + f", L_{target:08X}"
                        if "," in mnemonic
                        else mnemonic.split()[0] + f" L_{target:08X}")
        if instruction.isJumpWithAddress():
            target = instruction.getInstrIndexAsVram()
            if target == 0:
                replacement = "T_ZERO"
                zero_jump_count += 1
            elif start <= target < end:
                replacement = f"L_{target:08X}"
            else:
                source.append(f"    .word 0x{word:08X}")
                raw_count += 1
                continue
            mnemonic = FUNC_TARGET.sub(replacement, mnemonic)
        source.append("    " + mnemonic)
    stats = {"instructions": len(words), "branch_targets": len(branches),
             "jump_targets": len(jumps - {0}), "zero_target_jumps": zero_jump_count,
             "sqrt_s": sqrt_count, "raw_words": raw_count}
    return "\n".join(source) + "\n", stats


def run_tool(command: list[str], output: Path, log: Path) -> None:
    """Run an external tool and keep its output in a log file."""
    result = subprocess.run(command, cwd=output, capture_output=True, text=True,
                            timeout=900, check=False)
    log.write_text(result.stdout + result.stderr, encoding="utf-8")
    if result.returncode:
        raise ValueError(f"{Path(command[0]).name} failed; see {log}")
