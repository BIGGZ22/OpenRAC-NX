#!/usr/bin/env python3
"""
Strict verdict for a level overlay function (func_LNN_XXXXXXXX): unlike
try_func's masked compare for executable functions, EXACT here means every
relocation in the candidate's compiled object reaches the exact retail
address, not just that the instruction words match with link-dependent
fields masked out (docs/OVERLAYS.md, "Plan" step 2).

  python tools/overlay_check.py OBJ.o func_LNN_XXXXXXXX [--diff]

OBJ.o is an ELF relocatable object built the way tools/try_func.py builds
one (the "text" segment pipeline): its .text holds the function (and,
for a stub that branches into its neighbour, that neighbour too) as
unlinked code with REL relocations still pending.

check(obj_path, name, show=False) is the entry point tools/try_func.py
calls; the __main__ block below is only a thin CLI over it for standalone
use.

Placement and resolution (docs/OVERLAYS.md, "Names" and "Layout"):

  - The object's .text is placed so NAME's symbol lands at its canonical
    address (level NN, address XXXXXXXX -- read straight out of the name).
  - Every relocation touching NAME's bytes is resolved and applied in
    Python (R_MIPS_26, R_MIPS_HI16/LO16 with standard REL pairing,
    R_MIPS_GPREL16 against _gp = 0x166D00, R_MIPS_32); anything else is
    reported as an unknown type.
  - A symbol's address in level NN: func_LMM_Y/D_LMM_Y/jtbl_LMM_Y is Y when
    MM == NN; a func_LMM_Y with MM != NN, or an executable func_Y catalogued
    as kind "exe", is looked up in config/overlays/functions.tsv for its
    place in level NN (an error if it has none there); func_Y/D_Y below
    0x15F000 are resident and equal Y; _gp is 0x166D00. Anything else is
    unresolved -> verdict LINK.
  - A relocation against a section symbol (.text local labels, .rodata)
    resolves through that section's own placed base plus the relocation's
    in-place addend.
  - If the function's relocated references into its own object's .rodata
    (a switch's jump table, a float constant) all land on the same single
    local offset, that table is placed at the retail address its own
    asm/overlays/<name>.s names as jtbl_LNN_Z (MM == NN); with more than
    one distinct table referenced, or none named, the verdict is RODATA.

The final, relocated bytes are compared byte for byte (no masking) against
baserom/overlays/level_NN/text.bin at the function's address (minus the
"text" record's own address, from manifest.json), for the catalogue's
recorded size.

Verdicts, in try_func's vocabulary: EXACT, BYTES n/size (n differing
bytes; --diff lists the differing instruction offsets, retail vs ours),
SIZE ours/retail, plus LINK <names> and RODATA <detail>.
"""
import json
import re
import sys
from pathlib import Path

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

sys.path.insert(0, str(Path(__file__).resolve().parent))
import rabbitizer as rz  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
CATALOGUE = ROOT / "config/overlays/functions.tsv"
DUMP = ROOT / "baserom/overlays"
ASM_OVERLAYS = ROOT / "asm/overlays"

GP = 0x166D00
RESIDENT_MAX = 0x15F000
# The libgcc helpers the compiler calls by name (a float passed to a variadic
# function becomes a call to `fptodp`). They are resident, at the addresses
# tools/libgcc_ref.py finds them in retail (`python tools/libgcc_ref.py
# retail`, the EXACT rows), so C that leaves the conversion to the compiler
# can be checked instead of calling the helper by its address.
LIBGCC = {"__divdi3": 0x11DFE8, "__moddi3": 0x11E860, "__udivdi3": 0x11EF28, "__umoddi3": 0x11F4F8,
          "dpadd": 0x11FE48, "dpsub": 0x11FEA0, "dpmul": 0x11FF08, "dpdiv": 0x1201B0, "dpcmp": 0x120430,
          "litodp": 0x120480, "dptoli": 0x120538, "dptoul": 0x1205D0, "fptodp": 0x120778}

