#!/usr/bin/env python3
"""
Add the nops retail's assembler added, to compiled game code.

Retail's text segment was assembled by SN Systems' own ps2eeas, not the
GNU as this build uses, and ps2eeas inserts nops GNU as does not.
core_text was assembled by the same GNU as the build runs (the compiler
driver's ee/bin/as.exe), so this runs on src/game/ objects, plus
989snd.o, whose retail code has ps2eeas's short-loop padding on loops
with calls (func_0012E688, func_0012EC60). Four rules:

1. Short loops (the R5900 short-loop erratum). Every backward branch
   whose loop -- the target through the branch itself -- is shorter than
   six instructions gets nops right before the branch until it is
   exactly six. Measured on ps2eeas directly (tools in the toolchain
   mirrors): in reorder and noreorder code alike, branch-likely too,
   counting any other instructions in the loop, delay slots included.
   This is why 309 of retail's backward branches span exactly six
   instructions. The driver's as.exe already pads loops that contain no
   call, in the first-pass object this measures, so in effect this adds
   the padding ps2eeas also gives loops with a call.

   The driver's as.exe measures differently, though: it counts from the
   target label to the first jump after it, not to the branch. On a
   backward branch that is not a loop -- a cross-jump back into a short
   block that returns -- it pads where ps2eeas did not (func_00209188:
   three nops). Where GNU as gave a branch more nops than ps2eeas's rule,
   the branch is written as a `.word` with its offset computed from the
   label, which GNU as cannot pad, and gets ps2eeas's padding instead.
   Only branches in the compiler's noreorder blocks, whose delay slot is
   spelled out, are rewritten.

   Adding explicit padding can also suppress GNU's implicit FP compare
   hazard nop. Preserve that nop when padding a short FP loop.

2. FP compare then branch. A `c.cond.fmt` immediately followed by a
   `bc1*` gets a nop between them. In retail text the pair is never
   adjacent (191 of 191 have the nop). GNU as adds one on its own in
   some contexts and not in others (not when the compiler leaves an
   unfilled bc1 in reorder mode), so the pairs are found in the
   assembled object, never guessed from the source.

   GNU as can already have inserted this nop before a label between the
   compare and branch. SN's label instead names the nop: a jump into the
   shared branch must execute the hazard delay too. Spell the existing
   nop out after the label in that case, replacing GNU's implicit one
   without changing the instruction count (func_L00_002C0358).

3. GPR to FPU move. An `mtc1 $x, $fN` -- written out or from an `li.s`
   expansion -- immediately followed by an instruction that reads $fN
   gets a nop between them, in reorder code (215 times in retail text;
   the 17 adjacent pairs are all in hand-written noreorder code). GNU
   as never adds this one. Sites must be found alike in the object and in
   the source, or the tool refuses to guess. fastfunc.o is skipped: its
   retail code is hand-written noreorder assembly (every mtc1 there is
   directly followed by its reader), even where our C reproduces it.

4. FPU to GPR move, then a branch. A reorder-mode `mfc1 $x, $fN`
   followed by the compiler's own noreorder branch that reads $x: GNU as
   puts a nop between them and ps2eeas did not (measured on ps2eeas,
   docs/DECOMP_PROGRESS.md; in retail's compiled text the pair occurs
   four times, in func_L00_00269BE8 and func_L00_002761C0, adjacent each
   time, and never with a nop). The `mfc1` is put in a noreorder block
   of its own, where GNU as leaves the pair as written. Only pairs GNU
   as padded in the assembled object are touched, and source and object
   must show the same pairs.

ps2eeas itself is not the build's assembler yet (docs/BUILD_FIDELITY.md,
"Checked against the real ps2eeas"): it cannot read the GNU macros the
retail-assembly stubs use, and in one pass it only uses $gp for symbols
whose size it has seen. tools/check_ps2eeas.py runs it on the same compiler
output and compares: this tool and its two siblings give its code for
98.4% of the C functions.

Two passes, so that nothing is guessed about macro expansion or delay
slots: IN.o is IN.s assembled. Everything is measured in it, per compiled
function (the INCLUDE_ASM stubs are retail's bytes and already padded),
and the nops go into IN.s before the matching branch. A loop's span
counts the FP nops inserted inside it.

usage: python tools/ps2eeas_nops.py IN.s IN.o OUT.s
"""
import re
import sys

