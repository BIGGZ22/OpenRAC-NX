"""The sky: shells of triangles around the camera, baked into a panorama.

func_00203118 relocates the header, shell and cluster pointers. Field
meanings, the 1/1024 scale and the reversed winding follow Wrench. Where
a textured shell keeps texture coordinates, an untextured one keeps an
RGBA colour per vertex (Wrench reads those as coordinates too).
"""

from array import array
from dataclasses import dataclass
import math
import struct

from formats import FormatError, Texture, span, unpack
import games
from mesh import Mesh
from util import cross


@dataclass
class Sky:
    background: tuple[int, int, int]   # Header +0: shows where no shell covers.
    textures: list[Texture]
    shells: list[Mesh]                 # Drawn in order, each blended over the last.


def sky(data: bytes, fmt: games.Format = games.RAC1) -> Sky:
    """Shells of 0x20-byte clusters; each cluster points to vertices, UVs and faces.

    Vertices are (x, y, z, alpha) signed shorts with alpha 0x80 opaque, then
    either (s, t) fixed point or, on untextured shells (flag bit 0), RGBA
    bytes. Faces are three indices and a texture (0xff: untextured).

    Up Your Arsenal and Deadlocked keep a shell's cluster count and flags as two s16 (then the
    shell's rotation and angular velocity, not read), the first two games as two s32 (Wrench's
    UyaDlSkyShellHeader and RacGcSkyShellHeader); Deadlocked's sky textures are swizzled
    (level.unswizzle, the other way round from its level textures, as Wrench reads them).
    """
    shell_count, = unpack("<h", data, 6)
    texture_count, = unpack("<h", data, 0xc)
    if not 0 <= shell_count <= 8 or not 0 <= texture_count <= 256:
        raise FormatError("invalid sky shell or texture count")
    defs, texture_data = unpack("<II", data, 0x10)
    textures = []
    for i in range(texture_count):
        palette, pixels, width, height = unpack("<4I", data, defs + i * 16)
        indices = span(data, texture_data + pixels, width * height)
        if fmt.swizzled_textures:
            from level import unswizzle  # here: level imports this module's neighbours
            indices = unswizzle(indices, width, inverse=True)
        textures.append(Texture(width, height, indices, span(data, texture_data + palette, 1024)))
    result = Sky(tuple(span(data, 0, 3)), textures, [])
    for shell_id in range(shell_count):
        offset, = unpack("<I", data, 0x20 + shell_id * 4)
        clusters, flags = unpack("<hh" if fmt.sky_short_shells else "<II", data, offset)
        if clusters > 0x10000:
            raise FormatError("invalid sky cluster count")
        mesh = Mesh(f"Sky_{shell_id}", colours=[])
        for cluster in range(clusters):
            base, nv, nf, vertices, extra, faces, size = unpack("<I6H", data, offset + 0x20 + cluster * 0x20)
            block = span(data, base, size)
            first = len(mesh.positions)
            for (x, y, z, alpha), rgba in zip(struct.iter_unpack("<4h", span(block, vertices, nv * 8)),
                                              struct.iter_unpack("4B", span(block, extra, nv * 4))):
                if not 0 <= alpha <= 0x80:
                    raise FormatError("invalid sky vertex alpha")
                mesh.positions.append((x / 1024, y / 1024, z / 1024))
                if flags & 1:
                    r, g, b, a = rgba
                    mesh.uvs.append((0.0, 0.0))
                    mesh.colours.append((r / 255, g / 255, b / 255, min(a / 0x80, 1.0)))
                else:
                    s, t = struct.unpack("<2h", bytes(rgba))
                    mesh.uvs.append((s / 4096, t / 4096))
                    mesh.colours.append((1.0, 1.0, 1.0, alpha / 0x80))
            for a, b, c, texture in struct.iter_unpack("4B", span(block, faces, nf * 4)):
                if max(a, b, c) >= nv or (texture != 0xff and texture >= texture_count):
                    raise FormatError("invalid sky face index or texture")
                if texture != 0xff and flags & 1:
                    raise FormatError("textured face on an untextured sky shell")
                mesh.add_face(None if texture == 0xff else ("sky", texture), (first + c, first + b, first + a))
        result.shells.append(mesh)
    return result


def unit(p):
    length = math.sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2])
    return (p[0] / length, p[1] / length, p[2] / length)


def footprint(a, b, c, width: int, height: int):
    """Rows and columns a spherical triangle can touch, from points along its edges.

    A triangle around a pole reaches that edge of the image and every column.
    """
    points = [unit(tuple(p[i] + (q[i] - p[i]) * k / 8 for i in range(3)))
              for p, q in ((a, b), (b, c), (c, a)) for k in range(8)]
    rows = [math.acos(max(-1.0, min(1.0, p[2]))) / math.pi * height - 0.5 for p in points]
    top, bottom = max(0, math.floor(min(rows)) - 1), min(height - 1, math.ceil(max(rows)) + 1)
    normals = (cross(b, c), cross(c, a), cross(a, b))
    if all(n[2] >= 0 for n in normals):
        return 0, bottom, range(width)
    if all(n[2] <= 0 for n in normals):
        return top, height - 1, range(width)
    # Columns: the shortest arc of azimuths covering every edge point.
    angles = sorted(math.atan2(p[0], p[1]) % math.tau for p in points)
    gaps = [(angles[(i + 1) % len(angles)] - angles[i]) % math.tau for i in range(len(angles))]
    start = angles[(gaps.index(max(gaps)) + 1) % len(angles)]
    span_ = math.tau - max(gaps)
    first = math.floor(start / math.tau * width - 0.5) - 1
    last = math.ceil((start + span_) / math.tau * width - 0.5) + 1
    return top, bottom, [x % width for x in range(first, last + 1)]


