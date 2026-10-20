#include "common.h"

/* Near miss, and the clearest case of it in this run: the body is BYTE
   EXACT - `tools/check.py func_00123D30` says `1 of 1 places match` - but the
   function cannot be placed, so it is not a match.

   `tools/build.py boot` refuses it with

     func_00123D30: needs .rodata at its place, which is not C

   and the boot still comes out equal, with the function left as assembly.
   The reason is the `switch (insn)` at +0xF8: cc1 puts its 0x2F-entry jump
   table in the object's own `.rodata`, and the retail table sits at 0x13B070
   inside the loaded image where no C section can be placed. Unblocking it is
   a build-side change (a placement for a C object's `.rodata`, or a
   `tools/levels.py` piece), not a source change - the body below needs
   nothing further. It is the same failure class as `func_001330D0` and the
   other `$gp`/`.sdata` rows of nonmatching/README.md.

   libgcc frame.o: execute_cfa_insn, verbatim from the gcc/frame.c of
   gnu-ee-binutils-gcc-1.1 - the same public source the qualified compiler
   is built from, and the same file count_fdes (src/core/00123930.c) and
   fde_merge (src/core/001235C8.c) come from. Decodes one DWARF 2 call
   frame instruction; read_1byte/read_2byte/read_4byte and read_pointer are
   the file's own static inlines and `decode_uleb128` is func_00123530, both
   in src/core/001232F0.c, where `struct cie_info` is declared too.

   `frame_state` is frame.h's with FIRST_PSEUDO_REGISTER 79 and the 64-bit
   `long` of this ABI: 0x2F0 bytes, the block move `memcpy (save, state,
   sizeof (struct frame_state_internal))` expands at +0x22C. Profile:
   sdk -O2 -G2 -ffunction-sections. */

/* From src/core/001232F0.c, above the marker: */

struct cie_info {
    char *augmentation;
    void *eh_ptr;
    int code_align;
    int data_align;
    unsigned int ra_regno;
};

union unaligned {
    void *p;
    unsigned int b2 __attribute__ ((mode (HI)));
    unsigned int b4 __attribute__ ((mode (SI)));
} __attribute__ ((packed));

static __inline__ void *read_pointer(void *p) {
    union unaligned *up = p;
    return up->p;
}

static __inline__ unsigned int read_1byte(void *p) {
    return *(unsigned char *)p;
}

static __inline__ unsigned int read_2byte(void *p) {
    union unaligned *up = p;
    return up->b2;
}

static __inline__ unsigned int read_4byte(void *p) {
    union unaligned *up = p;
    return up->b4;
}

/* libgcc frame.o: execute_cfa_insn, verbatim from the gcc/frame.c of
   gnu-ee-binutils-gcc-1.1 - the same public source the qualified compiler
   is built from, and the same file count_fdes (src/core/00123930.c) and
   fde_merge (src/core/001235C8.c) come from. Decodes one DWARF 2 call
   frame instruction; read_1byte/read_2byte/read_4byte and read_pointer
   above are the file's own static inlines and `decode_uleb128` is
   func_00123530 in this file.

   `frame_state` is frame.h's with FIRST_PSEUDO_REGISTER 79 and the 64-bit
   `long` of this ABI: 0x2F0 bytes, the block move `memcpy (save, state,
   sizeof (struct frame_state_internal))` expands at +0x22C. The switch over
   `insn` is the jump table retail keeps at 0x13B070. */

#define FIRST_PSEUDO_REGISTER 79

typedef struct frame_state
{
  void *cfa;
  void *eh_ptr;
  long cfa_offset;
  long args_size;
  long reg_or_offset[FIRST_PSEUDO_REGISTER+1];
  unsigned short cfa_reg;
  unsigned short retaddr_column;
  char saved[FIRST_PSEUDO_REGISTER+1];
} frame_state;

#define REG_UNSAVED 0
#define REG_SAVED_OFFSET 1
#define REG_SAVED_REG 2

/* The current unwind state, plus a saved copy for DW_CFA_remember_state.  */

struct frame_state_internal
{
  struct frame_state s;
  struct frame_state_internal *saved_state;
};

