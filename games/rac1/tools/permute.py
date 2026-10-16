#!/usr/bin/env python3
"""
Find which source statement order reproduces retail, by trying all of them.

A recurring residual shape is "same instructions, same registers, wrong
order" -- GCC's scheduler emits independent stores in an order that is a
permutation of the source order, and the permutation is not the identity
and not a simple rotation. func_0012C4C0 was three such stores; reasoning
about the rotation got it wrong twice, and enumerating all 24 orderings
found the answer immediately (and it turned out to be the natural
ascending-offset order, i.e. almost certainly what the original source
said).

So: don't reason about the scheduler, enumerate against it.

Usage:
  1. Write the candidate function to a scratch .c file, marking the lines
     to permute with a trailing /*P*/ comment:

        int f(void *arg0, int arg1, int arg2) {
            char *p = (char *)arg0;
            *(int *)(p + 0xC) = arg1 >> 4;   /*P*/
            *(int *)(p + 0x10) = arg2 >> 4;  /*P*/
            *(int *)(p + 0x4) = arg1;        /*P*/
            *(int *)(p + 0x8) = arg2;        /*P*/
            return 1;
        }

  2. python tools/permute.py scratch.c func_0012C4C0 0x20

It compiles every permutation in a single translation unit (one compiler
invocation for all of them), compares each one's instruction sequence
against retail's bytes at the function's own address, and prints the
orderings that match -- or, if none do, the closest few, so you can see
whether the order is even the problem.

Up to 8 marked lines (40320 permutations); past that it refuses rather
than hanging.
"""
import itertools
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from diff_words import decode  # noqa: E402
from toolchain import sn  # noqa: E402

BASEROM = "baserom/SCES_509.16"
# Resolved to a native absolute path: CreateProcess fails with WinError 2
# on the forward-slash relative spelling even though the file is there.
CC = str(Path("toolchain/sn-prodg-24/local/sce/ee/gcc/bin/ee-gcc2953.exe").resolve())
CFLAGS = ["-O2", "-G2", "-Iinclude"]


def retail_bytes(vram, size):
    from elftools.elf.elffile import ELFFile
    with open(BASEROM, "rb") as f:
        elf = ELFFile(f)
        seg = next(s for s in elf.iter_segments() if s["p_type"] == "PT_LOAD")
        f.seek(vram - (seg["p_vaddr"] - seg["p_offset"]))
        return f.read(size)


def norm(text):
    """Instruction mnemonics+operands from compiler .s output, directives dropped."""
    out = []
    for l in text.splitlines():
        l = l.split("#")[0].strip()
        if not l or l.startswith(".") or l.endswith(":"):
            continue
        out.append(re.sub(r"\s+", " ", l))
    return out


def main():
    src, name, size = sys.argv[1], sys.argv[2], int(sys.argv[3], 16)
    lines = Path(src).read_text().splitlines()
    marked = [i for i, l in enumerate(lines) if l.rstrip().endswith("/*P*/")]
    if not marked:
        raise SystemExit("no lines marked with a trailing /*P*/ comment")
    if len(marked) > 8:
        raise SystemExit("%d marked lines is %d permutations -- too many"
                         % (len(marked), len(list(itertools.permutations(range(len(marked)))))))

    # Rename the function under test, not whatever identifier happens to
    # appear first -- an extern declaration above the definition used to
    # win the regex, and every variant then kept the same name, so the
    # translation unit was 24 redefinitions of one function.
    fname = name
    if fname not in "\n".join(lines):
        raise SystemExit("%s does not appear in %s" % (fname, src))

    variants, out = [], []
    for k, perm in enumerate(itertools.permutations(range(len(marked)))):
        v = list(lines)
        for slot, pick in zip(marked, perm):
            v[slot] = lines[marked[pick]]
        tag = "pv%04d" % k
        out.append("\n".join(v).replace(fname, tag, 1))
        variants.append((tag, perm))

    tmp = Path("build-sn/_permute.c")
    tmp.parent.mkdir(exist_ok=True)
    tmp.write_text("\n".join(out))
    asm = Path("build-sn/_permute.s")
    r = subprocess.run(sn(CC, *CFLAGS, "-S", "-o", str(asm), str(tmp)),
                       capture_output=True, text=True)
    if r.returncode:
        print(r.stderr[-2000:])
        raise SystemExit("compile failed")

    blocks, cur = {}, None
    for l in asm.read_text().splitlines():
        m = re.match(r"^(pv\d{4}):", l)
        if m:
            cur = m.group(1); blocks[cur] = []; continue
        if cur is not None:
            if l.strip().startswith(".end"):
                cur = None
            else:
                blocks[cur].append(l)

    want = [decode(int.from_bytes(retail_bytes(int(name[5:], 16), size)[i:i + 4], "little"))
            for i in range(0, size, 4)]
    print("retail (%d instructions):" % len(want))
    for i, w in enumerate(want):
        print("   +%#05x %s" % (i * 4, w))
    print()

    scored = []
    for tag, perm in variants:
        got = norm("\n".join(blocks.get(tag, [])))
        # compare shape only: mnemonic + operand text, retail's decode is
        # a different syntax, so score on mnemonic sequence
        gm = [g.split()[0] for g in got]
        wm = [w.split()[0] for w in want if w != "nop"]
        n = sum(1 for a, b in zip(gm, wm) if a == b)
        scored.append((n - abs(len(gm) - len(wm)) * 2, perm, gm))
    scored.sort(key=lambda t: -t[0])
    best = scored[0][0]
    print("best score %d; orderings achieving it:" % best)
    for s, perm, gm in scored:
        if s < best:
            break
        print("   source order ->", list(perm), " ", " ".join(gm))
    print("\nMarked source lines, in file order:")
    for i, ix in enumerate(marked):
        print("   [%d] %s" % (i, lines[ix].strip()))


main()
