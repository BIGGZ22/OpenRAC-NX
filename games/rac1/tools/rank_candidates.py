#!/usr/bin/env python3
"""
Ranks every remaining INCLUDE_ASM stub by how likely it is to match,
using the blocker signatures this project has actually confirmed.

Why this exists: the single most effective lever found so far is
CANDIDATE SELECTION, not source-shape iteration. Filtering the $gp pool
by one signature cut 199 stubs to 10 realistic candidates, and 4 of the
5 attempted from that list matched (2 first try). Grinding on a function
whose blocker is a toolchain question is pure waste; this tool exists to
stop that happening.

Verdicts:
  blocked   a confirmed hard blocker -- do not attempt without a new idea
  risky     a signature that has repeatedly produced same-size
            near-misses; attempt only when the candidate pool is dry
  candidate nothing known blocks it

Deliberately NOT treated as blockers, because both were over-broad
skip categories that cost this project real matches:
  * $gp-relative addressing -- unblocked by the -G2 switch; 9 matched.
  * movz/movn -- 6 reclaimed; most are arithmetic idioms (notably signed
    division by a power of two), not genuine conditional moves. Genuine
    ones are downgraded to `risky`, never `blocked`.

Usage:
  python tools/rank_candidates.py            # both segments
  python tools/rank_candidates.py text       # one segment
  python tools/rank_candidates.py text 40    # + show top N candidates
  python tools/rank_candidates.py --check func_00218928 func_001F84AC
  grep -rl 'dsll32' asm/nonmatchings | ... | python tools/rank_candidates.py --check -

--check also takes overlay names (func_LNN_XXXXXXXX, docs/OVERLAYS.md): it
screens asm/overlays/<name>.s through the same classify() rules. The bulk
ranking above (no --check) only covers the executable's two segments;
tools/wave.py's overlay pool calls classify() itself over the whole
catalogue instead.

SCREEN YOUR FAMILY-GREPS THROUGH --check.

Family-grep (find one match, then grep asm/nonmatchings for siblings
sharing a global or call) has repeatedly outperformed this ranking for
FINDING candidates -- but it reads the .s files directly and so bypasses
every blocker rule below. That is a real hole in the method, not in the
tool: one round burned two attempts on functions marked
`/* Handwritten function */`, both of which this tool had already
classified `blocked`, and then misreported the tool as lacking the check.
It has had that check since its first commit; 213 stubs carry it.

So: grep to FIND, then `--check` to SCREEN, then attempt. One pipe is
cheaper than one wasted decode.
"""
import re

SPILL_ARG_RE = re.compile(r"s[dwq]\s+\$([5-9]|1[01])\s*,\s*(0x[0-9A-Fa-f]+|[0-9]+)\(\$29\)")
import sys
from pathlib import Path

STUB = re.compile(r"INCLUDE_ASM\([^)]*\b(func_[0-9A-Fa-f]{8})\)")
sys.path.insert(0, str(Path(__file__).resolve().parent))
from libgcc_units import SEGMENT_SOURCES as SEGMENTS, ee29_sources, object_of

EE29 = ee29_sources()

# Overlay functions (docs/OVERLAYS.md): func_LNN_XXXXXXXX, whose asm lives in
# the flat asm/overlays/ directory rather than under asm/nonmatchings/<seg>.
# They are never ee29 (that only applies to Sony SDK objects in core_text),
# so classify() below only needs their address, not a segment lookup.
OVERLAY_NAME = re.compile(r"^func_L\d{2}_([0-9A-Fa-f]{8})$")
OVERLAY_ASM = Path("asm/overlays")


def vram_of(name: str) -> int:
    """The function's address, from either naming form (docs/OVERLAYS.md,
    "Names")."""
    m = OVERLAY_NAME.match(name)
    return int(m.group(1), 16) if m else int(name.split("_")[1], 16)


def segment_text(seg: str) -> str:
    """All game source for a segment (core_text is split across files)."""
    return "\n".join(Path(p).read_text(errors="replace") for p in SEGMENTS[seg])
# Matches both func_XXXXXXXX and the overlay form func_LNN_XXXXXXXX.
FUNCNAME = re.compile(r"\b(func_(?:L\d{2}_)?[0-9A-Fa-f]{8})\b")