#define DW_CFA_advance_loc	0x40
#define DW_CFA_offset		0x80
#define DW_CFA_restore		0xc0
#define DW_CFA_nop		0x00
#define DW_CFA_set_loc		0x01
#define DW_CFA_advance_loc1	0x02
#define DW_CFA_advance_loc2	0x03
#define DW_CFA_advance_loc4	0x04
#define DW_CFA_offset_extended	0x05
#define DW_CFA_restore_extended	0x06
#define DW_CFA_undefined	0x07
#define DW_CFA_same_value	0x08
#define DW_CFA_register		0x09
#define DW_CFA_remember_state	0x0a
#define DW_CFA_restore_state	0x0b
#define DW_CFA_def_cfa		0x0c
#define DW_CFA_def_cfa_register	0x0d
#define DW_CFA_def_cfa_offset	0x0e
#define DW_CFA_GNU_window_save	0x2d
#define DW_CFA_GNU_args_size	0x2e

extern void *func_00115F28(unsigned int);
extern void func_00115F78(void *);
extern void func_00115E28(void);

static void *
func_00123D30 (void *p, struct frame_state_internal *state,
	       struct cie_info *info, void **pc)
{
  unsigned insn = *(unsigned char *)p++;
  unsigned reg;
  int offset;

  if (insn & DW_CFA_advance_loc)
    *pc += ((insn & 0x3f) * info->code_align);
  else if (insn & DW_CFA_offset)
    {
      reg = (insn & 0x3f);
      p = func_00123530 (p, (unsigned int *)&offset);
      offset *= info->data_align;
      state->s.saved[reg] = REG_SAVED_OFFSET;
      state->s.reg_or_offset[reg] = offset;
    }
  else if (insn & DW_CFA_restore)
    {
      reg = (insn & 0x3f);
      state->s.saved[reg] = REG_UNSAVED;
    }
  else switch (insn)
    {
    case DW_CFA_set_loc:
      *pc = read_pointer (p);
      p += sizeof (void *);
      break;
    case DW_CFA_advance_loc1:
      *pc += read_1byte (p);
      p += 1;
      break;
    case DW_CFA_advance_loc2:
      *pc += read_2byte (p);
      p += 2;
      break;
    case DW_CFA_advance_loc4:
      *pc += read_4byte (p);
      p += 4;
      break;

    case DW_CFA_offset_extended:
      p = func_00123530 (p, &reg);
      p = func_00123530 (p, (unsigned int *)&offset);
      offset *= info->data_align;
      state->s.saved[reg] = REG_SAVED_OFFSET;
      state->s.reg_or_offset[reg] = offset;
      break;
    case DW_CFA_restore_extended:
      p = func_00123530 (p, &reg);
      state->s.saved[reg] = REG_UNSAVED;
      break;

    case DW_CFA_undefined:
    case DW_CFA_same_value:
    case DW_CFA_nop:
      break;

    case DW_CFA_register:
      {
	unsigned reg2;
	p = func_00123530 (p, &reg);
	p = func_00123530 (p, &reg2);
	state->s.saved[reg] = REG_SAVED_REG;
	state->s.reg_or_offset[reg] = reg2;
      }
      break;

    case DW_CFA_def_cfa:
      p = func_00123530 (p, &reg);
      p = func_00123530 (p, (unsigned int *)&offset);
      state->s.cfa_reg = reg;
      state->s.cfa_offset = offset;
      break;
    case DW_CFA_def_cfa_register:
      p = func_00123530 (p, &reg);
      state->s.cfa_reg = reg;
      break;
    case DW_CFA_def_cfa_offset:
      p = func_00123530 (p, (unsigned int *)&offset);
      state->s.cfa_offset = offset;
      break;

    case DW_CFA_remember_state:
      {
	struct frame_state_internal *save =
	  (struct frame_state_internal *)
	  func_00115F28 (sizeof (struct frame_state_internal));
	memcpy (save, state, sizeof (struct frame_state_internal));
	state->saved_state = save;
      }
      break;
    case DW_CFA_restore_state:
      {
	struct frame_state_internal *save = state->saved_state;
	memcpy (state, save, sizeof (struct frame_state_internal));
	func_00115F78 (save);
      }
      break;

      /* FIXME: Hardcoded for SPARC register window configuration.  */
    case DW_CFA_GNU_window_save:
      for (reg = 16; reg < 32; ++reg)
	{
	  state->s.saved[reg] = REG_SAVED_OFFSET;
	  state->s.reg_or_offset[reg] = (reg - 16) * sizeof (void *);
	}
      break;

    case DW_CFA_GNU_args_size:
      p = func_00123530 (p, (unsigned int *)&offset);
      state->s.args_size = offset;
      break;

    default:
      func_00115E28 ();
    }
  return p;
}