import rabbitizer as rz
from elftools.elf.elffile import ELFFile

MIN_SPAN = 6
# Objects whose retail code is hand-written noreorder assembly: rule 3
# does not apply to their functions.
NOREORDER_OBJECTS = {"build-sn/game/fastfunc.o"}


def noreorder_ranges():
    """[(start, end)] of the NOREORDER_OBJECTS in text, from the link list."""
    rows = []
    for line in open("config/text.objects"):
        line = line.strip()
        if line and not line.startswith("#"):
            obj, start = line.split()[:2]
            rows.append((int(start, 16), obj))
    rows.sort()
    return [(a, rows[k + 1][0] if k + 1 < len(rows) else 1 << 32)
            for k, (a, obj) in enumerate(rows) if obj in NOREORDER_OBJECTS]
BRANCH_LINE = re.compile(r"^\s*(b[a-z0-9]*)\s+(.*)$")
BC1_LINE = re.compile(r"^\s*bc1(t|f)l?\s")
LABEL_LINE = re.compile(r"^\s*(\$L\w+|\.L\w+):")


def label_before_hazard(lines, j):
    """True when a local label sits between an FP compare and the bc1 at
    source line J. Retail puts such a label before the hazard nop (a jump
    to it executes the nop); GNU as, which adds the nop itself, puts the
    label after it, so the nop is spelled out after the label instead."""
    seen, k = False, j - 1
    while k >= 0:
        stripped = lines[k].split("#")[0].strip()
        if LABEL_LINE.match(lines[k]):
            seen = True
        elif stripped and not stripped.startswith("."):
            return seen and re.match(r"c\.[a-z]+\.s\b", stripped) is not None
        k -= 1
    return False


def is_local_branch(mnemonic: str, operands: str) -> bool:
    """A conditional or unconditional branch to a local label: what closes a
    loop in compiled code. (`break` also starts with b; its operands are
    numbers, so it never qualifies.)"""
    target = operands.split(",")[-1].strip()
    return target.startswith(("$L", ".L"))


def decode(text, addr):
    return rz.Instruction(int.from_bytes(text[addr:addr + 4], "little"), vram=addr,
                          category=rz.InstrCategory.R5900)


def reads_fpr(mnemonic: str, operands: str, reg: str) -> bool:
    """Does this instruction read FP register REG? Normally the first
    operand is the destination; compares, stores (swc1, or s.s as the
    compiler writes them), moves out of the FPU and the accumulator ops
    (adda.s ...) read all of theirs."""
    parts = [o.strip() for o in operands.split(",")]
    used = re.compile(re.escape(reg) + r"(?![0-9])")
    if (mnemonic.startswith("c.") or mnemonic in ("swc1", "s.s", "mfc1")
            or re.match(r"(adda|suba|mula|madda|msuba)\.s$", mnemonic)):
        return any(used.search(p) for p in parts)
    return any(used.search(p) for p in parts[1:])


def scan(start, size, text):
    """(backward, fp, moves) for one function: backward is [(target,
    branch)] for every backward branch in order; fp is [(address,
    needs_nop)] for every bc1 in order, needs_nop when the instruction
    before it is an FP compare; moves is the address of every instruction
    that reads the FP register an mtc1 right before it wrote (the mtc1
    not in a delay slot)."""
    backward, fp, moves = [], [], []
    prev = before = None
    for addr in range(start, start + size, 4):
        ins = decode(text, addr)
        name = ins.getOpcodeName()
        if name.startswith("bc1"):
            fp.append((addr, prev is not None and prev.getOpcodeName().startswith("c.")))
        if prev is not None and prev.getOpcodeName() == "mtc1" \
                and not (before is not None and (before.isBranch() or before.isJump())):
            reg = prev.disassemble().split(",")[-1].strip()
            if reads_fpr(name, ins.disassemble().split(None, 1)[-1], reg):
                moves.append(addr)
        if ins.isBranch():
            target = ins.getBranchVramGeneric()
            if target <= addr:
                backward.append((target, addr))
        before, prev = prev, ins
    return backward, fp, moves