def already_attempted() -> set[str]:
    """
    Functions named anywhere in docs/ or notes/. A stub discussed in
    prose has almost always been attempted and reverted with its reason
    recorded, so it should not resurface at the top of a "fresh
    candidates" list. Re-attempting one is fine, but that should be a
    deliberate choice backed by a new idea, not an accident of ranking.
    """
    seen: set[str] = set()
    for d in (Path("docs"), Path("notes")):
        if d.is_dir():
            for f in d.glob("*.md"):
                seen |= set(FUNCNAME.findall(f.read_text(errors="replace")))

    # Also: a stub in src/ with a block comment immediately above it is a
    # documented revert -- someone decoded it, failed, and wrote down why.
    # Those live in source comments rather than in docs/notes, so scanning
    # only the markdown missed them and handed them straight back as top
    # candidates. (Caught when func_0012AAA8, reverted minutes earlier for
    # a delay-slot difference, reappeared at rank 25.)
    # rglob: sources live in subdirectories (src/core/, src/game/, ...) since
    # the split into retail objects -- a flat glob silently found nothing.
    # The comment must end on its own line: a trailing name annotation on the
    # PREVIOUS stub (`INCLUDE_ASM(...); /* Name(int) */`) is not a revert note.
    for f in Path("src").rglob("*.c"):
        src = f.read_text(errors="replace")
        seen |= set(re.findall(
            r"(?m)^(?![^\n]*INCLUDE_ASM)[^\n]*\*/\s*INCLUDE_ASM\([^)]*\b(func_(?:L\d{2}_)?[0-9A-Fa-f]{8})\)", src))
    return seen


# gp base from retail's own .reginfo (Elf32_RegInfo.ri_gp_value)
GP_BASE = 0x00166D00
SDA_LO, SDA_HI = GP_BASE - 0x8000, GP_BASE + 0x8000
VU0_LO, VU0_HI = 0x1F9B20, 0x1FB598


def label_positions(body):
    """Maps each .L label to the index of the instruction it precedes."""
    pos, n = {}, 0
    for line in body.splitlines():
        m = re.match(r"^(\.L[0-9A-Fa-f]+):", line.strip())
        if m:
            pos[m.group(1)] = n
        elif "/*" in line and "*/" in line and line.split("*/", 1)[1].strip():
            n += 1
    return pos


def instructions(body: str) -> list[str]:
    out = []
    for line in body.splitlines():
        if "/*" in line and "*/" in line:
            after = line.split("*/", 1)[1].strip()
            if after:
                out.append(after)
    return out


# A quadword move of something other than a saved register ($sp-relative
# saves and restores are ordinary). retail's `lq $2,0(a)` then
# `sq $2,0(b)` is qcopy() in include/common.h; any other register, offset
# or a store in a delay slot is a plain 128-bit copy through
# `typedef int u128 __attribute__((mode(TI)))` (checked 2026-10-01: SN gcc
# emits `lq $2,16($5)` / `sq $2,48($4)` for it). A zero store at offset 0
# (`sq $0,0(a)`) is qzero() in include/common.h; at another offset it has
# no C form: the zero is materialised with `por` first.
BARE_QUAD = re.compile(r"\b(sq|lq)\s+\$(?!29\b|1[6-9]\b|2[0-3]\b|3[01]\b)")
ZERO_QUAD = re.compile(r"\bsq\s+\$0,")
QZERO_SQ = re.compile(r"^sq\s+\$0,\s*0x0\(\$\d+\)$")
QCOPY_LQ = re.compile(r"^lq\s+\$2,\s*0x0\(\$\d+\)$")
QCOPY_SQ = re.compile(r"^sq\s+\$2,\s*0x0\(\$\d+\)$")


def only_qcopies(ins: list[str]) -> bool:
    """Every bare quadword move is one of qcopy()'s pairs."""
    for k, s in enumerate(ins):
        if not BARE_QUAD.search(s):
            continue
        if QCOPY_LQ.match(s) and k + 1 < len(ins) and QCOPY_SQ.match(ins[k + 1]):
            continue
        if QCOPY_SQ.match(s) and k and QCOPY_LQ.match(ins[k - 1]):
            continue
        return False
    return True


