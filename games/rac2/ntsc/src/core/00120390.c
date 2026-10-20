#include "include_asm.h"

extern int func_00115544(const char *, const char *);

void *func_00120390(const char *first, const char *second, void *value) {
    return func_00115544(first, second) ? 0 : value;
}

void func_001203C0(void) {
}

extern void *func_00115F28(unsigned int);
extern int func_00115484(unsigned char *, int, int);
extern void func_00120368(void);

char *func_001203C8(void) {
    char *record = func_00115F28(24);
    if (record == 0) func_00120368();
    func_00115484((unsigned char *)record, 0, 24);
    *(char **)(record + 4) = record + 16;
    return record;
}

extern char *(*D_00135D38)(void);

char *func_00120420(void) {
    return D_00135D38();
}

char *func_00120448(void) {
    return D_00135D38() + 8;
}

extern char *func_00120498(void);

char *func_00120470(void) {
    D_00135D38 = func_00120498;
    return D_00135D38();
}

extern int D_00141800;
extern char D_001417F0[];
extern char D_00141808[];

char *func_00120498(void) {
    if (D_00141800 == 0) {
        D_00141800 = 1;
        func_00115484((unsigned char *)D_001417F0, 0, 16);
        *(char **)(D_001417F0 + 4) = D_00141808;
    }
    return D_001417F0;
}

char *func_00120500(void) {
    return D_00135D38() + 4;
}

INCLUDE_ASM("asm/core", func_00120528);

int func_00120800(short *record) {
    return record[3];
}

int func_00120808(short *record) {
    return record[2];
}

INCLUDE_ASM("asm/core", func_001208E8);

INCLUDE_ASM("asm/core", func_00120A30);

extern void *func_00120A30(int, void *, int);
extern void *func_001153D4(void *, const void *, unsigned int);

void *func_00120AB8(int index, void *source, void *destination) {
    void *from = func_00120A30(index, source, 0);
    void *to = func_00120A30(index, destination, 0);
    unsigned int size = index < 31 ? 16 : index <= 63 ? 4 : 8;
    return func_001153D4(to, from, size);
}

void func_00120BC8(void)
{
}

/* libgcc _eh.o: throw_helper, the common grunt work of a throw, from the
   L_eh section of GCC's libgcc2.c (GPL version 2-or-later with the libgcc
   linking exception; the source is the public archive the qualified
   compiler is built from, gnu-ee-binutils-gcc-1.1, gcc/libgcc2.c). Its
   neighbours in the same object are in this file: func_001203C0 is
   __empty, func_00120368 is __terminate, func_00120BC8 is
   __unwinding_cleanup, and next_stack_level, get_reg_addr, copy_reg,
   find_exception_handler and old_find_exception_handler live in
   src/core/00120B38.c, src/core/00120A30.c (with func_00120AB8 here),
   src/core/001208E8.c and src/core/00120810.c.

   `frame_state` is frame.h's with FIRST_PSEUDO_REGISTER 79 (the archive's
   own config/mips/mips.h) and the 64-bit `long` of this ABI: 0x2F0 bytes,
   the block move both `saved_ustruct = *udata` and `memcpy (udata,
   my_udata, sizeof (*udata))` expand at +0x104 and +0x204.
   in_reg_window is the file's `return 0` variant (no INCOMING_REGNO on
   this target), so its two guards fold away and leave no code. */

typedef signed long long s64;

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

struct eh_context
{
  void *handler_label;
  void **dynamic_handler_chain;
  void *info;
  void *table_index;
};

typedef struct exception_descriptor
{
  void *runtime_id_field;
} exception_descriptor;

#define NEW_EH_RUNTIME ((void *) -2)

#define REG_SAVED_REG 2

extern void func_00120368(void);
extern void func_00120BC8(void);
extern void *func_001208E8(void *, void *, void *, int, int *);
extern void *func_00120810(void *, void *);
extern frame_state *func_00120B38(void *, frame_state *, frame_state *);

