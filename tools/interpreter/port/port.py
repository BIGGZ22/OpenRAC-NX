#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Builds a game's decompiled C for the host, as a library the runtime loads.

    port.py build GAME/VERSION [--only TEXT] [--out DIR]
    port.py wanted GAME/VERSION FILE... [--most N]

A decompilation's C is written for the console: 32-bit pointers, data at the
retail program's addresses. This compiles it for a 32-bit sandbox target
(wasm32), where both still hold, translates the result to C for the host
(wasm2c) and wraps every function so that its arguments and result pass
through the EE's registers. The runtime then runs a decompiled function in
place of the retail one, on the same memory image, wherever the game's
memory holds the code it was written from (runtime/docs/DESIGN.md, route C;
runtime/src/sys/native_abi.h).

Each source file becomes a module of its own. A call that leaves the file
goes through the registers, as on the console, so it reaches retail code or
another module alike, and a function declared with different prototypes in
different files does no harm.

The library is written to build/port/<game>-<version>/ and holds code built
from the repository's C only; the sizes and checksums of the retail
functions it stands in for are read from your own disc's files.

--only TEXT builds only the source files whose path contains TEXT.
`wanted` reads what `openrac-boot --native-calls FILE` wrote and lists the
guest functions the run called that no host function stood in for, the
busiest first: what to decompile next for the port.
It needs LLVM (clang with the wasm32 target, wasm-ld) and WABT (wasm2c).
"""
import concurrent.futures
import levels
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent

# Where a package manager puts the compiler when PATH does not reach it.
LLVM_DIRS = ["/opt/homebrew/opt/llvm/bin", "/usr/local/opt/llvm/bin", "/usr/lib/llvm/bin"]

# The source folders of a game that hold its own code; libraries the runtime
# replaces, and code that is never ported (movies), are left out.
SOURCE_DIRS = {
    "rac1/pal": ["src/core", "src/game", "src/overlays"],
}

# Where a game lists the places of its functions: a game with a program for each level has the
# same function at another address in each (the decompilation's catalogue of them).
CATALOGUES = {
    "rac1/pal": "config/overlays/functions.tsv",
}

# Where a game lists the functions it keeps as assembly because they were written as assembly.
HANDWRITTEN = {
    "rac1/pal": "config/handwritten_asm.txt",
}

# How many bytes at the start of a level's code tell the level's program from any other.
LEVEL_PROBE_BYTES = 256

CFLAGS = [
    "--target=wasm32-unknown-unknown",
    "-O2",
    # The sources are written for GCC 2.95: implicit declarations and loose pointer types.
    "-std=gnu89",
    "-ffreestanding",
    "-fno-builtin",
    # The retail compiler did neither of these optimisations, and the code relies on that.
    "-fno-strict-aliasing",
    "-fwrapv",
    "-w",
    # Position independent: every retail global and every function address is then a
    # value the host supplies, and the module's own data goes where the host puts it.
    "-fPIC",
    "-fvisibility=default",
    "-Wno-error=implicit-function-declaration",
    "-Wno-error=incompatible-pointer-types",
    "-Wno-error=int-conversion",
    "-Wno-error=incompatible-function-pointer-types",
    "-Wno-error=implicit-int",
    "-Wno-error=return-type",
    # `long` is 8 bytes on the console and 4 on this target.
    "-Dlong=long long",
]


def find_tool(name):
    """Returns the path of an LLVM tool, looking beyond PATH, or stops with a message."""
    path = os.pathsep.join(LLVM_DIRS + [os.environ.get("PATH", ""), "/opt/homebrew/bin", "/usr/local/bin"])
    found = shutil.which(name, path=path)
    if not found:
        raise SystemExit(f"port.py: {name} is not installed (it comes with LLVM)")
    return found


# Functions whose integer result is 64 bits wide whatever a source file declares (runtime/port/wide).
WIDE_RESULTS = set()


def filter_source(text):
    """Returns a source file's text as the host build compiles it.

    `long long` becomes `long`, which the build defines as `long long`, and
    an integer constant with the suffix `L` gets `LL`. A
    statement of assembly at file scope (padding between functions) is
    dropped; its lines are kept empty so that line numbers stay. A
    declaration of a function in `WIDE_RESULTS` is read as returning
    `long`, whichever integer type it names.
    """
    text = re.sub(r"\blong\s+long\b", "long", text)
    # An integer constant marked `L` is 64 bits on the console and 32 here: `0xFE00L << 46` must stay 64-bit.
    text = re.sub(r"\b(0[xX][0-9a-fA-F]+|\d+)([uU]?)[lL]\b", r"\1\2LL", text)
    for name in WIDE_RESULTS:
        text = re.sub(rf"\b(?:unsigned\s+(?:int|long)|int|unsigned|long|[su]32|[su]64)\s+{name}\s*\(", f"long {name}(", text)
    lines = text.split("\n")
    out = []
    inside = False
    for line in lines:
        if not inside and line.startswith("__asm__("):
            inside = True
        if inside:
            out.append("")
            if line.rstrip().endswith(");"):
                inside = False
        else:
            out.append(line)
    return "\n".join(out)


LABEL = re.compile(r'__asm__\s*\(\s*"([^"]+)"\s*\)')


def declared_name(text, end):
    """Returns the identifier a declarator ends with, scanning back from `end`: `name`, `name(...)` or `name[...]`."""
    at = end
    while at > 0 and text[at - 1].isspace():
        at -= 1
    # Step back over a parameter list and any array bounds.
    while at > 0 and text[at - 1] in ")]":
        close = text[at - 1]
        opening = "(" if close == ")" else "["
        depth = 0
        while at > 0:
            at -= 1
            if text[at] == close:
                depth += 1
            elif text[at] == opening:
                depth -= 1
                if depth == 0:
                    break
        while at > 0 and text[at - 1].isspace():
            at -= 1
    start = at
    while start > 0 and (text[start - 1].isalnum() or text[start - 1] == "_"):
        start -= 1
    return text[start:at]


def own_labels(text):
    """Gives each declaration that names its symbol a symbol of its own, "symbol$name".

    A file may declare one function several times under different C names with different
    prototypes (`extern int f_a(float) __asm__("func_X")`). On the console they are one symbol;
    here each needs its own, because a call's types are part of what is linked. The name after
    the "$" is the C name, so a declaration repeated is the same symbol again.
    """
    def replace(match):
        name = declared_name(text, match.start())
        return f'__asm__("{match.group(1)}${name}")' if name else match.group(0)

    return LABEL.sub(replace, text)


NAME = re.compile(r"^(?:func|D)_(?:L(\d\d)_)?([0-9A-Fa-f]{8})")

# Functions the compiler itself calls, by their bodies over guest memory (a0, a1, a2 are the arguments).
BUILTINS = {
    "memcpy": "memcpy(openrac_host->space + a0, openrac_host->space + a1, a2); return a0;",
    "memmove": "memmove(openrac_host->space + a0, openrac_host->space + a1, a2); return a0;",
    "memset": "memset(openrac_host->space + a0, (int)a1, a2); return a0;",
    # The console's square root and reciprocal square root, for the port's own C (runtime/port/hand).
    "openrac_sqrt": "return openrac_float(4, a0, 0.0f);",
    "openrac_rsqrt": "return openrac_float(7, a0, a1);",
}

# The translated code's load and store, with the console's device addresses set apart: the
# registers of the EE's devices and of the GS are at 0x10000000 to 0x13FFFFFF in each of the
# address segments (documented), except where the scratchpad is (0x7...). The rest is wasm2c's.
MEMORY_MACROS = r"""
#define OPENRAC_IS_HARDWARE(addr) \
  (UNLIKELY((((u32)(addr)) >> 28) != 7 && ((((u32)(addr)) & 0x1FFFFFFFu) - 0x10000000u) < 0x04000000u))

