"""Static terrain (tfrags) at LOD 0 or 2, read without running VIF or VU code.

Layouts follow Wrench's tfrag reader (games/rac1/docs/ASSETS.md); the stream
selection is confirmed by func_002352C8. Only the packet programs seen
on this disc are accepted; anything else raises FormatError.
"""

from dataclasses import dataclass
import math
import struct

from formats import FormatError, span, unpack
import lighting
from mesh import Mesh

FRAGMENT_SIZE = 0x40
# VIF UNPACK element sizes: V3-16, V4-32, V4-16, V4-8.
UNPACK_SIZES = {0x69: 6, 0x6c: 16, 0x6d: 8, 0x6e: 4}
STROW, STCYCL, STMOD = 0x30, 0x01, 0x05
INFO_ROW = 0x45000000  # STROW for vertex infos: float 2048.0 in the first two lanes.


@dataclass(frozen=True)
class Packet:
    command: int
    immediate: int
    count: int
    data: bytes


def packets(data: bytes) -> list[Packet]:
    """Frame a VIF command list, keeping UNPACK payloads packed."""
    result, pos = [], 0
    while pos < len(data):
        word, = unpack("<I", data, pos)
        command = word >> 24  # Masked or interrupting variants are unsupported.
        count = (word >> 16) & 0xff or 256
        if command in UNPACK_SIZES:
            size = UNPACK_SIZES[command] * count
        elif command == STROW:
            size = 16
        elif command in (0, STCYCL, STMOD):
            size = 0
        else:
            raise FormatError(f"unsupported terrain VIF command {command:#x}")
        padded = (size + 3) & ~3
        result.append(Packet(command, word & 0xffff, count, span(data, pos + 4, padded)[:size]))
        pos += 4 + padded
    return result


def program(data: bytes, commands: list[int]) -> list[Packet]:
    """The packets of a command list, which must match commands (NOPs dropped)."""
    parsed = [p for p in packets(data) if p.command]
    if [p.command for p in parsed] != commands:
        raise FormatError("unsupported VIF program")
    return parsed


def strip_faces(indices: bytes, strips: bytes, materials: int) -> list[tuple[int, tuple[int, int, int]]]:
    """Faces and material slots from the strip descriptors (Wrench's recover_faces).

    Each descriptor is (count, packet end, material offset, pad) as signed
    bytes; count 0 ends the list and a negative count switches material.
    Even strips are adjacent quads, split along the strip diagonal.
    """
    result, cursor, material = [], 0, None
    for encoded, _end, ad_offset, _pad in struct.iter_unpack("<4b", strips):
        if encoded == 0:
            break
        count = encoded & 0x7f
        if count < 3 or (count > 3 and count % 2):
            raise FormatError(f"unsupported terrain strip length {count}")
        if encoded < 0 and ad_offset >= 0:
            if ad_offset % 5:
                raise FormatError("terrain material offset is not a five-qword record")
            material = ad_offset // 5
        if material is None or not 0 <= material < materials:
            raise FormatError("terrain strip has no valid material")
        strip = span(indices, cursor, count)
        cursor += count
        if count == 3:
            result.append((material, tuple(strip)))
        else:
            for i in range(count - 2):
                a, b, c = strip[i:i + 3]
                result.append((material, (c, b, a) if i % 2 == 0 else (b, c, a)))
    else:
        raise FormatError("terrain strip list lacks a terminator")
    if len(indices) - cursor > 3:
        raise FormatError("terrain strip list leaves more than alignment padding")
    return result


