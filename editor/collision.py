"""Static collision: the level's baked world mesh, as triangles with surface types.

The layout follows ReRAC's notes on the format (games/rac1/docs/ASSETS.md, "Collision").
A three-level tree (Z, then Y, then X) of 4-unit cells leads to leaves; each
leaf has its own packed vertices and the faces that touch the cell, so a face
that spans several cells is stored once per cell and is merged here.
"""

from dataclasses import dataclass, field
import colorsys
import math
import struct

from formats import FormatError, check, span, unpack
from mesh import Mesh

CELL = 4
LEAF_LIMIT = 0x1000
SCALE = 64  # World coordinates are held in 1/64 units while decoding.
DEFAULT_SURFACE = 0x1f  # "No special surface".


@dataclass
class Collision:
    cells: int = 0
    faces: int = 0                                           # Stored faces, with duplicates across cells.
    triangles: list[tuple[tuple, tuple, tuple, int]] = field(default_factory=list)
    hero_groups: int = 0

    def surfaces(self) -> dict[int, int]:
        """Triangle count per surface id (the low five bits of the type byte)."""
        counts: dict[int, int] = {}
        for *_, kind in self.triangles:
            counts[kind & 0x1f] = counts.get(kind & 0x1f, 0) + 1
        return dict(sorted(counts.items()))


def signed(value: int, bits: int) -> int:
    return value - (1 << bits) if value >> (bits - 1) else value


def vertex(word: int, centre: tuple[int, int, int]) -> tuple[int, int, int]:
    """A packed vertex in 1/64 units: X, Y are 10 bits at 1/16, Z is 12 bits at 1/64."""
    return (centre[0] + signed(word & 0x3ff, 10) * 4,
            centre[1] + signed(word >> 10 & 0x3ff, 10) * 4,
            centre[2] + signed(word >> 20, 12))


def table(mesh: bytes, at: int, entry: str) -> tuple[int, list[int]]:
    """A tree node: a signed base, a count and that many entries."""
    base, count = unpack("<hH", mesh, at)
    return base, list(unpack(f"<{count}{entry}", mesh, at + 4)) if count else []


def leaves(mesh: bytes):
    """Yield ((x, y, z) cell, leaf bytes) for every non-empty cell."""
    z_base, z_slabs = table(mesh, 0, "H")
    for k, slab in enumerate(z_slabs):
        if not slab:
            continue
        y_base, rows = table(mesh, slab * 4, "I")
        for j, row in enumerate(rows):
            if not row:
                continue
            x_base, cells = table(mesh, row, "I")
            for i, word in enumerate(cells):
                if word:
                    size = (word & 0xff) * 16
                    yield (x_base + i, y_base + j, z_base + k), span(mesh, word >> 8, size)


def decode(block: bytes) -> Collision:
    """Merge the leaves of a collision block into triangles (cell by cell, in tree order).

    Triangles are wound so that the front face (the game's normal is
    (v2 - v0) x (v1 - v0)) is counter-clockwise, as glTF wants.
    """
    mesh_at, hero_at = unpack("<II", block)
    if mesh_at < 8 or mesh_at > len(block) or hero_at and not mesh_at <= hero_at <= len(block):
        raise FormatError("collision header is out of range")
    tree = span(block, mesh_at, (hero_at or len(block)) - mesh_at)
    result = Collision()
    seen = set()
    for (x, y, z), leaf in leaves(tree):
        faces, vertices, quads = unpack("<HBB", leaf)
        if quads > faces or 4 + 4 * vertices + 4 * faces + quads > len(leaf):
            raise FormatError(f"collision leaf at cell {(x, y, z)} is inconsistent")
        centre = tuple((4 * c + 2) * SCALE for c in (x, y, z))
        points = [vertex(w, centre) for w in unpack(f"<{vertices}I", leaf, 4)]
        records = [unpack("<4B", leaf, 4 + 4 * vertices + 4 * f) for f in range(faces)]
        extra = leaf[4 + 4 * vertices + 4 * faces:][:quads]
        result.cells += 1
        for f, (a, b, c, kind) in enumerate(records):
            corners = [a, b, c] + ([extra[f]] if f < quads else [])
            if max(corners) >= vertices:
                raise FormatError(f"collision face in cell {(x, y, z)} uses a missing vertex")
            result.faces += 1
            for i in range(1, len(corners) - 1):  # A quad splits along v0-v2.
                p = (points[corners[0]], points[corners[i + 1]], points[corners[i]])
                turn = min(range(3), key=p.__getitem__)
                key = (p[turn:] + p[:turn], kind)
                if key not in seen:
                    seen.add(key)
                    result.triangles.append((*p, kind))
    if hero_at:
        result.hero_groups, = unpack("<I", block, hero_at)
    return result


