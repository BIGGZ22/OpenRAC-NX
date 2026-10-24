#!/usr/bin/env python3
"""Read the retail symbol names of the modules and turn them into C prototypes.

The modules of this game carry the compiler's own mangled names, in the
SCO-style encoding the PS2 toolchains use: a readable function name, a
double underscore, then the encoded signature.

    GLOBALVARS_AddQuickSelect__FUi      free function, one unsigned int argument
    HUDFXCYCLE_Reset__FP11tHUDCYCLEFX   free function, one tHUDCYCLEFX* argument
    LevelDataCB__11ApeLevelDefP12TiXmlElement
                                          member of ApeLevelDef, (TiXmlElement*)

The encoding carries the **parameter list**, not the return type: five
bodies proved by the campaign read the argument registers and establish
it.  The tool therefore only proposes a prototype, and it does not decide
the types of a body.  A type that the encoding does not settle is
reported as ``?`` and must be fixed by the evidence of the body itself.

Usage:
    python3 tools/mangle.py GLOBALVARS_AddQuickSelect__FUi
    python3 tools/mangle.py --file <symbols.tsv> [--limit 40]
"""

from __future__ import annotations

import argparse
import csv
import re
from pathlib import Path

# Scalar type codes of the encoding.
SCALARS = {
    "v": "void",
    "i": "int",
    "c": "char",
    "b": "bool",
    "f": "float",
    "d": "double",
    "l": "long",
    "s": "short",
    "u": "unsigned int",
    "U": "unsigned char",
    "S": "unsigned short",
    "C": "const char",
    "T": "unsigned long long",
    "L": "long long",
    "M": "unsigned long",
}
# Two-character codes, tried before the single-character ones.
SCALAR_PAIRS = {
    "Ui": "unsigned int",
    "Uc": "unsigned char",
    "Us": "unsigned short",
    "Ul": "unsigned long",
    "Ub": "bool",
    "Uf": "float",
    "Ud": "double",
    "Ull": "unsigned long long",
}
CLASS_NAME = re.compile(r"(\d+)")


class Unknown:
    """A type the encoding does not settle."""

    def __init__(self, text: str):
        self.text = text

    def __str__(self) -> str:
        return "?"


def parse_type(text: str, index: int) -> tuple[str, int]:
    """Parse one type at `index`; return (C type, next index)."""
    if index >= len(text):
        return "?", index
    code = text[index]
    if code == "P":
        inner, index = parse_type(text, index + 1)
        if isinstance(inner, Unknown):
            return inner, index
        if inner.endswith("*"):
            return f"{inner[:-1]} *", index
        return f"{inner} *", index
    if code == "A":
        match = re.match(r"A(\d+)", text[index:])
        if not match:
            return "?", index + 1
        count = int(match.group(1))
        inner, index = parse_type(text, index + match.end())
        if isinstance(inner, Unknown):
            return inner, index
        return f"{inner}[{count}]", index
    match = CLASS_NAME.match(text, index)
    if match:
        length = int(match.group(1))
        start = match.end()
        name = text[start:start + length]
        if len(name) != length:
            return "?", index + 1
        return name, start + length
    pair = text[index:index + 2]
    if pair in SCALAR_PAIRS:
        return SCALAR_PAIRS[pair], index + 2
    if code in SCALARS:
        return SCALARS[code], index + 1
    return Unknown(code), index + 1


def demangle(name: str) -> dict[str, object]:
    """Return the parts of a mangled name: function, class, return, arguments."""
    result: dict[str, object] = {"raw": name, "class": None, "returns": "?",
                                  "arguments": [], "kind": "unknown"}
    kind = ""
    if name.startswith("_$_"):
        kind, rest = "destructor", name[3:]
    elif name.startswith("__"):
        kind, rest = "constructor", name[2:]
    else:
        rest = name
    base, _, suffix = rest.partition("__")
    result["kind"] = kind
    if kind == "constructor":
        result["function"] = base or "?"
        result["class"] = base or "?"
        result["returns"] = "void"
        return result
    result["function"] = base or "?"
    if not suffix:
        return result
    if suffix.startswith("F"):
        # The 'F' introduces the parameter list.  The return type is *not*
        # encoded: five proved bodies of the campaign establish it, by the
        # way the retail code reads the argument registers.
        index = 1
        while index < len(suffix):
            argument, index = parse_type(suffix, index)
            result["arguments"].append(argument)
        result["kind"] = "free function"
        return result
    # A member function: the first token is the class, then the arguments.
    first, index = parse_type(suffix, 0)
    if isinstance(first, Unknown) or not first[:1].isupper():
        result["kind"] = "unknown"
        return result
    result["class"] = first
    while index < len(suffix):
        argument, index = parse_type(suffix, index)
        result["arguments"].append(argument)
    result["kind"] = "member function"
    return result


def prototype(name: str) -> str:
    """Return the C prototype a mangled name suggests.

    The return type is not encoded: the name only carries the parameter
    list.  The prototype therefore shows it as ``?``.
    """
    parts = demangle(name)
    arguments = [str(argument) for argument in parts["arguments"]]
    returns = str(parts["returns"])
    if parts["kind"] == "member function":
        arguments = [f"{parts['class']} *self"] + arguments
    elif parts["kind"] == "constructor":
        returns = "void"
    return f"{returns} {parts['function']}({', '.join(arguments) or 'void'})"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("names", nargs="*", help="mangled names")
    parser.add_argument("--file", type=Path,
                        help="TSV with a name column (the exports catalogue)")
    parser.add_argument("--limit", type=int, default=40)
    args = parser.parse_args()

    names = list(args.names)
    if args.file:
        with args.file.open(encoding="utf-8", newline="") as handle:
            for row in csv.DictReader((line for line in handle
                                       if not line.startswith("#")), delimiter="\t"):
                names.append(row.get("symbol") or row.get("name") or "")
    if not names:
        parser.error("give names or --file")
    for name in names[:args.limit]:
        if not name:
            continue
        parts = demangle(name)
        kind = parts["kind"]
        print(f"{name}\n    {prototype(name)}    [{kind}]")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