INSN_LINE = re.compile(r"^\s*([a-z][a-z0-9.]*)\s*(.*?)\s*(#.*)?$")


def gnu_padding(text, start, branch, lines, j):
    """Nops GNU as put right before the branch at BRANCH (source line J):
    the object's run of nops there, not counting a delay slot, less the
    nops the source itself writes there."""
    run, a = 0, branch - 4
    while a >= start and text[a:a + 4] == b"\0\0\0\0":
        if a - 4 >= start:
            prev = decode(text, a - 4)
            if prev.isBranch() or prev.isJump():
                break
        run += 1
        a -= 4
    # A bc1's nop after its FP compare is the hazard nop (rule 2), which
    # ps2eeas has too.
    if run and a >= start and decode(text, branch).getOpcodeName().startswith("bc1") \
            and decode(text, a).getOpcodeName().startswith("c."):
        run -= 1
    written, k = 0, j - 1
    while k >= 0:
        stripped = lines[k].split("#")[0].strip()
        if not stripped or stripped.startswith(".") or stripped.endswith(":"):
            k -= 1
        elif stripped == "nop":
            written += 1
            k -= 1
        else:
            break
    return max(0, run - written)


def implicit_loop_compare_nop(text, start, branch, lines, j):
    """Whether explicit loop padding will replace GNU's FP hazard nop.

    GNU supplies the nop between a compare and a bc1 branch, but stops
    supplying it when we insert explicit nops there. That existing nop
    must be spelled out along with any additional short-loop padding.
    """
    if not decode(text, branch).getOpcodeName().startswith("bc1"):
        return False
    a = branch - 4
    if a < start or text[a:a + 4] != b"\0\0\0\0":
        return False
    while a >= start and text[a:a + 4] == b"\0\0\0\0":
        a -= 4
    if a < start or not decode(text, a).getOpcodeName().startswith("c."):
        return False
    for k in range(j - 1, -1, -1):
        source = lines[k].split("#")[0].strip()
        if not source or source.startswith(".") or source.endswith(":"):
            continue
        return source.startswith("c.")
    return False


REGS = {"$zero": 0, "$at": 1, "$gp": 28, "$sp": 29, "$fp": 30, "$ra": 31}
# op -> (primary opcode, operands: s = rs, t = rt, a digit = REGIMM's rt code)
BRANCHES = {
    "b": (0x04, ""), "beq": (0x04, "st"), "bne": (0x05, "st"),
    "beql": (0x14, "st"), "bnel": (0x15, "st"),
    "beqz": (0x04, "s"), "bnez": (0x05, "s"), "beqzl": (0x14, "s"), "bnezl": (0x15, "s"),
    "blez": (0x06, "s"), "bgtz": (0x07, "s"), "blezl": (0x16, "s"), "bgtzl": (0x17, "s"),
    "bltz": (0x01, "s0"), "bgez": (0x01, "s1"), "bltzl": (0x01, "s2"), "bgezl": (0x01, "s3"),
}


def branch_as_word(line):
    """LINE, a branch to a local label, as a `.word` that encodes it, or
    None for a form not handled here."""
    m = re.match(r"^(\s*)([a-z]+)\s+([^#]*?)\s*(#.*)?$", line.rstrip("\n"))
    if not m or m.group(2) not in BRANCHES:
        return None
    opcode, shape = BRANCHES[m.group(2)]
    ops = [o.strip() for o in m.group(3).split(",")]
    label, regs = ops[-1], ops[:-1]

    def reg(r):
        return REGS[r] if r in REGS else int(r[1:])
    word = opcode << 26
    if shape.startswith("s"):
        word |= reg(regs[0]) << 21
    if shape == "st":
        word |= reg(regs[1]) << 16
    elif shape[1:]:
        word |= int(shape[1:]) << 16
    return f"{m.group(1)}.word\t{word:#010x} | ((({label} - . - 4) >> 2) & 0xFFFF)\n"


