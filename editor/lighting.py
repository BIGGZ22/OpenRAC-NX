"""The colours the game lights its terrain with at level load (LightTfrags, func_002362B0).

Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs:
ISC License, Copyright (c) 2026 ReRAC contributors. The arithmetic is the EE's VU0 macro
code, modelled on raw float bits (no NaN, infinity or denormals; every result truncated), so
the bytes are the game's (docs/engine/systems/LIGHTING.md).

Inputs: the level's directional light sets (the gameplay file's light section) and the
executable's 256-entry (cos, sin) table, which the game reads rather than computes.
"""

import math
import struct

from formats import FormatError, span, unpack

# The (cos, sin) table in SCES_509.16; each version's is in disc.VERSIONS (NTSC-U's at 0x165500,
# the same 2048 bytes).
NORMAL_TABLE_ADDRESS = 0x165600
BANK_SETS = 16
MAX_LEVEL_LIGHTS = 12

SIGN, MAX, ONE = 0x80000000, 0x7FFFFFFF, 0x3F800000


def _exp(x: int) -> int:
    return (x >> 23) & 0xFF


def _man(x: int) -> int:
    return (x & 0x7FFFFF) | 0x800000


def _pack(sign: int, mag: int, lsb_exp: int) -> int:
    k = mag.bit_length() - 1
    e = lsb_exp + (k - 23)
    m = mag >> (k - 23) if k >= 23 else mag << (23 - k)
    if e > 255:
        return sign | MAX
    if e < 1:
        return sign
    return sign | (e << 23) | (m & 0x7FFFFF)


def mul(a: int, b: int) -> int:
    s = (a ^ b) & SIGN
    if _exp(a) == 0 or _exp(b) == 0:
        return s
    return _pack(s, (_man(a) * _man(b)) >> 23, _exp(a) + _exp(b) - 127)


def add(a: int, b: int) -> int:
    ea, eb = _exp(a), _exp(b)
    if ea == 0 or eb == 0:
        if ea:
            return a
        if eb:
            return b
        return SIGN if a & b & SIGN else 0
    hi, lo = (a, b) if ea >= eb else (b, a)
    d = _exp(hi) - _exp(lo)
    if d >= 25:
        return hi
    if d >= 1:
        lo &= (0xFFFFFFFF << (d - 1)) & 0xFFFFFFFF
    total = (-1 if hi & SIGN else 1) * (_man(hi) << d) + (-1 if lo & SIGN else 1) * _man(lo)
    if total == 0:
        return 0
    return _pack(SIGN if total < 0 else 0, abs(total), _exp(lo))


def sub(a: int, b: int) -> int:
    return add(a, b ^ SIGN)