#define DEFINE_LOAD(name, t1, t2, t3, force_read)                             \
  static inline t3 name##_unchecked(uint8_t* const wasm_rt_local_memory_base, \
                                    wasm_rt_memory_t* mem, u64 addr) {        \
    t1 result;                                                                \
    if (OPENRAC_IS_HARDWARE(addr)) {                                          \
      u64 raw = openrac_host->hardware_read(openrac_host->context, (u32)addr, \
                                            sizeof(t1));                      \
      wasm_rt_memcpy(&result, &raw, sizeof(t1));                              \
      return (t3)(t2)result;                                                  \
    }                                                                         \
    wasm_rt_memcpy(&result, MEM_ADDR_MEMOP(mem, addr, sizeof(t1)),            \
                   sizeof(t1));                                               \
    t3 ret = (t3)(t2)result;                                                  \
    force_read(ret);                                                          \
    return ret;                                                               \
  }                                                                           \
  DEF_MEM_CHECKS0(name, _, t1, return, t3)

#define DEFINE_STORE(name, t1, t2)                                     \
  static inline void name##_unchecked(                                 \
      uint8_t* const wasm_rt_local_memory_base, wasm_rt_memory_t* mem, \
      u64 addr, t2 value) {                                            \
    t1 wrapped = (t1)value;                                            \
    if (OPENRAC_IS_HARDWARE(addr)) {                                   \
      u64 raw = 0;                                                     \
      wasm_rt_memcpy(&raw, &wrapped, sizeof(t1));                      \
      openrac_host->hardware_write(openrac_host->context, (u32)addr,   \
                                   sizeof(t1), raw);                   \
      return;                                                          \
    }                                                                  \
    wasm_rt_memcpy(MEM_ADDR_MEMOP(mem, addr, sizeof(t1)), &wrapped,    \
                   sizeof(t1));                                        \
  }                                                                    \
  DEF_MEM_CHECKS1(name, _, t1, , void, t2)

"""

C_TYPES = {"u32": "uint32_t", "u64": "uint64_t", "f32": "float", "f64": "double", "void": "void"}


class Module:
    """One source file on its way to host code."""

    def __init__(self, source, game_dir, out_dir, index):
        self.source = source
        self.name = f"m{index}"
        # A file of the port's own (runtime/port/hand) is named from this folder, a game's from the game's.
        home = game_dir if game_dir in source.parents else HERE
        self.stem = out_dir / str(source.relative_to(home)).replace("/", "_")[:-2]
        self.error = None
        self.imports = []        # (C name, plain name, result, [parameter types])
        self.exports = []        # the same
        self.globals = []        # (C name, plain name) of imported address globals
        self.data_bytes = 0
        self.data_align = 0


def run(command):
    """Runs a tool. Returns its output when it failed, else None."""
    result = subprocess.run(command, capture_output=True, text=True)
    return None if result.returncode == 0 else (result.stderr or result.stdout)


def demangle(name):
    """Returns a wasm2c C name's symbol name: its escapes undone, the "$N" of a declaration dropped."""
    name = re.sub(r"0x([0-9A-F]{2})", lambda m: chr(int(m.group(1), 16)), name)
    return name.split("$")[0]


def address_of(name):
    """Returns (address, level or None) of a symbol named after its address, or None."""
    match = NAME.match(name)
    if not match:
        return None
    return int(match.group(2), 16), match.group(1)


INDIRECT = re.compile(r"CALL_INDIRECT\(\(\*instance->\w+\), ((void|u32|u64|f32|f64) \(\*\)\(void\*((?:, \w+)*)\)), ")


def indirect_calls(code):
    """Makes a translated module's calls through function pointers go through the registers.

    A function pointer in the game's memory is a guest address. wasm2c would look it up in a
    table of the module's own functions; instead, each call sets the registers as the console's
    convention has them and asks the runtime to call that address, whatever is there.
    """
    kinds = sorted(set(INDIRECT.findall(code)))
    lines = ["", "static uint32_t openrac_indirect_target;"]
    choices = []
    for n, (whole, result, rest) in enumerate(kinds):
        parameters = rest.replace(",", " ").split()
        plan, slots = marshal(parameters)
        arguments = "".join(f", {kind} a{i}" for i, kind in enumerate(parameters))
        lines.append(f"static {result} openrac_indirect_{n}(void* unused{arguments}) {{")
        lines.append("    uint32_t target = openrac_indirect_target;")
        lines.append("    (void)unused;")
        lines += pass_arguments(plan, slots)
        lines.append("    openrac_call(target);")
        lines += take_result(result, slots)
        lines.append("}")
        choices.append(f"{whole}: openrac_indirect_{n}")
    macro = ["#undef CALL_INDIRECT"]
    if choices:
        macro.append("#define CALL_INDIRECT(table, t, ft, x, inst, ...) \\")
        macro.append("  (openrac_indirect_target = (x), _Generic((t)0, " + ", ".join(choices) + ")((void*)0, ##__VA_ARGS__))")
    else:
        macro.append("#define CALL_INDIRECT(table, t, ft, x, inst, ...) (wasm_rt_trap(WASM_RT_TRAP_CALL_INDIRECT), 0)")
    block = "\n".join(lines + macro) + "\n"
    # After the last of wasm2c's own definitions for indirect calls.
    anchor = "   DO_CALL_INDIRECT(table, t, x, __VA_ARGS__))\n"
    at = code.index(anchor) + len(anchor)
    return code[:at] + block + code[at:]


def pass_arguments(plan, slots):
    """Returns the C lines that put a call's arguments (a0, a1, ...) where the console's convention wants them."""
    lines = []
    if slots:
        # A frame of its own below the caller's, as a compiled caller would have.
        lines.append("    uint32_t sp = (uint32_t)openrac_get_r(29);")
        lines.append(f"    uint32_t frame = (sp - {8 * slots}u) & ~15u;")
        lines.append("    openrac_set_r32(29, frame);")
    for n, (place, number, kind) in enumerate(plan):
        value = f"a{n}" if kind in ("u32", "u64") else f"openrac_bits_{kind}(a{n})"
        if place == "f":
            lines.append(f"    openrac_set_f({number}, a{n});")
        elif place == "s":
            wide = value if kind != "u32" else f"(uint64_t)(int64_t)(int32_t){value}"
            lines.append(f"    openrac_store64(frame + {8 * number}u, {wide});")
        elif kind == "u32":
            lines.append(f"    openrac_set_r32({number}, {value});")
        else:
            lines.append(f"    openrac_set_r64({number}, {value});")
    return lines


def take_result(result, slots):
    """Returns the C lines that end a call: the stack pointer put back, the result read from its register."""
    lines = ["    openrac_set_r32(29, sp);"] if slots else []
    if result == "u32":
        lines.append("    return (uint32_t)openrac_get_r(2);")
    elif result == "u64":
        lines.append("    return openrac_get_r(2);")
    elif result == "f32":
        lines.append("    return openrac_get_f(0);")
    elif result == "f64":
        lines.append("    return (double)openrac_get_f(0);")
    return lines


FLOAT_OPS = {"+": 0, "-": 1, "*": 2, "/": 3}