EXTERN = re.compile(r"^\s*\.extern\s+(\w+)\s*,\s*(\d+)")
SMALL = 2           # -G2: an object this size or smaller is $gp-relative, one instruction


def macro_access(lines, operands: str) -> bool:
    """A load or store to a bare symbol the assembler expands to `lui $at`
    plus the access (a global bigger than SMALL, or undeclared): the lui
    then sits between the mtc1 and the read, so there is no hazard. A
    $gp-relative one (an `__gp` alias, a small object) stays adjacent."""
    target = operands.split(",")[-1].strip()
    if "(" in target or not re.match(r"^[A-Za-z_][\w.]*(\s*[+-]\s*\w+)?$", target):
        return False
    sym = re.split(r"\s*[+-]", target)[0]
    if sym.endswith("__gp"):
        return False
    sizes = {m.group(1): int(m.group(2)) for l in lines if (m := EXTERN.match(l))}
    return sizes.get(sym, SMALL + 1) > SMALL


def move_sites(lines, start, end):
    """Source lines that read the FP register a reorder-mode `mtc1`/`li.s`
    on the previous instruction line wrote; the nop goes before them. A
    macro access to a large global is not one (macro_access())."""
    sites, reorder, pending = [], True, None
    for j in range(start, end):
        stripped = lines[j].strip()
        if stripped in (".set\tnoreorder", ".set noreorder"):
            reorder, pending = False, None
            continue
        if stripped in (".set\treorder", ".set reorder"):
            reorder, pending = True, None
            continue
        if not stripped or stripped.startswith("#"):
            continue
        if stripped.endswith(":") and not stripped.startswith("."):
            continue        # a label reached by fallthrough: the read after it still counts, as in scan()
        m = INSN_LINE.match(stripped)
        if not m or stripped.startswith("."):
            pending = None
            continue
        mnemonic, operands = m.group(1), m.group(2)
        if pending and reads_fpr(mnemonic, operands, pending) and not macro_access(lines, operands):
            sites.append(j)
        pending = None
        if reorder and mnemonic in ("mtc1", "li.s"):
            pending = operands.split(",")[1 if mnemonic == "mtc1" else 0].strip()
    return sites


def fp_label_nop(lines, start, j, text, addr):
    """An implicit GNU FP nop whose shared-branch label skipped the nop.

    Require both the assembled hazard pair and a source label between
    the compare and branch. An explicit source nop already fixes the
    label placement; ordinary comparisons without labels need no change.
    """
    if addr < 8 or text[addr - 4:addr] != b"\0\0\0\0" \
            or not decode(text, addr - 8).getOpcodeName().startswith("c."):
        return False
    labelled = False
    for k in range(j - 1, start - 1, -1):
        stripped = lines[k].split("#", 1)[0].strip()
        if not stripped:
            continue
        if stripped.endswith(":"):
            labelled = True
            continue
        if stripped.startswith("."):
            continue
        m = INSN_LINE.match(stripped)
        return labelled and m is not None and m.group(1).startswith("c.")
    return False


def reads_gpr(word: int, reg: int) -> bool:
    """Is WORD a conditional branch that reads general register REG?"""
    op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
    if op in (0x04, 0x05, 0x14, 0x15):          # beq bne beql bnel
        return reg in (rs, rt)
    return op in (0x01, 0x06, 0x07, 0x16, 0x17) and reg == rs   # REGIMM, blez bgtz blezl bgtzl


def mfc1_branches(start, size, text):
    """[(address of the mfc1, padded)] for every `mfc1 $x,$fN`, not in a
    delay slot, whose next instruction -- past one nop when PADDED -- is
    a branch that reads $x."""
    def word(a):
        return int.from_bytes(text[a:a + 4], "little")
    sites = []
    for addr in range(start, start + size - 4, 4):
        w = word(addr)
        if w & 0xFFE007FF != 0x44000000:
            continue
        if addr > start:
            before = decode(text, addr - 4)
            if before.isBranch() or before.isJump():
                continue
        reg = (w >> 16) & 31
        if reads_gpr(word(addr + 4), reg):
            sites.append((addr, False))
        elif word(addr + 4) == 0 and addr + 8 < start + size and reads_gpr(word(addr + 8), reg):
            sites.append((addr, True))
    return sites


