"""Run m2c on one of our splat .s files, with ctx.c.

asm/ uses numeric GPR names ($4, $31) because the SN assembler accepts
nothing else (see tools/sn_regnames.py). m2c needs symbolic names -- with
$31 it cannot recognise `jr $31` as a return and gives up looking for a
jump table. This maps the names back into a temporary copy and runs m2c.
Splat puts jump tables in asm/data/ and labels none of their targets, so
the copy also gets each table it uses, as .rodata, and a label at every
target.
Output is a reference sketch only: it is never matching as emitted.

usage: python tools/m2c.py func_XXXXXXXX [extra m2c args]
"""
import pathlib, re, subprocess, sys, tempfile

NAMES = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
         "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
         "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
         "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
# Longest numbers first is unnecessary thanks to the \b, but keep COP
# registers ($f12) untouched: only a bare $<digits> is a GPR.
GPR = re.compile(r"\$(\d{1,2})\b")

JTBL = re.compile(r"%hi\((jtbl_[0-9A-Fa-f]{8})\)")
INSN = re.compile(r"^(\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/)", re.M)


def jump_tables(text):
    names = sorted(set(JTBL.findall(text)))
    if not names:
        return text
    data = "\n".join(p.read_text() for p in pathlib.Path("asm/data").glob("*.s"))
    here = {m.group(2) for m in INSN.finditer(text)}
    rodata, targets = [".section .rodata"], set()
    for name in names:
        m = re.search(rf"dlabel {name}\n(.*?)enddlabel {name}", data, re.S)
        if m:
            words = []
            # A table ends at the first word that is no address in this
            # function: splat's data block can run on into padding or strings.
            for w in re.findall(r"\.word 0x([0-9A-Fa-f]{8})", m.group(1)):
                if w.upper() not in here:
                    break
                words.append(w.upper())
            targets |= set(words)
            rodata += [f"glabel {name}"] + [f".word .L{w}" for w in words]
    text = INSN.sub(lambda m: f".L{m.group(2)}:\n{m.group(1)}"
                    if m.group(2) in targets and f".L{m.group(2)}:" not in text else m.group(1), text)
    return text + "\n" + "\n".join(rodata) + "\n"


def main():
    name = sys.argv[1]
    hits = list(pathlib.Path("asm/nonmatchings").rglob(name + ".s")) \
        or list(pathlib.Path("asm/overlays").glob(name + ".s"))      # level code (docs/OVERLAYS.md)
    if len(hits) != 1:
        sys.exit(f"expected one .s for {name}, found {len(hits)}")
    text = hits[0].read_text()
    text = GPR.sub(lambda m: "$" + NAMES[int(m.group(1))]
                   if int(m.group(1)) < 32 else m.group(0), text)
    text = jump_tables(text)
    with tempfile.NamedTemporaryFile("w", suffix=".s", delete=False) as t:
        t.write(text)
    ctx_file = pathlib.Path("ctx.c")
    if not ctx_file.exists():
        try:
            sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
            from gen_ctx import generate_ctx
            generate_ctx()
        except Exception:
            pass

    ctx = ["--context", "ctx.c"] if ctx_file.exists() else []
    cmd = [sys.executable, "tools/ext/m2c/m2c.py", "-t", "mips-gcc-c",
           *ctx, *sys.argv[2:], t.name]
    sys.exit(subprocess.call(cmd))

main()