def console_floats(code):
    """Makes a translated module compute its floats as the console does.

    wasm2c writes each float operation as one statement on its stack variables (`var_f1 *=
    var_f2;`, `var_i0 = var_f0 < var_f1;`). Those statements are rewritten to ask the runtime's
    model of the console's arithmetic, which has no infinity, cuts towards zero and orders the
    patterns that are not numbers elsewhere. Without this a division by zero leaves an infinity
    where the retail code leaves the largest number, and everything computed from it differs.
    """
    code = re.sub(r"^(\s*)(var_f\d+) ([-+*/])= (var_f\d+);$",
                  lambda m: f"{m.group(1)}{m.group(2)} = openrac_float({FLOAT_OPS[m.group(3)]}, {m.group(2)}, {m.group(4)});",
                  code, flags=re.M)
    code = re.sub(r"^(\s*)(var_i\d+) = (var_f\d+) (<=|>=|==|!=|<|>) (var_f\d+);$",
                  lambda m: f"{m.group(1)}{m.group(2)} = openrac_float_order({m.group(3)}, {m.group(5)}) {m.group(4)} 0;",
                  code, flags=re.M)
    code = re.sub(r"\(f32\)\(s32\)\((var_i\d+)\)", r"openrac_float_from_int(\1)", code)
    # The conversion to an integer is a macro of wasm2c's: put the console's after its definition.
    marker = "DEFINE_LOAD(i32_load,"
    at = code.index(marker)
    override = ("#undef I32_TRUNC_SAT_S_F32\n#define I32_TRUNC_SAT_S_F32(x) openrac_float_to_int(x)\n"
                "#undef I32_TRUNC_S_F32\n#define I32_TRUNC_S_F32(x) openrac_float_to_int(x)\n\n")
    return code[:at] + override + code[at:]


# Reading unoptimised LLVM IR: a function's start, a local, a pointer made from another, any value.
DEFINE = re.compile(r"^define .*@\"?([\w$.]+)\"?\(")
VARIADIC = re.compile(r"^define .*@\"?([\w$.]+)\"?\([^)]*\.\.\.\)")
ALLOCA = re.compile(r"^\s+(%[\w.]+) = alloca (.+?), align")
DERIVED = re.compile(r"^\s+(%[\w.]+) = (?:getelementptr|bitcast)\b.*?\bptr (%[\w.]+)")
TOKEN = re.compile(r"%[\w.]+")

def written_only(ll):
    """Returns {function: [locals]} from unoptimised LLVM IR: arrays and structures that a function
    neither reads nor hands to anyone, whether it stores to them or never touches them.

    Such a local is there for a reason: the function passes a neighbouring local's address, and
    the callee reads or writes past it into this one (the original source had one structure or
    a larger array where the decompilation has two locals). That works only with the retail
    compiler's stack layout, so the host build leaves these functions to the interpreter.
    """
    found = {}
    function = None
    for line in ll.split("\n"):
        start = DEFINE.match(line)
        if start:
            function, roots, root_of, writes, reads = start.group(1), {}, {}, set(), set()
            continue
        if function is None:
            continue
        if line.startswith("}"):
            # Written and never read, or never touched at all: either way it is there for the layout.
            names = sorted(name for name in roots if roots[name] and name not in reads)
            if names:
                found[function] = names
            function = None
            continue
        made = ALLOCA.match(line)
        if made:
            kind = made.group(2)
            roots[made.group(1)] = kind.startswith("[") or kind.startswith("%struct.") or kind.startswith("%union.")
            root_of[made.group(1)] = made.group(1)
            continue
        step = DERIVED.match(line)
        if step and step.group(2) in root_of:
            root_of[step.group(1)] = root_of[step.group(2)]
            continue
        tokens = [token for token in TOKEN.findall(line.split("!dbg")[0]) if token in root_of]
        if not tokens:
            continue
        text = line.strip()
        if "@llvm.lifetime" in text or "@llvm.dbg" in text:
            continue
        store = re.match(r"store .*?, ptr (%[\w.]+)(?:,|$)", text)
        if store and tokens == [store.group(1)]:
            writes.add(root_of[store.group(1)])
            continue
        fill = re.match(r"call void @llvm\.mem(?:set|cpy|move)[\w.]*\(ptr (?:[\w()]+ )*(%[\w.]+), (.*)", text)
        if fill and fill.group(1) in root_of and not any(t in root_of for t in TOKEN.findall(fill.group(2))):
            writes.add(root_of[fill.group(1)])
            continue
        for token in tokens:
            reads.add(root_of[token])
    return found


# Functions that some source file hands a cut 64-bit result to (see `narrowed`); filled while translating.
NARROW_TAKERS = set()

WIDE_CALL = re.compile(r"^\s+(%[\w.]+) = (?:tail )?call i64 @\"?(?:\\01)?(func_(?:L\d\d_)?[0-9A-Fa-f]{8})")


def narrowed(ll):
    """Returns {function: [callees]} from unoptimised LLVM IR: 64-bit results of `WIDE_RESULTS` cut to 32 bits.

    The retail code moves such a result on in a 64-bit register whatever the C says, so a
    function that keeps it in an `int` or hands it to an `int` parameter matches the retail
    bytes and loses the upper half on a host. The fix is the type in the decompilation.
    """
    found = {}
    function = None
    wide, cut_values = {}, set()
    for line in ll.split("\n"):
        start = DEFINE.match(line)
        if start:
            function, wide, cut_values = start.group(1), {}, set()
            continue
        call = WIDE_CALL.match(line)
        if call and call.group(2) in WIDE_RESULTS:
            wide[call.group(1)] = call.group(2)
            continue
        cut = re.match(r"^\s+(%[\w.]+) = trunc i64 (%[\w.]+) to i32", line)
        if cut and function and cut.group(2) in wide:
            found.setdefault(function, []).append(wide[cut.group(2)])
            cut_values.add(cut.group(1))
            continue
        # The cut value handed on: the function that takes it has a 32-bit parameter for it, and
        # loses the upper half itself when retail code calls it.
        taker = re.match(r"^\s+(?:%[\w.]+ = )?(?:tail )?call [^@]*@\"?(?:\\01)?(func_(?:L\d\d_)?[0-9A-Fa-f]{8})[^(]*\((.*)\)", line)
        if taker and any(value in re.findall(r"%[\w.]+", taker.group(2)) for value in cut_values):
            NARROW_TAKERS.add(taker.group(1))
    return found


