/* _malloc_trim_r -- RAC5 boot body, w12/q25.
 *
 * Provenance: newlib libc/stdlib/mallocr.c (Doug Lea malloc, DEFINE_FREE),
 * EE newlib snapshot D:/RAC2/master/build/sdk-compiler-build/source/ee/newlib,
 * INTERNAL_NEWLIB profile. Body verbatim (check_chunk is the empty macro).
 * Retail globals: __malloc_av_ 0x01EFEAD0, __malloc_sbrk_base 0x01EFEEE8,
 * __malloc_current_mallinfo 0x01EFEF00 (arena = sbrked_mem), locks
 * 0x01EC87D8/0x01EC8838, _sbrk_r 0x01ECF1C8.
 * Retail: 0x01ECDD30, interval 368 bytes. Toolchain: SN ProDG 3.01
 * ee-gcc2953 (GCC 2.95.3) -O2 -G0, Ps2EeAs.exe, SN ld.exe.
 */
struct _reent;

typedef unsigned int size_t;

typedef struct malloc_chunk* mchunkptr;
typedef struct malloc_chunk* mbinptr;

struct malloc_chunk {
  unsigned int prev_size;
  unsigned int size;
  struct malloc_chunk* fd;
  struct malloc_chunk* bk;
};

struct mallinfo {
  int arena;
  int ordblks;
  int smblks;
  int hblks;
  int hblkhd;
  int usmblks;
  int fsmblks;
  int uordblks;
  int fordblks;
  int keepcost;
};

#define SIZE_SZ 4
#define PREV_INUSE 1
#define IS_MMAPPED 2
#define SIZE_BITS (PREV_INUSE | IS_MMAPPED)
#define NAV 128
#define MINSIZE (sizeof (struct malloc_chunk))
#define malloc_getpagesize 4096
#define MORECORE_FAILURE ((void *)-1)

extern mbinptr __malloc_av_[NAV * 2 + 2];
extern char *__malloc_sbrk_base;
extern struct mallinfo __malloc_current_mallinfo;
extern void __malloc_lock (struct _reent *);
extern void __malloc_unlock (struct _reent *);
extern void *_sbrk_r (struct _reent *, int);

#define av_ __malloc_av_
#define sbrk_base __malloc_sbrk_base
#define current_mallinfo __malloc_current_mallinfo
#define sbrked_mem (current_mallinfo.arena)
#define MALLOC_LOCK __malloc_lock (reent_ptr)
#define MALLOC_UNLOCK __malloc_unlock (reent_ptr)
#define MORECORE(size) _sbrk_r (reent_ptr, (size))

#define bin_at(i) ((mbinptr)((char *)&(av_[2*(i) + 2]) - 2*SIZE_SZ))
#define top (bin_at (0)->fd)
#define chunksize(p) ((p)->size & ~(SIZE_BITS))
#define set_head(p, s) ((p)->size = (s))

int
_malloc_trim_r (struct _reent *reent_ptr, size_t pad)
{
  long top_size;
  long extra;
  char *current_brk;
  char *new_brk;

  unsigned long pagesz = malloc_getpagesize;

  MALLOC_LOCK;

  top_size = chunksize (top);
  extra = ((top_size - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  if (extra < (long)pagesz)
  {
    MALLOC_UNLOCK;
    return 0;
  }
  else
  {
    current_brk = (char *)(MORECORE (0));
    if (current_brk != (char *)(top) + top_size)
    {
      MALLOC_UNLOCK;
      return 0;
    }
    else
    {
      new_brk = (char *)(MORECORE (-extra));

      if (new_brk == (char *)(MORECORE_FAILURE))
      {
        current_brk = (char *)(MORECORE (0));
        top_size = current_brk - (char *)top;
        if (top_size >= (long)MINSIZE)
        {
          sbrked_mem = current_brk - sbrk_base;
          set_head (top, top_size | PREV_INUSE);
        }
        MALLOC_UNLOCK;
        return 0;
      }
      else
      {
        set_head (top, (top_size - extra) | PREV_INUSE);
        sbrked_mem -= extra;
        MALLOC_UNLOCK;
        return 1;
      }
    }
  }
}
