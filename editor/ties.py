"""Ties (static instanced meshes): LOD-0 class meshes and PAL placements.

Layouts follow Wrench's tie reader (games/rac1/docs/ASSETS.md); func_00236A98
confirms the GIF table pointer, material count and 80-byte stride.
"""

import math
import struct

from formats import FormatError, span, unpack
from mesh import Mesh


def tie_mesh(data: bytes, remap: bytes, name: str) -> Mesh:
    """Recover LOD 0 by replaying each packet's GS writes in address order.

    GS qword costs: a material (AD GIF) takes 6, a strip's GIF tag 1 and
    a vertex 3. A vertex may be written to two addresses. Missing or
    conflicting writes raise instead of being guessed.
    """
    header = span(data, 0, 0x70)
    packet_table, = unpack("<I", header)
    scale, = unpack("<f", header, 0x40)
    if not math.isfinite(scale) or scale <= 0:
        raise FormatError("invalid tie scale")
    material_count = header[0x23]
    if not 0 < material_count <= 16:
        raise FormatError("invalid tie material count")
    textures = list(remap[:material_count])
    if 255 in textures:
        raise FormatError("tie material has no texture")
    span(data, unpack("<I", header, 0x2c)[0], material_count * 80)
    table = span(data, packet_table, header[0x20] * 16)
    mesh = Mesh(name, light_slots=[])
    for packet_id in range(header[0x20]):
        entry = table[packet_id * 16:(packet_id + 1) * 16]
        start = packet_table + unpack("<I", entry)[0]
        control = span(data, start, entry[8] * 16)
        span(control, 0, 0x2c)
        destinations = unpack("<4i", control)  # GS addresses of materials 1..3.
        sources = unpack("<4i", control, 16)   # Their AD GIF offsets (80-byte records).
        strips = list(struct.iter_unpack("<4B", span(control, 0x2c, control[0x23] * 4)))
        packed = span(data, start + entry[8] * 16, entry[9] * 16)
        dinky = control[0x28]  # Regular-vertex qwords plus four.
        if dinky < 4 or dinky % 2:
            raise FormatError("invalid tie vertex encoding")
        regular = span(packed, 0, (dinky - 4) * 8)
        fat = packed[len(regular):]
        if len(fat) % 24 not in (0, 8):
            raise FormatError("misaligned tie extended vertices")
        raw = list(struct.iter_unpack("<3hH3hH", regular))
        for at in range(0, len(fat) - 23, 24):  # An odd count leaves 8 bytes of padding.
            x, y, z = unpack("<3h", fat, at + 8)
            s, t, q, second = unpack("<3hH", fat, at + 16)
            raw.append((x, y, z, unpack("<H", fat, at + 6)[0], s, t, q, second))
        # Each vertex's light slot (ReRAC's tie.rs): one byte per regular vertex, then from the
        # next 4-byte boundary four per extended vertex, the first its own (the other two are
        # what it blends towards as it morphs away, which LOD 0 never does).
        regular_count = (dinky - 4) // 2
        slot_bytes = span(data, start + entry[10] * 16, entry[11] * 4)
        fat_base = (regular_count + 3) // 4 * 4
        slots = [slot_bytes[i] if i < regular_count else slot_bytes[fat_base + 4 * (i - regular_count)]
                 for i in range(len(raw))]
        writes = {}
        for (x, y, z, first, s, t, q, second), slot in zip(raw, slots):
            for address in {first, second} - {0}:
                if writes.setdefault(address, (x, y, z, s, t, q, slot)) != (x, y, z, s, t, q, slot):
                    raise FormatError("conflicting tie vertex writes")
        address, shader, strip_index = 6, 1, 0
        material, primitive, expected, winding = sources[0], [], 0, 0

        def check_material(source):
            if source < 0 or source % 80 or source // 80 >= material_count:
                raise FormatError("invalid tie material source")
            return source // 80

        def finish():
            if len(primitive) != expected:
                raise FormatError("tie strip vertex count mismatch")
            for i in range(2, len(primitive)):
                face = tuple(primitive[i - 2:i + 1])
                mesh.add_face(("tie", textures[material]), face if i % 2 == winding else face[::-1])

        material = check_material(material)
        while writes or strip_index < len(strips):
            if strip_index < len(strips) and strips[strip_index][2] == address:
                if expected:
                    finish()
                expected, pad, _, winding = strips[strip_index]
                if expected < 3 or pad or winding not in (0, 1):
                    raise FormatError("unsupported tie strip descriptor")
                primitive = []
                strip_index += 1
                address += 1
            elif address in writes:
                if not strip_index:
                    raise FormatError("tie vertex precedes its strip")
                x, y, z, s, t, q, slot = writes.pop(address)
                if q != 4096:
                    raise FormatError("unsupported tie perspective texture coordinate")
                primitive.append(len(mesh.positions))
                mesh.positions.append((x * scale / 1024, y * scale / 1024, z * scale / 1024))
                mesh.uvs.append((s / 4096, t / 4096))
                mesh.light_slots.append(slot & 0x3F)
                address += 3
            elif shader < 4 and destinations[shader - 1] == address:
                if expected:
                    finish()
                    primitive, expected = [], 0
                material = check_material(sources[shader])
                shader += 1
                address += 6
            else:
                raise FormatError(f"tie packet {packet_id}: missing GS event at {address}")
        if expected:
            finish()
    return mesh


