"""Extend the zero-store fold to the function-value `use` that reads it.

Measured wall (shared family `904cc63b097639e5`, 320 bytes x 28 placements): a
body that stores a TImode zero and returns the constant 0 makes reload
materialise the zero once in `$v0`, the function-value register, and degenerate
the return set into `(use (reg/i:SI 2 v0))`.  `rac2_reg_live_after_store_p` --
the guard that makes `rac2_fold_zero_ti_store` sound -- then sees a reference
after the store and refuses the fold, so the chain prints
`por $v0,$zero,$zero` + `sq $v0,0($base)` where the retail prints
`sq $zero,0($base)` + `move $v0,$zero`.

The rule being extended, cited as it stands in the qualified `mips.c`: the fold
is allowed **if and only if**, scanning forward from the store, the register is
rewritten before any read, or the function ends without reading it -- the store
is the last use of the register.  The guard is the safety condition of deleting
the materialisation, so an exception may only be admitted for a reader whose
value is *reconstructible*.

This transformer keeps that rule and adds exactly one case.  When the only
reader after the store is the function-value `(use (reg/i ...))`, reached in
straight line (no jump or call in between) and only one of them, the store folds
to architectural zero *and* the return value is re-materialised as a plain
`(set (reg:SI 2) (const_int 0))` in place of that `use`.  Deleting the shared
materialisation is then sound: nothing else reads the register, and the return
the function value that the deleted instruction held is reconstructed from the
same constant the fold writes.  The retail prints exactly that pair.

Case 2 is deliberately bounded: an exact `(use (reg/i ...))` pattern, the same
register, `REG_FUNCTION_VALUE_P` set, no jump or call between the store and the
use, and a single such `use`.  A zero shared with any other reconstructible
consumer (a call argument, a phi) stays refused.

Both changes go into the **same** `validate_change` / `apply_change_group`
group: if either replacement is not recognised, the whole group is abandoned and
nothing is folded -- a safe degradation that can never leave broken RTL.

Two measured counter-proofs are recorded with this file, because they stop a
later simplification of it:

* Variant C, "treat the `(use (reg v0))` as an end of life", is **not enough**.
  Measured: the store folds but no `move $v0,$zero` is emitted, so the function
  returns whatever `$v0` holds -- the null path's `jal CALLEE0` result.  That is
  silent corruption, not a one-instruction miss.  The return value must be
  *re-materialised*, which is what this script does.
* Variant B, re-admitting constraint `J` on the store alternatives of
  `movti_internal` (the retired `allow_zero_ti_store.patch`), reproduces this
  family as well and passes the 55 published sources, but it **regresses**
  `41eb487e64fb6b76`: the constraint is a global rule that removes the shared
  `por $rd,$zero,$zero` the retail keeps.  The local pass is the only one that
  distinguishes the two cases, so the constrained route must stay closed.

The annotation inside the replacement carries French text: it is part of the
qualified `mips.c` identity, so keep it as it stands.

Usage: fold_zero_ti_return_value.py <gcc source directory>   (the one holding
toplev.c).  Applies after `fold_zero_ti_store.py`, whose guard it replaces.
"""
from pathlib import Path
import sys

FUNC_HEAD = 'static int\nrac2_reg_live_after_store_p (insn, reg)'