def translate(job):
    """Compiles, links and translates one source file. Sets module.error when a step fails."""
    module, tools, game_dir = job
    filtered = module.stem.with_suffix(".c")
    text = module.source.read_text(errors="replace")
    # A wide function used before the file declares it would be taken to return `int`.
    wide = "".join(f"long {name}();\n" for name in sorted(WIDE_RESULTS) if re.search(rf"\b{name}\b", text))
    filtered.write_text(wide + f'#line 1 "{module.source}"\n' + own_labels(filter_source(text)))
    compile_command = [tools["clang"]] + CFLAGS + [
        f"-I{HERE / 'include'}", f"-I{game_dir / 'include'}", f"-I{module.source.parent}",
        "-c", str(filtered), "-o", str(module.stem.with_suffix(".o"))]
    link_command = [tools["wasm-ld"], "-shared", "--experimental-pic", "--import-memory", "--allow-undefined",
                    "--export-dynamic", "--fatal-warnings", str(module.stem.with_suffix(".o")),
                    "-o", str(module.stem.with_suffix(".wasm"))]
    layout_command = [tools["clang"]] + [flag for flag in CFLAGS if flag != "-O2"] + [
        "-O0", "-fno-discard-value-names", "-S", "-emit-llvm",
        f"-I{HERE / 'include'}", f"-I{game_dir / 'include'}", f"-I{module.source.parent}",
        str(filtered), "-o", str(module.stem.with_suffix(".ll"))]
    wasm2c_command = [tools["wasm2c"], str(module.stem.with_suffix(".wasm")), "--module-name", module.name,
                      "-o", str(module.stem) + "_w2c.c"]
    for step, command in (("compile", compile_command), ("link", link_command), ("translate", wasm2c_command)):
        error = run(command)
        if error:
            first = next((line for line in error.split("\n") if "error" in line), error.strip().split("\n")[0])
            module.error = f"{step}: {first[:200]}"
            return module
    # wasm2c takes a function for an import when an imported global has its name (the address
    # of a function defined in the module) and passes it that import's instance. It is the
    # module's own function: give it the module's instance.
    generated = Path(str(module.stem) + "_w2c.c")
    code = re.sub(r"instance->w2c_GOT0x2E(?:func|mem)_instance", "instance", generated.read_text())

    def drop_missing(match):
        # The same mix-up in the wrappers for tail calls from other modules, which nothing here
        # uses: a call to a helper that was never written is dropped.
        return match.group(0) if code.count(match.group(1) + "(") > 1 else ""

    code = re.sub(r"^\s*(wasm_tailcall_w2c_\w+)\(instance_ptr, tail_call_stack, next\);\n", drop_missing, code, flags=re.M)
    code = indirect_calls(code)
    code = console_floats(code)

    # Loads and stores at the console's device addresses go to the runtime's register model.
    start = code.index("#define DEFINE_LOAD(")
    end = code.index("DEFINE_LOAD(i32_load,")
    code = code[:start] + MEMORY_MACROS + code[end:]
    generated.write_text(code)
    read_interface(module, tools)
    # Which functions depend on the retail compiler's stack layout; without the listing, none are known.
    module.layout, module.narrow, module.variadic = {}, {}, set()
    if not run(layout_command):
        listing = module.stem.with_suffix(".ll").read_text(errors="replace")
        module.layout = written_only(listing)
        module.narrow = narrowed(listing)
        # A function defined with `...` takes its further arguments where the console's compiler put them.
        module.variadic = {match.group(1) for match in map(VARIADIC.match, listing.split("\n")) if match}
    return module


def read_interface(module, tools):
    """Reads what a translated module imports and exports from its generated header."""
    header = Path(str(module.stem) + "_w2c.h").read_text()
    function = re.compile(r"^(void|u32|u64|f32|f64) (w2c_\w+)\((?:struct w2c_env|w2c_" + module.name + r")\*((?:, \w+)*)\);$")
    for line in header.split("\n"):
        match = function.match(line)
        if match:
            result, c_name, rest = match.groups()
            parameters = [word for word in rest.replace(",", " ").split()]
            if c_name.startswith("w2c_env_"):
                module.imports.append((c_name, demangle(c_name[len("w2c_env_"):]), result, parameters))
            elif c_name.startswith(f"w2c_{module.name}_"):
                module.exports.append((c_name, demangle(c_name[len(f"w2c_{module.name}_"):]), result, parameters))
            continue
        match = re.match(r"^extern u32\* (w2c_GOT0x2E(?:mem|func)_(\w+))\(struct w2c_GOT0x2E(?:mem|func)\*\);$", line)
        if match:
            module.globals.append((match.group(1), demangle(match.group(2))))
    # How much memory the module's own data takes, from its dynamic-linking section.
    listing = subprocess.run([tools["wasm-objdump"], "-x", str(module.stem.with_suffix(".wasm"))],
                             capture_output=True, text=True).stdout
    size = re.search(r"mem_size\s*:\s*(\d+)", listing)
    align = re.search(r"mem_p2align\s*:\s*(\d+)", listing)
    module.data_bytes = int(size.group(1)) if size else 0
    module.data_align = 1 << int(align.group(1)) if align else 16
    module.has_table = "__indirect_function_table" in header
    module.accessors = re.findall(r"^extern (?:u32|wasm_rt_\w+)\* (w2c_env_\w+)\(struct w2c_env\*\);$", header, re.M)


class Retail:
    """The retail program's functions, from the user's own files: sizes and checksums."""

    def __init__(self, game_dir, key):
        import json
        import zlib
        self.crc32 = zlib.crc32
        self.leave = set()
        self.sizes = {}
        report = game_dir / "progress" / "report.json"
        if report.exists():
            for unit in json.loads(report.read_text())["units"]:
                for function in unit.get("functions", []):
                    self.sizes[function["name"]] = int(function["size"])
        self.symbols = {}  # (name, its address in each program) -> its number in the library's table of places
        places = {}  # name -> (size, {level number: [addresses]}), from the game's catalogue
        catalogue = game_dir / CATALOGUES.get(key, "none")
        if catalogue.is_file():
            for line in catalogue.read_text().split("\n"):
                fields = line.split("\t")
                if len(fields) >= 6 and not line.startswith("#"):
                    found = places.setdefault(fields[0], (int(fields[2]), {}))[1]
                    for place in fields[5].split(","):
                        found.setdefault(int(place[:2]), []).append(int(place[3:], 16))
        self.regions = {}  # level ("00") or None for the boot program -> [(address, bytes)]
        serial = json.loads((game_dir.parent / "game.json").read_text())["versions"][game_dir.name]["serial"]
        self.hooked = set()  # (None, address) of each boot-program function the runtime answers itself
        hooks = ROOT / "runtime" / "games" / f"{serial}.hooks"
        if hooks.is_file():
            for line in hooks.read_text().split("\n"):
                fields = line.split("#")[0].split()
                if len(fields) >= 2:
                    self.hooked.add((None, int(fields[0], 16)))
        boot = game_dir / "baserom" / serial
        gp = None
        if boot.exists():
            self.regions[None] = self.segments(boot.read_bytes())
            gp = self.global_pointer(boot.read_bytes())
        resident_end = 0xFFFFFFFF
        for folder in sorted((game_dir / "baserom" / "overlays").glob("level_*")):
            manifest = json.loads((folder / "manifest.json").read_text())
            text = next((r for r in manifest["records"] if r["name"] == "text"), None)
            if text and (folder / "text.bin").exists():
                self.regions[f"{manifest['level']:02d}"] = [(text["address"], (folder / "text.bin").read_bytes())]
                resident_end = min([resident_end] + [record["address"] for record in manifest["records"]])
        by_frame = {(int(level) if level else None): found for level, found in self.regions.items()}
        self.where = levels.Places(by_frame, places, self.sizes, gp, resident_end)

    @staticmethod
    def global_pointer(elf):
        """Returns the value the program's start-up gives the global pointer, from the ELF's register section."""
        import struct
        table, = struct.unpack_from("<I", elf, 32)
        entry_size, count = struct.unpack_from("<HH", elf, 46)
        for n in range(count):
            kind, _, _, offset = struct.unpack_from("<IIII", elf, table + n * entry_size + 4)
            # SHT_MIPS_REGINFO: four words of register masks... the sixth word is the pointer.
            if kind == 0x70000006:
                return struct.unpack_from("<I", elf, offset + 20)[0]
        return None

    @staticmethod
    def segments(elf):
        """Returns the loaded segments of an ELF file as (address, bytes)."""
        import struct
        table, = struct.unpack_from("<I", elf, 28)
        entry_size, count = struct.unpack_from("<HH", elf, 42)
        found = []
        for n in range(count):
            kind, offset, address, _, size, _ = struct.unpack_from("<IIIIII", elf, table + n * entry_size)
            if kind == 1 and size:
                found.append((address, elf[offset:offset + size]))
        return found

    def symbol(self, name, module):
        """Returns the number of a name, as a module uses it, in the library's table of places.

        Two modules that mean the same addresses by a name share a row; a module whose code
        calls another copy of a function that a level has twice gets a row of its own.
        """
        key = (name, tuple(self.where.column(name, module)))
        return self.symbols.setdefault(key, len(self.symbols))

    def levels(self):
        """Returns the numbers of the levels that have a program of their own, in order."""
        return sorted(int(level) for level in self.regions if level is not None)

    def place_rows(self):
        """Returns one C row for each numbered name: its address in the boot program, then in each level."""
        return ["    {" + ", ".join(f"0x{address:08X}u" for address in column) + f"}},  /* {name} */"
                for name, column in self.symbols]

    def copies(self, name, module):
        """Returns (address, size, crc, level or None) for each place a module's function may be used at.

        The place it is named after comes first. The others are its copies in the levels'
        programs, where `Places.usable` says that the module's names lead to the right addresses;
        a level may have several.
        """
        size = self.where.size_of(name)
        source = levels.frame_and_address(name)[0]
        found = []
        for frame in [source] + [level for level in self.where.levels if level != source]:
            for address in self.where.places(name, frame):
                code = self.where.bytes_at(frame, address, size)
                if code and self.where.usable(module, name, frame, address):
                    found.append((address, size, self.crc32(code) & 0xFFFFFFFF, frame))
        return found

    def probe_rows(self):
        """Returns one C row for each level: how the runtime tells that its program is in memory."""
        rows = []
        for level in self.levels():
            address, data = self.regions[f"{level:02d}"][0]
            run = data[:LEVEL_PROBE_BYTES]
            rows.append(f"    {{{level}, 0x{address:08X}u, {len(run)}, 0x{self.crc32(run) & 0xFFFFFFFF:08X}u}},")
        return rows

    def function(self, name):
        """Returns (address, size, crc) of a retail function by its name, or None when it is not on disk."""
        where = address_of(name)
        size = self.sizes.get(name)
        if not where or not size:
            return None
        address, level = where
        for base, data in self.regions.get(level, []):
            if base <= address and address + size <= base + len(data):
                return address, size, self.crc32(data[address - base:address - base + size]) & 0xFFFFFFFF
        return None