def div(a: int, b: int) -> int:
    s = (a ^ b) & SIGN
    if _exp(b) == 0:
        return s | MAX
    if _exp(a) == 0:
        return s
    return _pack(s, (_man(a) << 24) // _man(b), _exp(a) - _exp(b) + 127 - 1)


def sqrt(x: int) -> int:
    if _exp(x) == 0:
        return 0
    e = _exp(x) - 127
    odd = e % 2
    return _pack(0, math.isqrt(_man(x) << (23 + odd)), (e - odd) // 2 + 127)


def itof12(i: int) -> int:
    if i == 0:
        return 0
    return _pack(SIGN if i < 0 else 0, abs(i), 127 + 23 - 12)


def _key(x: int) -> int:
    return -(x & MAX) if x & SIGN else x


def fmax(a: int, b: int) -> int:
    return b if _key(b) > _key(a) else a


def fmin(a: int, b: int) -> int:
    return b if _key(b) < _key(a) else a


def bits(values) -> list[int]:
    return [struct.unpack("<I", struct.pack("<f", v))[0] for v in values]


def dot3(a: list[int], b: list[int]) -> int:
    p = [mul(x, y) for x, y in zip(a, b)]
    return add(add(p[0], p[1]), mul(ONE, p[2]))


def normal_table(exe: bytes, address: int = NORMAL_TABLE_ADDRESS) -> list[tuple[int, int]]:
    """The 256 (cos, sin) pairs as raw float bits at ADDRESS, from the executable's load segments."""
    phoff, = unpack("<I", exe, 0x1C)
    size, count = unpack("<HH", exe, 0x2A)
    for i in range(count):
        kind, offset, vaddr, _paddr, filesz = unpack("<5I", exe, phoff + i * size)
        if kind == 1 and vaddr <= address and address + 2048 <= vaddr + filesz:
            words = struct.unpack("<512I", span(exe, offset + address - vaddr, 2048))
            return [(words[2 * k], words[2 * k + 1]) for k in range(256)]
    raise FormatError("the executable has no normal table")


def light_bank(gameplay: bytes) -> list[list[list[int]]]:
    """16 directional sets (colour A, direction A, colour B, direction B as float bits); the
    loader copies at most 12 from the gameplay file (u32 at 0x04: count, 12 bytes, sets)."""
    at, = unpack("<I", gameplay, 0x04)
    count, = unpack("<i", gameplay, at)
    if count < 0:
        raise FormatError("negative directional light count")
    count = min(count, MAX_LEVEL_LIGHTS)
    sets = [[[0] * 4 for _ in range(4)] for _ in range(BANK_SETS)]
    for s in range(count):
        words = unpack("<16I", gameplay, at + 0x10 + s * 0x40)
        sets[s] = [list(words[k * 4:k * 4 + 4]) for k in range(4)]
    return sets


def _active_set(bank, select: int):
    if select & 0xFF00 == 0:
        ca, da, cb, db = (list(v) for v in bank[select & 0xF])
    else:
        t = itof12((select >> 4) & 0xFF0)
        w = sub(ONE, t)
        a, b = bank[select & 0xF], bank[(select >> 4) & 0xF]

        def blend(x, y):
            return [add(mul(p, w), mul(q, t)) for p, q in zip(x, y)]

        def norm(v):
            q = div(ONE, sqrt(dot3(v, v)))
            return [mul(v[0], q), mul(v[1], q), mul(v[2], q), v[3]]

        ca, cb = blend(a[0], b[0]), blend(a[2], b[2])
        da, db = norm(blend(a[1], b[1])), norm(blend(a[3], b[3]))
    wa, wb = add(0, ca[3]), add(0, cb[3])
    ca[3] = cb[3] = 0
    return ca, da, cb, db, wa, wb


def decode_normal(table, azimuth: int, elevation: int) -> list[int]:
    ca, sa = table[azimuth]
    ce, se = table[elevation]
    x, y, z = mul(ca, ce), mul(sa, ce), add(0, se)
    return [sub(0, x), sub(0, y), sub(0, z), se]


def pext5(c: int) -> list[int]:
    return [(c & 0x1F) << 3, ((c >> 5) & 0x1F) << 3, ((c >> 10) & 0x1F) << 3, (c >> 15) << 7]


def light_vertex(bank, normal: list[int], colour: int, select: int) -> tuple[int, int, int, int]:
    """One vertex's RGBA bytes: its 5:5:5:1 base colour plus the two lights of its set."""
    ca, da, cb, db, wa, wb = _active_set(bank, select)
    a, b = dot3(normal, da), dot3(normal, db)
    a, b = fmax(a, mul(a, wa)), fmax(b, mul(b, wb))
    acc = [mul(0x47800000 + c, ONE) for c in pext5(colour)]
    acc = [add(x, mul(y, a)) for x, y in zip(acc, ca)]
    acc = [add(x, mul(y, b)) for x, y in zip(acc, cb)]
    clamp = 0x478000FF
    return (fmin(acc[0], clamp) & 0xFF, fmin(acc[1], clamp) & 0xFF, fmin(acc[2], clamp) & 0xFF, acc[3] & 0xFF)


def light_records(records: bytes, single: int, bank, table) -> list[tuple[int, int, int, int]]:
    """A fragment's colours from its 8-byte light records (position offset, azimuth,
    elevation, 5:5:5:1 colour, set selector), one per position. `single` is the header's
    whole-fragment set (byte 0x34, signed; negative: each vertex's own selector)."""
    out = []
    for _offset, azimuth, elevation, colour, select in struct.iter_unpack("<HBBHH", records):
        if single >= 0:
            select = single & 0xF
        out.append(light_vertex(bank, decode_normal(table, azimuth, elevation), colour, select))
    return out


# Ties (LightTies, func_00238688; ReRAC's tie_light.rs): an instance's 64 light slots, each
# with the class's slot normal and the instance's 5:5:5:1 ambient colour, lit by its two
# directional lights turned into the class's space. RGB clamps at 243, not 255.
TIE_COLOUR_CLAMP = 0x478000F3
TIE_SLOTS = 64


def _len2(v: list[int]) -> int:
    return add(add(mul(v[0], v[0]), mul(v[1], v[1])), mul(ONE, mul(v[2], v[2])))


def tie_light_rows(columns: list[list[int]], bank, select: int):
    """The EE half for one instance: the selected (or blended) set's colours and back
    factors, and the light directions in class space (L = -N^T d, N the unit columns of
    the instance matrix), transposed as VU0 holds them. `columns`: the matrix's first three
    columns as float bits. No point lights: the load-time pass has none."""
    select &= 0xFFFF
    if select & 0xFF00 == 0:
        ca, da, cb, db = (list(v) for v in bank[select & 0xF])
    else:
        t = itof12((select >> 4) & 0xFF0)
        w = sub(ONE, t)
        a, b = bank[select & 0xF], bank[(select >> 4) & 0xF]

        def blend(x, y):
            return [add(mul(p, w), mul(q, t)) for p, q in zip(x, y)]

        ca, cb, da, db = blend(a[0], b[0]), blend(a[2], b[2]), blend(a[1], b[1]), blend(a[3], b[3])
        for d in (da, db):
            q = div(ONE, sqrt(_len2(d)))
            d[0], d[1], d[2] = mul(d[0], q), mul(d[1], q), mul(d[2], q)
    wa, wb = add(0, ca[3]), add(0, cb[3])
    ca[3] = cb[3] = 0
    units = []
    for c in columns:
        inv = div(ONE, sqrt(_len2(c)))
        units.append([mul(c[0], inv), mul(c[1], inv), mul(c[2], inv)])

    def class_space(d):
        out = []
        for c in range(3):
            n = units[c]
            acc = add(mul(sub(0, n[0]), d[0]), mul(sub(0, n[1]), d[1]))
            out.append(add(acc, mul(sub(0, n[2]), d[2])))
        return out

    la, lb, lp = class_space(da), class_space(db), [0, 0, 0]
    rows = [[la[i], lb[i], lp[i], 0] for i in range(3)]
    return [ca, cb, [0, 0, 0, 0]], [wa, wb, 0], rows


def tie_light_slot(colours, back, rows, normal: tuple[int, int, int], ambient: int) -> tuple[int, int, int, int]:
    n = bits([normal[k] / 32768.0 for k in range(3)])
    f = []
    for k in range(3):
        d = add(add(mul(rows[0][k], n[0]), mul(rows[1][k], n[1])), mul(rows[2][k], n[2]))
        f.append(fmax(d, mul(d, back[k])))
    out = []
    for c, base in enumerate(pext5(ambient)):
        acc = mul(0x47800000 + base, ONE)
        for light in range(3):
            acc = add(acc, mul(colours[light][c], f[light]))
        if c < 3:
            acc = fmin(acc, TIE_COLOUR_CLAMP)
        out.append(acc & 0xFF)
    return tuple(out)


def light_tie_instance(normals, matrix: list[float], ambient: list[int], select: int, bank) -> list[tuple]:
    """The instance's 64 lit RGBA colours, by light slot (0x80 = 1.0 under MODULATE).
    `normals`: the class's 64 (x, y, z) slot normals (s16 / 32768); `matrix`: the instance's
    column-major 4x4; `ambient`: its 64 5:5:5:1 colours; `select`: its light set selector."""
    columns = [bits(matrix[c * 4:c * 4 + 3]) for c in range(3)]
    colours, back, rows = tie_light_rows(columns, bank, select)
    return [tie_light_slot(colours, back, rows, normals[j], ambient[j]) for j in range(TIE_SLOTS)]


# Shrubs (LightShrubs, func_0022B8F8; ReRAC's shrub_light.rs): the same pass as the ties' on
# the class's 24 normals, except that a blended set scales only xyz (so the back factors add),
# and the ambient is the instance's one colour with alpha 0x80.
SHRUB_NORMALS = 24


def light_shrub_instance(normals, matrix: list[float], colour: list[int], select: int, bank) -> list[tuple]:
    """The instance's 24 lit RGBA colours, by class normal (0x80 = 1.0 under MODULATE)."""
    select &= 0xFFFF
    if select & 0xFF00 == 0:
        ca, da, cb, db = (list(v) for v in bank[select & 0xF])
    else:
        t = itof12((select >> 4) & 0xFF0)
        w = sub(ONE, t)
        a, b = bank[select & 0xF], bank[(select >> 4) & 0xF]

        def blend(x, y):
            return [add(mul(x[0], w), mul(y[0], t)), add(mul(x[1], w), mul(y[1], t)),
                    add(mul(x[2], w), mul(y[2], t)), add(x[3], y[3])]

        ca, cb, da, db = blend(a[0], b[0]), blend(a[2], b[2]), blend(a[1], b[1]), blend(a[3], b[3])
        for d in (da, db):
            q = div(ONE, sqrt(_len2(d)))
            d[0], d[1], d[2] = mul(d[0], q), mul(d[1], q), mul(d[2], q)
    wa, wb = add(0, ca[3]), add(0, cb[3])
    ca[3] = cb[3] = 0
    units = []
    for c in range(3):
        col = bits(matrix[c * 4:c * 4 + 3])
        q = div(ONE, sqrt(_len2(col)))
        units.append([mul(col[0], q), mul(col[1], q), mul(col[2], q)])

    def class_space(d):
        return [add(add(mul(sub(0, units[c][0]), d[0]), mul(sub(0, units[c][1]), d[1])),
                    mul(sub(0, units[c][2]), d[2])) for c in range(3)]

    la, lb = class_space(da), class_space(db)
    rows = [[add(0, la[i]), add(0, lb[i]), 0, 0] for i in range(3)]
    colours, back = [ca, cb, [0, 0, 0, 0]], [wa, wb, 0]
    ambient = [0x47800000 | (colour[k] & 0xFF) for k in range(3)] + [0x47800000 | 0x80]
    out = []
    for j in range(SHRUB_NORMALS):
        n = bits([normals[j][k] / 32768.0 for k in range(3)])
        f = []
        for k in range(3):
            d = add(add(mul(rows[0][k], n[0]), mul(rows[1][k], n[1])), mul(rows[2][k], n[2]))
            f.append(fmax(d, mul(d, back[k])))
        lit = []
        for c in range(4):
            acc = mul(ambient[c], ONE)
            for light in range(3):
                acc = add(acc, mul(colours[light][c], f[light]))
            if c < 3:
                acc = fmin(acc, TIE_COLOUR_CLAMP)
            lit.append(acc & 0xFF)
        out.append(tuple(lit))
    return out
