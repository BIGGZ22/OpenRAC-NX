# The gates of this project

A gate is a private script that rebuilds something and compares it with the
retail bytes. There are three of them, and each one proves something the
others do not. This page says what each gate measures, what it does not
measure, and how to run it.

The vocabulary is in `docs/EVIDENCE.md`: a **placement** is one address in
one module, a **source** is one authored file, and **bytes** are counted by
placement. The three counts are always reported separately.

## 1. The body gate (strict, private)

`work/E0/harness.py`, driven by the wave agents.

```
python3 <private workspace>/E0/harness.py FE.REL 0x1388 source.c
```

It compiles one authored source with the pinned chain (`cpp`, `cc1`, `as`),
links its `.text` at the measured module offset with the SN ProDG linker,
resolves the HI16/LO16 fixups and the imported calls from the retail
relocation tables, and compares **every byte** with the retail body.

- It proves: the instruction sequence and its encoding, at one placement.
- It does not prove: that the placement is right, that the surrounding code
  is untouched, that the data references resolve the same way, or that the
  source is the original one.
- `*** EXACT ***` is the only accepted result. A refusal (exit code 2) is
  not an attempt. A near miss is not a match.
- The negative control lives in `work/E0/preuves/neg/`: a body that differs
  by one constant is refused, and the report names the exact byte.

## 2. The placement gate (public, tracked)

`tools/placement_gate.py`, run by the root before a commit.

```
python3 tools/placement_gate.py --modules <private dir> --bounds <private dir> \
                                --harness <private harness.py>
```

For every row of `config/levels/bodies.tsv` it checks that

- the placement lies inside the candidate code interval of its module;
- the extent is the one the bounds measured, and the row is a bounded
  candidate, not an interval guess;
- no two placements of a module overlap;
- the **published** source still compiles to the retail bytes of that
  placement (the body gate, re-run on the file that is in the repository).

It prints three counts -- placements, sources, bytes by placement -- and
never a percentage.

- It proves: the ownership of the bytes, and that the published sources
  still match the retail bodies.
- It does not prove: anything about the program that contains them.

## 3. The module gate (public, tracked)

`tools/module_gate.py`, run by the root before a commit.

```
python3 tools/module_gate.py --modules <private dir> --harness <private harness.py>
```

For every module that carries proved bodies, the gate rebuilds each body
from its **published source**, writes the linked bytes at their measured
offsets in a copy of the retail module, and requires the result to equal
the retail module, SHA-256 included.

With `--baseline`, the code interval of the module is not copied from the
retail image but **rebuilt from the assembly baseline**: the interval is
rendered as R5900, assembled and linked, and the proved bodies are written
over it. The module that comes out is a rebuild -- of its code -- and not a
copy; its data, which no C source covers yet, is still the retail's.

With `--image`, the gate writes the rebuilt modules back into a copy of the
disc image, at their recorded extents, and requires the image to hash to
the pinned retail hash.  That is the program-level half of the evidence
standard, applied to the whole disc.

- It proves: the bodies belong where they are placed, they do not disturb
  their surroundings, and the module -- and the image that carries it --
  that contains them is the retail one.
- It does **not** prove: that the parts of the program that have no
  accepted source yet are rebuilt from source.  With `--baseline` they are
  rebuilt from the *assembly* baseline, and without it they are still the
  retail bytes.  This gate is the last step before the full rebuild, not
  the full rebuild.

## 4. The complete-program gate (missing -- the next root task)

This is the gate the evidence standard requires before a body is *promoted*:
rebuild the complete program, and require its hash to equal the pinned
retail hash.

For this disc the program is **one boot executable plus 31 relocatable
modules**, so the gate has to treat both:

1. **The assembly baseline** of a module. The candidate code interval of a
   module is disassembled as R5900, assembled with the SN assembler and
   linked; the result must equal the retail code interval byte for byte.
   This proves that the module's code is the code the interval holds, and
   it is the baseline every later C gate stands on.
   **This stage is qualified**: `tools/assembly_baseline.py` rebuilds the
   code interval of all 31 modules exactly -- 69 483 520 bytes -- and
   `config/levels/assembly_baseline.tsv` records the result.
2. **The module rebuild.** Every body of the module that has an accepted C
   source is compiled and linked at its placement; the module is rebuilt
   from the link of the whole set, and the rebuilt module must equal the
   retail module -- including the bytes outside the rebuilt bodies.
   *The module gate of section 3 already does the two halves of this: with
   `--baseline` it rebuilds the code interval from the **assembly
   baseline** and writes the proved C bodies over it, and the result is
   byte-identical to the retail module.  What is missing is the rebuild of
   the bodies that have no accepted source yet: today they come from the
   assembly baseline, and they must come from C.*
3. **The disc rebuild.** The boot executable and the 31 modules are put back
   at their recorded extents, and the rebuilt image must hash to the pinned
   retail hash.

Until the gate exists, the bodies published in `src/` are **proved, not
promoted**: the published numbers carry the body-level proof only, and no
percentage is published.
