"""Read glTF 2.0 files (.glb, or .gltf with its .bin and images) as a modder's
tools write them: Blender, Godot, this editor's own writer (gltf.py).

Only what the asset re-import needs is decoded: the JSON, the buffers and
each accessor as rows of numbers. Anything the reader cannot honour (sparse
accessors, Draco or meshopt compression, buffers outside the file's folder)
raises GltfError with a sentence a modder can act on.
"""

from __future__ import annotations

import base64
import json
from pathlib import Path
import struct

COMPONENTS = {5120: ("b", 1), 5121: ("B", 1), 5122: ("h", 2), 5123: ("H", 2), 5125: ("I", 4), 5126: ("f", 4)}
WIDTHS = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT2": 4, "MAT3": 9, "MAT4": 16}
# Normalized integers as floats (glTF 2.0, 3.11): the signed ones clamp at -1.
SCALE = {5120: 127.0, 5121: 255.0, 5122: 32767.0, 5123: 65535.0, 5125: 4294967295.0}
GLB_MAGIC, JSON_CHUNK, BIN_CHUNK = 0x46546C67, 0x4E4F534A, 0x004E4942
UNSUPPORTED = ("KHR_draco_mesh_compression", "EXT_meshopt_compression", "KHR_mesh_quantization")


class GltfError(ValueError):
    """A file this reader cannot use; the message says what to change."""


class Document:
    """A parsed glTF: `doc` is its JSON, `path` where it was read from."""

    def __init__(self, doc: dict, buffers: list[bytes], path: Path):
        self.doc, self.buffers, self.path = doc, buffers, path
        for name in doc.get("extensionsRequired", []):
            if name in UNSUPPORTED:
                raise GltfError(f"{path.name} needs {name}: export it again without compression")

    def list(self, key: str) -> list:
        return self.doc.get(key, [])

    def view_bytes(self, index: int) -> tuple[bytes, int | None]:
        """A buffer view's bytes and its stride."""
        view = self.list("bufferViews")[index]
        buffer = self.buffers[view["buffer"]]
        start = view.get("byteOffset", 0)
        end = start + view["byteLength"]
        if end > len(buffer):
            raise GltfError(f"{self.path.name}: buffer view {index} runs past its buffer")
        return buffer[start:end], view.get("byteStride")

    def accessor(self, index: int, *, normalize: bool = True) -> list[tuple]:
        """Accessor `index` as one tuple per element. Normalized integers become floats
        when `normalize`; other integers stay integers."""
        accessor = self.list("accessors")[index]
        if "sparse" in accessor:
            raise GltfError(f"{self.path.name}: accessor {index} is sparse (shape keys?): apply the shape keys "
                            "or export without them")
        code, size = COMPONENTS[accessor["componentType"]]
        width, count = WIDTHS[accessor["type"]], accessor["count"]
        if "bufferView" not in accessor:
            return [tuple(0 for _ in range(width))] * count
        data, stride = self.view_bytes(accessor["bufferView"])
        offset = accessor.get("byteOffset", 0)
        element = size * width
        if accessor["type"] in ("MAT2", "MAT3") and size < 4:
            raise GltfError(f"{self.path.name}: accessor {index}: padded matrix columns are not supported")
        stride = stride or element
        if count and offset + stride * (count - 1) + element > len(data):
            raise GltfError(f"{self.path.name}: accessor {index} runs past its buffer view")
        rows = [struct.unpack_from(f"<{width}{code}", data, offset + i * stride) for i in range(count)]
        if accessor.get("normalized") and normalize and code != "f":
            scale = SCALE[accessor["componentType"]]
            rows = [tuple(max(-1.0, v / scale) for v in row) for row in rows]
        return rows

    def image_path(self, image: dict) -> Path | None:
        """An image's file beside the document (None for an image inside the file)."""
        uri = image.get("uri")
        if uri is None or uri.startswith("data:"):
            return None
        return resolve(self.path.parent, uri, self.path.name)

    def image_bytes(self, image: dict) -> bytes:
        """An image's bytes, wherever the document keeps them."""
        uri = image.get("uri")
        if uri is None:
            data, _ = self.view_bytes(image["bufferView"])
            return data
        if uri.startswith("data:"):
            return data_uri(uri, self.path.name)
        path = self.image_path(image)
        if not path.is_file():
            raise GltfError(f"{self.path.name}: its image {uri} is not there")
        return path.read_bytes()

    def node_world(self, index: int) -> list[float]:
        """Node `index`'s world matrix (column-major), through its parents."""
        parents = {c: i for i, node in enumerate(self.list("nodes")) for c in node.get("children", [])}
        matrix = local_matrix(self.list("nodes")[index])
        while index in parents:
            index = parents[index]
            matrix = multiply(local_matrix(self.list("nodes")[index]), matrix)
        return matrix