def classify(name: str, body: str, seg: str, size: int) -> tuple[str, str, str]:
    """Returns (verdict, category, detail).

    NAME may be an overlay function (func_LNN_XXXXXXXX); pass seg="text"
    for those (they are never ee29, and never core_text objects)."""
    vram = vram_of(name)
    ins = instructions(body)
    text = "\n".join(ins)
    # Objects built with Sony's 2.9-ee make two of the risky signatures
    # below by themselves: a void function that ends in a call becomes a
    # tail jump, and a short loop is padded with its branch slot left
    # empty (func_001232A8).
    ee29 = seg == "core_text" and object_of(seg, vram)[1] in EE29

    if not ins:
        return "blocked", "empty", ""

    # A 4-byte "function" is almost never one. Most are the single word
    # retail's linker left when it dead-stripped an unreferenced function
    # of size 4 mod 8 (its last delay slot; see tools/strip_dead.py), the
    # rest linker fill between objects. The one true bare `jr $31`
    # (func_0011AE1C) has the next function's first word in its delay slot,
    # which C cannot emit either: `void f(void){}` is `jr $ra; nop`.
    if size and size <= 4:
        if re.match(r"jr\s+\$31\b", ins[0]):
            return "blocked", "bare jr (4 bytes)", "C cannot emit under 8 bytes"
        if ins[0].startswith("pref") and "0x0D, -0x3233" in ins[0]:
            return "blocked", "linker fill", "0xCDCDCDCD between objects"
        return "blocked", "dead-strip remnant", "delay slot of a stripped function"

    # core_text's sd/ld saves are Sony SDK code built with 2.9-ee (the `ee29`
    # objects), which emits them natively; the game compiler's core objects
    # save with sq like retail does there. No spill rewriting
    # (docs/BUILD_FIDELITY.md).
    if "Handwritten function" in body:
        return "blocked", "handwritten asm", "spimdisasm marker"
    # A bare tail jump (`j func_X`). Sony's 2.9-ee emits one for a void
    # function that ends in a call, so in its `ee29` objects it is ordinary
    # C. SN's 2.95.3 has no sibling-call optimisation (no flag turns one on),
    # and the build no longer rewrites its call-and-return into a jump
    # (docs/BUILD_FIDELITY.md, "Removed"), so outside those objects a tail
    # jump means another compiler or handwritten assembly: a wall.
    tail = bool(re.search(r"(?m)^j\s+func_[0-9A-Fa-f]{8}", text))
    if tail and not ee29:
        return "blocked", "tail call", "SN 2.95.3 emits no tail jumps"
    # Must come AFTER the tail-call test: a tail-called function ends in
    # `j`, not `jr $31`, so this rule would otherwise claim every tail call
    # is a fragment. Same verdict, but the category is what tells a future
    # reader whether a real function is there -- a fragment is not
    # independently callable, a tail call is a complete function blocked
    # only by a missing compiler optimisation.
    # These are, as far as checked, runs of dead-strip remnants: one
    # surviving delay-slot word per stripped function, nop-padded to 8
    # (tools/strip_dead.py). The stripped bodies are unknown, so C cannot
    # recreate them.
    if not tail and not re.search(r"\bjr\s+\$31\b", text):
        return "blocked", "fallthrough fragment", "no jr $31; dead-strip remnants"

    if len(ins) == 1 and "0xCDCDCDCD" in body:
        return "blocked", "padding", ""
    # A real function whose extent *begins* with 4 bytes of 0xCDCDCDCD
    # inter-function padding (splat folded the padding into its extent).
    # The body is ordinary code, but C cannot emit that leading word, so
    # it can never match. func_0011DD64 was a top candidate until this.
    if ins and ins[0].startswith("pref") and "CDCDCDCD" in body:
        return "blocked", "padding-prefixed", "leading 0xCDCDCDCD not from C"
    if VU0_LO <= vram <= VU0_HI:
        return "blocked", "VU0 cluster", "0x1F9B20-0x1FB598"
    if re.search(r"\b(v[a-z]+\.[xyzw]+|vcallms|qmfc2|qmtc2|cfc2|ctc2|pxor|pcpyud|pextlw|pnor)\b", text):
        return "blocked", "SIMD/COP2", ""
    if re.search(r"\b(adda|madd|msub)\.s\b", text):
        return "blocked", "FPU accumulate", "adda.s/madd.s not plain-C"
    # Varargs *definition*: the prologue spills the argument registers
    # $5..$11 into a CONTIGUOUS save area so va_arg can walk it, i.e.
    # consecutive registers at stack offsets rising by 8. Callers are fine.
    #
    # Both halves of that test matter. Looking only at the first 9
    # instructions missed func_0011A6C8, whose save area starts around
    # instruction 8. But merely counting distinct $5-$11 spills anywhere
    # over-blocks badly -- it flagged 92 functions including a
    # 1139-instruction one that just spills temps around calls, which
    # would have hidden ~70 real candidates. Requiring the regular
    # ascending run keeps it to actual save areas.
    spills = [(int(r), int(off, 0)) for r, off in
              SPILL_ARG_RE.findall(text)]
    run = 1
    best = 1
    for a, b in zip(spills, spills[1:]):
        if b[0] == a[0] + 1 and b[1] == a[1] + 8:
            run += 1
            best = max(best, run)
        else:
            run = 1
    if len(ins) > 4 and best >= 4:
        return "blocked", "varargs definition", "needs stdarg.h"
    qcopy = u128 = False
    if any(ZERO_QUAD.search(s) and not QZERO_SQ.match(s) for s in ins):
        return "blocked", "bare quadword", "sq $0 at an offset: only a zero store at offset 0 is qzero()"
    if BARE_QUAD.search(text):
        qcopy = True
        u128 = not only_qcopies(ins)

    # --- the $at macro store form (retail builds an address in $1 and
    # stores through it) was blocked here for many rounds. UNBLOCKED:
    # declare the global MACRO_ADDR (include/common.h) and the assembler
    # expands the compiler's unsplit `sw $x,D` through $at. The same
    # attribute gives the one-register load form. Hinted below.
    at_store = bool(re.search(r"\b[sl][bhwdq]c?1?\s+\$\w+,\s*[^,]*\(\$1\)", text)
                    and re.search(r"lui\s+\$1\b", text))

    # --- one variable reached BOTH via $gp and via lui/%lo in the same
    # function. This was blocked for many rounds as an unexpressible
    # per-TU declaration difference. It is NOT: retail's toolchain wrote
    # a one-instruction macro access $gp-relative when it landed in a
    # branch delay slot (505 of 524 such accesses image-wide), and
    # tools/check_macro_slots.py reproduces that from one MACRO_ADDR
    # declaration. Hinted, not blocked.
    gp_syms, hi_syms = set(), set()
    for i in ins:
        if "$28" in i:
            m = re.search(r"%gprel\(([^)]+)\)|(-?0x[0-9A-Fa-f]+)\(\$28\)", i)
            if m:
                if m.group(1):
                    gp_syms.add(m.group(1))
                else:
                    gp_syms.add(hex((GP_BASE + int(m.group(2), 16)) & 0xFFFFFFFF))
        m = re.search(r"%(?:hi|lo)\((D_[0-9A-Fa-f]+)\)", i)
        if m:
            hi_syms.add("0x" + m.group(1).split("_")[1].lstrip("0").lower())
    gp_hi = bool(gp_syms & hi_syms)

    # --- an epilogue fragment: a real function opens by RESERVING stack
    # (addiu $sp,$sp,-N). Opening with a positive adjustment means this
    # is the tail of some other function that splat gave its own label,
    # often several merged together. It ends in a jr, so the "no jr $31"
    # check above does not catch it.
    if re.match(r"addiu\s+\$29,\s*\$29,\s*0x", ins[0]):
        return "blocked", "epilogue fragment", "opens by releasing stack"

    # --- assembler load-delay: a tiny leaf loads an FP global and then
    # uses it, with retail carrying a load-delay nop the compiler won't
    # emit. The nop sits BETWEEN the load and the use, so look past it
    # rather than only at the next instruction.
    if len(ins) <= 8 and re.search(r"\bl[wd]c1\b", text) and "jal" not in text:
        for idx, a in enumerate(ins):
            m = re.match(r"l[wd]c1\s+\$(f\d+)", a)
            if m and any(m.group(1) in b for b in ins[idx + 1: idx + 4]):
                if "nop" in ins[idx + 1: idx + 3]:
                    return "blocked", "load-delay nop", "MIPS I interlock, not reachable from C"

    # --- alternate entry point: splat emits `alabel` for a second,
    # separately-callable label inside one function body. C has no way to
    # express two entry points into one function, so these can never
    # match no matter how the body is written. 72 .s files carry one.
    if "alabel" in body:   # raw body: `alabel` is a bare directive, not an instruction line
        return "blocked", "alternate entry point", "alabel: two entries, not expressible in C"

    # (GPR->FPU move delay -- `mtc1 $x, $fN` / `nop` / <use of $fN> -- is
    # no longer a blocker: the nop is ps2eeas's, and tools/ps2eeas_nops.py
    # adds it to compiled game code.)

    # --- R5900 short-loop erratum: nop padding before a tight backward
    # branch. `span` is the distance back to the branch TARGET; it used to
    # be the index from the start of the function, which let any tight loop
    # past instruction 7 escape.
    #
    # It is no longer a blocker anywhere. Retail's text was assembled by SN's
    # ps2eeas, which pads every loop shorter than six instructions, and
    # tools/ps2eeas_nops.py reproduces that. core_text was assembled by the
    # compiler driver's own as.exe (ee/bin/as.exe), which pads short loops
    # that contain no call -- the same assembler this build runs, so those
    # come out padded by themselves. (The standalone bin/ee-as.exe, with the
    # same version string, pads nothing; probing that one is what made this
    # look unexplained.)
    labels = label_positions(body)

    # ---- risky signatures (near-miss generators, not hard blockers) ----

    # Short-loop erratum, second form: retail leaves the loop branch's
    # DELAY SLOT unfilled where this compiler fills it, so we come out
    # 4 bytes short. Same erratum as the two-nop form above, which is
    # blocked; this one is only marked risky because there is a single
    # confirmation so far (func_0011D370) -- blanket-blocking on thin
    # evidence has cost this project real matches twice.
    for idx, i in enumerate(ins[:-1] if not ee29 else []):
        m = re.match(r"b(ne|eq|nez|eqz|gez|ltz|gtz|lez)l?\s.*?(\.L[0-9A-Fa-f]+)\s*$", i)
        if m and ins[idx + 1] == "nop":
            t2 = labels.get(m.group(2))
            if t2 is not None and t2 <= idx and idx - t2 <= 7:
                return "risky", "loop delay slot nop", "erratum: unfilled delay slot"

    # Destination-reuse load `lw $x, %lo(sym)($x)`. Long treated as an
    # allocator near-miss generator; it is the assembler's expansion of an
    # unsplit `lw $x,sym` macro, which MACRO_ADDR (include/common.h)
    # reproduces. No longer risky: hinted below instead.
    reuse_load = bool(re.search(r"l[wbhd]u?\s+\$(\w+),\s*%lo\([^)]*\)\(\$\1\)", text))

    # (A rule here once marked core_text functions that save $ra with sq as
    # risky, on the belief that core was built with v1.36. They are the
    # objects from 0x12DB18 on, 989snd and friends, built with SN 2.95.3,
    # which writes those sq saves itself.)

    # Genuine conditional move (not the div-by-power-of-two idiom, whose
    # movn is followed closely by an sra).
    if re.search(r"\bmov[zn]\b", text):
        idiom = False
        for idx, i in enumerate(ins):
            if re.match(r"mov[zn]\b", i):
                if any(re.match(r"sra\b", j) for j in ins[idx + 1: idx + 4]):
                    idiom = True
        if not idiom:
            return "risky", "genuine conditional move", "movz/movn, no sra idiom"

    detail = "2.9-ee" if ee29 else ""
    if tail:
        detail += " (void tail call)"
    if re.search(r"\$28\b", text):
        detail = (detail + "; " if detail else "") + "$gp (unblocked at -G2)"
    if at_store or reuse_load:
        detail = (detail + "; " if detail else "") + "MACRO_ADDR"
    if gp_hi:
        if "MACRO_ADDR" in detail:
            detail += " ($gp in slot)"
        else:
            detail = (detail + "; " if detail else "") + "MACRO_ADDR ($gp in slot)"
    if qcopy:
        detail = (detail + "; " if detail else "") + (
            "128-bit copies: typedef int u128 __attribute__((mode(TI)))" if u128 else "qcopy() for 16-byte copies")
    return "candidate", "candidate", detail


