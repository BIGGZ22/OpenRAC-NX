#!/usr/bin/env python3
"""
Is a candidate the same program as retail, apart from the compiler's
choices? A check for the port, not for matching. Level functions and the
executable's alike.

  bash tools/docker/run.sh python tools/equiv.py FUNC CANDIDATE.c [--show]
  bash tools/docker/run.sh python tools/equiv.py --staged [--jobs N]   every staged near miss

A near miss can be wrong (a constant, a test, an argument, a call) or right
with other registers and another instruction order, and its byte count does
not say which. This builds the candidate in its file as tools/try_func.py
does, relocates it as tools/overlay_check.py does, and compares the two
functions as bags of normalised operations:

- scratch registers are not compared (which register holds a value is the
  allocator's choice), nor is the order of instructions (the scheduler's);
- moves are dropped, and the value a move carries keeps its meaning: the
  instruction that computed an argument of a call is marked with that
  argument's register, the one that computed the result with `ret`;
- constants and addresses are compared: a register known to hold one (lui,
  li, an address formed from $gp or a lui pair) shows as its value wherever
  it is used, so a global reached through $gp and the same global reached
  through a lui pair are one access;
- stack slots are not compared (the frame's layout is the allocator's), and
  neither are the saves and restores of callee-saved registers;
- branch-likely is read as the plain branch (reorg's choice), and a branch's
  target is not compared, only its condition.

Verdicts: EXACT (try_func's own), FUNCTIONAL (the same operations),
NEAR (the same operations, but an argument value is matched to another
call, or an operation is duplicated: a delay slot filled by copying, a
constant rebuilt instead of kept; often a value spilled and reloaded,
which this cannot follow everywhere; read the --show list),
DIFFERENT (an operation one has and the other lacks: a real difference to
fix before the port can use it). It is a static comparison with known
blind spots (it cannot see which of two equal values went where after a
move); the port's run-time check against retail is the proof.
"""
from __future__ import annotations

import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import overlay_check as oc  # noqa: E402
import rabbitizer as rz  # noqa: E402
from elftools.elf.elffile import ELFFile  # noqa: E402

GPR = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
       "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
ARGS = [4, 5, 6, 7, 8, 9, 10, 11]           # EABI integer arguments
FARGS = [12, 13, 14, 15, 16, 17, 18, 19]    # EABI float arguments
SAVED = set(range(16, 24)) | {30, 31}
FSAVED = set(range(20, 32))
CALLER = set(range(1, 16)) | {24, 25, 31}   # what a call clobbers
MOVES = {"move", "daddu", "addu", "or", "dadd"}


def words(data: bytes) -> list[int]:
    return [int.from_bytes(data[i:i + 4], "little") for i in range(0, len(data) - 3, 4)]


def regnum(text: str) -> int | None:
    m = re.fullmatch(r"\$(\w+)", text.strip())
    if not m:
        return None
    r = m.group(1)
    if r in GPR:
        return GPR.index(r)
    if r.isdigit():
        return int(r)
    return None


def fregnum(text: str) -> int | None:
    m = re.fullmatch(r"\$f(\d+)", text.strip())
    return int(m.group(1)) if m else None


_NAMES: dict[tuple[int, int], str] = {}


def callee(level: int, address: int) -> str:
    """A call target by its catalogue name: identical copies of a function share a name."""
    if not _NAMES:
        for line in open(oc.ROOT / "config/overlays/functions.tsv"):
            if line.startswith("#"):
                continue
            c = line.rstrip("\n").split("\t")
            if len(c) < 6:
                continue
            for place in c[5].split(","):
                lv, _, addr = place.partition(":")
                if addr:
                    _NAMES[(int(lv), int(addr, 16))] = c[0]
    if level < 0:
        return f"func_{address:08X}"
    return _NAMES.get((level, address), f"{address:x}")