def colour(kind: int) -> tuple[float, float, float]:
    """The display colour of a type byte's surface id (bits 0-4)."""
    surface = kind & 0x1f
    if surface == DEFAULT_SURFACE:
        return (0.55, 0.55, 0.55)
    if surface == 0:
        return (0.2, 0.45, 1.0)  # Water, as the game's ground probe treats it.
    return colorsys.hsv_to_rgb((surface * 0.381966) % 1, 0.8, 0.95)


def collision_mesh(data: Collision) -> Mesh:
    """One mesh of unindexed triangles with a vertex colour per surface."""
    mesh = Mesh("collision", colours=[])
    for a, b, c, kind in data.triangles:
        first = len(mesh.positions)
        mesh.positions.extend(tuple(x / SCALE for x in p) for p in (a, b, c))
        mesh.colours.extend([(*(c ** 2.2 for c in colour(kind)), 1.0)] * 3)  # glTF colours are linear.
        mesh.add_face(None, (first, first + 1, first + 2))
    return mesh


# --- The block as data: read, edited and written again ------------------------------------------
#
# What follows reads a collision block into a model that writes back to the same bytes, and edits
# it. The layout, measured on all 19 PAL blocks: a 0x40-byte header (the mesh's offset, 0x40, and
# the hero groups' offset, then zeros); the mesh: the root table, then every slab table, then every
# row table (each 4-byte aligned), then every leaf (16-byte aligned, padded with zeros to its size in
# quadwords), the whole padded to 0x40; then the hero groups: a count, padded to 16, 16-byte records,
# each group's data 16-byte aligned, the block padded to 0x40. The order of the slabs, rows and leaves
# in memory is the build tool's, not a sorted one, so the model keeps it: a block written from an
# unchanged model is the disc's own, and an edit rewrites only the cells it touches.
#
# Coordinates in the model are world positions in 1/64 units, as the decoder's: X and Y on a 1/16
# grid (multiples of 4), Z on 1/64.

HEADER = 0x40
LEAF_MAX = 0xff * 16                     # the cell word's size byte, in quadwords
SPAN_XY, SPAN_Z = 511 * 4, 2047          # how far a leaf's vertex can be from its cell's centre
SPAN_XY_LOW, SPAN_Z_LOW = -512 * 4, -2048
WORLD = 1024 * SCALE                     # the game answers queries inside [0, 1024) units only
CELL_UNITS = CELL * SCALE                # a cell's side in model units (256)


def _align(n: int, a: int) -> int:
    return n + (-n % a)


@dataclass
class Leaf:
    """One cell's faces: vertices (world, 1/64) and faces (vertex indices, type byte). The first
    `quads` faces have four indices, the rest three; all in the game's winding."""
    vertices: list[tuple[int, int, int]]
    faces: list[tuple[tuple[int, ...], int]]

    @property
    def quads(self) -> int:
        return sum(1 for corners, _ in self.faces if len(corners) == 4)

    def size(self) -> int:
        return 4 + 4 * len(self.vertices) + 4 * len(self.faces) + self.quads


@dataclass
class HeroGroup:
    """Player-only geometry: a bounding sphere (u16 x, y, z, radius at 1/64), vertices (u16 at 1/64,
    absolute) and triangles of three vertex indices, as stored."""
    sphere: tuple[int, int, int, int]
    vertices: list[tuple[int, int, int]]
    triangles: list[tuple[int, int, int]]


@dataclass
class Block:
    z: tuple[int, int]                                  # root: base, count (in cells)
    slabs: dict[int, tuple[int, int]]                   # z -> y base, count
    rows: dict[tuple[int, int], tuple[int, int]]        # (z, y) -> x base, count
    cells: dict[tuple[int, int, int], Leaf]             # (x, y, z) -> its leaf
    slab_order: list[int]                               # memory order, as the disc has it
    row_order: list[tuple[int, int]]
    leaf_order: list[tuple[int, int, int]]
    heroes: list[HeroGroup] | None                      # None: the block has no hero section


