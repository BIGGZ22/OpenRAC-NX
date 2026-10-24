#!/usr/bin/env python3
"""
Check the C in src/ against another version, function by function (docs/VERSIONS.md).

  python3 tools/version_check.py --version us            # config/versions/us/map.tsv
  python3 tools/version_check.py --version us --func func_0012ABCD --diff
  python3 tools/version_check.py --version pal --self-test

The C is compiled once, by the primary build (tools/build_sn.sh for the executable's objects in
build-sn/{core,game,libgcc}, tools/gen_progress_report.py for every file of src/overlays/ in
build-sn/overlays/report/), with each file's own flags (config/file_cflags.txt): a file builds to
the same object whatever version it is checked against, so a function matches another version
only if the code retail has there is what that file's C compiles to. Version-specific C
(src/versions/<K>/, only where the bytes differ) is compiled the same way by this tool
(tools/try_func.py's pipeline and the flags of the file it stands in for).

Each C function is placed where the version has its counterpart (config/versions/<K>/functions.tsv)
and every relocation in it is resolved and applied as the primary's checkers do
(tools/try_func.py's resolve_relocations for the executable, tools/overlay_check.py's Placer and its
%hi/%lo pairing for a level), giving the primary version's address of what it refers to; that
address is then translated to the version's (the table below), and the relocated bytes are compared
with the version's bytes at the function's place, byte for byte. References into the object's own
.rodata (a switch's table, a float constant) stay masked, as try_func.py masks them in the
executable. EXACT is the stock checkers' verdict on that comparison.

The translation table is built from the code the two versions share: every pair of counterparts
places a function (an address inside one moves with it), and every pair whose instructions are the
same apart from address fields (`same` in functions.tsv) pairs each call target, each $gp offset
and each %hi/%lo address they encode. Each program has its own table (the executable's addresses,
and per level its main-segment ones; addresses below the main segment are the resident ones). An
address no pair names takes the offset of the nearest one that is named.

map.tsv (committed; names, places, verdicts): name, source file, status, place, size, closeness:
  match     the C builds to the version's bytes (EXACT)
  differs   the counterpart's code is not the same (same=no): the C cannot build both; it needs
            version-specific C in src/versions/<K>/
  refs      the same instructions, but some refer to another function or datum here than the
            translation gives (a call to code this version changed, a table that moved): code that
            differs in what it references, so it needs version-specific C like `differs`
  nomatch   the C could not be placed or sized here (LINK, SIZE): a tool problem to look at
  asm       original assembly (handwritten, linker remnant) of the primary whose counterpart has
            the same bytes: finished here too
  absent    no counterpart in this version
  copy      version-specific C for a function whose shared C matches here too (the sync check fails)
"""
import argparse
import bisect
import collections
import functools
import glob
import os
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
os.chdir(ROOT)
import versions  # noqa: E402
import version_map  # noqa: E402

PAL_MAIN = 0x15F000          # where the primary's main segment (the part a level replaces) starts
FUNC = re.compile(r"^func_([0-9A-F]{8})$")
FUNC_L = re.compile(r"^func_L(\d\d)_([0-9A-F]{8})$")
MAP_HEAD = """# {label} ({id}): every C function of src/ (and the primary's original assembly) checked against this
# version (tools/version_check.py; docs/VERSIONS.md). status: match (EXACT), differs (the code here is not the
# same: needs version-specific C), nomatch (the same code, but the C does not build to it), asm (original
# assembly with the same bytes here), absent (no counterpart). place: boot:ADDR or NN:ADDR.
# name\tsource\tstatus\tplace\tsize\tcloseness
"""


# --- the primary's side ------------------------------------------------------------------------

@functools.lru_cache(None)
def primary_places():
    """{name: [(space, address)]} of the primary's functions: space 'boot' for the executable's,
    the level number for a place in a level."""
    import overlays as ov
    out = collections.defaultdict(list)
    for name, addr, size, _b in ov.exe_functions(everything=True):
        out[name].append(("boot", addr))
    for name, _kind, _size, places in ov.read_catalogue():
        out[name] += [(lv, a) for lv, a in places]
    return out


