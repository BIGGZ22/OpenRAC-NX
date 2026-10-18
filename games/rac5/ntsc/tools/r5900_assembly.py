"""Private-output helpers for measured R5900 assembly experiments.

This module contains no game bytes. Its callers verify their pinned input and
write the generated source under ignored build/ before invoking local tools.
"""

from __future__ import annotations

import hashlib
import re
import subprocess
from pathlib import Path

ASSEMBLER_SHA256 = "c839dd63facabe7b76573c114056be61eaa7b3aa2329dd546930ab8f98898c76"
LINKER_SHA256 = "80f3724a63ce3c77c8b00d075f5c49448636518700ae251e5d6a6c809bbf5f48"


def check_tool(path: Path, expected: str) -> None:
    if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
        raise ValueError(f"tool hash does not match the measured profile: {path.name}")


def render_assembly(words: tuple[int, ...], start: int, *, symbol: str,
                    allow_zero_jump: bool = False,
                    raw_cop1_branches: bool = False,
                    exclude: tuple[int, int] | None = None) -> tuple[str, dict[str, int]]:
    try:
        import rabbitizer
    except ImportError as error:
        raise ValueError("optional rabbitizer 1.16.2 is required") from error
    if rabbitizer.__version__ != "1.16.2":
        raise ValueError("this assembly witness requires rabbitizer 1.16.2")

    rabbitizer.config.pseudos_enablePseudos = False
    rabbitizer.config.regNames_vr4300Cop0NamedRegisters = False
    rabbitizer.config.regNames_userFpcCsr = False
    end = start + 4 * len(words)
    instructions = [rabbitizer.Instruction(word, start + index * 4,
                                           rabbitizer.InstrCategory.R5900)
                    for index, word in enumerate(words)]
    if any(not instruction.isValid() for instruction in instructions):
        raise ValueError("candidate code contains an invalid R5900 instruction")

    branches = {item.getBranchVramGeneric() for item in instructions if item.isBranch()}
    jumps = {item.getInstrIndexAsVram() for item in instructions if item.isJumpWithAddress()}
    if any(not start <= target < end for target in branches):
        raise ValueError("relative branch target exits measured code interval")
    outside_jumps = {target for target in jumps if not start <= target < end}
    if outside_jumps - {0} or (0 in outside_jumps and not allow_zero_jump):
        raise ValueError("absolute jump target exits measured code interval")
    if exclude is not None:
        excluded_start, excluded_end = exclude
        if (not start < excluded_start < excluded_end < end
                or excluded_start % 4 or excluded_end % 4):
            raise ValueError("replacement interval lies outside aligned code range")
        if any(excluded_start <= target < excluded_end for target in branches | jumps):
            raise ValueError("control flow targets the excluded replacement interval")

    source = [".set noat", ".set noreorder",
              ".section .text.before" if exclude is not None else ".section .text",
              f".globl {symbol}", f"{symbol}:"]
    sqrt_count = raw_count = zero_jump_count = 0
    for index, (word, instruction) in enumerate(zip(words, instructions)):
        address = start + index * 4
        if exclude is not None:
            if address == excluded_end:
                source.append(".section .text.after")
            if excluded_start <= address < excluded_end:
                continue
        if address in branches or address in jumps:
            source.append(f"L_{address:08X}:")
        name = instruction.getOpcodeName()
        if (raw_cop1_branches and index > 0 and name.startswith("bc1")
                and instructions[index - 1].getOpcodeName().startswith("c.")):
            # SN inserts a retail-absent hazard nop in the pinned boot case.
            source.append(f"    .word 0x{word:08X}")
            raw_count += 1
            continue

        mnemonic = instruction.disassemble().strip()
        if name == "c1":
            # Rabbitizer does not name the R5900 two-register sqrt.s form.
            if ((word >> 26) != 0x11 or (word >> 21) & 31 != 0x10
                    or (word >> 11) & 31 != 0 or word & 63 != 4):
                raise ValueError(f"unmapped COP1 encoding at 0x{address:08X}")
            mnemonic = f"sqrt.s $f{(word >> 6) & 31}, $f{(word >> 16) & 31}"
            sqrt_count += 1
        if instruction.isBranch():
            label = f"L_{instruction.getBranchVramGeneric():08X}"
            mnemonic = (mnemonic.rsplit(",", 1)[0] + f", {label}" if "," in mnemonic
                        else mnemonic.split()[0] + f" {label}")
        if instruction.isJumpWithAddress():
            target = instruction.getInstrIndexAsVram()
            if target == 0:
                zero_jump_count += 1
            replacement = "T_ZERO" if target == 0 else f"L_{target:08X}"
            mnemonic = re.sub(r"func_[0-9A-Fa-f]+", replacement, mnemonic)
        source.append("    " + mnemonic)
    stats = {"instructions": len(words), "branch_targets": len(branches),
             "jump_targets": len(jumps - {0}), "zero_target_jumps": zero_jump_count,
             "sqrt_s": sqrt_count, "cop1_branch_words": raw_count}
    if exclude is not None:
        stats["excluded_words"] = (excluded_end - excluded_start) // 4
    return "\n".join(source) + "\n", stats


def run_tool(command: list[str], output: Path, log: Path) -> None:
    result = subprocess.run(command, cwd=output, capture_output=True, text=True,
                            timeout=300, check=False)
    log.write_text(result.stdout + result.stderr, encoding="utf-8")
    if result.returncode:
        raise ValueError(f"{Path(command[0]).name} failed; see {log}")
