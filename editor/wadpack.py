"""Compress a WAD stream: the inverse of formats.wad.

The format is LZO-like (formats.wad has the packets). What the game needs
beyond a stream that decodes, measured on every level's gameplay file of the
PAL disc (1,299 blocks):

- the stream is read in 0x2000-byte blocks, refilled by DMA: no packet
  crosses a block's end, and every block ends with a skip packet (a
  zero-distance long match) and at least 8 unused bytes after it;
- a literal run never follows a literal run (retail traps on it): one to
  three literals ride in the low bits of the match before them, and where
  there is no match before them, in a match that copies nothing.

The match search is greedy with a short hash chain: within a few percent of
the retail compressor, which is what matters for a file that has to be read
into the room the game has for it.
"""

from formats import FormatError

BLOCK = 0x2000
HEADER = 16
SKIP = bytes((0x12, 0, 0))       # distance 0, count 2: skip to the next block
ROOM = 12                        # unused bytes kept after a block's skip (retail: 8 to 242)
CHAIN = 24                       # candidates tried per position
MAX_LITERALS = 273               # one literal packet: 18 + 255


def _tokens(data: bytes) -> list[tuple[int, int, int]]:
    """(literal count, match length, match distance) triples covering DATA; the last has length 0."""
    out, n, i, start = [], len(data), 0, 0
    table: dict[bytes, list[int]] = {}
    while i + 3 <= n:
        key = data[i:i + 3]
        chain = table.get(key)
        best, where = 0, 0
        if chain:
            limit = min(n - i, 264)
            for j in reversed(chain[-CHAIN:]):
                if i - j > 0xBFFF:
                    break
                if data[j + best:j + best + 1] != data[i + best:i + best + 1]:
                    continue
                k = 3
                while k < limit and data[j + k] == data[i + k]:
                    k += 1
                if k > best:
                    best, where = k, j
                    if k == limit:
                        break
        if best >= 3 and (best > 3 or i - where <= 0x800):
            out.append((i - start, best, i - where))
            for p in range(i, min(i + best, n - 2)):
                table.setdefault(data[p:p + 3], []).append(p)
            i += best
            start = i
        else:
            table.setdefault(key, []).append(i)
            i += 1
    out.append((n - start, 0, 0))
    return out


def _match(length: int, distance: int) -> list[bytearray]:
    """The packets of one match (a long one is split); each ends with the byte whose low bits take trailing literals."""
    packets = []
    while length:
        take = length
        if distance <= 0x800 and take <= 8:
            packets.append(bytearray((((take - 1) << 5) | (((distance - 1) & 7) << 2), (distance - 1) >> 3)))
        elif distance <= 0x4000:
            take = min(take, 31 + 255 + 2)
            if take - 2 <= 31:
                head = bytearray((0x20 | (take - 2),))
            else:
                head = bytearray((0x20, take - 2 - 31))
            d = distance - 1
            packets.append(head + bytes(((d & 63) << 2, d >> 6)))
        else:
            take = min(take, 7 + 255 + 2)
            d = distance - 0x4000
            tag = 0x10 | ((d >> 11) & 8)
            if take - 2 <= 7:
                head = bytearray((tag | (take - 2),))
            else:
                head = bytearray((tag, take - 2 - 7))
            packets.append(head + bytes(((d & 63) << 2, (d >> 6) & 0xFF)))
        # never leave a remainder too short to be a match of its own
        rest = length - take
        if 0 < rest < 3:
            take -= 3 - rest
            packets.pop()
            packets += _match(take, distance)
        length -= take
    return packets


class _Writer:
    def __init__(self, header: bytes):
        self.out = bytearray(header)
        self.trail = None        # index of the byte whose low two bits may still take literals
        self.after_literals = False

    def room(self) -> int:
        return BLOCK - (len(self.out) - HEADER) % BLOCK

    def skip(self) -> None:
        self.out += SKIP
        self.out += bytes((BLOCK - (len(self.out) - HEADER) % BLOCK) % BLOCK)
        self.trail, self.after_literals = None, False

    def fit(self, size: int) -> None:
        """Start a new block unless SIZE more bytes leave room for the skip and its margin."""
        if size + len(SKIP) + ROOM > self.room():
            self.skip()

    def literals(self, data: bytes) -> None:
        # The stream opens with an ordinary literal packet, as every retail one does (0x00, 0x06,
        # 0x0a on the PAL disc's gameplay files): LZO's opening byte, a literal count plus 0x11,
        # is a match to the port's decoder (port/assets/disc/wad.cpp), with nothing to copy from.
        while data:
            if len(data) <= 3 and self.trail is not None and not self.after_literals:
                self.fit(len(data))
                if self.trail is not None:            # still in the block of that match
                    self.out[self.trail] |= len(data)
                    self.out += data
                    self.after_literals, self.trail = True, None
                    return
                continue
            if self.after_literals or len(data) <= 3:
                # a match that copies nothing: it ends a literal run, and carries up to three literals
                take = len(data) if len(data) <= 3 else 0
                self.fit(3 + take)
                self.out += bytes((0x11, take, 0)) + data[:take]
                self.after_literals, self.trail, data = take != 0, None, data[take:]
                continue
            take = min(len(data), MAX_LITERALS)
            if take + 2 + len(SKIP) + ROOM > self.room():
                take = self.room() - 2 - len(SKIP) - ROOM
                if take < 4:
                    self.skip()
                    continue
            if len(data) - take in (1, 2, 3) and take > 7:
                take -= 4 - (len(data) - take)            # leave a run long enough for a packet of its own
            head = bytes((take - 3,)) if take <= 18 else bytes((0, take - 18))
            self.out += head + data[:take]
            self.after_literals, self.trail, data = True, None, data[take:]

    def match(self, length: int, distance: int) -> None:
        for packet in _match(length, distance):
            self.fit(len(packet))
            self.trail = len(self.out) + len(packet) - 2
            self.out += packet
            self.after_literals = False


def compress(data: bytes, header: bytes | None = None) -> bytes:
    """DATA as a WAD stream. HEADER is the 16-byte header of the stream being replaced (its
    bytes after the size are kept); without one, a header with only the magic and the size."""
    head = bytearray(header[:HEADER] if header else b"WAD" + bytes(13))
    if head[:3] != b"WAD" or len(head) != HEADER:
        raise FormatError("expected a 16-byte WAD header")
    w = _Writer(bytes(head))
    pos = 0
    for literal, length, distance in _tokens(data):
        if literal:
            w.literals(data[pos:pos + literal])
            pos += literal
        if length:
            w.match(length, distance)
            pos += length
    w.out[3:7] = len(w.out).to_bytes(4, "little")
    return bytes(w.out)
