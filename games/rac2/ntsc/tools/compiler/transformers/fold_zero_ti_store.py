"""Fold a zero TImode materialisation into the store that consumes it.

Retires the earlier `allow_zero_ti_store.patch` constraint and replaces it with
a real back-end pass, because the constraint was measured useless and the bare
removal is not enough:

* Usefulness. The patch admitted constraint `J` on the register alternatives of
  `movti_internal` and printed that alternative with `%z1`. Control sources that
  store a zero through a 16-byte pointer emitted `sq $zero` with and without it;
  the patch only ever changed the *memory* alternatives, which already accepted
  `J`. It earned no witness of its own.
* Cost of the bare removal. Without the patch, a 128-bit zero materialises in a
  register (`por $rd,$zero,$zero`) and is then stored by register, which is the
  retail form. But two published bodies that the patch used to fold to
  `sq $zero` regress: `FUN_00282C88` (8 bytes) and `FUN_002E5FE0`. The count of
  exact bodies drops from 5463 to 5461 over the current corpus.
* The repair. `rac2_fold_zero_ti_store` re-folds that materialisation into the
  store when the register is dead after it, so the retail's single
  `sq $0,<off>(<base>)` comes back *and* the families that need the register
  form (a shared `por` feeding several stores) keep it. The pass must run
  **before the second scheduler**: placed after it (the obvious hook,
  `MACHINE_DEPENDENT_REORG_AFTER_DBR_2` territory), `FUN_002E5FE0` comes out
  with `addiu $v1,$v1,-1` before the store instead of after, because sched2
  never saw the folded form. Running before `flag_schedule_insns_after_reload`
  in `toplev.c` makes the scheduler plan the folded form and the body is exact
  again.

The three insertions below are the measured ones; a differently worded comment
inside the function body does not change `cc1`, but the *source* hashes are part
of the qualified identity, so keep this text as it stands.

Usage: fold_zero_ti_store.py <gcc source directory>   (the one holding toplev.c)
"""
from pathlib import Path
import sys

FUNC = r'''
/* RAC2 : replier une materialisation de zero TImode (por $rd,$0,$0) dans le
   store qui la consomme lorsque ce store est sa derniere utilisation avant
   reecriture du registre.  Le retail imprime alors une seule instruction
   `sq $0,<off>(<base>)` (FUN_00282C88 : `j $31 ; sq $0,0($4)`, 8 octets ;
   FUN_002E5FE0 : `sq $zero,0($v0)` dans sa boucle, bien que $a0 y soit
   ensuite reemploye comme pointeur).  Quand le registre alimente plusieurs
   stores, il est encore vivant apres le premier : le partage
   `por $v0,$zero,$zero` + trois `sq $v0` subsiste, comme le retail.  */
static int
rac2_reg_live_after_store_p (insn, reg)
     rtx insn;
     rtx reg;
{
  rtx scan;

  for (scan = NEXT_INSN (insn); scan; scan = NEXT_INSN (scan))
    {
      rtx pat, set;

      if (GET_CODE (scan) != INSN
	  && GET_CODE (scan) != JUMP_INSN
	  && GET_CODE (scan) != CALL_INSN)
	continue;

      pat = PATTERN (scan);
      if (pat == 0 || GET_CODE (pat) == SEQUENCE)
	continue;

      if (reg_referenced_p (reg, pat))
	return 1;			/* consomme avant toute reecriture */

      set = single_set (scan);
      if (set != 0 && GET_CODE (SET_DEST (set)) == REG
	  && REGNO (SET_DEST (set)) == REGNO (reg))
	return 0;			/* registre reecrit : le store etait la
					   derniere utilisation */
    }

  return 0;
}

void
rac2_fold_zero_ti_store (first)
     rtx first;
{
  rtx insn;

  for (insn = first; insn; insn = NEXT_INSN (insn))
    {
      rtx set, prev, pset, reg;
      int steps;

      if (GET_CODE (insn) != INSN)
	continue;

      set = single_set (insn);
      if (set == 0
	  || GET_CODE (SET_DEST (set)) != MEM
	  || GET_MODE (SET_DEST (set)) != TImode
	  || GET_CODE (SET_SRC (set)) != REG)
	continue;

      reg = SET_SRC (set);

      if (rac2_reg_live_after_store_p (insn, reg))
	continue;

      /* Remonter dans le meme bloc vers la materialisation.  */
      pset = 0;
      for (prev = PREV_INSN (insn), steps = 0;
	   prev != 0 && steps < 12;
	   prev = PREV_INSN (prev), steps++)
	{
	  rtx cand;

	  if (GET_CODE (prev) == NOTE)
	    continue;
	  if (GET_CODE (prev) != INSN)
	    break;			/* etiquette, saut, appel, barriere */
	  cand = single_set (prev);
	  if (cand != 0 && rtx_equal_p (SET_DEST (cand), reg))
	    {
	      if (GET_MODE (SET_DEST (cand)) == TImode
		  && ((GET_CODE (SET_SRC (cand)) == CONST_INT
		       && INTVAL (SET_SRC (cand)) == 0)
		      || (GET_CODE (SET_SRC (cand)) == REG
			  && REGNO (SET_SRC (cand)) == 0)))
		pset = prev;
	      break;			/* ecriture du registre : on s'arrete la */
	    }
	  if (cand != 0 && reg_referenced_p (reg, PATTERN (prev)))
	    break;			/* consommation avant le store : prudence */
	}

      if (pset == 0)
	continue;

      validate_change (insn, &SET_SRC (set), gen_rtx_REG (TImode, 0), 1);
      if (! apply_change_group ())
	continue;

      delete_insn (pset);
    }
}

'''

