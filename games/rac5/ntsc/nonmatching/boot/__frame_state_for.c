/* __frame_state_for -- gcc 2.9 frame.c (DWARF 2 unwinder for C++ EH).
   EE ABI: 32-bit pointers, 64-bit long.  Helpers are the static routines of
   the same translation unit, kept external here and pinned in the sidecar.  */

#define FIRST_PSEUDO_REGISTER 79

struct frame_state
{
  void *cfa;
  void *eh_ptr;
  long cfa_offset;
  long args_size;
  long reg_or_offset[FIRST_PSEUDO_REGISTER + 1];
  unsigned short cfa_reg;
  unsigned short retaddr_column;
  char saved[FIRST_PSEUDO_REGISTER + 1];
};

struct frame_state_internal
{
  struct frame_state s;
  struct frame_state_internal *saved_state;
};

typedef struct dwarf_fde fde;

struct dwarf_cie
{
  unsigned int length;
  int CIE_id;
  unsigned char version;
  char augmentation[0];
} __attribute__ ((packed, aligned (__alignof__ (void *))));

struct dwarf_fde
{
  unsigned int length;
  int CIE_delta;
  void *pc_begin;
  unsigned int pc_range;
} __attribute__ ((packed, aligned (__alignof__ (void *))));

struct cie_info
{
  char *augmentation;
  void *eh_ptr;
  int code_align;
  int data_align;
  unsigned ra_regno;
};

extern fde *find_fde (void *pc_target);
extern void *extract_cie_info (fde *f, struct cie_info *c);
extern void *execute_cfa_insn (void *p, struct frame_state_internal *state,
			       struct cie_info *info, void **pc);
extern void *decode_uleb128 (unsigned char *buf, unsigned *r);
extern void *memset (void *, int, unsigned int);
extern void *memcpy (void *, const void *, unsigned int);

static inline struct dwarf_cie *
get_cie (fde *f)
{
  return ((void *)&f->CIE_delta) - f->CIE_delta;
}

static inline fde *
next_fde (fde *p)
{
  return (fde *)(((char *)p) + p->length + sizeof (p->length));
}

struct frame_state *
__frame_state_for (void *pc_target, struct frame_state *state_in)
{
  fde *f;
  void *insn, *end, *pc;
  struct cie_info info;
  struct frame_state_internal state;

  f = find_fde (pc_target);
  if (f == 0)
    return 0;

  insn = extract_cie_info (f, &info);
  if (insn == 0)
    return 0;

  memset (&state, 0, sizeof (state));
  state.s.retaddr_column = info.ra_regno;
  state.s.eh_ptr = info.eh_ptr;

  /* First decode all the insns in the CIE.  */
  end = next_fde ((fde *) get_cie (f));
  while (insn < end)
    insn = execute_cfa_insn (insn, &state, &info, 0);

  insn = ((fde *)f) + 1;

  if (info.augmentation[0] == 'z')
    {
      int i;
      insn = decode_uleb128 (insn, &i);
      insn += i;
    }

  /* Then the insns in the FDE up to our target PC.  */
  end = next_fde (f);
  pc = f->pc_begin;
  while (insn < end && pc <= pc_target)
    insn = execute_cfa_insn (insn, &state, &info, &pc);

  memcpy (state_in, &state.s, sizeof (state.s));
  return state_in;
}