def check_names(names: list[str]) -> None:
    """
    Screens an explicit list of function names through the same rules.

    Exists so family-grep results can be filtered before any decoding
    effort is spent -- grepping .s files directly is the fastest way to
    FIND siblings, but it sees none of the blockers above.
    """
    attempted = already_attempted()
    width = max((len(n) for n in names), default=20)
    for name in names:
        if OVERLAY_NAME.match(name):
            p = OVERLAY_ASM / f"{name}.s"
            if not p.exists():
                print(f"  {name:<{width}}  ?         not found in {OVERLAY_ASM}")
                continue
            body = p.read_text(errors="replace")
            m = re.search(r"nonmatching\s+\S+,\s*(0x[0-9A-Fa-f]+)", body)
            size = int(m.group(1), 16) if m else 0
            verdict, cat, detail = classify(name, body, "text", size)
            if verdict == "candidate" and name in attempted:
                verdict, cat = "risky", "already attempted"
                detail = "discussed in docs/notes -- see reason there"
            # Whether it's still a stub isn't checked here (that needs a
            # scan of src/overlays/, which tools/wave.py's overlay pool
            # already does); this only screens the assembly's shape.
            print(f"  {name:<{width}}  {verdict:<9} {size:#7x}  {cat}"
                  f"{' -- ' + detail if detail else ''}")
            continue
        hit = None
        for seg in SEGMENTS:
            p = Path(f"asm/nonmatchings/{seg}/{name}.s")
            if p.exists():
                hit = (seg, p)
                break
        if hit is None:
            print(f"  {name:<{width}}  ?         not found in asm/nonmatchings")
            continue
        seg, p = hit
        body = p.read_text(errors="replace")
        m = re.search(r"nonmatching\s+\S+,\s*(0x[0-9A-Fa-f]+)", body)
        size = int(m.group(1), 16) if m else 0
        verdict, cat, detail = classify(name, body, seg, size)
        if verdict == "candidate" and name in attempted:
            verdict, cat = "risky", "already attempted"
            detail = "discussed in docs/notes -- see reason there"
        stub = re.search(
            r"INCLUDE_ASM\([^)]*\b" + name + r"\)",
            segment_text(seg),
        )
        done = "" if stub else "  [already decompiled]"
        print(f"  {name:<{width}}  {verdict:<9} {size:#7x}  {cat}"
              f"{' -- ' + detail if detail else ''}{done}")


