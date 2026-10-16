#!/usr/bin/env python3
"""
Audits EVERY function that has real C in src/*.c (i.e. is no longer an
INCLUDE_ASM stub) against retail, on BOTH size and bytes.

This exists because per-function spot checks let two distinct classes of
false positive through:
  * a stale object after a failed compile (the INCLUDE_ASM stub still
    holds retail's own bytes, so it reads 0/N), and
  * a function correct in its first N bytes but LONGER than retail (the
    surplus fell outside the compared window, so it also read 0/N).
Both were caught only by accident. A whole-project sweep with explicit
size comparison is the cheap way to keep them caught.

Run after a full rebuild+relink. Reports one line per function and a
summary, and exits nonzero if anything is not a clean match.

Usage: python tools/sweep_matches.py
"""
import re
import subprocess
import sys
from pathlib import Path

from elftools.elf.elffile import ELFFile

sys.path.insert(0, str(Path(__file__).resolve().parent))
from libgcc_units import FUNCTIONS as LIBGCC_FUNCTIONS, SEGMENT_SOURCES

BASEROM = "baserom/SCES_509.16"
LINKED_ELF = "build-sn/rac1.elf"

# (?!extern\b) so forward declarations aren't mistaken for definitions --
# they used to be, which made a declared-but-not-defined function report
# "could not check" forever.
# `.*?` is LAZY on purpose. Greedy, a one-line definition such as
# `void func_0011BC70(void) { func_00118C90(D); }` captured the CALLEE's
# name instead of the definition's, and because that callee was still a
# stub the function silently dropped out of the audit -- it read as "not
# decompiled" rather than as a failure. Found by writing exactly that
# one-liner. Take the FIRST name on the line: that is the one defined.
FUNC_DEF = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b(func_[0-9A-Fa-f]{8})(?:_r)?\s*\(", re.M)
STUB = re.compile(r"INCLUDE_ASM\([^)]*\b(func_[0-9A-Fa-f]{8})\)")
NONMATCHING = re.compile(r"nonmatching\s+(func_[0-9A-Fa-f]{8}),\s*(0x[0-9A-Fa-f]+)")


def trailing_padding(name: str) -> int:
    """
    Bytes of padding a function's .s file carries AFTER its `endlabel`.

    729 of the .s files have some, because retail aligns the next function
    to 16 bytes while splat only emits `.align 3`. While a function is
    still an INCLUDE_ASM stub its .s supplies that padding; the moment it
    is decompiled to C, the padding silently disappears and every later
    function in the object shifts, producing spurious 1-byte `jal` diffs
    far away from the actual cause. One instance of this shifted 179
    functions at once -- 179 apparent problems, one real cause.

    The fix in the C is an explicit alignment directive, e.g.
        __asm__(".align 4");
    after the function. This reports which decompiled functions need that
    so the trap is visible instead of being rediscovered.
    """
    for seg in ("core_text", "text"):
        p = Path(f"asm/nonmatchings/{seg}/{name}.s")
        if not p.exists():
            continue
        t = p.read_text(errors="replace")
        i = t.rfind("endlabel")
        if i == -1:
            return 0
        rest = t[i:].split("\n", 1)
        body = rest[1] if len(rest) > 1 else ""
        # each remaining instruction-comment line is one 4-byte word
        return 4 * len(re.findall(r"/\*.*?\*/", body))
    return 0


def body_size_by_gap(addr: int, sec_data: bytes, sec_base: int,
                     next_addr: int | None) -> int | None:
    """
    Our function's body size measured WITHOUT trusting the ELF symbol:
    the span to the next symbol in the same section, with trailing
    all-zero (padding `nop`) words stripped.

    This exists as a cross-check on ``st_size``. It is the WEAKER of the
    two measures, because a function may legitimately END in a real
    `nop` (145+ byte-exact functions contain standalone nops), which this
    would strip. So it is never used in place of ``st_size``; it is only
    used to shout when the two disagree in the direction that cannot be
    explained away -- real instructions living PAST the declared end.
    """
    if next_addr is None or next_addr <= addr:
        return None
    off = addr - sec_base
    end = next_addr - sec_base
    if off < 0 or end > len(sec_data):
        return None
    span = sec_data[off:end]
    while len(span) >= 4 and span[-4:] == bytes(4):
        span = span[:-4]
    return len(span)


def retail_size(name: str) -> int | None:
    for seg in ("core_text", "text"):
        p = Path(f"asm/nonmatchings/{seg}/{name}.s")
        if p.exists():
            m = NONMATCHING.search(p.read_text(errors="replace"))
            if m:
                return int(m.group(2), 16)
    return None


def main() -> None:
    decompiled: list[str] = []
    for srcs in SEGMENT_SOURCES.values():
        for src in srcs:
            text = Path(src).read_text(errors="replace")
            stubs = set(STUB.findall(text))
            for name in FUNC_DEF.findall(text):
                if name not in stubs:
                    decompiled.append(name)
    # libgcc modules are built from GCC's own source; their func_ names
    # exist as linker aliases (rac1.ld.sh), so they audit like any other.
    decompiled += LIBGCC_FUNCTIONS
    decompiled = sorted(set(decompiled))

    with open(BASEROM, "rb") as f:
        belf = ELFFile(f)
        seg = next(s for s in belf.iter_segments() if s["p_type"] == "PT_LOAD")
        delta = seg["p_vaddr"] - seg["p_offset"]
        baserom_bytes = Path(BASEROM).read_bytes()

    with open(LINKED_ELF, "rb") as f:
        oelf = ELFFile(f)
        symtab = oelf.get_section_by_name(".symtab")
        syms = {s.name: s for s in symtab.iter_symbols()}
        sections = {i: oelf.get_section(i).data() for i in range(oelf.num_sections())}
        sec_addr = {i: oelf.get_section(i)["sh_addr"] for i in range(oelf.num_sections())}
        # address -> address of the next symbol that starts strictly
        # after it, used by the independent body-size measure.
        addrs = sorted({s["st_value"] for s in symtab.iter_symbols()
                        if s["st_info"]["type"] == "STT_FUNC"})
        # Linker-script aliases (libgcc's func_ names, rac1.ld.sh) carry
        # st_size 0; the symbol gcc emitted at the same address has the size.
        sized_at = {}
        for s in symtab.iter_symbols():
            if s["st_size"] and s["st_info"]["type"] == "STT_FUNC":
                sized_at.setdefault(s["st_value"], s["st_size"])
        next_addr = {a: b for a, b in zip(addrs, addrs[1:])}

    exact, size_bad, byte_bad, missing = [], [], [], []
    for name in decompiled:
        rsize = retail_size(name)
        sym = syms.get(name)
        if rsize is None or sym is None:
            missing.append(name)
            continue
        rvram = int(name.split("_")[1], 16)
        orig = baserom_bytes[rvram - delta: rvram - delta + rsize]
        idx = sym["st_shndx"]
        off = sym["st_value"] - sec_addr[idx]
        ours = sections[idx][off: off + rsize]
        osize = sym["st_size"] or sized_at.get(sym["st_value"], 0)

        # --- size-measure cross-check -------------------------------
        # `st_size` comes from gcc's .ent/.end pair, which brackets the
        # function body ONLY: the `.align 3` that pads up to the next
        # function is emitted AFTER `.end`, so its bytes are not counted.
        # Measured, not assumed: of the 394 compiled-C functions, a large
        # fraction report sizes that are 4 mod 8, which is impossible if
        # 8-byte alignment padding were being folded in; and restoring
        # the known-4-bytes-short spelling of func_0012D688 makes the
        # sweep print `retail=164 ours=160`, i.e. the real body length,
        # not a padded 164 that would have read as a false pass.
        # So `osize` IS the true body size. The block below exists so
        # that if that ever stops being true -- a rewriter dropping
        # .ent/.end, a different assembler -- the sweep says so loudly
        # instead of silently reverting to the old ambiguity.
        gsize = body_size_by_gap(sym["st_value"], sections[idx],
                                 sec_addr[idx], next_addr.get(sym["st_value"]))
        if not osize:
            print(f"!! NO SYMBOL SIZE for {name}: .ent/.end missing? "
                  f"falling back to padding-strip measure {gsize}")
            osize = gsize or 0
        elif gsize is not None and gsize > osize:
            print(f"!! SIZE MEASURES DISAGREE for {name}: st_size={osize} "
                  f"but {gsize} bytes of non-padding follow the symbol. "
                  f"The symbol size is UNDER-reporting -- do not trust "
                  f"this function's result until this is understood.")

        if osize and osize != rsize:
            size_bad.append((name, rsize, osize))
            print(f"SIZE  {name}: retail={rsize} ours={osize}")
            continue
        mism = sum(1 for i in range(min(len(orig), len(ours))) if orig[i] != ours[i])
        if mism:
            byte_bad.append((name, mism, rsize))
            print(f"BYTES {name}: {mism}/{rsize}")
        else:
            exact.append(name)

    # Decompiled functions whose .s carried post-endlabel padding need an
    # explicit alignment directive in the C, or everything after them
    # shifts. Report them so the trap stays visible.
    # Only >4 bytes is worth reporting. 4 bytes is the common case and is
    # normally harmless: the *next* function's .s starts with its own
    # `.align 3`, which re-establishes 8-byte alignment anyway. It only
    # actually bites when retail wanted 16-byte alignment, i.e. more
    # padding than `.align 3` can account for -- which is the 8-byte case
    # that shifted 179 functions. Verified: ~30 decompiled functions carry
    # exactly 4 bytes and are all exact with zero drift, so flagging those
    # would be pure false alarm.
    needs_align = [(n, trailing_padding(n)) for n in decompiled]
    needs_align = [(n, b) for n, b in needs_align if b > 4]

    print(f"\n=== {len(decompiled)} decompiled functions audited ===")
    print(f"  exact (size AND bytes): {len(exact)}")
    print(f"  size mismatch:          {len(size_bad)}   (always revert these -- see docs)")
    print(f"  byte mismatch:          {len(byte_bad)}")
    if missing:
        print(f"  could not check:        {len(missing)} {missing}")
    if needs_align:
        print(f"\n  DIAGNOSTIC ONLY -- NOT a to-do list. {len(needs_align)} decompiled")
        print(f"  function(s) had >4 bytes of post-endlabel padding in their .s.")
        print(f"  Consult this ONLY when the sweep above already shows unexplained")
        print(f"  drift, to help locate its cause. Do NOT add alignment directives")
        print(f"  to these pre-emptively: doing that broke 7 of 13 verified-exact")
        print(f"  functions in one round (0/N -> 1-2/N in address fields), because")
        print(f"  at most sizes the padding is already accounted for. And .align 4")
        print(f"  cannot even express some of these gaps (one starts 16-byte aligned")
        print(f"  and ends unaligned, needing explicit nops instead).")
        for n, b in needs_align:
            print(f"    {n}: {b} bytes of padding")
    sys.exit(0 if not (size_bad or byte_bad) else 1)


if __name__ == "__main__":
    main()
