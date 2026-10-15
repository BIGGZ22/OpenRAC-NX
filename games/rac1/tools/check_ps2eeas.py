#!/usr/bin/env python3
"""
Checks the build's assembler steps against SN's real assembler (in the container).

  bash tools/docker/run.sh python tools/check_ps2eeas.py [SRC.c ...]

Retail's game code was assembled by SN's ps2eeas. This build assembles with
GNU as and adds what ps2eeas does differently with tools/ps2eeas_nops.py,
tools/ps2eeas_dli.py and tools/check_macro_slots.py (docs/BUILD_FIDELITY.md).
This tool takes every C file of the game code (src/game/, src/overlays/;
or the files given), builds it as the build does (tools/try_func.py's
pipeline), and assembles the same compiler output a second time with
ps2eeas itself: SDK 2.4's 1.9.6.516 by default, ProDG 3.01's 1.9.25.758 with
--as=1.9.25, or any other build with --as=PATH. Then it compares every C function of the two objects, words
with a relocation in either object left out (GNU as fills some fields in
itself, a local call or an absolute symbol's %hi, where ps2eeas leaves them
to the linker; both link to the same bytes).

ps2eeas cannot read include/labels.inc or include/macro.inc (GNU assembler
macros, which made it overflow its stack in earlier attempts), so the
`.include` lines the source's INCLUDE_ASM header and stubs add are dropped
from its input: the retail-assembly stubs vanish from its object, and only
the C functions are compared.

Verdicts per function:
  same   identical code;
  gp     identical once ps2eeas knows the externs' sizes before their first
         use: the same input assembled again with the compiler's
         `.extern SYM, SIZE` lines, which GCC 2.95 writes at the end of the
         file, also at the top. ps2eeas assembles in one pass and only uses
         $gp for a symbol whose size it has seen, so as it is it reaches
         those globals with lui/%lo (and adds its load-delay nops after
         them). Retail had the sizes early: a definition in the same file,
         or an `.extern` hint, as rac3-uya-decomp writes them;
  diff   anything else, which would mean a step models ps2eeas wrongly,
         or that retail's ps2eeas build differs from these.

Writes build-sn/check_ps2eeas/<as>.tsv and prints the totals.
"""
import os
import re
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from elftools.elf.elffile import ELFFile  # noqa: E402
from elftools.elf.relocation import RelocationSection  # noqa: E402
import gen_progress_report as gpr  # noqa: E402
import try_func  # noqa: E402
from toolchain import sn, start_wineserver  # noqa: E402

ASSEMBLERS = {"1.9.6": "toolchain/sn-prodg-24/local/sce/ee/gcc/ee/bin/ps2eeas.exe",
              "1.9.25": "toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/ee/bin/Ps2EeAs.exe"}
WORK = Path("build-sn/check_ps2eeas")
# Functions defined under their readable names (include/names.h maps each to its func_ name).
ALIAS = {m.group(1): m.group(2) for m in re.finditer(r"^#define\s+(\w+)\s+(func_[0-9A-F]{8})\b",
                                                      Path("include/names.h").read_text(), re.M)}
DEF_ANY = re.compile(r"^(?!extern\b|static\b|typedef\b|#)[A-Za-z_][\w \t\*]*?\b(\w+)\s*\([^;]*$", re.M)
EXTERN = re.compile(r"^\s*\.extern\s+\w+\s*,\s*\d+\s*$")


def functions(path):
    """{name: (words, relocated word offsets)} for every sized function in an object."""
    elf = ELFFile(open(path, "rb"))
    text = elf.get_section_by_name(".text").data()
    rel = set()
    for sec in elf.iter_sections():
        if isinstance(sec, RelocationSection) and sec.name == ".rel.text":
            rel.update(r["r_offset"] & ~3 for r in sec.iter_relocations())
    out = {}
    for s in elf.get_section_by_name(".symtab").iter_symbols():
        if s["st_info"]["type"] == "STT_FUNC" and s["st_size"]:
            o = s["st_value"]
            out[s.name] = ([int.from_bytes(text[i:i + 4], "little") for i in range(o, o + s["st_size"], 4)],
                           {i - o for i in range(o, o + s["st_size"], 4) if i in rel})
    return out