def left_to_the_interpreter(key):
    """Returns the names in the game's list of functions the host build leaves out (runtime/port/leave)."""
    return listed_names("leave", key)


def listed_names(folder, key):
    """Returns the first word of each line of a game's list in runtime/port/<folder>, comments apart."""
    listing = HERE / folder / (key.replace("/", "-") + ".txt")
    if not listing.exists():
        return set()
    names = set()
    for line in listing.read_text().split("\n"):
        fields = line.split()
        if fields and not fields[0].startswith("#"):
            names.add(fields[0])
    return names


def marshal(parameters):
    """Returns where each parameter travels, by the console's calling convention.

    A list of (place, number, type): place "r" is a general register, "f" an FPU register, "s" a
    stack slot of 8 bytes counted from the stack pointer. Integers and pointers go in registers 4
    to 11 in order, floats in FPU registers 12 to 19 in order, each kind counted by itself; what
    does not fit goes on the stack in the order of the parameters (seen in game code: the ninth
    integer of an 18-parameter call is stored at 0($sp), the next at 8($sp)). A double only
    arises from a call with no prototype or a variable argument list, where it travels as an
    integer of 64 bits.
    """
    plan = []
    integer, real, slot = 4, 12, 0
    for kind in parameters:
        if kind == "f32" and real < 20:
            plan.append(("f", real, kind))
            real += 1
        elif kind != "f32" and integer < 12:
            plan.append(("r", integer, kind))
            integer += 1
        else:
            plan.append(("s", slot, kind))
            slot += 1
    return plan, slot


# Why a decompiled function is left to the interpreter, and the file each kind is listed in beside the library.
LEFT = {
    "listed": ("are in the game's list of functions to leave (runtime/port/leave)", None),
    "hooked": ("are answered by the runtime itself (runtime/games/SERIAL.hooks)", None),
    "layout": ("fill locals that only a callee reads, through a neighbour's address", "left_by_stack_layout.txt"),
    "variadic": ("take a variable number of arguments", None),
    "narrow": ("keep a 64-bit result (runtime/port/wide) in 32 bits, or take one in a 32-bit parameter", "left_by_narrowing.txt"),
    "copy": ("use two copies of a function under one name", "left_by_copy.txt"),
    "caller": ("call one of the functions above directly, in the same source file", "left_by_call.txt"),
}


def decide(module, retail):
    """Sets which of a module's retail functions the library has (`module.own`) and which it leaves (`module.left`).

    Host code calls a function of its own source file directly, not through the game's
    memory, so a function that calls a left one directly is left as well: its callee would
    otherwise run as host code after all.
    """
    where = retail.where
    candidates = [plain for _, plain, result, _ in module.exports if retail.function(plain) and result != "f64"]
    # By the name, not by how the file declares it: a callback is sometimes declared as an array.
    named = {plain for _, plain, _, _ in module.imports if address_of(plain)} | {plain for _, plain in module.globals}
    module.function_names = sorted(plain for plain in named if plain.startswith("func_"))
    module.data_names = sorted(plain for plain in named if plain.startswith("D_"))
    module.left = {plain: "listed" for plain in candidates if plain in retail.leave}
    # A library function the runtime answers at its address: host code that called it directly
    # would run the decompiled body instead, so it is left, and its direct callers with it (below).
    module.left.update({plain: "hooked" for plain in candidates
                        if levels.frame_and_address(plain) in retail.hooked and plain not in module.left})
    module.left.update({plain: "layout" for plain in candidates if plain in module.layout and plain not in module.left})
    module.left.update({plain: "variadic" for plain in candidates if plain in module.variadic and plain not in module.left})
    module.left.update({plain: "narrow" for plain in candidates
                        if (plain in module.narrow or plain in NARROW_TAKERS) and plain not in module.left})

    def settle():
        module.own = [plain for plain in candidates if plain not in module.left]
        module.referenced, module.choices, module.usable, module.sorted = {}, {}, {}, {}
        where.prepare(module)

    settle()
    for plain in module.own:
        if not where.agrees(module, plain, levels.frame_and_address(plain)[0]):
            module.left[plain] = "copy"
    changed = True
    while changed:
        changed = False
        for plain in candidates:
            frame = levels.frame_and_address(plain)[0]
            called = where.calls(plain, frame) if plain not in module.left else ()
            if any(address in called for other in module.left for address in where.places(other, frame)):
                module.left[plain] = "caller"
                changed = True
    settle()