def panorama(data: Sky, width: int = 2048, height: int = 1024) -> bytes:
    """The sky as an equirectangular RGB image, as Godot's PanoramaSkyMaterial reads it.

    Pixel (x, y) looks along game direction (sin t sin p, sin t cos p, cos t)
    with p = 2 pi (x + 1/2) / width and t = pi (y + 1/2) / height: the top row
    is straight up (+Z) and the left edge faces +Y, which the level scene
    turns into Godot's -Z. Shells are blended in order over the background.
    Textures are sampled bilinearly, wrapping across and clamped down.
    """
    angles = [math.tau * (x + 0.5) / width for x in range(width)]
    sin_p, cos_p = [math.sin(p) for p in angles], [math.cos(p) for p in angles]
    texels = [[tuple(v / 255 for v in texture.colours[i]) for i in texture.pixels] for texture in data.textures]
    by_row = [[] for _ in range(height)]
    for mesh in data.shells:
        for key, faces in mesh.faces.items():
            for face in faces:
                a, b, c = (mesh.positions[v] for v in face)
                normals = (cross(b, c), cross(c, a), cross(a, b))
                volume = sum(a[i] * normals[0][i] for i in range(3))
                if abs(volume) < 1e-9:
                    continue  # Edge-on to the camera: covers no directions.
                sign = 1 if volume > 0 else -1
                normals = tuple(tuple(sign * n for n in normal) for normal in normals)
                top, bottom, columns = footprint(a, b, c, width, height)
                triangle = (normals, columns, [mesh.colours[v] for v in face], [mesh.uvs[v] for v in face],
                            None if key is None else (texels[key[1]], data.textures[key[1]]))
                for y in range(top, bottom + 1):
                    by_row[y].append(triangle)
    background = [c / 255 for c in data.background]
    out = bytearray()
    for y in range(height):
        theta = math.pi * (y + 0.5) / height
        sin_t, cos_t = math.sin(theta), math.cos(theta)
        row = array("f", background * width)
        for normals, columns, colours, uvs, texture in by_row[y]:
            (ax, ay, az), (bx, by, bz), (cx, cy, cz) = normals
            ax, ay, az, bx, by, bz = ax * sin_t, ay * sin_t, az * cos_t, bx * sin_t, by * sin_t, bz * cos_t
            cx, cy, cz = cx * sin_t, cy * sin_t, cz * cos_t
            for x in columns:
                s, c = sin_p[x], cos_p[x]
                la = ax * s + ay * c + az
                lb = bx * s + by * c + bz
                lc = cx * s + cy * c + cz
                if la < 0 or lb < 0 or lc < 0:
                    continue
                total = la + lb + lc
                la, lb, lc = la / total, lb / total, lc / total
                ca, cb, cc = colours
                alpha = la * ca[3] + lb * cb[3] + lc * cc[3]
                if texture is None:
                    r = la * ca[0] + lb * cb[0] + lc * cc[0]
                    g = la * ca[1] + lb * cb[1] + lc * cc[1]
                    b = la * ca[2] + lb * cb[2] + lc * cc[2]
                else:
                    pixels, info = texture
                    w, h = info.width, info.height
                    u = (la * uvs[0][0] + lb * uvs[1][0] + lc * uvs[2][0]) * w - 0.5
                    v = (la * uvs[0][1] + lb * uvs[1][1] + lc * uvs[2][1]) * h - 0.5
                    x0, y0 = math.floor(u), math.floor(v)
                    fx, fy = u - x0, v - y0
                    x0, x1 = x0 % w, (x0 + 1) % w
                    y1 = min(max(y0 + 1, 0), h - 1) * w
                    y0 = min(max(y0, 0), h - 1) * w
                    t00, t01, t10, t11 = pixels[y0 + x0], pixels[y0 + x1], pixels[y1 + x0], pixels[y1 + x1]
                    w00, w01, w10, w11 = (1 - fx) * (1 - fy), fx * (1 - fy), (1 - fx) * fy, fx * fy
                    r = t00[0] * w00 + t01[0] * w01 + t10[0] * w10 + t11[0] * w11
                    g = t00[1] * w00 + t01[1] * w01 + t10[1] * w10 + t11[1] * w11
                    b = t00[2] * w00 + t01[2] * w01 + t10[2] * w10 + t11[2] * w11
                    alpha *= t00[3] * w00 + t01[3] * w01 + t10[3] * w10 + t11[3] * w11
                alpha = min(alpha, 1.0)
                i = x * 3
                row[i] += (r - row[i]) * alpha
                row[i + 1] += (g - row[i + 1]) * alpha
                row[i + 2] += (b - row[i + 2]) * alpha
        out.extend(min(255, max(0, round(v * 255))) for v in row)
    return bytes(out)
