# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Aligned instruction diffs between a compiled candidate and retail: the permuter's score, 0 for a
function that is byte-identical apart from what the linker fills in.

Adapted from rac3-decomp's tools/permuter_scorer.py (GPL-3.0), made game-agnostic. Retail comes from
the function's splat .s (each instruction line carries its word in a `/* ... WORD */` comment, and
its operands say where a relocation goes: `%hi(`, `%lo(`, `%gp_rel(`, a symbol after `jal`), the
candidate from its object's code and relocations. The two are aligned with difflib on keys that
leave out every field a relocation could fill; each aligned pair must then be equal outside the
fields either side relocates, and a branch must land on the instruction aligned with retail's
target, and a call to a function named by its address (func_00123456) must call the one retail
calls. Score = instructions that differ.

A score of 0 is a strong hint, not the proof: a masked field can hide another symbol. The engine
asks the game's own checker before it calls a candidate a match.

    python3 tools/permuter score rac3/pal func_00381140 out.o     # through the CLI
"""
from __future__ import annotations

import difflib
import re
from dataclasses import dataclass
from pathlib import Path

from . import elf

# Opcodes with a 16-bit immediate a %lo or %gp_rel relocation can fill: loads, stores, addiu, daddiu, ori.
IMM_OPS = {0x09, 0x0D, 0x19, *range(0x1A, 0x40)} - {0x1C, 0x1D}
BRANCH_OPS = {4, 5, 6, 7, 20, 21, 22, 23}
REGIMM_BRANCHES = {0, 1, 2, 3, 16, 17, 18, 19}
GP = 28
LINE = re.compile(r"^\s*/\*\s*(?:[0-9A-Fa-f]+\s+)+([0-9A-Fa-f]{8})\s*\*/\s*(\S+)\s*(.*)$")
ADDRESS_NAME = re.compile(r"^func_(?:L\d+_|FB_)?[0-9A-Fa-f]{8}$")   # a name only one function has
GLABEL = re.compile(r"^\s*(?:glabel|alabel|dlabel|jlabel|\.globl|endlabel|nonmatching|\.size)\s+(\w+)")


@dataclass
class Function:
    """One function's words, per word the mask of the fields a relocation fills, and the name of
    the function a call or jump goes to where it is known ("" elsewhere)."""
    words: list[int]
    masks: list[int]
    calls: list[str] | None = None

    def call(self, i: int) -> str:
        return self.calls[i] if self.calls else ""


def is_branch(w: int) -> bool:
    op = w >> 26
    return op in BRANCH_OPS or op == 1 and (w >> 16) & 31 in REGIMM_BRANCHES


def branch_target(w: int, i: int) -> int | None:
    if not is_branch(w):
        return None
    off = w & 0xFFFF
    return i + 1 + (off - 0x10000 if off & 0x8000 else off)


def own_mask(w: int) -> int:
    """Fields every build fills alike: jump targets, $gp offsets."""
    op, rs = w >> 26, (w >> 21) & 31
    if op in (2, 3):
        return 0x03FFFFFF
    if op in IMM_OPS and rs == GP:
        return 0xFFFF
    return 0


def key(w: int, mask: int) -> int:
    """What alignment compares: the word without relocatable fields or branch offsets."""
    op = w >> 26
    m = mask | own_mask(w)
    if is_branch(w) or op == 0x0F or op in IMM_OPS:
        m |= 0xFFFF
    return w & ~m & 0xFFFFFFFF


def has_delay_slot(w: int) -> bool:
    op = w >> 26
    return is_branch(w) or op in (2, 3) or op == 0 and w & 0x3F in (8, 9)


def trim(f: Function) -> Function:
    """F without the zero words after its end (alignment padding); a nop in a delay slot stays."""
    n = len(f.words)
    while n > 1 and f.words[n - 1] == 0 and not has_delay_slot(f.words[n - 2]):
        n -= 1
    return Function(f.words[:n], f.masks[:n], f.calls[:n] if f.calls else None)


def parse_asm(text: str, func: str, word_order: str = "le-bytes") -> Function:
    """FUNC's instructions from splat assembly TEXT, which may hold other functions. WORD_ORDER is
    "le-bytes" when the comment shows the bytes as stored (`F0FFBD27`), "word" when it shows the
    value (`27BDFFF0`). Raises KeyError when TEXT has no FUNC."""
    words, masks, calls, inside, seen = [], [], [], False, False
    for line in text.splitlines():
        label = GLABEL.match(line)
        if label:
            ends = line.split()[0] in ("endlabel", ".size")
            if label.group(1) == func and not ends:
                inside = seen = True
            elif inside and (ends or label.group(1) != func):
                break
            continue
        m = LINE.match(line) if inside else None
        if not m:
            continue
        raw = m.group(1)
        w = int(raw[6:8] + raw[4:6] + raw[2:4] + raw[0:2], 16) if word_order == "le-bytes" else int(raw, 16)
        mnemonic, operands = m.group(2), m.group(3)
        mask, call = 0, ""
        if re.search(r"%(?:hi|lo|gp_rel|got|call16)\(", operands):
            mask = 0xFFFF
        elif mnemonic in ("j", "jal") and re.match(r"[A-Za-z_.]", operands.strip()):
            mask, call = 0x03FFFFFF, operands.strip()
        words.append(w)
        masks.append(mask)
        calls.append(call)
    if not seen:
        raise KeyError(func)
    return trim(Function(words, masks, calls))


def from_object(path: Path, func: str) -> Function:
    """FUNC's instructions in object PATH, with its relocations as masks. Raises KeyError."""
    code, relocs = elf.read(path).function(func)
    words = [int.from_bytes(code[i:i + 4], "little") for i in range(0, len(code) - 3, 4)]
    masks, calls = [], []
    for i in range(len(words)):
        kind, symbol = relocs.get(i * 4, (None, ""))
        masks.append(0x03FFFFFF if kind == elf.R_MIPS_26 else 0xFFFF if kind is not None else 0)
        calls.append(symbol if kind == elf.R_MIPS_26 else "")
    return trim(Function(words, masks, calls))


def diffs(ours: Function, retail: Function) -> int:
    """Aligned instruction diffs of OURS against RETAIL; 0 is a match."""
    a = [key(w, m) for w, m in zip(ours.words, ours.masks)]
    b = [key(w, m) for w, m in zip(retail.words, retail.masks)]
    ops = difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes()
    mapping = {}
    for tag, i1, i2, j1, _ in ops:
        if tag == "equal":
            for k in range(i2 - i1):
                mapping[i1 + k] = j1 + k
    n = 0
    for tag, i1, i2, j1, j2 in ops:
        if tag != "equal":
            n += max(i2 - i1, j2 - j1)
            continue
        for k in range(i2 - i1):
            i, j = i1 + k, j1 + k
            wa, wb = ours.words[i], retail.words[j]
            if is_branch(wa):
                ta, tb = branch_target(wa, i), branch_target(wb, j)
                n += mapping.get(ta, -1 if ta != len(a) else len(b)) != tb
                continue
            m = ours.masks[i] | retail.masks[j] | own_mask(wa)
            a_call, b_call = ours.call(i), retail.call(j)
            other_callee = ADDRESS_NAME.match(a_call) and ADDRESS_NAME.match(b_call) and a_call != b_call
            n += (wa & ~m) != (wb & ~m) or bool(other_callee)
    return n
