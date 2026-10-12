"""Extend the measured SDK 3.01 division padding to its branch trigger.

`pad_div_erratum_nops.py` reproduces the padding the authentic SDK 3.01
assembler inserts when a COP1 single-precision division opcode is emitted
fewer than two slots after the most recent **code label**. A retail census of
the first trigger showed the wall as "two nop before div.s"; the label alone
does not explain the sites where the two words sit *after a branch*.

Measured against the same oracle (authentic SDK 3.01 `Ps2EeAs`, SHA-256
`cb5adda955e64626564212ef7e0c1434708c4e1ef423344a92ec8033306ed3aa`):

  (D) Branch group.  A division opcode within two emitted slots after the end
      of a **branch group** — a delay-carrying branch plus its delay slot — is
      padded to land exactly two slots after that point. The delay slot is
      part of the group: `bc1t ; ld ; div.s` gains two nops,
      `bc1t ; ld ; addu ; div.s` gains one, and `bc1t ; ld ; addu ; addu ;
      div.s` gains none. Conditional branches (`beq`, `bne`, `bgez`, `bc1t`,
      ...), branch-likely forms (`beql`, `bc1tl`) and the unconditional `b`
      all trigger it.

  (E) Jumps do not trigger it. `j`, `jal`, `jr` and `jalr` own a delay slot
      but start no padding distance: `jal ; nop ; div.s` gains nothing, and
      `jal ; nop ; addu ; div.s` gains nothing either. The discriminator in
      the generated code is the opcode class (J-type 2/3 and SPECIAL funct
      8/9), not a game address or a mnemonic table.

  (F) The trigger span matches the label form. `sqrt.s` and `rsqrt.s` carry
      the same floor after a branch; integer HI/LO `div`/`divu` do not; and an
      already emitted nop inside the two-slot window satisfies one of the two
      slots instead of adding to them (the floor is a maximum, not a sum).

The branch floor is deliberately **not** cleared by an immediately preceding
`.set noreorder` region, unlike the reused hazard-nop line that clears
`nops`. The measured target site puts the branch and its delay slot inside a
compiler-generated `.set noreorder` block and the division in the following
`.set reorder` block, and the oracle still pads it. A division *inside* a
noreorder region remains unpadded, which is the same bounded gap
`pad_div_erratum_nops.py` documents.

This transformer is applied **after** `pad_div_erratum_nops.py`: it rewrites
that rule's own generated anchors, so the label trigger and its validated
text are left exactly as they are. Running it twice is a no-op; a missing
anchor stops it without touching the file.

Usage: pad_div_erratum_branch.py <gas source directory or tc-mips.c>
"""
from pathlib import Path
import sys

STATE_ANCHOR = "static segT rac2_div_code_label_segment;"

HELPER = '''
/* RAC2: the same incoming floor starts at the end of a branch group.
   Measured against the SDK 3.01 oracle: conditional, branch-likely and
   unconditional branches reset the padding distance once their delay slot
   has been emitted; jumps (j, jal, jr, jalr) do not.  This is the same
   minimum as the label floor, never an addition to it.
    */
static int rac2_div_insns_since_branch;
static int rac2_div_branch_defined;
static int rac2_div_branch_delay_pending;
static segT rac2_div_branch_segment;
static fragS *rac2_div_branch_end_frag;
static valueT rac2_div_branch_end_offset;

/* RAC2: a delay-carrying branch owns the padding origin; a jump does not.
   The J-type opcodes 2/3 and SPECIAL funct 8/9 are the measured
   non-triggering class.
    */
static int
rac2_div_branch_p (ip)
     struct mips_cl_insn *ip;
{
  unsigned long pinfo;
  unsigned int opcode;

  if (ip == 0 || ip->insn_mo == 0)
    return 0;
  pinfo = ip->insn_mo->pinfo;
  if ((pinfo & (INSN_UNCOND_BRANCH_DELAY
		| INSN_COND_BRANCH_DELAY
		| INSN_COND_BRANCH_LIKELY)) == 0)
    return 0;
  opcode = ip->insn_opcode;
  if ((opcode >> 26) == 0x02 || (opcode >> 26) == 0x03)
    return 0;                                   /* j, jal */
  if ((opcode >> 26) == 0x00
      && ((opcode & 0x3F) == 0x08 || (opcode & 0x3F) == 0x09))
    return 0;                                   /* jr, jalr */
  return 1;
}

/* RAC2: count emitted four-byte slots between the end of the latest branch
   group and the current position, when both remain in the current fragment.
   The fallback counter covers a fragment boundary.  */
static int
rac2_div_branch_distance ()
{
  if (rac2_div_branch_end_frag != 0 && rac2_div_branch_end_frag == frag_now)
    {
      valueT here = (valueT) frag_now_fix ();
      if (here >= rac2_div_branch_end_offset)
	{
	  valueT bytes = here - rac2_div_branch_end_offset;
	  if ((bytes & 3) == 0)
	    return bytes >= 8 ? 2 : (int) (bytes / 4);
	}
    }
  return rac2_div_insns_since_branch;
}

/* RAC2: the delay slot of a branch just completed; the floor starts here.  */
static void
rac2_div_branch_end ()
{
  rac2_div_branch_defined = 1;
  rac2_div_branch_segment = now_seg;
  rac2_div_branch_end_frag = frag_now;
  rac2_div_branch_end_offset = (valueT) frag_now_fix ();
  rac2_div_insns_since_branch = 0;
}

'''

