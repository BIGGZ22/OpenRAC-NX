# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""C from a function's assembly, for the functions the decompilation has no C for.

A function the decompilation still has as assembly (an INCLUDE_ASM stub) would
be a stub in the port that does nothing. Where the game's setup has written its
assembly (asm.py reads it), this writes C that does what the instructions do, in
hostgen's guest model, so the port runs it until the decompilation or a backfill
gives it real C, which always wins (only stubs are translated).

The C keeps the EE's registers as locals (port/runtime/include/openrac/ee.h):

- integer registers as 64-bit values, with a second 64-bit half (hN) where the
  function uses 128-bit loads, stores or MMI; HI and LO (and HI1, LO1); the FPU's
  registers as floats, its condition bit and its accumulator; VU0's registers are
  the program's one vector unit (openrac_vu0), since hand-written routines leave
  results there for the next;
- every instruction is one statement or a few; a branch target is a label; a
  branch's delay slot runs after the condition is taken and before the jump, and a
  likely branch's only when it is taken, as the EE runs them;
- $sp is the game stack (openrac_guest_sp at entry, given back at every return), so
  the function's frame and the addresses of its locals are game memory, and a call
  sees openrac_guest_sp at the caller's $sp; $gp is the game's;
- the function's parameters arrive in $a0-$t3 and $f12-$f19 by its prototype (the one
  hostgen inferred for the stub: the decompilation's declarations, or, for a function
  declared nowhere, the argument registers it reads first); jr $ra returns $v0 or $f0
  as the prototype says;