# R_MIPS_* type numbers we know how to apply.
R_MIPS_32 = 2
R_MIPS_26 = 4
R_MIPS_HI16 = 5
R_MIPS_LO16 = 6
R_MIPS_GPREL16 = 7

OVERLAY_NAME = re.compile(r"^func_L(\d{2})_([0-9A-Fa-f]{8})$")

FUNC_L = re.compile(r"^func_L(\d{2})_([0-9A-Fa-f]{8})$")
DATA_L = re.compile(r"^D_L(\d{2})_([0-9A-Fa-f]{8})$")
JTBL_L = re.compile(r"^jtbl_L(\d{2})_([0-9A-Fa-f]{8})$")
FUNC = re.compile(r"^func_([0-9A-Fa-f]{8})$")
DATA = re.compile(r"^D_([0-9A-Fa-f]{8})$")

JTBL_REF = re.compile(r"%(?:hi|lo)\((jtbl_L(\d{2})_([0-9A-Fa-f]{8}))\)")


def sign16(v: int) -> int:
    return v - 0x10000 if v & 0x8000 else v


class Unresolved(Exception):
    """A symbol or relocation this module cannot place."""
    def __init__(self, what):
        self.what = what
        super().__init__(what)


_catalogue_cache = None


JOINED = ROOT / "config/overlays/joined.tsv"
BARE_RETURN = "func_001E9768"     # `jr $31; nop`, the executable's empty function
_joined_cache = None


def load_joined() -> dict[str, list[str]]:
    """owner -> [piece, ...] from config/overlays/joined.tsv: catalogue entries that are
    really one C function (the owner and the entries that follow it)."""
    global _joined_cache
    if _joined_cache is not None:
        return _joined_cache
    rows = {}
    if JOINED.exists():
        for line in JOINED.read_text().splitlines():
            if not line or line.startswith("#"):
                continue
            owner, pieces = line.split("\t", 1)
            rows[owner] = pieces.split()
    _joined_cache = rows
    return rows


def joined_size(name: str, csize: int, catalogue) -> int:
    """CSIZE plus the pieces joined to NAME (docs/OVERLAYS.md, joined functions), measured from
    their places in NAME's level. A piece of kind exe is a shared fragment such as the delay slot
    that the catalogue split off the preceding jr (func_001EC030); it still counts as part of this
    function's bytes. A piece may start up to four nops after the previous one ends (the compiler
    aligns a loop's head to 8 bytes, and the assembler pads a short loop): the catalogue leaves
    those nops out of both pieces, and the check compares them with the rest."""
    pieces = load_joined().get(name, [])
    m = OVERLAY_NAME.match(name)
    level, start = int(m.group(1)), int(m.group(2), 16)
    end = start + csize
    for piece in pieces:
        if piece not in catalogue:
            raise SystemExit(f"joined.tsv: {name} is joined to {piece}, which is not in the catalogue")
        _kind, size, places = catalogue[piece]
        at = next((a for lv, a in places if lv == level and end <= a <= end + 16), None)
        if at is None:
            raise SystemExit(f"joined.tsv: {piece} does not follow the piece before it in level {level}")
        end = at + size
    # A function whose last piece is the bare return ends in that return's delay slot too: a nop,
    # which the catalogue takes for padding (it sizes func_001E9768 as the `jr $31` alone).
    if pieces and pieces[-1] == BARE_RETURN:
        end += 4
    return end - start


def load_catalogue():
    """name -> (kind, size, [(level, address), ...])."""
    global _catalogue_cache
    if _catalogue_cache is not None:
        return _catalogue_cache
    rows = {}
    for line in CATALOGUE.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        name, kind, size, _fp, _n, places = line.split("\t")
        place_list = [(int(p[:2]), int(p[3:], 16)) for p in places.split(",")]
        rows[name] = (kind, int(size), place_list)
    _catalogue_cache = rows
    return rows