@functools.lru_cache(None)
def primary_sizes():
    import overlays as ov
    out = {name: size for name, _a, size, _b in ov.exe_functions(everything=True)}
    for name, _kind, size, _p in ov.read_catalogue():
        out.setdefault(name, size)
    return out


def code_bytes(v, space, addr, size):
    if space == "boot":
        secs = versions.boot_sections(v)
        for s in ("core.text", ".text"):
            base, data = secs[s]
            if base <= addr < base + len(data):
                return data[addr - base:addr - base + size]
        raise KeyError(hex(addr))
    base, data = text_of(v, space)
    return data[addr - base:addr - base + size]


@functools.lru_cache(None)
def text_of(v, lid):
    return versions.level_text(v, lid)


# --- the translation table ---------------------------------------------------------------------

class Table:
    """Primary address -> this version's address, per space ('boot' or a level)."""

    def __init__(self, v):
        self.v = v
        self.funcs = collections.defaultdict(list)     # space -> [(pal start, size, x start)]
        self.obs = collections.defaultdict(collections.Counter)   # space -> Counter((pal, x))
        self.p_gp, self.x_gp = versions.gp_value(versions.PRIMARY), versions.gp_value(v)
        self.x_main = versions.level_records(v, 0)[1][0]["address"]   # the lit record: the main segment's start
        self.build()

    def space_of(self, ctx, addr):
        return ctx if ctx != "boot" and addr >= PAL_MAIN else "boot"

    def build(self):
        pp = primary_places()
        psz = primary_sizes()
        entries = version_map.read(self.v) if not self.v.primary else None
        if self.v.primary:
            for name, places in pp.items():
                for sp, a in places:
                    self.funcs[sp].append((a, psz[name], a))
            for sp in self.funcs:
                self.funcs[sp].sort()
            return
        for e in entries:
            c = e["counterpart"]
            if c is None or c not in pp:
                continue
            for sp in {s for s, _ in e["places"]}:
                xs = sorted(a for s, a in e["places"] if s == sp)
                ps = sorted(a for s, a in pp[c] if s == sp)
                for pa, xa in zip(ps, xs):
                    self.funcs[sp].append((pa, psz[c], xa))
                    if e["same"] and e["name"] == c and psz[c] == e["size"]:
                        self.observe(sp, pa, xa, e["size"])
        for sp in self.funcs:
            self.funcs[sp].sort()
        self.sorted_obs = {}
        for sp, cnt in self.obs.items():
            best = {}
            for (pa, xa), n in cnt.items():
                if pa not in best or n > best[pa][1]:
                    best[pa] = (xa, n)
            keys = sorted(best)
            self.sorted_obs[sp] = (keys, [best[k][0] - k for k in keys])

    def observe(self, sp, pa, xa, size):
        pb = code_bytes(versions.PRIMARY, sp, pa, size)
        xb = code_bytes(self.v, sp, xa, size)
        pw, xw = versions_words(pb), versions_words(xb)
        hi_p, hi_x = {}, {}
        sext = lambda i: i - 0x10000 if i & 0x8000 else i
        for i, (p, x) in enumerate(zip(pw, xw)):
            op, rs, rt, rd = p >> 26, (p >> 21) & 31, (p >> 16) & 31, (p >> 11) & 31
            if op in (2, 3):
                pt = ((p & 0x3FFFFFF) << 2) | ((pa + 4 * i) & 0xF0000000)
                xt = ((x & 0x3FFFFFF) << 2) | ((xa + 4 * i) & 0xF0000000)
                self.obs[self.space_of(sp, pt)][(pt, xt)] += 1
                continue
            if op == 0x0F:
                hi_p[rt], hi_x[rt] = (p & 0xFFFF) << 16, (x & 0xFFFF) << 16
                continue
            if rs == 28 and op >= 8 and op != 0x0F:
                pt, xt = self.p_gp + sext(p & 0xFFFF), self.x_gp + sext(x & 0xFFFF)
                self.obs[self.space_of(sp, pt)][(pt, xt)] += 1
            elif rs in hi_p and (op in (0x09, 0x19) or 0x20 <= op < 0x40):
                pt, xt = (hi_p[rs] + sext(p & 0xFFFF)) & 0xFFFFFFFF, (hi_x[rs] + sext(x & 0xFFFF)) & 0xFFFFFFFF
                if 0x100000 <= pt < 0x2000000:
                    self.obs[self.space_of(sp, pt)][(pt, xt)] += 1
            # what this instruction writes no longer holds a %hi
            if op == 0:
                hi_p.pop(rd, None)
                hi_x.pop(rd, None)
            elif op < 0x28 or op in (0x31, 0x35, 0x36, 0x37):
                if not (0x28 <= op < 0x30):
                    hi_p.pop(rt, None)
                    hi_x.pop(rt, None)

    def inverse(self, ctx, x):
        """The primary's address that this version's X translates from, in CTX, or None."""
        if not hasattr(self, "inv"):
            self.inv = {}
            for sp, (keys, deltas) in self.sorted_obs.items():
                pairs = sorted((k + d, d) for k, d in zip(keys, deltas))
                self.inv[sp] = ([a for a, _ in pairs], [d for _, d in pairs])
            for sp, lst in self.funcs.items():
                pass
        for sp in ([ctx, "boot"] if ctx != "boot" else ["boot"]):
            for pa, size, xa in self.funcs.get(sp, []):
                if xa <= x < xa + max(size, 4) and self(ctx, pa + (x - xa)) == x:
                    return pa + (x - xa)
            xs, ds = self.inv.get(sp, ([], []))
            k = bisect.bisect_left(xs, x)
            for j in (k, k - 1, k + 1):
                if 0 <= j < len(xs):
                    p = x - ds[j]
                    if self(ctx, p) == x:
                        return p
        return None

    def same_function(self, ctx, a, b):
        """Whether A and B (this version's addresses) are two places of one function."""
        if not hasattr(self, "owners"):
            self.owners = collections.defaultdict(dict)
            ents = primary_entries().values() if self.v.primary else version_map.read(self.v)
            for e in ents:
                for sp, x in e["places"]:
                    self.owners[sp][x] = e["name"]
        sp_a = ctx if ctx != "boot" and a >= self.x_main else "boot"
        sp_b = ctx if ctx != "boot" and b >= self.x_main else "boot"
        o = self.owners[sp_a].get(a)
        return o is not None and sp_a == sp_b and o == self.owners[sp_b].get(b)

    def func(self, sp, addr):
        lst = self.funcs.get(sp, [])
        k = bisect.bisect_right(lst, (addr, 1 << 40, 0)) - 1
        while k >= 0 and lst[k][0] == addr and k > 0 and lst[k - 1][0] == addr:
            k -= 1
        if k >= 0:
            pa, size, xa = lst[k]
            if pa <= addr < pa + max(size, 4):
                return xa + (addr - pa)
        return None

    def __call__(self, ctx, addr):
        """This version's address for the primary's ADDR, as code in CTX ('boot' or a level)
        refers to it."""
        if self.v.primary:
            return addr
        sp = self.space_of(ctx, addr)
        f = self.func(sp, addr)
        if f is not None:
            return f
        keys, deltas = self.sorted_obs.get(sp, ([], []))
        if not keys:
            return None
        k = bisect.bisect_left(keys, addr)
        if k < len(keys) and keys[k] == addr:
            return addr + deltas[k]
        lo = k - 1 if k else None
        hi = k if k < len(keys) else None
        if lo is not None and hi is not None:
            if deltas[lo] == deltas[hi]:
                return addr + deltas[lo]
            # a symbol's fields are named at or above its start: prefer the one above when close
            return addr + (deltas[hi] if keys[hi] - addr <= 0x40 else deltas[lo])
        return addr + deltas[lo if lo is not None else hi]


