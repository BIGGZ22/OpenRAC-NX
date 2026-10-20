#include "common.h"

/* NEAR MISS - candidate 756 bytes, retail's exact size (wave 4, 15 October 2026).

   newlib mallocr.c's `fREe` (DEFINE_FREE), transcribed from
   build/sdk-compiler-build/source/ee/newlib/libc/stdlib/mallocr.c and compiled
   with -O2 -G2 -ffunction-sections -mastra-quad-saves (src/core/00116460.c's and
   src/core/001166C8.c's profile: the prologue `addiu $sp,$sp,-0x30` with
   `sq $ra,0x20; sq $s1,0x10; sq $s0,0x0` is exact). trim_threshold 0x1345F0
   and top_pad 0x1345F8 are unsigned long, the other statics are the 0x1341E8
   bin array; no mmap arm and no check_inuse_chunk (HAVE_MMAP 0, DEBUG off).

   Wave 3 recorded 764 bytes against retail's 756 with two extra instructions:
   the islr branch's delay slot held `sz += nextsz` while the unlink path
   reloaded both `next->fd` and `next->bk`, and a load-to-branch `nop` sat in
   front of the frontlink large-bin loop's rotated `beql`.

   Wave 4 measured where the size comes from. The `unlink(next, bck, fwd)` macro
   makes cc1 emit a SECOND load of `next->fd` in the unlink block: this cc1 does
   not carry a load across the branch unless the value stays live past it (spelt
   as an explicit `fwd = next->fd` in the else arm alone, the candidate goes back
   to 764 bytes - measured). Naming the load before the test and using it as the
   unlink's FD keeps it live, and then:

   - the size becomes 756, retail's exact figure, and the islr region takes
     retail's layout: `sz += nextsz` inline, the `next->fd` load in the islr
     branch's delay slot, and the unlink path reaching the join with only
     `next->bk` left to load;
   - two words still differ. This cc1 keeps the loaded FD in $a2 and uses it
     directly, where retail loads into $v1 and copies at the join
     (`daddu $a2,$v1,$zero`, which is the macro's own `FD = P->fd` assignment:
     retail's cc1 had CSE'd the load, so the assignment became a copy. The
     earlier `unlink(p, bck, fwd)` above is what puts `fwd` in $a2 at both
     sites). Retail's copy is the one instruction this candidate lacks.
   - the load-to-branch `nop` in front of the frontlink large-bin loop is
     untouched by the arm above (`lw $a2,0x8($a2)` then `beql $a2,$t0` in
     retail; this cc1 inserts a `nop` between them, and moving `sz += nextsz`
     relative to the load does not change it).

   So the residual stays in wave 3's class - this cc1's delay-slot and
   cross-branch CSE choices, not a source shape - but the body is now
   size-exact and the islr region is understood to one instruction. The
   candidate below is the 756-byte one; its one deviation from the upstream
   transcription is the consolidate-forward arm (the load hoisted into `fwd`,
   the `unlink` macro written out for that arm). */

/*
 * A version of malloc/free/realloc written by Doug Lea and released to the
 * public domain (dlmalloc 2.6.5), adapted by newlib. See
 * THIRD_PARTY_NOTICES.md.
 */

/* Source: newlib mallocr.c, fREe under DEFINE_FREE
 * (build/sdk-compiler-build/source/ee/newlib/libc/stdlib/mallocr.c) - the unit
 * the free shim (rc1_func_00115F78) calls with the reentrancy pointer. Same
 * profile and same constants as src/core/001166C8.c (mALLOc, the unit before
 * it): -O2 -G2 -ffunction-sections -mastra-quad-saves, SIZE_SZ 4, MINSIZE 16,
 * MALLOC_ALIGNMENT 16, HAVE_MMAP 0, DEBUG off, so no mmap arm and no
 * check_inuse_chunk. The three statics this body needs are trim_threshold
 * 0x001345F0 (unsigned long, read with `ld`), top_pad 0x001345F8 (unsigned
 * long, narrowed to size_t for the malloc_trim call) and the bin array
 * 0x001341E8 (top at +8, last_remainder at +8 of bin 1). The lock/unlock pair
 * is func_00116300/func_00116360 and malloc_trim is func_00117108. */

struct _reent;

struct malloc_chunk {
    u32 prev_size;
    u32 size;
    struct malloc_chunk *fd;
    struct malloc_chunk *bk;
};

typedef struct malloc_chunk *mchunkptr;
typedef struct malloc_chunk *mbinptr;

typedef u32 INTERNAL_SIZE_T;
typedef void Void_t;

extern void func_00116300(struct _reent *);
extern void func_00116360(struct _reent *);
extern int func_00117108(struct _reent *, u32);

extern u64 D_001345F0;
extern u64 D_001345F8;

extern u32 D_001341E8[];

#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))
#define MALLOC_ALIGNMENT       16
#define MALLOC_ALIGN           MALLOC_ALIGNMENT
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MINSIZE                (sizeof(struct malloc_chunk))

#define chunk2mem(p)   ((Void_t *)((char *)(p) + 2 * SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char *)(mem) - 2 * SIZE_SZ))

#define PREV_INUSE 0x1
#define IS_MMAPPED 0x2
#define SIZE_BITS (PREV_INUSE | IS_MMAPPED)

