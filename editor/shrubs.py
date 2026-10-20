"""Shrubs (small static meshes): class meshes and PAL placements.

Layouts follow Wrench's shrub reader and recovery (games/rac1/docs/ASSETS.md).
"""

import math
import struct

from formats import FormatError, span, unpack
import games
from mesh import Mesh
from terrain import STCYCL, STMOD, program
from ties import placement

GIF_REGS = 0x412  # ST, RGBAQ, XYZ2 per vertex.


def shrub_mesh(data: bytes, remap: bytes, name: str) -> Mesh:
    """Replay each packet's GS writes in address order.

    GS qword costs: a GIF tag takes 1, a material (AD data) 5 and a vertex
    3. Short packets repeat their last vertex as padding. Winding is not
    stored, so each face faces the average of its vertices' stored normals.
    """
    header = span(data, 0, 64)
    scale, = unpack("<f", header, 0x20)
    count, = unpack("<h", header, 0x28)
    if not math.isfinite(scale) or scale <= 0 or count < 0:
        raise FormatError("invalid shrub scale or packet count")
    table = span(data, 64, count * 8)
    normal_table = list(struct.iter_unpack("<4h", span(data, unpack("<I", header, 0x2c)[0], 24 * 8)))
    mesh, normals = Mesh(name, light_slots=[]), []
    for packet_id, (offset, size) in enumerate(struct.iter_unpack("<ii", table)):
        p = program(span(data, offset, size), [STCYCL, STMOD, 0x6c, STMOD, 0x6d, STMOD, 0x6d])
        if (p[0].immediate, p[1].immediate, p[3].immediate, p[5].immediate) != (0x404, 0, 0, 0):
            raise FormatError("unsupported shrub VIF cycle or mode")
        control, xyz, st = p[2], p[4], p[6]
        materials, tag_count, vertex_count, vertex_offset = unpack("<4i", control.data)
        if min(materials, tag_count, vertex_count) <= 0:
            raise FormatError("invalid shrub packet counts")
        if (len(control.data) != 16 + tag_count * 16 + materials * 64
                or vertex_offset != len(control.data) // 16
                or xyz.count != vertex_count or st.count != vertex_count
                or (control.immediate, xyz.immediate, st.immediate)
                != (0x8000, 0x8000 + vertex_offset, 0x8000 + vertex_offset + vertex_count)
                or vertex_offset + 2 * vertex_count > 1024):
            raise FormatError("shrub VIF counts and addresses disagree")
        tags = [unpack("<QII", control.data, 16 + i * 16) for i in range(tag_count)]
        records = [span(control.data, 16 + tag_count * 16 + i * 64, 64) for i in range(materials)]
        positions = list(struct.iter_unpack("<4h", xyz.data))
        coords = list(struct.iter_unpack("<4h", st.data))
        ti = mi = vi = address = 0
        texture, primitive, expected, kind = None, [], 0, None

        def finish():
            if len(primitive) != expected:
                raise FormatError("shrub GIF vertex count mismatch")
            if not primitive:
                return
            if kind == 3 and len(primitive) % 3:
                raise FormatError("incomplete shrub triangle list")
            for i in range(0, len(primitive) - 2, 3 if kind == 3 else 1):
                a, b, c = primitive[i:i + 3]
                pa, pb, pc = (mesh.positions[v] for v in (a, b, c))
                ab = [pb[j] - pa[j] for j in range(3)]
                ac = [pc[j] - pa[j] for j in range(3)]
                cross = (ab[1] * ac[2] - ab[2] * ac[1], ab[2] * ac[0] - ab[0] * ac[2],
                         ab[0] * ac[1] - ab[1] * ac[0])
                if sum(x * x for x in cross) < 1e-24:
                    continue  # Zero-area faces are strip joins.
                dot = sum(cross[j] * (normals[a][j] + normals[b][j] + normals[c][j]) for j in range(3))
                mesh.add_face(texture, (c, b, a) if dot < 0 else (a, b, c))

        while ti < tag_count or mi < materials or vi < vertex_count:
            if ti < tag_count and tags[ti][2] == address:
                finish()
                tag, regs, _ = tags[ti]
                kind = (tag >> 47) & 7  # GS PRIM: 3 triangles, 4 triangle strip.
                if kind not in (3, 4) or (tag >> 58) & 3 or tag >> 60 != 3 or regs != GIF_REGS:
                    raise FormatError("unsupported shrub GIF primitive")
                expected = tag & 0x7fff
                if expected < 3:
                    raise FormatError("short shrub GIF primitive")
                primitive = []
                ti += 1
                address += 1
            elif mi < materials and unpack("<i", records[mi], 12)[0] == address:
                finish()
                primitive, expected = [], 0
                slot, = unpack("<I", records[mi], 48)  # TEX0 data, patched at load time.
                if slot >= len(remap) or remap[slot] == 255:
                    raise FormatError("shrub material has no texture")
                texture = ("shrub", remap[slot])
                mi += 1
                address += 5
            elif vi < vertex_count and positions[vi][3] == address:
                if not expected or texture is None:
                    raise FormatError("shrub vertex precedes its GIF tag or material")
                x, y, z, _ = positions[vi]
                s, t, q, n = coords[vi]
                if n & 0x7fff >= 24 or q != 4096:
                    raise FormatError("invalid shrub normal or texture Q")
                primitive.append(len(mesh.positions))
                mesh.positions.append((x * scale / 1024, y * scale / 1024, z * scale / 1024))
                mesh.uvs.append((s / 4096, t / 4096))
                normals.append(normal_table[n & 0x7fff][:3])
                mesh.light_slots.append(n & 0x7fff)  # which of the instance's 24 lit colours
                vi += 1
                address += 3
            elif (vi and ti == tag_count and mi == materials
                  and all(pos == positions[vi - 1] and uv[:3] == coords[vi - 1][:3]
                          and uv[3] & 0x7fff == coords[vi - 1][3] & 0x7fff
                          for pos, uv in zip(positions[vi:], coords[vi:]))):
                vi = vertex_count  # Padding: copies of the last vertex.
            else:
                raise FormatError(f"shrub packet {packet_id}: missing GS event at {address}")
        finish()
    return mesh