def write_glue(module, retail, out):
    """Writes the C that connects one translated module to the runtime. Returns how many functions it has."""
    name = module.name
    decide(module, retail)
    module.places = 0
    lines = [f"/* Generated by runtime/port/port.py from {module.source.name}. Do not edit. */",
             '#include "openrac_glue.h"', f'#include "{Path(str(module.stem) + "_names.h").name}"',
             f'#include "{Path(str(module.stem) + "_w2c.h").name}"', "",
             f"static w2c_{name} instance;", "static uint32_t memory_base;", "static uint32_t table_base;", ""]
    renames = []
    # The addresses of retail globals and functions the module names.
    for c_name, plain in module.globals:
        if not address_of(plain):
            module.unresolved.append(plain)
        unique = f"{name}_{c_name}"
        renames.append((c_name, unique))
        lines += [f"uint32_t* {unique}(struct {'w2c_GOT0x2Efunc' if c_name.startswith('w2c_GOT0x2Efunc') else 'w2c_GOT0x2Emem'}* unused) "
                  f"{{ (void)unused; return &openrac_place[{retail.symbol(plain, module)}]; }}"]
    for c_name in module.accessors:
        unique = f"{name}_{c_name}"
        renames.append((c_name, unique))
        if c_name.endswith("memory_base"):
            body = "return &memory_base;"
            kind = "uint32_t*"
        elif c_name.endswith("table_base"):
            body = "return &table_base;"
            kind = "uint32_t*"
        elif c_name.endswith("stack_pointer"):
            body = "return openrac_host->stack_pointer;"
            kind = "uint32_t*"
        elif c_name == "w2c_env_memory":
            body = "return &openrac_memory;"
            kind = "wasm_rt_memory_t*"
        else:
            body = "return &openrac_no_table;"
            kind = "wasm_rt_funcref_table_t*"
        lines.append(f"{kind} {unique}(struct w2c_env* unused) {{ (void)unused; {body} }}")
    lines.append("")
    # Calls that leave the module: through the registers to wherever the function is.
    for c_name, plain, result, parameters in module.imports:
        unique = f"{name}_{c_name}"
        renames.append((c_name, unique))
        where = address_of(plain)
        plan, slots = marshal(parameters)
        arguments = "".join(f", {C_TYPES[kind]} a{n}" for n, kind in enumerate(parameters))
        lines.append(f"{C_TYPES[result]} {unique}(struct w2c_env* unused{arguments}) {{")
        lines.append("    (void)unused;")
        if plain in BUILTINS:
            # What the compiler calls for a block copy or fill: done here, on guest memory.
            lines.append("    " + BUILTINS[plain])
        elif not where:
            lines.append(f'    openrac_stop("{plain} has no address in its name");')
            if result != "void":
                lines.append("    return 0;")
            module.stubs.append(plain)
        else:
            lines += pass_arguments(plan, slots)
            lines.append(f"    openrac_call(openrac_place[{retail.symbol(plain, module)}]);")
            lines += take_result(result, slots)
        lines.append("}")
    lines.append("")
    # The module's own functions, as retail code calls them.
    rows = []
    functions = 0
    for c_name, plain, result, parameters in module.exports:
        facts = retail.function(plain)
        plan, slots = marshal(parameters)
        if plain not in module.own:
            continue
        address, size, crc = facts

        def read(place, number, kind):
            if place == "f":
                return f"openrac_get_f({number})"
            source = f"openrac_get_r({number})" if place == "r" else f"openrac_load64((uint32_t)openrac_get_r(29) + {8 * number}u)"
            if kind == "u32":
                return f"(uint32_t){source}"
            if kind == "u64":
                return source
            return f"openrac_from_bits_{kind}({source})"

        call = ", ".join(["&instance"] + [read(place, number, kind) for place, number, kind in plan])
        lines.append(f"static void entry_{plain}(void) {{")
        if result == "void":
            lines.append(f"    {c_name}({call});")
        elif result == "f32":
            lines.append(f"    openrac_set_f(0, {c_name}({call}));")
        elif result == "u32":
            lines.append(f"    openrac_set_r32(2, {c_name}({call}));")
        else:
            lines.append(f"    openrac_set_r64(2, {c_name}({call}));")
        lines.append("}")
        returns = {"void": 0, "f32": 2}.get(result, 1)
        placed = retail.copies(plain, module)
        for address, size, crc, frame in placed:
            rows.append(f'    {{0x{address:08X}u, {size}, 0x{crc:08X}u, {"OPENRAC_NATIVE_BOOT" if frame is None else frame}, '
                        f'"{plain}", {returns}, entry_{plain}}},')
        module.places += len(placed)
        functions += 1
    # Setting the module up: its data's place, then the translated code's own start-up.
    starts = [c for c, plain, _, _ in module.exports if plain in ("__wasm_apply_data_relocs", "__wasm_call_ctors")]
    lines += ["", f"void openrac_start_{name}(void) {{",
              f"    memory_base = openrac_host->allocate(openrac_host->context, {module.data_bytes}, {module.data_align});",
              f"    wasm2c_{name}_instantiate(&instance" + ", 0" * module.instantiate_arguments + ");"]
    for wanted in ("__wasm_apply_data_relocs", "__wasm_call_ctors"):
        for c_name, plain, _, _ in module.exports:
            if plain == wanted:
                lines.append(f"    {c_name}(&instance);")
    lines += ["}", "", "/* Addresses the module keeps in its own data follow the level: they are written again. */",
              f"void openrac_relocate_{name}(void) {{"]
    lines += [f"    {c_name}(&instance);" for c_name, plain, _, _ in module.exports if plain == "__wasm_apply_data_relocs"]
    lines += ["}", "", f"const OpenracNativeFunction openrac_functions_{name}[] = {{"] + rows + [
        '    {0, 0, 0, 0, 0, 0, 0},', "};", ""]
    Path(str(module.stem) + "_glue.c").write_text("\n".join(lines))
    Path(str(module.stem) + "_names.h").write_text(
        "".join(f"#define {old} {new}\n" for old, new in renames))
    return functions