def parse(block: bytes) -> Block:
    """The model of a collision block; write(parse(b)) == b for the layouts on the disc."""
    mesh_at, hero_at = unpack("<II", block)
    check(mesh_at == HEADER and not any(block[8:HEADER]), "collision header is not the usual one")
    check(not hero_at or mesh_at <= hero_at <= len(block), "collision header is out of range")
    mesh = span(block, mesh_at, (hero_at or len(block)) - mesh_at)
    z_base, z_slabs = table(mesh, 0, "H")
    out = Block((z_base, len(z_slabs)), {}, {}, {}, [], [], [], None)
    where = []                                          # (offset, kind, key) to recover the order
    for k, slab in enumerate(z_slabs):
        if not slab:
            continue
        z = z_base + k
        y_base, rows = table(mesh, slab * 4, "I")
        out.slabs[z] = (y_base, len(rows))
        where.append((slab * 4, 0, z))
        for j, row in enumerate(rows):
            if not row:
                continue
            y = y_base + j
            x_base, words = table(mesh, row, "I")
            out.rows[(z, y)] = (x_base, len(words))
            where.append((row, 1, (z, y)))
            for i, word in enumerate(words):
                if not word:
                    continue
                cell = (x_base + i, y, z)
                leaf = span(mesh, word >> 8, (word & 0xff) * 16)
                faces, count, quads = unpack("<HBB", leaf)
                check(quads <= faces and 4 + 4 * count + 4 * faces + quads <= len(leaf),
                       f"collision leaf at cell {cell} is inconsistent")
                centre = tuple((4 * c + 2) * SCALE for c in cell)
                points = [vertex(w, centre) for w in unpack(f"<{count}I", leaf, 4)]
                extra = leaf[4 + 4 * count + 4 * faces:][:quads]
                records = []
                for f in range(faces):
                    a, b, c, kind = unpack("<4B", leaf, 4 + 4 * count + 4 * f)
                    corners = (a, b, c, extra[f]) if f < quads else (a, b, c)
                    check(max(corners) < count, f"collision face in cell {cell} uses a missing vertex")
                    records.append((corners, kind))
                out.cells[cell] = Leaf(points, records)
                where.append((word >> 8, 2, cell))
    where.sort()
    out.slab_order = [key for _, kind, key in where if kind == 0]
    out.row_order = [key for _, kind, key in where if kind == 1]
    out.leaf_order = [key for _, kind, key in where if kind == 2]
    if hero_at:
        count, = unpack("<I", block, hero_at)
        out.heroes = []
        for g in range(count):
            x, y, z, r, tris, verts, at = unpack("<4HHHI", block, hero_at + 16 + 16 * g)
            data = hero_at + at
            vertices = [unpack("<3H", block, data + 8 * v) for v in range(verts)]
            triangles = [unpack("<3B", block, data + 8 * verts + 4 * t) for t in range(tris)]
            out.heroes.append(HeroGroup((x, y, z, r), vertices, triangles))
    return out