MFC1_LINE = re.compile(r"^\s*mfc1\s+(\$\w+)\s*,\s*\$f\d+\s*(#.*)?$")


def mfc1_sites(lines, start, end):
    """Source lines of a reorder-mode `mfc1 $x,$fN` whose next instruction
    is a branch, in the compiler's noreorder block, that reads $x."""
    sites, reorder = [], True
    for j in range(start, end):
        stripped = lines[j].strip()
        if stripped in (".set\tnoreorder", ".set noreorder"):
            reorder = False
        elif stripped in (".set\treorder", ".set reorder"):
            reorder = True
        m = MFC1_LINE.match(lines[j])
        if not m or not reorder:
            continue
        k, entered = j + 1, False
        while k < end:
            s = lines[k].split("#")[0].strip()
            if s in (".set\tnoreorder", ".set noreorder"):
                entered = True
            elif s and s not in (".set\tnomacro", ".set nomacro"):
                break
            k += 1
        bm = re.match(r"^\s*([a-z]+)\s+([^#]*)", lines[k]) if k < end and entered else None
        if not bm or bm.group(1) not in BRANCHES:
            continue
        used = re.compile(re.escape(m.group(1)) + r"(?![0-9])")
        if any(used.fullmatch(o.strip()) for o in bm.group(2).split(",")[:-1]):
            sites.append(j)
    return sites


