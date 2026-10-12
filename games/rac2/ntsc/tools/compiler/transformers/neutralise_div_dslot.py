"""Make single-precision COP1 division ineligible for a delay slot.

`dbr_schedule` (reorg.c), reached from `toplev.c` under `DELAY_SLOTS` whenever
`optimize > 0 && flag_delayed_branch`, fills delay slots with a backward scan
and no `may_trap_p` guard. Whether a candidate may enter a slot is decided by
`eligible_for_delay`, generated from the `define_delay` forms of `mips.md`,
which accept an instruction only when its `dslot` attribute is "no" and its
length is one. `divsf3` is `type=fdiv, mode=SF, length=1`, so `div.s` is
eligible today and the scheduler moves it into a following `jal` slot.

The retail build never does that: a census of the pinned reference image
finds **0 of 19 806 `div.s` occupancies in a delay slot**, while integer
HI/LO division does occupy **61** slots. The oracle agrees: the authentic
SDK 3.01 assembler refuses a `div.s` in a delay slot ("Automatic padding
cannot take place") on the emitted assembly of the affected family, and
accepts the same body once the scheduler leaves the division in place.

The transformer adds the division/root family to the `dslot` barrier list.
Single precision only: `div.d`, `sqrt.d` and `rsqrt.d` are already rejected
by the SDK assembler outright and are not measured here, and integer
HI/LO division must stay eligible for the 61 retail slots.

Reserve: this is a **sufficient** mechanism, not the recovered original rule.
The retail compiler's own reason for keeping `div.s` out of slots is not
known; `reorg.c` and `rtlanal.c` in the qualified tree are byte-identical to
the pristine `gnu-ee-binutils-gcc-1.1.tar.gz` archive, so the difference is
not a RAC2 source adjustment that was lost. `sqrt.s` and `rsqrt.s` carry no
witness at all in the retail image (0 occurrences of each), so for them the
attribute change is an extrapolation from the same COP1 family.

The replacement text is part of the qualified source identity: a differently
worded attribute expression produces a different `mips.md` and therefore a
different `cc1` hash, even where the generated tables are identical.

Usage: neutralise_div_dslot.py <gcc source directory>
"""
from pathlib import Path
import sys

OLD = '''(define_attr "dslot" "no,yes"
  (if_then_else (ior (eq_attr "type" "branch,jump,call,xfer,hilo,fcmp")
\t\t     (and (eq_attr "type" "load")
\t\t\t  (and (eq (symbol_ref "mips_isa") (const_int 1))
\t\t\t       (and (eq (symbol_ref "mips16") (const_int 0))
                                    (eq_attr "cpu" "!r3900")))))
\t\t(const_string "yes")
\t\t(const_string "no")))'''

NEW = '''(define_attr "dslot" "no,yes"
  (if_then_else (ior (ior (eq_attr "type" "branch,jump,call,xfer,hilo,fcmp")
\t\t\t  (and (eq_attr "mode" "SF")
\t\t\t       (eq_attr "type" "fdiv,fsqrt,frsqrt")))
\t\t     (and (eq_attr "type" "load")
\t\t\t  (and (eq (symbol_ref "mips_isa") (const_int 1))
\t\t\t       (and (eq (symbol_ref "mips16") (const_int 0))
                                    (eq_attr "cpu" "!r3900")))))
\t\t(const_string "yes")
\t\t(const_string "no")))'''


def main() -> int:
    gcc = Path(sys.argv[1])
    md = gcc / "config" / "mips" / "mips.md"
    source = md.read_text(encoding="utf-8")

    if NEW in source:
        print("division-family dslot barrier already present in %s" % md)
        return 0

    assert source.count(OLD) == 1, "anchor: the dslot attribute definition"
    md.write_text(source.replace(OLD, NEW, 1), encoding="utf-8", newline="")
    print("Single-precision COP1 division removed from delay-slot candidates")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
