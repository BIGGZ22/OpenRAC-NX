"""Read PNG files a modder's image editor writes, to check re-imported textures.

Any colour type at 8 or 16 bits per channel (and 1, 2, 4-bit palettes and
greys), with a palette's tRNS. Interlaced files are refused with a message
saying how to save them; the game's own textures are never interlaced.
"""

import struct
import zlib

SIGNATURE = b"\x89PNG\r\n\x1a\n"
CHANNELS = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}


class PngError(ValueError):
    """A file that is not a PNG this reader takes; the message says why."""


def chunks(data: bytes):
    if data[:8] != SIGNATURE:
        raise PngError("not a PNG file")
    at = 8
    while at + 8 <= len(data):
        size, kind = struct.unpack_from(">I4s", data, at)
        yield kind, data[at + 8:at + 8 + size]
        at += 12 + size
        if kind == b"IEND":
            return
    raise PngError("the PNG file is cut short")


def size(data: bytes) -> tuple[int, int]:
    """Width and height from the header alone."""
    for kind, payload in chunks(data):
        if kind == b"IHDR":
            return struct.unpack(">2I", payload[:8])
        break
    raise PngError("the PNG file has no header")


def paeth(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    return a if pa <= pb and pa <= pc else b if pb <= pc else c


def unfilter(raw: bytes, height: int, stride: int, bpp: int) -> bytearray:
    out = bytearray(stride * height)
    prior = bytearray(stride)
    at = 0
    for y in range(height):
        kind = raw[at]
        line = bytearray(raw[at + 1:at + 1 + stride])
        at += 1 + stride
        if kind == 1:
            for i in range(bpp, stride):
                line[i] = (line[i] + line[i - bpp]) & 0xFF
        elif kind == 2:
            for i in range(stride):
                line[i] = (line[i] + prior[i]) & 0xFF
        elif kind == 3:
            for i in range(stride):
                line[i] = (line[i] + ((line[i - bpp] if i >= bpp else 0) + prior[i]) // 2) & 0xFF
        elif kind == 4:
            for i in range(stride):
                left = line[i - bpp] if i >= bpp else 0
                corner = prior[i - bpp] if i >= bpp else 0
                line[i] = (line[i] + paeth(left, prior[i], corner)) & 0xFF
        elif kind != 0:
            raise PngError(f"unknown PNG row filter {kind}")
        out[y * stride:(y + 1) * stride] = line
        prior = line
    return out


def expand(samples: bytes, colour: int, count: int) -> bytes:
    """8-bit grey, grey and alpha, RGB or RGBA samples as RGBA, by slices."""
    if colour == 6:
        return samples
    out = bytearray(count * 4)
    if colour == 2:
        for c in range(3):
            out[c::4] = samples[c::3]
        out[3::4] = b"\xff" * count
    elif colour == 0:
        for c in range(3):
            out[c::4] = samples
        out[3::4] = b"\xff" * count
    else:
        for c in range(3):
            out[c::4] = samples[0::2]
        out[3::4] = samples[1::2]
    return bytes(out)


def read(data: bytes) -> tuple[int, int, bytes]:
    """Width, height and the pixels as RGBA bytes, rows from the top."""
    header, palette, alpha, idat = None, None, None, []
    for kind, payload in chunks(data):
        if kind == b"IHDR":
            header = struct.unpack(">2I5B", payload[:13])
        elif kind == b"PLTE":
            palette = payload
        elif kind == b"tRNS":
            alpha = payload
        elif kind == b"IDAT":
            idat.append(payload)
    if header is None:
        raise PngError("the PNG file has no header")
    width, height, depth, colour, _compression, _filter, interlace = header
    if colour not in CHANNELS or depth not in (1, 2, 4, 8, 16) or (colour in (2, 4, 6) and depth < 8):
        raise PngError(f"unsupported PNG colour type {colour} at {depth} bits")
    if interlace:
        raise PngError("the PNG file is interlaced: save it again with interlacing off")
    if not 0 < width <= 16384 or not 0 < height <= 16384:
        raise PngError(f"a {width} x {height} image is too large")
    try:
        raw = zlib.decompress(b"".join(idat))
    except zlib.error as exc:
        raise PngError(f"the PNG data is damaged ({exc})") from None
    channels = CHANNELS[colour]
    bits = channels * depth
    stride = (width * bits + 7) // 8
    if len(raw) < (stride + 1) * height:
        raise PngError("the PNG data is shorter than its size says")
    rows = unfilter(raw, height, stride, max(1, bits // 8))
    if depth == 8 and colour != 3:
        return width, height, expand(bytes(rows), colour, width * height)
    out = bytearray(width * height * 4)
    for y in range(height):
        line = rows[y * stride:(y + 1) * stride]
        if depth < 8:
            per = 8 // depth
            mask = (1 << depth) - 1
            samples = [(line[x // per] >> (8 - depth * (x % per + 1))) & mask for x in range(width)]
        elif depth == 16:
            samples = line[0::2]
        else:
            samples = line
        for x in range(width):
            o = (y * width + x) * 4
            if colour == 3:
                index = samples[x]
                if palette is None or index * 3 + 3 > len(palette):
                    raise PngError("a palette PNG uses a colour its palette lacks")
                out[o:o + 3] = palette[index * 3:index * 3 + 3]
                out[o + 3] = alpha[index] if alpha is not None and index < len(alpha) else 255
            elif colour == 0:
                v = samples[x] * 255 // ((1 << depth) - 1) if depth < 8 else samples[x]
                out[o:o + 4] = bytes((v, v, v, 255))
            elif colour == 4:
                v, a = samples[x * 2], samples[x * 2 + 1]
                out[o:o + 4] = bytes((v, v, v, a))
            elif colour == 2:
                out[o:o + 3] = samples[x * 3:x * 3 + 3]
                out[o + 3] = 255
            else:
                out[o:o + 4] = samples[x * 4:x * 4 + 4]
    return width, height, bytes(out)