- jal goes through hostgen's own call path: to the callee's definition by its
  prototype (arguments from the registers by kind, as the EE passes them), through
  the address for a function the catalogue folded, through the level's hook for a
  hook stub; jalr and a jump through a register that is not a jump table go through
  GFN; the result lands in $v0 or $f0. A callee with no C that takes inputs in
  registers no prototype names (a hand-written helper) runs in place instead, back
  through $ra; a jump into code without C (a routine's continuation) is followed;
- an address formed from a symbol (%hi / %lo) or through $gp is relocated as hostgen
  relocates the C's globals (lower.Unit.global_address, code_address).

What it cannot write (system calls, COP0, VU0 microprograms, a few rare MMI
operations, a function reading a register no caller passes) keeps the stub, and the report lists
the function with the reason.

mode "console" writes the same C for the console's own compiler instead, calls and
globals by retail address: the form rac1-decomp's tools/fcheck.py checks against
retail (tests/asmc_console.h).
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field
from pathlib import Path

import asm
import ctype
from lower import FnReport, Unit, UnitReport, Unsupported, _Fn, hexaddr, LEVEL_NAME
from program import Function, Program, Signature, canon, cname, prototype

ARG_REGS = list(range(4, 12))     # $a0-$a3, $t0-$t3 (the EE's eight integer argument registers)
FARG_REGS = list(range(12, 20))   # $f12-$f19
INT_TYPES = set(ctype.INT_NAMES) | {"gaddr"}
WIDE = {"long long", "unsigned long long", "long", "unsigned long", "long long int", "long int",
        "unsigned long long int", "unsigned long int"}
SKIP = {"nop", "sync", "pref", "cache", "vnop", "vwaitq", "ssnop"}
UNSUPPORTED = {
    "syscall": "a system call", "mfc0": "COP0 (mfc0)", "mtc0": "COP0 (mtc0)", "eret": "COP0 (eret)",
    "ei": "COP0 (ei)", "di": "COP0 (di)", "cop0": "COP0", "vcallms": "a VU0 microprogram (vcallms)",
    "vcallmsr": "a VU0 microprogram (vcallmsr)", "vclipw": "VU0 clipping (vclipw)",
    "bc0f": "COP0 branch", "bc0t": "COP0 branch", "bc0fl": "COP0 branch", "bc0tl": "COP0 branch",
    "bc2f": "VU0 branch", "bc2t": "VU0 branch", "bc2fl": "VU0 branch", "bc2tl": "VU0 branch",
    "bltzal": "a conditional call", "bgezal": "a conditional call", "bltzall": "a conditional call",
    "bgezall": "a conditional call",
}
MMI_OPS = {
    "paddw", "psubw", "pcgtw", "pmaxw", "pminw", "pceqw", "paddh", "psubh", "pcgth", "pmaxh", "pminh",
    "pceqh", "paddb", "psubb", "pcgtb", "pceqb", "pextlw", "pextuw", "pextlh", "pextuh", "pextlb",
    "pextub", "ppacw", "ppach", "ppacb", "pcpyld", "pcpyud", "pcpyh", "pand", "por", "pxor", "pnor",
    "psllw", "psrlw", "psraw", "psllh", "psrlh", "psrah", "pexew", "pexcw", "prot3w", "pexeh", "pexch",
    "prevh", "pabsw", "pabsh", "padduw", "psubuw", "padduh", "psubuh", "paddub", "psubub", "paddsw",
    "psubsw", "paddsh", "psubsh", "paddsb", "psubsb", "plzcw", "pext5", "ppac5", "pinth", "pinteh",
}
CONDITIONAL = {"beq", "bne", "blez", "bgtz", "bltz", "bgez", "beql", "bnel", "blezl", "bgtzl", "bltzl",
               "bgezl", "bc1f", "bc1t", "bc1fl", "bc1tl"}


class AsmUnsupported(Exception):
    """An instruction or a shape of code this does not write; the function stays a stub."""


@dataclass
class Result:
    name: str
    text: str | None = None          # the C, when translated
    reason: str = ""                 # why not, when not
    source: str = ""                 # the assembly file, relative to the decompilation
    report: FnReport | None = None


def S32(x: str) -> str:
    return f"OPENRAC_EE_S32({x})"


def hx(v: int) -> str:
    return f"0x{v & 0xFFFFFFFF:08X}u"


def _comps(dest: int) -> list[int]:
    return [c for c in range(4) if dest & (8 >> c)]


def is_control(i: asm.Insn) -> bool:
    return i.op in asm.BRANCHES


class Translator:
    def __init__(self, program: Program, index: asm.Index | None, gp: int | None = None, mode: str = "host",
                 source: Path | None = None):
        self.program = program
        self.index = index
        self.gp = gp
        self.mode = mode
        self.source = source
        self._decoded: dict = {}
        self.host_text = ""   # the port's own C for the game (host/), which calls game functions by their prototypes

    # ---- What to translate ----

    def overlay_of(self, name: str) -> int:
        for pattern in self.program.code_names:
            m = pattern.match(name)
            if m:
                o = m.groupdict().get("overlay")
                return int(o) if o is not None else -1
        return -1

    def find(self, f: Function) -> tuple[int, int] | None:
        return self.index.find([f.symbol], self.program.code_places(f.symbol))

    # ---- One function ----

    def translate(self, f: Function, found: tuple[int, int] | None = None) -> Result:
        res = Result(f.symbol)
        found = found or self.find(f)
        if found is None:
            res.reason = "no assembly for it"
            return res
        line_block = self.index.code.get(found)
        if line_block is not None:
            res.source = self._relative(line_block[1].path)
        try:
            res.text, res.report = self._function(f, found)
        except (AsmUnsupported, Unsupported) as e:
            res.reason = str(e)
            res.text = None
        return res

    def _relative(self, path: Path) -> str:
        if self.source is not None:
            try:
                return path.resolve().relative_to(self.source.resolve()).as_posix()
            except ValueError:
                pass
        return path.name

    def _load(self, f: Function, found: tuple[int, int]) -> None:
        """Follows the function's flow from its entry through its program's code."""
        self.f = f
        self.home, self.entry_addr = found
        self._fregs = None
        self.inline_calls: set[int] = set()
        self.returns: set[int] = set()
        self.reach, self.delay_only, self.jtables = self._flow()
        self._check_inputs()

    _inline_cache: dict = {}

    def _inline_target(self, target: int) -> bool:
        """Whether a call to target runs the callee's code in place: a function with no C whose
        assembly is here and which reads, before writing, a register no caller passes arguments
        in (on its own it could not be translated)."""
        key = (id(self.index), self.home, target)
        cache = Translator._inline_cache
        if key in cache:
            return bool(cache[key])
        if self._is_other_entry(target) or self.at(target) is None:
            cache[key] = False
            return False
        cache[key] = False                   # while it is looked at: a call back to it is a call
        sub = Translator(self.program, self.index, self.gp, self.mode, self.source)
        try:
            sub._load(Function(f"sub_{target:08X}", Signature(ctype.Base("int"), (), False, False)),
                      (self.home, target))
            result = False
        except AsmUnsupported as e:
            result = "before writing it" in str(e)
        cache[key] = result
        return result

    # Registers a caller does not pass arguments in and does not keep: a function that reads one
    # before it writes it takes an input C cannot give it (a hand-written routine's own convention).
    SCRATCH = {1, 2, 3, 12, 13, 14, 15, 24, 25}
    FSCRATCH = set(range(0, 12))

    def _check_inputs(self) -> None:
        written_g, written_f = set(), set()
        stop = None
        for a in sorted(x for x in self.reach if x >= self.entry_addr):
            if stop is not None and a > stop:
                break
            ins = self.at(a)
            rg, wg, rf, wf = rw(ins)
            bad = (rg & self.SCRATCH) - written_g
            fbad = (rf & self.FSCRATCH) - written_f
            if bad or fbad:
                which = ", ".join([f"${r}" for r in sorted(bad)] + [f"$f{r}" for r in sorted(fbad)])
                raise AsmUnsupported(f"it reads {which} at {a:08X} before writing it: an input outside "
                                     "the argument registers")
            written_g |= wg
            written_f |= wf
            if is_control(ins) and stop is None:
                stop = a + 4

    def prepare_signature(self, f: Function, found: tuple[int, int]) -> None:
        """Gives f the signature its translation is written with (_signature)."""
        self._load(f, found)
        sig = self._signature(f)
        if re.search(r"\b" + re.escape(cname(f.symbol)) + r"\b", self.host_text):
            return  # the port's own C calls it by the signature it has
        if not f.sig.prototyped and self._compatible(sig, f.sig):
            # Declared `int f()`: callers keep calling it so; the definition with its integer
            # parameters is compatible with that declaration.
            return
        f.sig = sig

    def _compatible(self, sig: Signature, unprototyped: Signature) -> bool:
        """Whether a definition of sig may stand for a declaration `T f()` (unprototyped): the same
        result, no parameters that promotion changes (a float)."""
        c = lambda t: canon(t, self.program.typedefs)
        return (not unprototyped.prototyped and not unprototyped.params and c(sig.ret) == c(unprototyped.ret)
                and all(c(p) not in ("float", "char", "signed char", "unsigned char", "short",
                                     "unsigned short") for p in sig.params))

    def _function(self, f: Function, found: tuple[int, int]):
        self._load(f, found)
        self.report = FnReport(f.symbol, "translated")
        self.unit = Unit(self.program, "<asm>", {"inner": []}, UnitReport("<asm>"))
        self.unit.fn = _Fn(f.symbol, ctype.Func(f.sig.ret, tuple(f.sig.params), f.sig.variadic, f.sig.prototyped),
                           [], report=self.report)
        # The signature the program has for it (prepare_signature widened it where it could), or,
        # for one declared `int f()`, the integer parameters its registers say, which that
        # declaration allows.
        sig = self._signature(f)
        if self.mode == "host" and (f.sig.prototyped or not self._compatible(sig, f.sig)):
            sig = f.sig
        # (The console's form, a candidate of its own, takes every register its code reads.)
        self.sig = sig
        self.calls_k: dict[str, int] = {}
        body = self._body()
        line_block = self.index.code.get(found)
        self.where = self._relative(line_block[1].path) if line_block else ""
        return self._assemble(f, sig, body), self.report

    def at(self, addr: int) -> asm.Insn | None:
        """The instruction at addr in the function's program, decoded, or None."""
        key = (self.home, addr & 0xFFFFFFFF)
        if key not in self._decoded:
            lb = self.index.code.get(key)
            self._decoded[key] = None if lb is None else asm.decode(lb[0].word, lb[0].addr, lb[0].reloc)
        return self._decoded[key]

    def text_at(self, addr: int) -> str:
        lb = self.index.code.get((self.home, addr))
        return lb[0].text if lb else ""

    def _need(self, addr: int, why: str) -> asm.Insn:
        ins = self.at(addr)
        if ins is None:
            raise AsmUnsupported(f"{why} goes on into code with no assembly here, at {addr:08X} "
                                 "(a fragment of a function with C, or a gap)")
        if ins.word == 0xCDCDCDCD:
            raise AsmUnsupported(f"it reaches padding at {addr:08X}, not code")
        return ins

    def _is_other_entry(self, addr: int) -> bool:
        """Whether a jump to addr leaves for another function, a tail call: addr starts a
        function that has C (or the port's own). Other code the assembly has is followed into, as
        the EE runs it with every register as it is: a second entry of the same routine (alabel),
        or a hand-written routine's continuation in the next file, which takes registers no
        prototype says."""
        if addr == self.entry_addr:
            return False
        if self.at(addr) is None:
            return True                      # code with no assembly here: only a call can reach it
        sym = getattr(self.program, "by_place", {}).get((self.home, addr))
        if sym is None and self.home >= 0:
            sym = getattr(self.program, "by_place", {}).get((-1, addr))
        f = self.program.functions.get(sym) if sym else None
        has_c = f is not None and (f.unit not in (None, "<asm>") or sym in self.program.host)
        return has_c

    def _table_targets(self, jr_addr: int) -> list[int]:
        """Where a jump through a register at jr_addr may go: the jump tables written beside its
        function, or the labels spimdisasm marked as their targets (jlabel), in its file."""
        lb = self.index.code.get((self.home, jr_addr))
        if lb is None:
            return []
        block = lb[1]
        targets = set(block.jlabels)
        for t in block.tables.values():
            targets.update(t)
        return sorted(a for a in targets if self.at(a) is not None)

    def _flow(self):
        """The instructions reachable from the entry; those reached only as a delay slot; and
        each jump through a register that is a jump table, with its targets."""
        reach: set[int] = set()
        heads: set[int] = set()
        delay: set[int] = set()
        tables: dict[int, list[int]] = {}
        todo = [self.entry_addr]
        while todo:
            a = todo.pop()
            if a in heads:
                continue
            ins = self._need(a, "it")
            heads.add(a)
            reach.add(a)
            if ins.op in UNSUPPORTED:
                raise AsmUnsupported(f"{UNSUPPORTED[ins.op]} at {ins.addr:08X}")
            if not is_control(ins):
                todo.append(a + 4)
                continue
            d = self._need(a + 4, "a delay slot")
            if is_control(d):
                raise AsmUnsupported(f"a jump in the delay slot at {a + 4:08X}")
            reach.add(a + 4)
            delay.add(a + 4)
            op = ins.op
            if op == "jal" and self._inline_target(ins.target):
                # A hand-written routine that takes its inputs in registers no prototype names:
                # run in place, as the EE does, and back to the call through $ra (control).
                self.inline_calls.add(a)
                self.returns.add(a + 8)
                todo += [ins.target, a + 8]
            elif op in ("jal", "jalr"):
                todo.append(a + 8)
            elif op == "jr":
                if ins.rs != 31:
                    targets = self._table_targets(a)
                    if targets:
                        tables[a] = targets
                        todo.extend(targets)
            elif op == "j":
                if not self._is_other_entry(ins.target):
                    todo.append(ins.target)
            else:
                todo.append(ins.target)
                uncond = op in ("beq", "beql") and ins.rs == ins.rt
                if not uncond:
                    todo.append(a + 8)
        return reach, delay - heads, tables

    # ---- The signature ----

    def _signature(self, f: Function) -> Signature:
        sig = f.sig
        if sig.variadic:
            return sig
        if sig.prototyped or sig.params:
            # The decompilation's prototype, with the argument registers its code reads first that
            # the prototype leaves out (a declaration written for one caller): callers pass them,
            # as the EE's did (hostgen's call matching, passing a caller's own through).
            ints, floats, _, _ = self._entry_reads()
            have_i = sum(1 for p in sig.params if canon(p, self.program.typedefs) != "float")
            have_f = len(sig.params) - have_i
            extra = [ctype.Base("int")] * max(0, ints - have_i) + [ctype.Base("float")] * max(0, floats - have_f)
            if not extra:
                return sig
            return Signature(sig.ret, tuple(sig.params) + tuple(extra), False, True)
        # Declared nowhere with parameters: what it reads of the argument registers first.
        ints, floats, sets_v0, sets_f0 = self._entry_reads()
        params = [ctype.Base("int")] * ints + [ctype.Base("float")] * floats
        ret = sig.ret
        if not f.declared and not sets_v0 and sets_f0:
            ret = ctype.Base("float")
        return Signature(ret, tuple(params), False, True)

    def _entry_reads(self):
        """The argument registers read before they are written, along the code from the entry up
        to its first call or return, in address order (a parameter is used early)."""
        written_g, written_f = set(), set()
        read_g, read_f = set(), set()
        stop = None
        for a in sorted(x for x in self.reach if x >= self.entry_addr):
            if stop is not None and a > stop:
                break
            ins = self.at(a)
            rg, wg, rf, wf = rw(ins)
            if ins.op in ("sw", "sd", "sq", "swc1") and ins.rs == 29:
                # A register saved to the frame is not a use of it: a variadic routine saves every
                # argument register so, whatever its callers pass.
                rg, rf = rg - {ins.rt}, rf - {ins.ft}
            read_g |= {r for r in rg if r not in written_g}
            read_f |= {r for r in rf if r not in written_f}
            written_g |= wg
            written_f |= wf
            if ins.op in ("jal", "jalr", "jr") and stop is None:
                stop = a + 4   # its delay slot runs first
        sets_v0 = sets_f0 = False
        for a in self.reach:
            _, wg, _, wf = rw(self.at(a))
            sets_v0 |= 2 in wg
            sets_f0 |= 0 in wf
        ints = max([k + 1 for k, r in enumerate(ARG_REGS) if r in read_g], default=0)
        floats = max([k + 1 for k, r in enumerate(FARG_REGS) if r in read_f], default=0)
        return ints, floats, sets_v0, sets_f0

    # ---- Pieces of C ----

    @staticmethod
    def R(n: int) -> str:
        return "0" if n == 0 else f"r{n}"

    @staticmethod
    def H(n: int) -> str:
        return "0" if n == 0 else f"h{n}"

    def W(self, n: int, value: str) -> list[str]:
        return [] if n == 0 else [f"r{n} = {value};"]

    def vf(self, n: int, c: int) -> str:
        if n == 0:
            return "openrac_ee_f(0x3F800000u)" if c == 3 else "0.0f"
        return f"OPENRAC_VU0->vf[{n}][{c}]"

    def addr(self, ins: asm.Insn) -> str:
        """The address a load or store reaches."""
        if ins.rs == 28 and self.gp is not None:
            a = (self.gp + ins.imm) & 0xFFFFFFFF
            return f"(openrac_u32)({self._data(f'D_{a:08X}', a)})"
        if ins.rs == 0:
            return hx(ins.imm)
        return f"(openrac_u32)({self.R(ins.rs)} + {ins.imm})"

    def _data(self, sym: str, address: int) -> str:
        if self.mode != "host":
            return hx(address)
        return self.unit.global_address(address, self.unit._code_symbol(sym) is not None, self.program.data_level(sym))

    def _code(self, sym: str, address: int) -> str:
        if self.mode != "host":
            return hx(address)
        try:
            return self.unit.code_address(sym)
        except Unsupported:
            return hx(address)

    def _symbol_address(self, ins: asm.Insn) -> str | None:
        """For lui %hi(S): S's address as this function reaches it, minus S's own address (the
        relocation), or None when it is S's own address."""
        kind, sym, addend = ins.reloc
        address = None
        expr = None
        is_code = sym in self.program.functions or any(p.match(sym) for p in self.program.code_names)
        if sym.startswith("jtbl"):
            m = re.search(r"([0-9A-Fa-f]{8})", sym)
            if m:
                address = int(m.group(1), 16)
                expr = self._data(sym, address)
        elif is_code and not sym.startswith("D_"):
            places = self.program.code_places(self.program.canonical(sym))
            if places:
                own = [a for o, a in places if o == self.home] or [places[0][1]]
                address = own[0]
                expr = self._code(self.program.canonical(sym), address)
        else:
            address = self.program.data_address(sym)
            if address is not None:
                expr = self._data(sym, address)
        if address is None or expr is None or expr == hexaddr(address):
            return None
        return f"(openrac_u32)({expr}) - {hx(address)}"

    # ---- The body ----

    def _body(self) -> list[str]:
        """Every instruction group (an instruction, or a jump with its delay slot) in address order,
        each under its label, with a goto where the next in order is not where it falls through."""
        heads = sorted(self.reach - self.delay_only)
        out: list[str] = []
        if heads[0] != self.entry_addr:
            out.append(f"goto L_{self.entry_addr:08X};")
        for k, a in enumerate(heads):
            ins = self.at(a)
            out.append(f"L_{a:08X}:;")
            if not is_control(ins):
                out += self.stmt(ins)
                falls = a + 4
            else:
                delay = self.stmt(self.at(a + 4))
                out += self.control(ins, delay)
                falls = a + 8 if (ins.op in ("jal", "jalr") or (ins.op in CONDITIONAL and not (
                    ins.op in ("beq", "beql") and ins.rs == ins.rt))) else None
            nxt = heads[k + 1] if k + 1 < len(heads) else None
            if falls is not None and falls != nxt:
                out.append(f"goto L_{falls:08X};")
        # Only the labels something goes to.
        used = set(re.findall(r"goto (L_[0-9A-F]{8});", "\n".join(out)))
        return [s for s in out if not (s.startswith("L_") and s[:10] not in used)]

    def control(self, ins: asm.Insn, delay: list[str]) -> list[str]:
        op = ins.op
        R = self.R
        a = ins.addr
        if op in CONDITIONAL:
            cond = {
                "beq": f"{R(ins.rs)} == {R(ins.rt)}", "bne": f"{R(ins.rs)} != {R(ins.rt)}",
                "blez": f"(openrac_s64){R(ins.rs)} <= 0", "bgtz": f"(openrac_s64){R(ins.rs)} > 0",
                "bltz": f"(openrac_s64){R(ins.rs)} < 0", "bgez": f"(openrac_s64){R(ins.rs)} >= 0",
                "bc1t": "fcc_", "bc1f": "!fcc_",
            }[op[:-1] if ins.likely else op]
            go = f"goto L_{ins.target & 0xFFFFFFFF:08X};"
            if op in ("beq", "beql") and ins.rs == ins.rt:
                return delay + [go]
            if ins.likely:
                return [f"if ({cond}) {{", *["    " + s for s in delay], f"    {go}", "}"]
            return [f"c_ = {cond};", *delay, f"if (c_) {go}"]
        if op == "j":
            if not self._is_other_entry(ins.target):
                return delay + [f"goto L_{ins.target:08X};"]
            return delay + self.tail_call(ins.target, self._operand_name(a))
        if op == "jal":
            if a in self.inline_calls:
                return delay + [f"r31 = {S32(hx(a + 8))};", f"goto L_{ins.target:08X};"]
            return delay + [f"r31 = {S32(hx(a + 8))};"] + self.call(ins.target, self._operand_name(a))
        if op == "jalr":
            # The target register is read before the delay slot runs.
            return ([f"t_ = (openrac_u32){R(ins.rs)};"] + delay + self.W(ins.rd, S32(hx(a + 8)))
                    + self.indirect("t_", a))
        if op == "jr":
            if ins.rs == 31:
                if not self.returns:
                    return delay + self.leave()
                # Back from a routine run in place, to its call; else this function returns.
                lines = ["t_ = (openrac_u32)r31;", *delay, "switch (t_) {"]
                lines += [f"case {hx(r)}: goto L_{r:08X};" for r in sorted(self.returns)]
                return lines + ["default: break;", "}", *self.leave()]
            if a in self.jtables:
                return [f"t_ = (openrac_u32){R(ins.rs)};"] + delay + self.dispatch("t_", self.jtables[a])
            return [f"t_ = (openrac_u32){R(ins.rs)};"] + delay + self.indirect("t_", a) + self.leave()
        raise AsmUnsupported(f"{op} at {a:08X}")

    def _operand_name(self, addr: int) -> str | None:
        parts = self.text_at(addr).split()
        return parts[-1] if len(parts) > 1 and re.match(r"^[A-Za-z_][\w.$]*$", parts[-1]) else None

    def dispatch(self, value: str, targets: list[int]) -> list[str]:
        """A jump through a jump table: to the label at the same offset from the function's entry
        in the copy loaded (a level's copy's tables hold that copy's addresses)."""
        own = hx(self.entry_addr)
        if self.mode == "host" and self.program.code_places(self.f.symbol):
            own = self.unit._own_address()
        lines = [f"switch ((openrac_u32)({value} - (openrac_u32)({own}))) {{"]
        for a in targets:
            lines.append(f"case {hx(a - self.entry_addr)}: goto L_{a:08X};")
        lines.append(f"default: openrac_ee_lost(\"{self.f.symbol}\", {value}); {' '.join(self.leave())}")
        lines.append("}")
        return lines

    # ---- Calls ----

    def _callee(self, target: int, name: str | None) -> str | None:
        p = self.program
        if name and (name in p.functions or any(pt.match(name) for pt in p.code_names)):
            sym = p.canonical(name)
            if sym in p.functions:
                return sym
        for place in ((self.home, target), (-1, target)):
            sym = getattr(p, "by_place", {}).get(place)
            if sym is not None:
                return sym
        for sym, f in p.functions.items():
            if (self.home, target) in p.code_places(sym):
                return sym
        return None

    def call(self, target: int, name: str | None) -> list[str]:
        sym = self._callee(target, name)
        f = self.program.functions.get(sym) if sym else None
        if f is None or prototype(f, self.program.typedefs) is None:
            return self.indirect(self._code_literal(target), target)
        self.report.calls.add(sym)
        if f.sig.prototyped:
            args = self._args(sym, f.sig)
        else:
            # Declared without its parameters (`int f()`): the integer argument registers, which a
            # definition of that kind reads as far as it has parameters.
            args = [f"(int){self.R(r)}" for r in ARG_REGS]
        ret = canon(f.sig.ret, self.program.typedefs)
        fn_type = self._fn_type(f.sig)
        if self.mode != "host":
            # Retail code, which reads what the EE's registers hold whatever the decompilation
            # declares: every argument register, through a type that puts each where the EE has it.
            # (For the port the call is by the prototype, as C calls; the tests check that mapping.)
            if ret not in INT_TYPES and ret not in WIDE and ret not in ("void", "float"):
                raise AsmUnsupported(f"a call whose result is {ret}")
            # Four doublewords from the function's own $sp after them: the compiler puts parameters
            # past the registers at 0($sp), 8($sp)..., where a callee taking more than eight reads.
            params = ", ".join(["openrac_u64"] * 8 + ["float"] * 8 + ["openrac_u64"] * 4)
            regs = ([self.R(r) for r in ARG_REGS] + [f"f{r}" for r in FARG_REGS]
                    + [f"openrac_ee_ld((openrac_u32)r29 + {8 * k}u)" for k in range(4)])
            return self._invoke(f"(({ret} (*)({params})){hx(target)})({', '.join(regs)})", ret)
        elif sym in getattr(self.program, "level_hooks", set()) and self.unit._level_hook(sym) is not None:
            exe_fn, size, home, home_fn, stub = self.unit._level_hook(sym)
            k = self.calls_k.get(sym, 0)
            self.calls_k[sym] = k + 1
            callee = (f"GFN({fn_type}, openrac_hook_in_copy({hx(exe_fn)}, {size}u, {home}, {hx(home_fn)}, "
                      f"{hx(stub)}, {k}u))")
        elif self.unit._folded_places(sym):
            # One name for several places (folded by the catalogue): the place this call goes to.
            callee = f"GFN({fn_type}, {self._code_literal(target)})"
        else:
            callee = cname(sym)
        return self._invoke(f"{callee}({', '.join(args)})", ret)

    def _code_literal(self, target: int) -> str:
        """A code address of the program this assembly is from, as the function reaches it."""
        if self.mode != "host":
            return hx(target)
        if self.home < 0:
            return f"OPENRAC_CODE({hx(target)})" if getattr(self.program, "relocate", None) else hx(target)
        level = self.unit._shared_level()
        if level is not None:
            return f"OPENRAC_LCODE({level}, {hx(target)})"
        return hx(target)

    def _fn_type(self, sig: Signature) -> str:
        ret = canon(sig.ret, self.program.typedefs)
        params = [canon(p, self.program.typedefs) for p in sig.params]
        plist = ", ".join(params) + (", ..." if sig.variadic else "")
        return f"{ret} (*)({plist or ('void' if sig.prototyped else '')})"

    def _args(self, sym: str, sig: Signature) -> list[str]:
        """The call's arguments from the registers, by the callee's prototype: integers from $a0-$t3
        and then the stack (a doubleword each from 0($sp)), floats from $f12-$f19. A variadic
        callee gets the integer registers its fixed parameters leave, as doublewords."""
        out = []
        ni = nf = 0
        for p in sig.params:
            c = canon(p, self.program.typedefs)
            if c == "float":
                if nf >= len(FARG_REGS):
                    raise AsmUnsupported(f"a call to {sym} with more than eight float arguments")
                out.append(f"f{FARG_REGS[nf]}")
                nf += 1
            elif c in INT_TYPES or c in WIDE:
                if ni < len(ARG_REGS):
                    out.append(f"({c}){self.R(ARG_REGS[ni])}")
                else:
                    out.append(f"({c})openrac_ee_ld((openrac_u32)r29 + {8 * (ni - len(ARG_REGS))}u)")
                ni += 1
            else:
                raise AsmUnsupported(f"a call to {sym}, which takes {c or 'a record'}")
        if sig.variadic:
            out += [f"(openrac_u64){self.R(r)}" for r in ARG_REGS[ni:]]
        return out

    def _invoke(self, call: str, ret: str | None) -> list[str]:
        pre = ["OPENRAC_ASM_CALL();", "OPENRAC_VU0_OUT();"]
        post = ["OPENRAC_VU0_IN();"]
        if ret == "void":
            return pre + [f"{call};"] + post
        if ret == "float":
            return pre + [f"f0 = {call};"] + post
        if ret in WIDE:
            return pre + [f"r2 = (openrac_u64)({call});"] + post
        if ret in INT_TYPES:
            return pre + [f"r2 = {S32(f'({ret})({call})')};"] + post
        raise AsmUnsupported(f"a call whose result is {ret}")

    def indirect(self, value: str, at: int) -> list[str]:
        """A call through a code address: the registers as eight integers (and eight floats when the
        function sets any argument float), the result as an integer, or a float when the code after
        reads $f0 before it writes it."""
        floats = any(r in self._float_writes() for r in FARG_REGS)
        ret = "float" if self._reads_f0_after(at) else "int"
        params = ["openrac_u64"] * 8 + (["float"] * 8 if floats else [])
        args = [self.R(r) for r in ARG_REGS] + ([f"f{r}" for r in FARG_REGS] if floats else [])
        t = f"{ret} (*)({', '.join(params)})"
        callee = f"GFN({t}, {value})" if self.mode == "host" else f"(({t})({value}))"
        return self._invoke(f"{callee}({', '.join(args)})", ret)

    def _float_writes(self) -> set:
        """The FPU registers the function writes anywhere."""
        if self._fregs is None:
            self._fregs = set()
            for a in self.reach:
                self._fregs |= rw(self.at(a))[3]
        return self._fregs

    def _reads_f0_after(self, at: int) -> bool:
        """Whether the code after the call at `at` reads $f0 before it writes it (in a straight line)."""
        for a in range(at + 8, at + 8 + 4 * 64, 4):
            ins = self.at(a)
            if ins is None or a not in self.reach:
                return False
            _, _, rf, wf = rw(ins)
            if 0 in rf:
                return True
            if 0 in wf or is_control(ins):
                return False
        return False

    def tail_call(self, target: int, name: str | None) -> list[str]:
        return self.call(target, name) + self.leave()

    def leave(self) -> list[str]:
        return ["OPENRAC_ASM_LEAVE();", "OPENRAC_VU0_OUT();", self._return_text()]

    def _return_text(self) -> str:
        ret = canon(self.sig.ret, self.program.typedefs)
        if ret == "void":
            return "return;"
        if ret == "float":
            return "return f0;"
        if ret == "gaddr":
            return "return (gaddr)r2;"
        if ret in INT_TYPES or ret in WIDE:
            return f"return ({ret})r2;"
        raise AsmUnsupported(f"a result of type {ret}")

    # ---- One instruction ----

    def stmt(self, ins: asm.Insn) -> list[str]:
        op = ins.op
        R, W = self.R, self.W
        rs, rt, rd, sa, imm = ins.rs, ins.rt, ins.rd, ins.sa, ins.imm
        if op in SKIP:
            return []
        if op in UNSUPPORTED:
            raise AsmUnsupported(f"{UNSUPPORTED[op]} at {ins.addr:08X}")
        u32 = lambda x: f"(openrac_u32){x}"
        s32 = lambda x: f"(openrac_s32){x}"
        if op in ("addu", "add"):
            return W(rd, S32(f"{u32(R(rs))} + {u32(R(rt))}"))
        if op in ("subu", "sub"):
            return W(rd, S32(f"{u32(R(rs))} - {u32(R(rt))}"))
        if op in ("addiu", "addi"):
            if ins.reloc and ins.reloc[0] == "hi":
                raise AsmUnsupported(f"%hi in {op} at {ins.addr:08X}")
            return W(rt, S32(f"{u32(R(rs))} + {hx(imm)}"))
        if op in ("daddu", "dadd"):
            return W(rd, f"{R(rs)} + {R(rt)}")
        if op in ("dsubu", "dsub"):
            return W(rd, f"{R(rs)} - {R(rt)}")
        if op in ("daddiu", "daddi"):
            return W(rt, f"{R(rs)} + (openrac_u64)(openrac_s64){imm}")
        if op == "and":
            return W(rd, f"{R(rs)} & {R(rt)}")
        if op == "or":
            return W(rd, f"{R(rs)} | {R(rt)}")
        if op == "xor":
            return W(rd, f"{R(rs)} ^ {R(rt)}")
        if op == "nor":
            return W(rd, f"~({R(rs)} | {R(rt)})")
        if op == "andi":
            return W(rt, f"{R(rs)} & {hx(imm)}")
        if op == "ori":
            return W(rt, f"{R(rs)} | {hx(imm)}")
        if op == "xori":
            return W(rt, f"{R(rs)} ^ {hx(imm)}")
        if op == "lui":
            value = (ins.word & 0xFFFF) << 16
            if ins.reloc and ins.reloc[0] == "hi":
                moved = self._symbol_address(ins)
                if moved is not None:
                    return W(rt, S32(f"{hx(value)} + ({moved})"))
            return W(rt, S32(hx(value)))
        if op == "slt":
            return W(rd, f"(openrac_s64){R(rs)} < (openrac_s64){R(rt)}")
        if op == "sltu":
            return W(rd, f"{R(rs)} < {R(rt)}")
        if op == "slti":
            return W(rt, f"(openrac_s64){R(rs)} < {imm}")
        if op == "sltiu":
            return W(rt, f"{R(rs)} < (openrac_u64)(openrac_s64){imm}")
        if op == "sll":
            return W(rd, S32(f"{u32(R(rt))} << {sa}"))
        if op == "srl":
            return W(rd, S32(f"{u32(R(rt))} >> {sa}"))
        if op == "sra":
            return W(rd, S32(f"{s32(R(rt))} >> {sa}"))
        if op == "sllv":
            return W(rd, S32(f"{u32(R(rt))} << ({R(rs)} & 31)"))
        if op == "srlv":
            return W(rd, S32(f"{u32(R(rt))} >> ({R(rs)} & 31)"))
        if op == "srav":
            return W(rd, S32(f"{s32(R(rt))} >> ({R(rs)} & 31)"))
        if op in ("dsll", "dsll32"):
            return W(rd, f"{R(rt)} << {sa + (32 if op.endswith('32') else 0)}")
        if op in ("dsrl", "dsrl32"):
            return W(rd, f"{R(rt)} >> {sa + (32 if op.endswith('32') else 0)}")
        if op in ("dsra", "dsra32"):
            return W(rd, f"(openrac_u64)((openrac_s64){R(rt)} >> {sa + (32 if op.endswith('32') else 0)})")
        if op == "dsllv":
            return W(rd, f"{R(rt)} << ({R(rs)} & 63)")
        if op == "dsrlv":
            return W(rd, f"{R(rt)} >> ({R(rs)} & 63)")
        if op == "dsrav":
            return W(rd, f"(openrac_u64)((openrac_s64){R(rt)} >> ({R(rs)} & 63))")
        if op == "movz":
            return [] if rd == 0 else [f"if ({R(rt)} == 0) r{rd} = {R(rs)};"]
        if op == "movn":
            return [] if rd == 0 else [f"if ({R(rt)} != 0) r{rd} = {R(rs)};"]
        if op in ("mult", "multu", "mult1", "multu1"):
            one = "1" if op.endswith("1") else ""
            fn = "openrac_ee_multu" if op.startswith("multu") else "openrac_ee_mult"
            text = f"{fn}({R(rs)}, {R(rt)}, &lo{one}_, &hi{one}_)"
            return [f"r{rd} = {text};"] if rd else [f"{text};"]
        if op in ("madd", "maddu", "madd1", "maddu1"):
            one = "1" if op.endswith("1") else ""
            text = f"openrac_ee_madd({R(rs)}, {R(rt)}, &lo{one}_, &hi{one}_, {1 if 'u' in op else 0})"
            return [f"r{rd} = {text};"] if rd else [f"{text};"]
        if op in ("div", "divu", "div1", "divu1"):
            one = "1" if op.endswith("1") else ""
            fn = "openrac_ee_divu" if op.startswith("divu") else "openrac_ee_div"
            return [f"{fn}({R(rs)}, {R(rt)}, &lo{one}_, &hi{one}_);"]
        if op in ("mfhi", "mflo", "mfhi1", "mflo1"):
            return W(rd, f"{op[2:4]}{'1' if op.endswith('1') else ''}_")
        if op in ("mthi", "mtlo", "mthi1", "mtlo1"):
            return [f"{op[2:4]}{'1' if op.endswith('1') else ''}_ = {R(rs)};"]
        if op == "mfsa":
            return W(rd, "(openrac_u64)sa_")
        if op == "mtsa":
            return [f"sa_ = (openrac_u32){R(rs)} & 15u;"]
        if op == "mtsab":
            return [f"sa_ = ((openrac_u32){R(rs)} ^ {imm & 0xFFFF}u) & 15u;"]
        if op == "mtsah":
            return [f"sa_ = (((openrac_u32){R(rs)} ^ {imm & 0xFFFF}u) & 7u) * 2u;"]
        if op in ("break",):
            return [f"openrac_ee_break(\"{self.f.symbol}\", {hx(ins.addr)});"]
        if op in ("teq", "tne", "tge", "tgeu", "tlt", "tltu"):
            c = {"teq": "==", "tne": "!=", "tge": ">=", "tgeu": ">=", "tlt": "<", "tltu": "<"}[op]
            cast = "" if op.endswith("u") else "(openrac_s64)"
            return [f"if ({cast}{R(rs)} {c} {cast}{R(rt)}) openrac_ee_break(\"{self.f.symbol}\", {hx(ins.addr)});"]
        if op in ("teqi", "tnei", "tgei", "tgeiu", "tlti", "tltiu"):
            c = {"teqi": "==", "tnei": "!=", "tgei": ">=", "tgeiu": ">=", "tlti": "<", "tltiu": "<"}[op]
            if op.endswith("u"):
                cond = f"{R(rs)} {c} (openrac_u64)(openrac_s64){imm}"
            else:
                cond = f"(openrac_s64){R(rs)} {c} {imm}"
            return [f"if ({cond}) openrac_ee_break(\"{self.f.symbol}\", {hx(ins.addr)});"]
        # Loads and stores.
        loads = {"lb", "lbu", "lh", "lhu", "lw", "lwu", "ld"}
        if op in loads:
            return W(rt, f"openrac_ee_{op}({self.addr(ins)})")
        if op in ("lwl", "lwr", "ldl", "ldr"):
            return W(rt, f"openrac_ee_{op}({self.addr(ins)}, {R(rt)})")
        if op in ("sb", "sh", "sw", "sd", "swl", "swr", "sdl", "sdr"):
            return [f"openrac_ee_{op}({self.addr(ins)}, {R(rt)});"]
        if op == "lq":
            if rt == 0:
                return []
            return [f"openrac_ee_lq({self.addr(ins)}, &r{rt}, &h{rt});"]
        if op == "sq":
            return [f"openrac_ee_sq({self.addr(ins)}, {R(rt)}, {self.H(rt)});"]
        if op == "lwc1":
            return [f"f{ins.ft} = openrac_ee_lwc1({self.addr(ins)});"]
        if op == "swc1":
            return [f"openrac_ee_swc1({self.addr(ins)}, f{ins.ft});"]
        if op == "lqc2":
            if ins.ft == 0:
                return []
            a = self.addr(ins)
            return [f"t_ = {a} & ~15u;"] + [f"{self.vf(ins.ft, c)} = openrac_ee_lwc1(t_ + {4 * c}u);" for c in range(4)]
        if op == "sqc2":
            a = self.addr(ins)
            return [f"t_ = {a} & ~15u;"] + [f"openrac_ee_swc1(t_ + {4 * c}u, {self.vf(ins.ft, c)});" for c in range(4)]
        if op in MMI_OPS:
            if rd == 0:
                return []
            name = f"OPENRAC_MMI_{op.upper()}"
            return [f"q_ = openrac_ee_mmi({name}, openrac_ee_qof({R(rs)}, {self.H(rs)}), "
                    f"openrac_ee_qof({R(rt)}, {self.H(rt)}), {sa});", f"r{rd} = q_.lo;", f"h{rd} = q_.hi;"]
        if op == "qfsrv":
            if rd == 0:
                return []
            return [f"q_ = openrac_ee_qfsrv(openrac_ee_qof({R(rs)}, {self.H(rs)}), openrac_ee_qof({R(rt)}, {self.H(rt)}), sa_);",
                    f"r{rd} = q_.lo;", f"h{rd} = q_.hi;"]
        if op in ("pmfhi", "pmflo"):
            x = op[3:5]
            return [] if rd == 0 else [f"r{rd} = {x}_;", f"h{rd} = {x}1_;"]
        if op in ("pmthi", "pmtlo"):
            x = op[3:5]
            return [f"{x}_ = {R(rs)};", f"{x}1_ = {self.H(rs)};"]
        if op in ("pmfhl.lw", "pmfhl.uw"):
            if rd == 0:
                return []
            k = 0 if op == "pmfhl.lw" else 32
            return [f"r{rd} = ((lo_ >> {k}) & 0xFFFFFFFFull) | (((hi_ >> {k}) & 0xFFFFFFFFull) << 32);",
                    f"h{rd} = ((lo1_ >> {k}) & 0xFFFFFFFFull) | (((hi1_ >> {k}) & 0xFFFFFFFFull) << 32);"]
        if op == "pmthl":
            raise AsmUnsupported(f"pmthl at {ins.addr:08X}")
        # The FPU.
        if op.endswith(".s") or op in ("mtc1", "mfc1", "ctc1", "cfc1", "cvt.s.w"):
            return self.fpu(ins)
        if op in ("qmfc2", "qmtc2", "cfc2", "ctc2") or op.startswith("v"):
            return self.vu0(ins)
        raise AsmUnsupported(f"{op if op != '?' else 'an instruction it does not know'} at {ins.addr:08X}")

    def fpu(self, ins: asm.Insn) -> list[str]:
        op, fs, ft, fd = ins.op, f"f{ins.fs}", f"f{ins.ft}", f"f{ins.fd}"
        binary = {"add.s": "+", "sub.s": "-", "mul.s": "*"}
        if op in binary:
            return [f"{fd} = {fs} {binary[op]} {ft};"]
        if op == "div.s":
            return [f"{fd} = openrac_ee_fdiv({fs}, {ft});"]
        if op == "sqrt.s":
            return [f"{fd} = openrac_ee_sqrt({ft});"]
        if op == "rsqrt.s":
            return [f"{fd} = openrac_ee_rsqrt({fs}, {ft});"]
        if op == "abs.s":
            return [f"{fd} = openrac_ee_fabs({fs});"]
        if op == "neg.s":
            return [f"{fd} = openrac_ee_fneg({fs});"]
        if op == "mov.s":
            return [f"{fd} = {fs};"]
        if op == "max.s":
            return [f"{fd} = openrac_ee_max({fs}, {ft});"]
        if op == "min.s":
            return [f"{fd} = openrac_ee_min({fs}, {ft});"]
        if op == "adda.s":
            return [f"facc_ = {fs} + {ft};"]
        if op == "suba.s":
            return [f"facc_ = {fs} - {ft};"]
        if op == "mula.s":
            return [f"facc_ = {fs} * {ft};"]
        if op == "madda.s":
            return [f"facc_ = facc_ + {fs} * {ft};"]
        if op == "msuba.s":
            return [f"facc_ = facc_ - {fs} * {ft};"]
        if op == "madd.s":
            return [f"{fd} = facc_ + {fs} * {ft};"]
        if op == "msub.s":
            return [f"{fd} = facc_ - {fs} * {ft};"]
        if op == "c.f.s":
            return ["fcc_ = 0;"]
        if op in ("c.eq.s", "c.lt.s", "c.le.s"):
            c = {"c.eq.s": "==", "c.lt.s": "<", "c.le.s": "<="}[op]
            return [f"fcc_ = {fs} {c} {ft};"]
        if op == "cvt.s.w":
            return [f"{fd} = (openrac_f32)(openrac_s32)openrac_ee_bits({fs});"]
        if op == "cvt.w.s":
            return [f"{fd} = openrac_ee_f(openrac_ee_cvtws({fs}));"]
        if op == "mtc1":
            return [f"{fs} = openrac_ee_f({self.R(ins.rt)});"]
        if op == "mfc1":
            return self.W(ins.rt, S32(f"openrac_ee_bits({fs})"))
        if op == "cfc1":
            if ins.fs != 31:
                raise AsmUnsupported(f"cfc1 of FPU control register {ins.fs}")
            return self.W(ins.rt, "(openrac_u64)(fcc_ ? 0x00800000u : 0u)")
        if op == "ctc1":
            if ins.fs != 31:
                raise AsmUnsupported(f"ctc1 to FPU control register {ins.fs}")
            return [f"fcc_ = (int)(({self.R(ins.rt)} >> 23) & 1u);"]
        raise AsmUnsupported(f"{op} at {ins.addr:08X}")

    def vu0(self, ins: asm.Insn) -> list[str]:
        op = ins.op
        V = "OPENRAC_VU0->"
        vf = self.vf
        fs, ft, fd, bc = ins.fs, ins.ft, ins.fd, ins.bc
        comps = _comps(ins.dest)
        if op == "qmfc2":
            if ins.rt == 0:
                return []
            return [f"r{ins.rt} = (openrac_u64)openrac_ee_bits({vf(ins.rd, 0)}) | ((openrac_u64)openrac_ee_bits({vf(ins.rd, 1)}) << 32);",
                    f"h{ins.rt} = (openrac_u64)openrac_ee_bits({vf(ins.rd, 2)}) | ((openrac_u64)openrac_ee_bits({vf(ins.rd, 3)}) << 32);"]
        if op == "qmtc2":
            if ins.rd == 0:
                return []
            r, h = self.R(ins.rt), self.H(ins.rt)
            return [f"{vf(ins.rd, 0)} = openrac_ee_f({r});", f"{vf(ins.rd, 1)} = openrac_ee_f({r} >> 32);",
                    f"{vf(ins.rd, 2)} = openrac_ee_f({h});", f"{vf(ins.rd, 3)} = openrac_ee_f({h} >> 32);"]
        if op == "cfc2":
            n = ins.rd
            if n < 16:
                return self.W(ins.rt, "0" if n == 0 else f"(openrac_u64){V}vi[{n}]")
            if n in (20, 21, 22):
                reg = {20: "r", 21: "i", 22: "q"}[n]
                return self.W(ins.rt, S32(f"openrac_ee_bits({V}{reg})"))
            raise AsmUnsupported(f"cfc2 of VU0 control register {n}")
        if op == "ctc2":
            n = ins.rd
            if n == 0:
                return []
            if n < 16:
                return [f"{V}vi[{n}] = (openrac_u32){self.R(ins.rt)} & 0xFFFFu;"]
            if n in (20, 21, 22):
                reg = {20: "r", 21: "i", 22: "q"}[n]
                return [f"{V}{reg} = openrac_ee_f({self.R(ins.rt)});"]
            if n in (27, 28, 31):
                return []  # resets and microprogram starts: no microprograms run here
            raise AsmUnsupported(f"ctc2 to VU0 control register {n}")
        if op in ("vwaitq", "vnop"):
            return []

        def store(dst, values: dict[int, str]) -> list[str]:
            # Every operand read before any field is written: dst may be an operand.
            lines = [f"v_[{c}] = {values[c]};" for c in values]
            for c in values:
                if dst == "acc":
                    lines.append(f"{V}acc[{c}] = v_[{c}];")
                elif dst != 0:
                    lines.append(f"{vf(dst, c)} = v_[{c}];")
            return lines

        acc = lambda c: f"{V}acc[{c}]"
        binop = {"add": "+", "sub": "-", "mul": "*"}
        m = re.fullmatch(r"v(add|sub|mul|max|mini|madd|msub)(a?)(bc|q|i)?", op)
        if m:
            kind, to_acc, src = m.groups()
            dst = "acc" if to_acc else fd
            vals = {}
            for c in comps:
                b = vf(ft, bc) if src == "bc" else f"{V}q" if src == "q" else f"{V}i" if src == "i" else vf(ft, c)
                a = vf(fs, c)
                if kind in binop:
                    vals[c] = f"{a} {binop[kind]} {b}"
                elif kind == "max":
                    vals[c] = f"openrac_ee_max({a}, {b})"
                elif kind == "mini":
                    vals[c] = f"openrac_ee_min({a}, {b})"
                elif kind == "madd":
                    vals[c] = f"{acc(c)} + {a} * {b}"
                elif kind == "msub":
                    vals[c] = f"{acc(c)} - {a} * {b}"
            return store(dst, vals)
        if op in ("vopmula", "vopmsub"):
            rot_s, rot_t = [1, 2, 0], [2, 0, 1]
            vals = {}
            for c in comps:
                if c == 3:
                    continue
                prod = f"{vf(fs, rot_s[c])} * {vf(ft, rot_t[c])}"
                vals[c] = prod if op == "vopmula" else f"{acc(c)} - {prod}"
            return store("acc" if op == "vopmula" else fd, vals)
        if op in ("vmove", "vabs", "vmr32"):
            vals = {}
            for c in comps:
                if op == "vmove":
                    vals[c] = vf(fs, c)
                elif op == "vabs":
                    vals[c] = f"openrac_ee_fabs({vf(fs, c)})"
                else:
                    vals[c] = vf(fs, (c + 1) % 4)
            return store(ft, vals)
        m = re.fullmatch(r"v(ftoi|itof)(0|4|12|15)", op)
        if m:
            kind, frac = m.group(1), int(m.group(2))
            vals = {}
            for c in comps:
                if kind == "ftoi":
                    vals[c] = f"openrac_ee_f(openrac_ee_ftoi({vf(fs, c)}, {frac}))"
                else:
                    vals[c] = f"openrac_ee_itof(openrac_ee_bits({vf(fs, c)}), {frac})"
            return store(ft, vals)
        if op == "vdiv":
            return [f"{V}q = openrac_ee_fdiv({vf(fs, bc)}, {vf(ft, ins.ftf)});"]
        if op == "vsqrt":
            return [f"{V}q = openrac_ee_sqrt({vf(ft, ins.ftf)});"]
        if op == "vrsqrt":
            return [f"{V}q = openrac_ee_rsqrt({vf(fs, bc)}, {vf(ft, ins.ftf)});"]
        if op == "vmtir":
            return [] if ft & 15 == 0 else [f"{V}vi[{ft & 15}] = openrac_ee_bits({vf(fs, bc)}) & 0xFFFFu;"]
        if op == "vmfir":
            vi = "0" if fs & 15 == 0 else f"{V}vi[{fs & 15}]"
            return store(ft, {c: f"openrac_ee_f((openrac_u32)(openrac_s32)(openrac_s16)(openrac_u16){vi})" for c in comps})
        if op in ("viadd", "visub", "viand", "vior", "viaddi"):
            vi = lambda n: "0" if n & 15 == 0 else f"{V}vi[{n & 15}]"
            if op == "viaddi":
                dst, value = ft, f"{vi(fs)} + (openrac_u32)({ins.imm})"
            else:
                o = {"viadd": "+", "visub": "-", "viand": "&", "vior": "|"}[op]
                dst, value = fd, f"{vi(fs)} {o} {vi(ft)}"
            return [] if dst & 15 == 0 else [f"{V}vi[{dst & 15}] = ({value}) & 0xFFFFu;"]
        raise AsmUnsupported(f"VU0 {op} at {ins.addr:08X}")

    # ---- The whole function ----

    def _assemble(self, f: Function, sig: Signature, body: list[str]) -> str:
        p = self.program
        fake = Function(f.symbol, sig)
        proto = prototype(fake, p.typedefs)
        if proto is None:
            raise AsmUnsupported("its prototype has a type the port's prototypes cannot write")
        if sig.variadic:
            raise AsmUnsupported("it takes variable arguments")
        if f.symbol in p.wrap:
            proto = proto.replace(f" {cname(f.symbol)}(", f" {cname(f.symbol)}__game(", 1)
        text = "\n".join(body)
        gprs = sorted({int(m) for m in re.findall(r"\br(\d+)\b", text)} | {29, 31})
        highs = sorted({int(m) for m in re.findall(r"\bh(\d+)\b", text)})
        fprs = sorted({int(m) for m in re.findall(r"\bf(\d+)\b", text)})
        decl = []
        if self.mode == "host" and getattr(self.unit.fn, "uses_self", False):
            decl.append(f"    const gaddr openrac_self_ = openrac_guest_entry((openrac_host_fn){cname(f.symbol)});")
        frame = 64 + sum(-i.imm for i in map(self.at, self.reach) if i.op in ("addiu", "daddiu") and i.rs == 29
                         and i.rt == 29 and i.imm < 0)
        decl.append(f"    OPENRAC_ASM_FRAME({frame});")
        decl.append("    openrac_u64 " + ", ".join(f"r{n} = 0" for n in gprs) + ";")
        if highs:
            decl.append("    openrac_u64 " + ", ".join(f"h{n} = 0" for n in highs) + ";")
        if fprs:
            decl.append("    openrac_f32 " + ", ".join(f"f{n} = 0.0f" for n in fprs) + ";")
        extras = [("openrac_u64", "lo_"), ("openrac_u64", "hi_"), ("openrac_u64", "lo1_"), ("openrac_u64", "hi1_"),
                  ("int", "c_"), ("int", "fcc_"), ("openrac_f32", "facc_"), ("openrac_u32", "t_"),
                  ("openrac_u32", "sa_"), ("openrac_ee_q", "q_"), ("openrac_f32", "v_[4]")]
        for t, name in extras:
            bare = name.split("[")[0]
            if re.search(r"\b" + re.escape(bare) + r"\b", text):
                init = " = 0" if t not in ("openrac_ee_q",) and "[" not in name else ""
                decl.append(f"    {t} {name}{init};")
        setup = ["    r29 = OPENRAC_ASM_SP();"]
        if self.gp is not None and re.search(r"\br28\b", text):
            setup.append(f"    r28 = {S32(hx(self.gp))};")
        setup.append("    OPENRAC_VU0_IN();")
        ni = nf = 0
        stacked: list[str] = []
        for k, t in enumerate(sig.params):
            c = canon(t, p.typedefs)
            if c == "float":
                if nf < len(FARG_REGS) and f"f{FARG_REGS[nf]}" in [f"f{n}" for n in fprs]:
                    setup.append(f"    f{FARG_REGS[nf]} = a{k};")
                nf += 1
            elif c in INT_TYPES or c in WIDE:
                value = f"(openrac_u64)a{k}" if c in WIDE else S32(f"(openrac_s32)a{k}")
                if ni >= len(ARG_REGS):
                    # Past the eighth: where the EE's caller put it, a doubleword each above $sp.
                    stacked.append(f"    openrac_ee_sd((openrac_u32)r29 + {8 * (ni - len(ARG_REGS))}u, {value});")
                elif ARG_REGS[ni] in gprs:
                    setup.append(f"    r{ARG_REGS[ni]} = {value};")
                ni += 1
            else:
                raise AsmUnsupported(f"a parameter of type {c}")
        if stacked:
            setup[1:1] = [f"    r29 = {S32(f'(openrac_u32)r29 - {8 * len(stacked)}u')};"] + stacked
        lines = [f"/* {f.symbol}: translated from its assembly ({self.where}) */", proto + " {"]
        lines += decl + setup
        lines += ["    " + s if not s.startswith("L_") else s for s in body]
        lines.append("}")
        return "\n".join(lines) + "\n"


# ---- Which registers an instruction reads and writes (for the signature and the calls) ----

def rw(i: asm.Insn) -> tuple[set, set, set, set]:
    op = i.op
    g_r: set = set()
    g_w: set = set()
    f_r: set = set()
    f_w: set = set()
    three = {"addu", "add", "subu", "sub", "daddu", "dadd", "dsubu", "dsub", "and", "or", "xor", "nor", "slt",
             "sltu", "sllv", "srlv", "srav", "dsllv", "dsrlv", "dsrav", "movz", "movn"} | MMI_OPS | {"qfsrv"}
    if op in three:
        g_r |= {i.rs, i.rt}
        g_w.add(i.rd)
    elif op in ("addiu", "addi", "daddiu", "daddi", "andi", "ori", "xori", "slti", "sltiu"):
        g_r.add(i.rs)
        g_w.add(i.rt)
    elif op == "lui":
        g_w.add(i.rt)
    elif op in ("sll", "srl", "sra", "dsll", "dsrl", "dsra", "dsll32", "dsrl32", "dsra32"):
        g_r.add(i.rt)
        g_w.add(i.rd)
    elif op in ("mult", "multu", "mult1", "multu1", "madd", "maddu", "madd1", "maddu1"):
        g_r |= {i.rs, i.rt}
        g_w.add(i.rd)
    elif op in ("div", "divu", "div1", "divu1", "beq", "bne", "beql", "bnel"):
        g_r |= {i.rs, i.rt}
    elif op in ("blez", "bgtz", "bltz", "bgez", "blezl", "bgtzl", "bltzl", "bgezl", "jr", "mthi", "mtlo",
                "mthi1", "mtlo1", "mtsa", "mtsab", "mtsah", "pmthi", "pmtlo"):
        g_r.add(i.rs)
    elif op == "jalr":
        g_r.add(i.rs)
        g_w.add(i.rd)
    elif op in ("mfhi", "mflo", "mfhi1", "mflo1", "mfsa", "pmfhi", "pmflo", "pmfhl.lw", "pmfhl.uw"):
        g_w.add(i.rd)
    elif op in ("lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "lq"):
        g_r.add(i.rs)
        g_w.add(i.rt)
    elif op in ("lwl", "lwr", "ldl", "ldr"):
        g_r.add(i.rs)   # rt is merged into, its old bytes overwritten by the pair: not an input
        g_w.add(i.rt)
    elif op in ("sb", "sh", "sw", "sd", "sq", "swl", "swr", "sdl", "sdr"):
        g_r |= {i.rs, i.rt}
    elif op == "lwc1":
        g_r.add(i.rs)
        f_w.add(i.ft)
    elif op == "swc1":
        g_r.add(i.rs)
        f_r.add(i.ft)
    elif op in ("lqc2", "sqc2"):
        g_r.add(i.rs)
    elif op == "mtc1":
        g_r.add(i.rt)
        f_w.add(i.fs)
    elif op == "mfc1":
        f_r.add(i.fs)
        g_w.add(i.rt)
    elif op in ("ctc1", "ctc2", "qmtc2"):
        g_r.add(i.rt)
    elif op in ("cfc1", "cfc2", "qmfc2"):
        g_w.add(i.rt)
    elif op in ("add.s", "sub.s", "mul.s", "div.s", "max.s", "min.s", "rsqrt.s"):
        f_r |= {i.fs, i.ft}
        f_w.add(i.fd)
    elif op in ("madd.s", "msub.s"):
        f_r |= {i.fs, i.ft}
        f_w.add(i.fd)
    elif op in ("adda.s", "suba.s", "mula.s", "madda.s", "msuba.s", "c.eq.s", "c.lt.s", "c.le.s"):
        f_r |= {i.fs, i.ft}
    elif op == "sqrt.s":
        f_r.add(i.ft)
        f_w.add(i.fd)
    elif op in ("abs.s", "neg.s", "mov.s", "cvt.s.w", "cvt.w.s"):
        f_r.add(i.fs)
        f_w.add(i.fd)
    elif op in ("teq", "tne", "tge", "tgeu", "tlt", "tltu"):
        g_r |= {i.rs, i.rt}
    elif op in ("teqi", "tnei", "tgei", "tgeiu", "tlti", "tltiu"):
        g_r.add(i.rs)
    if op == "jal":
        g_w.add(31)
    g_r.discard(0)
    g_w.discard(0)
    return g_r, g_w, f_r, f_w


# ---- The program's part: which stubs, and the files ----

@dataclass
class Stage:
    """The translations of one program, and what could not be translated, for the report."""
    translated: list[Result] = field(default_factory=list)
    unsupported: dict[str, str] = field(default_factory=dict)
    without_asm: list[str] = field(default_factory=list)
    translator: "Translator | None" = None    # for the console's form (write_console)
    found: dict = field(default_factory=dict)  # name -> (program, address) of its entry


def asm_config(cfg: dict) -> dict | None:
    a = cfg.get("asm")
    if not a or not a.get("enabled", False):
        return None
    return a


def file_names(cfg: dict) -> list[str]:
    a = asm_config(cfg)
    return [f"asm_{k}.c" for k in range(int(a.get("files", 8)))] if a else []


def run(program: Program, cfg: dict, source: Path, host_text: str = "") -> tuple[Stage, dict[str, Result]]:
    """Reads the assembly (when it is there), and translates every stub it has assembly for.
    The functions translated are given C (Function.unit "<asm>") and the signature their
    translation is written with, before the units are translated, so calls to them are matched
    to it. Returns the stage's report and the translations by name."""
    stage = Stage()
    a = asm_config(cfg)
    if a is None:
        return stage, {}
    # The assembly is where the game's setup wrote it: the decompilation, or another copy of it.
    base = Path(a["source"]) if a.get("source") else source
    dirs = [base / d for d in a.get("dirs", [])]
    gp = int(str(a["gp"]), 0) if a.get("gp") is not None else None
    tr = Translator(program, None, gp, "host", base)
    tr.host_text = host_text
    index = asm.read_index(dirs, tr.overlay_of)
    tr.index = index
    if not index.blocks:
        return stage, {}
    # A stub whose place has C under another spelling (func_0020C828 for FUN_0020c828) is that C
    # already: calls to it go there (Program.canonical).
    stubs = [f for f in program.stubs() if f.symbol not in program.host and program.canonical(f.symbol) == f.symbol]
    found = {}
    for f in stubs:
        hit = tr.find(f)
        if hit is None:
            stage.without_asm.append(f.symbol)
        else:
            found[f.symbol] = hit
    # The signatures first (a function declared nowhere gets the one its registers say), then
    # the places, so calls between translated functions find each other.
    for sym, hit in found.items():
        f = program.functions[sym]
        try:
            tr.prepare_signature(f, hit)
        except (AsmUnsupported, Unsupported):
            pass
    for sym in found:
        program.functions[sym].unit = "<asm>"
    program.index_places()
    results: dict[str, Result] = {}
    for sym, hit in sorted(found.items()):
        r = tr.translate(program.functions[sym], hit)
        if r.text is None:
            stage.unsupported[sym] = r.reason
            program.functions[sym].unit = None
        else:
            results[sym] = r
            stage.translated.append(r)
    program.index_places()
    stage.translator = tr
    stage.found = found
    return stage, results


def write_files(out: Path, cfg: dict, results: dict[str, Result], game: str) -> list[str]:
    names = file_names(cfg)
    if not names:
        return []
    chunks: list[list[str]] = [[] for _ in names]
    for k, (sym, r) in enumerate(sorted(results.items())):
        chunks[k % len(names)].append(r.text)
    head = [f"/* Generated by port/tools/hostgen for {game} from the decompilation's assembly (asmc.py).",
            " * Do not edit: give the function C in the decompilation, and it replaces this. */",
            "#include <math.h>", "#include \"openrac/ee.h\"", "#include \"game_protos.h\"", "",
            "#define OPENRAC_ASM_FRAME(size) const gaddr sp_in_ = openrac_guest_sp",
            "#define OPENRAC_ASM_SP() OPENRAC_EE_S32(sp_in_)",
            "#define OPENRAC_ASM_CALL() (openrac_guest_sp = (gaddr)r29)",
            "#define OPENRAC_ASM_LEAVE() (openrac_guest_sp = sp_in_)", ""]
    for name, chunk in zip(names, chunks):
        text = "\n".join(head + chunk) + "\n"
        path = out / name
        if not path.exists() or path.read_text() != text:
            path.write_text(text)
    return names


# ---- The console's form, for the offline check against retail (rac1-decomp's tools/fcheck.py) ----

def _vu0_word(kind: str, n: int = 0, rt: int = 8, off: int = 0) -> str:
    """One VU0 instruction as its word (any assembler reads .word): what moves the vector unit's
    registers to and from a function's own state in the console's form. Encoded here from the
    instruction set, and decoded back (asm.decode) so the two agree."""
    words = {
        "sqc2": 0xF8000000 | 8 << 21 | n << 16 | (off & 0xFFFF),
        "lqc2": 0xD8000000 | 8 << 21 | n << 16 | (off & 0xFFFF),
        "cfc2": 0x48400000 | rt << 16 | n << 11,
        "ctc2": 0x48C00000 | rt << 16 | n << 11,
        "acc_to_vf1": 0x4BE00048,     # vmaddx.xyzw $vf1, $vf0, $vf0x: ACC + 0
        "q_to_vf1": 0x4B000060,       # vaddq.x $vf1, $vf0, Q
        "vf1_to_q": 0x4B800BBC,       # vdiv Q, $vf1x, $vf0w: x / 1
        "waitq": 0x4A0003BF,          # vwaitq
        "vf1_to_acc": 0x4BE009BF,     # vmulaw.xyzw ACC, $vf1, $vf0w: vf1 * 1
    }
    return f".word 0x{words[kind]:08X}"


def _vu0_sync() -> str:
    """OPENRAC_VU0_IN and OPENRAC_VU0_OUT for the console's form: the vector unit's registers
    into the function's own state and back, as macros (the candidate is one function)."""
    asm_ = '__asm__ __volatile__("move $8, %0\\n\\t{}" : : "r"({}) : "$8", "memory");'
    save = ["openrac_vu0_state *p_ = OPENRAC_VU0;", "openrac_u32 v_;"]
    for n in range(1, 32):
        save.append(asm_.format(_vu0_word("sqc2", n), f"&p_->vf[{n}][0]"))
    save.append(asm_.format(f"{_vu0_word('acc_to_vf1')}\\n\\t{_vu0_word('sqc2', 1)}", "&p_->acc[0]"))
    save.append(asm_.format(f"{_vu0_word('q_to_vf1')}\\n\\t{_vu0_word('sqc2', 1)}", "&p_->scratch[0]"))
    save.append("p_->q = p_->scratch[0];")
    save.append(asm_.format(_vu0_word("lqc2", 1), "&p_->vf[1][0]"))
    for n in list(range(1, 16)) + [21]:
        save.append(f'__asm__ __volatile__("{_vu0_word("cfc2", n, rt=8)}\\n\\tmove %0, $8" : "=r"(v_) : : "$8");')
        save.append(f"p_->vi[{n}] = v_ & 0xFFFFu;" if n < 16 else "p_->i = openrac_ee_f(v_);")
    save.append("p_->vi[0] = 0;")
    load = ["openrac_vu0_state *p_ = OPENRAC_VU0;", "openrac_u32 v_;"]
    for n in list(range(1, 16)) + [21]:
        load.append(f"v_ = {f'p_->vi[{n}]' if n < 16 else 'openrac_ee_bits(p_->i)'};")
        load.append(f'__asm__ __volatile__("move $8, %0\\n\\t{_vu0_word("ctc2", n, rt=8)}" : : "r"(v_) : "$8");')
    load.append("p_->scratch[0] = p_->q;")
    load.append(asm_.format(f"{_vu0_word('lqc2', 1)}\\n\\t{_vu0_word('vf1_to_q')}\\n\\t{_vu0_word('waitq')}",
                            "&p_->scratch[0]"))
    load.append(asm_.format(f"{_vu0_word('lqc2', 1)}\\n\\t{_vu0_word('vf1_to_acc')}", "&p_->acc[0]"))
    for n in range(1, 32):
        load.append(asm_.format(_vu0_word("lqc2", n), f"&p_->vf[{n}][0]"))

    def macro(name: str, body: list[str]) -> str:
        return f"#define {name}() do {{ \\\n" + " \\\n".join("    " + s for s in body) + " \\\n} while (0)"
    return macro("OPENRAC_VU0_IN", save) + "\n" + macro("OPENRAC_VU0_OUT", load) + "\n"


def console_text(name: str, c: str) -> str:
    """A translation's console form as one candidate file: the console's definitions, the
    instruction helpers, and the function, whose C name is its own (the decompilation's file
    may declare the function otherwise) and whose symbol is the function's."""
    include = HERE_INCLUDE / "asmc_console.h"
    ops = HERE_INCLUDE.parent.parent.parent / "runtime" / "include" / "openrac" / "ee_ops.h"
    uses_vu0 = "OPENRAC_VU0->" in c
    prelude = [include.read_text(), ops.read_text(),
               "#define OPENRAC_ASM_FRAME(size) openrac_u64 stk_[((size) + 31) / 8] __attribute__((aligned(16))); "
               "openrac_u64 vu0_raw_[sizeof(openrac_vu0_state) / 8 + 2]",
               "#define OPENRAC_ASM_SP() OPENRAC_EE_S32((openrac_u32)((char *)stk_ + sizeof stk_) & ~15u)",
               "#define OPENRAC_ASM_CALL() 0", "#define OPENRAC_ASM_LEAVE() 0",
               "#define OPENRAC_VU0 ((openrac_vu0_state *)(((openrac_u32)vu0_raw_ + 15u) & ~15u))"]
    if uses_vu0:
        prelude.append(_vu0_sync())
    else:
        prelude += ["#define OPENRAC_VU0_IN() 0", "#define OPENRAC_VU0_OUT() 0"]
    own = f"openrac_asmc_{name}"
    lines = c.splitlines()
    head = next(k for k, l in enumerate(lines) if l.rstrip().endswith("{") and "(" in l)
    proto = lines[head].rstrip()[:-1].rstrip()
    proto = re.sub(r"\b" + re.escape(cname(name)) + r"\(", own + "(", proto, count=1)
    lines[head] = proto + " {"
    return "\n".join(prelude + [f"{proto} __asm__(\"{name}\");", ""] + lines) + "\n"


HERE_INCLUDE = Path(__file__).resolve().parent / "include"


def write_console(stage: Stage, out: Path, names: list[str] | None = None) -> dict[str, str]:
    """Each translated function of the executable (or those named) in the console's form, one
    candidate file each under out (name.c). Returns name -> why, for those it could not write."""
    tr = stage.translator
    problems: dict[str, str] = {}
    if tr is None:
        return problems
    out.mkdir(parents=True, exist_ok=True)
    console = Translator(tr.program, tr.index, tr.gp, "console", tr.source)
    # Named functions are written whether or not they have C (the check of the translator on
    # more of the program than its stubs); otherwise every one translated for the port.
    todo = names or [r.name for r in stage.translated]
    for name in todo:
        f = tr.program.functions.get(name)
        found = stage.found.get(name) or (console.find(f) if f is not None else None)
        if f is None or found is None or found[0] != -1:
            problems[name] = "no assembly for it in the executable"
            continue
        again = console.translate(f, found)
        if again.text is None:
            problems[name] = again.reason
            continue
        (out / f"{name}.c").write_text(console_text(name, again.text))
    return problems