def tokens(code: bytes, base: int, level: int = -1) -> list[str]:
    """The function's operations, normalised (see the module docstring)."""
    ins = [rz.Instruction(w, vram=base + 4 * i, category=rz.InstrCategory.R5900)
           for i, w in enumerate(words(code))]
    targets = set()
    for i in ins:
        if i.isBranch():
            targets.add(i.getBranchVramGeneric())
    # A call's delay slot runs before the call: read it first.
    k = 0
    while k < len(ins) - 1:
        if ins[k].getOpcodeName() in ("jal", "jalr") and ins[k + 1].vram not in targets:
            ins[k], ins[k + 1] = ins[k + 1], ins[k]
            k += 2
        else:
            k += 1
    out: list[list[str]] = []           # each token as [text, tags...]
    fresh: set[int] = set()             # registers given a known value since the last call
    known: dict[int, int] = {}          # gpr -> constant it holds
    writer: dict[str, int] = {}         # "r4" / "f12" -> index in out of the op that made the value
    slots: dict[int, tuple] = {}        # stack offset -> (writer index, known constant) stored there

    def reset(caller_only: bool) -> None:
        for r in list(known):
            if not caller_only or r in CALLER:
                del known[r]
        for k in list(writer):
            n = int(k[1:])
            if not caller_only or (k[0] == "r" and n in CALLER) or (k[0] == "f" and n < 20):
                del writer[k]

    def operand(text: str) -> str:
        t = text.strip()
        r = regnum(t)
        if r is not None:
            if r in (0, 28, 29):
                return GPR[r]
            if r in known:
                return f"#{known[r]:x}"
            return "r"
        if fregnum(t) is not None:
            return "f"
        return t

    def emit(text: str, dest: str | None) -> None:
        out.append([text])
        if dest:
            writer[dest] = len(out) - 1

    for i in ins:
        if i.vram in targets:
            # A label joins paths. What a callee-saved register holds is kept: the
            # compiler keeps long-lived values (a table's address) there and
            # rebuilds them seldom; scratch registers are forgotten.
            saved = {r: v for r, v in known.items() if r in SAVED}
            reset(False)
            known.update(saved)
        name = i.getOpcodeName()
        if name == "nop":
            continue
        ops = [o.strip() for o in i.disassemble().split(None, 1)[1].split(",")] if " " in i.disassemble().strip() else []
        name = re.sub(r"l$", "", name) if i.isBranch() and name.endswith("l") and name not in ("bal",) else name
        rd = regnum(ops[0]) if ops else None
        fd = fregnum(ops[0]) if ops else None

        # Constants and addresses: tracked, not emitted.
        if name == "lui":
            known[rd] = (int(ops[1], 0) & 0xFFFF) << 16
            fresh.add(rd)
            writer.pop(f"r{rd}", None)
            continue
        if name in ("addiu", "daddiu", "ori") and len(ops) == 3:
            src = regnum(ops[1])
            imm = int(ops[2], 0)
            if src == 0 or src == 28 or src in known:
                b = 0 if src == 0 else oc.GP if src == 28 else known[src]
                known[rd] = ((b | (imm & 0xFFFF)) if name == "ori" else (b + imm)) & 0xFFFFFFFF
                fresh.add(rd)
                writer.pop(f"r{rd}", None)
                continue
        if name in MOVES and len(ops) == 3 and regnum(ops[2]) == 0:
            src = regnum(ops[1])
            if src in known:
                known[rd] = known[src]
                fresh.add(rd)
            else:
                known.pop(rd, None)
            if f"r{src}" in writer:
                writer[f"r{rd}"] = writer[f"r{src}"]
            continue
        if name == "mov.s":
            src = fregnum(ops[1])
            if f"f{src}" in writer:
                writer[f"f{fd}"] = writer[f"f{src}"]
            continue

        # Memory: by absolute address when known, the stack as one place.
        m = re.fullmatch(r"(.*)\((\$\w+)\)", ops[-1]) if ops else None
        if m and (name.startswith(("l", "s")) and name not in ("lui", "sll", "srl", "sra", "slt", "sltu", "slti",
                                                                "sltiu", "sllv", "srlv", "srav", "sub", "subu")):
            breg = regnum(m.group(2))
            off = int(m.group(1), 0) if m.group(1) else 0
            val = ops[0]
            store = name.startswith("s")
            if breg == 29:
                vr, vf = regnum(val), fregnum(val)
                if (vr in SAVED or vf in FSAVED) and name in ("sd", "sw", "sq", "swc1", "ld", "lw", "lq", "lwc1"):
                    if not store and vr is not None:
                        known.pop(vr, None)
                        writer.pop(f"r{vr}", None)
                    continue        # prologue and epilogue
                # A stack slot is the allocator's (spills) or a local's; either way what
                # matters is the value, which goes in and comes out with its meaning.
                key = f"r{vr}" if vr is not None else f"f{vf}"
                if store:
                    slots[off] = (writer.get(key), known.get(vr) if vr is not None else None)
                else:
                    w, k = slots.get(off, (None, None))
                    writer.pop(key, None)
                    if vr is not None:
                        known.pop(vr, None)
                    if w is not None:
                        writer[key] = w
                    if k is not None and vr is not None:
                        known[vr] = k
                continue
            elif breg == 28:
                where = f"@{(oc.GP + off) & 0xFFFFFFFF:x}"
            elif breg in known:
                where = f"@{(known[breg] + off) & 0xFFFFFFFF:x}"
            else:
                where = f"r+{off:x}"
            if store:
                emit(f"{name} {operand(val)} {where}", None)
            else:
                emit(f"{name} {where}", f"r{regnum(val)}" if regnum(val) is not None else f"f{fregnum(val)}")
                if regnum(val) is not None:
                    known.pop(regnum(val), None)
            continue

        if name in ("jal",):
            emit(f"jal {callee(level, i.getInstrIndexAsVram())}", None)
            for r in ARGS:
                if f"r{r}" in writer:
                    out[writer[f"r{r}"]].append(f"arg{GPR[r]}")
                elif r in known and r in fresh:
                    out[-1].append(f"{GPR[r]}=#{known[r]:x}")
            for r in FARGS:
                if f"f{r}" in writer:
                    out[writer[f"f{r}"]].append(f"argf{r}")
            reset(True)
            fresh.clear()
            writer["r2"] = len(out) - 1
            writer["f0"] = len(out) - 1
            continue
        if name == "jalr":
            emit("jalr", None)
            reset(True)
            continue
        if name == "jr":
            if ops and regnum(ops[0]) == 31:
                for k in ("r2", "f0"):
                    if k in writer:
                        out[writer[k]].append("ret")
                emit("return", None)
            else:
                emit("jr", None)
            continue
        if name == "mtc1" and regnum(ops[0]) in known:
            emit(f"fconst #{known[regnum(ops[0])]:x}", f"f{fregnum(ops[1])}")
            continue

        if name in ("addiu", "daddiu") and rd == 29:
            continue            # the frame's size: the allocator's
        # Everything else: the operation and its non-register operands.
        dest = None
        if i.isBranch():
            body = " ".join(operand(o) for o in ops[:-1])
        else:
            body = " ".join(operand(o) for o in ops[1:]) if rd is not None or fd is not None else \
                " ".join(operand(o) for o in ops)
            if rd is not None and rd != 0:
                dest = f"r{rd}"
                known.pop(rd, None)
            elif fd is not None:
                dest = f"f{fd}"
        emit(f"{name} {body}".strip(), dest)
    return [" ".join([t[0]] + sorted(t[1:])) for t in out]


