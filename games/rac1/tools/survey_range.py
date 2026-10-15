#!/usr/bin/env python3
"""
Classifies remaining INCLUDE_ASM stubs in an address range by which
known skip-category (if any) they fall into, so a range can be assessed
without hand-reading every .s file.

IMPORTANT: `sq`/`lq` callee-saved spills are NOT a skip category for the
`text` segment -- retail's text was built by SN BUILD v1.14, which emits
sq/lq, and that is what we now compile text.c with. This script reports
spill style as information only, never as "blocked". An earlier survey of
range A got this wrong and wrote off 95 functions on that basis.

Usage: python tools/survey_range.py <src_file> <segment> <start_hex> <end_hex>
  e.g. python tools/survey_range.py src/game/actuator.c text 0x1E9080 0x1F9B00
"""
import re
import sys
from pathlib import Path

STUB = re.compile(r"INCLUDE_ASM\([^)]*\b(func_[0-9A-Fa-f]{8})\)")


def classify(body: str) -> tuple[str, str]:
    """Returns (category, detail). 'candidate' means nothing blocks it."""
    lines = [l for l in body.splitlines() if "/*" in l and "*/" in l]
    instrs = []
    for l in lines:
        after = l.split("*/", 1)[1].strip()
        if after:
            instrs.append(after)
    text = "\n".join(instrs)

    if not instrs:
        return "empty/no-instructions", ""
    # Handwritten marker comes from spimdisasm itself.
    if "Handwritten function" in body:
        return "handwritten asm", ""
    # No return at all -> not independently callable.
    if not re.search(r"\bjr\s+\$31\b", text):
        return "fallthrough fragment", "no jr $31"
    # Pure padding.
    if len(instrs) == 1 and "0xCDCDCDCD" in body:
        return "padding", ""
    # VU0 / COP2 vector math.
    if re.search(r"\b(v[a-z]+\.[xyzw]+|vcallms|qmfc2|qmtc2|pxor|pcpyud|pextlw|pnor)\b", text):
        return "SIMD/COP2", ""
    # $gp-relative addressing (loads as well as stores). This was a blocker
    # under the old -G0 build; since the switch to -G2 it is reachable, so
    # treat this as a category to report, not as a verdict.
    if re.search(r"\$28\b", text):
        return "$gp-relative", ""
    # Conditional moves: heuristics differ in both directions.
    if re.search(r"\bmov[zn]\b", text):
        return "movz/movn", ""
    # Bare quadword store/load with no plain-C representation.
    if re.search(r"\b(sq|lq)\s+\$(?!29\b)", text) and not re.search(r"\b(sq|lq)\s+\$(1[6-9]|2[0-3]|31)\b", text):
        return "bare quadword", ""
    # Varargs prologue: spills a1-t3 to the stack immediately.
    if len(instrs) > 4 and sum(1 for i in instrs[:8] if re.match(r"s[dwq]\s+\$([5-7]|8|9|1[0-1])\b", i)) >= 3:
        return "varargs prologue", ""

    # Informational only, never blocking:
    spill = "sq/lq spill" if re.search(r"\b(sq|lq)\s+\$(1[6-9]|2[0-3]|31)\b", text) else ""
    if re.search(r"\bs[dq]\s+\$(1[6-9]|2[0-3])\b", text):
        spill += " (uses s-regs)"
    return "candidate", spill.strip()


def main() -> None:
    src, segment, start_s, end_s = sys.argv[1:5]
    start, end = int(start_s, 16), int(end_s, 16)
    stubs = STUB.findall(Path(src).read_text(errors="replace"))

    buckets: dict[str, list[tuple[str, int, str]]] = {}
    for name in sorted(set(stubs)):
        vram = int(name.split("_")[1], 16)
        if not (start <= vram <= end):
            continue
        p = Path(f"asm/nonmatchings/{segment}/{name}.s")
        if not p.exists():
            continue
        body = p.read_text(errors="replace")
        m = re.search(r"nonmatching\s+\S+,\s*(0x[0-9A-Fa-f]+)", body)
        size = int(m.group(1), 16) if m else 0
        cat, detail = classify(body)
        buckets.setdefault(cat, []).append((name, size, detail))

    total = sum(len(v) for v in buckets.values())
    print(f"{src} {segment} {start_s}..{end_s}: {total} remaining stubs\n")
    for cat in sorted(buckets, key=lambda c: -len(buckets[c])):
        print(f"{len(buckets[cat]):4d}  {cat}")
    print()
    cands = sorted(buckets.get("candidate", []), key=lambda t: t[1])
    print(f"--- {len(cands)} candidates, smallest first ---")
    for name, size, detail in cands:
        print(f"  {name}  {size:#6x}  {detail}")


if __name__ == "__main__":
    main()