def same(build, real):
    (bw, brel), (rw, rrel) = build, real
    return len(bw) == len(rw) and all(a == b or 4 * i in brel or 4 * i in rrel
                                      for i, (a, b) in enumerate(zip(bw, rw)))


def check_file(job):
    src, assembler = job
    src = Path(src)
    if "overlays" in src.parts:
        c_names = [n for n, is_c in gpr.overlay_file_functions(src) if is_c]
        seg = "text"
    else:
        text = src.read_text(errors="replace")
        names = set(gpr.FUNC_DEF.findall(text)) | {ALIAS[m] for m in DEF_ANY.findall(text) if m in ALIAS}
        c_names = sorted(n for n in names if n not in set(gpr.STUB.findall(text)))
        seg = next(s for s, srcs in gpr.SEGMENT_SOURCES.items() if str(src) in map(str, srcs))
    if not c_names:
        return []
    lines = src.read_text(errors="replace").splitlines()
    work = WORK / "work" / src.parent.name / src.stem
    obj = try_func.build(c_names[0], seg, src, 0, len(lines) - 1, "\n".join(lines) + "\n", work)
    if obj is None:
        return [(str(src), "-", "BUILD FAILED")]
    raw = work / "a.s"
    lines_s = [l for l in raw.read_text(errors="replace").splitlines(True) if ".include" not in l]
    # The same input with the compiler's `.extern SYM, SIZE` lines (written at the end of
    # the file) also at the top, so the one-pass assembler knows every size before use.
    externs = [l for l in lines_s if EXTERN.match(l)]
    objs = {}
    for kind, text in (("as_is", lines_s), ("sizes_first", externs + lines_s)):
        inp = work / f"ps2eeas_{kind}.s"
        inp.write_text("".join(text))
        out = work / f"ps2eeas_{assembler}_{kind}.o"
        out.unlink(missing_ok=True)
        r = subprocess.run(sn(ASSEMBLERS[assembler], "-G2", "-o", str(out), str(inp)),
                           capture_output=True, text=True)
        if r.returncode or not out.exists():
            return [(str(src), "-", "PS2EEAS FAILED")]
        objs[kind] = functions(out)
    ours = functions(obj)

    def verdict(n):
        if n not in ours or n not in objs["as_is"]:
            return "missing"
        if same(ours[n], objs["as_is"][n]):
            return "same"
        if n in objs["sizes_first"] and same(ours[n], objs["sizes_first"][n]):
            return "gp"
        return "diff"
    return [(str(src), n, verdict(n)) for n in c_names]


def main() -> None:
    assembler = next((a.split("=", 1)[1] for a in sys.argv if a.startswith("--as=")), "1.9.6")
    if assembler not in ASSEMBLERS:      # a path: another build of the assembler, for experiments
        ASSEMBLERS[Path(assembler).stem] = assembler
        assembler = Path(assembler).stem
    files = [a for a in sys.argv[1:] if not a.startswith("--")]
    if not files:
        game = [str(p) for p in gpr.SEGMENT_SOURCES.get("text", [])]
        files = game + sorted(str(p) for p in Path("src/overlays").glob("*/*.c"))
    start_wineserver()
    WORK.mkdir(parents=True, exist_ok=True)
    with ProcessPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        rows = [r for found in pool.map(check_file, [(f, assembler) for f in files]) for r in found]
    (WORK / f"{assembler}.tsv").write_text("".join(f"{v}\t{n}\t{s}\n" for s, n, v in rows))
    counts = {}
    for _, _, v in rows:
        counts[v] = counts.get(v, 0) + 1
    print(f"ps2eeas {assembler}: {len(rows)} C functions; " +
          ", ".join(f"{v} {n}" for v, n in sorted(counts.items(), key=lambda x: -x[1])))
    for s, n, v in rows:
        if v not in ("same", "gp"):
            print(f"  {v}\t{n}\t{s}")


if __name__ == "__main__":
    main()
