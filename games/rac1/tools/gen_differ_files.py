"""Produce the inputs asm-differ's binary mode needs (see diff_settings.py).

  build-sn/differ_base.bin  retail, flattened: file offset = vaddr - BASE
  build-sn/differ_my.bin    our linked ELF, flattened the same way
  build-sn/differ.map       a minimal GNU-ld-style map naming every symbol

Our ee-ld's own -Map output predates the "load address" lines asm-differ
keys on, so the map is synthesised from the ELF symbol table instead.
Re-run after every link; `python tools/diff.py` does that for you.
"""
from elftools.elf.elffile import ELFFile

BASE = 0x100000

def flatten(path, out):
    elf = ELFFile(open(path, "rb"))
    img = bytearray()
    for seg in elf.iter_segments():
        if seg["p_type"] != "PT_LOAD":
            continue
        off = seg["p_vaddr"] - BASE
        data = seg.data()[: seg["p_filesz"]]
        if off < 0:
            continue
        if len(img) < off + len(data):
            img.extend(b"\0" * (off + len(data) - len(img)))
        img[off : off + len(data)] = data
    open(out, "wb").write(img)
    return elf

def main():
    flatten("baserom/SCES_509.16", "build-sn/differ_base.bin")
    elf = flatten("build-sn/rac1.elf", "build-sn/differ_my.bin")
    syms = []
    for sec in elf.iter_sections():
        if sec.name == ".symtab":
            for s in sec.iter_symbols():
                if s.name and s["st_shndx"] != "SHN_UNDEF" and s["st_info"]["type"] in ("STT_FUNC", "STT_NOTYPE", "STT_OBJECT"):
                    syms.append((s["st_value"], s.name))
    syms.sort()
    with open("build-sn/differ.map", "w", newline="\n") as m:
        m.write(".text 0x%08x\n" % BASE)
        m.write("                0x0 load address 0x00000000\n")
        seen = set()
        for addr, name in syms:
            if name in seen:
                continue
            seen.add(name)
            m.write("                0x%08x                %s\n" % (addr, name))
    print("wrote build-sn/differ_{base,my}.bin and differ.map (%d symbols)" % len(seen))

main()
