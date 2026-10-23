"""RAC1 baked PVS export; layouts documented in docs/engine/formats/OCCLUSION.md.

Only local disc data is written. Unknown/stale object mappings remain always visible.
"""
import json
import struct
from pathlib import Path

from formats import span, unpack
import games


def export_pvs(level, directory: Path):
    if games.of(level).game != "rac1" or len(level.gameplay) < 0x90:
        return
    grid_offset, = unpack("<I", level.index, 0x0c)
    mapping_offset, = unpack("<I", level.gameplay, 0x8c)
    if not grid_offset or not mapping_offset:
        return
    grid = level.block(grid_offset)
    counts = unpack("<3I", level.gameplay, mapping_offset)
    records = list(struct.iter_unpack(
        "<ii", span(level.gameplay, mapping_offset + 16, sum(counts) * 8)))
    sections, start = [], 0
    for count in counts:
        sections.append(records[start:start + count])
        start += count
    def word(bit):
        return (bit >> 3) << 8 | 1 << (bit & 7) if 0 <= bit < 1023 else 0x7f80
    terrain_offset, = unpack("<I", level.index, 8)
    terrain = level.block(terrain_offset)
    table, count = unpack("<2I", terrain)
    terrain_words = [0x7f80] * count
    if count == counts[0] and all(terrain[table + i * 0x40 + 0x3d] == ident
                                  for i, (_, ident) in enumerate(sections[0])):
        terrain_words = [word(bit) for bit, _ in sections[0]]
    # Runtime ties match the instance's key, not its class or output-list index.
    ties = {}
    for bit, ident in sections[1]:
        ties.setdefault(str(ident), word(bit))
    tie_words = []
    tie_offset, = unpack("<I", level.gameplay, 0x34)
    if tie_offset:
        tie_count, = unpack("<I", level.gameplay, tie_offset)
        keys = [unpack("<i", level.gameplay, tie_offset + 16 + i * 0xe0 + 12)[0]
                for i in range(tie_count)]
        if tie_count == counts[1] and all(key == record[1] for key, record in zip(keys, sections[1])):
            tie_words = [word(bit) for bit, _ in sections[1]]
        else:
            tie_words = [ties.get(str(key), 0x7f80) for key in keys]
    mobys = {}
    for bit, ident in sections[2]:
        mobys.setdefault(str(ident), word(bit))
    document = {"format": 1, "game": "rac1", "version": level.version, "level": level.id,
                "grid": "pvs.bin", "terrain": terrain_words, "ties": ties,
                "tie_words": tie_words, "mobys": mobys}
    (directory / "pvs.bin").write_bytes(grid)
    (directory / "pvs.json").write_text(json.dumps(document, separators=(',', ':')) + '\n')


if __name__ == '__main__':
    import argparse
    from disc import Disc
    from level import load_level
    parser = argparse.ArgumentParser(description="Add PVS metadata to existing RAC1 port exports")
    parser.add_argument('image', type=Path)
    parser.add_argument('levels', type=Path)
    args = parser.parse_args()
    with Disc(args.image) as disc:
        for info in disc.survey()['levels']:
            directory = args.levels / f"level_{info['id']:02}"
            manifest = directory / 'manifest.json'
            if not manifest.is_file():
                continue
            identity = json.loads(manifest.read_text())
            if (identity.get('game'), identity.get('version'), identity.get('level')) != (
                    'rac1', disc.version.key, info['id']):
                raise ValueError(f"export/disc identity mismatch: {directory}")
            export_pvs(load_level(disc, info), directory)
            print(f"PVS: {directory}", flush=True)