NEW_FUNC = "/* RAC2 : le zero TImode materialise peut aussi etre lu par la valeur de\n   retour (`use (reg/i ...)`).  Dans ce cas le store se replie quand meme sur\n   $zero ; le retour est rematerialise en clair dans l'insn de retour, ce qui\n   redonne le couple retail `sq $zero,<off>(<base>)` + `move $v0,$zero`.  */\nstatic rtx rac2_fvu_insn = 0;\t/* le (use (reg/i ...)) reconnu */\nstatic rtx rac2_fvu_reg = 0;\t/* son registre, mode compris */\n\nstatic int\nrac2_reg_live_after_store_p (insn, reg)\n     rtx insn;\n     rtx reg;\n{\n  rtx scan;\n  int past_control = 0;\n\n  rac2_fvu_insn = 0;\n  rac2_fvu_reg = 0;\n\n  for (scan = NEXT_INSN (insn); scan; scan = NEXT_INSN (scan))\n    {\n      rtx pat, set;\n\n      if (GET_CODE (scan) == JUMP_INSN || GET_CODE (scan) == CALL_INSN)\n\tpast_control = 1;\n\n      if (GET_CODE (scan) != INSN\n\t  && GET_CODE (scan) != JUMP_INSN\n\t  && GET_CODE (scan) != CALL_INSN)\n\tcontinue;\n\n      pat = PATTERN (scan);\n      if (pat == 0 || GET_CODE (pat) == SEQUENCE)\n\tcontinue;\n\n      if (reg_referenced_p (reg, pat))\n\t{\n\t  rtx used = GET_CODE (pat) == USE ? XEXP (pat, 0) : 0;\n\n\t  /* Lecture par la valeur de retour, en ligne droite, une seule\n\t     fois : le zero du store peut etre rematerialise pour le retour.  */\n\t  if (used != 0 && GET_CODE (used) == REG\n\t      && REGNO (used) == REGNO (reg)\n\t      && REG_FUNCTION_VALUE_P (used)\n\t      && past_control == 0\n\t      && rac2_fvu_insn == 0)\n\t    {\n\t      rac2_fvu_insn = scan;\n\t      rac2_fvu_reg = used;\n\t      continue;\t\t\t/* pas une lecture de donnees */\n\t    }\n\n\t  return 1;\t\t\t/* consomme avant toute reecriture */\n\t}\n\n      set = single_set (scan);\n      if (set != 0 && GET_CODE (SET_DEST (set)) == REG\n\t  && REGNO (SET_DEST (set)) == REGNO (reg))\n\treturn rac2_fvu_insn != 0 ? 2 : 0;\t/* reecrit : fin de vie */\n    }\n\n  return rac2_fvu_insn != 0 ? 2 : 0;\n}\n"

OLD_DECL = '      rtx set, prev, pset, reg;\n      int steps;'

NEW_DECL = '      rtx set, prev, pset, reg;\n      int steps, live;'

OLD_CALL = '      if (rac2_reg_live_after_store_p (insn, reg))\n\tcontinue;\n'

NEW_CALL = '      live = rac2_reg_live_after_store_p (insn, reg);\n      if (live == 1)\n\tcontinue;\n'

OLD_FOLD = '      validate_change (insn, &SET_SRC (set), gen_rtx_REG (TImode, 0), 1);\n      if (! apply_change_group ())\n\tcontinue;\n'

NEW_FOLD = '      validate_change (insn, &SET_SRC (set), gen_rtx_REG (TImode, 0), 1);\n      if (live == 2)\n\tvalidate_change (rac2_fvu_insn, &PATTERN (rac2_fvu_insn),\n\t\t\t gen_rtx (SET, VOIDmode, copy_rtx (rac2_fvu_reg),\n\t\t\t\t  const0_rtx), 1);\n      if (! apply_change_group ())\n\tcontinue;\n'



def patch(text, old, new, what):
    """Replace `old` once, refusing a missing or ambiguous anchor."""
    count = text.count(old)
    assert count == 1, "%s : ancre introuvable ou ambigue (%d)" % (what, count)
    print("%s : patch applique" % what)
    return text.replace(old, new, 1)


def main(argv):
    if len(argv) != 2:
        print("Usage: fold_zero_ti_return_value.py <gcc source directory>")
        return 2
    path = Path(argv[1]) / "config/mips/mips.c"
    assert path.is_file(), "mips.c introuvable : %s" % path
    text = path.read_text(encoding="utf-8")

    if "rac2_fvu_insn" in text:
        print("mips.c : deja applique")
        return 0

    start = text.find(FUNC_HEAD)
    assert start >= 0, "mips.c : rac2_reg_live_after_store_p introuvable"
    end = text.index("\n}\n", start) + 3
    body = text[start:end]
    assert "rac2_fold_zero_ti_store" not in body, "mips.c : guard inattendu"
    assert "return 0;" in body and "reg_referenced_p" in body, "mips.c : guard inattendu"
    print("mips.c : fonction remplacee (%d octets)" % (end - start))
    text = text[:start] + NEW_FUNC + text[end:]

    text = patch(text, OLD_DECL, NEW_DECL, "mips.c declaration")
    text = patch(text, OLD_CALL, NEW_CALL, "mips.c appel")
    text = patch(text, OLD_FOLD, NEW_FOLD, "mips.c repli")

    path.write_text(text, encoding="utf-8", newline="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