def place_in_level(name: str, level: int, near: int) -> int:
    """NAME's address in LEVEL from the catalogue's places; raises
    Unresolved if NAME isn't catalogued or has no place there. A handful
    of tiny functions (identical bytes, folded to one catalogue entry) sit
    at more than one address in the same level -- e.g. three untouched
    file-static copies of the same helper, one per translation unit that
    still links its own. Nothing in the catalogue says which one a given
    call site used, so this picks whichever place is nearest the
    resolving relocation's own address: in every duplicate case seen while
    testing this module, that was the right one (the caller and its
    matching copy come from the same original object, which lands close
    together after linking), and it is exact (not just closest) whenever
    there is in fact only one place."""
    row = load_catalogue().get(name)
    if row is None:
        raise Unresolved(name)
    matches = places_in_level(name, level)
    if not matches:
        raise Unresolved(name)
    return min(matches, key=lambda a: abs(a - near))


def places_in_level(name: str, level: int) -> list[int]:
    """Every address the catalogue gives NAME in LEVEL (empty if none)."""
    row = load_catalogue().get(name)
    return sorted({a for lv, a in row[2] if lv == level}) if row else []


_owners = {}


def same_function(level: int, a: int, b: int) -> bool:
    """Whether A and B are two places of one catalogued function in LEVEL."""
    if level not in _owners:
        _owners[level] = {addr: name for name, (_k, _s, places) in load_catalogue().items()
                          for lv, addr in places if lv == level}
    owner = _owners[level].get(a)
    return owner is not None and owner == _owners[level].get(b)


def resolve_symbol(name: str, level: int, near: int) -> int:
    """A symbol's address in LEVEL, by name (docs/OVERLAYS.md, "Names").
    NEAR is the address of the relocation being resolved, used only to
    disambiguate a name with more than one place in this level (see
    place_in_level). Raises Unresolved if NAME can't be placed."""
    if name == "_gp":
        return GP
    if name in LIBGCC:
        return LIBGCC[name]
    if (m := FUNC_L.match(name)):
        mm, y = int(m.group(1)), int(m.group(2), 16)
        return y if mm == level else place_in_level(name, level, near)
    if (m := DATA_L.match(name)) or (m := JTBL_L.match(name)):
        mm, y = int(m.group(1)), int(m.group(2), 16)
        if mm == level:
            return y
        raise Unresolved(name)
    if (m := FUNC.match(name)):
        y = int(m.group(1), 16)
        return y if y < RESIDENT_MAX else place_in_level(name, level, near)
    if (m := DATA.match(name)):
        y = int(m.group(1), 16)
        if y < RESIDENT_MAX:
            return y
        raise Unresolved(name)
    raise Unresolved(name)


def section_name(elf: ELFFile, shndx) -> str | None:
    if not isinstance(shndx, int):
        return None
    sec = elf.get_section(shndx)
    return sec.name if sec is not None else None


def level_manifest(level: int) -> dict:
    return json.loads((DUMP / f"level_{level:02d}/manifest.json").read_text())


def text_record(manifest: dict) -> dict:
    return next(r for r in manifest["records"] if r["name"] == "text")


def retail_jtbl(name: str, level: int) -> int | None:
    """The retail address of the jump table NAME's own asm/overlays/<name>.s
    uses (jtbl_LNN_Z, its own canonical level), or None if it has no .s
    file or no such reference. A joined function's table may be used in one of its pieces."""
    for part in [name] + load_joined().get(name, []):
        p = ASM_OVERLAYS / f"{part}.s"
        if not p.exists():
            continue
        for full, mm, z in JTBL_REF.findall(p.read_text()):
            if int(mm) == level:
                return int(z, 16)
    return None