void *func_00120BD0(struct eh_context *eh, void *pc, frame_state *my_udata,
		    frame_state **udata_p)
{
  frame_state *udata = *udata_p;
  frame_state ustruct;
  frame_state *sub_udata = &ustruct;
  void *saved_pc = pc;
  void *handler;
  void *handler_p;
  void *pc_p;
  frame_state saved_ustruct;
  int new_eh_model;
  int cleanup = 0;
  int only_cleanup = 0;
  int rethrow = 0;
  int saved_state = 0;
  void *eh_info = eh->info;

  /* Do we find a handler based on a re-throw PC? */
  if (eh->table_index != (void *) 0)
    rethrow = 1;

  handler = (void *) 0;
  for (;;)
    {
      frame_state *p = udata;
      udata = func_00120B38 (pc, udata, sub_udata);
      sub_udata = p;

      /* If we couldn't find the next frame, we lose.  */
      if (! udata)
	break;

      if (udata->eh_ptr == 0)
        new_eh_model = 0;
      else
        new_eh_model = (((exception_descriptor *)(udata->eh_ptr))->
                                          runtime_id_field == NEW_EH_RUNTIME);

      if (rethrow)
        {
          rethrow = 0;
          handler = func_001208E8 (eh->table_index, udata->eh_ptr,
                                   eh_info, 1, &cleanup);
          eh->table_index = (void *)0;
        }
      else
        if (new_eh_model)
          handler = func_001208E8 (pc, udata->eh_ptr, eh_info,
                                   0, &cleanup);
        else
          handler = func_00120810 (pc, udata->eh_ptr);

      /* If we found one, we can stop searching, if its not a cleanup.
         for cleanups, we save the state, and keep looking. This allows
         us to call a debug hook if there are nothing but cleanups left. */
      if (handler)
	if (cleanup)
	  {
	    if (!saved_state)
	      {
		saved_ustruct = *udata;
		handler_p = handler;
		pc_p = pc;
		saved_state = 1;
		only_cleanup = 1;
	      }
	  }
	else
	  {
	    only_cleanup = 0;
	    break;
	  }

      /* Otherwise, we continue searching.  We subtract 1 from PC to avoid
	 hitting the beginning of the next region.  */
      {
        s64 value = *(s64 *)func_00120A30 (udata->retaddr_column, udata,
					    (int)sub_udata);
        pc = (void *)((int)value - 1);
      }
    }

  if (saved_state)
    {
      udata = &saved_ustruct;
      handler = handler_p;
      pc = pc_p;
      if (only_cleanup)
        func_00120BC8 ();
    }

  /* If we haven't found a handler by now, this is an unhandled
     exception.  */
  if (! handler)
    func_00120368 ();

  eh->handler_label = handler;

  if (pc == saved_pc)
    /* We found a handler in the throw context, no need to unwind.  */
    udata = my_udata;
  else
    {
      int i;

      /* Unwind all the frames between this one and the handler by copying
	 their saved register values into our register save slots.  */

      /* Remember the PC where we found the handler.  */
      void *handler_pc = pc;

      /* Start from the throw context again.  */
      pc = saved_pc;
      memcpy (udata, my_udata, sizeof (*udata));

      while (pc != handler_pc)
	{
	  frame_state *p = udata;
	  udata = func_00120B38 (pc, udata, sub_udata);
	  sub_udata = p;

	  for (i = 0; i < FIRST_PSEUDO_REGISTER; ++i)
	    if (i != udata->retaddr_column && udata->saved[i])
	      func_00120AB8 (i, udata, my_udata);

	  {
	    s64 value = *(s64 *)func_00120A30 (udata->retaddr_column, udata,
						(int)sub_udata);
	    pc = (void *)((int)value - 1);
	  }
	}

      /* But we do need to update the saved return address register from
	 the last frame we unwind, or the handler frame will have the wrong
	 return address.  */
      if (udata->saved[udata->retaddr_column] == REG_SAVED_REG)
	{
	  i = udata->reg_or_offset[udata->retaddr_column];
	}
    }
  /* udata now refers to the frame called by the handler frame.  */

  *udata_p = udata;
  return handler;
}

INCLUDE_ASM("asm/core", func_00120F00);

INCLUDE_ASM("asm/core", func_001210E0);

INCLUDE_ASM("asm/core", func_001212C4);