def tie_classes(level) -> dict[int, Mesh]:
    """Class ID -> mesh, from the core index's 32-byte tie class entries."""
    result = {}
    for entry in level.table(0x20, 32):
        start, class_id = unpack("<II", entry)
        if class_id in result or not start:
            raise FormatError(f"duplicate or empty tie class {class_id}")
        try:
            result[class_id] = tie_mesh(level.block(start), entry[16:32], f"tie_{class_id}")
        except FormatError as exc:
            raise FormatError(f"tie class {class_id}: {exc}") from exc
    return result


def tie_slot_normals(level) -> dict[int, list[tuple[int, int, int]]]:
    """Class ID -> its 64 light-slot normals (s16 x, y, z; unit length in 1/32768), at the
    class header's 0x0C."""
    result = {}
    for entry in level.table(0x20, 32):
        start, class_id = unpack("<II", entry)
        data = level.block(start)
        at, = unpack("<I", data, 0x0C)
        result[class_id] = [unpack("<3h", data, at + 8 * j) for j in range(64)]
    return result


def placement(record: bytes, what: str) -> tuple[list[float], float]:
    """A column-major affine matrix at +0x10, and the W the game stores (0.01)."""
    matrix = list(unpack("<16f", record, 0x10))
    if not all(map(math.isfinite, matrix)) or any(matrix[j] for j in (3, 7, 11)):
        raise FormatError(f"invalid {what} matrix")
    stored_w, matrix[15] = matrix[15], 1.0
    return matrix, stored_w


def tie_instances(gameplay: bytes, classes) -> list[dict]:
    """PAL tie instances: gameplay +0x34 points to a count, then 0xe0-byte records."""
    start, = unpack("<I", gameplay, 0x34)
    if not start:
        return []
    count, = unpack("<I", gameplay, start)
    table = span(gameplay, start + 16, count * 0xe0)
    result = []
    for i in range(count):
        record = table[i * 0xe0:(i + 1) * 0xe0]
        class_id, draw_distance, _, occlusion = unpack("<4i", record)
        if class_id not in classes:
            raise FormatError(f"tie instance {i}: unknown class {class_id}")
        matrix, stored_w = placement(record, "tie instance")
        lights, uid = unpack("<2i", record, 0xd0)
        result.append({"index": i, "class_id": class_id, "matrix": matrix, "stored_w": stored_w,
                       "draw_distance": draw_distance, "occlusion_index": occlusion,
                       "directional_lights": lights, "uid": uid,
                       # One 5:5:5:1 ambient colour per light slot (LightTies' input).
                       "ambient": list(unpack("<64H", record, 0x50))})
    return result
