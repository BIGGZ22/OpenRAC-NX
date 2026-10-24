#!/usr/bin/env python3
"""Reject instruction injection and fixed-register bindings in C/C++ inputs.

The evidence of this project is the instruction the **compiler** produced.
A source that carries its own instructions -- inline assembly, a global
``asm`` block, a ``.word`` directive, a register variable -- is not C
evidence, whatever it compiles to.

An external assembler-name label on a file-scope declaration is allowed: it
only names an ELF symbol, it emits no instruction.

Adapted from ``OpenRAC/rac5-sm-decomp`` (GPL-3.0); see
``THIRD_PARTY_NOTICES.md``.
"""

from __future__ import annotations

import re

TOKEN = re.compile(
    r'(?P<comment>/\*.*?\*/|//[^\n]*)|'
    r'(?P<string>"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\')|'
    r'(?P<word>[A-Za-z_$][A-Za-z0-9_$]*)|(?P<other>\S)', re.S)
ASM_NAMES = {"asm", "__asm", "__asm__"}
SYMBOL = re.compile(r'"[A-Za-z_][A-Za-z0-9_.$]*"\Z')
# A directive that hands bytes to the assembler.
DIRECTIVE = re.compile(r"(?<![\w.])\.\s*(word|byte|half|short|long|dword|4byte|2byte)\b",
                       re.I)


def policy_errors(source: str) -> list[str]:
    """Return source-line diagnostics, ignoring comments and literal contents."""
    # C joins escaped newlines before tokenization, including inside identifiers.
    text = re.sub(r'\\\r?\n', '', source)
    # Line markers and includes do not belong to the following declaration.
    text = re.sub(r'(?m)^[ \t]*#[ \t]*(?:(?:line[ \t]+)?\d+|include\b)[^\n]*', '', text)
    problems: list[str] = []

    for number, line in enumerate(text.splitlines(), 1):
        if DIRECTIVE.search(line):
            problems.append(f"line {number}: an assembler directive is not C evidence")

    tokens = [m for m in TOKEN.finditer(text) if m.lastgroup != "comment"]
    depth = 0
    start = 0
    for index, match in enumerate(tokens):
        value = match[0]
        if match.lastgroup == "word" and value in ASM_NAMES:
            prefix = [item[0] for item in tokens[start:index]]
            suffix = [item[0] for item in tokens[index + 1:index + 5]]
            # Only a declaration's assembler label, never a global asm statement,
            # local asm, volatile asm, extended operands or a register variable.
            symbol_label = (
                depth == 0 and len(prefix) >= 2 and "register" not in prefix
                and "#" not in prefix and prefix[-1] not in ("=", ",", ":")
                and len(suffix) == 4 and suffix[0] == "(" and suffix[2:] == [")", ";"]
                and SYMBOL.fullmatch(suffix[1]) is not None
            )
            if not symbol_label:
                line = text.count('\n', 0, match.start()) + 1
                problems.append(f"line {line}: inline/global assembly or "
                                "fixed-register binding is not C evidence")
        if match.lastgroup == "word" and value == "register":
            line = text.count('\n', 0, match.start()) + 1
            problems.append(f"line {line}: a register variable is a fixed-register "
                            "binding, not C evidence")
        if match.lastgroup != "other":
            continue
        if value == "{":
            depth += 1
        elif value == "}":
            depth -= 1
            if depth == 0:
                start = index + 1
        elif value == ";" and depth == 0:
            start = index + 1
    return problems


def require_c_source(source: str, label: str) -> None:
    """Raise when a source is not pure C."""
    problems = policy_errors(source)
    if problems:
        raise ValueError(f"{label}: " + "; ".join(problems))