def refinement(data, start, header, vu, origin, positions, infos):
    """Add the LOD-1 and LOD-0 stages' vertices; return the finest strips.

    Each stage adds absolute positions with two earlier parents each, and
    vertex infos for them plus texture seams. Optional migration arrays
    (for LOD morphing) are checked for placement but not interpreted.
    """
    shared, high, begin = unpack("<3H", header, 0x16)
    end, = unpack("<H", header, 0x1e)
    middle = shared + header[0x22] * 16
    if not high <= begin < middle < end or begin + header[0x23] * 16 != end:
        raise FormatError("invalid terrain refinement boundaries")
    indices = strips = None
    for stage, raw in enumerate((span(data, start + begin, middle - begin),
                                 span(data, start + middle, end - middle))):
        p = [x for x in packets(raw) if x.command]
        count = vu[2 + stage * 2]
        cursor = 0

        def take(command, immediate=None, row=None):
            nonlocal cursor
            if cursor >= len(p):
                raise FormatError("truncated terrain refinement program")
            packet = p[cursor]
            cursor += 1
            if packet.command != command or immediate not in (None, packet.immediate):
                raise FormatError("unsupported terrain refinement command or address")
            if row is not None and unpack("<4i", packet.data) != row:
                raise FormatError("unsupported terrain refinement origin or base")
            return packet

        index_row = (vu[7],) * 4
        info_row = (INFO_ROW, INFO_ROW, vu[6], vu[6])
        position_base, info_base = len(positions), len(infos)
        pos = info = parents = None
        if stage == 1:
            if count:
                pos = take(0x69, 0x8000 | (vu[6] + position_base * 2))
            take(STMOD, 0)
            take(STCYCL, 0x404)
            strips = take(0x6e, 0x8000 | vu[18]).data
            take(STROW, row=index_row)
            take(STMOD, 1)
            indices = take(0x6e, 0xc000 | vu[13]).data
        elif count:
            take(STROW, row=index_row)
            take(STMOD, 1)
        if count:
            parents = take(0x6e, 0xc000 | vu[14 + stage * 2]).data
            if not 0 <= len(parents) - count < 4:
                raise FormatError("terrain parent array count mismatch")
            if cursor < len(p) and p[cursor].command == 0x6e:
                take(0x6e, 0xc000 | vu[15 + stage * 2])  # Migration data.
            take(STROW, row=info_row)
            info = take(0x6d, 0x8000 | vu[9 + stage * 2])
            if vu[9 + stage * 2] != vu[7] + info_base:
                raise FormatError("non-contiguous terrain vertex-info array")
        if stage == 0:
            take(STROW, row=(*origin, 0))
            take(STCYCL, 0x102)
            if count:
                pos = take(0x69, 0x8000 | (vu[6] + position_base * 2))
        else:
            take(STMOD, 0)
        if cursor != len(p):
            raise FormatError("unexpected terrain refinement packets")
        if not count:
            continue
        if pos.count != count or info.count < count:
            raise FormatError("terrain refinement vertex count mismatch")
        positions.extend(struct.iter_unpack("<3h", pos.data))
        added = list(struct.iter_unpack("<4h", info.data))
        infos.extend(added)
        # The first infos cover the new positions once each, in any order.
        if {e[3] for e in added[:count]} != set(range(position_base * 2, len(positions) * 2, 2)):
            raise FormatError("terrain refinement position coverage mismatch")
        for i, (_, _, parent, _) in enumerate(added):
            if parent < 0 or parent % 2 or parent // 2 >= len(positions):
                raise FormatError("invalid terrain refinement parent position")
            if i < count and (parents[i] >= info_base or infos[parents[i]][3] // 2 >= position_base
                              or parent // 2 >= position_base):
                raise FormatError("invalid terrain refinement parent relationship")
    return indices, strips


def fragment(data: bytes, table_offset: int, header: bytes, lod: int, name: str, lights=None) -> Mesh:
    """One tfrag: common vertices, then LOD-2 strips or both refinements. With `lights`
    (the level's light bank and the normal table, lighting.py), each vertex gets the colour
    the game lights it with at level load, as RGBA / 128 (0x80 = 1.0 under MODULATE)."""
    start = table_offset + unpack("<I", header, 0x10)[0]
    low, shared, high = unpack("<3H", header, 0x14)
    if not low < shared < high:
        raise FormatError("invalid terrain program boundaries")
    coarse = program(span(data, start + low, shared - low), [STROW, STMOD, 0x6e, STMOD, 0x6e])
    common = program(span(data, start + shared, high - shared),
                     [0x6d, 0x6c, STROW, STMOD, 0x6d, STROW, STCYCL, 0x69, STCYCL, STMOD])
    if (coarse[1].immediate, coarse[3].immediate, common[3].immediate,
            common[6].immediate, common[8].immediate, common[9].immediate) != (1, 0, 1, 0x102, 0x404, 0):
        raise FormatError("unsupported terrain VIF cycle or mode")
    for packet, unsigned in ((coarse[2], True), (coarse[4], False), (common[0], True),
                             (common[1], False), (common[4], False), (common[7], False)):
        if packet.immediate & 0x3c00 or bool(packet.immediate & 0x4000) != unsigned:
            raise FormatError("unsupported terrain UNPACK flags")
        if not packet.immediate & 0x8000:
            raise FormatError("terrain UNPACK is not TOPS-relative")
    if len(common[0].data) != 40:
        raise FormatError("unexpected terrain VU header size")
    vu = unpack("<20H", common[0].data)  # VU memory map (Wrench's TfragHeaderUnpack).
    if common[7].count != vu[0]:
        raise FormatError("terrain common position count mismatch")
    if (common[7].immediate & 0x3ff, common[4].immediate & 0x3ff, coarse[2].immediate & 0x3ff,
            coarse[4].immediate & 0x3ff) != (vu[6], vu[7], vu[13], vu[18]):
        raise FormatError("terrain VU addresses disagree with the header")
    if unpack("<4i", coarse[0].data) != (vu[7],) * 4:
        raise FormatError("unsupported terrain index base")
    if unpack("<4I", common[2].data) != (INFO_ROW, INFO_ROW, 0, vu[6]):
        raise FormatError("unsupported terrain vertex-info base")
    materials = header[0x28]
    if len(common[1].data) != materials * 80:
        raise FormatError("terrain texture primitive count mismatch")
    # Five-qword GS primitives; the first word is the texture index until
    # func_00204340 patches it at load time.
    textures = [unpack("<I", common[1].data, i * 80)[0] for i in range(materials)]
    positions = list(struct.iter_unpack("<3h", common[7].data))
    origin = unpack("<4i", common[5].data)[:3]
    infos = list(struct.iter_unpack("<4h", common[4].data))
    if any(parent != 0x1000 or position < 0 or position % 2 or position // 2 >= len(positions)
           for _, _, parent, position in infos):
        raise FormatError("invalid terrain common vertex info")
    indices, strips = coarse[2].data, coarse[4].data
    if lod == 0:
        indices, strips = refinement(data, start, header, vu, origin, positions, infos)
    sphere = unpack("<4f", header)
    if not all(math.isfinite(x) for x in sphere) or sphere[3] < 0:
        raise FormatError("invalid terrain bounding sphere")
    mesh = Mesh(name)
    colours = None
    if lights is not None:
        # One light record per position (common, LOD-01, LOD-0), after the origin quadword.
        light_offset, = unpack("<H", header, 0x30)
        records = span(data, start + light_offset + 0x10, header[0x3c] * 8)
        single = struct.unpack("<b", header[0x34:0x35])[0]
        colours = lighting.light_records(records, single, *lights)
        if len(colours) < len(positions):
            raise FormatError("terrain light records do not cover the positions")
        mesh.colours = []
    for s, t, _parent, position in infos:
        if position < 0 or position % 2 or position // 2 >= len(positions):
            raise FormatError("invalid terrain vertex reference")
        mesh.positions.append(tuple((origin[j] + positions[position // 2][j]) / 1024 for j in range(3)))
        # Negative coordinates are halved, as in Wrench; not yet traced in VU code.
        mesh.uvs.append((s / (8192 if s < 0 else 4096), t / (8192 if t < 0 else 4096)))
        if colours is not None:
            mesh.colours.append(tuple(c / 128 for c in colours[position // 2]))
    for material, face in strip_faces(indices, strips, materials):
        if max(face) >= len(mesh.positions):
            raise FormatError("terrain face index outside the vertex infos")
        mesh.add_face(("terrain", textures[material]), face)
    return mesh


def terrain(data: bytes, lod: int = 0, lights=None, problems: list | None = None) -> list[Mesh]:
    """Every fragment of a tfrag block at one static LOD, as Terrain_NNN meshes (lit when
    `lights` is given, as fragment() says). PROBLEMS (a later game's level.problems): a fragment
    that does not decode is left out and noted there instead of stopping the level."""
    if lod not in (0, 2):
        raise FormatError("terrain LOD must be 0 or 2")
    table_offset, count = unpack("<II", data)
    table = span(data, table_offset, count * FRAGMENT_SIZE)
    meshes = []
    for i in range(count):
        try:
            header = table[i * FRAGMENT_SIZE:(i + 1) * FRAGMENT_SIZE]
            meshes.append(fragment(data, table_offset, header, lod, f"Terrain_{i:03}", lights))
        except FormatError as exc:
            if problems is None:
                raise FormatError(f"terrain fragment {i}: {exc}") from exc
            problems.append(f"terrain fragment {i}: not extracted ({exc})")
    return meshes
