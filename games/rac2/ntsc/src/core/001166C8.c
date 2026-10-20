#include "common.h"

/*
 * A version of malloc/free/realloc written by Doug Lea and released to the
 * public domain (dlmalloc 2.6.5), adapted by newlib. See
 * THIRD_PARTY_NOTICES.md.
 */

/* Source: newlib mallocr.c, mALLOc under DEFINE_MALLOC
 * (build/sdk-compiler-build/source/ee/newlib/libc/stdlib/mallocr.c) - the real
 * allocator the shim malloc (src/core/00115F28.c) calls with the heap handle in
 * place of the reentrancy pointer, as every RARG here does.
 * The unit is built like the rest of the family (src/core/00116460.c,
 * src/core/00117108.c): -O2 -G2 -ffunction-sections -mastra-quad-saves, the
 * profile that puts $s0..$s4 and $ra in TImode (`sq`, 16-byte slots).
 * INTERNAL_SIZE_T is 4 bytes, so SIZE_SZ is 4, MINSIZE = sizeof(malloc_chunk) =
 * 16 and MALLOC_ALIGNMENT is 16 (the `#if MALLOC_ALIGN == 16` arm of the bin
 * scan, `idx < MAX_SMALLBIN`, is the one that is compiled in). HAVE_MMAP is 0
 * and DEBUG is off: the mmap arms and the check_* calls are not in the object.
 * Lock/unlock are func_00116300/func_00116360 and malloc_extend_top is
 * func_00116460; the only global is the bin array 0x001341E8, whose bin 0 holds
 * the top chunk at +8, `binblocks` at +4 and last_remainder at +8. */

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
extern void func_00116460(struct _reent *, INTERNAL_SIZE_T);

extern u32 D_001341E8[];

/* sizes, alignments */

#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))
#define MALLOC_ALIGNMENT       16
#define MALLOC_ALIGN           MALLOC_ALIGNMENT
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MINSIZE                (sizeof(struct malloc_chunk))

#define chunk2mem(p)   ((Void_t *)((char *)(p) + 2 * SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char *)(mem) - 2 * SIZE_SZ))