def main() -> None:
    src_path, obj_path, dst_path = sys.argv[1:4]
    lines = open(src_path).readlines()
    elf = ELFFile(open(obj_path, "rb"))
    text = elf.get_section_by_name(".text").data()
    tidx = next(i for i, s in enumerate(elf.iter_sections()) if s.name == ".text")
    funcs = {s.name: (s["st_value"], s["st_size"])
             for s in elf.get_section_by_name(".symtab").iter_symbols()
             if s["st_shndx"] == tidx and s["st_info"]["type"] == "STT_FUNC" and s["st_size"]}

    inserts = {}  # line index -> number of nops to put before it
    as_words = set()  # branch lines to write as .word (GNU as over-padded them)
    unpad_moves = set()  # mfc1 lines to put in a noreorder block of their own (rule 4)
    loops = fps = moves = fp_labels = 0
    hand_written = noreorder_ranges()
    i = 0
    while i < len(lines):
        m = re.match(r"^\s*\.ent\s+(\S+)", lines[i])
        if not m or m.group(1) not in funcs:
            i += 1
            continue
        name = m.group(1)
        end = next(j for j in range(i, len(lines)) if re.match(rf"^\s*\.end\s+{re.escape(name)}\s*$", lines[j]))
        # Backward branches only: a loop closes backward, and the assembler's
        # own macro branches (the div-by-zero trap guard) all jump forward,
        # so pairing the backward ones keeps source and object one to one.
        # bc1 branches are never macro-generated, so all of them pair.
        seen, src_back, src_bc1 = set(), [], []
        for j in range(i, end):
            lm = re.match(r"^\s*(\$L\w+|\.L\w+):", lines[j])
            if lm:
                seen.add(lm.group(1))
            if BC1_LINE.match(lines[j]):
                src_bc1.append(j)
            bm = BRANCH_LINE.match(lines[j])
            if bm and is_local_branch(bm.group(1), bm.group(2)) \
                    and bm.group(2).split(",")[-1].strip() in seen:
                src_back.append(j)
        start, size = funcs[name]
        obj_back, obj_fp, obj_moves = scan(start, size, text)
        src_moves = move_sites(lines, i, end)
        vram = int(name[5:], 16) if re.fullmatch(r"func_[0-9A-Fa-f]{8}", name) else None
        if vram is not None and any(a <= vram < b for a, b in hand_written):
            obj_moves, src_moves = [], []
        if len(obj_back) != len(src_back) or len(obj_fp) != len(src_bc1) \
                or len(obj_moves) != len(src_moves):
            sys.exit(f"ps2eeas_nops: {name}: object has {len(obj_back)} backward branches, "
                     f"{len(obj_fp)} bc1, {len(obj_moves)} mtc1 uses; source has "
                     f"{len(src_back)}, {len(src_bc1)}, {len(src_moves)} -- refusing to guess")
        obj_mfc1, src_mfc1 = mfc1_branches(start, size, text), mfc1_sites(lines, i, end)
        if len(obj_mfc1) != len(src_mfc1):
            sys.exit(f"ps2eeas_nops: {name}: object has {len(obj_mfc1)} mfc1 then branch pairs, "
                     f"source has {len(src_mfc1)} -- refusing to guess")
        # The nops GNU as put after an mfc1 and ps2eeas did not: they go, so
        # a loop around one is that much shorter.
        gone = [addr + 4 for addr, padded in obj_mfc1 if padded]
        unpad_moves.update(j for (addr, padded), j in zip(obj_mfc1, src_mfc1) if padded)
        fp_nops = [addr for addr, needs in obj_fp if needs] + obj_moves
        for (addr, needs), j in zip(obj_fp, src_bc1):
            if needs or label_before_hazard(lines, j):
                inserts[j] = inserts.get(j, 0) + 1
                fps += 1
            elif fp_label_nop(lines, i, j, text, addr):
                inserts[j] = inserts.get(j, 0) + 1
                fp_labels += 1
        for j in src_moves:
            inserts[j] = inserts.get(j, 0) + 1
            moves += 1
        for (target, branch), j in zip(obj_back, src_back):
            span = (branch - target) // 4 + 1 + sum(1 for a in fp_nops if target <= a <= branch) \
                - sum(1 for a in gone if target <= a < branch - 4)
            gnu = gnu_padding(text, start, branch, lines, j)
            need = max(0, MIN_SPAN - (span - gnu))
            if gnu > need:
                as_words.add(j)
                if need:
                    inserts[j] = inserts.get(j, 0) + need
                    loops += 1
            elif span < MIN_SPAN:
                replace_hazard = j not in inserts and implicit_loop_compare_nop(
                    text, start, branch, lines, j)
                inserts[j] = inserts.get(j, 0) + MIN_SPAN - span + int(replace_hazard)
                loops += 1
        i = end + 1

    out, reorder = [], True
    for j, line in enumerate(lines):
        if j in inserts:
            nops = ["\tnop\n"] * inserts[j]
            if reorder:
                # Make the assembler keep them as written; the branch itself
                # stays in reorder mode.
                nops = ["\t.set\tnoreorder\n"] + nops + ["\t.set\treorder\n"]
            # Inside the compiler's own .set noreorder block (the usual case:
            # the branch and its delay slot are spelled out) plain nops keep
            # the branch and its delay slot exactly as they were.
            out += nops
        stripped = line.strip()
        if stripped == ".set\tnoreorder" or stripped == ".set noreorder":
            reorder = False
        elif stripped == ".set\treorder" or stripped == ".set reorder":
            reorder = True
        if j in as_words:
            word = branch_as_word(line) if not reorder else None
            if word is None:
                sys.exit(f"ps2eeas_nops: {src_path}:{j + 1}: GNU as pads this branch more "
                         f"than ps2eeas did, and it cannot be written as a .word here: {stripped}")
            line = word
        if j in unpad_moves:
            out += ["\t.set\tnoreorder\n", line, "\t.set\treorder\n"]
            continue
        out.append(line)
    open(dst_path, "w").writelines(out)
    print(f"ps2eeas_nops: padded {loops} short loop(s), {fps} FP compare(s), "
          f"{moves} mtc1 use(s), placed {fp_labels} shared FP label(s), "
          f"unpadded {len(as_words)} branch(es), {len(unpad_moves)} mfc1 use(s) "
          f"{src_path} -> {dst_path}")


if __name__ == "__main__":
    main()