ANCHOR_MIPS_C = "void\nmachine_dependent_reorg (first)\n"

ANCHOR_MIPS_H = "#define MACHINE_DEPENDENT_REORG_AFTER_DBR_2(X) mips_r5900_annul_dead_slots (X)\n"
HOOK_MIPS_H = ("/* RAC2 : replier la materialisation d'un zero TImode avant la seconde\n"
               "   planification (voir rac2_fold_zero_ti_store dans mips.c).  */\n"
               "#define MACHINE_DEPENDENT_REORG_AFTER_RELOAD(X) rac2_fold_zero_ti_store (X)\n"
               "extern void rac2_fold_zero_ti_store ();\n")

NL = chr(10)
TAB = chr(9)
ANCHOR_TOPLEV = ("  /* END CYGNUS LOCAL */" + NL + TAB + "  " + NL
                 + "  if (optimize > 0 && flag_schedule_insns_after_reload)" + NL)
CALL_TOPLEV = ("  /* END CYGNUS LOCAL */" + NL + TAB + "  " + NL
               + "  /* RAC2 : replier la materialisation d'un zero TImode dans son store" + NL
               + "     avant la seconde planification, pour que la planification voie la" + NL
               + "     forme repliee (le retail imprime sq $0 en une instruction).  */" + NL
               + "#ifdef MACHINE_DEPENDENT_REORG_AFTER_RELOAD" + NL
               + "  if (optimize > 0)" + NL
               + "    MACHINE_DEPENDENT_REORG_AFTER_RELOAD (insns);" + NL
               + "#endif" + NL
               + "" + NL
               + "  if (optimize > 0 && flag_schedule_insns_after_reload)" + NL)

MDS_CONSTRAINT = "\t(match_operand:TI 1 \"movti_operand\"         \"d,R,m,dJ,dJ,J,K,L,M,i\"))]"
MDS_CONSTRAINT_REVERTED = "\t(match_operand:TI 1 \"movti_operand\"         \"d,R,m,d,d,J,K,L,M,i\"))]"
MDS_PRINTER = "  sq %z1,%0\n  sq %z1,%0\n"
MDS_PRINTER_REVERTED = "  sq %1,%0\n  sq %1,%0\n"


def patch(path: Path, old: str, new: str, what: str, marker: str) -> None:
    source = path.read_text(encoding="utf-8")
    if marker in source:
        print("%s : deja applique" % what)
        return
    assert source.count(old) == 1, (what, source.count(old))
    path.write_text(source.replace(old, new, 1), encoding="utf-8", newline="")
    print("%s : patch applique" % what)


def main() -> int:
    gcc = Path(sys.argv[1])
    md = gcc / "config/mips/mips.md"
    source = md.read_text(encoding="utf-8")
    if MDS_CONSTRAINT in source:
        assert source.count(MDS_CONSTRAINT) == 1, "Unexpected movti register alternatives"
        assert source.count(MDS_PRINTER) == 1, "Unexpected movti store alternatives"
        assert source.count("sq %z1,%0") == 2, "Unexpected movti zero printing"
        source = source.replace(MDS_CONSTRAINT, MDS_CONSTRAINT_REVERTED, 1)
        source = source.replace(MDS_PRINTER, MDS_PRINTER_REVERTED)
        md.write_text(source, encoding="utf-8", newline="")
        print("mips.md : contrainte J et impression %z1 retirees")
    else:
        assert MDS_CONSTRAINT_REVERTED in source, "Unexpected movti machine description"
        print("mips.md : retrait deja applique")

    patch(gcc / "config/mips/mips.c", ANCHOR_MIPS_C,
          FUNC.lstrip("\n") + ANCHOR_MIPS_C, "mips.c",
          "rac2_fold_zero_ti_store (first)")
    patch(gcc / "config/mips/mips.h", ANCHOR_MIPS_H, ANCHOR_MIPS_H + HOOK_MIPS_H,
          "mips.h", "MACHINE_DEPENDENT_REORG_AFTER_RELOAD")
    patch(gcc / "toplev.c", ANCHOR_TOPLEV, CALL_TOPLEV, "toplev.c",
          "MACHINE_DEPENDENT_REORG_AFTER_RELOAD")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