PAD_ANCHOR = """      if (prev_insn_unreordered)
	nops = 0;
"""

PAD = PAD_ANCHOR + """
      /* RAC2: the floor also starts at the end of a branch group.  This test
   runs after the noreorder reset on purpose: the measured site keeps its
   branch and delay slot inside a compiler .set noreorder block and its
   division in the following .set reorder block.  A division emitted inside
   a noreorder region still receives no padding.
    */
      if (rac2_div_branch_defined
	  && ! mips_opts.mips16
	  && rac2_div_erratum_p (ip)
	  && rac2_div_branch_segment == now_seg
	  && rac2_div_branch_distance () < 2)
	{
	  int rac2_need = 2 - rac2_div_branch_distance ();
	  if (nops < rac2_need)
	    nops = rac2_need;

	  if (insn_labels != NULL && ! mips_opts.noreorder
	      && ! prev_insn_unreordered)
	    rac2_div_keep_labels = 1;
	}
"""

COUNT_ANCHOR = """  /* RAC2: count this instruction; saturate at two slots. */
  if (rac2_div_insns_since_label < 2)
    ++rac2_div_insns_since_label;
"""

COUNT = COUNT_ANCHOR + """
  /* RAC2: count this instruction for the branch-group floor, then close the
   group when the instruction just emitted was the delay slot of a branch.
   In reorder mode the delay slot is filled by the very call that appends
   the branch; in noreorder mode it is the next appended instruction.
    */
  if (rac2_div_insns_since_branch < 2)
    ++rac2_div_insns_since_branch;

  if (! mips_opts.mips16)
    {
      if (rac2_div_branch_delay_pending)
	{
	  rac2_div_branch_delay_pending = 0;
	  rac2_div_branch_end ();
	}
      else if (place == NULL && rac2_div_branch_p (ip))
	{
	  if (mips_opts.noreorder)
	    rac2_div_branch_delay_pending = 1;
	  else
	    rac2_div_branch_end ();
	}
    }
"""

NOP_EMIT_ANCHOR = """   (rac2_div_insns_since_label < 2\\
    ? ++rac2_div_insns_since_label : rac2_div_insns_since_label))"""
NOP_EMIT = """   (rac2_div_insns_since_label < 2\\
    ? ++rac2_div_insns_since_label : rac2_div_insns_since_label),\\
   (rac2_div_insns_since_branch < 2\\
    ? ++rac2_div_insns_since_branch : rac2_div_insns_since_branch))"""

FORGET_ANCHOR = """  if (! preserve)
    {
      prev_insn.insn_mo = &dummy_opcode;
      prev_prev_insn.insn_mo = &dummy_opcode;
"""
FORGET = """  if (! preserve)
    {
      /* RAC2: a forgotten instruction history also forgets a pending delay
	 slot, so the branch floor cannot carry across a section change.  */
      rac2_div_branch_delay_pending = 0;
      prev_insn.insn_mo = &dummy_opcode;
      prev_prev_insn.insn_mo = &dummy_opcode;
"""


def main() -> int:
    p = Path(sys.argv[1])
    if p.is_dir():
        p = p / "gas" / "config" / "tc-mips.c"
    source = p.read_text(encoding="utf-8")

    if "rac2_div_insns_since_branch" in source:
        print("branch trigger already present in %s" % p)
        return 0

    assert source.count(STATE_ANCHOR) == 1, "anchor: rac2 division state block"
    assert source.count(PAD_ANCHOR) == 1, "anchor: noreorder nop reset"
    assert source.count(COUNT_ANCHOR) == 1, "anchor: rac2 division slot counter"
    assert source.count(NOP_EMIT_ANCHOR) == 1, "anchor: emitted-nop macro"
    assert source.count(FORGET_ANCHOR) == 1, "anchor: mips_no_prev_insn body"

    source = source.replace(STATE_ANCHOR, STATE_ANCHOR + HELPER, 1)
    source = source.replace(PAD_ANCHOR, PAD, 1)
    source = source.replace(COUNT_ANCHOR, COUNT, 1)
    source = source.replace(NOP_EMIT_ANCHOR, NOP_EMIT, 1)
    source = source.replace(FORGET_ANCHOR, FORGET, 1)

    p.write_text(source, encoding="utf-8", newline="")
    print("Branch-group division floor installed in %s" % p)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