def resolve(folder: Path, uri: str, name: str) -> Path:
    """A relative URI inside or beside `folder`; never an absolute path or another drive."""
    from urllib.parse import unquote
    relative = unquote(uri)
    if relative.startswith(("/", "\\")) or ":" in relative.split("/")[0]:
        raise GltfError(f"{name} refers to {uri}, outside its folder: export with relative paths")
    return (folder / relative).resolve()


def data_uri(uri: str, name: str) -> bytes:
    head, _, payload = uri.partition(",")
    if not head.endswith(";base64"):
        raise GltfError(f"{name}: a data URI that is not base64")
    return base64.b64decode(payload)


def read(path: Path) -> Document:
    """A .glb or .gltf file and the buffers it names."""
    path = Path(path)
    raw = path.read_bytes()
    if raw[:4] == struct.pack("<I", GLB_MAGIC):
        magic, version, length = struct.unpack_from("<3I", raw)
        if version != 2:
            raise GltfError(f"{path.name} is glTF {version}; only glTF 2.0 is read")
        at, doc, binary = 12, None, None
        while at + 8 <= min(length, len(raw)):
            size, kind = struct.unpack_from("<2I", raw, at)
            chunk = raw[at + 8:at + 8 + size]
            if kind == JSON_CHUNK:
                doc = json.loads(chunk.decode("utf-8"))
            elif kind == BIN_CHUNK and binary is None:
                binary = chunk
            at += 8 + size + (-size % 4)
        if doc is None:
            raise GltfError(f"{path.name} has no JSON chunk")
    else:
        try:
            doc, binary = json.loads(raw.decode("utf-8")), None
        except (UnicodeDecodeError, json.JSONDecodeError) as exc:
            raise GltfError(f"{path.name} is neither a .glb nor a .gltf file ({exc})") from None
    if not str(doc.get("asset", {}).get("version", "")).startswith("2"):
        raise GltfError(f"{path.name} is not glTF 2.0")
    buffers = []
    for i, buffer in enumerate(doc.get("buffers", [])):
        uri = buffer.get("uri")
        if uri is None:
            if binary is None:
                raise GltfError(f"{path.name}: buffer {i} has no data")
            data = binary
        elif uri.startswith("data:"):
            data = data_uri(uri, path.name)
        else:
            file = resolve(path.parent, uri, path.name)
            if not file.is_file():
                raise GltfError(f"{path.name} needs {uri} beside it")
            data = file.read_bytes()
        if len(data) < buffer.get("byteLength", 0):
            raise GltfError(f"{path.name}: buffer {i} is shorter than it says")
        buffers.append(data)
    return Document(doc, buffers, path)


# ---- matrices: column-major 4 x 4, as glTF stores them ---------------------------------

IDENTITY = [1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0]


def multiply(a: list[float], b: list[float]) -> list[float]:
    return [sum(a[k * 4 + r] * b[c * 4 + k] for k in range(4)) for c in range(4) for r in range(4)]


def trs(translation=(0, 0, 0), rotation=(0, 0, 0, 1), scale=(1, 1, 1)) -> list[float]:
    x, y, z, w = rotation
    rows = [[1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)]]
    m = [0.0] * 16
    for c in range(3):
        for r in range(3):
            m[c * 4 + r] = rows[r][c] * scale[c]
    m[12:15] = translation
    m[15] = 1.0
    return m


def local_matrix(node: dict) -> list[float]:
    if "matrix" in node:
        return [float(v) for v in node["matrix"]]
    return trs(node.get("translation", (0, 0, 0)), node.get("rotation", (0, 0, 0, 1)), node.get("scale", (1, 1, 1)))


def invert(m: list[float]) -> list[float]:
    """The inverse of an affine matrix (last row 0, 0, 0, 1)."""
    a = [[m[c * 4 + r] for c in range(3)] for r in range(3)]
    det = (a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) - a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0])
           + a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]))
    if abs(det) < 1e-20:
        raise GltfError("a joint or node has a matrix that cannot be inverted (zero scale?)")
    inv = [[(a[(r + 1) % 3][(c + 1) % 3] * a[(r + 2) % 3][(c + 2) % 3]
             - a[(r + 1) % 3][(c + 2) % 3] * a[(r + 2) % 3][(c + 1) % 3]) / det for r in range(3)] for c in range(3)]
    t = m[12:15]
    out = [0.0] * 16
    for c in range(3):
        for r in range(3):
            out[c * 4 + r] = inv[r][c]
    for r in range(3):
        out[12 + r] = -sum(inv[r][k] * t[k] for k in range(3))
    out[15] = 1.0
    return out


def point(m: list[float], p) -> tuple[float, float, float]:
    return tuple(m[r] * p[0] + m[4 + r] * p[1] + m[8 + r] * p[2] + m[12 + r] for r in range(3))


def vector(m: list[float], v) -> tuple[float, float, float]:
    return tuple(m[r] * v[0] + m[4 + r] * v[1] + m[8 + r] * v[2] for r in range(3))