def main() -> None:
    args = [a for a in sys.argv[1:]]

    if "--check" in args:
        rest = [a for a in args if a != "--check"]
        if rest == ["-"] or not rest:
            raw = sys.stdin.read().split()
        else:
            raw = rest
        names = []
        for tok in raw:
            names += FUNCNAME.findall(tok)  # tolerates paths like .../func_X.s
        seen_once: list[str] = []
        for n in names:
            if n not in seen_once:
                seen_once.append(n)
        if not seen_once:
            print("no func_XXXXXXXX names found on the command line or stdin")
            return
        print(f"--- screening {len(seen_once)} function(s) ---")
        check_names(seen_once)
        return

    segs = [a for a in args if a in SEGMENTS] or list(SEGMENTS)
    topn = next((int(a) for a in args if a.isdigit()), 25)

    attempted = already_attempted()
    rows = []
    for seg in segs:
        for name in sorted(set(STUB.findall(segment_text(seg)))):
            p = Path(f"asm/nonmatchings/{seg}/{name}.s")
            if not p.exists():
                continue
            body = p.read_text(errors="replace")
            m = re.search(r"nonmatching\s+\S+,\s*(0x[0-9A-Fa-f]+)", body)
            size = int(m.group(1), 16) if m else 0
            verdict, cat, detail = classify(name, body, seg, size)
            if verdict == "candidate" and name in attempted:
                verdict, cat = "risky", "already attempted"
                detail = "discussed in docs/notes -- see reason there"
            rows.append((verdict, cat, name, seg, size, detail))

    counts: dict[tuple[str, str], int] = {}
    for verdict, cat, *_ in rows:
        counts[(verdict, cat)] = counts.get((verdict, cat), 0) + 1

    print(f"{len(rows)} remaining stubs in {', '.join(segs)}\n")
    for verdict in ("candidate", "risky", "blocked"):
        n = sum(v for (vd, _), v in counts.items() if vd == verdict)
        print(f"{n:5d}  {verdict}")
        for (vd, cat), v in sorted(counts.items(), key=lambda kv: -kv[1]):
            if vd == verdict and cat != verdict:
                print(f"        {v:4d}  {cat}")
    print()

    cands = sorted([r for r in rows if r[0] == "candidate"], key=lambda r: r[4])
    print(f"--- {len(cands)} candidates, smallest first (showing {min(topn, len(cands))}) ---")
    for _, _, name, seg, size, detail in cands[:topn]:
        print(f"  {name}  {seg:9s} {size:#7x}  {detail}")


if __name__ == "__main__":
    main()