class Placer:
    """Resolves relocations for one function placed at (level, address),
    including a single .rodata table it may use of its own."""

    def __init__(self, elf: ELFFile, name: str, level: int, address: int):
        self.elf = elf
        self.name = name
        self.level = level
        self.address = address
        self.text = elf.get_section_by_name(".text")
        self.rodata = elf.get_section_by_name(".rodata")
        self.symtab = elf.get_section_by_name(".symtab")
        sym = next((s for s in self.symtab.iter_symbols() if s.name == name), None)
        if sym is None or not sym["st_size"]:
            raise Unresolved(f"{name} not defined by the candidate")
        self.sym = sym
        self.off = sym["st_value"]
        self.size = sym["st_size"]
        self.base_text = address - self.off
        self.unresolved = []       # names/messages seen, in order, deduped
        self.unknown_types = []
        self.base_rodata = None
        self.rodata_error = None
        self.retail = None         # retail's bytes of the function, once known
        self.shared_lo = {}        # offset of a %hi without a %lo of its own -> the LO16 it shares

    def place_text_offset(self, toff: int, near: int) -> int:
        """Where offset TOFF of our .text lands. The assembler writes a
        reference to a function defined in the same file against the .text
        section, so it follows our file's layout; retail interleaves other
        functions (the executable's among them). So an offset inside another
        named function is placed through that function's own address."""
        owner = None
        for sym in self.symtab.iter_symbols():
            if (sym["st_shndx"] == self.sym["st_shndx"] and sym["st_size"]
                    and sym["st_value"] <= toff < sym["st_value"] + sym["st_size"]
                    and (FUNC_L.match(sym.name) or FUNC.match(sym.name))):
                owner = sym
                break
        if owner is None or owner.name == self.name:
            return self.base_text + toff
        return resolve_symbol(owner.name, self.level, near) + (toff - owner["st_value"])

    def is_text_section(self, sym) -> bool:
        return (sym["st_info"]["type"] == "STT_SECTION"
                and section_name(self.elf, sym["st_shndx"]) == ".text")

    def note_unresolved(self, what: str):
        if what not in self.unresolved:
            self.unresolved.append(what)

    def relocations(self):
        for sec in self.elf.iter_sections():
            if isinstance(sec, RelocationSection) and sec.name == ".rel.text":
                for rel in sec.iter_relocations():
                    o = rel["r_offset"]
                    if self.off <= o < self.off + self.size:
                        yield rel

    def sym_of(self, rel):
        return self.symtab.get_symbol(rel["r_info_sym"])

    def word_at(self, buf: bytes, off: int) -> int:
        return int.from_bytes(buf[off:off + 4], "little")

    def hi_los(self, orig: bytes):
        """[(hi_rel, lo_rel, hi_imm, lo_imm)] pairing every R_MIPS_LO16 in
        the function with its R_MIPS_HI16 (standard REL pairing: GNU as
        does not repeat a R_MIPS_HI16 when %hi is reused across several
        %lo's of the same symbol, so a LO16 pairs with the *last* HI16
        relocation seen for its own symbol, not a strictly one-to-one
        stack). A HI16 that reloads a value into a register without ever
        being followed by its own LO16 (the rest of its use is a bare
        0-offset access, or another HI16 for the same symbol takes the
        actual LO16) is still its own valid %hi: it comes back here paired
        with lo_rel=None, lo_imm=0 (the standalone-%hi convention)."""
        # Relocation-table order, not address order: GNU as moves each
        # R_MIPS_HI16 right before the R_MIPS_LO16 it pairs with, which is
        # what tells apart two %hi's of one symbol (two function pointers
        # into this file's .text, both relocated against the section)
        # when retail interleaves their loads.
        rels = list(self.relocations())
        all_his = sorted(({"rel": r, "sym": self.sym_of(r).name,
                           "imm": self.word_at(orig, r["r_offset"]) & 0xFFFF, "used": False}
                          for r in rels if r["r_info_type"] == R_MIPS_HI16),
                         key=lambda h: h["rel"]["r_offset"])
        his = []   # [{"rel", "sym", "imm", "used"}], in the order seen
        out = []
        for rel in rels:
            rtype = rel["r_info_type"]
            if rtype == R_MIPS_HI16:
                sym = self.sym_of(rel)
                his.append({"rel": rel, "sym": sym.name,
                            "imm": self.word_at(orig, rel["r_offset"]) & 0xFFFF, "used": False})
            elif rtype == R_MIPS_LO16:
                sym = self.sym_of(rel)
                hi = next((h for h in reversed(his) if h["sym"] == sym.name), None)
                if hi is None:
                    # No %hi of this symbol earlier in the table (a pass such
                    # as fix_orphan_hi.py can turn one into a constant): the
                    # previous address-order pairing, the nearest HI16 before
                    # this LO16, of the same symbol if there is one.
                    before = [h for h in all_his if h["rel"]["r_offset"] < rel["r_offset"]]
                    hi = next((h for h in reversed(before) if h["sym"] == sym.name), None) \
                        or (before[-1] if before else (his[-1] if his else None))
                if hi is None:
                    self.note_unresolved(f"unmatched R_MIPS_LO16 at +0x{rel['r_offset'] - self.off:x}")
                    continue
                hi["used"] = True
                lo_imm = self.word_at(orig, rel["r_offset"]) & 0xFFFF
                out.append((hi["rel"], rel, hi["imm"], lo_imm))
        los = sorted((r for r in rels if r["r_info_type"] == R_MIPS_LO16), key=lambda r: r["r_offset"])
        for h in his:
            if not h["used"]:
                # A %hi whose %lo another %hi took (two paths joining on one
                # shared %lo): REL keeps the addend's low half only in a
                # LO16, so take it from the shared one -- the next LO16 of
                # the same symbol in address order, else the last before.
                # Without it a relocation against this object's .text loses
                # the function's low 16 bits and resolves to another one
                # (func_L17_002EDE50 +0xC00, func_L15_002CCFC0).
                same = [r for r in los if self.sym_of(r).name == h["sym"]]
                after = [r for r in same if r["r_offset"] > h["rel"]["r_offset"]]
                shared = after[0] if after else (same[-1] if same else None)
                lo_imm = self.word_at(orig, shared["r_offset"]) & 0xFFFF if shared is not None else 0
                if shared is not None:
                    self.shared_lo[h["rel"]["r_offset"]] = shared
                out.append((h["rel"], None, h["imm"], lo_imm))
        return out

    def local_rodata_offset(self, sym) -> int | None:
        """SYM's own offset into .rodata (its section's own coordinates,
        ignoring any placement), or None if it doesn't live there."""
        secname = section_name(self.elf, sym["st_shndx"])
        if sym["st_info"]["type"] == "STT_SECTION":
            return 0 if secname == ".rodata" else None
        if secname == ".rodata":
            return sym["st_value"]
        return None

    def scan_rodata(self, orig: bytes):
        """Find every distinct local .rodata offset the function's
        relocations touch (base + in-place addend, in the object's own
        .rodata coordinates), and place that table if there is exactly
        one, from the function's own asm/overlays/<name>.s (jtbl_LNN_Z)."""
        if self.rodata is None:
            return
        touches = []
        for rel in self.relocations():
            rtype = rel["r_info_type"]
            if rtype in (R_MIPS_HI16, R_MIPS_LO16):
                continue    # handled via hi_los() below
            sym = self.sym_of(rel)
            local = self.local_rodata_offset(sym)
            if local is None:
                continue
            word = self.word_at(orig, rel["r_offset"])
            if rtype == R_MIPS_GPREL16:
                touches.append(local + sign16(word & 0xFFFF))
            elif rtype == R_MIPS_32:
                touches.append(local + word)
            else:
                touches.append(local)
        for hi_rel, _lo_rel, hi_imm, lo_imm in self.hi_los(orig):
            local = self.local_rodata_offset(self.sym_of(hi_rel))
            if local is None:
                continue
            ahl = (hi_imm << 16) + sign16(lo_imm)
            touches.append(local + ahl)
        distinct = sorted(set(touches))
        if not distinct:
            return
        if len(distinct) > 1:
            self.rodata_error = f"RODATA {len(distinct)} distinct .rodata references"
            return
        z = retail_jtbl(self.name, self.level)
        if z is None:
            self.rodata_error = (f"RODATA no retail table named for {self.name} "
                                  f"(local offset 0x{distinct[0]:X})")
            return
        self.base_rodata = z - distinct[0]

    def resolve_address(self, sym, near: int) -> int:
        """The placed address SYM's own value denotes (not yet combined
        with any in-place addend from the instruction). A symbol actually
        defined within this object (not SHN_UNDEF) is placed by where its
        own section landed; a section symbol IS that placement. Anything
        else -- including a defined symbol that happens to carry one of
        our naming conventions -- is placed by name (docs/OVERLAYS.md,
        "Names"), since that is how retail's own addressing works
        regardless of how this one candidate's object happens to be laid
        out (e.g. a stub's neighbour, included only to satisfy a branch).
        NEAR is the relocation's own placed address, passed through to
        resolve_symbol for its "more than one place in this level" tie
        break."""
        secname = section_name(self.elf, sym["st_shndx"])
        is_section = sym["st_info"]["type"] == "STT_SECTION"
        if is_section or (isinstance(sym["st_shndx"], int) and not (sym.name and
                          (FUNC_L.match(sym.name) or DATA_L.match(sym.name) or
                           JTBL_L.match(sym.name) or FUNC.match(sym.name) or DATA.match(sym.name)))):
            if secname == ".text":
                return self.base_text + sym["st_value"]
            if secname == ".rodata":
                if self.base_rodata is None:
                    raise Unresolved(".rodata (unplaced)")
                return self.base_rodata + sym["st_value"]
            if secname and secname.startswith(".rodata.jtbl_"):
                # tools/fix_jump_tables.py names each compiled table's
                # section after the retail table it replaces.
                return resolve_symbol(secname[len(".rodata."):], self.level, near) + sym["st_value"]
            raise Unresolved(f"section {secname or sym['st_shndx']}")
        return resolve_symbol(sym.name, self.level, near)

    def apply(self, ours: bytearray, orig: bytes):
        """Apply every relocation touching the function's bytes to OURS
        (already containing the unrelocated instruction words)."""
        for hi_rel, lo_rel, hi_imm, lo_imm in self.hi_los(orig):
            hi_off = hi_rel["r_offset"]
            try:
                s = self.resolve_address(self.sym_of(hi_rel), self.base_text + hi_off)
            except Unresolved as e:
                self.note_unresolved(str(e.what))
                continue
            ahl = (hi_imm << 16) + sign16(lo_imm)
            value = (s + ahl) & 0xFFFFFFFF
            if self.is_text_section(self.sym_of(hi_rel)):
                try:
                    value = self.place_text_offset(ahl, self.base_text + hi_off) & 0xFFFFFFFF
                except Unresolved as e:
                    self.note_unresolved(str(e.what))
                    continue
            # A function with several identical copies in this level: the
            # address is right if it is any of them, so take the copy
            # retail's %hi/%lo pair names (as for calls below). A %hi that
            # shares another one's %lo (three paths loading one callback and
            # joining on one addiu, func_L00_002C9820) reads retail's low
            # half at the shared %lo.
            named_lo = lo_rel if lo_rel is not None else self.shared_lo.get(hi_off)
            if self.retail is not None and named_lo is not None:
                r_hi = self.word_at(self.retail, hi_off - self.off) & 0xFFFF
                r_lo = self.word_at(self.retail, named_lo["r_offset"] - self.off) & 0xFFFF
                theirs = ((r_hi << 16) + sign16(r_lo)) & 0xFFFFFFFF
                if theirs != value and same_function(self.level, value, theirs):
                    value = theirs
            new_lo = value & 0xFFFF
            new_hi = ((value - sign16(new_lo)) >> 16) & 0xFFFF
            orig_hi_word = self.word_at(orig, hi_off)
            ours[hi_off:hi_off + 4] = ((orig_hi_word & 0xFFFF0000) | new_hi).to_bytes(4, "little")
            if lo_rel is not None:
                lo_off = lo_rel["r_offset"]
                orig_lo_word = self.word_at(orig, lo_off)
                ours[lo_off:lo_off + 4] = ((orig_lo_word & 0xFFFF0000) | new_lo).to_bytes(4, "little")

        for rel in self.relocations():
            rtype = rel["r_info_type"]
            if rtype in (R_MIPS_HI16, R_MIPS_LO16):
                continue    # already applied above, paired
            o = rel["r_offset"]
            word = self.word_at(orig, o)
            near = self.base_text + o
            if rtype == R_MIPS_26:
                try:
                    s = self.resolve_address(self.sym_of(rel), near)
                except Unresolved as e:
                    self.note_unresolved(str(e.what))
                    continue
                addend = (word & 0x03FFFFFF) << 2
                p = self.base_text + o
                target = ((addend + s) & 0x0FFFFFFF) | (p & 0xF0000000)
                if self.is_text_section(self.sym_of(rel)):
                    try:
                        target = self.place_text_offset(addend, near)
                    except Unresolved as e:
                        self.note_unresolved(str(e.what))
                        continue
                # A function with several identical copies in this level
                # (file-static helpers): the call is right if it reaches any
                # of them, so take the copy retail calls.
                if self.retail is not None:
                    r = self.word_at(self.retail, o - self.off)
                    theirs = ((r & 0x03FFFFFF) << 2) | (p & 0xF0000000)
                    if theirs != target and same_function(self.level, target, theirs):
                        target = theirs
                new_field = (target >> 2) & 0x03FFFFFF
                ours[o:o + 4] = ((word & 0xFC000000) | new_field).to_bytes(4, "little")
            elif rtype == R_MIPS_GPREL16:
                try:
                    s = self.resolve_address(self.sym_of(rel), near)
                except Unresolved as e:
                    self.note_unresolved(str(e.what))
                    continue
                a = sign16(word & 0xFFFF)
                value = (s + a - GP) & 0xFFFF
                ours[o:o + 4] = ((word & 0xFFFF0000) | value).to_bytes(4, "little")
            elif rtype == R_MIPS_32:
                try:
                    s = self.resolve_address(self.sym_of(rel), near)
                except Unresolved as e:
                    self.note_unresolved(str(e.what))
                    continue
                new_word = (s + word) & 0xFFFFFFFF
                if self.is_text_section(self.sym_of(rel)):
                    try:
                        new_word = self.place_text_offset(word, near) & 0xFFFFFFFF
                    except Unresolved as e:
                        self.note_unresolved(str(e.what))
                        continue
                ours[o:o + 4] = new_word.to_bytes(4, "little")
            else:
                self.unknown_types.append((o - self.off, rtype))