def executable_code(name: str, obj_path: Path) -> tuple[bytes, bytes, int]:
    """Ours relocated as tools/try_func.py relocates it, retail's from the
    executable: an executable function's two codes and its address."""
    import try_func
    asm = next(iter(sorted(Path("asm/nonmatchings").glob(f"*/{name}.s"))), None) \
        or next(iter(sorted(Path("asm/handwritten").glob(f"*/{name}.s"))), None)
    if asm is None:
        raise FileNotFoundError(f"no assembly for {name} under asm/nonmatchings or asm/handwritten")
    rsize = int(try_func.SIZE.search(asm.read_text()).group(2), 16)
    raw = Path(try_func.BASEROM).read_bytes()
    relf = ELFFile(open(try_func.BASEROM, "rb"))
    load = next(s for s in relf.iter_segments() if s["p_type"] == "PT_LOAD")
    delta = load["p_vaddr"] - load["p_offset"]
    elf = ELFFile(open(obj_path, "rb"))
    text = elf.get_section_by_name(".text").data()
    sym = next(s for s in elf.get_section_by_name(".symtab").iter_symbols() if s.name == name)
    off, osize = sym["st_value"], sym["st_size"]
    vram = int(name[5:], 16)
    resolved, _masked = try_func.resolve_relocations(elf, text, vram - off)
    filled = bytearray(text)
    for o, w in resolved.items():
        filled[o:o + 4] = w.to_bytes(4, "little")
    return bytes(filled[off:off + osize]), raw[vram - delta:vram - delta + rsize], vram