def versions_words(b):
    return struct.unpack("<%dI" % (len(b) // 4), b[:len(b) // 4 * 4])


# --- the objects ---------------------------------------------------------------------------------

@functools.lru_cache(None)
def object_index():
    """{symbol name: [(object path, value, size)]} of every sized function symbol in the build's
    relocatable objects."""
    from elftools.elf.elffile import ELFFile
    paths = [p for d in ("core", "game", "libgcc") for p in glob.glob(f"build-sn/{d}/**/*.o", recursive=True)]
    paths += glob.glob("build-sn/overlays/report/**/obj.o", recursive=True)
    paths += glob.glob("build-sn/versions/*/src/**/obj.o", recursive=True)
    out = collections.defaultdict(list)
    for p in sorted(paths):
        try:
            with open(p, "rb") as f:
                elf = ELFFile(f)
                st = elf.get_section_by_name(".symtab")
                if st is None:
                    continue
                for s in st.iter_symbols():
                    if s["st_info"]["type"] == "STT_FUNC" and s["st_size"] and isinstance(s["st_shndx"], int):
                        if elf.get_section(s["st_shndx"]).name == ".text":
                            out[s.name].append((p, s["st_value"], s["st_size"]))
        except Exception:
            continue
    return out


@functools.lru_cache(None)
def linked_symbols():
    """{name: address} of every defined symbol of the primary's linked image (build-sn/rac1.elf):
    the names the executable's C uses that carry no address (memset, libgcc's own names)."""
    from elftools.elf.elffile import ELFFile
    out = {}
    with open("build-sn/rac1.elf", "rb") as f:
        for s in ELFFile(f).get_section_by_name(".symtab").iter_symbols():
            if s.name and s["st_shndx"] != "SHN_UNDEF":
                out.setdefault(s.name, s["st_value"])
    return out


@functools.lru_cache(None)
def linked_real_names():
    """{func_ name: the sized symbol at its address in the linked image} (libgcc's aliases)."""
    from elftools.elf.elffile import ELFFile
    out = {}
    if not Path("build-sn/rac1.elf").exists():
        return out
    with open("build-sn/rac1.elf", "rb") as f:
        syms = list(ELFFile(f).get_section_by_name(".symtab").iter_symbols())
    sized = {}
    for s in syms:
        if s["st_size"] and s["st_info"]["type"] == "STT_FUNC":
            sized.setdefault(s["st_value"], s.name)
    for s in syms:
        if FUNC.match(s.name) and not s["st_size"] and s["st_value"] in sized:
            out[s.name] = sized[s["st_value"]]
    return out


# --- relocating one function -----------------------------------------------------------------------

EXTRA_OBJECTS = {}        # name -> object to check it from (tools/version_src.py try)


class Mask(Exception):
    pass


def relocate(obj, name, ctx, pal_addr, xlate, x_gp, theirs=None):
    """(this version's bytes of NAME from OBJ placed at the primary's PAL_ADDR and translated,
    the offsets of masked bytes, the function's size, unresolved names)."""
    import overlay_check as oc
    import try_func
    from elftools.elf.elffile import ELFFile
    elf = ELFFile(open(obj, "rb"))
    level = ctx if ctx != "boot" else 0
    P = oc.Placer(elf, name, level, pal_addr)
    orig = elf.get_section_by_name(".text").data()
    ours = bytearray(orig)
    masked, unresolved = set(), []
    sext = oc.sign16

    def named(n, near):
        if ctx == "boot":
            a = try_func.symbol_address(n)
            if a is None:
                a = linked_symbols().get(n)
            if a is None:
                raise oc.Unresolved(n)
            return a
        try:
            return oc.resolve_symbol(n, level, near)
        except oc.Unresolved:
            a = linked_symbols().get(n)
            if a is None or a >= PAL_MAIN:
                raise
            return a

    def owner_offset(toff, near):
        """A .text section offset: through the function that holds it in this object."""
        for s in P.symtab.iter_symbols():
            if (s["st_shndx"] == P.sym["st_shndx"] and s["st_size"]
                    and s["st_value"] <= toff < s["st_value"] + s["st_size"]
                    and (oc.FUNC_L.match(s.name) or oc.FUNC.match(s.name))):
                if s.name == name:
                    break
                return named(s.name, near) + (toff - s["st_value"])
        return P.base_text + toff

    tag = getattr(getattr(xlate, "v", None), "tag", "")
    direct = re.compile(r"^(?:func|D|jtbl)_%s_(?:L\d\d_)?([0-9A-F]{8})$" % tag) if tag else None

    def value_of(sym, near):
        sec = oc.section_name(elf, sym["st_shndx"])
        if direct and sym.name and direct.match(sym.name):
            # code or data only this version has (func_<TAG>_..., D_<TAG>_...): its own address
            return ("direct", int(direct.match(sym.name).group(1), 16))
        conv = sym.name and (oc.FUNC_L.match(sym.name) or oc.DATA_L.match(sym.name) or oc.JTBL_L.match(sym.name)
                             or oc.FUNC.match(sym.name) or oc.DATA.match(sym.name))
        if sym["st_info"]["type"] == "STT_SECTION" or (isinstance(sym["st_shndx"], int) and not conv):
            if sec == ".text":
                return ("text", sym["st_value"])
            if sec and sec.startswith(".rodata.jtbl_"):
                return ("abs", named(sec[len(".rodata."):], near) + sym["st_value"])
            raise Mask()
        return ("abs", named(sym.name, near))

    def target(sym, addend, near):
        kind, val = value_of(sym, near)
        if kind == "direct":
            return (val + addend) & 0xFFFFFFFF
        pal = owner_offset(val + addend, near) if kind == "text" else val + addend
        x = xlate(ctx, pal & 0xFFFFFFFF)
        if x is None:
            raise oc.Unresolved("no address in this version for 0x%X" % pal)
        return x

    word = lambda o: int.from_bytes(orig[o:o + 4], "little")
    touched = set()
    fields = []           # (kind, offsets, symbol name, this version's address it gets): version_clone.py

    def put(o, w):
        ours[o:o + 4] = (w & 0xFFFFFFFF).to_bytes(4, "little")
        touched.update(range(o, o + 4))
    x_base = xlate(ctx, pal_addr)
    for hi_rel, lo_rel, hi_imm, lo_imm in P.hi_los(orig):
        ho = hi_rel["r_offset"]
        try:
            value = target(P.sym_of(hi_rel), (hi_imm << 16) + sext(lo_imm), P.base_text + ho)
        except Mask:
            masked.update((ho, ho + 1))
            if lo_rel is not None:
                masked.update((lo_rel["r_offset"], lo_rel["r_offset"] + 1))
            continue
        except oc.Unresolved as e:
            unresolved.append(str(e.what))
            continue
        named_lo = lo_rel if lo_rel is not None else P.shared_lo.get(ho)
        if theirs is not None and named_lo is not None:
            r_hi = int.from_bytes(theirs[ho - P.off:ho - P.off + 4], "little") & 0xFFFF
            r_lo = int.from_bytes(theirs[named_lo["r_offset"] - P.off:named_lo["r_offset"] - P.off + 4], "little") & 0xFFFF
            rv = ((r_hi << 16) + sext(r_lo)) & 0xFFFFFFFF
            if rv != value and xlate.same_function(ctx, value, rv):
                value = rv
        fields.append(("hilo", (ho - P.off, (named_lo["r_offset"] - P.off) if named_lo is not None else None),
                       P.sym_of(hi_rel).name, value, value - target(P.sym_of(hi_rel), 0, P.base_text + ho)))
        lo = value & 0xFFFF
        put(ho, (word(ho) & 0xFFFF0000) | (((value - sext(lo)) >> 16) & 0xFFFF))
        if lo_rel is not None:
            # the low half is the LO16's own symbol's: retail's assembler can pair a %lo with
            # another symbol's %hi when the two share the high half (989snd.o)
            lsym = P.sym_of(lo_rel)
            if lsym.name != P.sym_of(hi_rel).name or lsym["st_shndx"] != P.sym_of(hi_rel)["st_shndx"]:
                try:
                    lo = target(lsym, sext(lo_imm), P.base_text + lo_rel["r_offset"]) & 0xFFFF
                except Mask:
                    masked.update((lo_rel["r_offset"], lo_rel["r_offset"] + 1))
                    continue
                except oc.Unresolved as e:
                    unresolved.append(str(e.what))
                    continue
            put(lo_rel["r_offset"], (word(lo_rel["r_offset"]) & 0xFFFF0000) | lo)
    for rel in P.relocations():
        t, o = rel["r_info_type"], rel["r_offset"]
        if t in (oc.R_MIPS_HI16, oc.R_MIPS_LO16):
            continue
        w = word(o)
        near = P.base_text + o
        try:
            if t == oc.R_MIPS_26:
                tg = target(P.sym_of(rel), (w & 0x03FFFFFF) << 2, near)
                # a function with several identical copies in the program: the call is right
                # if it reaches any of them, so take the copy retail calls (as overlay_check.py)
                if theirs is not None and o - P.off + 4 <= len(theirs):
                    r = int.from_bytes(theirs[o - P.off:o - P.off + 4], "little")
                    rt = ((r & 0x03FFFFFF) << 2) | (tg & 0xF0000000)
                    if rt != tg and xlate.same_function(ctx, tg, rt):
                        tg = rt
                put(o, (w & 0xFC000000) | ((tg >> 2) & 0x03FFFFFF))
                fields.append(("j", (o - P.off,), P.sym_of(rel).name, tg, 0))
            elif t == oc.R_MIPS_GPREL16:
                tg = target(P.sym_of(rel), sext(w & 0xFFFF), near)
                fields.append(("gp", (o - P.off,), P.sym_of(rel).name, tg, tg - target(P.sym_of(rel), 0, near)))
                put(o, (w & 0xFFFF0000) | ((tg - x_gp) & 0xFFFF))
            elif t == oc.R_MIPS_32:
                put(o, target(P.sym_of(rel), w, near))
            else:
                unresolved.append("relocation type %d" % t)
        except Mask:
            masked.update(range(o, o + 4) if t in (oc.R_MIPS_26, oc.R_MIPS_32) else (o, o + 1))
        except oc.Unresolved as e:
            unresolved.append(str(e.what))
    relocate.touched = {m - P.off for m in touched}
    relocate.fields = fields
    return bytes(ours[P.off:P.off + P.size]), {m - P.off for m in masked}, P.size, unresolved


# --- the check -------------------------------------------------------------------------------------

def c_functions():
    """{name: source file} of every C function of src/ (the report's lists)."""
    import gen_progress_report as g
    import libgcc_units
    import srcnames
    out = {}
    for seg, srcs in libgcc_units.SEGMENT_SOURCES.items():
        for src in srcs:
            text = srcnames.read_c(src, errors="replace")
            stubs = set(g.STUB.findall(text))
            for n in g.FUNC_DEF.findall(text):
                if n not in stubs:
                    out.setdefault(n, src)
    for mod, src, fns, _stubs in libgcc_units.MODULES:
        for n in fns:
            out.setdefault(n, src)
    file_map, _ = g.overlay_c_functions()
    for path, fns in file_map.items():
        for n, is_c in fns:
            if is_c:
                out.setdefault(n, str(path))
    return out


def original_asm():
    """Names of the primary's original assembly (handwritten, linker remnants), executable and levels."""
    import gen_progress_report as g
    return set(g.classified_asm()) | g.overlay_remnants() | g.overlay_handwritten()


def joined():
    import gen_progress_report as g
    return g.overlay_joined()


def canonical_place(e):
    places = sorted(e["places"], key=lambda p: (-1 if p[0] == "boot" else p[0], p[1]))
    return places[0]


def check_one(v, table, name, src, entry, show=False):
    """(status, closeness) of NAME's C against V at ENTRY's place."""
    # the first place of this version's in a program where the primary has the function too
    # (an entry can be the executable's here and a level's there)
    mine = sorted(entry["places"], key=lambda p: (-1 if p[0] == "boot" else p[0], p[1]))
    theirs_sp = {p[0] for p in primary_places().get(name, [])}
    sp, xa = next((p for p in mine if p[0] in theirs_sp), mine[0])
    canon = min(primary_places().get(name, [(sp, 0)]), key=lambda p: (-1 if p[0] == "boot" else p[0], p[1]))[0]
    if sp != canon and not v.primary:
        # the C names its symbols as its canonical place (the executable, or its lowest level) sees
        # them; this version has the code only elsewhere, its copy there is another function
        return "differs", 0.0, f"the code differs at the primary's canonical place ({canon})"
    pp = [p for p in primary_places().get(name, []) if p[0] == sp]
    if not pp:
        # the counterpart is not at the same kind of place (an executable function in a level)
        return "nomatch", 0.0, "no place of the primary's in the same program"
    pal_addr = sorted(a for _, a in pp)[0]
    idx = object_index()
    real = linked_real_names().get(name, name)
    cands = idx.get(real, []) or idx.get(name, [])
    if name in EXTRA_OBJECTS:
        cands = [(EXTRA_OBJECTS[name], 0, 0)]
    elif src.startswith("src/versions/"):
        cands = [c for c in cands if c[0].startswith(f"build-sn/versions/{v.key}/src/")]
    else:
        cands = [c for c in cands if not c[0].startswith("build-sn/versions/")]
    if real != name:
        cands = [c for c in cands if "libgcc" in c[0]]
    if not cands:
        return "nomatch", 0.0, "no object defines it"
    best = None
    for obj, _val, size in cands:
        try:
            ours, masked, size, unresolved = relocate(obj, real, sp, pal_addr, table, table.x_gp,
                                                      code_bytes(v, sp, xa, entry["size"] + 64))
        except Exception as ex:          # noqa: BLE001  (an object this tool cannot place)
            best = best or ("nomatch", 0.0, f"{type(ex).__name__}: {ex}")
            continue
        want = joined_size(v, name, entry, size)
        theirs = code_bytes(v, sp, xa, want)
        if size != want:
            r = ("differs", 0.0, f"SIZE ours {size} / here {want}")
        elif unresolved:
            r = ("nomatch", 0.0, "LINK " + ", ".join(sorted(set(unresolved))[:4]))
        else:
            diff = [i for i in range(size) if i not in masked and ours[i] != theirs[i]]
            if not diff:
                return "match", 100.0, f"EXACT ({len(masked)} bytes masked)" if masked else "EXACT"
            # only in relocated fields: an address this tool's table gets wrong; anywhere else:
            # the code is not the same (a field offset, a constant the identity did not compare)
            kind = "refs" if set(diff) <= relocate.touched else "differs"
            r = (kind, 100.0 * (size - len(diff)) / size, f"BYTES {len(diff)}/{size}")
            if show:
                import rabbitizer as rz
                for i in range(0, size, 4):
                    if any(k in diff for k in range(i, i + 4)):
                        a = int.from_bytes(ours[i:i + 4], "little")
                        b = int.from_bytes(theirs[i:i + 4], "little")
                        da = rz.Instruction(a, vram=xa + i, category=rz.InstrCategory.R5900).disassemble()
                        db = rz.Instruction(b, vram=xa + i, category=rz.InstrCategory.R5900).disassemble()
                        print(f"  +{i:4x}  ours {da:40s} {v.key} {db}")
        if best is None or (r[0] == "refs", r[1]) > (best[0] == "refs", best[1]):
            best = r
    return best


def joined_size(v, name, entry, csize):
    """The bytes NAME's C covers here: its own entry's and those of the pieces joined to it
    (config/overlays/joined.tsv) that follow it at its place."""
    pieces = joined().get(name)
    if not pieces:
        return entry["size"]
    return csize


def entries_by_name(v):
    return {e["name"]: e for e in version_map.read(v)}


def run(v, only=None, show=False):
    table = Table(v)
    names = c_functions()
    overrides = version_sources(v)
    asm = original_asm()
    ents = entries_by_name(v) if not v.primary else primary_entries()
    rows = []
    counts = collections.Counter()
    for name in sorted(set(names) | set(overrides) | asm):
        if only and name not in only:
            continue
        e = ents.get(name)
        src = overrides.get(name) or names.get(name) or "-"
        if e is None:
            status, close, note = "absent", 0.0, ""
        elif name in asm and name not in names and name not in overrides:
            status, close, note = ("asm", 100.0, "") if e["same"] else ("differs", 0.0, "")
        elif not e["same"] and name not in overrides:
            status, close, note = "differs", 0.0, ""
        else:
            status, close, note = check_one(v, table, name, src, e, show)
            if name in overrides and name in names:
                # version-specific C only where the shared C cannot build this version's bytes
                shared = check_one(v, table, name, names[name], e)
                if shared[0] == "match":
                    status, note = "copy", "the shared C in %s matches here too" % names[name]
        if only:
            print(f"{name}: {status} {note} ({src})")
        counts[status] += 1
        place = "-" if e is None else fmt_place(canonical_place(e))
        rows.append((name, src, status, place, e["size"] if e else 0, close))
    # a joined function's later pieces are finished with it
    for owner, pieces in joined().items():
        st = next((r[2] for r in rows if r[0] == owner), None)
        if st is None:
            continue
        have = {r[0] for r in rows}
        for p in pieces:
            if p in ents and p not in have:
                piece = "joined" if st in ("match", "asm") and ents[p]["same"] else ("differs" if st != "absent" else st)
                rows.append((p, "-", piece, fmt_place(canonical_place(ents[p])), ents[p]["size"],
                             100.0 if piece == "joined" else 0.0))
                counts[piece] += 1
    return rows, counts


def primary_entries():
    """The primary version as functions.tsv entries (its own counterpart, the same code)."""
    out = {}
    psz = primary_sizes()
    for name, places in primary_places().items():
        out[name] = {"name": name, "size": psz[name], "same": True, "counterpart": name, "places": places}
    return out


def fmt_place(p):
    return f"boot:{p[1]:08X}" if p[0] == "boot" else f"{p[0]:02d}:{p[1]:08X}"


def version_sources(v):
    """{name: file} of the version-specific C in src/versions/<K>/ (tools/version_src.py)."""
    import version_src
    return {n: str(p.relative_to(ROOT)) for n, p in version_src.overrides(v).items()}


def map_path(v):
    return v.config / "map.tsv"


def write_map(v, rows):
    rows = sorted(rows)
    map_path(v).write_text(MAP_HEAD.format(label=v.label, id=v.id) + "".join(
        f"{n}\t{s}\t{st}\t{p}\t{z}\t{c:.2f}\n" for n, s, st, p, z, c in rows))


def read_map(v):
    out = {}
    for line in map_path(v).read_text().splitlines():
        if line and not line.startswith("#"):
            n, s, st, p, z, c = line.split("\t")
            out[n] = {"source": s, "status": st, "place": p, "size": int(z), "closeness": float(c)}
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", required=True)
    ap.add_argument("--func", action="append")
    ap.add_argument("--diff", action="store_true")
    ap.add_argument("--self-test", action="store_true", help="the primary against itself: every C function must match")
    a = ap.parse_args()
    v = versions.get(a.version)
    rows, counts = run(v, set(a.func) if a.func else None, a.diff)
    print(f"{v.key}: " + ", ".join(f"{n} {k}" for k, n in counts.most_common()))
    if a.self_test:
        bad = [r for r in rows if r[2] not in ("match", "asm", "joined")]
        for r in bad:
            print("  ", r)
        sys.exit(1 if bad else 0)
    if not a.func and not v.primary:
        write_map(v, rows)
        print(f"wrote {map_path(v).relative_to(ROOT)}")


if __name__ == "__main__":
    main()