RESIDENT_SYM = re.compile(r"\b(?:D|func)_([0-9A-Fa-f]{8})\b")
STUB_INSN = re.compile(r"\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+(\S+)\s+(.*)")


def symbol_offsets(name: str, address: int) -> dict[int, str]:
    """Offsets where retail's assembly (asm/overlays/NAME.s) references a
    symbol: %hi/%lo operands and calls. The candidate needs a relocation at
    each: a literal address there matches this level's bytes but is wrong
    in the other levels, where the data sits elsewhere.

    Resident symbols (D_/func_XXXXXXXX below RESIDENT_MAX) sit at the same
    address in every level, so a literal is right for them; our assembler
    leaves one when it fills a branch delay slot with a copy of the
    target's `lui` (func_L00_00212790). They are not listed."""
    p = ROOT / f"asm/overlays/{name}.s"
    out = {}
    if not p.exists():
        return out
    for line in p.read_text(errors="replace").splitlines():
        m = STUB_INSN.match(line)
        if not m:
            continue
        op, args = m.group(2), m.group(3)
        if "%hi(" in args or "%lo(" in args or (op in ("jal", "j") and args.strip().startswith("func_")):
            r = RESIDENT_SYM.search(args)
            if r and int(r.group(1), 16) < RESIDENT_MAX:
                continue
            out[int(m.group(1), 16) - address] = f"{op} {args.strip()}"
    return out