GLUE_HEADER = """\
/* Generated by runtime/port/port.py. What every module's glue shares. */
#ifndef OPENRAC_GLUE_H
#define OPENRAC_GLUE_H

#include <stdint.h>
#include <string.h>

#include "sys/native_abi.h"
#include "wasm-rt.h"

extern const OpenracHost* openrac_host;
/* What each numbered name stands for in the program that is in memory now (see openrac_main.c). */
extern uint32_t openrac_place[];
extern wasm_rt_memory_t openrac_memory;
extern wasm_rt_funcref_table_t openrac_no_table;

void openrac_stop(const char* why);

static inline uint64_t openrac_get_r(int n) { return openrac_host->get_r(openrac_host->context, n); }
static inline void openrac_set_r64(int n, uint64_t value) { openrac_host->set_r(openrac_host->context, n, value); }
/* A 32-bit value sits in a register sign-extended, as the console's instructions leave it. */
static inline void openrac_set_r32(int n, uint32_t value) {
    openrac_host->set_r(openrac_host->context, n, (uint64_t)(int64_t)(int32_t)value);
}
static inline float openrac_get_f(int n) {
    uint32_t bits = openrac_host->get_f(openrac_host->context, n);
    float value;
    memcpy(&value, &bits, 4);
    return value;
}
static inline void openrac_set_f(int n, float value) {
    uint32_t bits;
    memcpy(&bits, &value, 4);
    openrac_host->set_f(openrac_host->context, n, bits);
}
static inline void openrac_call(uint32_t address) { openrac_host->call(openrac_host->context, address); }
/* The console's float arithmetic (see native_abi.h): op 0 add, 1 subtract, 2 multiply, 3 divide. */
static inline float openrac_float(uint32_t op, float a, float b) {
    uint32_t x, y, z;
    float result;
    memcpy(&x, &a, 4);
    memcpy(&y, &b, 4);
    z = openrac_host->float_op(op, x, y);
    memcpy(&result, &z, 4);
    return result;
}
static inline int32_t openrac_float_order(float a, float b) {
    uint32_t x, y;
    memcpy(&x, &a, 4);
    memcpy(&y, &b, 4);
    return openrac_host->float_compare(x, y);
}
static inline float openrac_float_from_int(uint32_t value) {
    uint32_t z = openrac_host->float_op(5, value, 0);
    float result;
    memcpy(&result, &z, 4);
    return result;
}
static inline uint32_t openrac_float_to_int(float value) {
    uint32_t x;
    memcpy(&x, &value, 4);
    return openrac_host->float_op(6, x, 0);
}
/* Guest memory, for arguments that travel on the stack. */
static inline void openrac_store64(uint32_t at, uint64_t value) { memcpy(openrac_host->space + at, &value, 8); }
static inline uint64_t openrac_load64(uint32_t at) {
    uint64_t value;
    memcpy(&value, openrac_host->space + at, 8);
    return value;
}
/* A float or a double as the bits an integer register or a stack slot carries. */
static inline uint64_t openrac_bits_f32(float value) {
    uint32_t bits;
    memcpy(&bits, &value, 4);
    return bits;
}
static inline uint64_t openrac_bits_f64(double value) {
    uint64_t bits;
    memcpy(&bits, &value, 8);
    return bits;
}
static inline float openrac_from_bits_f32(uint64_t bits) {
    uint32_t low = (uint32_t)bits;
    float value;
    memcpy(&value, &low, 4);
    return value;
}
static inline double openrac_from_bits_f64(uint64_t bits) {
    double value;
    memcpy(&value, &bits, 8);
    return value;
}

#endif
"""


def write_main(modules, key, out_dir, retail):
    """Writes the library's own file: the runtime's side of wasm2c, the tables and the start function."""
    lines = ["/* Generated by runtime/port/port.py. Do not edit. */", '#include "openrac_glue.h"', "",
             "#include <stdio.h>", "#include <stdlib.h>", "",
             "const OpenracHost* openrac_host;", "wasm_rt_memory_t openrac_memory;",
             "wasm_rt_funcref_table_t openrac_no_table;", "",
             "/* What the translated code asks of its runtime. A trap is a bug in the build or in the C. */",
             "bool wasm_rt_is_initialized(void) { return true; }",
             "void wasm_rt_trap(wasm_rt_trap_t code) {",
             '    fprintf(stderr, "openrac: host code trapped (%d)\\n", (int)code);', "    abort();", "}",
             "void openrac_stop(const char* why) {",
             '    fprintf(stderr, "openrac: host code cannot go on: %s\\n", why);', "    abort();", "}", ""]
    for module in modules:
        lines.append(f"extern const OpenracNativeFunction openrac_functions_{module.name}[];")
        lines.append(f"void openrac_start_{module.name}(void);")
        lines.append(f"void openrac_relocate_{module.name}(void);")
    columns = len(retail.levels()) + 1
    count = max(len(retail.symbols), 1)
    lines += ["", "/* Each name's address in the boot program, then in each level's program, in the levels' order. */",
              f"static const uint32_t places[{count}][{columns}] = {{"] + (retail.place_rows() or ["    {0},"]) + ["};", "",
              f"uint32_t openrac_place[{count}];", "",
              "static const OpenracNativeLevel levels[] = {"] + retail.probe_rows() + ["    {0, 0, 0, 0},", "};", "",
              "static int started;", "",
              "/* Column 0 of the places is the boot program; a level is at its row of `levels` plus one. */",
              "static void set_level(int32_t level) {", "    uint32_t column = 0;",
              "    for (uint32_t n = 0; levels[n].size; n++) {",
              "        if (levels[n].level == level) {", "            column = n + 1;", "        }", "    }",
              f"    for (uint32_t n = 0; n < {count}; n++) {{", "        openrac_place[n] = places[n][column];", "    }",
              "    if (!started) {", "        return;", "    }"]
    lines += [f"    openrac_relocate_{module.name}();" for module in modules]
    lines += ["}", "", "static OpenracNativeFunction* table;", "static uint32_t count;", "",
              "static void start(const OpenracHost* host) {", "    openrac_host = host;",
              "    set_level(OPENRAC_NATIVE_BOOT);",
              "    openrac_memory.data = host->space;", "    openrac_memory.page_size = 65536;",
              "    openrac_memory.pages = 65536;", "    openrac_memory.max_pages = 65536;",
              "    openrac_memory.size = (uint64_t)1 << 32;"]
    lines += [f"    openrac_start_{module.name}();" for module in modules]
    lines += ["    started = 1;", "}", "", "static const OpenracNativeFunction* const lists[] = {"]
    lines += [f"    openrac_functions_{module.name}," for module in modules]
    lines += ["    0,", "};", "",
              "const OpenracNativeLibrary* openrac_native_library(void) {",
              "    static OpenracNativeLibrary library;", "    uint32_t n = 0;",
              "    for (uint32_t l = 0; lists[l]; l++) {",
              "        for (const OpenracNativeFunction* f = lists[l]; f->entry; f++) {", "            n++;", "        }", "    }",
              "    table = malloc((n ? n : 1) * sizeof(*table));", "    count = 0;",
              "    for (uint32_t l = 0; lists[l]; l++) {",
              "        for (const OpenracNativeFunction* f = lists[l]; f->entry; f++) {",
              "            table[count++] = *f;", "        }", "    }",
              "    library.abi = OPENRAC_NATIVE_ABI;", f'    library.game = "{key}";',
              "    library.count = count;", "    library.functions = table;", "    library.start = start;",
              f"    library.level_count = {len(retail.levels())};", "    library.levels = levels;",
              "    library.set_level = set_level;",
              "    return &library;", "}", ""]
    (out_dir / "openrac_main.c").write_text("\n".join(lines))
    (out_dir / "openrac_glue.h").write_text(GLUE_HEADER)


def host_compile(job):
    """Compiles one generated C file for the host. Returns (file, error or None)."""
    cc, source, include, forced = job
    command = [cc, "-O2", "-fPIC", "-w", "-fno-strict-aliasing", f"-I{source.parent}", f"-I{ROOT / 'runtime' / 'src'}",
               f"-I{include}", "-c", str(source), "-o", str(source.with_suffix(".host.o"))]
    if forced:
        command[1:1] = ["-include", str(source.parent / "openrac_glue.h"), "-include", str(forced)]
    return source, run(command)


