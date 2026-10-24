#!/usr/bin/env python3
"""Assemble a whole SN-compiled object with the original ps2eeas binary.

Only assembler declarations are prepared: extern sizes precede first use,
and our GNU label macros in retail linker remnants are expanded into
ps2eeas directives. Compiler instructions and remnant instructions are
untouched. The caller runs the existing check_macro_slots metadata step.
"""
import hashlib
from pathlib import Path
import re
import subprocess
import sys

from toolchain import sn

ROOT = Path(__file__).resolve().parents[1]
ASSEMBLER = ROOT / "toolchain/sn-prodg-24/local/sce/ee/gcc/ee/bin/ps2eeas.exe"
SHA256 = "44bcd9aaa229d8a453730142792d542e56da14761e9677cffcd1a183f603836d"
EXTERN = re.compile(r"^\s*\.extern\s+\w+\s*,\s*\d+\s*$")
INCLUDE = re.compile(r'^\s*\.include "([^"\n]+)"[^\n]*$', re.M)


def expand_include(match):
    name = match.group(1)
    if name in ("include/labels.inc", "include/macro.inc"):
        return ""
    path = (ROOT / name).resolve()
    if not path.is_relative_to((ROOT / "asm/remnants").resolve()) or path.suffix != ".s":
        raise ValueError("ps2eeas profile supports only retail linker remnants: " + name)
    text = path.read_text()
    text = re.sub(
        r"^nonmatching (\w+), (\S+)$",
        lambda m: (f".globl {m[1]}.NON_MATCHING\n.type {m[1]}.NON_MATCHING, @object\n"
                   f".size {m[1]}.NON_MATCHING, {m[2]}\n{m[1]}.NON_MATCHING:"),
        text, flags=re.M)
    text = re.sub(r"^glabel (\w+)$",
                  lambda m: f".globl {m[1]}\n.ent {m[1]}\n{m[1]}:", text, flags=re.M)
    text = re.sub(r"^endlabel (\w+)$", lambda m: f".end {m[1]}", text, flags=re.M)
    return text


def prepare(text):
    text = INCLUDE.sub(expand_include, text)
    externs = [line for line in text.splitlines(True) if EXTERN.match(line)]
    return "".join(externs) + text


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: python tools/ps2eeas.py INPUT.s OUTPUT.o")
    source, output = map(Path, sys.argv[1:])
    # A preparation error must not leave an older, apparently valid object.
    output.unlink(missing_ok=True)
    if not ASSEMBLER.is_file() or hashlib.sha256(ASSEMBLER.read_bytes()).hexdigest() != SHA256:
        raise SystemExit("Missing verified SN ps2eeas 1.9.6.516 binary in the toolchain mirror.")
    prepared = output.with_suffix(".ps2eeas.s")
    try:
        prepared.write_text(prepare(source.read_text()))
        subprocess.run(sn(str(ASSEMBLER), "-G2", "-o", str(output), str(prepared)), check=True)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        output.unlink(missing_ok=True)
        raise SystemExit(str(error))


if __name__ == "__main__":
    main()