def shrub_classes(level) -> dict[int, Mesh]:
    """Class ID -> mesh, from 48-byte shrub class entries (billboards not read)."""
    result = {}
    for entry in level.table(0x28, 48):
        start, class_id = unpack("<II", entry)
        if class_id in result or not start:
            raise FormatError(f"duplicate or empty shrub class {class_id}")
        try:
            result[class_id] = shrub_mesh(level.block(start), entry[16:32], f"shrub_{class_id}")
        except FormatError as exc:
            if not games.of(level).lenient:
                raise FormatError(f"shrub class {class_id}: {exc}") from exc
            # A later game's class that does not decode is left out, and its instances with it.
            level.problems.append(f"shrub class {class_id}: not extracted ({exc})")
    return result


def shrub_class_normals(level) -> dict[int, list[tuple[int, int, int]]]:
    """Class ID -> its 24 normals (s16 x, y, z; unit length in 1/32768), at the header's 0x2C."""
    result = {}
    for entry in level.table(0x28, 48):
        start, class_id = unpack("<II", entry)
        data = level.block(start)
        at, = unpack("<I", data, 0x2c)
        result[class_id] = [unpack("<3h", data, at + 8 * j) for j in range(24)]
    return result


def shrub_instances(gameplay: bytes, classes, fmt: games.Format = games.RAC1,
                    problems: list | None = None) -> list[dict]:
    """Shrub instances: the gameplay header word fmt.shrubs (Ratchet & Clank's +0x3c) points to a
    count, then 0x70-byte records, the same in all four games. Deadlocked's are in its art
    instances file (level.Level.instances). PROBLEMS: as ties.tie_instances."""
    start, = unpack("<I", gameplay, fmt.shrubs)
    if not start:
        return []
    count, = unpack("<I", gameplay, start)
    table = span(gameplay, start + 16, count * 0x70)
    result = []
    for i in range(count):
        record = table[i * 0x70:(i + 1) * 0x70]
        class_id, draw_distance = unpack("<if", record)
        if class_id not in classes:
            if problems is None:
                raise FormatError(f"shrub instance {i}: unknown class {class_id}")
            problems.append(f"shrub instance {i}: left out with its class {class_id}")
            continue
        if not math.isfinite(draw_distance):
            raise FormatError(f"shrub instance {i}: invalid draw distance")
        matrix, stored_w = placement(record, "shrub instance")
        result.append({"index": i, "class_id": class_id, "matrix": matrix, "stored_w": stored_w,
                       "draw_distance": draw_distance, "colour": list(unpack("<3i", record, 0x50)),
                       "directional_lights": unpack("<i", record, 0x60)[0]})
    return result
