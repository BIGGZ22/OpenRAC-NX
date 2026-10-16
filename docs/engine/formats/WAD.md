# WAD compression

Insomniac's LZ compression, used for single assets all over the disc (core
data, gameplay files, HUD banks, cutscene chunks, gadget classes, most
global textures). "WAD" is the three-byte magic of a compressed stream, not
an archive.

**Games.** RAC1, measured: ReRAC decodes every one of the 4,674 streams on
the NTSC-U disc, and OpenRAC's extractor every stream it reads on PAL, where
the game's decoder is `func_0020C468` (NTSC-U 0x20B618)
([ASSETS.md](../../../games/rac1/docs/ASSETS.md#wad-compression)). The
sequels use the same magic: RAC2's prerelease level data holds `"WAD"`
containers ([LEVEL-ARCHIVE-FORMAT.md](../../../games/rac2/ntsc/docs/LEVEL-ARCHIVE-FORMAT.md)),
and RAC3 and Deadlocked wrap their boot image in one (Wrench, **reference**).
Whether the packet rules below hold unchanged in the sequels is not measured.

## 1. Header (0x10 bytes)

| Offset | Size | Field |
|---|---:|---|
| 0x0 | 3 | `"WAD"` |
| 0x3 | 4 | compressed size, unaligned, including this header |
| 0x7 | 9 | a short ASCII tag, for example `coredata`, `gameplay`, `hud_bank`, `gadget`, `transition`; Wrench writes `WRENCH010`. Not used by a reader |

There is no decompressed size and no checksum. The one place the result's
size is known in advance is a level's core data, whose size is in the core
index at +0x8C ([LEVEL.md](LEVEL.md)).

## 2. Packets

The stream after the header is a sequence of packets, each starting with a
flag byte. P is the output length when the packet is decoded.

| Flag | Packet | Copies | From | Then |
|---|---|---|---|---|
| 0x00 | long literal | next byte + 18 bytes of input | | |
| 0x01–0x0F | literal | flag + 3 bytes of input | | |
| 0x10–0x1F | far match | length `flag & 7`, or next byte + 7 when 0; then + 2 | P − (0x4000 × (A + 1) + b1 × 0x40 + (b0 >> 2)), A = bit 3 of the flag | `b0 & 3` literals |
| 0x20–0x3F | match | `flag & 0x1F`, or next byte + 0x1F when 0; then + 2 | P − (b2 × 0x40 + (b1 >> 2) + 1) | `b1 & 3` literals |
| 0x40–0xFF | short match | `(flag >> 5) + 1` | P − (b1 × 8 + ((flag >> 2) & 7) + 1) | `flag & 3` literals |

- **Little literals.** After every match, 0 to 3 bytes are copied straight
  from the input; the count is the low two bits of the second-to-last byte
  the packet consumed (the table's last column). Literal packets carry none.
- **No two literal packets in a row.** The format relies on it; a decoder
  can treat it as an error.
- **Overlapping copies.** A match copies forwards one byte at a time from
  output it may itself be writing (run-length expansion), so `memcpy` is
  wrong.
- **Far matches with A = 1.** Wrench's compressor and decompressor disagree
  on bit 3 (0x4000 × (A + 1) against 0x4000 + A × 0x800). ReRAC settled it on
  retail data: with 0x4000 × (A + 1) every stream decodes and every level's
  core data has the size its index records. PAL's decoder adds 0x4000 for a
  far match as well.
- **Control packets.** A far match whose distance fields are all zero is not
  a copy:
  - length 1 (`11 00 00`, or `11 0N 00` and N literals): a dummy packet that
    only carries little literals; it separates two literals;
  - any other length (`12 00 00`): a block marker. The input skips to the
    next block boundary and the packet ends with no literals. Filler bytes
    follow it (Wrench writes 0xEE).

## 3. Block markers and the scratchpad

The game decompresses from the scratchpad and refills it by DMA one block at
a time, so no packet may straddle a block boundary.

- **PAL code:** the block is 0x2000 bytes ([ASSETS.md](../../../games/rac1/docs/ASSETS.md#wad-compression)).
- **Wrench's reader** aligns the skip to 0x1000 and its writer pads to
  0x2000 (to offset 0x10 of the next 0x2000 block, counting the header).
  ReRAC reads with 0x1000, which accepts both; on the discs measured every
  marker lands where both give the same result.

A writer should pad to 0x2000: before a packet, if
`((offset + 0x1FF0) % 0x2000) + packet_size > 0x2000 − 3`, emit `12 00 00`
and fill until `offset % 0x2000 == 0x10`.

## 4. Writing

Re-compression need not reproduce the original bytes. Limits a writer must
keep: literals of 4–273 bytes in their own packet, 1–3 bytes only as little
literals; short matches 3–8 bytes within 2,048; matches 3–288 bytes within
16,384; far matches 3–264 bytes within 16,385–32,704; never set bit 3 of a
far match. ReRAC's writer uses a 32,768-entry hash chain over three bytes.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/disc_layout.md` sections 3 and 5 and
`docs/formats/wad_layouts_rac1.md` section 0.3; PAL facts from
[ASSETS.md](../../../games/rac1/docs/ASSETS.md).
