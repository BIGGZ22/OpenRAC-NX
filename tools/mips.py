"""
R5900 code helpers shared by OpenRAC's tools: finding function boundaries in a
block of code, and fingerprints that say when two functions are the same code
at different addresses. Standard library only.

Lifted from games/rac1/tools/overlays.py (rac1-decomp, while it was MIT), where the
same rules pair 99.7% of the US build's code with the PAL build's. Nothing
here knows about one game.
"""
import hashlib
import struct
from pathlib import Path

# Opcodes whose 16-bit immediate is an offset from a base register (loads, stores, cache, lwc1...).
MEM = {0x1A, 0x1B, 0x1E, 0x1F, *range(0x20, 0x30), 0x31, 0x36, 0x37, 0x39, 0x3E, 0x3F}


def words(b: bytes) -> tuple:
    return struct.unpack(f"<{len(b) // 4}I", b[:len(b) // 4 * 4])


def trim(b: bytes) -> bytes:
    """B without the zero padding after it."""
    while b.endswith(b"\0\0\0\0"):
        b = b[:-4]
    return b


def mask(w: int) -> int:
    """One instruction with every link-dependent field cleared, and with them
    every constant and struct offset: jump targets, `lui` immediates, and the
    16-bit immediate of anything not relative to $sp."""
    op, rs = w >> 26, (w >> 21) & 31
    if op in (2, 3):                                           # j, jal: target
        return w & 0xFC000000
    if op == 0x0F or rs == 28 and op >= 8:                     # lui, $gp-relative
        return w & 0xFFFF0000
    if (op in MEM or op in (0x09, 0x0D, 0x19)) and rs != 29:   # %lo halves and other non-stack offsets
        return w & 0xFFFF0000
    return w


# Opcodes whose 16-bit immediate can be the %lo half of an address: MEM, addiu and daddiu.
LO16 = MEM | {0x09, 0x19}
# Opcodes that write rt: immediate arithmetic, lui, and the loads into a general register.
WRITES_RT = {*range(8, 16), 0x18, 0x19, 0x1A, 0x1B, 0x1E, *range(0x20, 0x28), 0x37}
CALLER_SAVED = (*range(1, 16), 24, 25, 31)


def references(code: bytes, base: int, gp: int | None = None) -> list[tuple[int, str, int]]:
    """(instruction index, kind, address) for every address CODE forms, in
    instruction order: `call` (a jal, or a j that leaves the function), `data`
    (a `lui` and the %lo half that completes it, given at the %lo instruction)
    and `gp` ($gp-relative, when GP is known).

    Two copies of one function give the same indexes and kinds, so the n-th
    address one forms is the n-th the other forms: that is how tools/port.py
    translates a function's symbols between versions.

    Which `lui` a %lo half completes is decided along the function's branches,
    not by reading down the page: a register holds a top half where every way
    of reaching the instruction left the same one in it. A top half follows
    moves and an added index, and ends when its register is overwritten or a
    call returns. Code reached only through a jump table starts from what was
    held at the `jr`; code nothing reaches (a nop after a jump) passes nothing
    on."""
    ws = words(code)
    n, end = len(ws), base + 4 * len(ws)

    def fields(w):
        return w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31, w & 0xFFFF

    def step(i: int, state: dict) -> dict:
        """STATE after instruction I (register -> the top half it holds)."""
        op, rs, rt, rd, imm = fields(ws[i])
        dest, carried = 0, None
        if op == 0:
            dest = rd
            if ws[i] & 0x3F in (0x21, 0x25, 0x2D) and (rs in state) != (rt in state):   # addu, or, daddu: a move or an index
                carried = state[rs] if rs in state else state[rt]
        elif op == 0x1C:
            dest = rd
        elif op in WRITES_RT or op in (0x10, 0x11, 0x12) and rs in (0, 1, 2):          # ... and mfc0, mfc1, cfc1
            dest = rt
        if not dest or dest not in state and op != 0x0F and carried is None:
            return state
        state = {r: v for r, v in state.items() if r != dest}
        if op == 0x0F:
            state[rt] = imm << 16
        elif carried is not None:
            state[rd] = carried
        return state

    def flow(i: int) -> tuple[str, int | None]:
        """How instruction I transfers control: (kind, target index)."""
        w = ws[i]
        op, rs, rt, _rd, imm = fields(w)
        target = i + 1 + (imm - 0x10000 if imm & 0x8000 else imm)
        if op == 2:
            dest = (base + 4 * i + 4) & 0xF0000000 | (w & 0x03FFFFFF) << 2
            return ("jump", (dest - base) // 4) if base <= dest < end else ("leave", None)
        if op == 3 or op == 0 and w & 0x3F == 9:
            return "call", None
        if op == 0 and w & 0x3F == 8:
            return ("leave", None) if rs == 31 else ("table", None)
        if op == 4 and rs == rt:                                     # b: a beq that always goes
            return "jump", target
        if op in (4, 5, 6, 7) or op == 1 and rt in (0, 1, 16, 17) or op in (0x10, 0x11, 0x12) and rs == 8 and not rt & 2:
            return "branch", target
        if op in (0x14, 0x15, 0x16, 0x17) or op == 1 and rt in (2, 3, 18, 19) or op in (0x10, 0x11, 0x12) and rs == 8:
            return "likely", target
        return "next", None

    before: list = [None] * n          # what each instruction finds in the registers; None = not reached yet
    tables: list = []                  # what was held at each jump through a table

    def meet(a, b):
        return b if a is None else {r: v for r, v in a.items() if b.get(r) == v}

    def reach(i, state, work):
        if 0 <= i < n:
            merged = meet(before[i], state)
            if merged != before[i]:
                before[i] = merged
                work.append(i)

    def run(work):
        while work:
            i = work.pop()
            state = before[i]
            kind, target = flow(i)
            if kind == "next":
                reach(i + 1, step(i, state), work)
                continue
            slot = step(i + 1, state) if i + 1 < n else state       # the delay slot runs before control moves
            if i + 1 < n:
                before[i + 1] = meet(before[i + 1], state)
            if kind == "call":
                reach(i + 2, {r: v for r, v in slot.items() if r not in CALLER_SAVED}, work)
            elif kind == "table":
                tables.append(slot)
            elif kind in ("jump", "branch", "likely"):
                reach(target, slot, work)
                if kind != "jump":
                    reach(i + 2, state if kind == "likely" else slot, work)       # a likely branch not taken skips its slot

    if n:
        before[0] = {}
        run([0])
    while None in before:               # reached only through a jump table, or not at all
        i = before.index(None)
        if not tables or not ws[i]:     # dead: the padding after a jump. Nothing flows out of it,
            before[i] = {}              # or it would wipe what the code after it really holds
            continue
        start = {}
        for held in tables:
            start = held if held is tables[0] else meet(start, held)
        before[i] = dict(start)
        run([i])

    out = []
    for i, w in enumerate(ws):
        op, rs, _rt, _rd, imm = fields(w)
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if op in (2, 3):
            target = (base + 4 * i + 4) & 0xF0000000 | (w & 0x03FFFFFF) << 2
            if op == 3 or not base <= target < end:
                out.append((i, "call", target))
        elif op in LO16:
            if rs == 28:
                if gp is not None:
                    out.append((i, "gp", (gp + simm) & 0xFFFFFFFF))
            elif rs in before[i]:
                out.append((i, "data", (before[i][rs] + simm) & 0xFFFFFFFF))
    return out


def shape(b: bytes) -> str:
    """The loose fingerprint: equal for two functions with the same instructions
    whatever their addresses, constants and struct offsets. Two functions that
    differ only in a number share it, so it finds relatives, not copies."""
    return hashlib.sha1(struct.pack(f"<{len(trim(b)) // 4}I", *map(mask, words(trim(b))))).hexdigest()[:16]


def identity(b: bytes) -> bytes:
    """One function's words with only its address fields masked: what two
    copies must share to be the same function.

    A register is address-derived when a `lui` loads it with the top half of a
    RAM address, or when it is copied or computed from one that is; immediates
    on those (and on $gp) are %lo halves and stay masked. An immediate on
    $zero, a small offset on any other register and an `ori` are the code's own
    and are compared. Large offsets on other registers stay masked: an address
    half can reach one through a spill."""
    w = words(trim(b))
    tainted, grew = set(), True
    while grew:
        grew = False
        for x in w:
            op, rs, rt, rd = x >> 26, (x >> 21) & 31, (x >> 16) & 31, (x >> 11) & 31
            new = None
            if op == 0x0F and 0x10 <= x & 0xFFFF < 0x200:
                new = rt
            elif op in (0x09, 0x19) and rs in tainted:
                new = rt
            elif op == 0 and x & 0x3F in (0x21, 0x2D, 0x25) and (rs in tainted or rt in tainted):
                new = rd
            if new and new not in tainted:
                tainted.add(new)
                grew = True
    out = []
    for x in w:
        op, rs, imm = x >> 26, (x >> 21) & 31, x & 0xFFFF
        if op in (2, 3):
            x &= 0xFC000000
        elif op == 0x0F:
            if 0x10 <= imm < 0x200:
                x &= 0xFFFF0000
        elif rs == 28 and op >= 8:
            x &= 0xFFFF0000
        elif (op in MEM or op in (0x09, 0x0D, 0x19)) and rs != 29:
            small = imm < 0x1000 or imm >= 0xF000
            if rs in tainted or (rs != 0 and op != 0x0D and not small):
                x &= 0xFFFF0000
        out.append(x)
    return struct.pack(f"<{len(out)}I", *out)


def fingerprint(b: bytes) -> str:
    """The strict fingerprint: equal only for the same function at another
    address (identity above)."""
    return hashlib.sha1(identity(b)).hexdigest()[:16]


def ends_in_jump(w: int) -> bool:
    """jr, j or b (beq $0, $0): the last instruction of a function whose delay
    slot follows it."""
    return (w & 0xFC1FFFFF) == 0x00000008 or w >> 26 == 2 or w >> 16 == 0x1000


def code_size(b: bytes) -> int:
    """The function's size: its bytes without the padding after it, but with a
    nop in the delay slot of its final jump, which trim() takes for padding."""
    t = trim(b)
    if len(t) < len(b) and t and ends_in_jump(words(t[-4:])[0]):
        return len(t) + 4
    return len(t)


def split(text: bytes, base: int, extra=()) -> list[tuple[int, int]]:
    """(offset, size) of each function in TEXT loaded at BASE: starts at call
    targets, after returns, after tail calls followed by a frame opener, at a
    frame opener after padding, and at EXTRA word indices. The size includes
    the padding up to the next function; code_size() takes it off."""
    w = words(text)
    starts = {0, *extra}
    for i, x in enumerate(w):
        if x >> 26 == 3:
            t = ((x & 0x3FFFFFF) << 2) | (base & 0xF0000000)
            if base <= t < base + len(text):
                starts.add((t - base) // 4)
        if (x == 0x03E00008 or x >> 26 == 2) and i + 2 < len(w):
            j = i + 2
            while j < len(w) and w[j] == 0:
                j += 1
            if x == 0x03E00008 or (j < len(w) and w[j] >> 16 == 0x27BD and w[j] & 0x8000):
                starts.add(j)
        if x >> 16 == 0x27BD and x & 0x8000 and i and w[i - 1] == 0:
            starts.add(i)
    s = sorted(x for x in starts if x < len(w))
    return [(a * 4, (b - a) * 4) for a, b in zip(s, s[1:] + [len(w)])]


def elf_sections(path: Path) -> dict[str, tuple[int, bytes, int]]:
    """name -> (address, bytes, flags) of the sections an ELF file stores (not .bss)."""
    b = Path(path).read_bytes()
    if b[:4] != b"\x7fELF":
        raise ValueError(f"{path}: not an ELF file")
    shoff, = struct.unpack_from("<I", b, 0x20)
    entsize, count, names_at = struct.unpack_from("<3H", b, 0x2E)
    if not count:
        return {}
    strtab = struct.unpack_from("<6I", b, shoff + names_at * entsize)[4]
    out = {}
    for i in range(count):
        name, kind, flags, addr, off, size = struct.unpack_from("<6I", b, shoff + i * entsize)
        if kind == 1 and size:                                  # SHT_PROGBITS
            out[b[strtab + name:b.index(b"\0", strtab + name)].decode()] = (addr, b[off:off + size], flags)
    return out
