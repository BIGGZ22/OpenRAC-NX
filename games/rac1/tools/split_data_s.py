#!/usr/bin/env python3
"""
Split a splat data .s file around labels that a compiled object now owns.

  python tools/split_data_s.py IN OUT_PREFIX LABEL [LABEL ...]

Writes OUT_PREFIX_1.s, OUT_PREFIX_2.s, ... : every `nonmatching`/`dlabel`
block of IN in order, with each named LABEL's block left out and the file
cut there. The file header (everything before the first block) is
repeated in each part.

Why: a compiled object can bring read-only data of its own, and the
retail blob has to be split so the linker can put the object's .rodata
back in exactly its hole (see rac1.ld.sh). libgcc's __divdi3 carries its
static __clz_tab (0x100 bytes), which retail has at D_00152B18 in the
middle of core_rdata; a game function's switch brings its jump table,
which retail has among the text objects' read-only data at the end of
the data segment.

Every piece after a cut lays its blocks out with `.org` at their retail
offsets from the piece's start, in place of the data's own `.align`
lines: those assume retail's absolute addresses, and a piece can start
less aligned than its contents (after a 20-byte jump table, say).
"""
import re
import sys

# A data block: `D_` for splat's data labels, `jtbl_` for jump tables.
LABEL = r"nonmatching\s+((?:D|jtbl)_[0-9A-F]{8})\b"


def first_address(body: list[str]) -> int | None:
    """VRAM of the first data line (spimdisasm's `/* ROM VRAM ... */`)."""
    for line in body:
        m = re.match(r"\s*/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\b", line)
        if m:
            return int(m.group(1), 16)
    return None


def table_rest(body: list[str]) -> list[str]:
    """What splat merged into a jtbl_ block after the table itself: the
    table is its leading run of words pointing into code (a label or a
    text address); anything after them belongs to the next piece."""
    lines = [l for l in body if first_address([l]) is not None]
    k = 0
    while k < len(lines):
        m = re.search(r"\.word\s+(\S+)", lines[k])
        if not m:
            break
        v = m.group(1)
        if not v.startswith((".L", "L")):
            try:
                if not 0x100000 <= int(v, 16) < 0x300000:
                    break
            except ValueError:
                break
        k += 1
    return lines[k:]


def main() -> None:
    src, prefix, cuts = sys.argv[1], sys.argv[2], set(sys.argv[3:])
    lines = open(src, newline="").read().splitlines(keepends=True)
    starts = [i for i, l in enumerate(lines) if re.match(LABEL, l)]
    header = lines[: starts[0]]
    blocks = []
    for k, i in enumerate(starts):
        j = starts[k + 1] if k + 1 < len(starts) else len(lines)
        name = re.match(LABEL, lines[i]).group(1)
        blocks.append((name, lines[i:j]))
    missing = cuts - {n for n, _ in blocks}
    if missing:
        sys.exit(f"labels not found in {src}: {sorted(missing)}")

    parts, cur = [], []
    for name, body in blocks:
        if name in cuts:
            parts.append(cur)
            cur = []
            # The object brings only the table; data splat merged into the
            # block after it (strings, say) stays, at its own address.
            if name.startswith("jtbl_") and table_rest(body):
                cur.append(table_rest(body))
        else:
            cur.append(body)
    parts.append(cur)
    for n, piece in enumerate(parts, 1):
        body = [l for b in piece for l in b]
        # A piece after a cut starts where retail's next item starts, and
        # the linker only puts it there if its section is aligned at least
        # as much as that address is (the gap after our table is always
        # smaller than that). So open it with the address's own alignment,
        # capped at 16. The first piece starts at the segment's own base.
        align = []
        start = first_address(body)
        if n > 1 and start is not None:
            bits = min((start & -start).bit_length() - 1, 4)
            align = [f".align {bits}\n"]
            # Inside the piece, place every block at its retail offset from
            # the piece's start. The data's own .align lines assume retail's
            # absolute addresses, and would raise the section's alignment
            # past what its start has (a piece can start at 4 mod 8).
            body = []
            for b in piece:
                addr = first_address(b)
                if addr is not None:
                    body.append(f".org 0x{addr - start:X}\n")
                for k, line in enumerate(b):
                    if not re.match(r"\s*\.align\b", line):
                        body.append(line)
                        continue
                    # An .align inside a block: go straight to the address
                    # of the next data line instead.
                    nxt = first_address(b[k + 1:])
                    if nxt is not None:
                        body.append(f".org 0x{nxt - start:X}\n")
        with open(f"{prefix}_{n}.s", "w", newline="") as f:
            f.writelines(header + align + body)
    print(f"split {src} into {len(parts)} part(s) around {sorted(cuts)}")


if __name__ == "__main__":
    main()