def encode_leaf(cell: tuple[int, int, int], leaf: Leaf) -> bytes:
    """A leaf's bytes, padded to 16 (faces must already be quads first)."""
    centre = tuple((4 * c + 2) * SCALE for c in cell)
    words = []
    for p in leaf.vertices:
        dx, dy, dz = (p[i] - centre[i] for i in range(3))
        check(dx % 4 == 0 and dy % 4 == 0, f"cell {cell}: a vertex is off the 1/16 grid")
        check(SPAN_XY_LOW <= dx <= SPAN_XY and SPAN_XY_LOW <= dy <= SPAN_XY and SPAN_Z_LOW <= dz <= SPAN_Z,
               f"cell {cell}: a vertex is more than 32 units from the cell")
        words.append((dx // 4 & 0x3ff) | (dy // 4 & 0x3ff) << 10 | (dz & 0xfff) << 20)
    quads = leaf.quads
    check(all(len(c) == 4 for c, _ in leaf.faces[:quads]), f"cell {cell}: quads must come first")
    out = bytearray(struct.pack("<HBB", len(leaf.faces), len(leaf.vertices), quads))
    out += struct.pack(f"<{len(words)}I", *words)
    for corners, kind in leaf.faces:
        out += bytes((*corners[:3], kind))
    out += bytes(c[3] for c, _ in leaf.faces[:quads])
    return bytes(out) + bytes(-len(out) % 16)


def write(model: Block) -> bytes:
    """The collision block of a model (validate() first for a model that was edited)."""
    z_base, z_count = model.z
    root = 4 + 2 * z_count
    at = _align(root, 4)
    slab_at, row_at, leaf_at = {}, {}, {}
    for z in model.slab_order:
        slab_at[z] = at
        at += 4 + 4 * model.slabs[z][1]
    for key in model.row_order:
        row_at[key] = at
        at += 4 + 4 * model.rows[key][1]
    at = _align(at, 16)
    leaves = {}
    for cell in model.leaf_order:
        leaves[cell] = encode_leaf(cell, model.cells[cell])
        leaf_at[cell] = at
        at += len(leaves[cell])
    mesh = bytearray(_align(at, 0x40))
    entries = []
    for k in range(z_count):
        offset = slab_at.get(z_base + k, 0)
        check(offset // 4 <= 0xffff, "collision: the slab tables do not fit the root's 16-bit entries")
        entries.append(offset // 4)
    struct.pack_into(f"<hH{z_count}H", mesh, 0, z_base, z_count, *entries)
    for z, offset in slab_at.items():
        y_base, count = model.slabs[z]
        struct.pack_into(f"<hH{count}I", mesh, offset, y_base, count,
                         *(row_at.get((z, y_base + j), 0) for j in range(count)))
    for (z, y), offset in row_at.items():
        x_base, count = model.rows[(z, y)]
        words = []
        for i in range(count):
            cell = (x_base + i, y, z)
            if cell in leaf_at:
                check(leaf_at[cell] < 1 << 24, "collision: the mesh is larger than a cell word can address")
                words.append(leaf_at[cell] << 8 | len(leaves[cell]) // 16)
            else:
                words.append(0)
        struct.pack_into(f"<hH{count}I", mesh, offset, x_base, count, *words)
    for cell, offset in leaf_at.items():
        mesh[offset:offset + len(leaves[cell])] = leaves[cell]
    hero_at = HEADER + len(mesh) if model.heroes is not None else 0
    out = bytearray(struct.pack("<II", HEADER, hero_at) + bytes(HEADER - 8)) + mesh
    if model.heroes is not None:
        out += struct.pack("<I12x", len(model.heroes))
        records, data = bytearray(), bytearray()
        start = 16 + 16 * len(model.heroes)
        for g in model.heroes:
            data += bytes(-len(data) % 16)
            records += struct.pack("<4HHHI", *g.sphere, len(g.triangles), len(g.vertices), start + len(data))
            for p in g.vertices:
                data += struct.pack("<3H2x", *p)
            for t in g.triangles:
                data += bytes((*t, 0))
        out += records + data
    return bytes(out) + bytes(-len(out) % 0x40)


def validate(model: Block) -> None:
    """Raises FormatError for what the game cannot hold: leaf limits, coordinates, the tables."""
    check(0 <= model.z[0] and model.z[0] + model.z[1] <= 256 and model.z[1] <= 0xffff,
           "collision: Z cells outside 0-255 (the game answers queries inside 0-1024 units only)")
    for z, (y_base, count) in model.slabs.items():
        check(0 <= y_base and y_base + count <= 256, f"collision: slab {z} has Y cells outside 0-255")
    for key, (x_base, count) in model.rows.items():
        check(0 <= x_base and x_base + count <= 256, f"collision: row {key} has X cells outside 0-255")
    for cell, leaf in model.cells.items():
        x, y, z = cell
        check(model.rows.get((z, y), (0, 0))[0] <= x < sum(model.rows.get((z, y), (0, 0))),
               f"collision: cell {cell} is not in its row")
        check(len(leaf.faces) <= 0xffff, f"collision: cell {cell} has too many faces")  # the disc has empty leaves too
        check(len(leaf.vertices) <= 255, f"collision: cell {cell} has {len(leaf.vertices)} vertices, 255 at most: "
                                          "simplify the mesh there")
        check(leaf.quads <= 255, f"collision: cell {cell} has more than 255 quads")
        check(leaf.size() <= LEAF_MAX, f"collision: cell {cell} needs {leaf.size()} bytes, {LEAF_MAX} at most")
        for corners, kind in leaf.faces:
            check(len(set(corners)) == len(corners) and max(corners) < len(leaf.vertices) and 0 <= kind <= 0xff,
                   f"collision: a face of cell {cell} is malformed")
    for n, g in enumerate(model.heroes or []):
        check(0 < len(g.triangles) <= 255 and len(g.vertices) <= 255,
               f"collision: hero group {n} has {len(g.triangles)} triangles and {len(g.vertices)} vertices, "
               "255 of each at most")
        check(all(0 <= c <= 0xffff for p in g.vertices for c in p) and all(0 <= c <= 0xffff for c in g.sphere),
               f"collision: hero group {n} is outside 0-1024 units")
        check(all(max(t) < len(g.vertices) for t in g.triangles), f"collision: hero group {n} uses a missing vertex")


# --- Edits: triangles removed and added ----------------------------------------------------------

def canonical(p: tuple) -> tuple:
    """A triangle (three points, glTF winding) turned to start at its least point: one key per triangle."""
    turn = min(range(3), key=p.__getitem__)
    return p[turn:] + p[:turn]


def face_triangles(leaf: Leaf, corners: tuple[int, ...]) -> list[tuple[tuple, tuple]]:
    """A face's triangles: (the game's order of corner indices, the canonical key in glTF winding),
    a quad split along v0-v2 as the decoder splits it."""
    out = []
    for i in range(1, len(corners) - 1):
        game = (corners[0], corners[i], corners[i + 1])
        p = (leaf.vertices[game[0]], leaf.vertices[game[2]], leaf.vertices[game[1]])
        out.append((game, canonical(p)))
    return out


def block_triangles(model: Block) -> set[tuple]:
    """Every distinct (canonical triangle, type byte) of the model's cells: what the editor shows."""
    out = set()
    for leaf in model.cells.values():
        for corners, kind in leaf.faces:
            for _, key in face_triangles(leaf, corners):
                out.add((key, kind))
    return out


def overlaps(p: tuple, cell: tuple[int, int, int]) -> bool:
    """Whether a triangle touches a cell's box (boundary included), by the separating axis test in
    exact integers."""
    h = CELL_UNITS // 2
    c = [cell[i] * CELL_UNITS + h for i in range(3)]
    v = [[q[i] - c[i] for i in range(3)] for q in p]
    for i in range(3):
        if min(q[i] for q in v) > h or max(q[i] for q in v) < -h:
            return False
    e = [[v[(k + 1) % 3][i] - v[k][i] for i in range(3)] for k in range(3)]

    def cross(a, b):
        return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])

    def dot(a, b):
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]

    n = cross(e[0], e[1])
    if abs(dot(n, v[0])) > h * sum(abs(x) for x in n):
        return False
    for edge in e:
        for axis in ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
            a = cross(edge, axis)
            if a == (0, 0, 0):
                continue
            ps = [dot(a, q) for q in v]
            r = h * sum(abs(x) for x in a)
            if min(ps) > r or max(ps) < -r:
                return False
    return True


def quantise(p: tuple[float, float, float]) -> tuple[int, int, int]:
    """A point in game units as the leaves store it: X and Y to 1/16, Z to 1/64."""
    return (round(p[0] * 16) * 4, round(p[1] * 16) * 4, round(p[2] * SCALE))


def cells_of(p: tuple) -> list[tuple[int, int, int]]:
    """The cells a triangle (model units) touches."""
    low = [min(q[i] for q in p) // CELL_UNITS for i in range(3)]
    high = [max(q[i] for q in p) // CELL_UNITS for i in range(3)]
    return [(x, y, z) for z in range(low[2], high[2] + 1) for y in range(low[1], high[1] + 1)
            for x in range(low[0], high[0] + 1) if overlaps(p, (x, y, z))]


def fits(p: tuple, cell: tuple[int, int, int]) -> bool:
    centre = tuple((4 * c + 2) * SCALE for c in cell)
    return all(SPAN_XY_LOW <= q[0] - centre[0] <= SPAN_XY and SPAN_XY_LOW <= q[1] - centre[1] <= SPAN_XY
               and SPAN_Z_LOW <= q[2] - centre[2] <= SPAN_Z for q in p)


def degenerate(p: tuple) -> bool:
    a = [p[1][i] - p[0][i] for i in range(3)]
    b = [p[2][i] - p[0][i] for i in range(3)]
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]) == (0, 0, 0)


def split(p: tuple) -> list[tuple]:
    """Triangles small enough that each cell they touch can hold all three corners: the longest edge
    is halved (its midpoint on the leaves' grid) until they are."""
    out, todo = [], [p]
    while todo:
        t = todo.pop()
        cells = cells_of(t)
        if all(fits(t, c) for c in cells):
            out.append((t, cells))
            continue
        lengths = [sum((t[(k + 1) % 3][i] - t[k][i]) ** 2 for i in range(3)) for k in range(3)]
        k = lengths.index(max(lengths))
        a, b, c = t[k], t[(k + 1) % 3], t[(k + 2) % 3]
        m = ((a[0] + b[0]) // 8 * 4, (a[1] + b[1]) // 8 * 4, (a[2] + b[2]) // 2)
        todo += [s for s in ((a, m, c), (m, b, c)) if not degenerate(s)]
    return out


def edit(model: Block, removed: set[tuple], added: list[tuple[tuple, int]]) -> tuple[int, int, int]:
    """Edits MODEL in place: REMOVED holds (canonical triangle, type byte) keys to take out of every
    cell; ADDED holds (triangle in glTF winding, model units, type byte) to put into every cell it
    touches. A quad that loses one half keeps the other as a triangle. Cells nothing touches keep
    their leaves as they were. Returns (cells rewritten, cells added, triangles added after splitting)."""
    touched: dict[tuple, list] = {}
    pieces = 0
    for p, kind in added:
        for t, cells in split(p):
            pieces += 1
            for cell in cells:
                touched.setdefault(cell, []).append((t, kind))
    if removed:
        for cell, leaf in model.cells.items():
            if cell in touched:
                continue
            if any((key, kind) in removed for corners, kind in leaf.faces for _, key in face_triangles(leaf, corners)):
                touched[cell] = []
    rewritten = new = 0
    for cell, extra in touched.items():
        leaf = model.cells.get(cell)
        quads, tris, keys = [], [], set()
        if leaf is not None:
            for corners, kind in leaf.faces:
                halves = face_triangles(leaf, corners)
                keep = [h for h in halves if (h[1], kind) not in removed]
                if len(keep) == len(halves):
                    (quads if len(corners) == 4 else tris).append((tuple(leaf.vertices[i] for i in corners), kind))
                else:
                    tris += [(tuple(leaf.vertices[i] for i in game), kind) for game, _ in keep]
                keys.update((key, kind) for _, key in keep)
        for (a, b, c), kind in extra:
            if (canonical((a, b, c)), kind) not in keys:
                keys.add((canonical((a, b, c)), kind))
                tris.append(((a, c, b), kind))           # the game's winding is the other way
        faces = quads + tris
        if not faces:
            if leaf is not None:
                del model.cells[cell]
                model.leaf_order.remove(cell)
                rewritten += 1
            continue
        old = leaf.vertices if leaf is not None else []
        used = {p for points, _ in faces for p in points}
        points = [p for p in old if p in used]
        points += list(dict.fromkeys(p for pts, _ in faces for p in pts if p not in set(old)))
        index = {p: i for i, p in enumerate(points)}
        model.cells[cell] = Leaf(points, [(tuple(index[p] for p in pts), kind) for pts, kind in faces])
        if leaf is None:
            new += 1
            place(model, cell)
        else:
            rewritten += 1
    return rewritten, new, pieces


def place(model: Block, cell: tuple[int, int, int]) -> None:
    """Make room in the tables for a new cell, and put its leaf after the others."""
    x, y, z = cell
    z_base, z_count = model.z
    if z_count == 0:
        model.z = (z, 1)
    elif not z_base <= z < z_base + z_count:
        low, high = min(z_base, z), max(z_base + z_count, z + 1)
        model.z = (low, high - low)
    if z not in model.slabs:
        model.slabs[z] = (y, 1)
        model.slab_order.append(z)
    else:
        y_base, count = model.slabs[z]
        low, high = min(y_base, y), max(y_base + count, y + 1)
        model.slabs[z] = (low, high - low)
    if (z, y) not in model.rows:
        model.rows[(z, y)] = (x, 1)
        model.row_order.append((z, y))
    else:
        x_base, count = model.rows[(z, y)]
        low, high = min(x_base, x), max(x_base + count, x + 1)
        model.rows[(z, y)] = (low, high - low)
    model.leaf_order.append(cell)


def hero_group(triangles: list[tuple[tuple, tuple, tuple]]) -> HeroGroup:
    """A hero group from triangles (model units, the order to store), with its bounding sphere."""
    points = list(dict.fromkeys(p for t in triangles for p in t))
    index = {p: i for i, p in enumerate(points)}
    low = [min(p[i] for p in points) for i in range(3)]
    high = [max(p[i] for p in points) for i in range(3)]
    centre = [(low[i] + high[i]) // 2 for i in range(3)]
    radius = max(math.isqrt(sum((p[i] - centre[i]) ** 2 for i in range(3))) + 1 for p in points)
    return HeroGroup((*centre, min(radius, 0xffff)), points, [tuple(index[p] for p in t) for t in triangles])