#define request2size(req)                                                     \
 (((long)((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) <                            \
  (long)(MINSIZE + MALLOC_ALIGN_MASK)) ?                                      \
  ((MINSIZE + MALLOC_ALIGN_MASK) & ~(MALLOC_ALIGN_MASK)) :                    \
  (((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) & ~(MALLOC_ALIGN_MASK)))

/* physical chunk operations */

#define PREV_INUSE 0x1
#define IS_MMAPPED 0x2
#define SIZE_BITS (PREV_INUSE | IS_MMAPPED)

#define chunk_at_offset(p, s)  ((mchunkptr)(((char *)(p)) + (s)))

#define set_inuse_bit_at_offset(p, s)                                         \
 (((mchunkptr)(((char *)(p)) + (s)))->size |= PREV_INUSE)

#define chunksize(p)          ((p)->size & ~(SIZE_BITS))
#define set_head(p, s)        ((p)->size = (s))
#define set_foot(p, s)   (((mchunkptr)((char *)(p) + (s)))->prev_size = (s))

/* bins */

#define av_       D_001341E8

#define bin_at(i)      ((mbinptr)((char *)&(av_[2 * (i) + 2]) - 2 * SIZE_SZ))
#define next_bin(b)    ((mbinptr)((char *)(b) + 2 * sizeof(mbinptr)))
#define prev_bin(b)    ((mbinptr)((char *)(b) - 2 * sizeof(mbinptr)))

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

#define is_small_request(nb) (nb < MAX_SMALLBIN_SIZE - SMALLBIN_WIDTH)

#define BINBLOCKWIDTH     4

#define binblocks      (bin_at(0)->size)

#define idx2binblock(ix)    ((unsigned long)1 << (ix / BINBLOCKWIDTH))
#define mark_binblock(ii)   (binblocks |= idx2binblock(ii))

/* long > size_t: see mallocr.c's SIZE_T_SMALLER_THAN_LONG note */

#define long_sub_size_t(x, y) ( (x < y) ? -((long)(y - x)) : (x - y) );

/* debugging is off in this build */

#define check_malloced_chunk(P, N)

/* macro-based internal utilities */

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

#define clear_last_remainder \
  (last_remainder->fd = last_remainder->bk = last_remainder)

#define MALLOC_LOCK   func_00116300(reent_ptr)
#define MALLOC_UNLOCK func_00116360(reent_ptr)
#define malloc_extend_top(nb) func_00116460(reent_ptr, (nb))
#define RCALL reent_ptr,

void *func_001166C8(struct _reent *reent_ptr, u32 bytes) {
  mchunkptr victim;                  /* inspected/selected chunk */
  INTERNAL_SIZE_T victim_size;       /* its size */
  int       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  long      remainder_size;          /* its size */
  int       remainder_index;         /* its bin index */
  unsigned long block;               /* block traverser bit */
  int       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  INTERNAL_SIZE_T nb  = request2size(bytes);  /* padded request size; */

  MALLOC_LOCK;

  /* Check for exact match in a bin */

  if (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_index(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = last(q);

    if (victim != q)
    {
      victim_size = chunksize(victim);
      unlink(victim, bck, fwd);
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      MALLOC_UNLOCK;
      return chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  else
  {
    idx = bin_index(nb);
    bin = bin_at(idx);

    for (victim = last(bin); victim != bin; victim = victim->bk)
    {
      victim_size = chunksize(victim);
      remainder_size = long_sub_size_t(victim_size, nb);

      if (remainder_size >= (long)MINSIZE) /* too big */
      {
        --idx; /* adjust to rescan below after checking last remainder */
        break;
      }

      else if (remainder_size >= 0) /* exact fit */
      {
        unlink(victim, bck, fwd);
        set_inuse_bit_at_offset(victim, victim_size);
        check_malloced_chunk(victim, nb);
        MALLOC_UNLOCK;
        return chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  if ( (victim = last_remainder->fd) != last_remainder)
  {
    victim_size = chunksize(victim);
    remainder_size = long_sub_size_t(victim_size, nb);

    if (remainder_size >= (long)MINSIZE) /* re-split */
    {
      remainder = chunk_at_offset(victim, nb);
      set_head(victim, nb | PREV_INUSE);
      link_last_remainder(remainder);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);
      check_malloced_chunk(victim, nb);
      MALLOC_UNLOCK;
      return chunk2mem(victim);
    }

    clear_last_remainder;

    if (remainder_size >= 0)  /* exhaust */
    {
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      MALLOC_UNLOCK;
      return chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_size, remainder_index, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  if ( (block = idx2binblock(idx)) <= binblocks)
  {

    /* Get to the first marked block */

    if ( (block & binblocks) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      block <<= 1;
      while ((block & binblocks) == 0)
      {
        idx += BINBLOCKWIDTH;
        block <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    for (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      do
      {
        /* Find and use first big enough chunk ... */

        for (victim = last(bin); victim != bin; victim = victim->bk)
        {
          victim_size = chunksize(victim);
          remainder_size = long_sub_size_t(victim_size, nb);

          if (remainder_size >= (long)MINSIZE) /* split */
          {
            remainder = chunk_at_offset(victim, nb);
            set_head(victim, nb | PREV_INUSE);
            unlink(victim, bck, fwd);
            link_last_remainder(remainder);
            set_head(remainder, remainder_size | PREV_INUSE);
            set_foot(remainder, remainder_size);
            check_malloced_chunk(victim, nb);
            MALLOC_UNLOCK;
            return chunk2mem(victim);
          }

          else if (remainder_size >= 0)  /* take */
          {
            set_inuse_bit_at_offset(victim, victim_size);
            unlink(victim, bck, fwd);
            check_malloced_chunk(victim, nb);
            MALLOC_UNLOCK;
            return chunk2mem(victim);
          }

        }

       bin = next_bin(bin);

#if MALLOC_ALIGN == 16
       if (idx < MAX_SMALLBIN)
         {
           bin = next_bin(bin);
           ++idx;
         }
#endif
      } while ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      do   /* Possibly backtrack to try to clear a partial block */
      {
        if ((startidx & (BINBLOCKWIDTH - 1)) == 0)
        {
          binblocks &= ~block;
          break;
        }
        --startidx;
       q = prev_bin(q);
      } while (first(q) == q);

      /* Get to the next possibly nonempty block */

      if ( (block <<= 1) <= binblocks && (block != 0) )
      {
        while ((block & binblocks) == 0)
        {
          idx += BINBLOCKWIDTH;
          block <<= 1;
        }
      }
      else
        break;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  remainder_size = long_sub_size_t(chunksize(top), nb);
  if (chunksize(top) < nb || remainder_size < (long)MINSIZE)
  {

    /* Try to extend */

    malloc_extend_top(RCALL nb);
    remainder_size = long_sub_size_t(chunksize(top), nb);
    if (chunksize(top) < nb || remainder_size < (long)MINSIZE)
    {
      MALLOC_UNLOCK;
      return 0; /* propagate failure */
    }
  }

  victim = top;
  set_head(victim, nb | PREV_INUSE);
  top = chunk_at_offset(victim, nb);
  set_head(top, remainder_size | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  MALLOC_UNLOCK;
  return chunk2mem(victim);
}
