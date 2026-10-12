"""Just enough ELF32 (little-endian MIPS) for the build: read an executable's sections and
segments, write a level program as an executable, and compare two executables' loaded bytes."""
import struct
from pathlib import Path

SHT_PROGBITS, SHT_NOBITS, SHT_STRTAB = 1, 8, 3
PT_LOAD = 1


def read(path: Path) -> dict:
    data = Path(path).read_bytes()
    if data[:4] != b"\x7fELF":
        raise ValueError(f"{path} is not an ELF file")
    entry, phoff, shoff = struct.unpack_from("<III", data, 0x18)
    phentsize, phnum, shentsize, shnum, shstrndx = struct.unpack_from("<HHHHH", data, 0x2A)
    segments = []
    for i in range(phnum):
        kind, offset, address, _, filesz, memsz, flags, _ = struct.unpack_from("<8I", data, phoff + i * phentsize)
        segments.append({"type": kind, "offset": offset, "address": address, "filesz": filesz,
                         "memsz": memsz, "flags": flags})
    headers = [struct.unpack_from("<10I", data, shoff + i * shentsize) for i in range(shnum)]
    names = headers[shstrndx][4] if shnum else 0
    sections = []
    for h in headers:
        raw = data[names + h[0]:]
        sections.append({"name": raw[:raw.index(b"\0")].decode(), "type": h[1], "address": h[3],
                         "offset": h[4], "size": h[5]})
    return {"type": struct.unpack_from("<H", data, 0x10)[0], "entry": entry,
            "segments": segments, "sections": sections, "data": data}


def write_exec(path: Path, entry: int, sections: list[dict]) -> None:
    """An ET_EXEC holding SECTIONS ({name, address, type, data}); every PROGBITS section is
    its own PT_LOAD, and a NOBITS section takes no file space."""
    names = b"\0" + b"".join(s["name"].encode() + b"\0" for s in sections) + b".shstrtab\0"
    body, offset, placed = b"", 0x1000, []
    for s in sections:
        if s["type"] == SHT_NOBITS:
            placed.append((s, offset, len(s["data"])))
            continue
        offset = (offset + 15) & ~15
        body += b"\0" * (offset - 0x1000 - len(body)) + s["data"]
        placed.append((s, offset, len(s["data"])))
        offset += len(s["data"])
    loads = [(s, off, n) for s, off, n in placed if s["type"] != SHT_NOBITS]
    phoff = 0x34
    shoff = (0x1000 + len(body) + len(names) + 15) & ~15
    out = bytearray(shoff + 40 * (len(sections) + 2))
    struct.pack_into("<4sBBBB8xHHIIIIIHHHHHH", out, 0, b"\x7fELF", 1, 1, 1, 0, 2, 8, 1, entry, phoff,
                     shoff, 0x20924001, 0x34, 32, len(loads), 40, len(sections) + 2, len(sections) + 1)
    for i, (s, off, n) in enumerate(loads):
        flags = 5 if s["name"] == ".text" else 6
        struct.pack_into("<8I", out, phoff + 32 * i, PT_LOAD, off, s["address"], s["address"], n, n, flags, 16)
    out[0x1000:0x1000 + len(body)] = body
    names_at = 0x1000 + len(body)
    out[names_at:names_at + len(names)] = names
    at = 1
    for i, (s, off, n) in enumerate(placed):
        flags = 6 if s["name"] == ".text" else 3          # alloc + exec, or alloc + write
        struct.pack_into("<10I", out, shoff + 40 * (i + 1), at, s["type"], flags, s["address"], off, n, 0, 0, 16, 0)
        at += len(s["name"]) + 1
    struct.pack_into("<10I", out, shoff + 40 * (len(sections) + 1), at, SHT_STRTAB, 0, 0, names_at, len(names), 0, 0, 1, 0)
    Path(path).write_bytes(bytes(out))


def compare_loads(reference: Path, candidate: Path) -> tuple[bool, str]:
    """Whether every PT_LOAD of CANDIDATE has REFERENCE's address, sizes, flags and bytes, and
    the entry points agree; else where they first differ."""
    a, b = read(reference), read(candidate)
    if a["entry"] != b["entry"]:
        return False, f"entry {b['entry']:#x}, expected {a['entry']:#x}"
    la = sorted((s for s in a["segments"] if s["type"] == PT_LOAD), key=lambda s: s["address"])
    lb = sorted((s for s in b["segments"] if s["type"] == PT_LOAD), key=lambda s: s["address"])
    if len(la) != len(lb):
        return False, f"{len(lb)} loadable segments, expected {len(la)}"
    for x, y in zip(la, lb):
        for field in ("address", "filesz", "memsz", "flags"):
            if x[field] != y[field]:
                return False, f"segment at {x['address']:#x}: {field} {y[field]:#x}, expected {x[field]:#x}"
        rx = a["data"][x["offset"]:x["offset"] + x["filesz"]]
        ry = b["data"][y["offset"]:y["offset"] + y["filesz"]]
        if rx != ry:
            at = next(i for i, (p, q) in enumerate(zip(rx, ry)) if p != q)
            return False, f"bytes differ at {x['address'] + at:#010x}"
    return True, f"{sum(s['filesz'] for s in la)} loaded bytes equal"


SHT_SYMTAB, SHT_REL = 2, 9
R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16 = 2, 4, 5, 6, 7


def read_object(path: Path) -> dict:
    """A relocatable object: {"sections": [{name, type, data, size}], "symbols": [{name, value,
    size, section, bind, type}], "relocations": {section index: [(offset, type, symbol index)]}}."""
    data = Path(path).read_bytes()
    shoff = struct.unpack_from("<I", data, 0x20)[0]
    shentsize, shnum, shstrndx = struct.unpack_from("<HHH", data, 0x2E)
    headers = [struct.unpack_from("<10I", data, shoff + i * shentsize) for i in range(shnum)]

    def string(table: int, at: int) -> str:
        raw = data[headers[table][4] + at:]
        return raw[:raw.index(b"\0")].decode()
    sections = [{"name": string(shstrndx, h[0]), "type": h[1], "size": h[5], "link": h[6], "info": h[7],
                 "data": data[h[4]:h[4] + h[5]] if h[1] != SHT_NOBITS else b""} for h in headers]
    symbols, relocations = [], {}
    for s in sections:
        if s["type"] == SHT_SYMTAB:
            for at in range(0, len(s["data"]), 16):
                name, value, size, info, _, shndx = struct.unpack_from("<IIIBBH", s["data"], at)
                symbols.append({"name": string(s["link"], name), "value": value, "size": size,
                                "section": shndx, "bind": info >> 4, "type": info & 15})
    for s in sections:
        if s["type"] == SHT_REL:
            relocations[s["info"]] = [(o, i & 0xFF, i >> 8) for o, i in
                                      (struct.unpack_from("<II", s["data"], at) for at in range(0, len(s["data"]), 8))]
    return {"sections": sections, "symbols": symbols, "relocations": relocations}
