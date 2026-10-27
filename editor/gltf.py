"""A small glTF 2.0 binary (GLB) writer for extracted meshes.

Positions stay in game axes; the Godot scene rotates the whole level
once. Textures are referenced by relative URI so that meshes share one
set of PNGs, which Godot imports as ordinary textures.
"""

import json
import math
import struct

from mesh import Mesh

FLOAT, SHORT, UNSIGNED_BYTE, UNSIGNED_INT = 5126, 5122, 5121, 5125
ARRAY_BUFFER, ELEMENT_ARRAY_BUFFER = 34962, 34963
KINDS = {1: "SCALAR", 2: "VEC2", 3: "VEC3", 4: "VEC4", 16: "MAT4"}
REPEAT, CLAMP_TO_EDGE, LINEAR, LINEAR_MIPMAP_LINEAR = 10497, 33071, 9729, 9987


def face_normal(a, b, c) -> tuple[float, float, float]:
    ab = [b[i] - a[i] for i in range(3)]
    ac = [c[i] - a[i] for i in range(3)]
    n = (ab[1] * ac[2] - ab[2] * ac[1], ab[2] * ac[0] - ab[0] * ac[2], ab[0] * ac[1] - ab[1] * ac[0])
    length = math.sqrt(sum(x * x for x in n))
    return tuple(x / length for x in n) if length > 1e-20 else (0.0, 0.0, 1.0)