#define chunk_at_offset(p, s)  ((mchunkptr)(((char *)(p)) + (s)))

#define inuse_bit_at_offset(p, s)                                             \
 (((mchunkptr)(((char *)(p)) + (s)))->size & PREV_INUSE)

#define chunksize(p)          ((p)->size & ~(SIZE_BITS))
#define set_head(p, s)        ((p)->size = (s))
#define set_foot(p, s)   (((mchunkptr)((char *)(p) + (s)))->prev_size = (s))

#define av_       D_001341E8

#define bin_at(i)      ((mbinptr)((char *)&(av_[2 * (i) + 2]) - 2 * SIZE_SZ))

#define top            (bin_at(0)->fd)
#define last_remainder (bin_at(1))

#define first(b) ((b)->fd)
#define last(b)  ((b)->bk)

#define bin_index(sz)                                                         \
(((((unsigned long)(sz)) >> 9) ==    0) ?       (((unsigned long)(sz)) >>  3): \
 ((((unsigned long)(sz)) >> 9) <=    4) ?  56 + (((unsigned long)(sz)) >>  6): \
 ((((unsigned long)(sz)) >> 9) <=   20) ?  91 + (((unsigned long)(sz)) >>  9): \
 ((((unsigned long)(sz)) >> 9) <=   84) ? 110 + (((unsigned long)(sz)) >> 12): \
 ((((unsigned long)(sz)) >> 9) <=  340) ? 119 + (((unsigned long)(sz)) >> 15): \
 ((((unsigned long)(sz)) >> 9) <= 1364) ? 124 + (((unsigned long)(sz)) >> 18): \
                                          126)

#define MAX_SMALLBIN_SIZE   512
#define SMALLBIN_WIDTH        8
#define SMALLBIN_WIDTH_BITS   3
#define MAX_SMALLBIN        (MAX_SMALLBIN_SIZE / SMALLBIN_WIDTH) - 1

#define smallbin_index(sz)  (((unsigned long)(sz)) >> SMALLBIN_WIDTH_BITS)

#define BINBLOCKWIDTH     4

#define binblocks      (bin_at(0)->size)

#define idx2binblock(ix)    ((unsigned long)1 << (ix / BINBLOCKWIDTH))
#define mark_binblock(ii)   (binblocks |= idx2binblock(ii))

#define check_inuse_chunk(P)

#define frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  if (S < MAX_SMALLBIN_SIZE)                                                  \
  {                                                                           \
    IDX = smallbin_index(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    IDX = bin_index(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    if (FD == BK) mark_binblock(IDX);                                         \
    else                                                                      \
    {                                                                         \
      while (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

#define unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}

#define link_last_remainder(P)                                                \
{                                                                             \
  last_remainder->fd = last_remainder->bk =  P;                               \
  P->fd = P->bk = last_remainder;                                             \
}

#define trim_threshold D_001345F0
#define top_pad        D_001345F8

#define MALLOC_LOCK   func_00116300(reent_ptr)
#define MALLOC_UNLOCK func_00116360(reent_ptr)
#define malloc_trim(pad) func_00117108(reent_ptr, (pad))
#define RCALL reent_ptr,

void func_00116E10(struct _reent *reent_ptr, Void_t *mem) {
  mchunkptr p;         /* chunk corresponding to mem */
  INTERNAL_SIZE_T hd;  /* its head field */
  INTERNAL_SIZE_T sz;  /* its size */
  int       idx;       /* its bin index */
  mchunkptr next;      /* next contiguous chunk */
  INTERNAL_SIZE_T nextsz; /* its size */
  INTERNAL_SIZE_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  int       islr;      /* track whether merging with last_remainder */

  if (mem == 0)                              /* free(0) has no effect */
    return;

  MALLOC_LOCK;

  p = mem2chunk(mem);
  hd = p->size;

  check_inuse_chunk(p);

  sz = hd & ~PREV_INUSE;
  next = chunk_at_offset(p, sz);
  nextsz = chunksize(next);

  if (next == top)                            /* merge with top */
  {
    sz += nextsz;

    if (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_size;
      p = chunk_at_offset(p, -prevsz);
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    set_head(p, sz | PREV_INUSE);
    top = p;
    if ((unsigned long)(sz) >= (unsigned long)trim_threshold)
      malloc_trim(RCALL top_pad);
    MALLOC_UNLOCK;
    return;
  }

  set_head(next, nextsz);                    /* clear inuse bit */

  islr = 0;

  if (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_size;
    p = chunk_at_offset(p, -prevsz);
    sz += prevsz;

    if (p->fd == last_remainder)             /* keep as last_remainder */
      islr = 1;
    else
      unlink(p, bck, fwd);
  }

  if (!(inuse_bit_at_offset(next, nextsz)))   /* consolidate forward */
  {
    fwd = next->fd;
    sz += nextsz;

    if (!islr && fwd == last_remainder)        /* re-insert last_remainder */
    {
      islr = 1;
      link_last_remainder(p);
    }
    else
    {
      bck = next->bk;
      fwd->bk = bck;
      bck->fd = fwd;
    }
  }


  set_head(p, sz | PREV_INUSE);
  set_foot(p, sz);
  if (!islr)
    frontlink(p, sz, idx, bck, fwd);

  MALLOC_UNLOCK;
}
