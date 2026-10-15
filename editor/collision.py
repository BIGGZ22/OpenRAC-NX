"""Static collision: the level's baked world mesh, as triangles with surface types.

The layout follows ReRAC's notes on the format (games/rac1/docs/ASSETS.md, "Collision").
A three-level tree (Z, then Y, then X) of 4-unit cells leads to leaves; each
leaf has its own packed vertices and the faces that touch the cell, so a face
that spans several cells is stored once per cell and is merged here.
"""

from dataclasses import dataclass, field
import colorsys

from formats import FormatError, span, unpack
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