def build(key, out_dir, only):
    """Builds the library for a game version. Returns its path."""
    game_dir = ROOT / "games" / key
    if key not in SOURCE_DIRS:
        raise SystemExit(f"port.py: no source folders are listed for {key}")
    sources = sorted(path for folder in SOURCE_DIRS[key] for path in (game_dir / folder).rglob("*.c")
                     if "movie" not in path.parts and (not only or only in str(path)))
    # The game's hand-written assembly routines that the port has in C (runtime/port/hand).
    sources += sorted(path for path in (HERE / "hand" / key.replace("/", "-")).glob("*.c") if not only or only in str(path))
    out_dir.mkdir(parents=True, exist_ok=True)
    tools = {name: find_tool(name) for name in ("clang", "wasm-ld", "wasm2c", "wasm-objdump")}
    modules = [Module(source, game_dir, out_dir, index) for index, source in enumerate(sources)]
    WIDE_RESULTS.update(listed_names("wide", key))
    with concurrent.futures.ThreadPoolExecutor() as pool:
        list(pool.map(translate, [(module, tools, game_dir) for module in modules]))
    good = [module for module in modules if not module.error]
    retail = Retail(game_dir, key)
    retail.leave = left_to_the_interpreter(key)
    total = 0
    for module in good:
        module.unresolved, module.stubs = [], []
        header = Path(str(module.stem) + "_w2c.h").read_text()
        signature = re.search(rf"void wasm2c_{module.name}_instantiate\(([^)]*)\);", header).group(1)
        module.instantiate_arguments = signature.count(",")
        module.functions = write_glue(module, retail, out_dir)
        total += module.functions
    # A module that names a global with no address in its name cannot be linked to the game's data.
    usable = [module for module in good if not module.unresolved]
    write_main(usable, key, out_dir, retail)
    include = Path(tools["wasm2c"]).resolve().parents[1] / "include"
    cc = shutil.which("cc") or tools["clang"]
    jobs = [(cc, out_dir / "openrac_main.c", include, None)]
    for module in usable:
        names = Path(str(module.stem) + "_names.h")
        jobs.append((cc, Path(str(module.stem) + "_w2c.c"), include, names))
        jobs.append((cc, Path(str(module.stem) + "_glue.c"), include, None))
    with concurrent.futures.ThreadPoolExecutor() as pool:
        compiled = list(pool.map(host_compile, jobs))
    failed = [(source, error) for source, error in compiled if error]
    for source, error in failed[:10]:
        first = next((line for line in error.split("\n") if "error" in line), error.strip().split("\n")[0])
        print(f"  host compile of {source.name}: {first[:200]}")
    if failed:
        raise SystemExit(f"port.py: {len(failed)} generated files did not compile for the host")
    library = out_dir / ("libopenrac-native.dylib" if sys.platform == "darwin" else "libopenrac-native.so")
    objects = [str(source.with_suffix(".host.o")) for source, _ in compiled]
    listing = out_dir / "objects.txt"
    listing.write_text("\n".join(objects) + "\n")
    link = [cc, "-shared", "-o", str(library)] + (["-Wl,-filelist," + str(listing)] if sys.platform == "darwin" else ["@" + str(listing)])
    error = run(link)
    if error:
        raise SystemExit("port.py: linking the library failed:\n" + error[:2000])
    print(f"{len(sources)} source files: {len(good)} translated, {len(usable)} in the library, "
          f"{sum(module.functions for module in usable)} functions at {sum(module.places for module in usable)} places")
    for module in modules:
        if module.error:
            print(f"  left out, {module.source.relative_to(game_dir)}: {module.error}")
    for module in good:
        if module.unresolved:
            print(f"  left out, {module.source.relative_to(game_dir)}: no address for {', '.join(sorted(set(module.unresolved))[:4])}")
    for why, (words, file_name) in LEFT.items():
        names = sorted(plain for module in usable for plain, reason in module.left.items() if reason == why)
        if names and file_name:
            (out_dir / file_name).write_text("".join(f"{plain}\n" for plain in names))
        if names:
            print(f"  {len(names)} functions are left to the interpreter: they {words}" + (f" ({file_name})" if file_name else ""))
    stubs = sorted({plain for module in usable for plain in module.stubs})
    if stubs:
        print(f"  {len(stubs)} callees cannot be called yet (the call stops the game): {', '.join(stubs[:8])}")
    print(library)
    BUILT.update(modules=modules, usable=usable, retail=retail)
    return library


# What the last `build` in this process worked out, for `wanted`.
BUILT = {}


def wanted(key, out_dir, files, most):
    """Prints the guest functions a run called that no host function stood in for, the busiest first.

    `files` are what `openrac-boot --native-calls FILE` wrote (one run each, say one a level).
    Each line of the answer is a function by its name in the decompilation, its size, how
    often it was called, in which levels, and why it is not host code: not decompiled yet, or
    decompiled and left out. This is the list of what to decompile next for the port.
    """
    import json
    build(key, out_dir, None)
    retail, modules, usable = BUILT["retail"], BUILT["modules"], BUILT["usable"]
    where = retail.where
    # Which catalogued function has a place at an address of a level's program.
    owner = {}
    for name, (_, by_level) in where.catalogue.items():
        for level, addresses in by_level.items():
            for address in addresses:
                owner[(level, address)] = name
    matched = set()
    report = ROOT / "games" / key / "progress" / "report.json"
    if report.exists():
        for unit in json.loads(report.read_text())["units"]:
            matched.update(f["name"] for f in unit.get("functions", []) if f.get("fuzzy_match_percent") == 100.0)
    by_hand = set()
    listing = ROOT / "games" / key / HANDWRITTEN.get(key, "none")
    if listing.is_file():
        by_hand = {line.strip().split("/")[-1] for line in listing.read_text().split("\n") if line.strip() and not line.startswith("#")}
    status = {}
    for module in modules:
        built = module in usable
        for _, plain, _, _ in (module.exports if not module.error else []):
            if not built:
                status[plain] = "its source file is not in the library"
            elif plain in module.left:
                status[plain] = "left out: they " + LEFT[module.left[plain]][0]
            elif plain in module.own:
                status[plain] = "in the library, not usable at this place"
    calls = {}
    for path in files:
        for line in Path(path).read_text().split("\n"):
            fields = line.split()
            if len(fields) != 4 or fields[3] == "1":
                continue
            level, address, count = int(fields[0]), int(fields[1], 16), int(fields[2])
            if level >= 0 and (level, address) in owner:
                name = owner[(level, address)]
            elif level >= 0 and address >= where.resident_end:
                name = f"func_L{level:02d}_{address:08X}"
            else:
                name = f"func_{address:08X}"
            entry = calls.setdefault(name, [0, set()])
            entry[0] += count
            entry[1].add(level)
    total = sum(entry[0] for entry in calls.values())
    print(f"{len(calls)} guest functions were called with no host function standing in, {total} calls")
    for name, (count, seen) in sorted(calls.items(), key=lambda item: -item[1][0])[:most]:
        size = where.size_of(name)
        if name in status:
            why = status[name]
        elif name in by_hand:
            why = "hand-written assembly: the port needs a C version of its own (runtime/port/hand)"
        elif name in matched:
            why = "decompiled; its source file did not build for the host"
        elif size:
            why = "not decompiled"
        else:
            why = "not a function the decompilation names (a library routine, or the middle of one)"
        levels_seen = ",".join("boot" if level < 0 else str(level) for level in sorted(seen))
        print(f"{count:10d}  {name:<22} {size:6d}  levels {levels_seen}: {why}")


def main():
    arguments = sys.argv[1:]
    if len(arguments) >= 2 and arguments[0] == "build":
        out = ROOT / "build" / "port" / arguments[1].replace("/", "-")
        only = None
        if "--out" in arguments:
            out = Path(arguments[arguments.index("--out") + 1])
        if "--only" in arguments:
            only = arguments[arguments.index("--only") + 1]
        build(arguments[1], out, only)
    elif len(arguments) >= 3 and arguments[0] == "wanted":
        out = ROOT / "build" / "port" / arguments[1].replace("/", "-")
        most = 60
        files = arguments[2:]
        if "--most" in files:
            most = int(files[files.index("--most") + 1])
            del files[files.index("--most"):files.index("--most") + 2]
        wanted(arguments[1], out, files, most)
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main()