def compare(name: str, obj_path: Path) -> tuple[str, list[str], list[str]]:
    m = oc.OVERLAY_NAME.match(name)
    if not m:
        ours, retail, vram = executable_code(name, obj_path)
        return judge(collections.Counter(tokens(ours, vram)), collections.Counter(tokens(retail, vram)))
    level, address = int(m.group(1)), int(m.group(2), 16)
    catalogue = oc.load_catalogue()
    _kind, csize, _places = catalogue[name]
    csize = oc.joined_size(name, csize, catalogue)
    elf = ELFFile(open(obj_path, "rb"))
    placer = oc.Placer(elf, name, level, address)
    text = bytes(elf.get_section_by_name(".text").data())
    placer.scan_rodata(text)
    ours = bytearray(text)
    placer.retail = None
    placer.apply(ours, text)
    trec = oc.text_record(oc.level_manifest(level))
    raw = (oc.DUMP / f"level_{level:02d}/text.bin").read_bytes()
    retail = raw[address - trec["address"]: address - trec["address"] + csize]
    a = collections.Counter(tokens(bytes(ours[placer.off:placer.off + placer.size]), address, level))
    b = collections.Counter(tokens(retail, address, level))
    return judge(a, b)


def judge(a: collections.Counter, b: collections.Counter) -> tuple[str, list[str], list[str]]:
    missing = sorted((b - a).elements())
    extra = sorted((a - b).elements())
    if not missing and not extra:
        return "FUNCTIONAL", missing, extra
    def bare(c):
        return collections.Counter(re.sub(r" (arg\w+|ret|\w+=#\w+)", "", k) for k in c.elements())
    if bare(a) == bare(b):
        return f"NEAR ({len(missing)} differ only in which call an argument value goes to)", missing, extra
    if set(missing) <= set(a) and set(extra) <= set(b):
        return f"NEAR ({len(missing)} missing, {len(extra)} extra, all copies)", missing, extra
    return f"DIFFERENT ({len(missing)} missing, {len(extra)} extra)", missing, extra


def run(name: str, candidate: Path, work: Path) -> str:
    import try_func
    seg, src, first, last = try_func.find_stub(name)
    obj = try_func.build(name, seg, src, first, last, candidate.read_text(errors="replace"), work)
    if obj is None:
        return "COMPILE failed"
    if oc.OVERLAY_NAME.match(name) and oc.check(obj, name) == "EXACT":
        return "EXACT"
    try:
        verdict, missing, extra = compare(name, obj)
    except (ValueError, TypeError) as e:
        # VU0 macro instructions (lqc2, vmul, vcallms...) and some MMI ones are not modelled.
        return f"NOT COMPARABLE (instructions this tool does not model: {e}); check by behaviour or by hand"
    except Exception as e:      # an unresolved symbol, an unknown relocation
        return f"LINK {e}"
    if "--show" in sys.argv:
        for t in missing:
            print("   retail only:", t)
        for t in extra:
            print("   ours only:  ", t)
    return verdict


def staged(jobs: int) -> None:
    import glob
    from concurrent.futures import ThreadPoolExecutor
    import subprocess
    rows = []
    for path in sorted(glob.glob("nonmatching/*/func_*.c")):
        name = Path(path).stem
        rows.append((name, path))

    def one(row):
        name, path = row
        text = Path(path).read_text(errors="replace")
        body = text.split("*/\n", 1)[1] if text.startswith("/* NON_MATCHING") else text
        work = Path("build-sn/try") / name / "equiv"
        work.mkdir(parents=True, exist_ok=True)
        cand = work / "staged.c"
        cand.write_text(body)
        r = subprocess.run([sys.executable, __file__, name, str(cand)], capture_output=True, text=True)
        return name, (r.stdout.strip().splitlines() or [r.stderr.strip()[-200:]])[-1]

    with ThreadPoolExecutor(jobs) as ex:
        for name, verdict in ex.map(one, rows):
            print(f"{name}\t{verdict}", flush=True)


def main() -> None:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if "--staged" in sys.argv:
        jobs = int(sys.argv[sys.argv.index("--jobs") + 1]) if "--jobs" in sys.argv else 4
        staged(jobs)
        return
    if len(args) != 2:
        sys.exit(__doc__)
    name, cand = args
    work = Path("build-sn/try") / name / "equiv"
    print(f"{name}: {run(name, Path(cand), work)}")


if __name__ == "__main__":
    main()