def show_size_diff(elf, placer, level: int, address: int, csize: int, limit: int = 80) -> None:
    """For a SIZE verdict with --diff: our instructions aligned against
    retail's by mnemonic (difflib), printing only the stretches that
    differ, so a long function shows where it has instructions too many or
    too few. Relocations are not applied: operands of calls and %hi/%lo
    halves may read as zero on our side."""
    import difflib
    manifest = level_manifest(level)
    raw = (DUMP / f"level_{level:02d}/text.bin").read_bytes()
    base = text_record(manifest)["address"]
    retail = raw[address - base: address - base + csize]
    ours = elf.get_section_by_name(".text").data()[placer.off:placer.off + placer.size]

    def listing(data):
        return [rz.Instruction(int.from_bytes(data[i:i + 4], "little"), vram=address + i,
                               category=rz.InstrCategory.R5900).disassemble() for i in range(0, len(data) - 3, 4)]
    a, b = listing(ours), listing(retail)
    printed = 0
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, [x.split()[0] for x in a],
                                                       [x.split()[0] for x in b], autojunk=False).get_opcodes():
        if tag == "equal":
            continue
        print(f"  {tag}: ours +{i1 * 4:x}..+{i2 * 4:x}  retail +{j1 * 4:x}..+{j2 * 4:x}")
        for k in range(max(i2 - i1, j2 - j1)):
            if printed >= limit:
                print("  ... (more differences)")
                return
            oa = a[i1 + k] if i1 + k < i2 else "-"
            ob = b[j1 + k] if j1 + k < j2 else "-"
            print(f"      ours {oa:40s} retail {ob}")
            printed += 1


