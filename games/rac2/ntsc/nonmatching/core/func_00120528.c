/* libgcc _eh.o: __sjthrow, __sjpopnthrow and __eh_rtime_match, transcribed
   from the L_eh section of GCC's libgcc2.c (GPL version 2-or-later with the
   libgcc linking exception; the source is the public archive the qualified
   compiler is built from, gnu-ee-binutils-gcc-1.1, gcc/libgcc2.c).

   The boot catalogue carries one 728-byte extent for this object, but the
   retail bytes hold three functions: __sjthrow to 0x001206A0,
   __sjpopnthrow to 0x001207B8, __eh_rtime_match to the end. The extent is
   what the linker sees, so the two later bodies are pinned into the first
   one's section with a section attribute - the same layout the linker gives
   the three separate sections of a normal object, and the only source-level
   way to hand tools/build.py a single 728-byte symbol. */

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

typedef void *(*__eh_matcher)(void *, void *, void *);

typedef struct
{
  __eh_matcher match_function;
  short language;
  short version;
} __eh_info;

#define NEW_EH_RUNTIME ((void *) -2)

#ifndef NULL
#define NULL ((void *) 0)
#endif

/* Support routines for setjmp/longjmp exception handling.  */

extern void func_00120528(void) __attribute__ ((__noreturn__));

void func_00120528(void)
{
  struct eh_context *eh = (struct eh_context *) D_00135D38 ();
  void ***dhc = &eh->dynamic_handler_chain;
  void *jmpbuf;
  void (*func)(void *, int);
  void *arg;
  void ***cleanup;

  /* The cleanup chain is one word into the buffer.  Get the cleanup
     chain.  */
  cleanup = (void***)&(*dhc)[1];

  /* If there are any cleanups in the chain, run them now.  */
  if (cleanup[0])
    {
      double store[200];
      void **buf = (void**)store;
      buf[1] = 0;
      buf[0] = (*dhc);

      /* try { */
      if (! __builtin_setjmp (&buf[2]))
	{
	  *dhc = buf;
	  while (cleanup[0])
	    {
	      func = (void(*)(void*, int))cleanup[0][1];
	      arg = (void*)cleanup[0][2];

	      /* Update this before running the cleanup.  */
	      cleanup[0] = (void **)cleanup[0][0];

	      (*func)(arg, 2);
	    }
	  *dhc = buf[0];
	}
      /* catch (...) */
      else
	{
	  func_00120368 ();
	}
    }

  /* We must call terminate if we try and rethrow an exception, when
     there is no exception currently active and when there are no
     handlers left.  */
  if (! eh->info || (*dhc)[0] == 0)
    func_00120368 ();

  /* Find the jmpbuf associated with the top element of the dynamic
     handler chain.  The jumpbuf starts two words into the buffer.  */
  jmpbuf = &(*dhc)[2];

  /* Then we pop the top element off the dynamic handler chain.  */
  *dhc = (void**)(*dhc)[0];

  /* And then we jump to the handler.  */
  __builtin_longjmp (jmpbuf, 1);
}

/* Run cleanups on the dynamic cleanup stack for the current dynamic
   handler, then pop the handler off the dynamic handler stack, and
   then throw.  This is used to skip the first handler, and transfer
   control to the next handler in the dynamic handler stack.  */

void func_001206A0(void)
     __attribute__ ((section (".text.func_00120528")));

void func_001206A0(void)
{
  struct eh_context *eh = (struct eh_context *) D_00135D38 ();
  void ***dhc = &eh->dynamic_handler_chain;
  void (*func)(void *, int);
  void *arg;
  void ***cleanup;

  cleanup = (void***)&(*dhc)[1];

  if (cleanup[0])
    {
      double store[200];
      void **buf = (void**)store;
      buf[1] = 0;
      buf[0] = (*dhc);

      if (! __builtin_setjmp (&buf[2]))
	{
	  *dhc = buf;
	  while (cleanup[0])
	    {
	      func = (void(*)(void*, int))cleanup[0][1];
	      arg = (void*)cleanup[0][2];

	      cleanup[0] = (void **)cleanup[0][0];

	      (*func)(arg, 2);
	    }
	  *dhc = buf[0];
	}
      else
	{
	  func_00120368 ();
	}
    }

  /* Then we pop the top element off the dynamic handler chain.  */
  *dhc = (void**)(*dhc)[0];

  func_00120528 ();
}

/* Support code for all exception region-based exception handling.  */

int func_001207B8(void *rtime)
     __attribute__ ((section (".text.func_00120528")));

int func_001207B8(void *rtime)
{
  void *info;
  __eh_matcher matcher;
  void *ret;

  info = *(void **)func_00120448 ();
  matcher = ((__eh_info *)info)->match_function;
  if (! matcher)
    {
      return 0;
    }
  ret = (*matcher) (info, rtime, (void *)0);
  return (ret != NULL);
}

/* Near miss: 720 bytes against retail's 728, and the body is not exactly
   retail's. Three independent differences, all of them the allocator and the
   block placer rather than the source:

   1. The `cleanup` local is built in $a1 by retail and in $a0 here (both are
      spilled and reloaded, so only the register byte differs at +0x20 and
      +0xC0).
   2. __builtin_setjmp's "returns 1" block is inline in retail (a `b` over an
      `addiu $v0,$zero,1` at +0xB0) and sunk to the end of the function here.
   3. __builtin_longjmp is an indirect jump through the buffer in retail
      (`lw $v0,0x4($v1) ; jr $v0`) and a direct branch here, which is two
      instructions shorter - the 8 bytes of the size difference.

   What is right and reusable: the three bodies, the 0x750 frame, the whole
   __sjpopnthrow needing the noreturn declaration of __sjthrow to drop its
   epilogue, and __eh_rtime_match. The boot catalogue carries one 728-byte
   extent for this object (`config/boot/functions.tsv`, 00120528 728) while
   the retail bytes hold three functions at 0x00120528, 0x001206A0 and
   0x001207B8; the section attributes below are what makes one 728-byte
   symbol out of them, and they stay the right shape once the extent is
   split. Verified with tools/check.py func_00120528: `720 bytes, expected
   728`. */