class Gltf:
    def __init__(self):
        self.buffer = bytearray()
        self.doc = {"asset": {"version": "2.0", "generator": "OpenRAC editor"},
                    "scene": 0, "scenes": [{"nodes": []}], "nodes": [], "meshes": [],
                    "materials": [], "textures": [], "images": [], "accessors": [], "bufferViews": [],
                    "samplers": [{"magFilter": LINEAR, "minFilter": LINEAR_MIPMAP_LINEAR,
                                  "wrapS": REPEAT, "wrapT": REPEAT}]}
        self.materials: dict = {}

    def view(self, payload: bytes, target: int | None) -> int:
        """A buffer view of payload, 4-byte aligned; target is for vertex and index data."""
        self.buffer.extend(bytes(-len(self.buffer) % 4))
        view = {"buffer": 0, "byteOffset": len(self.buffer), "byteLength": len(payload)}
        if target is not None:
            view["target"] = target
        self.doc["bufferViews"].append(view)
        self.buffer.extend(payload)
        return len(self.doc["bufferViews"]) - 1

    def floats(self, rows: list[tuple], bounds: bool = False, target: int | None = ARRAY_BUFFER) -> int:
        """An accessor for float scalars, vectors or matrices; bounds are those
        of the stored float32 values."""
        width = len(rows[0])
        payload = struct.pack(f"<{len(rows) * width}f", *(x for row in rows for x in row))
        accessor = {"bufferView": self.view(payload, target), "componentType": FLOAT,
                    "count": len(rows), "type": KINDS[width]}
        if bounds:
            stored = list(struct.iter_unpack(f"<{width}f", payload))
            accessor["min"] = [min(r[i] for r in stored) for i in range(width)]
            accessor["max"] = [max(r[i] for r in stored) for i in range(width)]
        self.doc["accessors"].append(accessor)
        return len(self.doc["accessors"]) - 1

    def sampler(self, clamp: int) -> int:
        """The sampler that clamps the axes of clamp (bit 0 S, bit 1 T) and repeats the others;
        sampler 0 repeats both."""
        wrap = {"wrapS": CLAMP_TO_EDGE if clamp & 1 else REPEAT, "wrapT": CLAMP_TO_EDGE if clamp & 2 else REPEAT}
        samplers = self.doc["samplers"]
        for i, sampler in enumerate(samplers):
            if sampler["wrapS"] == wrap["wrapS"] and sampler["wrapT"] == wrap["wrapT"]:
                return i
        samplers.append({**samplers[0], **wrap})
        return len(samplers) - 1

    def material(self, name: str, uri: str, *, cutout: bool = False, clamp: int = 0) -> int:
        """A double-sided diffuse material; cutout uses glTF's MASK mode, clamp the axes the
        texture is clamped on (bit 0 S, bit 1 T), as the game's GS CLAMP setting."""
        key = (name, uri, cutout, clamp)
        if key not in self.materials:
            self.doc["images"].append({"uri": uri})
            self.doc["textures"].append({"source": len(self.doc["images"]) - 1, "sampler": self.sampler(clamp)})
            pbr = {"baseColorTexture": {"index": len(self.doc["textures"]) - 1},
                   "metallicFactor": 0, "roughnessFactor": 1}
            material = {"name": name, "doubleSided": True, "pbrMetallicRoughness": pbr}
            if cutout:
                material.update(alphaMode="MASK", alphaCutoff=0.5)
            self.doc["materials"].append(material)
            self.materials[key] = len(self.doc["materials"]) - 1
        return self.materials[key]

    def indices(self, values: list[int]) -> int:
        """An accessor for triangle vertex indices."""
        view = self.view(struct.pack(f"<{len(values)}I", *values), ELEMENT_ARRAY_BUFFER)
        self.doc["accessors"].append({"bufferView": view, "componentType": UNSIGNED_INT,
                                      "count": len(values), "type": "SCALAR"})
        return len(self.doc["accessors"]) - 1

    def skin_attributes(self, skins: list[tuple]) -> dict:
        """JOINTS_0 and WEIGHTS_0 from up to four (joint, weight in 256ths) pairs per vertex."""
        joints, weights = [], []
        for pairs in skins:
            if len(pairs) > 4 or any(not 0 <= j < 256 for j, _ in pairs):
                raise ValueError("unsupported skin influence")
            padded = list(pairs) + [(0, 0)] * (4 - len(pairs))
            joints.extend(j for j, _ in padded)
            weights.append(tuple(w / 256 for _, w in padded))
        view = self.view(bytes(joints), ARRAY_BUFFER)
        self.doc["accessors"].append({"bufferView": view, "componentType": UNSIGNED_BYTE,
                                      "count": len(skins), "type": "VEC4"})
        return {"JOINTS_0": len(self.doc["accessors"]) - 1, "WEIGHTS_0": self.floats(weights)}

    def mesh(self, mesh: Mesh, materials: dict) -> int:
        """One primitive per texture. Without vertex normals it is unindexed,
        so each face keeps a flat normal; with them it is indexed.

        materials maps each of the mesh's texture keys to a material index.
        """
        primitives = []
        for key, faces in mesh.faces.items():
            corners = [v for face in faces for v in face]
            if mesh.normals is not None:
                used = list(dict.fromkeys(corners))  # In order of first use.
                local = {v: i for i, v in enumerate(used)}
                attributes = {"POSITION": self.floats([mesh.positions[v] for v in used], bounds=True),
                              "NORMAL": self.floats([mesh.normals[v] for v in used]),
                              "TEXCOORD_0": self.floats([mesh.uvs[v] for v in used])}
                if mesh.skins is not None:
                    attributes.update(self.skin_attributes([mesh.skins[v] for v in used]))
                if mesh.colours is not None:
                    attributes["COLOR_0"] = self.floats([mesh.colours[v] for v in used])
                if mesh.light_slots is not None:
                    attributes["_LIGHT_SLOT"] = self.floats([(mesh.light_slots[v],) for v in used])
                primitives.append({"attributes": attributes, "indices": self.indices([local[v] for v in corners]),
                                   "material": materials[key], "mode": 4})
                continue
            flat = [face_normal(*(mesh.positions[v] for v in face)) for face in faces]
            attributes = {"POSITION": self.floats([mesh.positions[v] for v in corners], bounds=True),
                          "NORMAL": self.floats([n for n in flat for _ in range(3)]),
                          "TEXCOORD_0": self.floats([mesh.uvs[v] for v in corners])}
            if mesh.colours is not None:
                attributes["COLOR_0"] = self.floats([mesh.colours[v] for v in corners])
            if mesh.light_slots is not None:
                attributes["_LIGHT_SLOT"] = self.floats([(mesh.light_slots[v],) for v in corners])
            primitives.append({"attributes": attributes, "material": materials[key], "mode": 4})
        self.doc["meshes"].append({"name": mesh.name, "primitives": primitives})
        return len(self.doc["meshes"]) - 1

    def overlay(self, name: str, alpha: float) -> int:
        """An unshaded, double-sided, translucent material that shows vertex colours."""
        self.doc["extensionsUsed"] = ["KHR_materials_unlit"]
        self.doc["materials"].append({
            "name": name, "doubleSided": True, "alphaMode": "BLEND",
            "pbrMetallicRoughness": {"baseColorFactor": [1, 1, 1, alpha], "metallicFactor": 0},
            "extensions": {"KHR_materials_unlit": {}}})
        return len(self.doc["materials"]) - 1

    def coloured(self, mesh: Mesh, material: int) -> int:
        """One unindexed primitive with a colour per vertex (mesh.colours), no textures."""
        corners = [v for faces in mesh.faces.values() for face in faces for v in face]
        attributes = {"POSITION": self.floats([mesh.positions[v] for v in corners], bounds=True),
                      "COLOR_0": self.floats([mesh.colours[v] for v in corners])}
        self.doc["meshes"].append({"name": mesh.name, "primitives": [
            {"attributes": attributes, "material": material, "mode": 4}]})
        return len(self.doc["meshes"]) - 1

    def node(self, name: str, mesh: int | None = None, *, root: bool = True, **fields) -> int:
        """A node; fields are glTF node properties (skin, translation, children...)."""
        if root:
            self.doc["scenes"][0]["nodes"].append(len(self.doc["nodes"]))
        self.doc["nodes"].append({"name": name, **({"mesh": mesh} if mesh is not None else {}), **fields})
        return len(self.doc["nodes"]) - 1

    def skin(self, joints: list[int], inverse_binds: list[list[float]]) -> int:
        """A skin over joint nodes; inverse bind matrices are column-major."""
        self.doc.setdefault("skins", []).append(
            {"inverseBindMatrices": self.floats(inverse_binds, target=None), "joints": joints})
        return len(self.doc["skins"]) - 1

    def animation(self, name: str, times: list[float], channels: list[tuple[int, str, list[tuple]]]) -> None:
        """Linear channels (node, path, one value per time) sharing key times
        in seconds. A channel that never changes gets a single key. Rotations
        are normalized shorts, which glTF allows for them. Everything goes in
        one buffer view."""
        payload, accessors = bytearray(), self.doc["accessors"]
        view = len(self.doc["bufferViews"])

        def add(data: bytes, component: int, count: int, kind: str, **extra) -> int:
            payload.extend(bytes(-len(payload) % 4))
            accessors.append({"bufferView": view, "byteOffset": len(payload), "componentType": component,
                              "count": count, "type": kind, **extra})
            payload.extend(data)
            return len(accessors) - 1

        stored = list(struct.unpack(f"<{len(times)}f", struct.pack(f"<{len(times)}f", *times)))
        inputs = {len(times): add(struct.pack(f"<{len(times)}f", *times), FLOAT, len(times), "SCALAR",
                                  min=[stored[0]], max=[stored[-1]])}
        samplers = []
        for _, path, values in channels:
            if all(v == values[0] for v in values):
                values = values[:1]
            if len(values) not in inputs:
                inputs[len(values)] = add(struct.pack("<f", 0.0), FLOAT, 1, "SCALAR", min=[0.0], max=[0.0])
            if path == "rotation":
                shorts = [max(-32767, min(32767, round(c * 32767))) for row in values for c in row]
                output = add(struct.pack(f"<{len(shorts)}h", *shorts), SHORT, len(values), "VEC4", normalized=True)
            else:
                output = add(struct.pack(f"<{len(values) * 3}f", *(c for row in values for c in row)), FLOAT,
                             len(values), "VEC3")
            samplers.append({"input": inputs[len(values)], "interpolation": "LINEAR", "output": output})
        self.view(bytes(payload), None)
        self.doc.setdefault("animations", []).append(
            {"name": name, "samplers": samplers,
             "channels": [{"sampler": i, "target": {"node": node, "path": path}}
                          for i, (node, path, _) in enumerate(channels)]})

    def glb(self) -> bytes:
        self.buffer.extend(bytes(-len(self.buffer) % 4))
        doc = {k: v for k, v in self.doc.items() if v != []}  # glTF arrays must not be empty.
        doc["buffers"] = [{"byteLength": len(self.buffer)}]
        text = json.dumps(doc, separators=(",", ":"), allow_nan=False).encode()
        text += b" " * (-len(text) % 4)
        return (struct.pack("<3I", 0x46546C67, 2, 28 + len(text) + len(self.buffer))
                + struct.pack("<I4s", len(text), b"JSON") + text
                + struct.pack("<I4s", len(self.buffer), b"BIN\0") + bytes(self.buffer))