def check(obj_path, name: str, show: bool = False) -> str:
    m = OVERLAY_NAME.match(name)
    if not m:
        return f"LINK not an overlay function name: {name}"
    level, address = int(m.group(1)), int(m.group(2), 16)

    catalogue = load_catalogue()
    row = catalogue.get(name)
    if row is None:
        return f"LINK {name} is not in the catalogue"
    _kind, csize, _places = row
    csize = joined_size(name, csize, catalogue)

    elf = ELFFile(open(obj_path, "rb"))
    try:
        placer = Placer(elf, name, level, address)
    except Unresolved as e:
        return f"LINK {e.what}"

    if placer.size != csize:
        if show:
            show_size_diff(elf, placer, level, address, csize)
        return f"SIZE ours {placer.size} / retail {csize}"

    manifest = level_manifest(level)
    trec = text_record(manifest)
    text_base = trec["address"]
    raw = (DUMP / f"level_{level:02d}/text.bin").read_bytes()
    retail = raw[address - text_base: address - text_base + csize]

    text_bytes = elf.get_section_by_name(".text").data()
    padded_orig = bytearray(text_bytes)  # keep absolute offsets for relocation math

    placer.scan_rodata(bytes(padded_orig))
    if placer.rodata_error:
        return placer.rodata_error

    ours = bytearray(padded_orig)
    placer.retail = retail
    placer.apply(ours, bytes(padded_orig))

    if placer.unresolved or placer.unknown_types:
        parts = list(placer.unresolved)
        parts += [f"unknown relocation type {t} at +0x{o:x}" for o, t in placer.unknown_types]
        return "LINK " + ", ".join(parts)

    ours_func = bytes(ours[placer.off:placer.off + placer.size])
    diff = sum(1 for a, b in zip(ours_func, retail) if a != b)
    if diff == 0:
        relocated = {rel["r_offset"] - placer.off for rel in placer.relocations()}
        missing = sorted(set(symbol_offsets(name, address)) - relocated)
        if missing:
            refs = symbol_offsets(name, address)
            return ("LINK literal address where retail references a symbol: "
                    + ", ".join(f"+0x{o:x} ({refs[o]})" for o in missing[:4]))
        return "EXACT"
    if show:
        vram = address
        for i in range(0, csize, 4):
            a = int.from_bytes(ours_func[i:i + 4], "little")
            b = int.from_bytes(retail[i:i + 4], "little")
            if a != b:
                da = rz.Instruction(a, vram=vram + i, category=rz.InstrCategory.R5900).disassemble()
                db = rz.Instruction(b, vram=vram + i, category=rz.InstrCategory.R5900).disassemble()
                print(f"  +{i:4x}  ours {da:40s} retail {db}")
    return f"BYTES {diff}/{csize}"


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    obj_path, name = sys.argv[1], sys.argv[2]
    print(check(obj_path, name, "--diff" in sys.argv))


if __name__ == "__main__":
    main()
